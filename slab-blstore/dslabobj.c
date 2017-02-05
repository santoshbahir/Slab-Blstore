#include <stdlib.h>
#include "dslabobj.h"
#include "dslabbuild.h"
#include "dlist.h"
#include "virtptr.h"
#include "bllbarw.h"
#include "debugmsg.h"
#include "dslab.h"


vptr dslab_alloc_os(vptr_cache vcp)
{
	vptr buf=vnull;
	vptr_bufctl headbuf=vbnull;
	vptr incaddr=vnull;
	vptr_slab tmp_slab=vsnull;
	
	struct dmem_cache *cp=(struct dmem_cache *)__getmem(CACHE_LEN);
	struct dmem_slab *sp=(struct dmem_slab *)__getmem(SLAB_LEN);

	PRINTNL;
	PDEBUG_DSLABOBJ("Enetered:vcp=");
     PRINTA_DSLABOBJ(vcp); PDEBUG_DSLABOBJ("\n");
	loadc(vcp, CACHE_LEN, (unsigned char *)cp);

	if(NOTEQUAL(cp->partialslabs,vsnull))
	{
		PDEBUG_DSLABOBJ("From Partialslab=");
	     PRINTA_DSLABOBJ(cp->partialslabs); PDEBUG_DSLABOBJ("\n");

		loads(cp->partialslabs, SLAB_LEN, (unsigned char *)sp);

		PDEBUG_DSLABOBJ("From Partialslab:1--->\n");
		PDEBUG_DSLABOBJ("CP->nobjs:%d\n",cp->nobjs);
		PDEBUG_DSLABOBJ("refcount:%d\n",sp->refcount);

		CAST_VPTR(buf, sp->head);

		PDEBUG_DSLABOBJ("From Partialslab:2--->\n");
	     PRINTA_DSLABOBJ(buf); PDEBUG_DSLABOBJ("\n");

		CAST_VPTR(incaddr,sp->head);
		inc(&incaddr, cp->size);
		load(incaddr, sizeof(vptr), (unsigned char *)(&headbuf));
		sp->head=headbuf;

		PDEBUG_DSLABOBJ("From Partialslab:3--->\n");
			
		sp->refcount=sp->refcount+1;

		stores(cp->partialslabs, SLAB_LEN, (unsigned char *)sp);

		if(sp->refcount == cp->nobjs)
		{
			tmp_slab=cp->partialslabs;
			cp->partialslabs=rm_slab(cp->partialslabs,cp->partialslabs);
			cp->fullslabs=add_slab(cp->fullslabs,tmp_slab);
		}
		free(sp);
	}
	else
	{
		PDEBUG_DSLABOBJ("From freeslabs=");
	     PRINTA_DSLABOBJ(cp->freeslabs); PDEBUG_DSLABOBJ("\n");

		loads(cp->freeslabs, SLAB_LEN, (unsigned char *)sp);
		CAST_VPTR(buf, sp->head);

		CAST_VPTR(incaddr,sp->head);
		inc(&incaddr, cp->size);
		load(incaddr, sizeof(vptr), (unsigned char *)(&headbuf));
		sp->head=headbuf;

		sp->refcount=sp->refcount+1;

		stores(cp->freeslabs, SLAB_LEN, (unsigned char *)sp);

		tmp_slab=cp->freeslabs;
		cp->freeslabs=rm_slab(cp->freeslabs, cp->freeslabs);
		cp->partialslabs=add_slab(cp->partialslabs,tmp_slab);

		PDEBUG_DSLABOBJ("Removed from freeslabs:2--->\n");
	     PRINTA_DSLABOBJ(cp->freeslabs); PDEBUG_DSLABOBJ("\n");

		free(sp);
	}

	storec(vcp, CACHE_LEN, (unsigned char *)cp);
	free(cp);

	PDEBUG_DSLABOBJ("Exiting:buf=\n");
	PRINTA_DSLABOBJ(buf); PDEBUG_DSLABOBJ("\n");
	PRINTNL;
	return buf;
}


