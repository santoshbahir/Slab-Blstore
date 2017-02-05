#include <stdio.h>
#include <sys/user.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <slab.h>
#include <slabdefine.h>

/**************************************************************************
  General Notes: This is the slab allocator test program
   1) Object Definitions must be written in objects.def
   2) Testing can be customized using switches defined below

  This program was written by Swaroop -- a mere mortal -- and is therefore
  not immaculate. However, since it is far simpler than the slab allocator
  itself, it is also less error prone. Therefore, you should try to
  fix your code before suspecting the test program. If you are fairly
  certain about a problem in the test program, please file a
  bug-report to cs418instructors@cs.jhu.edu 
***************************************************************************/


/**********************************************************************
           Direction Adjustments
**********************************************************************/

/* Enable the switch   COUNT_FROM_LEFT   if: In the 
   debug_get_slab_info(c, i) function, you implemented 'i' as ith slab 
   from the left.(rather than from the right).

   NOTE: DO NOT CHANGE THE TEST PROGRAM. Instead, pass this argument from
   the Makefile (-D COUNT_FROM_LEFT) */

/**********************************************************************
           Test Switches
**********************************************************************/

#define LOOK_AT_BUFCTL_SLAB
#define FILL
#define FULL_TEST
#define ST_TEST

/**********************************************************************
           Auxiliary Definitions
**********************************************************************/

#define CONS_VAL 'c'
#define FILL_VAL 'f'
#define DCONS_VAL 'd'

#define ULONG(x) ((unsigned long) (x))

typedef void* cache;
typedef void* obj;
typedef void* slab;


/**********************************************************************
              Calculations
**********************************************************************/

#define EFFECTIVE_SIZE(s, a)					\
  ((a>0)?((((s)%(a))>0)?((s) + ((a) - ((s)%(a)))):(s)):(s))

/* Fragmentation as a fraction of used space */
#define FRAGMENTATION(obj, mem)			\
  (((float)((mem) - (obj)))/((float)(obj)))


/**********************************************************************
              CAREFUL: Globals
**********************************************************************/

static bool constructor_was_called, destructor_was_called;
static size_t small_bufctl_size = 8;

/* Unfortunately, some people passed the wrong size argument
   to constructors / destructors. So, I had to build in my own 
   communication using these globals */
static bool check_size = false;
static bool alreadyPanicked = false;
static size_t Osize; /* Object size */


/**********************************************************************
           Constructor / Destructor for ALL objects
**********************************************************************/

static void
grand_constructor(void *const v, const size_t n)
{
  if(check_size && (Osize != n) && !alreadyPanicked) {
    printf("PANIC: Wrong size passed to Constructor. ");
    printf("Expected %lu, got %lu\n", ULONG(Osize), ULONG(n));
    alreadyPanicked = true;
  }
	
  constructor_was_called = true;
#ifdef FILL
  size_t i;
  unsigned char *const p = (unsigned char *const) v;
  /* This loop really wants to be 0 - n */
  for(i=0; i<Osize; i++) 
    p[i] = CONS_VAL;
#endif
}

static void
grand_destructor(void *const v, const size_t n)
{
  if(check_size && (Osize != n) && !alreadyPanicked) {
    printf("PANIC: Wrong size passed to Destructor. ");
    printf("Expected  %lu, got %ld\n", ULONG(Osize), ULONG(n));
    alreadyPanicked = true;
  }
  destructor_was_called = true;
#ifdef FILL
  size_t i;
  unsigned char *const p = (unsigned char *const) v;
  /* This loop really wants to be 0 - n */
  for(i=0; i<Osize; i++) 
    p[i] = DCONS_VAL;
#endif
}

/**********************************************************************
           Checking well-formedness of Objects
**********************************************************************/

static bool
isProper(const void *const v, const size_t n) {
#ifdef FILL
  unsigned char *const p = (unsigned char *const) v;
  size_t i;
  for(i=0; i<n; i++) 
    if(p[i] != CONS_VAL)
      return false;
#endif
  
  return true;
} 

