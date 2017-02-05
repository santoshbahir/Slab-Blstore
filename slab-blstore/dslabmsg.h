#include<stdio.h>

/*LIST.H*/
//#define DEBUG_MSG_LIST

#ifdef DEBUG_MSG_LIST
#define PDEBUG_LIST(fmt, args...) printf("(%s:%d) " fmt, __func__, __LINE__, ## args)
#else
#define PDEBUG_LIST(fmt, args...)
#endif

/*SLABBUILD.H*/
//#define DEBUG_MSG_SLABBUILD

#ifdef DEBUG_MSG_SLABBUILD
#define PDEBUG_SLABBUILD(fmt, args...) printf("(%s:%d) " fmt, __func__, __LINE__, ## args)
#define PRINTA_SLABBUILD(addr) printf("(%s:%d)(addr.lba=%llu:addr.index=%hu)\n", __func__, __LINE__,LBA(addr.lba),addr.index)
#else
#define PDEBUG_SLABBUILD(fmt, args...)
#define PRINTA_SLABBUILD(addr)
#endif

/*SLABOBJ.H*/
//#define DEBUG_MSG_SLABOBJ

#ifdef DEBUG_MSG_SLABOBJ
#define PDEBUG_SLABOBJ(fmt, args...) printf("(%s:%d) " fmt, __func__, __LINE__, ## args)
#else
#define PDEBUG_SLABOBJ(fmt, args...)
#endif


/*SLAB.H*/
//#define DEBUG_MSG_SLAB

#ifdef DEBUG_MSG_SLAB
#define PDEBUG_SLAB(fmt, args...) printf("(%s:%d) " fmt, __func__, __LINE__, ## args)
#else
#define PDEBUG_SLAB(fmt, args...)
#endif


