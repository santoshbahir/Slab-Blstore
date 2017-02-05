
/***The implementation of slabbuild.h***/

#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include "dslab.h"
#include "dlist.h"
#include "dslabdefine.h"
#include "dslabbuild.h"
#include "debugmsg.h"

#include "bllbarw.h"
#include "blockstore.h"

/******************************************************************************
 * * This three are baap structures residing outside of slab and their
 * correspond-*
 * * -ing pointers.
 * *
 * *******************************************************************************/ 

void __init(bool flbaBuild)
{
	/* Straight away write in the global array data for 3 structurey */
	/* But if the commit is going into picture then we should read the data
	 * first lba.
	 */
 
	vptr flba;
	PRINTNL;
	PDEBUG_DSLABBUILD("Entered\n"); 
	
		/*Initialize the values first three virtual pointers*/
		size_t cc_size=sizeof(struct dmem_cache);

	LBA(cc_vptr.lba)=1; cc_vptr.index=0;
	LBA(cs_vptr.lba)=1;	cs_vptr.index=cc_size;
	LBA(cb_vptr.lba)=1;	cb_vptr.index=2*cc_size;

	PRINTA_DSLABBUILD(cc_vptr);
	PRINTA_DSLABBUILD(cs_vptr);
	PRINTA_DSLABBUILD(cb_vptr);

	if(flbaBuild)
	{
		size_t i;

		null_init();


		struct dmem_cache *cc_ptr=(struct dmem_cache *)__getmem(cc_size);

		/*Hard coding hihawhawhaw*/
		/*Intialize cache object for cache of caches */
		strcpy(cc_ptr->name, "C-CACHE");
		cc_ptr->size=sizeof(struct dmem_cache);
		cc_ptr->obj_size=sizeof(struct dmem_cache);
		cc_ptr->align=1;

		PDEBUG_DSLABBUILD("EFFECTIVE_SIZE:%d\n",EFFECTIVE_SIZE((cc_ptr->obj_size+SMALL_BUFCTL_SIZE),cc_ptr->align));
		__slab_calc(EFFECTIVE_SIZE((cc_ptr->obj_size+SMALL_BUFCTL_SIZE),cc_ptr->align),
				&(cc_ptr->nobjs), &(cc_ptr->slabsize));
		cc_ptr->obj_type=__det_objtype(cc_ptr->size+SMALL_BUFCTL_SIZE);	

		cc_ptr->freeslabs=vsnull;
		cc_ptr->partialslabs=vsnull;
		cc_ptr->fullslabs=vsnull;
		cc_ptr->constructor=&__cache_constructor;
		cc_ptr->destructor=&__cache_destructor;

		/*Copying the cache of caches at the first location on First LBA*/	
		for(i=0; i<cc_size; i++)
		{
			*(firstlba+i)=*((unsigned char *)cc_ptr+i);
		}

		free(cc_ptr);

		/*Intialize cache object for cache of slabs */
		struct dmem_cache *cs_ptr=(struct dmem_cache *)__getmem(cc_size);

	     strcpy(cs_ptr->name,"C-SLAB");
     	cs_ptr->size=sizeof(struct dmem_slab);
		cs_ptr->obj_size=sizeof(struct dmem_slab);
     	cs_ptr->align=0;

		__slab_calc(EFFECTIVE_SIZE((cs_ptr->obj_size+SMALL_BUFCTL_SIZE),cs_ptr->align),
    	           &(cs_ptr->nobjs), &(cs_ptr->slabsize));
		PDEBUG_DSLABBUILD("slabcache_size:%d\n",cs_ptr->obj_size); 
		PDEBUG_DSLABBUILD("EFFECTIVE_SIZE:%d\n",EFFECTIVE_SIZE((cs_ptr->obj_size+SMALL_BUFCTL_SIZE),cs_ptr->align));
		PDEBUG_DSLABBUILD("nobjs:%d\tslabsize:%d\n",cs_ptr->nobjs, cs_ptr->slabsize); 
   		cs_ptr->obj_type=__det_objtype(cs_ptr->size+SMALL_BUFCTL_SIZE);

	     cs_ptr->freeslabs=vsnull;
    		cs_ptr->partialslabs=vsnull;
	     cs_ptr->fullslabs=vsnull;

     	cs_ptr->constructor=&__slab_constructor;
	     cs_ptr->destructor=&__slab_destructor;

     	/*Copying the cache of slabs at the second location on First LBA*/
	     for(i=cc_size; i<(2*cc_size); i++)
	     {
    		     *(firstlba+i)=*((unsigned char *)cs_ptr+i-cc_size);
	     }

     	free(cs_ptr);


		/*Intialize cache object for cache of bufctl */
		struct dmem_cache *cb_ptr=(struct dmem_cache *)__getmem(cc_size);

	     strcpy(cb_ptr->name,"C-BUFCT");
	     cb_ptr->size=sizeof(struct dmem_bufctl);
     	cb_ptr->obj_size=sizeof(struct dmem_bufctl);
	     cb_ptr->align=1;

		__slab_calc(EFFECTIVE_SIZE((cb_ptr->obj_size+SMALL_BUFCTL_SIZE),cb_ptr->align),
          	     &(cb_ptr->nobjs), &(cb_ptr->slabsize));
	     cb_ptr->obj_type=__det_objtype(cb_ptr->size+SMALL_BUFCTL_SIZE);

     	cb_ptr->freeslabs=vsnull;
	     cb_ptr->partialslabs=vsnull;
     	cb_ptr->fullslabs=vsnull;

	     cb_ptr->constructor=&__bufctl_constructor;
     	cb_ptr->destructor=&__bufctl_destructor;

	     /*Copying the cache of bufctls at the third location on First LBA*/
     	for(i=(2*cc_size); i<(3*cc_size); i++)
	     {
     	     *(firstlba+i)=*((unsigned char *)cb_ptr+i-(2*cc_size));
	     }

     	free(cb_ptr);

		/*Storing this on the blockstore at 1st lba-START*/
		result_t result;
		lba_t first_pLBA;


		fprintf(stdout, "Trying to acquire Father LBA... ");
		result = bl_allocLBA(slabtrans, 1, &first_pLBA);

		if (result != result_OK) {
			fprintf(stdout, "FAILED\n");
			fprintf(stderr, "Result aquiring 1 lbas: %s\n", result_name(result));
			exit(1);
		}
		else
			fprintf(stdout, "OK\n");

		flba.lba=first_pLBA; flba.index=0;

//		store(flba, BLK_SZ, (unsigned char *)firstlba);
		result=bl_write(slabtrans, first_pLBA, firstlba);

		if(result != result_OK)
		{
			printf("First block is not stored successfully\n");
			exit(0);
		}

		/*Storing this on the blockstore at 1st lba-END*/
	}
	else
	{
		LBA(flba.lba)=1;	flba.index=0;
		load(flba, BLK_SZ, (unsigned char *)firstlba);
		PRINTA_DSLABBUILD(flba);
		struct dmem_cache *ccp=(struct dmem_cache *)malloc(CACHE_LEN);

		ccp=(struct dmem_cache *)firstlba;
		PDEBUG_DSLABBUILD("ccp->slabsize:%d\n", ccp->slabsize); 
	}

	PDEBUG_DSLABBUILD("Exiting\n"); 
	PRINTNL;
	return;
	
}


