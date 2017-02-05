#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include "blockstore.h"
#include "hashmap.h"
#include "gmamap.h"
#include "bitmap.h"
#include "hmamap.h"
#include "mapblockaddr.h"
#include "block_cache.h"
#include "phyblockrw.h"


int get_hashmapsize(struct HashFun *hfp)
{
	return((hfp->modvalue)*(hfp->bucketsize));
}


dba_t hashmap_init(struct BlockStore *bsp, struct HashFun *hfp, dba_t start_dba)
{
	int hashmapsize;
	dba_t mapblockdba, last_metadba;
	int i;
	char *block;

	int tmp;


	PDEBUG_HASH("Hashmap_init entered\n");
	hashmapsize=get_hashmapsize(hfp);
	PDEBUG_HASH("Hashmapsize=%d\n",hashmapsize);
	PDEBUG_HASH("GMAMAP_HEIGHT=%d\n",get_gmamap_height(hashmapsize));
	
	PDEBUG_HASH("Before calling gmamap_init\n");
	last_metadba=gmamap_init(bsp->fd, hashmapsize, start_dba);	
	PDEBUG_HASH("After calling gmamap_init\n");

	for(i=0; i<hashmapsize; i++)
	{
		tmp=get_gmamap_height(hashmapsize);
		mapblockdba=getmapblockaddr(bsp, i,tmp,start_dba);

		/*Calloc and fill with zero, unsigned long zero*/
		block=(char *)calloc(BLK_SZ, sizeof(char));
		write_block(bsp->fd, block, mapblockdba);
	}
	PDEBUG_HASH("Hashmap_init leaving\n");
	return last_metadba;
}


dba_t getdba(struct BlockStore *bsp, lba_t lba_addr, bool op)
{
	dba_t dba_addr, ret_addr, dba_free;
	bool lbafound=false;
	bool block_status;

	PDEBUG_HASH("Entering\n");
	lbafound=searchlba(bsp, lba_addr,&dba_addr);

	if(op)
	{
		if(( DBA(dba_addr)==0) ||\
	        ( NULL == cache_lookup(dba_addr, &block_status)) || \
  	        ((NULL != cache_lookup(dba_addr, &block_status)) && \
             (block_status==CLEAN)) 
		  )
		{
			PDEBUG_HASH("CALLING allocatedba\n");
			dba_free=allocatedba(bsp);
			PDEBUG_HASH("RETURNING allocatedba with: %lu\n",DBA(dba_free));
			updatelba(bsp, lba_addr, dba_free);
	
			if((DBA(dba_addr)!=0) && (DBA(dba_addr)!=1))
			{
				PDEBUG_HASH("CALLING deallocatedba with: %lu\n",DBA(dba_addr));
				deallocatedba(bsp, dba_addr);
				PDEBUG_HASH("RETURNING deallocatedba\n");
				cache_blockdelete(dba_addr);
			}
		
			ret_addr=dba_free;
		}
		else
			ret_addr=dba_addr;
		
	}
	else
	{
		ret_addr=dba_addr;
	}
	
	PDEBUG_HASH("Exiting:%lu\n",DBA(dba_free));
	return(ret_addr);
}


bool searchlba(struct BlockStore *bsp, lba_t lba_addr, dba_t *dba_addr)
{
     int bucketid;
     int i, blocknum, gmamap_height;
     dba_t mapblockaddr, ret_dba;
     char block[BLK_SZ];
	bool lbafound;
	size_t readbytes;
     struct hash_addr *hap;

	PDEBUG_HASH("Entered:%llu\n",LBA(lba_addr));
     bucketid=(int)(LBA(lba_addr)%(bsp->hfp->modvalue));
     blocknum=bucketid*(bsp->hfp->bucketsize);

	gmamap_height= get_gmamap_height(get_hashmapsize(bsp->hfp));
	mapblockaddr = getmapblockaddr(bsp,blocknum,\
					gmamap_height, bsp->sbp->gma_root);
     /*Here loop may have to include when we are gonna implement rescale hash*/
	readbytes=cache_blockread(bsp, mapblockaddr, block);

	DBA(ret_dba)=0;
	lbafound=false;
     for(i=0; i < bsp->hfp->bucksize; i++)
     {
          hap=(struct hash_addr *)(block+(i*sizeof(struct hash_addr)));
		
          if(LBA(hap->lba)==LBA(lba_addr))
          {
			PDEBUG_HASH("IF Entered\n");
			lbafound=true;
               ret_dba=hap->dba;
               break;
          }
     }
	*dba_addr=ret_dba;

	PDEBUG_HASH("Exiting:%lu\n",DBA(ret_dba));

	if(lbafound) PDEBUG_HASH("lbafound=true\n");
	else PDEBUG_HASH("lbafound=false\n");

     return lbafound;
}


