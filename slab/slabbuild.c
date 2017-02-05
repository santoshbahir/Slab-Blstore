
/***The implementation of slabbuild.h***/

#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <strings.h>
#include "slab.h"
#include "list.h"
#include "slabdefine.h"
#include "slabbuild.h"
#include "slabobj.h"
#include "slabmsg.h"

/******************************************************************************
 * * This three are baap structures residing outside of slab and their
 * correspond-*
 * * -ing pointers.
 * *
 * *******************************************************************************/ 

void __init()
{
	cc_ptr=&cc;
	cs_ptr=&cs;
	cb_ptr=&cb;

	PDEBUG_SLABBUILD("Entered\n"); 
     PDEBUG_SLABBUILD("CC_PTR=%lu\tCS_PTR=%lu\tCB_PTR=%lu\n",\
          (unsigned long)cc_ptr,(unsigned long)cs_ptr,(unsigned long)cb_ptr);

	/*Hard coding hihawhawhaw*/
	/*Intialize cache object for cache of caches */
	cc_ptr->name="Cache of kmem_caches";
	cc_ptr->size=sizeof(struct kmem_cache);
	cc_ptr->obj_size=sizeof(struct kmem_cache);
	cc_ptr->align=1;

	PDEBUG_SLABBUILD("EFFECTIVE_SIZE:%d\n",EFFECTIVE_SIZE((cc_ptr->obj_size+SMALL_BUFCTL_SIZE),cc_ptr->align));
	__slab_calc(EFFECTIVE_SIZE((cc_ptr->obj_size+SMALL_BUFCTL_SIZE),cc_ptr->align),
			&(cc_ptr->nobjs), &(cc_ptr->slabsize));
	cc_ptr->obj_type=__det_objtype(cc_ptr->size+SMALL_BUFCTL_SIZE);	

	cc_ptr->freeslabs=NULL;
	cc_ptr->partialslabs=NULL;
	cc_ptr->fullslabs=NULL;
	cc_ptr->constructor=&__cache_constructor;
	cc_ptr->destructor=&__cache_destructor;

	/*Intialize cache object for cache of slabs */
     cs_ptr->name="Cache of kmem_slabs";
     cs_ptr->size=sizeof(struct kmem_slab);
	cs_ptr->obj_size=sizeof(struct kmem_slab);
     cs_ptr->align=0;

	__slab_calc(EFFECTIVE_SIZE((cs_ptr->obj_size+SMALL_BUFCTL_SIZE),cs_ptr->align),
               &(cs_ptr->nobjs), &(cs_ptr->slabsize));
	PDEBUG_SLABBUILD("slabcache_size:%d\n",cs_ptr->obj_size); 
	PDEBUG_SLABBUILD("EFFECTIVE_SIZE:%d\n",EFFECTIVE_SIZE((cs_ptr->obj_size+SMALL_BUFCTL_SIZE),cs_ptr->align));
	PDEBUG_SLABBUILD("nobjs:%d\tslabsize:%d\n",cs_ptr->nobjs, cs_ptr->slabsize); 
     cs_ptr->obj_type=__det_objtype(cs_ptr->size+SMALL_BUFCTL_SIZE);

     cs_ptr->freeslabs=NULL;
     cs_ptr->partialslabs=NULL;
     cs_ptr->fullslabs=NULL;
     cs_ptr->constructor=&__slab_constructor;
     cs_ptr->destructor=&__slab_destructor;

	/*Intialize cache object for cache of bufctl */
     cb_ptr->name="Cache of kmem_bufctl";
     cb_ptr->size=sizeof(struct kmem_bufctl);
     cb_ptr->obj_size=sizeof(struct kmem_bufctl);
     cb_ptr->align=1;

	__slab_calc(EFFECTIVE_SIZE((cb_ptr->obj_size+SMALL_BUFCTL_SIZE),cb_ptr->align),
               &(cb_ptr->nobjs), &(cb_ptr->slabsize));
     cb_ptr->obj_type=__det_objtype(cb_ptr->size+SMALL_BUFCTL_SIZE);

     cb_ptr->freeslabs=NULL;
     cb_ptr->partialslabs=NULL;
     cb_ptr->fullslabs=NULL;
     cb_ptr->constructor=&__bufctl_constructor;
     cb_ptr->destructor=&__bufctl_destructor;

	PDEBUG_SLABBUILD("Exiting\n"); 
	return;
}


