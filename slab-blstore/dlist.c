#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h> 
#include <strings.h>
#include <stdbool.h>
#include "dlist.h"
#include "virtptr.h"
#include "bllbarw.h"
#include "dslabbuild.h"
#include "debugmsg.h"


/*These are the temporary list headers actual list headers ponters will be in  *
 * either slab-header structure or dmem_bufctl structure                      */
//struct dmem_bufctl *list_head=NULL;
//struct dmem_slab *list_head=NULL;

/*Doubly Linked list operations for the slab-header structure:STARTS*/
bool slabListEmpty(vptr_slab list_head)
{
	if(EQUAL(vsnull,list_head))
		return true;
	else
		return false;
}

vptr_slab search_slab(vptr_slab list_head, vptr_slab slab_addr)
{
	vptr_slab tmp_header;
	int i=0;

	PDEBUG_DLIST("Entered\n");

	if(slabListEmpty(list_head))
	{
		PDEBUG_DLIST("EXITING:Slab header list is empty\n");
		return vsnull;
	}
	else
	{
		tmp_header=list_head;
		do{
			PDEBUG_DLIST("This is slab number %d\n",i);	

			if(EQUAL(tmp_header,slab_addr))
			{
				PDEBUG_DLIST("EXITING:SLAB FOUND:=%d\n",i);
				return tmp_header;
			}

			struct dmem_slab *sp=__getmem(sizeof(struct dmem_slab));
			loads(tmp_header, SLAB_LEN, (unsigned char *)sp);	
			tmp_header=sp->next;
			free(sp);

			i++;
		}while(NOTEQUAL(tmp_header, list_head));
	}

	PDEBUG_DLIST("EXITING:SLAB DID NOT FIND\n");
	PRINTA_DLIST(slab_addr);	
	PRINTNL;
	return vsnull;
}

vptr_slab add_slab(vptr_slab list_head, vptr_slab slab)
{
     vptr_slab tmp_header;

	PRINTNL;
	PDEBUG_DLIST("Entered:-");	PRINTA_DLIST(slab);	
	PDEBUG_DLIST("list_head:-");	PRINTA_DLIST(list_head);	

     if(slabListEmpty(list_head))
     {
		PDEBUG_DLIST("First slab header to be added\n");

		struct dmem_slab *sp=__getmem(sizeof(struct dmem_slab));
		loads(slab, SLAB_LEN, (unsigned char *)sp);

          sp->next=slab;
          sp->prev=slab;

          list_head=slab;
		
		PDEBUG_DLIST("EXITING:list_head ");	PRINTA_DLIST(list_head); 
		PDEBUG_DLIST("sp->next ");	PRINTA_DLIST(sp->next); 
		PDEBUG_DLIST("sp->prev ");	PRINTA_DLIST(sp->prev); 


		stores(slab, SLAB_LEN, (unsigned char *)sp);
		free(sp);
		return list_head;
     }
     else
     {
		PDEBUG_DLIST("Second onwards slab header to be added\n");

     	tmp_header=list_head;

		struct dmem_slab *slabp=__getmem(sizeof(struct dmem_slab));
		struct dmem_slab *listheadp=__getmem(sizeof(struct dmem_slab));
		struct dmem_slab *tmpslabp=__getmem(sizeof(struct dmem_slab));

		loads(slab, SLAB_LEN, (unsigned char *)slabp);
		loads(list_head, SLAB_LEN, (unsigned char *)listheadp);
		loads(tmp_header, SLAB_LEN, (unsigned char *)tmpslabp);

		while(NOTEQUAL(tmpslabp->next, list_head))	
		{
			PDEBUG_DLIST("list_head ");	PRINTA_DLIST(list_head);
			PDEBUG_DLIST("tmp_header ");	PRINTA_DLIST(tmp_header);
			
			PDEBUG_DLIST("Moving forward in the list\n");
			tmp_header=tmpslabp->next;
			loads(tmp_header, SLAB_LEN, (unsigned char *)tmpslabp);
		}


		if(EQUAL(tmp_header, list_head))
		{
	          slabp->next=list_head;
     	     slabp->prev=list_head;
			stores(slab, SLAB_LEN, (unsigned char *)slabp);

			listheadp->prev=slab;
			listheadp->next=slab;
			stores(list_head, SLAB_LEN, (unsigned char *)listheadp);
		}
		else
		{
          	slabp->next=list_head;
	          slabp->prev=tmp_header;
			stores(slab, SLAB_LEN, (unsigned char *)slabp);

	          tmpslabp->next=slab;
			listheadp->prev=slab;

			stores(tmp_header, SLAB_LEN, (unsigned char *)tmpslabp);
			stores(list_head, SLAB_LEN, (unsigned char *)listheadp);
		}
		free(slabp);
		free(listheadp);
		free(tmpslabp);
     }
	

	PDEBUG_DLIST("Exiting\n");
     return list_head;
} 

