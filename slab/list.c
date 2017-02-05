#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h> 
#include <strings.h>
#include <stdbool.h>
#include "list.h"
#include "slabmsg.h"


/*These are the temporary list headers actual list headers ponters will be in  *
 * either slab-header structure or kmem_bufctl structure                      */
struct kmem_bufctl *list_head=NULL;
//struct kmem_slab *list_head=NULL;

/*Doubly Linked list operations for the slab-header structure:STARTS*/
bool slabListEmpty(struct kmem_slab *list_head)
{
	if(NULL==list_head)
		return true;
	else
		return false;
}

struct kmem_slab *search_slab(struct kmem_slab *list_head,\
						struct kmem_slab *slab_addr)
{
	
	struct kmem_slab *tmp_header;
	int i=0;

	PDEBUG_LIST("Entered\n");

	if(slabListEmpty(list_head))
	{
		PDEBUG_LIST("EXITING:Slab header list is empty\n");
		return NULL;
	}
	else
	{
		tmp_header=list_head;
		do{
			PDEBUG_LIST("This is slab number %d\n",i);	

			if(tmp_header==slab_addr)
			{
				PDEBUG_LIST("EXITING:SLAB FOUND:=%d\n",i);
				return tmp_header;
			}
			tmp_header=tmp_header->next;
			i++;
		}while(tmp_header!=list_head);
	}

	PDEBUG_LIST("EXITING:SLAB DID NOT FIND=%lu\n",(unsigned long)slab_addr);	
	return NULL;
}

struct kmem_slab *add_slab(struct kmem_slab *list_head, \
					struct kmem_slab *slab)
{
     struct kmem_slab *tmp_header;

	PDEBUG_LIST("Entered\n");

     tmp_header=list_head;
     if(slabListEmpty(list_head))
     {
		PDEBUG_LIST("First slab header to be added\n");

          slab->next=slab;
          slab->prev=slab;

          list_head=slab;
		PDEBUG_LIST("EXITING:list_head=%lu,list_head->num=%d\n",
					(unsigned long)list_head,list_head->num);
		return list_head;
     }
     else
     {
		PDEBUG_LIST("Second onwards slab header to be added\n");

		tmp_header=list_head;
		while(tmp_header->next!=list_head)	
			tmp_header=tmp_header->next;

          slab->next=tmp_header->next;
          slab->prev=tmp_header;
          tmp_header->next=slab;
		list_head->prev=slab;
     }
	
	PDEBUG_LIST("Exiting\n");
     return list_head;
} 

struct kmem_slab *rm_slab(struct kmem_slab *list_head, \
	struct kmem_slab *slab)
{
     struct kmem_slab *tmp_header;

	PDEBUG_LIST("Entered\n");

     tmp_header=search_slab(list_head,slab);

     if(NULL==tmp_header)
	{	
		PDEBUG_LIST("EXITING:The slab header do not exists\n");
          return list_head; //No action is required as dba does not exists
	}

     if((tmp_header->next==list_head) && (tmp_header->prev==list_head) \
			&& (tmp_header->prev->next == list_head))
     {
          list_head=NULL;
//          free(tmp_header);
		PDEBUG_LIST("EXITING:Only one slab exixted and that was removed \n");
          return list_head;
     }

     tmp_header->prev->next=tmp_header->next;
     tmp_header->next->prev=tmp_header->prev;

     if(tmp_header == list_head)
          list_head=tmp_header->next;

     //free(tmp_header);

	PDEBUG_LIST("Exiting\n");

     return list_head;
}


struct kmem_slab *search_slabaddr(struct kmem_slab *list_head, void *slabaddr)
{
     struct kmem_slab *tmp_header;
     int i=0;

     PDEBUG_LIST("Entered\n");

     if(slabListEmpty(list_head))
     {
          PDEBUG_LIST("EXITING:Slab header list is empty\n");
          return NULL;
     }
     else
     {
          tmp_header=list_head;
          do{
               PDEBUG_LIST("This is slab number %d\n",i);

               if(tmp_header->slab_addr==slabaddr)
               {
                    PDEBUG_LIST("EXITING:SLAB FOUND:=%d\n",i);
                    return tmp_header;
               }
               tmp_header=tmp_header->next;
               i++;
          }while(tmp_header!=list_head);
     }

