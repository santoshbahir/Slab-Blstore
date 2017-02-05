#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include "blstoremsg.h"
#include "blockstore.h"
#include "hashmap.h"
#include "bitmap.h"
#include "inmem_bitmap.h"
#include "gmamap.h"
#include "hmamap.h"
#include "block_cache.h"
#include "phyblockrw.h"


const char *result_name(result_t r)
{
  switch (r) {
  case result_OK:           return "OK";
  case result_NoAccess:     return "no accesss";
  case result_NotFound:     return "not found";
  case result_Exists:       return "store exists";
  case result_NoSpace:      return "no space";
  case result_BadArg:       return "bad argument value";
  case result_UnInit:       return "uninitialized read";
  case result_IoError:      return "I/O error";
  case result_TxAbort:      return "transaction aborted";
  case result_TooSmall:     return "too small";
  case result_BadFS:        return "Inconsistent File System";
  }

  assert(0 && "unkown result code");
}

/*Definition all the global variables. This will be exported as extern from the header file
 *Some of these could be defined as #define constant also
 */

unsigned long long buckets_per_block = 15;
unsigned long long total_lbas = 42; //225;

/*int no_blks_for_hashmap;
int no_blks_for_bitmap;
int no_of_gma_per_blk;
int no_of_hma_per_blk;
int blocks_per_level[MAX_LEVEL];*/

BlockStore *
bl_create(const char *path, dba_t nBlocks, result_t *result)
{

     PDEBUG_BLOCKSTORE("Entered the bl_create\n");
	BlockStore *bstore;
	struct superblock sb1, sb2;
	char *pad;

	off_t offset;
	size_t size;
	dba_t start_addr, tmpdba;
	dba_t	gmatree_root;
	dba_t	hmatree_root;
	struct HashFun *hfp;


	pad = (char*)calloc(DBA(nBlocks)*BLK_SZ, sizeof(char));
	//	off_t offset;		/*This is the position sought by lseek*/

	bstore =(BlockStore *)malloc(sizeof(BlockStore));
	//open file	

	bstore->file = path;
	bstore->fd = open(path, O_RDWR | O_CREAT, S_IRWXU);
	bstore->nBlocks = nBlocks;
     PDEBUG_BLOCKSTORE("\nBlockstore is created successfully:%d\n",bstore->fd);

	if(bstore->fd == -EEXIST)
	{
		*result = result_Exists;
	}
	else if(errno == -EACCES || errno == -EPERM)
	{
		*result = result_NoAccess;
	}
	else
	{
		*result = result_OK;
	}
	
	if(*result == result_OK)
	{
		offset = lseek(bstore->fd,0,SEEK_SET);
		size=write(bstore->fd,pad,BLK_SZ*DBA(nBlocks));
		if(size != (size_t)BLK_SZ*DBA(nBlocks))
		{
			PDEBUG_BLOCKSTORE("Error in zeroing out the whole blockstore");
		}
		
		DBA(start_addr) = 2;
	
		hfp=(struct HashFun *)malloc(sizeof(struct HashFun));
		hfp->modvalue=255;
		hfp->bucketsize=1;
		hfp->bucksize=85;
		hfp->lowerlimit=-1; /*For rescale hash*/
		hfp->upperlimit=-1; /*For rescale hash*/
	     PDEBUG_BLOCKSTORE("Hash function details are stored successfully\n ");

		gmatree_root=start_addr;
	     PDEBUG_BLOCKSTORE("Before calling hashmap_init\n ");
		tmpdba=hashmap_init(bstore, hfp, start_addr);
	     PDEBUG_BLOCKSTORE("After calling hashmap_init\n ");

		DBA(tmpdba)=DBA(tmpdba)+1;
		hmatree_root=tmpdba;
		bitmap_init(bstore, tmpdba, nBlocks);
	     PDEBUG_BLOCKSTORE("Bitmap initialized successfully\n ");
	}

	sb1.hashinfo = *hfp;
	sb1.hma_root = hmatree_root;
	sb1.gma_root = gmatree_root;
	sb1.nBlocks = nBlocks;	
	DBA(sb1.selfaddr) = 0;
	sb1.transaction_id=0;	
	LBA(sb1.latestlba)=1;
	
	sb2 = sb1;
	DBA(sb2.selfaddr) = 1;
	
	/*Write first superblock*/
	DBA(tmpdba)=0;
	write_block(bstore->fd, (char *)(&sb1), tmpdba);

	/*Write second superblock*/
	DBA(tmpdba)=1;
	write_block(bstore->fd, (char *)(&sb2), tmpdba);

	close(bstore->fd);
     PDEBUG_BLOCKSTORE("returning from the bl_create");
	return bstore;
}