void __getslab_os(vptr_cache cache_vptr)
{
	struct dmem_cache *cp;
	struct dmem_slab *sp;
	
	vptr_slab	sp_v;

	vptr tmp_addr;	

	PRINTNL;
	PDEBUG_DSLABBUILD("Entered\n"); 
	PDEBUG_DSLABBUILD("cache_vptr:"); PRINTA_DSLABBUILD(cache_vptr);

	cp=(struct dmem_cache *)__getmem(CACHE_LEN);
	sp=(struct dmem_slab *)__getmem(SLAB_LEN);

	loadc(cache_vptr, CACHE_LEN, (unsigned char*)cp);	
		
	tmp_addr=__get_rawslab(cp->slabsize);
/*	slab_header=(struct kmem_slab *)((tmp_addr + \
				(cache_ptr->slabsize*PAGESIZE)) - \
				sizeof(struct kmem_slab));*/
	sp->slab_addr=tmp_addr;
	sp->head=vbnull; 
	sp->refcount=0;	
	sp->color=0; 
	sp->used=vbnull; 
	sp->parent=cache_vptr;
	
	/* Storing the slab_header at the end of slab; So calculation staring 
 	 * location for slab_header */
	size_t offset=(cp->slabsize*PAGESIZE)-SLAB_LEN;
	PDEBUG_DSLABBUILD("offset:%d\n", offset);
	PDEBUG_DSLABBUILD("cp->slabsize:%d\n", cp->slabsize);
	PDEBUG_DSLABBUILD("PAGESIZE:%d\n", PAGESIZE);
	PDEBUG_DSLABBUILD("SLAB_LEN:%d\n", SLAB_LEN);

	PDEBUG_DSLABBUILD("tmp_addr="); PRINTA_DSLABBUILD(tmp_addr);
	inc(&tmp_addr,offset);
	PDEBUG_DSLABBUILD("tmp_addr="); PRINTA_DSLABBUILD(tmp_addr);
	CAST_SLAB(sp_v, tmp_addr);
	PDEBUG_DSLABBUILD("sp_v="); PRINTA_DSLABBUILD(sp_v);
	
	stores(sp_v, SLAB_LEN, (unsigned char *)sp);
	PDEBUG_DSLABBUILD("sp->parent="); PRINTA_DSLABBUILD(sp->parent);
	cp->freeslabs=add_slab(cp->freeslabs,sp_v);
     PDEBUG_DSLABBUILD("cp->freeslabs:");
     PRINTA_DSLABBUILD(cp->freeslabs);  

	storec(cache_vptr, CACHE_LEN, (unsigned char *)cp);

	PDEBUG_DSLABBUILD("slab_header->head="); PRINTA_DSLABBUILD(sp->head);
	__buildbufctl_os(sp_v);

	loadc(cache_vptr, CACHE_LEN, (unsigned char *)cp);
     PDEBUG_DSLABBUILD("cp->freeslabs:");
     PRINTA_DSLABBUILD(cp->freeslabs);  

	/*FREE TIME*/	
	free(cp);
	free(sp);
	PDEBUG_DSLABBUILD("Exiting\n"); 
	PRINTNL;
	return;
}

