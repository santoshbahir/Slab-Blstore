#ifndef INMEM_BITMAP_H
#define INMEM_BITMAP_H

#include <inttypes.h>

#include "blockstore.h"

#define DIRTY 1
#define CLEAN 0

/*******************************************************************************
 * This is the datastructure which will hold the placeholder for the each block*
 * of the in-memory copy of bitmap along with its metadata. Presently I can    *
 * think of only two piece of information.                                     *
 ******************************************************************************/
struct bitmap_block {
	bool status;  /*Either can be CLEAN or DIRTY*/
	bool dba_assigned;
	dba_t old_mapblockdba;
	dba_t new_mapblockdba;
	int mapblockaddr;
	char mapblock[BLK_SZ];
};

/******************************************************************************* 
 * This two will store the in-memory copy of the bitmap one at the start of the*
 * trasaction and the other at the end of the transactions.                    *
 ******************************************************************************/
struct bitmap_block **old_bitmap;
struct bitmap_block **new_bitmap;

/*******************************************************************************
 * This function will make two copies of the in-memory bitmap. The start_bitmap*
 * and end_bitmap will be two copies of the bitmap at the start of the transac-*
 * -tion and at the end of the transaction. When the transaction is committed, *
 * the end_bitmap should be written on the volume.                             *
 ******************************************************************************/
void init_inmembitmap(struct BlockStore *bsp, int bitmapsize,\
					int tree_height, dba_t bitmap_root);

/*******************************************************************************
 * These below two functions are interfaces to get the base pointer of the old *
 * and new inmemory copy of the bitmap.                                        *
 ******************************************************************************/
struct bitmap_block ** get_oldbitmap();
struct bitmap_block ** get_newbitmap();


/*******************************************************************************
 * This function finds out the total free space in terms of number of dbas     *
 * available on the blockstore                                                 *
 ******************************************************************************/
unsigned long getfreespace(struct BlockStore *bsp);


/*******************************************************************************
 * This function sets the bit for dba dba_addr. Seeting bit means that,this    *
 * dba is in use.                                                              *
 * INPUT : pointer to block of bitmap in which bit for dba_addr presents       *
 *         dba_addr for which bit needs to be set                              *
 ******************************************************************************/ 
void set_bit(uint8_t *block, unsigned long dba_addr);


/******************************************************************************
 * This function clears the bit for dba dba_addr. Clearing bit means that,this*
 * dba is now free.                                                           *
 * INPUT : pointer to block of bitmap in which bit for dba_addr presents      *
 *         dba_addr for which bit needs to be cleared                         *
 ******************************************************************************/ 
void clear_bit(uint8_t *block, unsigned long dba_addr);

  
/******************************************************************************
 * This function checks the staus of bit for dba_addr. The value of the       *
 * character in block whose one of the bit represets of dba_addr shows the    *
 * status of dba_addr. If it is free it returns zero else it returns non-zero *
 * value.                                                                     *
 * INPUT : pointer to block of bitmap in which bit for dba_addr presents      *
 *         dba_addr whose status need to be cheched                           *
 * OUTPUT: zero if the dba_addr is free else nonzero value less than 256      *        
 ******************************************************************************/ 
uint8_t check_bit(uint8_t *block, unsigned long dba_addr);

  
/*******************************************************************************
 * This fuction checks the both the copies of in-memory bitmap and return the  *
 * free dba. The free dba is the one for which both the copies show the 0. i.e.*
 * initially at the start of this transaction this dba was free - which is     *
 * determined by one in-memory copy of bitmap. And during this transaction and *
 * before this request this dba was free.                                      *
 * INPUT : size of bitmap in terms of no of blocks required                    *
 *         number of blocks on the blockstore                                  *
 * OUTPUT: first free DBA 		                                            *
 ******************************************************************************/
dba_t getfreedba(unsigned long bitmapsize, dba_t nBlocks);


/*******************************************************************************
 * This fuction checks the both the copies of in-memory bitmap and return the  *
 * free dba. The free dba is the one for which both the copies show the 0. i.e.*
 * initially at the start of this transaction this dba was free - which is     *
 * determined by one in-memory copy of bitmap. And during this transaction and *
 * before this request this dba was free.                                      *
 * INPUT : dba address to be set free                                          *
 ******************************************************************************/
void setfreedba(dba_t dba_addr);


/*******************************************************************************
 * This function checks the two bitmaps bits for dba dba_addr. If both are zero*
 * then this is valid free dba and actually returned to module who requested it*
 * INPUT : Index of the bitmap block(i.e. 1st bitmap blk, 7th bitmap blk, etc) *
 *         dba_addr to be checked for freeness                                 *
 * OUTPUT: status if dba_addr is free                                          *        
 ******************************************************************************/
bool validfreedba(int index, dba_t dba_addr);


/*******************************************************************************
 * This function checks if the dba_addr is allocated in this transaction.      *
 * INPUT : Index of the bitmap block(i.e. 1st bitmap blk, 7th bitmap blk, etc) *
 *         dba_addr to be checked for allocation in current transaction        *
 * OUTPUT: status of dba_addr if it is allocated in the current transaction    *        
 ******************************************************************************/
bool check_new_dbastatus(int index, dba_t dba_addr);


/*******************************************************************************
 * This function checks if the dba_addr is allocated in any of prev transaction*
 * INPUT : Index of the bitmap block(i.e. 1st bitmap blk, 7th bitmap blk, etc) *
 *         dba_addr to be checked for allocation in previous transaction       *
 * OUTPUT: status of dba_addr if it is allocated in the previous transaction   *        
 ******************************************************************************/
bool check_old_dbastatus(int index, dba_t dba_addr);

/*******************************************************************************
 * This function pushes the in-memory copy of the bitmap into block cache.     *
 * This function is called at the time of commit.                              *
 * INPUT : The size of the bitmap in terms of number of blocks                 *
 *         Pointer to blockstore                                               *
 ******************************************************************************/
void bitmap_fflush(struct BlockStore *bsp, int bitmapsize);


/*******************************************************************************
 * This function will delete all the inmem structure in case current transact- *
 * -ion is aborted.                                                            *
 * INPUT : The size of the bitmap in terms of number of blocks                 *
 *         Pointer to blockstore                                               *
 ******************************************************************************/
void bitmap_abort(struct BlockStore *bsp, int bitmapsize);
#endif
