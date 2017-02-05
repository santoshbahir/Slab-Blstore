#ifndef LIST_H
#define LIST_H

#include <stdbool.h>
#include "dslabdefine.h"
#include "virtptr.h"

/*struct dmem_slab{
     int num;
     struct dmem_slab *next;
     struct dmem_slab *prev;
};

struct dmem_bufctl{
     int num;
     struct dmem_bufctl *next;
//     struct dmem_bufctl *prev;
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
bool slabListEmpty(vptr_slab list_head);


/*******************************************************************************
 * This function searches the list of slab header to check if the slab with    *
 * slab_addr is present in the list of slab headers                            *
 * INPUT : The pointer to head of the list.                                    *
 *         slab_addr to be searched.                                           *
 * OUTPUT: pointer to slab header of slab with address slab_addr.              *
 ******************************************************************************/
vptr_slab search_slab(vptr_slab list_head, vptr_slab slab_addr);


/*******************************************************************************
 * This function add the slab header for the slab with slab address slab_addr  *
 * INPUT : The pointer to head of the list.                                    *
 *         pointer to slab header for slab with slab address slab_addr         *
 * OUTPUT: The pointer to head of the list.                                    *
 ******************************************************************************/
vptr_slab add_slab(vptr_slab list_head, vptr_slab slab);


/*******************************************************************************
 * This function delete the slab header for the slab with slab address         *
 * slab_addr                                                                   *
 * INPUT : The pointer to head of the list.                                    *
 *         pointer to slab header for slab with slab address with slab_addr    *
 ******************************************************************************/
vptr_slab rm_slab(vptr_slab list_head, vptr_slab slab);


/******************************************************************************
 * This function searches the doubly linekd list of slab to see if any of     *
 * the slab header is for the slabaddr.This is helpful particularly in the    *
 * case of huge objects.                                                      *
 * INPUT : The pointer to head of the list                                    *
 *         The slab address.                                                  *
 * OUTPUT: pointer to the slab header for this slab addr                      *
 *         NULL otherwise                                                     *
 ******************************************************************************/
vptr_slab search_slabaddr(vptr_slab list_head,vptr slabaddr);


/*#############################################################################
* Intefaces for the singly linked list of dmem_bufctl Manipulation            *
#############################################################################*/

 
/*******************************************************************************
 * This function checks if list of the dmem_bufctl is empty                    *
 * INPUT : The pointer to head of the list                                     *
 * OUTPUT: true if the list is empty.                                          *
 *         false if the list is not empty.                                     *
 ******************************************************************************/
bool bufctlListEmpty(vptr_bufctl list_head);


/*******************************************************************************
 * This function searches the list of dmem_bufctl to check if the buff with    *
 * buf_addr is present in the list of dmem_bufctls                             *
 * INPUT : The pointer to head of the list.                                    *
 *         bufctl_addr to be searched.                                         *
 * OUTPUT: pointer to slab header of slab with address slab_addr.              *
 ******************************************************************************/
vptr_bufctl search_dmem_bufctl(vptr_bufctl list_head, vptr_bufctl bufctl_addr);


/*******************************************************************************
 * This function add the dmem_bufctl in the list with list_head as a head      *
 * INPUT : The pointer to head of the list.                                    *
 *         pointer to dmem_bufctl to be added on the list                      *
 ******************************************************************************/
vptr_bufctl add_dmem_bufctl(vptr_bufctl list_head, vptr_bufctl dmem_bufctl);


/*******************************************************************************
 * This function delete the dmem_bufctl                                        *
 * INPUT : The pointer to head of the list.                                    *
 *         pointer to dmem_bufctl to be deleted from the list                  *
 ******************************************************************************/
vptr_bufctl rm_dmem_bufctl(vptr_bufctl list_head, vptr_bufctl dmem_bufctl);


/*******************************************************************************
 * This function searches all the bufctl for buf                               *
 * INPUT : The pointer to head of the list.                                    *
 *         pointer to dmem_bufctl for the buffer with address buf              *
 ******************************************************************************/
vptr_bufctl search_buf(vptr_bufctl list_head, vptr buf);


#endif