void __getslab_olh(vptr_cache cache_vptr)
{
     struct dmem_cache *cp;
     struct dmem_slab *sp;
    
     vptr_slab sp_v;
     vptr tmp_addr; 

	PRINTNL;
	PDEBUG_DSLABBUILD("Entered\n"); 

     cp=(struct dmem_cache *)__getmem(CACHE_LEN);
     sp=(struct dmem_slab *)__getmem(SLAB_LEN);

     loadc(cache_vptr, CACHE_LEN, (unsigned char *)cp);

	tmp_addr=dmem_cache_alloc(cs_vptr);
	CAST_SLAB(sp_v, tmp_addr);

	sp->slab_addr=__get_rawslab(cp->slabsize);

	if(MAX_COLOR(cp))
		sp->color=(cp->prev_color + cp->align)%(MAX_COLOR(cp));
	else
		sp->color=0;

	sp->head=vbnull; 
	sp->refcount=0;	
	sp->color=0; 
	sp->used=vbnull; 
	sp->parent=cache_vptr;

	stores(sp_v, SLAB_LEN, (unsigned char *)sp);
	PDEBUG_DSLABBUILD("slab_header->head="); PRINTA_DSLABBUILD(sp->head);
	
	/*Calling function to build bufctls for large/huge objects*/
	if(cp->obj_type==OBJ_LARGE)
		__buildbufctl_ol(sp_v);
	else
		__buildbufctl_oh(sp_v);

	PDEBUG_DSLABBUILD("slab_header->head="); PRINTA_DSLABBUILD(sp->head);

	cp->freeslabs=add_slab(cp->freeslabs,sp_v);
	storec(cache_vptr, SLAB_LEN, (unsigned char *)cp);

	PDEBUG_DSLABBUILD("Exiting\n"); 
	PRINTNL;
	return;
}

