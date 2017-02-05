#ifndef SLABOBJ_H 
#define SLABOBJ_H 

#include "dslabdefine.h"
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
 * SMALL OBJECTS                                                              *
 ******************************************************************************/
vptr dslab_alloc_os(vptr_cache vcp);


/******************************************************************************
 * This functions does the same functionality as above function but for large *
 * objects. LARGE OBJECTS                                                     *
 ******************************************************************************/
vptr dslab_alloc_ol(vptr_cache vcp);


/******************************************************************************
 * This functions does the same functionality as above function but for huge  *
 * objects. HUGE  OBJECTS                                                     *
 ******************************************************************************/
vptr dslab_alloc_oh(vptr_cache vcp);


/******************************************************************************
 * This functions search the buffer in the partialslabs list and fullslab list*
 * and if found it moves the slab in appropriate list. Similarly it moves the *
 * the bufctl in the appropriate list. SMALL OBJECTS                          *
 ******************************************************************************/
void dslab_free_os(vptr_cache vcp, vptr buf);


/******************************************************************************
 * This functions does the same functionality as above function but for large *
 * objects: LARGE OBJECTS                                                     *
 ******************************************************************************/
void dslab_free_ol(vptr_cache vcp, vptr buf);


/******************************************************************************
 * This functions does the same functionality as above function but for large *
 * objects: HUGE OBJECTS                                                      *
 ******************************************************************************/
void dslab_free_oh(vptr_cache vcp, vptr buf);

#endif
