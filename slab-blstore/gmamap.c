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
#include "gmamap.h"

int height_of_gmamap;

int get_gmamap_height(int hashmapsize)
{
	int gmamapleaves;
	int current_level_nodes, parent_level_nodes;
	int gmamap_height;

	gmamapleaves = (int)ceil((double)hashmapsize/PER_BLOCK_DBA);
	
	current_level_nodes = gmamapleaves;
	gmamap_height = 0;
//	parent_level_nodes = (unsigned long)ceil((double)current_level_nodes/PER_BLOCK_DBA);
	parent_level_nodes = 0;

	/*If gmamapleaves is equal to one means there is no leaf. What present is
 	* is root of the tree
 	*/
	if(gmamapleaves != 1)
	{
		while (parent_level_nodes != 1)
		{
			parent_level_nodes = (int)ceil((double)current_level_nodes/PER_BLOCK_DBA);
			current_level_nodes = parent_level_nodes;
			gmamap_height += 1;
		}
	}
	return gmamap_height;
}

/*int get_gmamapsize(int hashmapsize)
{
	int gmamapleaves;
	int current_level_nodes, parent_level_nodes;
	int size;

	gmamapleaves = (int)ceil((double)hashmapsize/PER_BLOCK_DBA);
	
	current_level_nodes = gmamapleaves;
	size = 0;
	parent_level_nodes = 0; // This is required to handle the case where only root 
	
	if(gmamapleaves != 1)
	{
		while (parent_level_nodes != 1)
		{
			parent_level_nodes = (int)ceil((double)current_level_nodes/PER_BLOCK_DBA);
			current_level_nodes = parent_level_nodes;
			size += parent_level_nodes;
		}
	}
	size+=gmamapleaves;

	return size;
}*/

int get_gmamapsize(int hashmapsize, int level_nodes[])
{
	int gmamapleaves;
	int current_level_nodes, parent_level_nodes;
	int size,level;
	
	level=get_gmamap_height(hashmapsize)+1;
	/* 
 	* We are considering the hashmap blocks also but array couting starts at 0
	*/	

	level_nodes[level--]=hashmapsize;
	gmamapleaves = (int)ceil((double)hashmapsize/PER_BLOCK_DBA);
	level_nodes[level--]=gmamapleaves;	

	current_level_nodes = gmamapleaves;
	size = 0;
	parent_level_nodes = 0; // This is required to handle the case where only root 
	
	if(gmamapleaves != 1)
	{
		while (parent_level_nodes != 1)
		{
			parent_level_nodes = (int)ceil((double)current_level_nodes/PER_BLOCK_DBA);
			level_nodes[level--]=parent_level_nodes;
			current_level_nodes = parent_level_nodes;
			size += parent_level_nodes;
		}
	}
	size+=gmamapleaves;

	return size;
}


dba_t gmamap_init(int fd, int hashmapsize, dba_t root_gmamap)
{
	int gmamapsize, levels;
	size_t write_bytes;
	int i,j,k;
 	void *buf;
	dba_t tmp_addr, end_hashmap, gmamap_addr;

	int level_nodes[(get_gmamap_height(hashmapsize)+2)];
	/* 2 added because height of gmamap is already less by 1 than the levels
	present in the tree(+1) and also we need to consider the actual hashmap
	also(+1)*/	

	PDEBUG_GMAMAP("GMAMAP start at:%lu\n", DBA(root_gmamap));
	gmamapsize=get_gmamapsize(hashmapsize, level_nodes);
	levels=get_gmamap_height(hashmapsize)+2;
	PDEBUG_GMAMAP("GMAPMAPSIZE=%d\n",gmamapsize);
	PDEBUG_GMAMAP("levels=%d\n",levels);

	buf = (void *)malloc(sizeof(dba_t)*PER_BLOCK_DBA);

	DBA(tmp_addr)=DBA(root_gmamap)+1;
	gmamap_addr=root_gmamap;
	for(i=1; i<levels; i++)
	{
		PDEBUG_GMAMAP("CURRENT_LEVEL=%d\n",i);
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
			//write block at dba_t root_gmamap+i		
			write_bytes=write_block(fd, (char *)buf, gmamap_addr);
			DBA(gmamap_addr) = DBA(gmamap_addr)+1;
		}
	}

	DBA(end_hashmap)=DBA(tmp_addr)-1; 
	// 1 is substracted while calculating end_gmamap as root_gmamap is also used
	PDEBUG_GMAMAP("GMAMAP ends at:%lu\n", DBA(end_hashmap));
	return(end_hashmap);
}


