/***********************************************************
 Bare-bone source file. Please fill in the function stubs  
 with real implementations. Feel free to reorganize code structure
 into multiple files (upate the Makefile accordingly). 
 ***********************************************************/

#include <stdlib.h>
#include <stdlib.h>
#include "dslab.h"
#include "dslabdefine.h"
#include "dslabbuild.h"
#include "dlist.h"
#include "dslabobj.h"
#include "bllbarw.h"
#include "debugmsg.h"

/****************************************************************
   The Main Allocator 
 ***************************************************************/

bool booted=false;

vptr_cache
dmem_cache_create(char *name,
		  size_t size,
		  int align,
		  void (*constructor)(void *, size_t),
		  void (*destructor)(void *, size_t))
{
	PRINTNL;
	PDEBUG_DSLAB("Entered:name=%s\tsize=%d\n",name,size);
     if(!booted)
     {
          __init(false);
          booted=true;
     }

	vptr tmp_vcp;
	vptr_cache vcp;

	struct dmem_cache *cp=(struct dmem_cache *)__getmem(CACHE_LEN);

	tmp_vcp=dmem_cache_alloc(cc_vptr);
	CAST_CACHE(vcp, tmp_vcp);
	
	strcpy(cp->name, name);
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
	
	cp->fullslabs=vsnull;
	cp->freeslabs=vsnull;
	cp->partialslabs=vsnull;

	PDEBUG_DSLAB("cp->freeslabs:");	PRINTA_DSLAB(cp->freeslabs);
	PDEBUG_DSLAB("cp->partialslabs:");	PRINTA_DSLAB(cp->partialslabs);	
	PDEBUG_DSLAB("cp->fullslabs:");	PRINTA_DSLAB(cp->fullslabs);	
	PDEBUG_DSLAB("Exiting:cp=");		PRINTA_DSLAB(vcp);	

	storec(vcp, CACHE_LEN, (unsigned char *)cp);
	free(cp);
	PRINTNL;
	return vcp;
}
 
void 
dmem_cache_destroy(vptr_cache vcp)
{
	size_t npages=0;	
	vptr tmp_cache;
	

	struct dmem_cache *cp=(struct dmem_cache *)__getmem(CACHE_LEN);
	loadc(vcp, CACHE_LEN, (unsigned char *)cp);
	
	if(!slabListEmpty(cp->partialslabs) || !slabListEmpty(cp->fullslabs))
	{
		PDEBUG_DSLAB("You cant destroy this cache as there is active objects\n");
	}
	else
	{
		npages=dmem_cache_reap(vcp);
		CAST_VPTR(tmp_cache, vcp);
		dmem_cache_free(cc_vptr, tmp_cache);
		PDEBUG_DSLAB("%d pages freed\n",npages);
	}

	free(cp);
	return;
}

vptr
dmem_cache_alloc(vptr_cache vcp)
{
	vptr ret_buf;
	static int slab_num=0;	
	int obj_type;

	struct dmem_cache *cp=(struct dmem_cache *)__getmem(CACHE_LEN);
	loadc(vcp, CACHE_LEN, (unsigned char *)cp);

	PRINTNL;
	PDEBUG_DSLAB("Entered:");		PRINTA_DSLAB(vcp);	
	PDEBUG_DSLAB("cp->freeslabs:");	PRINTA_DSLAB(cp->freeslabs);
	PDEBUG_DSLAB("cp->partialslabs:");	PRINTA_DSLAB(cp->partialslabs);	

	obj_type=cp->obj_type;
	if(EQUAL(cp->freeslabs, vsnull) && EQUAL(cp->partialslabs,vsnull))
	{

		switch(obj_type){
			case OBJ_SMALL	:
				__getslab_os(vcp);
				break;

			case OBJ_LARGE	:
			case OBJ_HUGE	:
				__getslab_olh(vcp);
				break;
			default		:
				printf("Object type is stored wrongly\n");
				exit(0);
		}
		if(cp->size==3300) 
			slab_num++;
		PDEBUG_DSLAB("OBJ SIZE IS: %d:\t SLAB NUM:%d\n",cp->size,slab_num);
	}

     loadc(vcp, CACHE_LEN, (unsigned char *)cp);
     PDEBUG_DSLABBUILD("cp->freeslabs:");     PRINTA_DSLABBUILD(cp->freeslabs);

     switch(obj_type){
          case OBJ_SMALL :
			ret_buf=dslab_alloc_os(vcp);
               break;

          case OBJ_LARGE :
			ret_buf=dslab_alloc_ol(vcp);
               break;

          case OBJ_HUGE  :
			ret_buf=dslab_alloc_oh(vcp);
               break;
          default        :
               printf("Object type is stored wrongly\n");
               exit(0);
     }

     loadc(vcp, CACHE_LEN, (unsigned char *)cp);
	PDEBUG_DSLAB("vcp:");			PRINTA_DSLAB(vcp);	
	PDEBUG_DSLAB("cp->freeslabs:");	PRINTA_DSLAB(cp->freeslabs);	
	PDEBUG_DSLAB("cp->partialslabs:");	PRINTA_DSLAB(cp->partialslabs);	

	PDEBUG_DSLAB("Exiting:buffer allocated:");	PRINTA_DSLAB(ret_buf);
	
	PRINTNL;
	free(cp);
	return ret_buf;
}

