#include "phyblockrw.h"
#include "hmamap.h"
#include "mapblockaddr.h"
#include "inmem_bitmap.h"
#include "block_cache.h"

void init_inmembitmap(struct BlockStore *bsp, int bitmapsize, \
			  		int tree_height, dba_t bitmap_root)
{
	int i;
	struct bitmap_block *tmpptr_old, *tmpptr_new;
	dba_t tmp_dba;
	size_t read_bytes;
//testing.sn
//uint8_t j, status;
//testing.en

	old_bitmap = (struct bitmap_block **) 
		 	   malloc(bitmapsize * sizeof(struct bitmap_block *));

	
	new_bitmap = (struct bitmap_block **) 
			   malloc(bitmapsize *  sizeof(struct bitmap_block *));

	/* for loop equal to the size of bitmap, say bitmapsize*/
	for(i=0; i < bitmapsize; i++)
	{
		/*allocate the memory for the in-memory structure for the heap*/
		tmpptr_old = (struct bitmap_block *) 
					 malloc(sizeof(struct bitmap_block));
		tmpptr_new = (struct bitmap_block *) 
					 malloc(sizeof(struct bitmap_block));
		
		/*Get the ith block and add in the in-memory datastructure*/
		tmp_dba =	getmapblockaddr(bsp, i, tree_height, bitmap_root);
		PDEBUG_INMEMBM("BITMAP ADDR=%lu\n",DBA(tmp_dba));
		/*Initialize old bitmap block*/
		read_bytes = read_block(bsp->fd, tmpptr_old->mapblock,tmp_dba);
		tmpptr_old->status=CLEAN;
		tmpptr_old->old_mapblockdba=tmp_dba;
		tmpptr_old->dba_assigned=false;
		tmpptr_old->mapblockaddr=i;	

		/*Initialize new bitmap block*/
		read_bytes = read_block(bsp->fd, tmpptr_new->mapblock,tmp_dba);
		tmpptr_new->status=CLEAN;
		tmpptr_old->old_mapblockdba=tmp_dba;
		tmpptr_new->dba_assigned=false;
		tmpptr_new->mapblockaddr=i;	

		/*Assign the value to the pointer in the in-memory block array*/
		old_bitmap[i]=tmpptr_old;
		new_bitmap[i]=tmpptr_new;

//testing.sn			
//		for(j=0; j<=49; j++)
//		{
//			status=check_bit((uint8_t *)(new_bitmap[i]->mapblock), j);
//			PDEBUG_INMEMBM("%hhu BLOCK STATUS=%hhu\n",j,status);
//		}
//testing.en

	} 
	
	return;
}


struct bitmap_block ** get_oldbitmap()
{
	return old_bitmap;
}


struct bitmap_block ** get_newbitmap()
{
	return new_bitmap;
}


unsigned long getfreespace(struct BlockStore *bsp)
{
	unsigned long i, freeCount=0;
	bool isFree;
	int index=0;
	dba_t tmpdba;
	
	isFree=false;
	for(i=0; i<DBA(bsp->nBlocks); i++)
	{
		index=i/(BLK_SZ*8);
		DBA(tmpdba)=i;
		//isFree=check_new_dbastatus(index, tmpdba);	
		isFree=validfreedba(index, tmpdba);	

		if(isFree)	
			freeCount++;
	}
	return freeCount;
}


void set_bit(uint8_t *block, unsigned long dba_addr)
{
     block[dba_addr/8] |= 1 << (7 - (dba_addr % 8)); 
}


void clear_bit(uint8_t *block, unsigned long dba_addr)
{
     block[dba_addr/8] &= ~(1 <<(7 - (dba_addr % 8)));
}


uint8_t check_bit(uint8_t *block, unsigned long dba_addr)
{
     uint8_t pos;
     pos = (block[dba_addr/8] & (1<<(7 -(dba_addr % 8))));
     return pos;
}


dba_t getfreedba(unsigned long bitmapsize, dba_t nBlocks)
{
	int i, j,index;
	bool dbafound;
	unsigned long tmpdba;
     dba_t freedba;

     unsigned long lastblockaddrs=DBA(nBlocks)%(BLK_SZ*8);

	PDEBUG_INMEMBM("Entered\n");

     DBA(freedba)=0;
	dbafound=false;
     for(i=0; i<bitmapsize; i++)
     {
          if(i==(bitmapsize-1))
          {
               for(j=0; j<lastblockaddrs; j++)
               {
				if(validfreedba(i, freedba))
				{
					dbafound=true;
					break;
				}
				
                    DBA(freedba)=DBA(freedba)+1;
               }
          }
          else
          {
               for(j=0;j<(BLK_SZ*8); j++)
               {
				if(validfreedba(i, freedba))
				{
					dbafound=true;
					break;
				}

                    DBA(freedba)=DBA(freedba)+1;
               }
          }
	
		if(dbafound)
			break;
     }

	if(!dbafound);
		/*No free dba exists. Needs to hadle this condition*/
		
	tmpdba=DBA(freedba);
	index=DBA(freedba)/(BLK_SZ*8);
	set_bit((uint8_t *)(new_bitmap[index]->mapblock), tmpdba);
	PDEBUG_INMEMBM("Exiting:%d\tfreedba:%lu\n", index,DBA(freedba));
	return(freedba);	
}