vptr_slab rm_slab(vptr_slab list_head,vptr_slab slab)
{
     vptr_slab tmp_header;

	PDEBUG_DLIST("Entered"); PRINTA_DLIST(list_head);

     tmp_header=search_slab(list_head,slab);
	PDEBUG_DLIST("slab"); PRINTA_DLIST(slab);
	PDEBUG_DLIST("tmp_header"); PRINTA_DLIST(tmp_header);

     if(EQUAL(vsnull,tmp_header))
	{	
		PDEBUG_DLIST("EXITING:The slab header do not exists\n");
		exit(0);
          return list_head; //No action is required as dba does not exists
	}

     struct dmem_slab *tmpslabp=__getmem(sizeof(struct dmem_slab));
     struct dmem_slab *tmpslab_np=__getmem(sizeof(struct dmem_slab));
     struct dmem_slab *tmpslab_pp=__getmem(sizeof(struct dmem_slab));

     loads(tmp_header, SLAB_LEN, (unsigned char *)tmpslabp);
     loads(tmpslabp->next, SLAB_LEN, (unsigned char *)tmpslab_np);
     loads(tmpslabp->prev, SLAB_LEN, (unsigned char *)tmpslab_pp);

	PDEBUG_DLIST("tmpslabp->next"); PRINTA_DLIST(tmpslabp->next);
	PDEBUG_DLIST("tmpslabp->prev"); PRINTA_DLIST(tmpslabp->prev);

     if(EQUAL(tmpslabp->next,list_head) && EQUAL(tmpslabp->prev,list_head) \
			&& EQUAL(tmpslab_pp->next,list_head))
     {
          list_head=vsnull;
		PDEBUG_DLIST("EXITING:Only one slab exixted and that was removed \n");
		free(tmpslabp);
		free(tmpslab_np);
		free(tmpslab_pp);
		
          return list_head;
     }

	if(NOTEQUAL(tmpslabp->prev,tmpslabp->next))
	{
	     tmpslab_pp->next=tmpslabp->next;
     	tmpslab_np->prev=tmpslabp->prev;
     	stores(tmpslabp->prev, SLAB_LEN, (unsigned char *)tmpslab_pp);
	     stores(tmpslabp->next, SLAB_LEN, (unsigned char *)tmpslab_np);
	}
	else
	{
	     tmpslab_pp->next=tmpslabp->next;
	     tmpslab_pp->prev=tmpslabp->prev;
     	stores(tmpslabp->prev, SLAB_LEN, (unsigned char *)tmpslab_pp);
	}

     if(EQUAL(tmp_header, list_head))
          list_head=tmpslabp->next;


	free(tmpslabp);
	free(tmpslab_np);
	free(tmpslab_pp);

	PDEBUG_DLIST("Exiting"); PRINTA_DLIST(list_head);

     return list_head;
}


