#ifndef BLOCKSTORE_H
#define BLOCKSTORE_H

#include <stddef.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <libgen.h>

/*****************************************************************
 *****************************************************************
           BLOCK STORE LOW LEVEL INTERFACE
 *****************************************************************
 *****************************************************************/

/****************************************************************
   BLOCK STORE CONSTANTS
*****************************************************************/

#define BLK_SZ 1024 

#define BLK_SZ 1024 

#define POINTER_SIZE 4
#define MAX_LEVEL 10
#define HASH_BUCKET_SIZE 5

#define MOD 41
#define HASH_BUCKET_SIZE 5
#define BUCKETS_PER_BLOCK 15
#define BITS_PER_LEVEL 8
#define  EMPTY 0       

#define PER_BLOCK_DBA (BLK_SZ/sizeof(dba_t))
/****************************************************************
   BLOCK STORE TYPES
*****************************************************************/

typedef unsigned char obByte;

#ifdef PRODUCTION

typedef unsigned long long lba_t;
typedef unsigned long dba_t;

#define LBA(x) (x)
#define DBA(x) (x)

#else /* !PRODUCTION */

/* LBAs and DBAs are structs because this will prevent the compiler */
/* from allowing assignments of the form "lba = dba" or "dba = lba". */
/* You should never be doing such an assignment! */

/* LBAs are 64 bits: */
typedef struct {
  unsigned long long lba_value;
} lba_t;

/* DBAs are 32 bits */
typedef struct {
  unsigned long dba_value;
} dba_t;

/*All the global variables will be declared here as extern
 */
extern unsigned long long buckets_per_block;
extern unsigned long long total_lbas; 

extern int no_blks_for_hashmap;
extern int no_blks_for_bitmap;
extern int no_of_gma_per_blk;
extern int no_of_hma_per_blk;
extern int height_of_hmatree;
extern int height_of_gmatree;
extern int blocks_per_level[MAX_LEVEL];

/*Blockstore*/
struct BlockStore {
	struct bl_transaction *current_trans; 
	int fd;
	const char *file;
	struct HashFun *hfp;
	dba_t nBlocks;
	struct superblock *sbp;   /* Current Transaction pointer */
	struct superblock *sbp_old;
	dba_t sb_addr;
};

struct HashFun {
	int modvalue;
	int bucketsize; /*storage size requried, in terms of number of blocks*/
	int bucksize;   /*No of entries it stores*/
	int lowerlimit;
	int upperlimit;
};

/*superblock*/
struct superblock {
	dba_t hma_root;				/*This is just 32 bit DBA address:DBA*/
	dba_t gma_root;				/*This is just 32 bit DBA address:LBA*/
	unsigned long long transaction_id;
	int checksum;
	struct HashFun hashinfo;
	dba_t nBlocks;
	lba_t latestlba;
	dba_t selfaddr;
	char padding[BLK_SZ - ((4 * sizeof(dba_t)) + sizeof(int) + sizeof(lba_t)+\
			   sizeof(struct HashFun)+ sizeof(unsigned long long))];
};

/*transaction*/
struct bl_transaction {
	unsigned long long trans_id;
	int trans_status;
	struct BlockStore *bsp;
};

struct ht_node {
	dba_t d_addr[BLK_SZ];
};

#define LBA(x) (x).lba_value
#define DBA(x) (x).dba_value

#endif /* PRODUCTION */

/****************************************************************
    OPERATION RESULTS
*****************************************************************/

typedef enum {
  result_OK,             // operation completed successfully
  result_NoAccess,       // Insufficient access to underlying file-system
  result_NotFound,       // requested File / LBA not found
  result_Exists,         // Store (to be created) already exists
  result_NoSpace,        // storage is exhausted
  result_BadArg,         // bad argument value to operation
  result_UnInit,         // Trying to read a block that was not written
  result_IoError,        // IO error during any operation
  result_TxAbort,        // transaction aborted
  result_TooSmall,       // Cannot safely shrink to the specified size.
  result_BadFS           // Inconsistent File system
} result_t;

const char *result_name(result_t);

/****************************************************************
   THE MAIN INTERFACE
*****************************************************************/

typedef struct BlockStore BlockStore;

/**********************************************************************
  Create (initialize) a new block layer store in the file (logical
  volume) named by path.  Return an appropriately initialized
  BlockStore pointer and result_OK on success, or NULL and an
  appropriate result code on failure.

  This store knows about lba->dba, but it doesn't know anything at
  all about objects.

  If initialization is being done on a block device, nBlocks is
  ignored and the size is determined from the block device size.
  Otherwise a file will be created containing the requested number of
  blocks.

  ERRORS:
     if creating new file and nBlocks < 2:  result_BadArg
     if creating new file and file already exists:  result_Exists
     if disk volume and actual size < 2 blocks:  result_NoSpace
     if insufficient permissions: result_NoAccess
*************************************************************************/
BlockStore *bl_create(const char *path, dba_t nBlocks, result_t *result);

/**********************************************************************
  Open a block store, returning a BlockStore structure and returning
  it.

  ERRORS:
    File does not exist:  result_NotFound
    Both super-blocks do not checksum: result_BadFS
    Other:  result_NoAccess
*************************************************************************/
BlockStore *bl_open(const char *path, result_t *result);