     PDEBUG_LIST("EXITING:SLAB DID NOT FIND=%lu\n",(unsigned long)slab_addr);
     return NULL;
}
/*Doubly Linked list operations for the slab-header structure:ENDS*/


/*Singly Linked list operations for the kmem_bufctl structure:STARTS*/
bool bufctlListEmpty(struct kmem_bufctl *list_head)
{
	if(NULL==list_head)
		return true;
	else
		return false;
}

struct kmem_bufctl *search_kmem_bufctl(struct kmem_bufctl *list_head,\
									struct kmem_bufctl *bufctl_addr)
{
	
	struct kmem_bufctl *tmp_bufctl;

	PDEBUG_LIST("Entered\n");

	if(bufctlListEmpty(list_head))
	{
		PDEBUG_LIST("EXITING:bufctl list is empty\n");
		return NULL;
	}
	else
	{
		tmp_bufctl=list_head;
		do{
			PDEBUG_LIST("This is bufctl number %d\n",tmp_bufctl->num);	

			if(tmp_bufctl==bufctl_addr)
			{
				PDEBUG_LIST("EXITING:BUFCTL FOUND:=%d\n",tmp_bufctl->num);
				return tmp_bufctl;
			}
			tmp_bufctl=tmp_bufctl->next;
		}while(tmp_bufctl!=NULL);
	}

	PDEBUG_LIST("EXITING:BUFCTL DID NOT FIND=%lu\n",(unsigned long)bufctl_addr);
	return NULL;
}

struct kmem_bufctl *add_kmem_bufctl(struct kmem_bufctl *list_head, \
					struct kmem_bufctl *kmem_bufctl)
{
     struct kmem_bufctl *tmp_bufctl, *bufctl_exists;

	PDEBUG_LIST("Entered:list_head%lu\n",(unsigned long)list_head);

	bufctl_exists=search_kmem_bufctl(list_head, kmem_bufctl);

	if(bufctl_exists!=NULL)
	{
		PDEBUG_LIST("The bufctl you are adding exists on the list\n");
		PDEBUG_LIST("And List head:%lu\tbufctl is:%lu",\
			(unsigned long)list_head,(unsigned long)bufctl_exists);
		exit(0);
	}

     if(bufctlListEmpty(list_head))
     {
		PDEBUG_LIST("First bufctl to be added\n");

          kmem_bufctl->next=NULL;

          list_head=kmem_bufctl;
		PDEBUG_LIST("EXITING:list_head=%lu,list_head->num=%d\n",
					(unsigned long)list_head,list_head->num);
		return list_head;
     }
     else
     {
		PDEBUG_LIST("Second onwards bufctls to be added\n");
     	tmp_bufctl=list_head;

          kmem_bufctl->next=NULL;
		while(tmp_bufctl->next!=NULL)	
		{
			
			PDEBUG_LIST("tmp_bufctl=%lu\ttmp_bufctl->next=%lu\n",\
				(unsigned long)tmp_bufctl, (unsigned long)tmp_bufctl->next);
			tmp_bufctl=tmp_bufctl->next;
		}

          tmp_bufctl->next=kmem_bufctl;
     }
	
	PDEBUG_LIST("Exiting\n");
     return list_head;
} 

struct kmem_bufctl *rm_kmem_bufctl(struct kmem_bufctl *list_head, \
	struct kmem_bufctl *kmem_bufctl)
{
     struct kmem_bufctl *tmp_bufctl, *curr_bufctl, *prev_bufctl;

	PDEBUG_LIST("Entered\n");

     tmp_bufctl=search_kmem_bufctl(list_head, kmem_bufctl);

     if(NULL==tmp_bufctl)
	{	
		PDEBUG_LIST("EXITING:The kmem_bufctl do not exists\n");
          return NULL; //No action is required as dba does not exists
	}

	curr_bufctl=list_head;
	prev_bufctl=list_head;