void __buildbufctl_os(vptr_slab slab_vptr)
{
     size_t i, buf_offset, bufctl_offset;
//     struct kmem_bufctl *tmp_bufctl, **bufctl_addr;
     //void *tmp_buf;
     struct dmem_cache *cp;
     struct dmem_slab *sp;

	vptr tmp_buf, incaddr;
     vptr_bufctl tmp_bufctl, bufctl_addr, bufctl_val;

	PRINTNL;
	PDEBUG_DSLABBUILD("Entered\n");
	
     cp=(struct dmem_cache *)__getmem(CACHE_LEN);
     sp=(struct dmem_slab *)__getmem(SLAB_LEN);

     loads(slab_vptr, SLAB_LEN, (unsigned char *)sp);
     loadc(sp->parent, CACHE_LEN, (unsigned char *)cp);

	PDEBUG_DSLABBUILD("sp->parent:"); PRINTA_DSLABBUILD(sp->parent);
     //tmp_buf=slab_ptr->slab_addr;
     tmp_buf=sp->slab_addr;

     PDEBUG_DSLABBUILD("BUilding bufctl for small objects\n");
	CAST_BUFCTL(tmp_bufctl,tmp_buf);
     sp->head=tmp_bufctl;
	PDEBUG_DSLABBUILD("sp->head:");	PRINTA_DSLABBUILD(sp->head);
     buf_offset=cp->size+SMALL_BUFCTL_SIZE;

     PDEBUG_DSLABBUILD("Total objects(nobjs):%d\n",cp->nobjs);
     for(i=0; i<(cp->nobjs); i++)
     {
		incaddr=sp->slab_addr;
		bufctl_offset=(cp->size*(i+1))+(i*SMALL_BUFCTL_SIZE);
		inc(&incaddr,bufctl_offset);
		CAST_BUFCTL(bufctl_addr,incaddr);
		
       //   cp->constructor(tmp_buf, cp->obj_size); /****PANIC****/
          PDEBUG_DSLABBUILD("cache name:%s\t%dth object\t buf= ",cp->name,i);
		PRINTA_DSLABBUILD(tmp_buf);

          if(i==((cp->nobjs)-1))
		{
			LBA(bufctl_val.lba)=0;
			bufctl_val.index=0;
               storeb(bufctl_addr,sizeof(bufctl_val), (unsigned char *)(&bufctl_val));
		}
          else
          {
			PDEBUG_DSLABBUILD("B4 INC:");	PRINTA_DSLABBUILD(tmp_buf);
               inc(&tmp_buf, buf_offset);
			PDEBUG_DSLABBUILD("After INC:");	PRINTA_DSLABBUILD(tmp_buf);
			CAST_BUFCTL(bufctl_val, tmp_buf);
			storeb(bufctl_addr,sizeof(bufctl_val),(unsigned char *)(&bufctl_val));
          }
     }

	PDEBUG_DSLABBUILD("sp->head:");	PRINTA_DSLABBUILD(sp->head);
	stores(slab_vptr, SLAB_LEN, (unsigned char *)sp);

	free(sp);
	free(cp);
     PDEBUG_DSLABBUILD("Exiting\n");
	PRINTNL;
     return;
}