vptr dslab_alloc_ol(vptr_cache vcp)
{
	vptr buf=vnull;
     vptr_bufctl tmp_bufctl=vbnull;
     vptr_slab tmp_slab=vsnull;

     struct dmem_cache *cp=(struct dmem_cache *)__getmem(CACHE_LEN);
     struct dmem_slab *sp=(struct dmem_slab *)__getmem(SLAB_LEN);
     struct dmem_bufctl *bp=(struct dmem_bufctl *)__getmem(BUFCTL_LEN);

	PRINTNL;
     PDEBUG_DSLABOBJ("Enetered:vcp=");
     PRINTA_DSLABOBJ(vcp); PDEBUG_DSLABOBJ("\n");
     loadc(vcp, CACHE_LEN, (unsigned char *)cp);

	if(!slabListEmpty(cp->partialslabs))
	{
		PDEBUG_DSLABOBJ("CACHE:%s\tobj_size:%d\n",cp->name,cp->obj_size);
     	loads(cp->partialslabs, SLAB_LEN, (unsigned char *)sp);
     	loadb(sp->head, BUFCTL_LEN, (unsigned char *)bp);

     	PDEBUG_DSLABOBJ("Allocating large object from partailslabs=");
	     PRINTA_DSLABOBJ(sp->head); PDEBUG_DSLABOBJ("\n");

		buf=bp->buf;

		tmp_bufctl=sp->head;
		sp->head=rm_dmem_bufctl(sp->head, sp->head);
		sp->used=add_dmem_bufctl(sp->used,tmp_bufctl);

		sp->refcount=sp->refcount+1;
	
		PDEBUG_DSLABOBJ("cp->partialslabs->refcount:%d\tcp->nobjs=%d\n",\
						sp->refcount,cp->nobjs);

		stores(cp->partialslabs, SLAB_LEN, (unsigned char *)sp);

		tmp_slab=cp->partialslabs;
		if(sp->refcount == cp->nobjs)
		{
			cp->partialslabs=rm_slab(cp->partialslabs,cp->partialslabs);
			cp->fullslabs=add_slab(cp->fullslabs,tmp_slab);
		}
		free(sp);
		free(bp);
	}
	else
	{
		PDEBUG_DSLABOBJ("CACHE:%s\tobj_size:%d\n",cp->name,cp->obj_size);
     	loads(cp->freeslabs, SLAB_LEN, (unsigned char *)sp);
     	loadb(sp->head, BUFCTL_LEN, (unsigned char *)bp);

		PDEBUG_DSLABOBJ("Allocating large object from freeslabs\n");

		buf=bp->buf;
			
		tmp_bufctl=sp->head;
		sp->head=rm_dmem_bufctl(sp->head, sp->head);
		sp->used=add_dmem_bufctl(sp->used,tmp_bufctl);
			
		PDEBUG_DSLABOBJ("cp->freeslabs->used:");
	     PRINTA_DSLABOBJ(sp->used); PDEBUG_DSLABOBJ("\n");
		PDEBUG_DSLABOBJ("cp->freeslabs->head:");
	     PRINTA_DSLABOBJ(sp->head); PDEBUG_DSLABOBJ("\n");

		sp->refcount=sp->refcount+1;
	
		PDEBUG_DSLABOBJ("cp->freeslabs->refcount:%d\tcp->nobjs=%d\n",\
						sp->refcount,cp->nobjs);

		stores(cp->freeslabs, SLAB_LEN, (unsigned char *)sp);

		if(sp->refcount == cp->nobjs)
		{
			tmp_slab=cp->freeslabs;
			cp->freeslabs=rm_slab(cp->freeslabs, cp->freeslabs);
			cp->fullslabs=add_slab(cp->fullslabs,tmp_slab);
		}
		else
		{
			tmp_slab=cp->freeslabs;
			cp->freeslabs=rm_slab(cp->freeslabs, cp->freeslabs);
			cp->partialslabs=add_slab(cp->partialslabs,tmp_slab);
		}
		free(sp);
		free(bp);
	}

	PDEBUG_DSLABOBJ("Exiting:cp->size:%d\tcp->nobjs:%d\n",\
                         cp->obj_size, cp->nobjs);

	storec(vcp, CACHE_LEN, (unsigned char *)cp);
	free(cp);
	PDEBUG_DSLABOBJ("Exiting:buf=\n");
	PRINTA_DSLABOBJ(buf); PDEBUG_DSLABOBJ("\n");
	PRINTNL;
	return buf;
}