void __getslab(struct kmem_cache *cache_ptr)
{
	struct kmem_slab *slab_header;
	void *tmp_addr;	

	PDEBUG_SLABBUILD("Entered\n"); 
//	if(cache_ptr->obj_type!=OBJ_HUGE)
	{
		if(cache_ptr->obj_type==OBJ_SMALL)
		{
			PDEBUG_SLABBUILD("Getting slab for small objects\n"); 
			tmp_addr=__get_rawslab(cache_ptr->slabsize);
			slab_header=(struct kmem_slab *)((tmp_addr + \
						(cache_ptr->slabsize*PAGESIZE)) - \
						sizeof(struct kmem_slab));
			slab_header->slab_addr=tmp_addr;
			slab_header->head=NULL; /*For large obj; constr will set it to NULL*/
		}
		else 
		{
			PDEBUG_SLABBUILD("Getting slab for large objects\n"); 
			slab_header=(struct kmem_slab *)kmem_cache_alloc(cs_ptr);
			slab_header->slab_addr=__get_rawslab(cache_ptr->slabsize);
			if(MAX_COLOR(cache_ptr))
				slab_header->color=(cache_ptr->prev_color + cache_ptr->align)%\
							(MAX_COLOR(cache_ptr));
			else
				slab_header->color=0;
		}

		slab_header->head=NULL; 
		slab_header->refcount=0;	
		slab_header->color=0; /******PANIC**********/
		slab_header->used=NULL; /******PANIC**********/
		slab_header->parent=cache_ptr;
		PDEBUG_SLABBUILD("slab_header->head=%lu\n",(unsigned long)slab_header->head); 
		__buildbufctl(slab_header);
		PDEBUG_SLABBUILD("slab_header->head=%lu\n",(unsigned long)slab_header->head); 
		cache_ptr->freeslabs=add_slab(cache_ptr->freeslabs,slab_header);
	}
/*	else	
	{
		PDEBUG_SLABBUILD("Getting slab for large objects\n"); 
		slab_header=(struct kmem_slab *)kmem_cache_alloc(cs_ptr);\
		slab_header->slab_addr=__get_rawslab(cache_ptr->slabsize);
	     slab_header->refcount=0; 
          slab_header->color=-1; //PANIC
          slab_header->used=NULL; //PANIC
          slab_header->parent=cache_ptr;
		__buildbufctl(slab_header);
		cache_ptr->freeslabs=add_slab(cache_ptr->freeslabs,slab_header);
	}*/
	PDEBUG_SLABBUILD("Exiting\n"); 
	return;
}