void __buildbufctl_ol(vptr_slab slab_vptr)
{
     size_t i, buf_offset;
//     struct kmem_bufctl *tmp_bufctl, **bufctl_addr;
     //void *tmp_buf;

     struct dmem_cache *cp;
     struct dmem_slab *sp;
	struct dmem_bufctl *bp;

     vptr tmp_buf, vaddr;
     vptr_bufctl tmp_bufctl;

	PRINTNL;
     PDEBUG_DSLABBUILD("Entered\n");

     cp=(struct dmem_cache *)__getmem(CACHE_LEN);
     sp=(struct dmem_slab *)__getmem(SLAB_LEN);
     bp=(struct dmem_bufctl *)__getmem(BUFCTL_LEN);

     loads(slab_vptr, SLAB_LEN, (unsigned char *)sp);
     loadc(sp->parent, CACHE_LEN, (unsigned char *)cp);

     //tmp_buf=slab_ptr->slab_addr;
     tmp_buf=sp->slab_addr;

     PDEBUG_DSLABBUILD("BUilding bufctl for large objects cache:%s\n",cp->name);
     PDEBUG_DSLABBUILD("nobjs:%d\n",cp->nobjs);

     for(i=0; i<(cp->nobjs); i++)
     {
		buf_offset=i*(cp->size);
          tmp_buf=sp->slab_addr;
		inc(&tmp_buf, buf_offset);
		
//          cp->constructor(tmp_buf, slab_ptr->parent->obj_size);/***PANIC***/

          PDEBUG_DSLABBUILD("Only place where bufctl objects allocated\n");
		vaddr=dmem_cache_alloc(cb_vptr);
          CAST_BUFCTL(tmp_bufctl,vaddr);
          bp->buf=tmp_buf;
          bp->parent=slab_vptr;

          PDEBUG_DSLABBUILD("parent_cache="); 		PRINTA_DSLABBUILD(sp->parent);
          PDEBUG_DSLABBUILD("current_slab="); 		PRINTA_DSLABBUILD(slab_vptr);
          PDEBUG_DSLABBUILD("bn=%d\ttmp_bufctl=",i);	PRINTA_DSLABBUILD(tmp_bufctl);
          PDEBUG_DSLABBUILD("obj_size=%d\n", cp->obj_size);
          PDEBUG_DSLABBUILD("tmp_bufctl->buf=");		PRINTA_DSLABBUILD(bp->buf);
          PDEBUG_DSLABBUILD("tmp_bufctl->next=");		PRINTA_DSLABBUILD(bp->next);

		storeb(tmp_bufctl, BUFCTL_LEN, (unsigned char *)bp);

          sp->head=add_dmem_bufctl(sp->head,tmp_bufctl);

          PDEBUG_DSLABBUILD("tmp_bufctl->next=");			PRINTA_DSLABBUILD(bp->next);
          PDEBUG_DSLABBUILD("bn=%d\tslab_ptr->head=\n",i);	PRINTA_DSLABBUILD(sp->next);
     }

	stores(slab_vptr, SLAB_LEN, (unsigned char *)sp);

	free(bp);
	free(sp);
	free(cp);
	PDEBUG_DSLABBUILD("Exiting\n");
	PRINTNL;
     return;
}

void __buildbufctl_oh(vptr_slab slab_vptr)
{
//	slab_ptr->parent->constructor(slab_ptr->slab_addr, slab_ptr->parent->obj_size);	
	PRINTNL;
	PDEBUG_DSLABBUILD("Enetered\n"); 
	PDEBUG_DSLABBUILD("is it failing here?\n");
	PDEBUG_DSLABBUILD("Exiting\n"); 
	PRINTNL;
	return;
}