vptr dslab_alloc_oh(vptr_cache vcp)
{
	vptr buf=vnull;
	vptr_slab tmp_slab=vsnull;

     struct dmem_cache *cp=(struct dmem_cache *)__getmem(CACHE_LEN);
     struct dmem_slab *sp=(struct dmem_slab *)__getmem(SLAB_LEN);

	PRINTNL;
     PDEBUG_DSLABOBJ("Enetered:vcp=");
     PRINTA_DSLABOBJ(vcp); PDEBUG_DSLABOBJ("\n");
     loadc(vcp, CACHE_LEN, (unsigned char *)cp);
    	loads(cp->partialslabs, SLAB_LEN, (unsigned char *)sp);

     buf=sp->slab_addr;
     tmp_slab=cp->freeslabs;
     cp->freeslabs=rm_slab(cp->freeslabs, cp->freeslabs);
     cp->fullslabs=add_slab(cp->fullslabs,tmp_slab);

     storec(vcp, CACHE_LEN, (unsigned char *)cp);
	free(cp);
	free(sp);
	
	PDEBUG_DSLABOBJ("Exiting:buf=\n");
	PRINTA_DSLABOBJ(buf); PDEBUG_DSLABOBJ("\n");
	PRINTNL;
	return buf;
}


void dslab_free_os(vptr_cache vcp, vptr buf)
{

	vptr_bufctl found_bufctl, tmp_bufctl;
	vptr_slab tmp_slab;	
	vptr obj_addr,incaddr;
	
	int i;
	bool obj_found=false;

	obj_addr=vnull;
	found_bufctl=vbnull;
	
	PRINTNL;
	PDEBUG_DSLABOBJ("Entered:cp=");
	PRINTA_DSLABOBJ(vcp); PDEBUG_DSLABOBJ("\tbuf=");	PRINTA_DSLABOBJ(buf); 
	PDEBUG_DSLABOBJ("\n");	

     struct dmem_cache *cp=(struct dmem_cache *)__getmem(CACHE_LEN);
     struct dmem_slab *sp=(struct dmem_slab *)__getmem(SLAB_LEN);

     loadc(vcp, CACHE_LEN, (unsigned char *)cp);

	tmp_slab=cp->fullslabs;
	if(!slabListEmpty(cp->fullslabs))
	{
		obj_found=false;
		PDEBUG_DSLABOBJ("Freeing buf from fullslab:buf=");
		PRINTA_DSLABOBJ(buf); PDEBUG_DSLABOBJ("\n");

		do
		{
     		loads(tmp_slab, SLAB_LEN, (unsigned char *)sp);
			obj_addr=sp->slab_addr;
			
			for(i=0; i<cp->nobjs; i++)
			{
				if(EQUAL(buf, obj_addr))
				{
					PDEBUG_DSLABOBJ("Buf found:=");
					PRINTA_DSLABOBJ(buf); PDEBUG_DSLABOBJ("\n");
					obj_found=true;
					//put the buf on the free list
					CAST_VPTR(incaddr, buf);
					inc(&incaddr, cp->size);
//					tmp_bufctl=(struct kmem_bufctl **)(buf+cp->size);
					store(incaddr, sizeof(vptr_bufctl), (unsigned char *)(&(sp->head)));
//					*tmp_bufctl=tmp_slab->head;
					CAST_BUFCTL(tmp_bufctl, buf);	
					sp->head=tmp_bufctl;

					sp->refcount=	sp->refcount - 1;

					stores(tmp_slab, SLAB_LEN, (unsigned char *)sp);
					//Add this to partialslabs;
					cp->fullslabs=rm_slab(cp->fullslabs,tmp_slab);
					cp->partialslabs=add_slab(cp->partialslabs,tmp_slab);

					break;
				}			
				else
					inc(&obj_addr, (cp->size+SMALL_BUFCTL_SIZE));
			}
			if(obj_found)
				break;
			else
				tmp_slab=sp->next;
		}while (NOTEQUAL(tmp_slab, cp->fullslabs));
	}
	if(!obj_found)	
	{
		PDEBUG_DSLABOBJ("Freeing buf from partialslab:buf=");
		PRINTA_DSLABOBJ(buf); PDEBUG_DSLABOBJ("\n");
		PDEBUG_DSLABOBJ("ObJECT SIZE:%d\n",cp->obj_size);
		tmp_slab=cp->partialslabs;
		do
		{
     		loads(tmp_slab, SLAB_LEN, (unsigned char *)sp);
			obj_addr=sp->slab_addr;

			for(i=0;i<cp->nobjs; i++)
			{
				if(EQUAL(buf, obj_addr))
				{
					PDEBUG_DSLABOBJ("Buf found:=");
					PRINTA_DSLABOBJ(buf); PDEBUG_DSLABOBJ("\n");
					obj_found=true;
					//put the buf on the free list
					CAST_VPTR(incaddr, buf);
					inc(&incaddr, cp->size);
//					tmp_bufctl=(struct kmem_bufctl **)(buf+cp->size);
					store(incaddr, sizeof(vptr_bufctl), (unsigned char *)(&(sp->head)));
//					*tmp_bufctl=tmp_slab->head;
					CAST_BUFCTL(tmp_bufctl, buf);	
					sp->head=tmp_bufctl;

					sp->refcount=	sp->refcount - 1;
					stores(tmp_slab, SLAB_LEN, (unsigned char *)sp);
					//if refcount 0; add this to freelist;
					if(sp->refcount == 0)
					{
						cp->partialslabs=rm_slab(cp->partialslabs,tmp_slab);
						cp->freeslabs=add_slab(cp->freeslabs,tmp_slab);
					}	
					break;
				}			
				else
					inc(&obj_addr,(cp->size+SMALL_BUFCTL_SIZE));
			}
			if(obj_found)
				break;
			else	
				tmp_slab=sp->next;
		}while (NOTEQUAL(tmp_slab, cp->partialslabs));
	}

	storec(vcp, CACHE_LEN, (unsigned char *)cp);
	PDEBUG_DSLABOBJ("Exiting:cp->nobjs:%d\ttmp_slab->refcount:%d\n",\
					cp->nobjs, sp->refcount);

	free(sp);
	free(cp);
	PRINTNL;
	return;
}


