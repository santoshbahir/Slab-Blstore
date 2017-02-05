/*
 * This File will contain the all  the data structure required by Slab Allocator.
*/

/*
 * kmem_cache is a datastructure to store the metadata of cache for the object of 'size' 
 * This objects will be initialized by function  pointed by constructor and defined by 
 * client.
*/
#ifndef DSLABDEFINE_H
#define DSLABDEFINE_H

#include "blockstore.h"
#include "virtptr.h"

#define OBJ_SMALL	0
#define OBJ_LARGE	1
#define OBJ_HUGE	2

#define SMALL_BUFCTL_SIZE sizeof(vptr)
#define HUGE_OBJ_SIZE 9216

#define CACHE_LEN	sizeof(struct dmem_cache)
#define SLAB_LEN	sizeof(struct dmem_slab)
#define BUFCTL_LEN	sizeof(struct dmem_bufctl)

#define EFFECTIVE_SIZE(s, a)                      \
  ((a>0)?((((s)%(a))>0)?((s) + ((a) - ((s)%(a)))):(s)):(s))

#define MOD_NUM	16
#define FRAG_LIMIT	12.50
//#define PAGESIZE	getpagesize()
#define PAGESIZE	BLK_SZ

#define MAX_COLOR(cp) (cp->slabsize*PAGESIZE%cp->size)

struct dmem_cache {
	char 				name[8];
	size_t 				size;
	size_t 				obj_size;
	int					nobjs;	// Total objects one slab can accomodate
	int					slabsize;	// In terms of number of pages
	int					obj_type;	//SMALL-LARGE-HUGE
	int					align;
	int					prev_color;
	vptr_slab freeslabs;
	vptr_slab partialslabs;
	vptr_slab fullslabs;
	void (*constructor)(void *,size_t);
	void (*destructor)(void *,size_t);
};

/*
 * slab is a datasturcute to store the metadata of slab of cache pointed by 'parent'
*/

struct dmem_slab {
	int				refcount;
	int				color;
	vptr				slab_addr; 
	int num; /*This is temp variable for the testing of linkded list*/
	vptr_bufctl 	head;
	vptr_bufctl 	used;
	vptr_cache  	parent;
	vptr_slab 	prev;
     vptr_slab 	next;
};

/*
 * kmem_bufctl is a datasturcute to store the metadata of buffer pointed by 'buf'
*/

struct dmem_bufctl {
	vptr 		buf;
	int num; /*This is temp variable for the testing of linkded list*/
	vptr_bufctl	next;
	vptr_slab		parent;
};
/*
 *This is hash bucket element structure
 */

struct dptr_hash {
	vptr			buf;
	vptr_bufctl	buf_parent;
	vptr_hash		next;
};

vptr_cache cc_vptr;
vptr_cache cs_vptr;
vptr_cache cb_vptr;

unsigned char firstlba[BLK_SZ];

#endif