vptr_slab search_slabaddr(vptr_slab list_head, vptr slabaddr)
{
     vptr_slab tmp_header;
     int i=0;

     PDEBUG_DLIST("Entered\n");

     if(slabListEmpty(list_head))
     {
          PDEBUG_DLIST("EXITING:Slab header list is empty\n");
          return vsnull;
     }
     else
     {
    		struct dmem_slab *tmpslabp=__getmem(sizeof(struct dmem_slab));

          tmp_header=list_head;
          do{
               PDEBUG_DLIST("This is slab number %d\n",i);
		     loads(tmp_header, SLAB_LEN, (unsigned char *)tmpslabp);

               if(EQUAL(tmpslabp->slab_addr,slabaddr))
               {
                    PDEBUG_DLIST("EXITING:SLAB FOUND:=%d\n",i);
                    return tmp_header;
               }
               tmp_header=tmpslabp->next;
               i++;
          }while(NOTEQUAL(tmp_header, list_head));
	
		free(tmpslabp);
     }

     PDEBUG_DLIST("EXITING:SLAB DID NOT FIND");
	PRINTA_DLIST(slabaddr); PDEBUG_DLIST("\n");
     return vsnull;
}
/*Doubly Linked list operations for the slab-header structure:ENDS*/


/*Singly Linked list operations for the dmem_bufctl structure:STARTS*/
bool bufctlListEmpty(vptr_bufctl list_head)
{
	if(EQUAL(vbnull,list_head))
		return true;
	else
		return false;
}

vptr_bufctl search_dmem_bufctl(vptr_bufctl list_head, vptr_bufctl bufctl_addr)
{
	vptr_bufctl tmp_bufctl;

	PDEBUG_DLIST("Entered\n");

	if(bufctlListEmpty(list_head))
	{
		PDEBUG_DLIST("EXITING:bufctl list is empty\n");
		return vbnull;
	}
	else
	{
    		struct dmem_bufctl *tmpbufctlp=__getmem(sizeof(struct dmem_bufctl));
		tmp_bufctl=list_head;
		do{
//			PDEBUG_DLIST("This is bufctl number %d\n",tmp_bufctl->num);	

			if(EQUAL(tmp_bufctl, bufctl_addr))
			{
//				PDEBUG_DLIST("EXITING:BUFCTL FOUND:=%d\n",tmp_bufctl->num);
				return tmp_bufctl;
			}
		     loadb(tmp_bufctl, BUFCTL_LEN, (unsigned char *)tmpbufctlp);
			tmp_bufctl=tmpbufctlp->next;
		}while(NOTEQUAL(tmp_bufctl, vbnull));

		free(tmpbufctlp);
	}

	PDEBUG_DLIST("EXITING_:BUFCTL DID NOT FIND\n");
     PRINTA_DLIST(bufctl_addr); PDEBUG_DLIST("\n");

	return vbnull;
}