/*Need to be addressed the issue of updating the gmamap tree recursively
 * If at all this function is going to be used anywhere. Currently, I do
 * not see the need to use this function*/
void updatelba(struct BlockStore *bsp, lba_t lba_addr, dba_t dba_addr)
{
     int bucketid;
     int i, blocknum, gmamap_height;
     dba_t mapblockaddr, free_dba, gma_root;
     char block[BLK_SZ];
	size_t readbytes, writebytes;
     struct hash_addr *hap;
	bool block_status;

     bucketid=(int)(LBA(lba_addr)%bsp->hfp->modvalue);
     blocknum=bucketid*(bsp->hfp->bucketsize);

	gmamap_height= get_gmamap_height(get_hashmapsize(bsp->hfp));
	mapblockaddr = getmapblockaddr(bsp,blocknum,\
					gmamap_height, bsp->sbp->gma_root);
     /*Here loop may have to include when we are gonna implement rescale hash*/
	readbytes=cache_blockread(bsp, mapblockaddr, block);

     for(i=0; i < bsp->hfp->bucksize; i++)
     {
          hap=(struct hash_addr *)(block+(i*sizeof(struct hash_addr)));
		PDEBUG_HASH("LBA=%llu\n",LBA(hap->lba));

          if(LBA(hap->lba)==LBA(lba_addr))
          {
               hap->dba=dba_addr;
			PDEBUG_HASH("LBA=%llu\tDBA=%lu\n",LBA(hap->lba),DBA(hap->dba));
               break;
          }
     }

	if(  (NULL == cache_lookup(mapblockaddr, &block_status)) || \
	     ((NULL != cache_lookup(mapblockaddr, &block_status)) && \
         (block_status==CLEAN)) )
	{
		PDEBUG_HASH("CALLING allocatedba\n");
		free_dba=allocatedba(bsp);
		PDEBUG_HASH("RETURNING allocatedba with:%lu\n",DBA(free_dba));
		writebytes=cache_blockwrite(bsp, mapblockaddr, free_dba, block);

		PDEBUG_HASH("CALLING updatemapblockaddr for GMA_ROOT\n");
	     gma_root=updatemapblockaddr(bsp, blocknum, gmamap_height,\
     	                    free_dba, bsp->sbp->gma_root);
		PDEBUG_HASH("UPDATED:blocknum=%d\tmapblockdba=%lu\n",blocknum,DBA(free_dba));

     	bsp->sbp->gma_root=gma_root;
	}
	else
	{
		writebytes=cache_blockwrite(bsp, mapblockaddr, mapblockaddr, block);
	}

     return;
}


void deletelba(struct BlockStore *bsp, lba_t lba_addr)
{
     int bucketid;
     int i, blocknum, gmamap_height;
     dba_t mapblockaddr, free_dba, gma_root;
	lba_t freelba;
     char block[BLK_SZ];
	size_t readbytes, writebytes;
     struct hash_addr *hap;
	bool block_status;

	PDEBUG_HASH("Entered:%llu\n", LBA(lba_addr));

     bucketid=(int)(LBA(lba_addr)%bsp->hfp->modvalue);
     blocknum=bucketid*(bsp->hfp->bucketsize);

	gmamap_height= get_gmamap_height(get_hashmapsize(bsp->hfp));
	mapblockaddr = getmapblockaddr(bsp,blocknum,\
					gmamap_height, bsp->sbp->gma_root);
     /*Here loop may have to include when we are gonna implement rescale hash*/
	readbytes=cache_blockread(bsp, mapblockaddr, block);

	LBA(freelba)=0;
     for(i=0; i < bsp->hfp->bucksize; i++)
     {
		PDEBUG_HASH("Inside for:%llu\n",LBA(lba_addr));
          hap=(struct hash_addr *)(block+(i*sizeof(struct hash_addr)));
          if(LBA(hap->lba)==LBA(lba_addr))
          {
			PDEBUG_HASH("Inside if:%llu\n",LBA(lba_addr));
			PDEBUG_HASH("Inside if:%llu\n",LBA(hap->lba));
               hap->lba=freelba;
			PDEBUG_HASH("Inside if:%llu\n",LBA(hap->lba));
			PDEBUG_HASH("Inside if:%llu\n",LBA(freelba));
               break;
          }
     }


	if(  (NULL == cache_lookup(mapblockaddr, &block_status)) || \
	     ((NULL != cache_lookup(mapblockaddr, &block_status)) && \
         (block_status==CLEAN)) )
	{
		PDEBUG_HASH("CALLING allocatedba\n");
		free_dba=allocatedba(bsp);
		PDEBUG_HASH("RETURNING allocatedba with:%lu\n",DBA(free_dba));
		writebytes=cache_blockwrite(bsp, mapblockaddr, free_dba, block);

		PDEBUG_HASH("CALLING updatemapblockaddr for GMA_ROOT\n");
	     gma_root=updatemapblockaddr(bsp, blocknum, gmamap_height,\
                         free_dba, bsp->sbp->gma_root);
		PDEBUG_HASH("UPDATED:blocknum=%d\tmapblockdba=%lu\n",blocknum,DBA(free_dba));
	     bsp->sbp->gma_root=gma_root;
	}
	else
	{
		writebytes=cache_blockwrite(bsp, mapblockaddr, mapblockaddr, block);
	}

	PDEBUG_HASH("Exiting\n");
     return;
}