void __buildbufctl(struct kmem_slab *slab_ptr)
{
	int i, offset;
	struct kmem_bufctl *tmp_bufctl, **bufctl_addr;
	void *tmp_buf;

	PDEBUG_SLABBUILD("Entered\n"); 
	tmp_buf=slab_ptr->slab_addr;
	if(slab_ptr->parent->obj_type==OBJ_SMALL)
	{
		PDEBUG_SLABBUILD("BUilding bufctl for small objects\n"); 
		slab_ptr->head=(struct kmem_bufctl *)tmp_buf;
		offset=(slab_ptr->parent->size)+SMALL_BUFCTL_SIZE;

		PDEBUG_SLABBUILD("Total objects(nobjs):%d\n",slab_ptr->parent->nobjs); 
		for(i=0; i<(slab_ptr->parent->nobjs); i++)
		{
			bufctl_addr=(struct kmem_bufctl **)(slab_ptr->slab_addr+\
					  ((slab_ptr->parent->size*(i+1)+\
					  i*SMALL_BUFCTL_SIZE)));

			slab_ptr->parent->constructor(tmp_buf, slab_ptr->parent->obj_size);
			PDEBUG_SLABBUILD("cache name:%s\t%dth object\t buf=%lu\n",slab_ptr->parent->name,i,
				(unsigned long)((struct kmem_cache *)tmp_buf)); 
			if(i==((slab_ptr->parent->nobjs)-1))
				*bufctl_addr=NULL;
			else
			{
				tmp_buf=tmp_buf+offset;	
				*bufctl_addr=(struct kmem_bufctl *)tmp_buf;
			}
		}
	}
	else if(slab_ptr->parent->obj_type==OBJ_LARGE)
	{
		PDEBUG_SLABBUILD("BUilding bufctl for large objects cache:%s\n",\
						slab_ptr->parent->name); 
		PDEBUG_SLABBUILD("nobjs:%d\n",(slab_ptr->parent->nobjs)); 
		for(i=0; i<(slab_ptr->parent->nobjs); i++)
		{
			tmp_buf=slab_ptr->slab_addr+(i*(slab_ptr->parent->size));
			slab_ptr->parent->constructor(tmp_buf, slab_ptr->parent->obj_size);
			
			PDEBUG_SLABBUILD("Only place where bufctl objects allocated\n");
			tmp_bufctl=(struct kmem_bufctl *)kmem_cache_alloc(cb_ptr);
			tmp_bufctl->buf=tmp_buf;
			tmp_bufctl->parent=slab_ptr;

			PDEBUG_SLABBUILD("parent_cache=%lu\tparent_slab=%lu\n",(unsigned long)slab_ptr->parent,\
							(unsigned long)slab_ptr);

			PDEBUG_SLABBUILD("bn=%d\ttmp_bufctl=%lu\tobj_size=%d\n",i,\
							(unsigned long)tmp_bufctl, slab_ptr->parent->obj_size);
			
			PDEBUG_SLABBUILD("bn=%d\ttmp_bufctl->buf=%lu\ttmp_bufctl->next=%lu\n",i,\
				(unsigned long)tmp_bufctl->buf,(unsigned long)tmp_bufctl->next); 

			slab_ptr->head=add_kmem_bufctl(slab_ptr->head,tmp_bufctl);

			PDEBUG_SLABBUILD("bn=%d\tslab_ptr->head=%lu\n",i,\
							(unsigned long)slab_ptr->head); 
		}
	}
	else
	{	
		slab_ptr->parent->constructor(slab_ptr->slab_addr, slab_ptr->parent->obj_size);	
		PDEBUG_SLABBUILD("is it failing here?\n");
	}

	PDEBUG_SLABBUILD("Exiting\n"); 
	return;
}


void *__get_rawslab(int slabsize)
{
	void *slab_addr;
	PDEBUG_SLABBUILD("Entered\n"); 
	slab_addr=mmap(NULL, slabsize*getpagesize(), \
			PROT_READ|PROT_WRITE|PROT_EXEC,MAP_ANONYMOUS|MAP_PRIVATE,-1,0);
	PDEBUG_SLABBUILD("Exiting\n"); 
	return slab_addr;	
}

void freeslab(struct kmem_slab *slab_ptr)
{
	int i, offset;	
	void *tmp_buf;
	struct kmem_bufctl **bufctl_addr,*tmp_bufctl,*free_bufctl;

	if(slab_ptr->parent->obj_type == OBJ_SMALL)
	{
          PDEBUG_SLABBUILD("freeing objects for small objects\n"); 
	     offset=(slab_ptr->parent->size)+SMALL_BUFCTL_SIZE;
		tmp_buf=slab_ptr->slab_addr;	

          PDEBUG_SLABBUILD("Total objects(nobjs):%d\n",slab_ptr->parent->nobjs); 
          for(i=0; i<(slab_ptr->parent->nobjs); i++)
          {
               bufctl_addr=(struct kmem_bufctl **)(slab_ptr->slab_addr+\
                           ((slab_ptr->parent->size*(i+1)+\
                           i*SMALL_BUFCTL_SIZE)));

               slab_ptr->parent->destructor(tmp_buf, slab_ptr->parent->obj_size);
               PDEBUG_SLABBUILD("cache name:%s\t%dth object\t buf=%lu\n",slab_ptr->parent->name,i,
                    (unsigned long)((struct kmem_cache *)tmp_buf)); 
               if(i!=((slab_ptr->parent->nobjs)-1))
               {
                    tmp_buf=tmp_buf+offset;  
                    *bufctl_addr=(struct kmem_bufctl *)tmp_buf;
               }
          }
	}
	else if(slab_ptr->parent->obj_type == OBJ_LARGE)
	{
          PDEBUG_SLABBUILD("freeing slab for large objects\n"); 
		tmp_bufctl=slab_ptr->head;	
		do{
			slab_ptr->parent->destructor(tmp_bufctl->buf, slab_ptr->parent->obj_size);	
			free_bufctl=tmp_bufctl;
			tmp_bufctl=tmp_bufctl->next;
			kmem_cache_free(cb_ptr, free_bufctl);
		}while(tmp_bufctl!=NULL);
		
		__free_rawslab(slab_ptr->slab_addr, slab_ptr->parent->slabsize);	
		kmem_cache_free(cs_ptr, slab_ptr);
	}
	else	
	{
          PDEBUG_SLABBUILD("freeing slab for huge objects\n");
          slab_ptr->parent->destructor(slab_ptr->slab_addr, slab_ptr->parent->obj_size);

          __free_rawslab(slab_ptr->slab_addr, slab_ptr->parent->slabsize);
          kmem_cache_free(cs_ptr, slab_ptr);
	}
}