	do{
		if(curr_bufctl==tmp_bufctl)
		{
			if(curr_bufctl != prev_bufctl)
			{
				prev_bufctl->next=curr_bufctl->next;
//				free(curr_bufctl);
				PDEBUG_LIST("EXITING:The elememt was not "\
							"first one and only one\n");
				return list_head;

			}
			else
			{
				list_head=curr_bufctl->next;
//				free(curr_bufctl);
				PDEBUG_LIST("EXITING:The elememt was first one and " \
							"may be or may not be the only one\n");
				return list_head;
			}
		}
		else
		{
			prev_bufctl=curr_bufctl;
			curr_bufctl=curr_bufctl->next;
		}
	}while(curr_bufctl!=NULL);

	PDEBUG_LIST("Exiting\n");

     return list_head;
}


struct kmem_bufctl *search_buf(struct kmem_bufctl *list_head,\
									void *buf)
{
	
	struct kmem_bufctl *tmp_bufctl;

	PDEBUG_LIST("Entered\n");

	if(bufctlListEmpty(list_head))
	{
		PDEBUG_LIST("EXITING:bufctl list is empty\n");
		return NULL;
	}
	else
	{
		tmp_bufctl=list_head;
		do{
			PDEBUG_LIST("This is bufctl number %d\n",tmp_bufctl->num);	

			if(tmp_bufctl->buf==buf)
			{
				PDEBUG_LIST("EXITING:BUF FOUND:=%lu\n",(unsigned long)buf);
				return tmp_bufctl;
			}
			tmp_bufctl=tmp_bufctl->next;
		}while(tmp_bufctl!=NULL);
	}

	PDEBUG_LIST("EXITING:BUFFER DID NOT FIND=%lu\n",(unsigned long)buf);
	return NULL;
}

/*Singly Linked list operations for the kmem_bufctl structure:ENDS*/

/*
int main()
{
	int i;
		
	struct kmem_slab *tmp_header;

	for(i=0;i<10; i++)
	{
		tmp_header=(struct kmem_slab *)malloc(sizeof(struct kmem_slab));
		tmp_header->num=i;
		list_head=add_slab(list_head, tmp_header);
		PDEBUG_LIST("list_head=%d\n",(unsigned long)list_head);
		
		tmp_header=search_slab(list_head,i);

		PDEBUG_LIST("If add is successful search should be successful \
				and hence i=%d\n",tmp_header->num);
	}	

	tmp_header=search_slab(list_head, 0);
	list_head=rm_slab(list_head, tmp_header);
	
	tmp_header=search_slab(list_head, 8);
	list_head=rm_slab(list_head, tmp_header);

	for(i=0;i<10; i++)
	{
		tmp_header=search_slab(list_head, i);

		if(tmp_header!=NULL)
		PDEBUG_LIST("ITERATION:%d: If add/delete is successful;\
					 search should be successful\
					and hence i=%d\n",i,tmp_header->num);
	}	

	int i;
		
	struct kmem_bufctl *tmp_bufctl;

	for(i=0;i<10; i++)
	{
		tmp_bufctl=(struct kmem_bufctl *)malloc(sizeof(struct kmem_bufctl));
		tmp_bufctl->num=i;
		list_head=add_kmem_bufctl(list_head, tmp_bufctl);
		PDEBUG_LIST("list_head=%lu\n",(unsigned long)list_head);
		
		tmp_bufctl=search_kmem_bufctl(list_head,i);

		PDEBUG_LIST("If add is successful search should be successful \
				and hence i=%d\n",tmp_bufctl->num);
	}	

	tmp_bufctl=search_kmem_bufctl(list_head, 0);
	list_head=rm_kmem_bufctl(list_head, tmp_bufctl);
	
	tmp_bufctl=search_kmem_bufctl(list_head,9);
	list_head=rm_kmem_bufctl(list_head, tmp_bufctl);

	tmp_bufctl=(struct kmem_bufctl *)malloc(sizeof(struct kmem_bufctl));
	tmp_bufctl->num=0;
	list_head=add_kmem_bufctl(list_head, tmp_bufctl);

	for(i=0;i<10; i++)
	{
		tmp_bufctl=search_kmem_bufctl(list_head,i);

		if(tmp_bufctl!=NULL)
		PDEBUG_LIST("ITERATION:%d: If add/delete is successful;\
					 search should be successful\
					and hence i=%d\n",i,tmp_bufctl->num);
	}	

	return 0;

}
*/
