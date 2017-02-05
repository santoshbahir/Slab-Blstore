#ifndef DEBUGMSG_H
#define DEBUGMSG_H

/*New Line Printing*/
//#define PRINT_NEWLINE

#ifdef PRINT_NEWLINE
#define PRINTNL printf("\n")
#else
#define PRINTNL
#endif


/*BLLBARW.H*/
//#define DEBUG_MSG_BLLBARW

#ifdef DEBUG_MSG_BLLBARW
#define PDEBUG_BLLBARW(fmt, args...) printf("(%s:%d) " fmt, __func__, __LINE__, ## args)
#define PRINTA_BLLBARW(addr) printf("(addr.lba=%llu:addr.index=%hu)\n",LBA(addr.lba),addr.index)
#else
#define PDEBUG_BLLBARW(fmt, args...)
#define PRINTA_BLLBARW(addr)
#endif

/*VIRTPTR.H*/
//#define DEBUG_MSG_VIRTPTR

#ifdef DEBUG_MSG_VIRTPTR
#define PDEBUG_VIRTPTR(fmt, args...) printf("(%s:%d) " fmt, __func__, __LINE__, ## args)
#define PRINTA_VIRTPTR(addr) printf("(addr.lba=%llu:addr.index=%hu)\n",LBA(addr.lba),addr.index)
#else
#define PDEBUG_VIRTPTR(fmt, args...)
#define PRINTA_VIRTPTR(addr)
#endif

/*SLABBUILD.H*/
//#define DEBUG_MSG_DSLABBUILD

#ifdef DEBUG_MSG_DSLABBUILD
#define PDEBUG_DSLABBUILD(fmt, args...) printf("(%s:%d) " fmt, __func__, __LINE__, ## args)
#define PRINTA_DSLABBUILD(addr) printf("(addr.lba=%llu:addr.index=%hu)\n",LBA(addr.lba),addr.index)
#else
#define PDEBUG_DSLABBUILD(fmt, args...)
#define PRINTA_DSLABBUILD(addr)
#endif

/*DLIST.H*/
//#define DEBUG_DLIST

#ifdef DEBUG_DLIST
#define PDEBUG_DLIST(fmt, args...) printf("(%s:%d) " fmt, __func__, __LINE__, ## args)
#define PRINTA_DLIST(addr) printf("(addr.lba=%llu:addr.index=%hu)\n",LBA(addr.lba),addr.index)
#else
#define PDEBUG_DLIST(fmt, args...)
#define PRINTA_DLIST(addr)
#endif


/*DSLABOBJ.H*/
//#define DEBUG_DSLABOBJ

#ifdef DEBUG_DSLABOBJ
#define PDEBUG_DSLABOBJ(fmt, args...) printf("(%s:%d) " fmt, __func__, __LINE__, ## args)
#define PRINTA_DSLABOBJ(addr) printf("(addr.lba=%llu:addr.index=%hu)\n",LBA(addr.lba),addr.index)
#else
#define PDEBUG_DSLABOBJ(fmt, args...)
#define PRINTA_DSLABOBJ(addr)
#endif


/*DSLAB.H*/
//#define DEBUG_DSLAB

#ifdef DEBUG_DSLAB
#define PDEBUG_DSLAB(fmt, args...) printf("(%s:%d) " fmt, __func__, __LINE__, ## args)
#define PRINTA_DSLAB(addr) printf("(addr.lba=%llu:addr.index=%hu)\n",LBA(addr.lba),addr.index)
#else
#define PDEBUG_DSLAB(fmt, args...)
#define PRINTA_DSLAB(addr)
#endif


#endif
