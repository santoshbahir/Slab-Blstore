/*############################################################################*
 * This module will provide the ready to use slab i.e. all the metadata for   *
 * slab header objects and bufctl objects is initialized for requested slab   *
 * Also, this keeps the constructor and destructof for the metadata objects   *
 * which are - kmem_cache, slab_header and kmem_bufctl and kmem_hash.         *
 * And this is the place for some auxillary functions like object type deter- *
 * -mination,slab type calculations and bootstrap function which initializes  *
 * Three caches structures which resides outside of the slab allocator memory *
 *############################################################################*/
#ifndef SLABBUILD_H
#define SLABBUILD_H

#include "slabdefine.h"

#define FRAGMENTATION(obj, mem)              \
  (((float)((mem) - (obj)))/((float)(obj)))


/******************************************************************************
 * This function will intialize the three cache headers - cache of caches,    *
 * cache of slabs and cache of bufctls with appropriate data.                 *
 ******************************************************************************/
void __init();


/******************************************************************************
 * This function gets the slab from virtual memroy manager and allocate it to *
 * cache pointed by cache_ptr. This should properly initialize all the data   *
 * inside the slab header structure and update all the required fields in the *
 * cache_ptr fileds(mainly linked lists of slab headers)                      *
 ******************************************************************************/
void __getslab(struct kmem_cache *cache_ptr);


/******************************************************************************
 * This function will build the linked list of the bufctl objects for the     *
 * buffer of the objects to be cached(OTBC).If the OTBC are of small type,    *
 * there wont be bufctl objects but this function will still build the linked *
 * list of the free OTBC. At the end of the OTBC there would be address of the*
 * next OTBC and this address acts as the bufctl objects.                     *
 ******************************************************************************/
void __buildbufctl(struct kmem_slab *slab_ptr);


/******************************************************************************
 * This function actually gets the memory from the VM Manager. It uses mmap   *
 * system call to get memory. Essentially, there would be only one istruction *
 * which calls the mmap system call and I could have put this in __getslab.   *
 * But this is the place which is the source of all memory only opening from  *
 * where we are getting all the memory from VM. So I wanted to keep it in     *
 * different function. In rest of the functions, I am operating on the memory *
 * ultimately derived from this source. This is the source ...!!!!            *
 ******************************************************************************/
void *__get_rawslab(int slabsize);


/*******************************************************************************
 * This function release the slab pointed by slab_head and calls the destructor*
 * on each object present in the slab                                          *
 *******************************************************************************/
void freeslab(struct kmem_slab *slab_ptr);

/*******************************************************************************
 * This function release the slab of size slabsize and returns the memory to   *
 * VM-Manager.                                                                 *
 *******************************************************************************/
void	__free_rawslab(void *slab_addr, int slabsize); 

/******************************************************************************
 * These three are the constructors for the three meta-structures, namely,    *
 * cache, slab and bufctl.                                                    *
 ******************************************************************************/
void __cache_constructor(void *buf, size_t size);
void __slab_constructor(void *buf, size_t size);
void __bufctl_constructor(void *buf, size_t size); 


/******************************************************************************
 * These three are the destructors for the three meta-structures, namely,     *
 * cache, slab and bufctl.                                                    *
 ******************************************************************************/
void __cache_destructor(void *buf, size_t size);
void __slab_destructor(void *buf, size_t size);
void __bufctl_destructor(void *buf, size_t size); 


/******************************************************************************
 * This function determines the object type - SMALL, LARGE or HUGE, based upon*
 * the size of the OTBC.                                                      *
 ******************************************************************************/
int __det_objtype(size_t size);


/******************************************************************************
 * This function does the slab calculations i.e finds the size of slab in     *
 * terms of number of pages and number of objects, a slab can accomodate.     *
 * ####COPIED FROM SWAROOP'S CODE:-No copyright violations :-\ ###########    *
 ******************************************************************************/
void __slab_calc(size_t  objsize, int *nobjs, \
					int *npgs);

#endif
