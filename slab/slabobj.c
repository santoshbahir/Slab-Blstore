#include <stdlib.h>
#include "slabobj.h"
#include "list.h"
#include "slabmsg.h"
#include "slab.h"

void *
kslab_alloc(struct kmem_cache *cp)
{
	void *buf=NULL;
	struct kmem_bufctl *tmp_bufctl=NULL;
	struct kmem_slab *tmp_slab=NULL;
	
	PDEBUG_SLABOBJ("Entered:cp=%lu\n",(unsigned long)cp);
	if(cp->obj_type == OBJ_SMALL)
	{
		if(cp->partialslabs!=NULL)
		{
			PDEBUG_SLABOBJ("From Partialslab:%lu\n",\
						(unsigned long)cp->partialslabs);

			PDEBUG_SLABOBJ("From Partialslab:1--->\n");
			PDEBUG_SLABOBJ("CP->nobjs:%d\tcp->partialslabs:%lu\n",cp->nobjs,(unsigned long)cp->partialslabs);
			PDEBUG_SLABOBJ("CP->nobjs:%d\trefcount:%d\n",cp->nobjs,cp->partialslabs->refcount);
			buf=(void *)cp->partialslabs->head;
			PDEBUG_SLABOBJ("From Partialslab:2--->%lu\n",(unsigned long)buf);
			cp->partialslabs->head=*((struct kmem_bufctl **)((void *)\
								(cp->partialslabs->head)+cp->size));
			PDEBUG_SLABOBJ("From Partialslab:3--->\n");
			
			cp->partialslabs->refcount=cp->partialslabs->refcount+1;
	
			if(cp->partialslabs->refcount == cp->nobjs)
			{
				tmp_slab=cp->partialslabs;
				cp->partialslabs=rm_slab(cp->partialslabs,cp->partialslabs);
				cp->fullslabs=add_slab(cp->fullslabs,tmp_slab);
			}
		}
		else
		{
			PDEBUG_SLABOBJ("FROM freeslabs:%lu\n",\
						(unsigned long)cp->freeslabs);

			buf=(void *)cp->freeslabs->head;
			cp->freeslabs->head=*((struct kmem_bufctl **)((void *)\
								(cp->freeslabs->head)+cp->size));

			cp->freeslabs->refcount=cp->freeslabs->refcount+1;

			tmp_slab=cp->freeslabs;
			cp->freeslabs=rm_slab(cp->freeslabs, cp->freeslabs);
			cp->partialslabs=add_slab(cp->partialslabs,tmp_slab);
			PDEBUG_SLABOBJ("Removed from freeslabs:%lu\n",\
						(unsigned long)cp->freeslabs);
		}
		
	}
	else if(cp->obj_type==OBJ_LARGE)
	{
		if(!slabListEmpty(cp->partialslabs))
		{
			PDEBUG_SLABOBJ("CACHE:%s\tobj_size:%d\n",cp->name,cp->obj_size);
			PDEBUG_SLABOBJ("Allocating large object from partailslabs:%lu\n",\
						(unsigned long)cp->partialslabs->head);
			buf=cp->partialslabs->head->buf;

			tmp_bufctl=cp->partialslabs->head;

			cp->partialslabs->head=rm_kmem_bufctl(cp->partialslabs->head,\
								cp->partialslabs->head);
			cp->partialslabs->used=add_kmem_bufctl(cp->partialslabs->used,\
								tmp_bufctl);

			cp->partialslabs->refcount=cp->partialslabs->refcount+1;
	
			PDEBUG_SLABOBJ("cp->partialslabs->refcount:%d\tcp->nobjs=%d\n",\
							cp->partialslabs->refcount,cp->nobjs);
			tmp_slab=cp->partialslabs;
			if(cp->partialslabs->refcount == cp->nobjs)
			{
				cp->partialslabs=rm_slab(cp->partialslabs,cp->partialslabs);
				cp->fullslabs=add_slab(cp->fullslabs,tmp_slab);
			}
		}
		else
		{
			PDEBUG_SLABOBJ("CACHE:%s\tobj_size:%d\n",cp->name,cp->obj_size);
			PDEBUG_SLABOBJ("Allocating large object from freeslabs\n");
			buf=cp->freeslabs->head->buf;
			
			tmp_bufctl=cp->freeslabs->head;
			
			cp->freeslabs->head=rm_kmem_bufctl(cp->freeslabs->head,\
								cp->freeslabs->head);
			cp->freeslabs->used=add_kmem_bufctl(cp->freeslabs->used,\
								tmp_bufctl);

			PDEBUG_SLABOBJ("cp->freeslabs->used:%lu\n",(unsigned long)cp->freeslabs->used);
			PDEBUG_SLABOBJ("cp->freeslabs->head:%lu\n",(unsigned long)cp->freeslabs->head);

			cp->freeslabs->refcount=cp->freeslabs->refcount+1;
	
			PDEBUG_SLABOBJ("cp->freeslabs->refcount:%d\tcp->nobjs=%d\n",\
							cp->freeslabs->refcount,cp->nobjs);
			if(cp->freeslabs->refcount == cp->nobjs)
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
		}
		PDEBUG_SLABOBJ("Exiting:cp->size:%d\tcp->nobjs:%d\ttmp_slab->refcount:%d\n",\
                          cp->obj_size, cp->nobjs, tmp_slab->refcount);
	}
	else
	{
		buf=cp->freeslabs->slab_addr;
		tmp_slab=cp->freeslabs;
		cp->freeslabs=rm_slab(cp->freeslabs, cp->freeslabs);
		cp->fullslabs=add_slab(cp->fullslabs,tmp_slab);
	}
		;	/******PANIC*****/
	PDEBUG_SLABOBJ("Exiting:buf=%lu\n",(unsigned long)buf);
	return buf;
}