vptr_bufctl add_dmem_bufctl(vptr_bufctl list_head, vptr_bufctl dmem_bufctl)
{
     vptr_bufctl tmp_bufctl, bufctl_exists;

	PDEBUG_DLIST("Entered:list_head:-");	PRINTA_DLIST(list_head);

	bufctl_exists=search_dmem_bufctl(list_head, dmem_bufctl);

	if(NOTEQUAL(bufctl_exists, vbnull))
	{
		PDEBUG_DLIST("The bufctl you are adding exists on the list\n");

		PDEBUG_DLIST("list_head:-");		PRINTA_DLIST(list_head);
		PDEBUG_DLIST("bufctl is:-");		PRINTA_DLIST(bufctl_exists); 

		exit(0);
	}

	struct dmem_bufctl *dmembufctlp=__getmem(sizeof(struct dmem_bufctl));
     loadb(dmem_bufctl, BUFCTL_LEN, (unsigned char *)dmembufctlp);

     if(bufctlListEmpty(list_head))
     {
		PDEBUG_DLIST("First bufctl to be added\n");

          dmembufctlp->next=vbnull;
          list_head=dmem_bufctl;

		PDEBUG_DLIST("list_head:-");		PRINTA_DLIST(list_head);
		
     	storeb(dmem_bufctl, BUFCTL_LEN, (unsigned char *)dmembufctlp);
		free(dmembufctlp);
		return list_head;
     }
     else
     {
		PDEBUG_DLIST("Second onwards bufctls to be added\n");
     	tmp_bufctl=list_head;

		struct dmem_bufctl *tmpbufctlp=__getmem(sizeof(struct dmem_bufctl));
     	loadb(tmp_bufctl, BUFCTL_LEN, (unsigned char *)tmpbufctlp);

          dmembufctlp->next=vbnull;
		while(NOTEQUAL(tmpbufctlp->next,vbnull))	
		{
			PDEBUG_DLIST("list_head:-");	PRINTA_DLIST(tmp_bufctl); 
			PDEBUG_DLIST("list_head:-");	PRINTA_DLIST(tmpbufctlp->next); 
			
			tmp_bufctl=tmpbufctlp->next;
     		loadb(tmp_bufctl, BUFCTL_LEN, (unsigned char *)tmpbufctlp);
		}

          tmpbufctlp->next=dmem_bufctl;
     	storeb(tmp_bufctl, BUFCTL_LEN, (unsigned char *)tmpbufctlp);
     	storeb(dmem_bufctl, BUFCTL_LEN, (unsigned char *)dmembufctlp);

		free(dmembufctlp);
		free(tmpbufctlp);
     }
	
	PDEBUG_DLIST("Exiting\n");
     return list_head;
} 

vptr_bufctl rm_dmem_bufctl(vptr_bufctl list_head,vptr_bufctl dmem_bufctl)
{
     vptr_bufctl tmp_bufctl, curr_bufctl, prev_bufctl;

	PDEBUG_DLIST("Entered\n");

     tmp_bufctl=search_dmem_bufctl(list_head, dmem_bufctl);

     if(EQUAL(vbnull, tmp_bufctl))
	{	
		PDEBUG_DLIST("EXITING:The dmem_bufctl do not exists\n");
          return vbnull; //No action is required as dba does not exists
	}

	curr_bufctl=list_head;
	prev_bufctl=list_head;

	do{
		if(EQUAL(curr_bufctl,tmp_bufctl))
		{
			if(NOTEQUAL(curr_bufctl, prev_bufctl))
			{
				/*load and store*/
		     	struct dmem_bufctl *currbufctlp=__getmem(sizeof(struct dmem_bufctl));
		     	struct dmem_bufctl *prevbufctlp=__getmem(sizeof(struct dmem_bufctl));

			     loadb(curr_bufctl, BUFCTL_LEN, (unsigned char *)currbufctlp);
			     loadb(prev_bufctl, BUFCTL_LEN, (unsigned char *)prevbufctlp);

				prevbufctlp->next=currbufctlp->next;
				PDEBUG_DLIST("EXITING:The elememt was not "\
							"first one and only one\n");
				
			     storeb(prev_bufctl, BUFCTL_LEN, (unsigned char *)prevbufctlp);
				
				free(currbufctlp);
				free(prevbufctlp);

				return list_head;
			}
			else
			{
				/*load*/
		     	struct dmem_bufctl *currbufctlp=__getmem(sizeof(struct dmem_bufctl));
			     loadb(curr_bufctl, BUFCTL_LEN, (unsigned char *)currbufctlp);

				list_head=currbufctlp->next;
				PDEBUG_DLIST("EXITING:The elememt was first one and " \
							"may be or may not be the only one\n");
				free(currbufctlp);
				return list_head;
			}
		}
		else
		{
			/*load and store*/
		    	struct dmem_bufctl *currbufctlp=__getmem(sizeof(struct dmem_bufctl));
		     loadb(curr_bufctl, BUFCTL_LEN, (unsigned char *)currbufctlp);

			prev_bufctl=curr_bufctl;
			curr_bufctl=currbufctlp->next;

			free(currbufctlp);
		}
	}while(NOTEQUAL(curr_bufctl, vbnull));

	PDEBUG_DLIST("Exiting\n");

     return list_head;
}


