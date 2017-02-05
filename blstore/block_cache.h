#include "blockstore.h"


#define CLEAN 0	/* BLOCK Copy on the VOLUME and cache is same; */
				/* retrieved block from VOLUME for read purpose */
#define DIRTY 1     /* Copy on the VOLUME and cache differs; */

struct cache_buffer{
	dba_t block_addr;
	bool status;	          /*Colud be CLEAN or DIRTY*/
	char block[BLK_SZ];
	struct cache_buffer *prev;
	struct cache_buffer *next;
};


/*******************************************************************************
 * For every transaction, we initiate the cache.                               *
 * Here we will be creatin cache_header without any element in the cache       *
 *********************************************************************** ******/
void cache_init();


/*******************************************************************************
 * This checks if cache is empty; if yes returns true else return false        *
 *******************************************************************************/
bool cache_empty(struct cache_buffer *cache_header);

/*******************************************************************************
 * This is an internal function to this module.                                *
 * It avoids a procedure to allocate a memory from heap.                       *
 * It allocate a memory of size 'cache buffer' and return the pointer to the   *
 * cache buffer.                                                               *
 *********************************************************************** ******/
struct cache_buffer *alloc_cache_buffer();


/*******************************************************************************
 * This will check if the dba address is present in the cache                  *
 * This is and internal function to this module. I will use it to find a block *
 * to be read/write is present in the cache.                                   *
 * If present returns :                                                        *
 *                    - pointer to found cache_buffer                          *
 *                    - NULL                                                   *
 ******************************************************************************/
struct cache_buffer *cache_lookup(dba_t address, bool *status);


/*******************************************************************************
 * This will put the cache_buffer buffer in the cache(cache linked list)       *
 * This is an internal function to this module.                                *
 ******************************************************************************/
void cache_blockadd(struct cache_buffer *buffer);


/*******************************************************************************
 * This function removes the block for dba_addr from cache as this has been    *
 * freed and hence should not be read by any function.                         *
 ******************************************************************************/
void cache_blockdelete(dba_t dba_addr);


/*******************************************************************************
 * This will check if the dba_t address is present in the cache if present copy*
 * the content from the cache into block buffer.                               *
 * If the block is not present it will read the physical block from the VOLUME,*
 * put the read block into cache and copy the content of this buffer into block*
 * buffer.                                                                     *
 * Return values :                                                             *
 *               - blocksize if successful                                     *
 *               - -1 if some failure occurs                                   *
 ******************************************************************************/
size_t cache_blockread(struct BlockStore *bsp, dba_t address, char *block);


/*******************************************************************************
 * This will check if the dba_t address is present in the cache if present;    *
 * overwrite the content of buffer block corresponding cache buffer.           *
 * If the block is not present it will put this block into the cache (add a    *
 * node to the link list)                                                      *
 * Return values :                                                             *
 *               - blocksize if successful                                     *
 *               - -1 if some failure occurs                                   *
 ******************************************************************************/
size_t cache_blockwrite(struct BlockStore *bsp, dba_t oldaddr,\
					dba_t newaddr, char *block);


/*******************************************************************************
 * This will put all the blocks present in the cache on to VOLUME and free the *
 * whole cache(No element will be on the linked list. cache_header will be     *
 * pointing to NULL                                                            *
 ******************************************************************************/
void cache_fflush(int fd);


/*******************************************************************************
 * This will free all the memory occupied by cache data-structure in case the  *
 * transaction is aborted.                                                     *
 ******************************************************************************/
void cache_abort(int fd);