/**********************************************************************
   Close a block store. Note that when a blockstore is closed, if a
   transaction is currently in progress, it  must be marked
   as aborted, and all subsequent attempts to resume that transaction   
   should fail.
  
   ERRORS:
     none
*************************************************************************/
result_t bl_close(BlockStore *);

/**********************************************************************
  Grow the blockstore by `nBlocks' blocks  
  ERRORS:
     if trying to grow a block device: result_BadArg
     if Blockstore parameter is invalid: result_BadArg
     if no space on disk:  result_NoSpace
*************************************************************************/
result_t bl_grow(BlockStore *, dba_t nBlocks);

/**********************************************************************
  Shrink the blockstore by `nBlocks' blocks -- this operation must 
       maintain file system consistency.  
  ERRORS:
     if trying to shrink a block device: result_BadArg
     if Blockstore parameter is invalid: result_BadArg
     if new-size < 2 blocks: result_BadArg
     if the size is not sufficient to hold all data 
                            and meta-data:  result_TooSmall
*************************************************************************/
result_t bl_shrink(BlockStore *, dba_t nBlocks);


/**********************************************************************
  The bl_transaction structure holds per-transaction information.
*************************************************************************/
typedef struct bl_transaction *bl_transaction_t;

/**********************************************************************
 Allocate a new per-transaction data structure and return it.

 ERRORS:
    none
*************************************************************************/
bl_transaction_t bl_BeginTransaction(BlockStore *);

/**********************************************************************
   Allocate a continuous (sequentially numbered) range of LBAs that
   can later be used with bl_write(). If no error occurs, the value
   of pLBA is updated to contain the first allocated LBA, and the
   caller has allocated LBAs in the range *pLBA .. *pLBA+nLBA-1.

  ERRORS:
    No space for matadata updates: result_NoSpace
    Already committed / aborted bad transaction: result_BadArg
 ************************************************************************/
result_t bl_allocLBA(bl_transaction_t trans, size_t nLBA, lba_t *pLBA);

/**********************************************************************
   Delete the named block. 

  ERRORS:
    Already committed / aborted bad transaction: result_BadArg
                    LBA not found: result_NotFound
    No space for matadata updates: result_NoSpace (!!!)

************************************************************************/
result_t bl_drop(bl_transaction_t trans, lba_t lba);

/**********************************************************************
   Read a block form the store. Note that all blocks are exactly the
   same size, so there is no need to specify an explicit size -- it is
   the responsibility of the application to provide a buffer of BLK_SZ
   obBytes to receive the  requested block.
  
  ERRORS:
     Already committed / aborted bad transaction: result_BadArg
                                   LBA not found: result_NotFound
               Block not previously written into: result_UnInit
 ************************************************************************/
result_t bl_read(bl_transaction_t trans, lba_t, obByte*);

/**********************************************************************
   Write a block to the store. This write must always proceed to a
   newly allocated DBA. This implies that a write to an existing block  
   may fail for lack of space. The write operation must also arrange 
   to update the mapping from LBAs to DBAs as needed to reflect the 
   new location of the block.
  
  ERRORS:
     If there are not enough disk blocks to write the 
        data and meta data to new blocks: result_NoSpace
     Already committed / aborted bad transaction: result_BadArg
                                   LBA not found: result_NotFound
 ************************************************************************/
result_t bl_write(bl_transaction_t trans, lba_t, obByte *);

/**********************************************************************
   Commit the transaction. After this operation, the current value
   of transaction_t is invalid and should no longer be used.
 
  ERRORS:
    Already committed / aborted bad transaction: result_BadArg
************************************************************************/
result_t bl_commit(bl_transaction_t trans);

/**********************************************************************
   Abort the transaction, discarding all of its operations. After 
   this operation, the current value of transaction_t is invalid 
   and should no longer be used.
  
   NOTE that the "success" result from this operation is result_TxAbort!

   ERRORS:
    Already committed / aborted bad transaction: result_BadArg
************************************************************************/
result_t bl_abort(bl_transaction_t trans);

/****************************************************************
     THE DEBUGGING INTERFACE
*****************************************************************/

/**********************************************************************
  Find the DBA corresponding to the given LBA 
  (if any such mapping exists).

  The DBA value is returned through the dba reference parameter.
  Returns result_OK on success
 
   ERRORS:
      result_NotFound if no mapping occurs for the given LBA
************************************************************************/
result_t bl_lookup_lba(bl_transaction_t trans, 
		       lba_t lba, dba_t *dba);

/**********************************************************************
  Is the block named by dba allocated in the current transaction?
 
   ERRORS:
     none
************************************************************************/
bool bl_dba_is_now_allocated(bl_transaction_t trans, 
			     dba_t dba);

/**********************************************************************
  Is the block named by dba allocated in the previous 
  (that is, now committed) transaction?

   ERRORS:
     none
************************************************************************/
bool bl_dba_was_allocated(bl_transaction_t trans, 
			  dba_t dba);

#endif /* BLOCKSTORE_H */
