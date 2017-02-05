#ifndef BLLBARW_H
#define BLLBARW_H

#include "virtptr.h"
#include "blockstore.h"

/******************************************************************************
 * This function reads the character stream of length of size present at addr-*
 * -ess addr on blockstore into buffer.                                       *
 * INPUT : addr to be read from blockstore                                    *
 *         size to be read from blockstore                                    *
 *         buffer in which data should be read                                *
 * OUTPUT: Actual read size                                                   *
 ******************************************************************************/
void load(vptr addr, size_t len, unsigned char *buffer);


/******************************************************************************
 * These three functions are helper functions which load  the cache objects,  *
 * slab objects and bufctl objects resp.                                      *
 ******************************************************************************/
void loadc(vptr_cache addr, size_t len, unsigned char *buffer);
void loads(vptr_slab addr, size_t len, unsigned char *buffer);
void loadb(vptr_bufctl addr, size_t len, unsigned char *buffer);

/******************************************************************************
 * This function writes the character stream of length of size at address on  *
 * on blockstore from buffer.                                                 *
 * INPUT : addr to be written at on blockstore                                *
 *         size to be written on blockstore                                   *
 *         buffer from which data should be written                           *
 * OUTPUT: Actual written size                                                *
 ******************************************************************************/
void store(vptr addr, size_t len, unsigned char *buffer);


/******************************************************************************
 * These three functions are helper functions which store  the cache objects, *
 * slab objects and bufctl objects resp.                                      *
 ******************************************************************************/
void storec(vptr_cache addr, size_t len, unsigned char *buffer);
void stores(vptr_slab addr, size_t len, unsigned char *buffer);
void storeb(vptr_bufctl addr, size_t len, unsigned char *buffer);

/******************************************************************************
 * This function counts the total lba to be read/write from the blockstore;   *
 * given the starting vptr and total length to be read/written.               *
 * INPUT : starting address                                                   *
 *         length to be read/written                                          *
 *         buffer from which data should be written                           *
 * OUTPUT: Total lbas to be read/written                                      *
 ******************************************************************************/
unsigned long __totlbarw(vptr addr, size_t len);


bl_transaction_t slabtrans;

#endif