/**********************************************************************
           Expected Size Calculations
**********************************************************************/
static void
slab_calc(const size_t  objsize, 
	  unsigned long *const nobjs, unsigned long *const npgs) 
{
  unsigned long npages=1, nobjects=1;
  float total_obj_size=0, memsize=0, perc;
  while(1) {
    do {	
      total_obj_size = (nobjects * objsize);
      memsize = (npages * PAGE_SIZE);
      if(memsize < total_obj_size)
	npages++;
    } while(memsize < total_obj_size);
    perc = (memsize - total_obj_size) / memsize;
    perc = FRAGMENTATION(total_obj_size, memsize);
    if(perc < 0.125)
      break;
    nobjects++;
  }

  *nobjs = nobjects;
  *npgs = npages;
}

/**********************************************************************
              Printing Slab Statistics
**********************************************************************/

static void
slab_stats(const cache c, const slab sl)
{
  printf("Slab Statistics:\n");
  const struct slab_query s = debug_get_slab_info(c, sl);

  const unsigned long nobjects = s.nFree + s.nAlloc;
  size_t bufctl_overhead = 0;

  bool isSmallObject = false;

  /* This inference is not perfect, due to alignment, 
     bufctl space for small objects, etc. FIX if needed. */  
  if(s.obj_size < PAGE_SIZE/8) {
    bufctl_overhead = small_bufctl_size;
    isSmallObject = true;
    printf("\tSlab-kind (inferred) = SMALL\n");
  }
  else {
    bufctl_overhead = 0;
    isSmallObject = false;
    printf("\tSlab-kind (inferred) = LARGE\n");
  }
  const size_t tot_size = (nobjects * 
			   EFFECTIVE_SIZE((s.obj_size + bufctl_overhead),
					  s.align));
  
  /* The small_bufctl_size should be used for calculating
     fragmentation of small objects, where bufctls count as payload */
  const size_t tot_mem = EFFECTIVE_SIZE(tot_size, PAGE_SIZE);
  const unsigned long npages = tot_mem / PAGE_SIZE;
  const float frag = FRAGMENTATION(tot_size, tot_mem);

  /* My expectation of the Objects xin this slab */
  unsigned long expobjs, exppgs;
  slab_calc(EFFECTIVE_SIZE(s.obj_size + bufctl_overhead, s.align), &expobjs, &exppgs); 
  printf("\tColor      = %lu\n",  ULONG(s.color));
  printf("\tSize       = %lu\n",  ULONG(s.obj_size));
  printf("\tAlign      = %lu\n",  ULONG(s.align));
  printf("\t#Free      = %d\n",   s.nFree);
  printf("\t#Allocated = %d\n\n", s.nAlloc);

  printf("\tTotal Objects (calculated)    = %ld\n", nobjects);
  printf("\tMin No. Objects I expected    = %ld\n", expobjs);
  printf("\tNo. Pages     (calculated)    = %ld\n", npages);
  printf("\tNo. Pages I expected          = %ld\n", exppgs);
  printf("\tTotal Payload (calculated)    = %lu\n", ULONG(tot_size));
  printf("\tTotal memory (allegedly) used = %lu\n", ULONG(tot_mem));
  printf("\tFRAGMENTATION (as calculated using above data) = %f\n", frag);
    
  if(frag > 0.125)
    printf("\tPANIC: *** FRAGMENTATION > 12.5%% ***\n");
  
  if(!isSmallObject && (nobjects > expobjs) && (npages > exppgs))
    printf("\tWARNING: Slab may contain more than necessary objects\n");

  if(nobjects < expobjs)
    printf("\tWARNING: This slab has LESS Objects than expected\n");

  if((s.align != 0) &&((s.color % s.align) != 0))
    printf("PANIC: Coloring Voilates Alignment\n");
}

/**********************************************************************
              Detailed Testing
**********************************************************************/