void __free_rawslab(void *slab_addr, int slabsize)
{
     int ret_value;

     ret_value=munmap(slab_addr, slabsize*PAGESIZE);

     if(ret_value==0)
          PDEBUG_SLABBUILD("The unmapping is successful\n");
     else
          PDEBUG_SLABBUILD("The unmapping is failed\n");
	
	return;
}


/*Three meta-objects constructors*/

void __cache_constructor(void *buf, size_t size)
{
	struct kmem_cache *tmp=(struct kmem_cache *)buf;

     tmp->name="";
	tmp->prev_color=0;
     tmp->size=0;
     tmp->obj_size=0;
     tmp->nobjs=0;
	tmp->slabsize=0;

     tmp->obj_type=-1;
     tmp->align=0;

     tmp->freeslabs=NULL;
     tmp->partialslabs=NULL;
     tmp->fullslabs=NULL;
     tmp->constructor=NULL;
     tmp->destructor=NULL;

	return;
}

void __slab_constructor(void *buf, size_t size)
{
	struct kmem_slab *tmp=(struct kmem_slab *)buf;

     tmp->refcount=0;
     tmp->color=0;
     tmp->slab_addr=NULL;
     tmp->head=NULL;
     tmp->used=NULL;
     tmp->parent=NULL;
     tmp->prev=NULL;
     tmp->next=NULL;

	return;
}

void __bufctl_constructor(void *buf, size_t size)
{
	struct kmem_bufctl *tmp=(struct kmem_bufctl *)buf;

	tmp->buf=NULL;
	tmp->next=NULL;
  	tmp->parent=NULL;

	return;
}


/*Three meta-objects destructors*/
void __cache_destructor(void *buf, size_t size)
{
     struct kmem_cache *tmp=(struct kmem_cache *)buf;

     tmp->name="";
     tmp->prev_color=0;
     tmp->size=0;
     tmp->obj_size=0;
     tmp->nobjs=0;
     tmp->slabsize=0;

     tmp->obj_type=-1;
     tmp->align=0;

     tmp->freeslabs=NULL;
     tmp->partialslabs=NULL;
     tmp->fullslabs=NULL;
     tmp->constructor=NULL;
     tmp->destructor=NULL;

	return;
}

void __slab_destructor(void *buf, size_t size)
{
     struct kmem_slab *tmp=(struct kmem_slab *)buf;

     tmp->refcount=0;
     tmp->color=0;
     tmp->slab_addr=NULL;
     tmp->head=NULL;
     tmp->used=NULL;
     tmp->parent=NULL;
     tmp->prev=NULL;
     tmp->next=NULL;

	return;
}
void __bufctl_destructor(void *buf, size_t size)
{
     struct kmem_bufctl *tmp=(struct kmem_bufctl *)buf;

     tmp->buf=NULL;
     tmp->next=NULL;
     tmp->parent=NULL;
}


int __det_objtype(size_t size)
{
	int obj_type=-1;
	if(size < PAGESIZE/8)
		obj_type=0;
	else if (size>=HUGE_OBJ_SIZE)
		obj_type=2;
	else
		obj_type=1;

	return obj_type;
}


void __slab_calc(size_t  objsize, int *nobjs, \
					int *npgs)
{

	unsigned long npages=1, nobjects=1;
	float total_obj_size=0, memsize=0, perc;

	while(1) {
    		do {
			total_obj_size = (nobjects * objsize);
			memsize = (npages * PAGESIZE);

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

	return;
}
