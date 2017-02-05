 #include <stdio.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <sys/stat.h>
 #include <stdlib.h>
 #include <inttypes.h>
 #include "blockstore.h"   
 #include "inmem_bitmap.h"


/*******************************************************************************
 * This function returns the bitmapsize in terms of number of blocks           *
 * INPUT: volumesize in terms of number of blocks                              *
 ******************************************************************************/
int get_bitmapsize(dba_t volumesize);


void bitmap_init(struct BlockStore *bsp, dba_t bitmap_start_addr,dba_t nBlocks);

/*******************************************************************************
 * This function allocates dba. That is it returns the free dba to requested   *
 * module or function and updates bitmap as certain blocks of bitmap need to be*
 * updated. It works as follows:-                                              *
 * It gets the free dba from in-memory bitmap and set the bit to one for the   *
 * dba. Now the block containing this bit has to be written to fresh block, for*
 * that we need new free dba; so recursive call to this function itself is made*
 * But if the dba is allocated from block, for which dba is assigned already,  *
 * we dont need to allocate dba again for this block and this is the stopping  *
 * condition for recursive call to this function itself.                       *
 * INPUT : Pointer to BlockStore as it contains requied info about bitmap      *
 * OUTPUT: free dba                                                            *
 ******************************************************************************/ 
dba_t allocatedba(struct BlockStore *bsp);


/*******************************************************************************
 * This function deallocates the dba in bitmap. So it will trigger the updates *
 * in bitmap itself. The updating logic works exactly as it works for allocate-*
 * -dba.                                                                       *
 * INPUT : Pointer ro BlockStore as it contains required info about bitmap     *
 *         dba to be deallocated                                               *
 ******************************************************************************/ 
void deallocatedba(struct BlockStore *bsp, dba_t dba_addr);
