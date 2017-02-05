#include<stdio.h>

int debug_flag;
/*PHYBLOCKRW.H*/
//#define DEBUG_MSG_BLOCKRW

#ifdef DEBUG_MSG_BLOCKRW
#define PDEBUG_BLOCKRW(fmt, args...) printf("(%s:%d) " fmt, __func__, __LINE__, ## args)
#else
#define PDEBUG_BLOCKRW(fmt, args...)
#endif

/*BLOCK_CACHE.H*/
//#define DEBUG_MSG_BLOCKCACHE

#ifdef DEBUG_MSG_BLOCKCACHE
#define PDEBUG_BLOCKCACHE(fmt, args...) printf("(%s:%d) " fmt, __func__, __LINE__, ## args)
#else
#define PDEBUG_BLOCKCACHE(fmt, args...)
#endif


/*MAPBLOCKADDR.H*/
//#define DEBUG_MSG_MAPBLOCKADDR

#ifdef DEBUG_MSG_MAPBLOCKADDR
#define PDEBUG_MAPBLOCKADDR(fmt, args...) printf("(%s:%d) " fmt, __func__, __LINE__, ## args)
#else
#define PDEBUG_MAPBLOCKADDR(fmt, args...)
#endif

/*HMAMAP.H*/
//#define DEBUG_MSG_HMAMAP

#ifdef DEBUG_MSG_HMAMAP
#define PDEBUG_HMAMAP(fmt, args...) printf("(%s:%d) " fmt, __func__, __LINE__, ## args)
#else
#define PDEBUG_HMAMAP(fmt, args...)
#endif

/*BITMAP.H*/
//#define DEBUG_MSG_BITMAP 

#ifdef DEBUG_MSG_BITMAP
#define PDEBUG_BITMAP(fmt, args...) printf("(%s:%d) " fmt, __func__, __LINE__, ## args)
#else
#define PDEBUG_BITMAP(fmt, args...)
#endif

/*INMEM_BITMAP.H*/
//#define DEBUG_MSG_INMEMBM 

#ifdef DEBUG_MSG_INMEMBM
#define PDEBUG_INMEMBM(fmt, args...) printf("(%s:%d) " fmt, __func__, __LINE__, ## args)
#else
#define PDEBUG_INMEMBM(fmt, args...)
#endif

/*GMAMAP.H*/
//#define DEBUG_MSG_GMAMAP

#ifdef DEBUG_MSG_GMAMAP
#define PDEBUG_GMAMAP(fmt, args...) printf("(%s:%d) " fmt, __func__, __LINE__, ## args)
#else
#define PDEBUG_GMAMAP(fmt, args...)
#endif


/*HASHMAP.H*/
//#define DEBUG_MSG_HASH

#ifdef DEBUG_MSG_HASH
#define PDEBUG_HASH(fmt, args...) printf("(%s:%d) " fmt, __func__, __LINE__, ## args)
#else
#define PDEBUG_HASH(fmt, args...)
#endif

/*BLOCKSTORE.H*/
//#define DEBUG_MSG_BLOCKSTORE

#ifdef DEBUG_MSG_BLOCKSTORE
#define PDEBUG_BLOCKSTORE(fmt, args...) printf("(%s:%d) " fmt, __func__, __LINE__, ## args)
#else
#define PDEBUG_BLOCKSTORE(fmt, args...)
#endif

