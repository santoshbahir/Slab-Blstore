#ifndef SLABOBJ_H 
#define SLABOBJ_H 

#include "slabdefine.h"
/*#############################################################################
 * This is the module wherein all the operation where actual objects are added#
 * and removed from the slab.                                                 #
 *############################################################################*/

/******************************************************************************
 * This functions search the slab in the freeslabs list and partialslabs list;*
 * if found free buffer on the partialslabs list and this allocation makes    *
 * this partialslab full, this function moves this slab to fullslabs list.    *
 * Similarly it moves the empty slab to partial slab list.                    *
 * It also removes the bufctl from freelist and allocates it to the used list *
 ******************************************************************************/
void *
kslab_alloc(struct kmem_cache *cp);


/******************************************************************************
 * This functions search the buffer in the partialslabs list and fullslab list*
 * and if found it moves the slab in appropriate list. Similarly it moves the *
 * the bufctl in the appropriate list.                                        *
 ******************************************************************************/
void 
kslab_free(struct kmem_cache *cp, void *buf);

#endif
