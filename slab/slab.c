/***********************************************************
 Bare-bone source file. Please fill in the function stubs  
 with real implementations. Feel free to reorganize code structure
 into multiple files (upate the Makefile accordingly). 
 ***********************************************************/

#include <slab.h>
#include <stdlib.h>
#include <slab.h>
#include <stdlib.h>
#include "slabdefine.h"
#include "slabbuild.h"
#include "list.h"
#include "slabobj.h"
#include "slabmsg.h"

/****************************************************************
   The Main Allocator 
 ***************************************************************/

bool booted=false;


struct kmem_cache *
kmem_cache_create(char *name,
		  size_t size,
		  int align,
		  void (*constructor)(void *, size_t),
		  void (*destructor)(void *, size_t))
{
	PDEBUG_SLAB("Entered:name=%s\tsize=%d\n",name,size);
     if(!booted)
     {
          __init();
          booted=true;
     }

	struct kmem_cache *cp;

	cp=kmem_cache_alloc(cc_ptr);
	
	cp->name=name;
	/*Need to do alignment calculation before assigning size*/
	cp->obj_size=size;
	cp->align=align;
	
	cp->obj_type=__det_objtype(cp->obj_size);

	if(cp->obj_type == OBJ_SMALL)
		cp->size=EFFECTIVE_SIZE((cp->obj_size+SMALL_BUFCTL_SIZE),cp->align)-\
				SMALL_BUFCTL_SIZE;
	else
		cp->size=EFFECTIVE_SIZE((cp->obj_size+0),cp->align);

	if(cp->obj_type == OBJ_SMALL)
	    __slab_calc(EFFECTIVE_SIZE((cp->obj_size+SMALL_BUFCTL_SIZE),cp->align),\
     	           &(cp->nobjs), &(cp->slabsize));
	else //if(cp->obj_type == OBJ_LARGE)
	    __slab_calc((cp->size),
     	           &(cp->nobjs), &(cp->slabsize));
		/****PANIC****/

	cp->constructor=constructor;
	cp->destructor=destructor;

	PDEBUG_SLAB("cp->freeslabs:%lu\n",(unsigned long)cp->freeslabs);
	PDEBUG_SLAB("cp->partialslabs:%lu\n",(unsigned long)cp->partialslabs);
	PDEBUG_SLAB("cp->fullslabs:%lu\n",(unsigned long)cp->fullslabs);
	PDEBUG_SLAB("Exiting:cp=%lu\n",(unsigned long)cp);
	return cp;
}
 
void 
kmem_cache_destroy(struct kmem_cache *cp)
{
	size_t npages=0;	
	if(!slabListEmpty(cp->partialslabs) || !slabListEmpty(cp->fullslabs))
	{
		PDEBUG_SLAB("You cant destroy this cache as there is active objects\n");
	}
	else
	{
		npages=kmem_cache_reap(cp);
		kmem_cache_free(cc_ptr, (void *)cp);
		PDEBUG_SLAB("%d pages freed\n",npages);
	}
	return;
}

void *
kmem_cache_alloc(struct kmem_cache *cp)
{
	void *ret_buf;
	static int slab_num=0;	

	PDEBUG_SLAB("Entered:%lu\n",(unsigned long)cp);
	PDEBUG_SLAB("cp->freeslabs:%lu\n",(unsigned long)cp->freeslabs);
	PDEBUG_SLAB("cp->partialslabs:%lu\n",(unsigned long)cp->partialslabs);
	if(cp->freeslabs==NULL && cp->partialslabs==NULL)
	{
		__getslab(cp);
		if(cp->size==3300) 
			slab_num++;
		PDEBUG_SLAB("OBJ SIZE IS: %d:\t SLAB NUM:%d\n",cp->size,slab_num);
	}
	ret_buf=kslab_alloc(cp);
	PDEBUG_SLAB("Exiting:buffer allocated:%lu\n",(unsigned long)ret_buf);
	return ret_buf;
}

void 
kmem_cache_free(struct kmem_cache *cp, void *buf)
{
	PDEBUG_SLAB("Enetered\n");
	kslab_free(cp, buf);
	PDEBUG_SLAB("Exiting\n");
	return;
}

