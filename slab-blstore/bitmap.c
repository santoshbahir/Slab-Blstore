/*******************************************************************************
 * 1. Changed the parameter to get_hmamap_height function from nBlocks to      *
 *    bitmapsize                                                               *
 ******************************************************************************/

 #include <stdio.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <sys/stat.h>
 #include <stdlib.h>
 #include <inttypes.h>
 #include "blockstore.h"   
 #include "hmamap.h"
 #include "bitmap.h"   
 #include "phyblockrw.h"   
 #include "hashmap.h"   
 #include "math.h"   
 #include "mapblockaddr.h"   


int get_bitmapsize(dba_t volumesize)
{
     return (int)ceil((double)DBA(volumesize)/(double)(BLK_SZ*8));
}


void bitmap_init(struct BlockStore *bsp, dba_t bitmap_start_addr, dba_t nBlocks)
{
	int i;
	unsigned long j;
	dba_t tmp_dba, last_metadba, mapblockdba, setcleardba;
	int bitmapsize;
	char bitmapblock[BLK_SZ];

	PDEBUG_BITMAP("Entered bitmap_init\n");
	bitmapsize=get_bitmapsize(nBlocks);
     PDEBUG_BITMAP("BITMAPSIZE=%d\n",bitmapsize);
     PDEBUG_BITMAP("HMAMAP_HEIGHT=%d\n",get_hmamap_height(bitmapsize));
	last_metadba=hmamap_init(bsp->fd, get_bitmapsize(nBlocks), bitmap_start_addr);
	
	DBA(tmp_dba)=0;
	for(i=0; i<bitmapsize; i++)
	{
		mapblockdba=getmapblockaddr(bsp, i, get_hmamap_height(bitmapsize),\
								bitmap_start_addr);

		for(j=0; j<(BLK_SZ*8); j++)
		{
			if (DBA(tmp_dba) < DBA(nBlocks))
			{
				if(DBA(tmp_dba) <= DBA(last_metadba))
				{
					DBA(setcleardba)=j;
					set_bit((uint8_t *)bitmapblock, j);
				}
				else
				{
					DBA(setcleardba)=j;
					clear_bit((uint8_t *)bitmapblock, j);
				}
			}
			else
			{
				break;	
			}
			DBA(tmp_dba)=DBA(tmp_dba) + 1;
		}
		
		write_block(bsp->fd, bitmapblock, mapblockdba);
	}
	PDEBUG_BITMAP("Leaving bitmap_init\n");
}


dba_t allocatedba(struct BlockStore *bsp)
{
	struct bitmap_block **bitmap;
	dba_t free_dba, bmp_dba, hma_root;
	int index, hmamap_height;
	
	PDEBUG_BITMAP("Entered allocatedba\n");
	hmamap_height=get_hmamap_height(get_bitmapsize(bsp->nBlocks));
	PDEBUG_BITMAP("hmamap_height:%d\n",hmamap_height);
	bitmap=get_newbitmap();
	free_dba=getfreedba(get_bitmapsize(bsp->nBlocks),	bsp->nBlocks);
	PDEBUG_BITMAP("free_dbaaaaaaaaaaaaaaaaaaaaaaaaaaaa:%lu\n",DBA(free_dba));
	
	index=DBA(free_dba)/(BLK_SZ*8);
	PDEBUG_BITMAP("index:%d\n", index);
	bitmap[index]->status=DIRTY;

	if(!(bitmap[index]->dba_assigned))
	{
		PDEBUG_BITMAP("Recursion failing condition\n");
		bitmap[index]->dba_assigned=true;
		PDEBUG_BITMAP("CALLING allocatedba\n");
		bmp_dba=allocatedba(bsp);
		PDEBUG_BITMAP("RETURNED allcoatedba with dba:%lu\n",DBA(bmp_dba));
		bitmap[index]->new_mapblockdba=bmp_dba;

		PDEBUG_BITMAP("For hma_root calling updatemapblockaddr\n");
		hma_root=updatemapblockaddr(bsp, index, hmamap_height,\
                         bmp_dba, bsp->sbp->hma_root);
		PDEBUG_BITMAP("New hma_root:%lu\n",DBA(hma_root));
		bsp->sbp->hma_root=hma_root;
	}

	PDEBUG_BITMAP("Exiting \n");
	return(free_dba); 
}

void deallocatedba(struct BlockStore *bsp, dba_t dba_addr)
{
	struct bitmap_block **bitmap;
	dba_t bmp_dba, hma_root;
	int index, hmamap_height;

	PDEBUG_BITMAP("Entered\n");
	hmamap_height=get_hmamap_height(get_bitmapsize(bsp->nBlocks));
	bitmap=get_newbitmap();
	setfreedba(dba_addr);

	index=DBA(dba_addr)/(BLK_SZ*8);
	PDEBUG_BITMAP("Index:%d\n", index);
	bitmap[index]->status=DIRTY;

	if(!(bitmap[index]->dba_assigned))
	{
		bitmap[index]->dba_assigned=true;
		PDEBUG_BITMAP("CALLING allocatedba,\n");
		bmp_dba=allocatedba(bsp);
		PDEBUG_BITMAP("RETURNED allocatedba with dba:%lu\n",DBA(bmp_dba));
		bitmap[index]->new_mapblockdba=bmp_dba;

		hma_root=updatemapblockaddr(bsp, index, hmamap_height,\
                         bmp_dba, bsp->sbp->hma_root);
		bsp->sbp->hma_root=hma_root;
	}
	
	PDEBUG_BITMAP("Freeeeeeeeeeeeeeeeeeeeeeeeeeeeed DBA:%lu\n",DBA(dba_addr));
	PDEBUG_BITMAP("Exiting\n");
	return;
}
