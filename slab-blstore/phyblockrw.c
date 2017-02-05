#include <stdio.h>
#include "phyblockrw.h"
#include "blockstore.h"

int no_blks_for_hashmap;
int no_blks_for_bitmap;
int no_of_gma_per_blk;
int no_of_hma_per_blk;
int blocks_per_level[MAX_LEVEL];

void cal_global_parameter(dba_t nBlocks)
{
	int div, rem;

	div = MOD / BUCKETS_PER_BLOCK;
	rem = MOD % BUCKETS_PER_BLOCK;

	no_blks_for_hashmap = (rem == 0) ? div : (div + 1);


	div = DBA(nBlocks) / (BLK_SZ*8);
	rem = DBA(nBlocks) % (BLK_SZ*8);

	no_blks_for_bitmap = (rem == 0) ? div : (div + 1);

	no_of_hma_per_blk = BLK_SZ / sizeof(dba_t);
	/*DBAs are stored in the block as a address of GMAMAP tree nodes*/
	no_of_gma_per_blk = BLK_SZ / sizeof(dba_t); 
}

size_t read_block(int fd, char *buffer, dba_t dba_addr)
{
	off_t offset;
	size_t read_bytes;

	if((offset = lseek(fd,DBA(dba_addr)*BLK_SZ, SEEK_SET)) < 0)
	{
		PDEBUG_BLOCKRW("Error in going to start of the requested block\n");
	}
	else
   	{
		if((read_bytes=read(fd,buffer,BLK_SZ)) < BLK_SZ)
      	{
	      		PDEBUG_BLOCKRW("Error in reading a block completely:%d\n",read_bytes);
		}
	}		
	return(read_bytes);
}

size_t write_block(int fd, char *buffer, dba_t dba_addr)
{
	off_t offset;
	int write_bytes;

	if((offset = lseek(fd,DBA(dba_addr)*BLK_SZ, SEEK_SET)) < 0)
   	{
		PDEBUG_BLOCKRW("Error in going to start of the requested block\n");
	}
	else
	{
		if((write_bytes=write(fd,buffer,BLK_SZ)) < BLK_SZ)
		{	
      		PDEBUG_BLOCKRW("Error in writing a block completely\n");
		}
	}		
	return(write_bytes);
}

void test_read(int fd)
{
   char block[1024];
   PDEBUG_BLOCKRW("\n************JANAM DEKH LO..!! Beginning of local testing");   
   off_t offset;
 //  uint8_t status;
   size_t read_bytes;
   
      if((offset = lseek(fd,8*BLK_SZ, SEEK_SET)) < 0)
      {
         PDEBUG_BLOCKRW("Error in going to start of the bitmap\n"); 
      }
      else
      {
         if((read_bytes=read(fd,block,BLK_SZ)) < BLK_SZ)
         {
            PDEBUG_BLOCKRW("Error in reading a block completely\n");
         }
      }
}

/*
int main()
{
	dba_t tmp;
	DBA(tmp) = 1024*8+1;
	cal_global_parameter(tmp);
	PDEBUG("%d\n",no_of_hma_per_blk);
	PDEBUG("%d\n",no_of_gma_per_blk);
	PDEBUG("%d\n",no_blks_for_hashmap );
	PDEBUG("%d\n",no_blks_for_bitmap );
	return 0;
}*/