static obj
create_first_obj(const cache c, const size_t s, const size_t a, 
		 const char *const nm) 
{
  constructor_was_called = false;
  const obj o = kmem_cache_alloc(c);
  if(!constructor_was_called)
    printf("ATTN: %s: constructor *NOT* called when FIRST obj was created\n", nm);
  if(!isProper(o, s))
    printf("WARNING: %s: Improper Construction\n", nm);
  if((unsigned long)o % (unsigned long)a)
    printf("PANIC: Alignment Violation addr = %lx, align = %lu\n", 
	   ULONG(o), ULONG(a));

  return o;
}

static void
test(const cache c, const char *const cname) 
{
  printf("Testing slabs of cache %s:\n", cname);
  printf("As of now only one object should exist:\n");
  slab slb, slb1;
  struct slab_query s, s1;

  slb = debug_get_slab(c, 0);

  if(slb == NULL) {
    printf("PANIC: no slabs\n");
    return;
  }

  if(debug_get_slab(c, 1) != NULL)
    printf("PANIC: 2nd slab exists when only 1 object is created");

  /* Print out slab Statistics including Fragmentation */
  s = debug_get_slab_info(c, slb);
  const size_t toAlloc = s.nFree; 
  obj *const oo = (obj) malloc(sizeof(obj) * (toAlloc + 4));
  int i=0;

  for(i=0; i<toAlloc; i++) {
    constructor_was_called = false;
    oo[i] = kmem_cache_alloc(c);
    if(constructor_was_called)
      printf("PANIC: Constructor was called during allocation of a free object\n");
  }
    
  /* Now the slab should be full */
  printf("The slab should be full now:\n");
  slab_stats(c, slb);
  s = debug_get_slab_info(c, slb);
  if(s.nFree != 0)
    printf("PANIC: Slab not yet FULL\n");

  /* This should allocate a new slab */
    
  constructor_was_called = false;
  oo[toAlloc] = kmem_cache_alloc(c);
  if(debug_get_slab(c, 1) == NULL) 
    printf("PANIC: New slab not created\n");

  if(constructor_was_called == false)
    printf("PANIC: New Objects not initialized\n");

  /* Print the New Slab */
  printf("This should be a new slab:\n");

#ifdef COUNT_FROM_LEFT
  slb1 = debug_get_slab(c, 0);
  slb  = debug_get_slab(c, 1);
#else
  slb  = debug_get_slab(c, 0);
  slb1 = debug_get_slab(c, 1);
#endif

  s1 = debug_get_slab_info(c, slb1);
  slab_stats(c, slb1);

  /* Test proper coloring */
  if((((unsigned long)(oo[toAlloc])) % PAGE_SIZE) != s1.color)
    printf("PANIC: Coloring done wrong; addr = %lx color = %ld\n", 
	   ULONG(oo[toAlloc]), ULONG(s1.color));

  printf("If you have not got any Panics till now, Allocation works fine for %s\n", cname);

  /* Destroy the newly created object. */
  destructor_was_called = false;
  kmem_cache_free(c, oo[toAlloc]);

  /* This should release the newly created slab */
  kmem_cache_reap(c);
    
  if(destructor_was_called == false)
    printf("PANIC: Destructor not called on New slab\n");

  if(debug_get_slab(c, 1) != NULL)
    printf("PANIC: 2nd slab still Exists\n");
 
  destructor_was_called = false;    
  for(i--;i>=0; i--) 
    kmem_cache_free(c, oo[i]);
    
  if(destructor_was_called)
    printf("PANIC: Objects Destroyed Unnecessarily\n");

  printf("If you have not got any Panics till now, Destruction works fine for %s\n", cname);

  slb = debug_get_slab(c, 0);
  s = debug_get_slab_info(c, slb);
  /* Testing Object Caching on a deteled object just to be really sure*/
  if(s.nFree == 0) {
    constructor_was_called = false;
    oo[0] = kmem_cache_alloc(c);
    if(constructor_was_called)
      printf("PANIC: Constructor was called during allocation of a previously deleted object\n");
    if(!isProper(oo[0], 2))
      printf("PANIC: Re-allocation of a previously deleted object in IMPROPER state\n");
    kmem_cache_free(c, oo[0]);
  }

  free(oo); 
  printf("---------------------------------------------------------\n\n");
}