size_t 
kmem_cache_reap(struct kmem_cache *cp)
{
	size_t npages;
	struct kmem_slab *tmp_slab, *free_slab;
	
	PDEBUG_SLAB("Entered\n");	
	npages=0;
	tmp_slab=cp->freeslabs;

	if(!slabListEmpty(tmp_slab))
	{
		PDEBUG_SLAB("Free_slab is not empty\n");	
		do{
			PDEBUG_SLAB("Parrrrrrrrrrrrr:%lu\n",(unsigned long)tmp_slab);	
			free_slab=tmp_slab;
			cp->freeslabs=rm_slab(cp->freeslabs,tmp_slab);
			tmp_slab=free_slab->next;
			freeslab(free_slab);
			npages +=	cp->slabsize;
			PDEBUG_SLAB("Tarrrrrrrrrrrrr:%lu\n",(unsigned long)tmp_slab);	

		}while(cp->freeslabs!=NULL);
	}
	else
		npages=0;
	
	PDEBUG_SLAB("Exiting\n");	
	return npages;
}
/****************************************************************
   Debugging Support 
 ***************************************************************/

size_t 
debug_get_nslabs(struct kmem_cache *cp)
{
	struct kmem_slab *tmp_slab;
	int nslabs=0;

	PDEBUG_SLAB("Entered\n");	
	if(!slabListEmpty(cp->fullslabs))
	{
		tmp_slab=cp->fullslabs;
		do{
			nslabs++;
			tmp_slab=tmp_slab->next;
		}while(tmp_slab!=cp->fullslabs);
		PDEBUG_SLAB("No of full slabs:%d\n", nslabs);	
	}
		

	if(!slabListEmpty(cp->partialslabs))
	{
		tmp_slab=cp->partialslabs;
		do{
			nslabs++;
			tmp_slab=tmp_slab->next;
		}while(tmp_slab!=cp->partialslabs);
		PDEBUG_SLAB("No of partial slabs:%d\n", nslabs);	
	}
		

	if(!slabListEmpty(cp->freeslabs))
	{
		tmp_slab=cp->freeslabs;
		do{
			nslabs++;
			tmp_slab=tmp_slab->next;
		}while(tmp_slab!=cp->freeslabs);
		PDEBUG_SLAB("No of free slabs:%d\n", nslabs);	
	}
		
	return nslabs;
}


struct kmem_slab *
debug_get_slab(struct kmem_cache *cp, size_t i)
{
	struct kmem_slab *tmp_slab, *ret_slab;
	int nslabs=0;

	ret_slab=NULL;
	if(!slabListEmpty(cp->fullslabs))
	{
		tmp_slab=cp->fullslabs;
		do{
			if(nslabs==i)
				return tmp_slab;
			nslabs++;
			tmp_slab=tmp_slab->next;
		}while(tmp_slab!=cp->fullslabs);
	}
		

	if(!slabListEmpty(cp->partialslabs))
	{
		tmp_slab=cp->partialslabs;
		do{
			if(nslabs==i)
				return tmp_slab;
			nslabs++;
			tmp_slab=tmp_slab->next;
		}while(tmp_slab!=cp->partialslabs);
	}
		

	if(!slabListEmpty(cp->freeslabs))
	{
		tmp_slab=cp->freeslabs;
		do{
			if(nslabs==i)
				return tmp_slab;
			nslabs++;
			tmp_slab=tmp_slab->next;
		}while(tmp_slab!=cp->freeslabs);
	}
	if(nslabs==0) return NULL;		
	return ret_slab;
}

struct slab_query
debug_get_slab_info(struct kmem_cache *cp, 
		    struct kmem_slab *slab)
{
	struct slab_query s = {0,0,0,0,0};
	
	s.color=slab->color;
	s.obj_size=cp->obj_size;
	s.align=cp->align;
	s.nFree=cp->nobjs - slab->refcount;
	s.nAlloc=slab->refcount;

	return s;  
}

struct kmem_cache *
debug_get_cacheheader_cache()
{	
  return cc_ptr;
}

struct kmem_cache *
debug_get_bufctl_cache()
{
  return cb_ptr;
}
