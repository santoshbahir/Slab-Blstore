#ifndef SLAB_H
#define SLAB_H

/****************************************************************
    EN 600.318/418 Operating Systems
    Slab Allocator Project.

    This header is a modified version of the interface written by 
    Prof. Jonathan S Shapiro.

 ***************************************************************/

/****************************************************************
   General Notes: 

   1) This is a modified version of the Jeff Bonwik's slab allocator. 
   2) You must provide an implementation for all of the following
      functions. 
   3) All invalid arguments must result in an assert() failure
      (not a segmentation fault).
   4) If any PRECONDITIONS fail, it must result in an assert() failure
      and thus terminate the program.
 ***************************************************************/

#include <unistd.h>


/****************************************************************
   Main allocator interface
 ***************************************************************/

/* Create a cache that produces objects of the specified size and
   alignment, that will be constructed and destructed using the
   specified constructor and destructor. */
struct kmem_cache *
kmem_cache_create(char *name,
		  size_t size,
		  int align,
		  void (*constructor)(void *, size_t),
		  void (*destructor)(void *, size_t));

/* Destroy an entire cache, including all of its slabs.
   PRECONDITION: The cache referred to by `cp' has no slabs containing
                 active objects. */ 
void kmem_cache_destroy(struct kmem_cache *cp);

/* Allocate an object from a previously allocated cache. Allocates a
   new underlying slab if needed. Note that this does NOT take a flags
   argument, which is different from the paper! */
void *kmem_cache_alloc(struct kmem_cache *cp);

/* Deallocate an object, returning its storage to the cache.
   PRECONDITION: Object `buf' must belong to cache `cp'. */
void kmem_cache_free(struct kmem_cache *cp, void *buf);

/* Free all empty slabs in the cache `cp', and return the allocated
   pages to the main backend store. This function returns the number
   of pages freed.

   This is a backend ``interface'' to trigger garbage collection. */
size_t kmem_cache_reap(struct kmem_cache *cp);


/*********************************************************************
 * Debugging interface
 *********************************************************************/

/* Return the number of slabs in a cache */
size_t 
debug_get_nslabs(struct kmem_cache *cp);

/* Return a pointer to the i'th slab associated with the given cache,
   where i=0 gives the first such slab. 

   If there is no i'th slab, you _must_ return a NULL pointer 
   (not an assert() failure). 
   
   The pointer to the slab_header should remain
   valid until the next call that frees a slab or a cache. */
struct kmem_slab *
debug_get_slab(struct kmem_cache *cp, size_t i);

struct slab_query {
  /* color: the offset IN BYTES from the start of the slab at which
     the first object appears */
  size_t color;
  size_t obj_size;			/* size of objects in this slab */
  size_t align;			/* alignment of objects in this slab */
  unsigned nFree;		/* number of free objects in this slab*/
  unsigned nAlloc;		/* number of allocated objects in this
				   slab*/
};

/* Given a slab pointer (previously returned by debug_get_slab()) and
   a pointer to its containing cache, return the information in the
   slab_query structure defined above. */
struct slab_query
debug_get_slab_info(struct kmem_cache *cp, struct kmem_slab *slab);

/* Return a pointer to the cache header for the cache of cache headers */
struct kmem_cache *debug_get_cacheheader_cache();

/* Return a pointer to the cache header for the bufctl cache, if such
   a cache currently exists, or NULL if it does not: */
struct kmem_cache *debug_get_bufctl_cache();

#endif /* SLAB_H */