/**********************************************************************
              Main Driver
**********************************************************************/

int
main()
{
  /************************************************************
            OBJECT TYPE CHECKING 
  ************************************************************/

  /* This program assumes:
        0 
     <= Small Objects 
     <  1/8 PAGE_SIZE 
     <= Large Objects 
     <  9K
     <= Huge Objects           */ 
  
#define OBJECT(t,n,es,lim) do{						\
    if(es > lim) {							\
      printf("SPEC ERROR: Object" #n " is of type " #t ", but ");	\
      printf("effective-size = %ld > %ld \n", ULONG(es), ULONG(lim));	\
      exit(1);								\
    }									\
  }while(0)
#define SMALL(n,s,a,q) OBJECT(Small,n,(s+a),(PAGE_SIZE/8))
#define LARGE(n,s,a,q) OBJECT(Large,n,(s+a),(9*1024))
#define HUGE(n,s,a,q) 
#include "objects.def"
#undef OBJECT
#undef SMALL
#undef LARGE
#undef HUGE
  
  /************************************************************
            START THE TESTS 
  ************************************************************/
  
#define OBJECT(t,n,s,a,q)			\
  cache c##n;					\
  obj o##n;
#define SMALL(n,s,a,q) OBJECT(Small,n,s,a,q)
#define LARGE(n,s,a,q) OBJECT(Large,n,s,a,q)
#define HUGE(n,s,a,q) OBJECT(Huge,n,s,a,q)
#include "objects.def"
#undef OBJECT

#ifdef LOOK_AT_BUFCTL_SLAB
  cache bufctl_cache;
  slab bufctl_slab;
#endif

  
  constructor_was_called = false;
  destructor_was_called = false;

  /* Sanity Check - to detect programs that crash before starting up */
  printf("Starting Tests\n");

  /************************************************************
            SLAB ALLOCATOR INITIALIZATION 
  ************************************************************/

#if 0
  /* Implementation Specific: 
     You **may** want to enable these checks if your implementation
     creates a cache-of-cache-headers before the world starts */
  if((ch_cache = debug_get_cacheheader_cache()) == NULL)
    printf("WARNING: No Cache Header cache at startup\n");

  else if ((sl = debug_get_slab(ch_cache, 0)) != NULL) {
    printf("WARNING: Cache Header cache ALREADY HAS a slab\n");
    slab_stats(ch_cache, sl);
  }
#endif  

  /************************************************************
            CACHE CREATION 
  ************************************************************/

  printf("Creating Object Caches...\n");
  /* Some Small Objects */
  
#define OBJECT(t,n,s,a,q)						\
  c##n = kmem_cache_create(#t, s, a, grand_constructor, grand_destructor)
#include "objects.def"
#undef OBJECT

  if(constructor_was_called)
    printf("WARNING: Constructor was called at cache Creation\n");

  if(destructor_was_called)
    printf("PANIC: destructor was called at cache creation\n");
  
  if(debug_get_cacheheader_cache() == NULL)
    printf("PANIC: NO cache-header cache\n");
  

  /************************************************************
            OBJECT / SLAB CREATION 
  ************************************************************/

  check_size = true;
  printf("Allocating Objects ...\n");

#define OBJECT(t,n,s,a,q) do {			\
    Osize = s;					\
    alreadyPanicked = false;			\
    o##n = create_first_obj(c##n, s, a, "o"#n);	\
  } while(0)		
#include "objects.def"
#undef OBJECT

  printf("DONE allocating Objects\n");
  Osize = 2;
  check_size = false;

  /* At this point, there must be a slab for all caches, 
     and there must be a bufctl slab */