BlockStore *
bl_open(const char *path, result_t *result)
{
	BlockStore *bstore;
	dba_t dbaaddr;
	struct superblock *sup_blk1, *sup_blk2;
	size_t read_bytes;
	off_t offset;
  
     bstore =(BlockStore *)malloc(sizeof(BlockStore));

	bstore->fd = open(path, O_RDWR | O_CREAT, S_IRWXU);
	bstore->file = path;
	/*Need to add nBlocks need to calculate or get from somewhere*/	
/*	
 *	Handle file not found case
 */
	if(errno == -EACCES || errno == -EPERM)
	{
   	*result = result_NoAccess;
	}
	else 
	{
   	*result = result_OK;
	}

	if(*result == result_OK)
	{
		/*
	 	* 	Get the appropriate superblock and get transaction ID
	 	*  Read the first superblock,
	 	*  read the transaction ID and increment and store in the blockstore struct
	 	*/

		sup_blk1=(struct superblock *)malloc(sizeof(struct superblock));
		sup_blk2=(struct superblock *)malloc(sizeof(struct superblock));

		offset = lseek(bstore->fd,0, SEEK_SET);
		DBA(dbaaddr)=0;
		read_bytes=read_block(bstore->fd, (char *)sup_blk1,dbaaddr);
		DBA(dbaaddr)=1;
		read_bytes=read_block(bstore->fd, (char *)sup_blk2,dbaaddr);

		if(sup_blk1->transaction_id > sup_blk2->transaction_id)
		{
			bstore->sbp=sup_blk2;
			bstore->sbp_old=sup_blk1;
			if(LBA(sup_blk1->latestlba) > LBA(sup_blk2->latestlba))
				bstore->sbp->latestlba=sup_blk1->latestlba;
			DBA(bstore->sb_addr)=1;
		//	sup_blk2->transaction_id=sup_blk2->transaction_id+1;
			bstore->hfp=&(sup_blk2->hashinfo);
			bstore->nBlocks=sup_blk2->nBlocks;
		}
		else
		{
		//	sup_blk1->transaction_id=sup_blk1->transaction_id+1;
			bstore->sbp=sup_blk1;
			bstore->sbp_old=sup_blk2;
			if(LBA(sup_blk2->latestlba) > LBA(sup_blk1->latestlba))
				bstore->sbp->latestlba=sup_blk2->latestlba;
			DBA(bstore->sb_addr)=0;
			bstore->hfp=&(sup_blk1->hashinfo);
			bstore->nBlocks=sup_blk1->nBlocks;
		}
		
		bstore->hfp=&(bstore->sbp->hashinfo);
	}

  	return bstore;
}

result_t 
bl_close(BlockStore *bl)
{
	free(bl->sbp);
	free(bl->sbp_old);
	close(bl->fd);
	free(bl);
	return result_OK;
}

result_t bl_grow(BlockStore *bl, dba_t nBlocks)
{
  return result_OK;
}

result_t 
bl_shrink(BlockStore *bl, dba_t nBlocks)
{
  return result_OK;
}

bl_transaction_t 
bl_BeginTransaction(BlockStore *bl)
{
	bl_transaction_t curr_trans;
	int hmatree_height;
	int bitmapsize;
/*testing.sn
	char block[BLK_SZ];
	uint8_t status;
	size_t read_bytes;
	dba_t tmp_dba;
	int i, bmpsize;
	bool s;
//testing.en*/

	PDEBUG_BLOCKSTORE("Entered bl_BeginTransaction \n");
	curr_trans =(bl_transaction_t)malloc(sizeof(struct bl_transaction));
	curr_trans->trans_id = bl->sbp->transaction_id + 1;
	curr_trans->trans_status = 1;	// valid transaction
	curr_trans->bsp=bl;

	PDEBUG_BLOCKSTORE("1--->nBLocks=%lu\n",DBA(bl->nBlocks));
	bitmapsize=get_bitmapsize(bl->nBlocks);
	PDEBUG_BLOCKSTORE("2--->\n");
	hmatree_height=get_hmamap_height(bitmapsize);
	PDEBUG_BLOCKSTORE("bitmapsize=%d\thmatree_height=%d\n"\
					,bitmapsize,hmatree_height);
	
	cache_init();
	PDEBUG_BLOCKSTORE("Cache initiated:\n");
	init_inmembitmap(bl, bitmapsize, hmatree_height, bl->sbp->hma_root);
	PDEBUG_BLOCKSTORE("in-memory bitmap initiated:\n");

/*testing.sn
	DBA(tmp_dba)=45;
	read_bytes=read_block(bl->fd, block, tmp_dba);
	
	bmpsize=get_bitmapsize(bl->nBlocks);
	tmp_dba=getfreedba(bmpsize, bl->nBlocks);

	debug_flag=1;
	DBA(tmp_dba)=8999;
	s=validfreedba(1,tmp_dba);
	if(s)
		PDEBUG_BLOCKSTORE("This should print 47:%d\n",i);
	debug_flag=0;
	for(i=0; i<100; i++)
	{
		status=check_bit((uint8_t *)block, i);
//		PDEBUG_BLOCKSTORE("%d And Hope is good :-):%hhu\n",i, status);
	}
//testing.en*/

	bl->current_trans=curr_trans;
	PDEBUG_BLOCKSTORE("Exitting bl_BeginTransaction:%llu \n",curr_trans->trans_id);
	return curr_trans;
}

