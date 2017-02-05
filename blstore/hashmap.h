#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include "blockstore.h"

struct hash_addr{
	lba_t lba;
	dba_t dba;
};

/*******************************************************************************
 * This function returns the size of the hashmap in terms of number of blocks. *
 * This is decided by the hash function details such as hash function, number  *
 * of buckets, per block buckets, number of nodes per bucket                   *
 * INPUT : pointer to structure which contains details about hash function     *
 * OUTPUT: size of the hashmap in terms of number of blocks.                   *
 ******************************************************************************/ 
int get_hashmapsize(struct HashFun *hfp);


/*******************************************************************************
 * This function initialize the hashmap when first time VOLUME is created.     *
 * It has to fill all the blocks for hashmap with zeroes. It internally calls  *
 * gmamap_init function which initializes the gmammap and returns the last add-*
 * -ress used for the the gmamap tree plus hashmap itself.                     *
 * INPUT : The file pointer of a file containing the storage volume.           *
 *         pointer to hashfunction details.                                    *
 *         Starting address for the hashmap + gmamap tree                      *
 * OUTPUT: The last dba addr used for the hashmap and hashmaptre.              *
 ******************************************************************************/ 
dba_t hashmap_init(struct BlockStore *bsp,struct HashFun *hfp, dba_t start_dba);


/*******************************************************************************
 * This        function returns the DBA for input LBA. We have to map LBA to   *
 * DBA in read request as well as write requests. This is the common function  *
 * which converts the LBA to DBA.                                              *
 * If it is read operation, then we need to get the corresponding DBA from     *
 * hashmap and return it to requesting function and there wont be any change in*
 * the hashmap itself and hashmap tree. But if it is the write operation then  *
 * we need to get dba for input lba(if it is fresh write then dba corresponding*
 * to lba should be zero as client program must have invoked alloLBA before.But*
 * if it is old write then there must be dba present for this lba.)            *
 * In either case we need to get new dba from bitmap, only in second case read *
 * block from volume and copy in newly got dba, and in both the cases, return  *
 * the newly allocated dba to requested function.                              *
 * INPUT : Pointer to BlockStore data structure. This should contain the hash  *
 *         details.                                                            *
 *         lba for which we need to get actual dba                             *
 *         Type of operation: 0-read;1-write                                   *
 * OUTPUT: mapped dba for input lba.                                           *
 ******************************************************************************/ 
dba_t getdba(struct BlockStore *bsp, lba_t lba_addr, bool op);


/*******************************************************************************
 * This function searches the hashmap, locate the bucket-ID for the lba_addr,  *
 * for that bucket-ID, it will find block ID and from that it will find        *
 * dba_addr for lba_addr.                                                      *
 * INPUT : pointer to blockstore structure                                     *
 *         lba address to search dba address of                                *
 * OUTPUT: dba address for lba address                                         *
 *         true if lba is present false otherwise                              *
 ******************************************************************************/ 
bool searchlba(struct BlockStore *bsp, lba_t lba_addr, dba_t *dba_addr);


/*******************************************************************************
 * This function searches the hashmap, locate the bucket-ID for the lba_addr,  *
 * for that bucket-ID, it will find block ID and will update dba_addr for this *
 * lba_addr.                                                                   *
 * INPUT : pointer to blockstore structure                                     *
 *         lba address of which dba address has to be updated                  *
 *         corresponding dba address for this lba                              *
 ******************************************************************************/ 
void updatelba(struct BlockStore *bsp, lba_t lba_addr, dba_t dba_addr);


/*******************************************************************************
 * This function searches the hashmap, locate the bucket-ID for the lba_addr,  *
 * for that bucket-ID, it will find block ID and will update dba_addr for this *
 * lba_addr.                                                                   *
 * INPUT : pointer to blockstore structure                                     *
 *         lba address of which dba address has to be removed                  *
 ******************************************************************************/ 
void deletelba(struct BlockStore *bsp, lba_t lba_addr);


/*******************************************************************************
 * This function adds the lba_addr to the hashmap. It gets the freeDBA from    *
 * bitmap and will allocate to this lba and hashmap will be updated accordingly*
 * INPUT : pointer to blockstore structure                                     *
 *         lba address which is to be added in the hashmap.                    *
 ******************************************************************************/ 
void addlba(struct BlockStore *bsp, lba_t lba_addr);


/*******************************************************************************
 *00000000000000000000000000000000000000000000000000000000000000000000000000000*
 *00000000000000000000000000000000000000000000000000000000000000000000000000000*
 *00000000000000000000000000000000000000000000000000000000000000000000000000000*
 *00000000000000000000000000000000000000000000000000000000000000000000000000000*
 *00000000000000000000000000000000000000000000000000000000000000000000000000000*
 *00000000000000000000000000000000000000000000000000000000000000000000000000000*
 *00000000000000000000000000000000000000000000000000000000000000000000000000000*
 ******************************************************************************/ 
void rescalehash(int dosomethingdude);



/*******************************************************************************
 * This function returns the required space for writing single block to the    *
 * volumestore.                                                                *
 * INPUT:  A poitner to current transaction structure                          *
 ******************************************************************************/ 
unsigned long getreqspace(bl_transaction_t trans);