#ifdef LOOK_AT_BUFCTL_SLAB
  bufctl_cache = debug_get_bufctl_cache();
  if(bufctl_cache == NULL) {
    printf("PANIC: NO BUFCTL cache\n");
    printf("ATTN: Please Choose RIGHT Value for small_bufctl_size; I picked 8 \n");
    small_bufctl_size = 8;
  }
  else {
    bufctl_slab = debug_get_slab(bufctl_cache, 0);
    if(bufctl_slab == NULL) {
      printf("PANIC: NO BUFCTL Slab\n");
      printf("ATTN: Please Choose RIGHT Value for small_bufctl_size I picked 8\n");
      small_bufctl_size = 8;
    }
    else {
      /* bufctls for small objects should be 1 or 2 link pointers */
      printf("bufctl statistics:\n");
      const struct slab_query s = debug_get_slab_info(bufctl_cache, bufctl_slab); 
      if(((s.nFree + s.nAlloc) * (EFFECTIVE_SIZE((s.obj_size + 8), s.align))) > PAGE_SIZE) {
	small_bufctl_size = 4;
	printf("ATTN: Picking small_bufctl_size = 4\n");
      }
      else {
	small_bufctl_size = 8;
	printf("ATTN: Picking small_bufctl_size = 8\n");
      }
      slab_stats(bufctl_cache, bufctl_slab);
    }
  }
#endif

  /************************************************************
            SLAB / FRAGMENTATION / OBJECT CACHING TESTS 
  ************************************************************/

#define OBJECT(t,n,s,a,q)			\
  test(c##n, "c"#n)
#undef HUGE
#define HUGE(n,s,a,q)  
#include "objects.def"
#undef HUGE
#define HUGE(n,s,a,q) OBJECT(Huge, n,s,a,q)
#undef OBJECT
  /* I am not testing huge objects for object caching because the behaviour was not
     well specified and there is nmo gold standard for declaring a object huge.
     However, I have already tested proper allocation and construction  */

  /************************************************************
            OBJECT DELETION 
  ************************************************************/

#define OBJECT(t,n,s,a,q) kmem_cache_free(c##n, o##n)
#include "objects.def"
#undef OBJECT

  /************************************************************
            STRESS TESTS 
  ************************************************************/
  
#ifdef ST_TEST
  printf("Running Stress Test...\n");

#define OBJECT(t,n,s,a,q)			\
  obj oo##n[q];
#include "objects.def"
#undef OBJECT

#define OBJECT(t,n,s,a,q) do {				\
    size_t i;						\
    for(i=0; i<q; i++){					\
      oo##n[i] = kmem_cache_alloc(c##n);}		\
  } while(0)
#include "objects.def"
#undef OBJECT

/*  bool something_deleted=false;
  size_t i=0;
  do {
    something_deleted=false;
#define OBJECT(t,n,s,a,q) do {			\
      if(i < q) {				\
	something_deleted = true;		\
	kmem_cache_free(c##n, oo##n[i]);	\
      }						\
    } while(0)
#include "objects.def"
#undef OBJECT
    i++;    
  } while(something_deleted);  */


#define OBJECT(t,n,s,a,q) do {				\
    size_t i;						\
    for(i=0; i<q; i++){					\
     kmem_cache_free(c##n, oo##n[i]);}		\
  } while(0)
#include "objects.def"
#undef OBJECT


#endif /* ST_TEST */

  /************************************************************
            MEMORY REAP 
  ************************************************************/
  
  printf("Now testing kmem_reap()\n");
#define OBJECT(t,n,s,a,q) do {				\
    kmem_cache_reap(c##n);				\
    size_t nSlabs = debug_get_nslabs(c##n);		\
    if(nSlabs) {					\
      printf("PANIC: Empty Cache-" #n);			\
      printf(" not completely reap()ed --");		\
    }							\
  } while(0)
#include "objects.def"
#undef OBJECT

  /************************************************************
            CACHE DELETION 
  ************************************************************/

  printf("Now freeing All Caches\n");

#define OBJECT(t,n,s,a,q)			\
  kmem_cache_destroy(c##n)
#include "objects.def"
#undef OBJECT
#undef SMALL
#undef LARGE
#undef HUGE

  printf("If you have not got a seg-fault or Panic till now, the allocator is probably OK\n");
  
  printf("End of ALL tests\n");
  fflush(stdout);

  return 0;
}