result_t 
bl_read(bl_transaction_t trans, lba_t lba, obByte* s)
{
	bool opWrite=false;
	dba_t dba;
	size_t readBytes;

	PDEBUG_BLOCKSTORE("LBA=%llu\n", LBA(lba));

	if((trans->trans_id) < (trans->bsp->current_trans->trans_id))
		return result_BadArg;		

	if(!searchlba(trans->bsp, lba, &dba))
		return result_NotFound;
	
	dba=getdba(trans->bsp, lba, opWrite);
	PDEBUG_BLOCKSTORE("DBA=%lu\n", DBA(dba));
	if(DBA(dba) != 0)
		readBytes=cache_blockread(trans->bsp, dba, (char *)s);
	

	return result_OK;
}

result_t 
bl_write(bl_transaction_t trans, lba_t lba, obByte *s)
{
	bool opWrite=true, lbaFound=false;
	dba_t newdba, olddba;
	size_t writeBytes;
	unsigned long reqSpace, freeSpace;

	PDEBUG_BLOCKSTORE("Entered=%llu\n", LBA(lba));
	PDEBUG_BLOCKSTORE("CLIENT TRANS_ID=%llu\n", trans->trans_id);
	PDEBUG_BLOCKSTORE("OUR_ID=%llu\n", (trans->bsp->current_trans->trans_id));

	reqSpace=getreqspace(trans);
	freeSpace=getfreespace(trans->bsp);
	
	PDEBUG_BLOCKSTORE("reqSpace=%lu\tfreeSpace=%lu\n",reqSpace,freeSpace);

	if(reqSpace > freeSpace)
		return result_NoSpace;
	
/*Checking noSpace condition*/

	if((trans->trans_id) < (trans->bsp->current_trans->trans_id))
		return result_BadArg;		

	lbaFound=searchlba(trans->bsp, lba, &olddba);
	PDEBUG_BLOCKSTORE("OLDDBA=%lu\tOLDDBA=%lu\n", DBA(olddba),DBA(olddba));

	if(!lbaFound)
		return result_NotFound;
	
	newdba=getdba(trans->bsp, lba, opWrite);
	PDEBUG_BLOCKSTORE("OLDDBA=%lu\tNEWDBA=%lu\n", DBA(olddba),DBA(newdba));
	writeBytes=cache_blockwrite(trans->bsp, olddba, newdba, (char *)s);

	return result_OK;
}

result_t 
bl_allocLBA(bl_transaction_t trans, size_t nLBA, lba_t *pLBA)
{
	result_t ret_result;
	unsigned long long i, reqsize;
	lba_t latestlba, tmplba;
	unsigned long reqSpace, freeSpace;	

	latestlba=trans->bsp->sbp->latestlba;
	reqsize=(unsigned long long)nLBA;

	if(LBA(latestlba)==30) debug_flag=1;

	PDEBUG_BLOCKSTORE("nLBA=%llu:LATESTLBA:%llu\n",reqsize,LBA(latestlba));
//	if((LBA(latestlba)+reqsize)<LBA(latestlba))
//		ret_result=result_NoSpace;

	reqSpace=getreqspace(trans);
	freeSpace=getfreespace(trans->bsp);
	
	if(reqSpace > freeSpace)
		ret_result=result_NoSpace;

	/*Need to handle the result_NoSpace properly*/
	else	
	{
		*pLBA=latestlba;
		for(i=0; i<reqsize; i++)
		{
			LBA(tmplba)=LBA(latestlba)+i;
			addlba(trans->bsp,tmplba);
		}
		LBA(trans->bsp->sbp->latestlba)=LBA(latestlba)+reqsize;
		ret_result=result_OK;
	}

	return ret_result;
}