void setfreedba(dba_t dba_addr)
{
	unsigned long tmpdba;

	PDEBUG_INMEMBM("Entered:dba_addr:%lu\n", DBA(dba_addr));

	tmpdba=DBA(dba_addr);
	int index=DBA(dba_addr)/(BLK_SZ*8);
	clear_bit((uint8_t *)(new_bitmap[index]->mapblock), tmpdba);

	PDEBUG_INMEMBM("Exiting:dba_addr:%lu\n", DBA(dba_addr));

}

bool validfreedba(int index, dba_t dba_addr)
{
     uint8_t oldstatus, newstatus;
	unsigned long tmpdba;

	PDEBUG_INMEMBM("Entered\n");

	tmpdba=DBA(dba_addr);
     oldstatus=check_bit((uint8_t *)(old_bitmap[index]->mapblock),tmpdba);
     newstatus=check_bit((uint8_t *)(new_bitmap[index]->mapblock),tmpdba);

	if(tmpdba==62)
	{
		PDEBUG_INMEMBM("dba=%lu OLD STATUS:%hhu\n", tmpdba, oldstatus);
		PDEBUG_INMEMBM("dba=%lu NEW STATUS:%hhu\n", tmpdba, newstatus);
	}

     if(oldstatus==0 && newstatus==0)
	{
		PDEBUG_INMEMBM("TRUE: dba %lu is valid free dba\n",tmpdba);
          return true;
	}
     else
	{
		PDEBUG_INMEMBM("FALSE: dba %lu is not valid free dba\n",tmpdba);
          return false;
	}

	PDEBUG_INMEMBM("Exiting\n");
}


bool check_new_dbastatus(int index, dba_t dba_addr)
{
     uint8_t newstatus;
	unsigned long tmpdba;

	PDEBUG_INMEMBM("Entered\n");

	tmpdba=DBA(dba_addr);
     newstatus=check_bit((uint8_t *)(new_bitmap[index]->mapblock),tmpdba);

     if(newstatus==0)
          return true;
     else
          return false;

	PDEBUG_INMEMBM("Exiting\n");
}


bool check_old_dbastatus(int index, dba_t dba_addr)
{
     uint8_t oldstatus;
	unsigned long tmpdba;

	PDEBUG_INMEMBM("Entered\n");

	tmpdba=DBA(dba_addr);
     oldstatus=check_bit((uint8_t *)(old_bitmap[index]->mapblock),tmpdba);

     if(oldstatus==0)
          return true;
     else
          return false;

	PDEBUG_INMEMBM("Exiting\n");
}


void bitmap_fflush(struct BlockStore *bsp, int bitmapsize)
{
	int i;
	size_t writebytes;
	dba_t olddba;
	
	DBA(olddba)=0;
	
	PDEBUG_INMEMBM("Entered\n");
	for(i=0; i<bitmapsize; i++)
	{
		if((new_bitmap[i]->status==DIRTY) && (new_bitmap[i]->dba_assigned))
		{
			PDEBUG_INMEMBM("new_bitmap[i]->old_mapblockdba=%lu\t \
						new_bitmap[i]->new_mapblockdba=%lu\n",\
						DBA(new_bitmap[i]->old_mapblockdba),\
						DBA(new_bitmap[i]->new_mapblockdba));
			writebytes=cache_blockwrite(bsp, new_bitmap[i]->old_mapblockdba,
				new_bitmap[i]->new_mapblockdba, new_bitmap[i]->mapblock);
			free(new_bitmap[i]);
			free(old_bitmap[i]);			
		}
	}		
	free(new_bitmap);
	free(old_bitmap);
	PDEBUG_INMEMBM("Exiting\n");
	return;
}


void bitmap_abort(struct BlockStore *bsp, int bitmapsize)
{
	int i;
	
	PDEBUG_INMEMBM("Entered\n");

	for(i=0; i<bitmapsize; i++)
	{
		free(new_bitmap[i]);
		free(old_bitmap[i]);			
	}		
	free(new_bitmap);
	free(old_bitmap);

	PDEBUG_INMEMBM("Exiting\n");
	return;
}
