#include <stdio.h>
#include <unistd.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <math.h>
#include "blstoremsg.h"
#include "blockstore.h"
#include "phyblockrw.h"
#include "hmamap.h"

int height_of_hmamap;

int get_hmamap_height(int bitmapsize)
{
	int hmamapleaves;
	int current_level_nodes, parent_level_nodes;
	int hmamap_height;

	PDEBUG_HMAMAP("Entered\n");
	PDEBUG_HMAMAP("INPUT:BITMAPSIZE=%d\n",bitmapsize);
	hmamapleaves = (int)ceil((double)bitmapsize/PER_BLOCK_DBA);
	PDEBUG_HMAMAP("hmamapleaves=%d\n",hmamapleaves);
	
	current_level_nodes = hmamapleaves;
	hmamap_height = 0;
//	parent_level_nodes = (unsigned long)ceil((double)current_level_nodes/PER_BLOCK_DBA);
	parent_level_nodes=0;

	if(hmamapleaves != 1)
	{
		while (parent_level_nodes != 1)
		{
			PDEBUG_HMAMAP("parent_level_nodes=%d\n",parent_level_nodes);
			parent_level_nodes = (int)ceil((double)current_level_nodes/PER_BLOCK_DBA);
			current_level_nodes = parent_level_nodes;
			hmamap_height += 1;
		}
	}

	PDEBUG_HMAMAP("Exiting\n");
	return hmamap_height;
}

int get_hmamapsize(int bitmapsize, int level_nodes[])
{
	int hmamapleaves;
	int current_level_nodes, parent_level_nodes;
	int size, level;

	level=get_hmamap_height(bitmapsize)+1;
	/* 
 	* We are considering the hashmap blocks also but array couting starts at 0
     */

	level_nodes[level--]=bitmapsize;
	hmamapleaves = (int)ceil((double)bitmapsize/PER_BLOCK_DBA);
     level_nodes[level--]=hmamapleaves;	
	
	current_level_nodes = hmamapleaves;
	size = 0;
	parent_level_nodes = 0; // This is required to handle the case where only root 

	if(hmamapleaves != 1)
	{
		while (parent_level_nodes != 1)
		{
			parent_level_nodes = (int)ceil((double)current_level_nodes/PER_BLOCK_DBA);
               level_nodes[level--]=parent_level_nodes;
			current_level_nodes = parent_level_nodes;
			size += parent_level_nodes;
		}
	}

	size+=hmamapleaves;

	PDEBUG_HMAMAP("HMAMAPSIZE=%d\n",size);
	return size;
}

dba_t hmamap_init(int fd, int bitmapsize, dba_t root_hmamap)
{
	int hmamapsize, levels;
	size_t write_bytes;
	int i,j,k;
 	void *buf;
	dba_t tmp_addr, end_bitmap, hmamap_addr;
	
	int level_nodes[get_hmamap_height(bitmapsize)+2];
     /* 2 added because height of bitmap is already less by 1 than the levels
        present in the tree(+1) and also we need to consider the actual bitmap
        level also(+1)
	*/  


	PDEBUG_HMAMAP("Entered hmamap_init\n");
	PDEBUG_HMAMAP("HMAMAP start at:%lu\n", DBA(root_hmamap));
	hmamapsize=get_hmamapsize(bitmapsize,level_nodes);
	levels=get_hmamap_height(bitmapsize)+2;
	PDEBUG_HMAMAP("HMAPMAPSIZE =%d\n",hmamapsize);
	PDEBUG_HMAMAP("LEVELS =%d\n",levels);

	buf = (void *)malloc(sizeof(dba_t)*PER_BLOCK_DBA);

	DBA(tmp_addr)=DBA(root_hmamap)+1;
     hmamap_addr=root_hmamap;
     for(i=1; i<levels; i++)
     {
          PDEBUG_HMAMAP("CURRENT_LEVEL=%d\n",i);
          j=0;
          while(j < level_nodes[i])
          {
               for(k=0; k<PER_BLOCK_DBA; k++)
               {
                    if(j>=level_nodes[i])
                         break;
                    else
                    {
                         *((unsigned long *)buf+k)=DBA(tmp_addr);
                         j++;
                         DBA(tmp_addr)=DBA(tmp_addr)+1;
                    }
               }
               //write block at dba_t gmamap_addr        
               write_bytes=write_block(fd, (char *)buf,hmamap_addr);
               DBA(hmamap_addr) = DBA(hmamap_addr)+1;
		}	
	}

	DBA(end_bitmap)=DBA(tmp_addr)-1;
     //1 is substracted while calculating end_hmamap as root_gmamap is also used
     PDEBUG_HMAMAP("bitmap ends at:%lu\n", DBA(end_bitmap));

	return(end_bitmap);
}