result_t 
bl_drop(bl_transaction_t trans, lba_t lba)
{
	dba_t dba;
	bool lbafound;

	lbafound=searchlba(trans->bsp, lba, &dba);

	if(!lbafound)
	{
		PDEBUG_BLOCKSTORE("%lu:LBA NOT FOUND\n",DBA(dba));
		return result_NotFound;
	}
	else
	{
		PDEBUG_BLOCKSTORE("%lu:LBA FOUND TO DROP HIHAWHAW\n",DBA(dba));
		deletelba(trans->bsp, lba);
		return result_OK;
	}
}

result_t 
bl_commit(bl_transaction_t trans)
{
	result_t result;
	size_t writebytes;




	uint8_t pos=0;
	unsigned long dba=62;
	lba_t lba;
	dba_t olddba;
	LBA(lba)=0;
	DBA(olddba)=0;
	
	bool lbaFound=false;
	int j;

	for(j=0; j<=50; j++)
	{
		LBA(lba)=j;
		lbaFound=searchlba(trans->bsp, lba, &olddba);
		if(lbaFound)
		{
			PDEBUG_BLOCKSTORE("FOUND:LBA=%d\tDBA=%lu\n",j,DBA(olddba));
		}
		else
		{
			DBA(olddba)=1000000;
			PDEBUG_BLOCKSTORE("NOT FOUND:LBA=%d\tDBA=%lu\n",j,DBA(olddba));
		}
	}

	PDEBUG_BLOCKSTORE("Entered\n");
	if(trans->trans_id <= trans->bsp->sbp->transaction_id)
		result=result_BadArg;
	else
	{
		pos=check_bit((uint8_t *)new_bitmap[0]->mapblock, dba);
		PDEBUG_BLOCKSTORE("DBA 62 should be free as pos is %hhu\n",pos);
		bitmap_fflush(trans->bsp, get_bitmapsize(trans->bsp->nBlocks));
		trans->bsp->sbp->transaction_id = trans->trans_id;
		PDEBUG_BLOCKSTORE("sb_addr:%lu\tlatestlba=%llu\tnBlocks=%lu\n",\
						DBA(trans->bsp->sbp->selfaddr),\
						LBA(trans->bsp->sbp->latestlba),\
						DBA(trans->bsp->sbp->nBlocks));
		writebytes=cache_blockwrite(trans->bsp, trans->bsp->sb_addr, \
				 trans->bsp->sb_addr,(char *)(trans->bsp->sbp));

		trans->bsp->sbp_old->gma_root=trans->bsp->sbp->gma_root;
		trans->bsp->sbp_old->hma_root=trans->bsp->sbp->hma_root;
		writebytes=cache_blockwrite(trans->bsp, trans->bsp->sbp_old->selfaddr,\
				 trans->bsp->sbp_old->selfaddr,(char *)(trans->bsp->sbp_old));
		cache_fflush(trans->bsp->fd);
		result=result_OK;
	}
	PDEBUG_BLOCKSTORE("Exiting\n");
	return result;
}

result_t 
bl_abort(bl_transaction_t trans)
{
	PDEBUG_BLOCKSTORE("Entered\n");
	cache_abort(trans->bsp->fd);
	bitmap_abort(trans->bsp, get_bitmapsize(trans->bsp->nBlocks));
	return result_TxAbort;
}

result_t bl_lookup_lba(bl_transaction_t trans, 
		       lba_t lba, dba_t *dba)
{
     bool lbafound;

     lbafound=searchlba(trans->bsp, lba, dba);

     if(!lbafound)
     {
          PDEBUG_BLOCKSTORE("%lu:LBA NOT FOUND\n",DBA(*dba));
          return result_NotFound;
     }
     else
     {
          PDEBUG_BLOCKSTORE("%lu:LBA FOUND HIHAWHAW\n",DBA(*dba));
          return result_OK;
     }

}

bool bl_dba_is_now_allocated(bl_transaction_t trans, 
			     dba_t dba)
{ 
	int index;
	bool isFree;

	index=DBA(dba)/(BLK_SZ*8);
	isFree=check_new_dbastatus(index, dba);

	if(isFree)
		return false;
	else
		return true;
}

bool bl_dba_was_allocated(bl_transaction_t trans, 
			  dba_t dba)
{
	int index;
	bool isFree;

	index=DBA(dba)/(BLK_SZ*8);
	isFree=check_old_dbastatus(index, dba);

	if(isFree)
		return false;
	else
		return true;
}