void 
dmem_cache_free(vptr_cache vcp, vptr buf)
{
	int obj_type;

	PDEBUG_DSLAB("Enetered\n");
	struct dmem_cache *cp=(struct dmem_cache *)__getmem(CACHE_LEN);
	loadc(vcp, CACHE_LEN, (unsigned char *)cp);

	obj_type=cp->obj_type;

     switch(obj_type){
          case OBJ_SMALL :
			dslab_free_os(vcp, buf);
               break;

          case OBJ_LARGE :
			dslab_free_ol(vcp, buf);
               break;

          case OBJ_HUGE  :
			dslab_free_oh(vcp, buf);
               break;
          default        :
               printf("Object type is stored wrongly\n");
               exit(0);
     }

	free(cp);
	PDEBUG_DSLAB("Exiting\n");
	return;
}

size_t 
dmem_cache_reap(vptr_cache vcp)
{
	size_t npages;
	int obj_type;
	vptr_slab tmp_slab, free_slab;
	
	PDEBUG_DSLAB("Entered\n");	

	struct dmem_cache *cp=(struct dmem_cache *)__getmem(CACHE_LEN);
	struct dmem_slab *sp=(struct dmem_slab *)__getmem(SLAB_LEN);
	loadc(vcp, CACHE_LEN, (unsigned char *)cp);

	npages=0;
     obj_type=cp->obj_type;
	tmp_slab=cp->freeslabs;

	if(!slabListEmpty(tmp_slab))
	{
		PDEBUG_DSLAB("Freeslablist is not empty\n");	
		do{
			PDEBUG_DSLAB("Parrrrrrrrrrrrr:");	
			PRINTA_DSLAB(tmp_slab);
			free_slab=tmp_slab;
			PDEBUG_DSLAB("cp->obj_type:%d\n",cp->obj_type);	

			cp->freeslabs=rm_slab(cp->freeslabs,tmp_slab);

			loads(tmp_slab, SLAB_LEN, (unsigned char *)sp);
			tmp_slab=sp->next;

     		switch(obj_type){
	     	     case OBJ_SMALL :
					freeslab_os(free_slab);
          	     	break;

		          case OBJ_LARGE :
					freeslab_ol(free_slab);
          		     break;

		          case OBJ_HUGE  :
					freeslab_oh(free_slab);
          		     break;

		          default        :
     		          printf("Object type is stored wrongly\n");
          		     exit(0);
		     }   

			npages +=	cp->slabsize;
			PDEBUG_DSLAB("Tarrrrrrrrrrrrr:");	
			PRINTA_DSLAB(tmp_slab);	
		}while(NOTEQUAL(cp->freeslabs,vsnull));

		storec(vcp, CACHE_LEN, (unsigned char *)cp);
	}
	else
		npages=0;
	
	free(sp);
	free(cp);

	PDEBUG_DSLAB("Exiting\n");	
	return npages;
}
/****************************************************************
   Debugging Support 
 ***************************************************************/