vptr __get_rawslab(int slabsize)
{
	vptr slab_addr;
	result_t alloc_result;
	lba_t pLBA;

	PRINTNL;
	PDEBUG_DSLABBUILD("Entered\n"); 

	alloc_result=bl_allocLBA(slabtrans, slabsize, &pLBA);

	slab_addr.lba=pLBA;
	slab_addr.index=0;

	PRINTA_DSLABBUILD(slab_addr);	
	PDEBUG_DSLABBUILD("Exiting\n"); 
	PRINTNL;
	return slab_addr;	
}


void freeslab_os(vptr_slab slab_vptr)
{
     int i, buf_offset;

     struct dmem_cache *cp;
     struct dmem_slab *sp;

     vptr tmp_buf;

	PRINTNL;
     PDEBUG_DSLABBUILD("freeing objects for small objects\n");

     cp=(struct dmem_cache *)__getmem(CACHE_LEN);
     sp=(struct dmem_slab *)__getmem(SLAB_LEN);

     loads(slab_vptr, SLAB_LEN, (unsigned char *)sp);
     loadc(sp->parent, CACHE_LEN, (unsigned char *)cp);

     buf_offset=(cp->size)+SMALL_BUFCTL_SIZE;
     tmp_buf=sp->slab_addr;

     PDEBUG_DSLABBUILD("Total objects(nobjs):%d\n",cp->nobjs);
     for(i=0; i<(cp->nobjs); i++)
     {
//          cp->destructor(tmp_buf, slab_ptr->parent->obj_size);

          PDEBUG_DSLABBUILD("cache name:%s\t%dth object\t buf= ",cp->name,i);
          PRINTA_DSLABBUILD(tmp_buf);
          PDEBUG_DSLABBUILD("\n");


          if(i!=((cp->nobjs)-1))
          {
               inc(&tmp_buf, buf_offset);
          }
     }

	free(sp);
	free(cp);
	PRINTNL;
	return;
}


void freeslab_ol(vptr_slab slab_vptr)
{
     struct dmem_cache *cp;
     struct dmem_slab *sp;
     struct dmem_bufctl *bp;

//     vptr tmp_buf;
     vptr_bufctl tmp_bufctl, free_bufctl;

	PRINTNL;
     PDEBUG_DSLABBUILD("freeing slab for large objects\n");

     cp=(struct dmem_cache *)__getmem(CACHE_LEN);
     sp=(struct dmem_slab *)__getmem(SLAB_LEN);
     bp=(struct dmem_bufctl *)__getmem(BUFCTL_LEN);

     loads(slab_vptr, SLAB_LEN, (unsigned char *)sp);
     loadc(sp->parent, CACHE_LEN, (unsigned char *)cp);

     tmp_bufctl=sp->head;
     loadb(tmp_bufctl, BUFCTL_LEN, (unsigned char *)bp);
     do{
//          slab_ptr->parent->destructor(tmp_bufctl->buf, slab_ptr->parent->obj_size);
          free_bufctl=tmp_bufctl;
          tmp_bufctl=bp->next;
//          kmem_cache_free(cb_ptr, free_bufctl);
     	loadb(tmp_bufctl, BUFCTL_LEN, (unsigned char *)bp);
     }while(NOTEQUAL(tmp_bufctl,vbnull));

     __free_rawslab(sp->slab_addr, cp->slabsize);
//     kmem_cache_free(cs_ptr, slab_ptr);

	free(cp);
	free(sp);
	free(bp);

	PRINTNL;
	return;
}


void freeslab_oh(vptr_slab slab_vptr)
{
     struct dmem_cache *cp=(struct dmem_cache *)__getmem(CACHE_LEN);
     struct dmem_slab *sp=(struct dmem_slab *)__getmem(SLAB_LEN);

     loads(slab_vptr, SLAB_LEN, (unsigned char *)sp);
     loadc(sp->parent, CACHE_LEN, (unsigned char *)cp);

	PRINTNL;
	PDEBUG_DSLABBUILD("freeing slab for huge objects\n");
//     slab_ptr->parent->destructor(slab_ptr->slab_addr, slab_ptr->parent->obj_size);

     __free_rawslab(sp->slab_addr, cp->slabsize);
//     kmem_cache_free(cs_ptr, slab_ptr);

	free(sp);
	free(cp);
	PDEBUG_DSLABBUILD("Exiting\n");
	PRINTNL;
	return;	
}