void 
kslab_free(struct kmem_cache *cp, void *buf)
{
	struct kmem_bufctl *tmp_head, *found_bufctl, **tmp_bufctl;
	struct kmem_slab *tmp_slab;	

	void	*obj_addr;
	int i;
	bool obj_found=false;

	obj_addr=NULL;
	found_bufctl=NULL;
//	tmp_slab=cp->fullslabs;
	
	PDEBUG_SLABOBJ("Entered:cp=%lu\t%lu\n",(unsigned long)cp,(unsigned long)buf);
	if(cp->obj_type == OBJ_SMALL)
	{
		tmp_slab=cp->fullslabs;
		if(!slabListEmpty(cp->fullslabs))
		{
			obj_found=false;
			PDEBUG_SLABOBJ("Freeing buf from fullslab:buf=%lu\n",(unsigned long)buf);
			do
			{
				obj_addr=tmp_slab->slab_addr;
				
				for(i=0; i<cp->nobjs; i++)
				{
					if(buf == obj_addr)
					{
						PDEBUG_SLABOBJ("Buf found:=%lu\n",(unsigned long)buf);
						obj_found=true;
						//put the buf on the free list
						tmp_bufctl=(struct kmem_bufctl **)(buf+cp->size);
						*tmp_bufctl=tmp_slab->head;
						tmp_slab->head=(struct kmem_bufctl *)buf;

						tmp_slab->refcount=	tmp_slab->refcount - 1;
						//Add this to partialslabs;
						cp->fullslabs=rm_slab(cp->fullslabs,tmp_slab);
						cp->partialslabs=add_slab(cp->partialslabs,tmp_slab);

						break;
					}			
					else
						obj_addr=obj_addr+(cp->size+SMALL_BUFCTL_SIZE);
				}
				if(obj_found)
					break;
				else
					tmp_slab=tmp_slab->next;
			}while (tmp_slab != cp->fullslabs);			
		}
		if(!obj_found)	
		{
			PDEBUG_SLABOBJ("Freeing buf from partialslab:buf=%lu\n",(unsigned long)buf);
			PDEBUG_SLABOBJ("ObJECT SIZE:%d\n",cp->obj_size);
			tmp_slab=cp->partialslabs;
			do
			{
				obj_addr=tmp_slab->slab_addr;

				for(i=0;i<cp->nobjs; i++)
				{
					if(buf == obj_addr)
					{
						PDEBUG_SLABOBJ("Buf found:=%lu\n",(unsigned long)buf);
						obj_found=true;
						tmp_bufctl=(struct kmem_bufctl **)(buf+cp->size);
						*tmp_bufctl=tmp_slab->head;
						tmp_slab->head=(struct kmem_bufctl *)buf;

						tmp_slab->refcount=	tmp_slab->refcount - 1;
						//if refcount 0; add this to freelist;
						if(tmp_slab->refcount == 0)
						{
							cp->partialslabs=rm_slab(cp->partialslabs,tmp_slab);
							cp->freeslabs=add_slab(cp->freeslabs,tmp_slab);
						}	
						break;
					}			
					else
						obj_addr=obj_addr+(cp->size+SMALL_BUFCTL_SIZE);
				}
				if(obj_found)
					break;
				else	
					tmp_slab=tmp_slab->next;
			}while (tmp_slab != cp->partialslabs);			
			
		}

		PDEBUG_SLABOBJ("Exiting:cp->nobjs:%d\ttmp_slab->refcount:%d\n",\
						cp->nobjs, tmp_slab->refcount);
		return;
	}

	else if(cp->obj_type == OBJ_LARGE)
	{

	tmp_slab=cp->fullslabs;
	found_bufctl=NULL;

	PDEBUG_SLABOBJ("This is large object deletion:tmp_slab=%lu\n",(unsigned long)tmp_slab);
	if(tmp_slab != NULL)
	do{
		tmp_head=tmp_slab->used;
		if(tmp_head != NULL)
		do{
			found_bufctl=search_buf(tmp_head,buf);
			if(found_bufctl != NULL)
			{
				PDEBUG_SLABOBJ("Bufctl found:%lu\n",(unsigned long)found_bufctl);
				tmp_slab->used=rm_kmem_bufctl(tmp_slab->used, found_bufctl);
				tmp_slab->head=add_kmem_bufctl(tmp_slab->head, found_bufctl);
				tmp_slab->refcount=tmp_slab->refcount - 1;
			
				cp->fullslabs=rm_slab(cp->fullslabs, tmp_slab);
				if(tmp_slab->refcount==0)
					cp->freeslabs=add_slab(cp->freeslabs, tmp_slab);
				else
					cp->partialslabs=add_slab(cp->partialslabs, tmp_slab);
	
				break;
			}
			tmp_head=tmp_head->next;
		}while(tmp_head!=NULL);
		
		if(found_bufctl!=NULL)
			break;
		else
			tmp_slab=tmp_slab->next;
	}while(tmp_slab!=cp->fullslabs);
	
	if(found_bufctl == NULL)
	{
		tmp_slab=cp->partialslabs;
		do
		{
			PDEBUG_SLABOBJ("Reached here:tmp_slab:%lu\n",(unsigned long)tmp_slab);
			tmp_head=tmp_slab->used;
			do
			{
				found_bufctl=search_buf(tmp_head,buf);
				PDEBUG_SLABOBJ("Reached here0000000?\n");
				if(found_bufctl != NULL)
				{
					tmp_slab->used=rm_kmem_bufctl(tmp_slab->used, found_bufctl);
					tmp_slab->head=add_kmem_bufctl(tmp_slab->head, found_bufctl);
					tmp_slab->refcount=tmp_slab->refcount - 1;
			
					PDEBUG_SLABOBJ("Reached here1111111?\n");
					PDEBUG_SLABOBJ("No of slabs per cache:%d\n",debug_get_nslabs(cp));
					if(tmp_slab->refcount==0)
					{
						PDEBUG_SLABOBJ("Reached here2222222?\n");
						cp->partialslabs=rm_slab(cp->partialslabs, tmp_slab);
						PDEBUG_SLABOBJ("After Removing from partial list:No of slabs per cache:%d\n",debug_get_nslabs(cp));
						cp->freeslabs=add_slab(cp->freeslabs, tmp_slab);
						PDEBUG_SLABOBJ("After Adding on free list:No of slabs per cache:%d\n",debug_get_nslabs(cp));
					}
					PDEBUG_SLABOBJ("No of slabs per cache:%d\n",debug_get_nslabs(cp));
					break;
				}
				tmp_head=tmp_head->next;
			}while(tmp_head!=NULL);
			if(found_bufctl!=NULL)
				break;
			else
				tmp_slab=tmp_slab->next;
		}while(tmp_slab!=cp->partialslabs);
	}

	PDEBUG_SLABOBJ("Exiting:cp->size:%d\tcp->nobjs:%d\ttmp_slab->refcount:%d\n",\
					cp->obj_size, cp->nobjs, tmp_slab->refcount);
	PDEBUG_SLABOBJ("Exiting\n");
	return;
	} /*obj_type = OBJ_LARGE ends here */

     else
     {
		tmp_slab=search_slabaddr(cp->fullslabs, buf);
          cp->fullslabs=rm_slab(cp->fullslabs, tmp_slab);
          cp->freeslabs=add_slab(cp->freeslabs,tmp_slab);
/*		Search the fullslab list for this buffer addrees.
		once found remove from fulllist.
		add to empty list.*/
/*          buf=cp->freeslabs->slab_addr;
          tmp_slab=cp->freeslabs;*/
     }
          ;    /******PANIC*****/

	if(found_bufctl == NULL)
		; /*Something is worng*/

	PDEBUG_SLABOBJ("Exting\n");
	return;
}