size_t 
debug_get_nslabs(vptr_cache vcp)
{
	vptr_slab tmp_slab;
	int nslabs=0;

	PDEBUG_DSLAB("Entered\n");	

	struct dmem_cache *cp=(struct dmem_cache *)__getmem(CACHE_LEN);
	struct dmem_slab *sp=(struct dmem_slab *)__getmem(SLAB_LEN);
	loadc(vcp, CACHE_LEN, (unsigned char *)cp);

	if(!slabListEmpty(cp->fullslabs))
	{
		tmp_slab=cp->fullslabs;
		do{
			nslabs++;
			loads(tmp_slab, SLAB_LEN, (unsigned char *)sp);
			tmp_slab=sp->next;
		}while(NOTEQUAL(tmp_slab, cp->fullslabs));
		PDEBUG_DSLAB("No of full slabs:%d\n", nslabs);	
	}
		

	if(!slabListEmpty(cp->partialslabs))
	{
		tmp_slab=cp->partialslabs;
		do{
			nslabs++;
			loads(tmp_slab, SLAB_LEN, (unsigned char *)sp);
			tmp_slab=sp->next;
		}while(NOTEQUAL(tmp_slab, cp->partialslabs));
		PDEBUG_DSLAB("No of partial slabs:%d\n", nslabs);	
	}
		

	if(!slabListEmpty(cp->freeslabs))
	{
		tmp_slab=cp->freeslabs;
		do{
			nslabs++;
			loads(tmp_slab, SLAB_LEN, (unsigned char *)sp);
			tmp_slab=sp->next;
		}while(NOTEQUAL(tmp_slab, cp->freeslabs));
		PDEBUG_DSLAB("No of free slabs:%d\n", nslabs);	
	}
		
	free(sp);
	free(cp);
	return nslabs;
}


vptr_slab
debug_get_slab(vptr_cache vcp, size_t i)
{
	vptr_slab tmp_slab, ret_slab;
	int nslabs=0;

	struct dmem_cache *cp=(struct dmem_cache *)__getmem(CACHE_LEN);
	struct dmem_slab *sp=(struct dmem_slab *)__getmem(SLAB_LEN);
	loadc(vcp, CACHE_LEN, (unsigned char *)cp);

	ret_slab=vsnull;
	if(!slabListEmpty(cp->fullslabs))
	{
		tmp_slab=cp->fullslabs;
		do{
			if(nslabs==i)
				return tmp_slab;
			nslabs++;
			loads(tmp_slab, SLAB_LEN, (unsigned char *)sp);
			tmp_slab=sp->next;
		}while(NOTEQUAL(tmp_slab, cp->fullslabs));
	}
		

	if(!slabListEmpty(cp->partialslabs))
	{
		tmp_slab=cp->partialslabs;
		do{
			if(nslabs==i)
				return tmp_slab;
			nslabs++;
			loads(tmp_slab, SLAB_LEN, (unsigned char *)sp);
			tmp_slab=sp->next;
		}while(NOTEQUAL(tmp_slab, cp->partialslabs));
	}
		

	if(!slabListEmpty(cp->freeslabs))
	{
		tmp_slab=cp->freeslabs;
		do{
			if(nslabs==i)
				return tmp_slab;
			nslabs++;
			loads(tmp_slab, SLAB_LEN, (unsigned char *)sp);
			tmp_slab=sp->next;
		}while(NOTEQUAL(tmp_slab, cp->freeslabs));
	}

	free(sp);
	free(cp);
			
	if(nslabs==0) 
		return vsnull;		

	return ret_slab;
}

struct slab_query
debug_get_slab_info(vptr_cache vcp, vptr_slab slab)
{
	struct slab_query s = {0,0,0,0,0};
	
	struct dmem_cache *cp=(struct dmem_cache *)__getmem(CACHE_LEN);
	struct dmem_slab *sp=(struct dmem_slab *)__getmem(SLAB_LEN);
	loadc(vcp, CACHE_LEN, (unsigned char *)cp);
	loads(slab, CACHE_LEN, (unsigned char *)sp);

	s.color=sp->color;
	s.size=cp->obj_size;
	s.align=cp->align;
	s.nFree=cp->nobjs - sp->refcount;
	s.nAlloc=sp->refcount;

	free(cp);
	free(sp);

	return s;  
}

vptr_cache
debug_get_cacheheader_cache()
{	
	PDEBUG_DSLAB("Entered\n");
	PRINTA_DSLAB(cc_vptr);
	PDEBUG_DSLAB("Exiting\n");
	return cc_vptr;
}

vptr_cache
debug_get_bufctl_cache()
{
  return cb_vptr;
}
