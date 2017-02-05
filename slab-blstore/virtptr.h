/*This is the place where the virtual pointer resides*/
#ifndef VIRTPTR_H
#define VIRTPTR_H

#include <inttypes.h>
#include "blockstore.h"
//#include "slabdefine.h"

/*	CASTING	*/

/*b is casted to a's type and a's type should be VPTR(void..:-\)*/
#define	CAST_VPTR(a,b)	a.lba=b.lba;	\
a.index=b.index;

/*b is casted to a's type and a's type should be CACHE*/
#define	CAST_CACHE(a,b)	a.lba=b.lba;	\
a.index=b.index;

/*b is casted to a's type and a's type should be SLAB*/
#define	CAST_SLAB(a,b)		a.lba=b.lba;	\
a.index=b.index;

/*b is casted to a's type and a's type should be BUFCTL*/
#define	CAST_BUFCTL(a,b)	a.lba=b.lba;	\
a.index=b.index;

/*if custom pointer a and b are equals */
#define	EQUAL(a,b)	(LBA(a.lba) == LBA(b.lba)) && (a.index == b.index)
/*if custom pointer a and b are not equals */
#define	NOTEQUAL(a,b)	(LBA(a.lba) != LBA(b.lba)) || (a.index != b.index)

struct virtual_pointer{
	lba_t	lba;
	uint16_t	index;
};

struct pointer_cacheobj{
	lba_t	lba;
	uint16_t	index;
};

struct pointer_slabobj{
	lba_t	lba;
	uint16_t	index;
};

struct pointer_bufctlobj{
	lba_t	lba;
	uint16_t	index;
};

typedef struct virtual_pointer	vptr;
typedef struct pointer_cacheobj	vptr_cache;
typedef struct pointer_slabobj	vptr_slab;
typedef struct pointer_bufctlobj	vptr_bufctl;
typedef struct pointer_bufctlobj	vptr_hash;	

vptr vnull;
vptr_cache vcnull;
vptr_slab vsnull;
vptr_bufctl vbnull;


void null_init(void);
void inc(vptr *addr, unsigned long long nplaces);
void inc_pc(vptr_cache *addr, unsigned long long nplaces);
void inc_ps(vptr_slab *addr, unsigned long long nplaces);
void inc_pb(vptr_bufctl *addr, unsigned long long nplaces);

#endif 