vptr_bufctl search_buf(vptr_bufctl list_head, vptr buf)
{
	vptr_bufctl tmp_bufctl;

	PDEBUG_DLIST("Entered\n");

	if(bufctlListEmpty(list_head))
	{
		PDEBUG_DLIST("EXITING:bufctl list is empty\n");
		return vbnull;
	}
	else
	{
		tmp_bufctl=list_head;
	   	struct dmem_bufctl *tmpbufctlp=__getmem(sizeof(struct dmem_bufctl));

		do{
	     	loadb(tmp_bufctl, BUFCTL_LEN, (unsigned char *)tmpbufctlp);
//			PDEBUG_DLIST("This is bufctl number %d\n",tmp_bufctl->num);	

			if(EQUAL(tmpbufctlp->buf,buf))
			{
				PDEBUG_DLIST("EXITING:BUF FOUND\n");
				PRINTA_DLIST(buf); PDEBUG_DLIST("\n");
				free(tmpbufctlp);
				return tmp_bufctl;
			}
			tmp_bufctl=tmpbufctlp->next;
		}while(NOTEQUAL(tmp_bufctl,vbnull));
	}

	PDEBUG_DLIST("EXITING:BUF DID NOT FIND\n");
	PRINTA_DLIST(buf); PDEBUG_DLIST("\n");
	return vbnull;
}

/*Singly Linked list operations for the dmem_bufctl structure:ENDS*/

/*
int main()
{
	int i;
		
	struct dmem_slab *tmp_header;

	for(i=0;i<10; i++)
	{
		tmp_header=(struct dmem_slab *)malloc(sizeof(struct dmem_slab));
		tmp_header->num=i;
		list_head=add_slab(list_head, tmp_header);
		PDEBUG_DLIST("list_head=%d\n",(unsigned long)list_head);
		
		tmp_header=search_slab(list_head,i);

		PDEBUG_DLIST("If add is successful search should be successful \
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
		PDEBUG_DLIST("ITERATION:%d: If add/delete is successful;\
					 search should be successful\
					and hence i=%d\n",i,tmp_header->num);
	}	

	int i;
		
	struct dmem_bufctl *tmp_bufctl;

	for(i=0;i<10; i++)
	{
		tmp_bufctl=(struct dmem_bufctl *)malloc(sizeof(struct dmem_bufctl));
		tmp_bufctl->num=i;
		list_head=add_dmem_bufctl(list_head, tmp_bufctl);
		PDEBUG_DLIST("list_head=%lu\n",(unsigned long)list_head);
		
		tmp_bufctl=search_dmem_bufctl(list_head,i);

		PDEBUG_DLIST("If add is successful search should be successful \
				and hence i=%d\n",tmp_bufctl->num);
	}	

	tmp_bufctl=search_dmem_bufctl(list_head, 0);
	list_head=rm_dmem_bufctl(list_head, tmp_bufctl);
	
	tmp_bufctl=search_dmem_bufctl(list_head,9);
	list_head=rm_dmem_bufctl(list_head, tmp_bufctl);

	tmp_bufctl=(struct dmem_bufctl *)malloc(sizeof(struct dmem_bufctl));
	tmp_bufctl->num=0;
	list_head=add_dmem_bufctl(list_head, tmp_bufctl);

	for(i=0;i<10; i++)
	{
		tmp_bufctl=search_dmem_bufctl(list_head,i);

		if(tmp_bufctl!=NULL)
		PDEBUG_DLIST("ITERATION:%d: If add/delete is successful;\
					 search should be successful\
					and hence i=%d\n",i,tmp_bufctl->num);
	}	

	return 0;

}
*/