void addlba(struct BlockStore *bsp, lba_t lba_addr)
{
     int bucketid;
     int i, blocknum, gmamap_height;
     dba_t mapblockaddr, free_dba, gma_root;
     char block[BLK_SZ];
	size_t readbytes,  writebytes;
     struct hash_addr *hap;
	bool block_status;

	PDEBUG_HASH("Entered:lba=%llu\n",LBA(lba_addr));
     bucketid=(int)(LBA(lba_addr)%bsp->hfp->modvalue);
     blocknum=bucketid*(bsp->hfp->bucketsize);
	PDEBUG_HASH("Hashmap blocknum=%d\n",blocknum);

	gmamap_height= get_gmamap_height(get_hashmapsize(bsp->hfp));
	PDEBUG_HASH("gmamap height=%d\n",gmamap_height);

	mapblockaddr = getmapblockaddr(bsp,blocknum,
					gmamap_height, bsp->sbp->gma_root);
	PDEBUG_HASH("blocknum=%d\tblockaddr=%lu\n",blocknum, DBA(mapblockaddr));
     /*Here loop may have to include when we are gonna implement rescale hash*/

	readbytes=cache_blockread(bsp, mapblockaddr, block);

     for(i=0; i < bsp->hfp->bucksize; i++)
     {
          hap=(struct hash_addr *)(block+(i*sizeof(struct hash_addr)));
          if(LBA(hap->lba)==0)
          {
			PDEBUG_HASH("Entered...???\n");
			DBA(free_dba)=0;
               hap->lba=lba_addr;
               hap->dba=free_dba;
               break;
          }
     }
	/*There should be condition here to check if the bucksize has crossed
 	 *the lower limit and hence we should rescale the hash*/

	if(  (NULL == cache_lookup(mapblockaddr, &block_status)) || \
	     ((NULL != cache_lookup(mapblockaddr, &block_status)) && \
         (block_status==CLEAN)) )
	{
		PDEBUG_HASH("calling allocatedba \n");
		free_dba=allocatedba(bsp);	
		PDEBUG_HASH("Returning allocatedba with:%lu\n",DBA(free_dba));
		writebytes=cache_blockwrite(bsp, mapblockaddr, free_dba, block);

		PDEBUG_HASH("CALLING updatemapblockaddr for GMA_ROOT\n");
		gma_root=updatemapblockaddr(bsp, blocknum, gmamap_height,\
                         free_dba, bsp->sbp->gma_root);
		PDEBUG_HASH("UPDATED:blocknum=%d\tmapblockdba=%lu\n",blocknum,DBA(free_dba));

		bsp->sbp->gma_root=gma_root;
	}
	else
	{
		writebytes=cache_blockwrite(bsp, mapblockaddr, mapblockaddr, block);
	}

	PDEBUG_HASH("Exiting\n");
     return;
}

unsigned long getreqspace(bl_transaction_t trans)
{
     int bitmapsize, hmamapsize, hashmapsize, gmamap_height;
     int *level_nodes;
     unsigned long reqspace;

     bitmapsize=get_bitmapsize(trans->bsp->nBlocks);

     level_nodes=(int *)malloc((get_hmamap_height(bitmapsize)+2)*sizeof(int));
     hmamapsize=get_hmamapsize(bitmapsize, level_nodes);
     free(level_nodes);

     hashmapsize=get_hashmapsize(trans->bsp->hfp);
     gmamap_height=get_gmamap_height(hashmapsize);

     reqspace=bitmapsize+hmamapsize+(gmamap_height+2)+1;
//     freeSpace=getfreespace(trans->bsp);

     PDEBUG_HASH("bitmapsize=%d\thmamapsize=%d\n",bitmapsize,hmamapsize);
     PDEBUG_HASH("gmamap_height=%d\n",gmamap_height);
     PDEBUG_HASH("reqSpace=%lu\n",reqspace);

	return reqspace;
}
