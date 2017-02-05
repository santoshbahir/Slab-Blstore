/*
 * This File will contain the all  the data structure required by Slab Allocator.
*/

/*
 * kmem_cache is a datastructure to store the metadata of cache for the object of 'size' 
 * This objects will be initialized by function  pointed by constructor and defined by 
 * client.
*/
#ifndef SLABDEFINE_H
#define SLABDEFINE_H


#define OBJ_SMALL	0
#define OBJ_LARGE	1
#define OBJ_HUGE	2

#define SMALL_BUFCTL_SIZE 8
#define HUGE_OBJ_SIZE 9216

#define EFFECTIVE_SIZE(s, a)                      \
  ((a>0)?((((s)%(a))>0)?((s) + ((a) - ((s)%(a)))):(s)):(s))

#define MOD_NUM	16
#define FRAG_LIMIT	12.50
#define PAGESIZE	getpagesize()

#define MAX_COLOR(cp) (cp->slabsize*PAGESIZE%cp->size)

struct kmem_cache {
	char* 				name;
	size_t 				size;
	size_t 				obj_size;
	int					nobjs;	// Total objects one slab can accomodate
	int					slabsize;	// In terms of number of pages
	int					obj_type;	//SMALL-LARGE-HUGE
	int					align;
	int					prev_color;
	struct kmem_slab* freeslabs;
	struct kmem_slab* partialslabs;
	struct kmem_slab* fullslabs;
	void (*constructor)(void *,size_t);
	void (*destructor)(void *,size_t);
};

/*
 * slab is a datasturcute to store the metadata of slab of cache pointed by 'parent'
*/

struct kmem_slab {
	int				refcount;
	int				color;
	void*			slab_addr; 
	int num; /*This is temp variable for the testing of linkded list*/
	struct kmem_bufctl*	head;
	struct kmem_bufctl*	used;
	struct kmem_cache* 	parent;
	struct kmem_slab*	prev;
	struct kmem_slab*	next;
};

/*
 * kmem_bufctl is a datasturcute to store the metadata of buffer pointed by 'buf'
*/

struct kmem_bufctl {
	void* 			buf;
	struct kmem_bufctl*	next;
	int num; /*This is temp variable for the testing of linkded list*/
	struct kmem_slab*	parent;
};

/*
 *This is hash bucket element structure
 */

struct kmem_hash {
	void*			buf;
	struct kmem_bufctl*	buf_parent;
	struct kmem_hash*	next;
};


struct kmem_cache cc;
struct kmem_cache cs;
struct kmem_cache cb;

struct kmem_cache *cc_ptr;
struct kmem_cache *cs_ptr;
struct kmem_cache *cb_ptr;

#endif
