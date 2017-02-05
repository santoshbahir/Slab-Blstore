#ifndef LIST_H
#define LIST_H

#include <stdbool.h>
#include "slabdefine.h"
/*struct kmem_slab{
     int num;
     struct kmem_slab *next;
     struct kmem_slab *prev;
};

struct kmem_bufctl{
     int num;
     struct kmem_bufctl *next;
//     struct kmem_bufctl *prev;
};*/


/*#############################################################################
* Intefaces for the doubly linked list of slab-header Manipulation            *
#############################################################################*/

 
/*******************************************************************************
 * This function checks if list of the slab header is empty                    *
 * INPUT : The pointer to head of the list                                     *
 * OUTPUT: true if the list is empty.                                          *
 *         false if the list is not empty.                                     *
 ******************************************************************************/
bool slabListEmpty(struct kmem_slab *list_head);


/*******************************************************************************
 * This function searches the list of slab header to check if the slab with    *
 * slab_addr is present in the list of slab headers                            *
 * INPUT : The pointer to head of the list.                                    *
 *         slab_addr to be searched.                                           *
 * OUTPUT: pointer to slab header of slab with address slab_addr.              *
 ******************************************************************************/
struct kmem_slab *search_slab(struct kmem_slab *list_head,\
									struct kmem_slab *slab_addr);


/*******************************************************************************
 * This function add the slab header for the slab with slab address slab_addr  *
 * INPUT : The pointer to head of the list.                                    *
 *         pointer to slab header for slab with slab address slab_addr         *
 * OUTPUT: The pointer to head of the list.                                    *
 ******************************************************************************/
struct kmem_slab *add_slab(struct kmem_slab *list_head, \
							struct kmem_slab *slab);


/*******************************************************************************
 * This function delete the slab header for the slab with slab address         *
 * slab_addr                                                                   *
 * INPUT : The pointer to head of the list.                                    *
 *         pointer to slab header for slab with slab address with slab_addr    *
 ******************************************************************************/
struct kmem_slab *rm_slab(struct kmem_slab *list_head, \
								struct kmem_slab *slab);

/******************************************************************************
 * This function searches the doubly linekd list of slab to see if any of     *
 * the slab header is for the slabaddr.This is helpful particularly in the    *
 * case of huge objects.                                                      *
 * INPUT : The pointer to head of the list                                    *
 *         The slab address.                                                  *
 * OUTPUT: pointer to the slab header for this slab addr                      *
 *         NULL otherwise                                                     *
 ******************************************************************************/
struct kmem_slab *search_slabaddr(struct kmem_slab *list_head,void *slabaddr);

/*#############################################################################
* Intefaces for the singly linked list of kmem_bufctl Manipulation            *
#############################################################################*/

 
/*******************************************************************************
 * This function checks if list of the kmem_bufctl is empty                    *
 * INPUT : The pointer to head of the list                                     *
 * OUTPUT: true if the list is empty.                                          *
 *         false if the list is not empty.                                     *
 ******************************************************************************/
bool bufctlListEmpty(struct kmem_bufctl *list_head);


/*******************************************************************************
 * This function searches the list of kmem_bufctl to check if the buff with    *
 * buf_addr is present in the list of kmem_bufctls                             *
 * INPUT : The pointer to head of the list.                                    *
 *         bufctl_addr to be searched.                                         *
 * OUTPUT: pointer to slab header of slab with address slab_addr.              *
 ******************************************************************************/
struct kmem_bufctl *search_kmem_bufctl(struct kmem_bufctl *list_head,\
								struct kmem_bufctl *bufctl_addr);


/*******************************************************************************
 * This function add the kmem_bufctl in the list with list_head as a head      *
 * INPUT : The pointer to head of the list.                                    *
 *         pointer to kmem_bufctl to be added on the list                      *
 ******************************************************************************/
struct kmem_bufctl *add_kmem_bufctl(struct kmem_bufctl *list_head, \
							struct kmem_bufctl *kmem_bufctl);


/*******************************************************************************
 * This function delete the kmem_bufctl                                        *
 * INPUT : The pointer to head of the list.                                    *
 *         pointer to kmem_bufctl to be deleted from the list                  *
 ******************************************************************************/
struct kmem_bufctl *rm_kmem_bufctl(struct kmem_bufctl *list_head, \
								struct kmem_bufctl *kmem_bufctl);

/*******************************************************************************
 * This function searches all the bufctl for buf                               *
 * INPUT : The pointer to head of the list.                                    *
 *         pointer to kmem_bufctl for the buffer with address buf              *
 ******************************************************************************/
struct kmem_bufctl *search_buf(struct kmem_bufctl *list_head,void *buf);

#endif