void dslab_free_ol(vptr_cache vcp, vptr buf)
{

	vptr_bufctl tmp_head, found_bufctl;
	vptr_slab tmp_slab;	

	vptr obj_addr;

	obj_addr=vnull;
	found_bufctl=vbnull;
	
	PRINTNL;
	PDEBUG_DSLABOBJ("Entered:cp=");
	PRINTA_DSLABOBJ(vcp); PDEBUG_DSLABOBJ("\tbuf=");	PRINTA_DSLABOBJ(buf); 
	PDEBUG_DSLABOBJ("\n");	

     struct dmem_cache *cp=(struct dmem_cache *)__getmem(CACHE_LEN);
     struct dmem_slab *sp=(struct dmem_slab *)__getmem(SLAB_LEN);
     struct dmem_bufctl *bp=(struct dmem_bufctl *)__getmem(BUFCTL_LEN);

     loadc(vcp, CACHE_LEN, (unsigned char *)cp);

	tmp_slab=cp->fullslabs;
	found_bufctl=vbnull;

	PDEBUG_DSLABOBJ("This is large object deletion:tmp_slab=");
	PRINTA_DSLABOBJ(tmp_slab);	PDEBUG_DSLABOBJ("\n");	

	if(NOTEQUAL(tmp_slab,vsnull))
	do{
     	loads(tmp_slab, SLAB_LEN, (unsigned char *)sp);
		tmp_head=sp->used;
		if(NOTEQUAL(tmp_head, vbnull))
		{
			do{
				found_bufctl=search_buf(tmp_head,buf);
				if(NOTEQUAL(found_bufctl, vbnull))
				{
					PDEBUG_DSLABOBJ("Bufctl found=");
					PRINTA_DSLABOBJ(found_bufctl);	PDEBUG_DSLABOBJ("\n");	

					sp->used=rm_dmem_bufctl(sp->used, found_bufctl);
					sp->head=add_dmem_bufctl(sp->head, found_bufctl);
					PDEBUG_DSLABOBJ("refcount is:%d\n",sp->refcount);
					sp->refcount=sp->refcount - 1;
			
					cp->fullslabs=rm_slab(cp->fullslabs, tmp_slab);

					stores(tmp_slab, SLAB_LEN, (unsigned char *)sp);
					if(sp->refcount==0)
						cp->freeslabs=add_slab(cp->freeslabs, tmp_slab);
					else
						cp->partialslabs=add_slab(cp->partialslabs, tmp_slab);
					break;
				}
     			loadb(tmp_head, BUFCTL_LEN, (unsigned char *)bp);
				tmp_head=bp->next;
			}while(NOTEQUAL(tmp_head, vbnull));
		}	
		if(NOTEQUAL(found_bufctl, vbnull))
			break;
		else
			tmp_slab=sp->next;
	}while(NOTEQUAL(tmp_slab,cp->fullslabs));
	
	if(EQUAL(found_bufctl,vbnull))
	{
		tmp_slab=cp->partialslabs;
		do
		{
			PDEBUG_DSLABOBJ("Reached here:tmp_slab=");
			PRINTA_DSLABOBJ(tmp_slab);	PDEBUG_DSLABOBJ("\n");	
     		loads(tmp_slab, SLAB_LEN, (unsigned char *)sp);
			tmp_head=sp->used;
			do
			{
				found_bufctl=search_buf(tmp_head,buf);
				PDEBUG_DSLABOBJ("Reached here0000000?\n");
				if(NOTEQUAL(found_bufctl, vbnull))
				{
					sp->used=rm_dmem_bufctl(sp->used, found_bufctl);
					sp->head=add_dmem_bufctl(sp->head, found_bufctl);
					sp->refcount=sp->refcount - 1;
			
					stores(tmp_slab, SLAB_LEN, (unsigned char *)sp);

					PDEBUG_DSLABOBJ("Reached here1111111?\n");
//					PDEBUG_DSLABOBJ("No of slabs per cache:%d\n",debug_get_nslabs(cp));
					if(sp->refcount==0)
					{
						PDEBUG_DSLABOBJ("Reached here2222222?\n");
						cp->partialslabs=rm_slab(cp->partialslabs, tmp_slab);
//						PDEBUG_DSLABOBJ("After Removing from partial list:No of slabs per cache:%d\n",debug_get_nslabs(cp));
						cp->freeslabs=add_slab(cp->freeslabs, tmp_slab);
//						PDEBUG_DSLABOBJ("After Adding on free list:No of slabs per cache:%d\n",debug_get_nslabs(cp));
					}
//					PDEBUG_DSLABOBJ("No of slabs per cache:%d\n",debug_get_nslabs(cp));
					break;
				}
     			loadb(tmp_head, BUFCTL_LEN, (unsigned char *)bp);
				tmp_head=bp->next;
			}while(NOTEQUAL(tmp_head,vbnull));
			if(NOTEQUAL(found_bufctl, vbnull))
				break;
			else
				tmp_slab=sp->next;
		}while(NOTEQUAL(tmp_slab,cp->partialslabs));
	}

	storec(vcp, CACHE_LEN, (unsigned char *)cp);

	PDEBUG_DSLABOBJ("Exiting:cp->size:%d\tcp->nobjs:%d\ttmp_slab->refcount:%d\n",\
					cp->obj_size, cp->nobjs, sp->refcount);

	free(bp);
	free(sp);
	free(cp);

	PDEBUG_DSLABOBJ("Exiting\n");
	PRINTNL;
	return;
}

void dslab_free_oh(vptr_cache vcp, vptr buf)
{
	vptr_slab tmp_slab;
     struct dmem_cache *cp=(struct dmem_cache *)__getmem(CACHE_LEN);

	PRINTNL;
	PDEBUG_DSLABOBJ("Entered\n");
	loadc(vcp, CACHE_LEN, (unsigned char *)cp);
		
	tmp_slab=search_slabaddr(cp->fullslabs, buf);
     cp->fullslabs=rm_slab(cp->fullslabs, tmp_slab);
     cp->freeslabs=add_slab(cp->freeslabs,tmp_slab);
	
	storec(vcp, CACHE_LEN, (unsigned char *)cp);

	free(cp);
	PDEBUG_DSLABOBJ("Exiting\n");
	PRINTNL;
	return;	
}