void __free_rawslab(vptr slab_addr, int slabsize)
{
	int i;
	lba_t tmp;
	result_t result_drop;
	
	PRINTNL;
	tmp=slab_addr.lba;

	for(i=0; i<slabsize; i++)
	{
		result_drop=bl_drop(slabtrans, tmp);
		if(result_drop!= result_OK)
		{
			printf("Panic:failed to drop\n");
			exit(0);
		}
		LBA(tmp)=LBA(tmp)+1;
	}
	
	PRINTNL;
	return;
}


/*Three meta-objects constructors*/

void __cache_constructor(void *buf, size_t size)
{
	struct dmem_cache *tmp=(struct dmem_cache *)buf;

     strcpy(tmp->name,"CACHE");
	tmp->prev_color=0;
     tmp->size=0;
     tmp->obj_size=0;
     tmp->nobjs=0;
	tmp->slabsize=0;

     tmp->obj_type=-1;
     tmp->align=0;

     tmp->freeslabs=vsnull;
     tmp->partialslabs=vsnull;
     tmp->fullslabs=vsnull;
     tmp->constructor=NULL;
     tmp->destructor=NULL;

	return;
}

void __slab_constructor(void *buf, size_t size)
{
	struct dmem_slab *tmp=(struct dmem_slab *)buf;

     tmp->refcount=0;
     tmp->color=0;
     tmp->slab_addr=vnull;
     tmp->head=vbnull;
     tmp->used=vbnull;
     tmp->parent=vcnull;
     tmp->prev=vsnull;
     tmp->next=vsnull;

	return;
}

void __bufctl_constructor(void *buf, size_t size)
{
	struct dmem_bufctl *tmp=(struct dmem_bufctl *)buf;

	tmp->buf=vnull;
	tmp->next=vbnull;
  	tmp->parent=vsnull;

	return;
}


/*Three meta-objects destructors*/
void __cache_destructor(void *buf, size_t size)
{
     struct dmem_cache *tmp=(struct dmem_cache *)buf;

     strcpy(tmp->name,"");
     tmp->prev_color=0;
     tmp->size=0;
     tmp->obj_size=0;
     tmp->nobjs=0;
     tmp->slabsize=0;

     tmp->obj_type=-1;
     tmp->align=0;

     tmp->freeslabs=vsnull;
     tmp->partialslabs=vsnull;
     tmp->fullslabs=vsnull;
     tmp->constructor=NULL;
     tmp->destructor=NULL;

	return;
}

void __slab_destructor(void *buf, size_t size)
{
     struct dmem_slab *tmp=(struct dmem_slab *)buf;

     tmp->refcount=0;
     tmp->color=0;
     tmp->slab_addr=vnull;
     tmp->head=vbnull;
     tmp->used=vbnull;
     tmp->parent=vcnull;
     tmp->prev=vsnull;
     tmp->next=vsnull;

	return;
}
void __bufctl_destructor(void *buf, size_t size)
{
     struct dmem_bufctl *tmp=(struct dmem_bufctl *)buf;

     tmp->buf=vnull;
     tmp->next=vbnull;
     tmp->parent=vsnull;
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

/*
 * This is the only place from where memory from heap should be taken.	
 * This memory is used for the local purposes only. So be careful while
 * passing the memory ref across the functions as you may actually want to
 * pass the virtual pointer
 */
void  *__getmem(size_t size)
{
	void *tmp;
	tmp=malloc(size);
	return tmp;	
}
