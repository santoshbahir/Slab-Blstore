#include <math.h>
#include "blockstore.h"
#include "block_cache.h"
#include "phyblockrw.h"
#include "mapblockaddr.h"
#include "bitmap.h"

struct tree_level_info * get_tree_levels_info(unsigned long bitmapsize,\
                                             int tree_height)
{
	int i;
	unsigned long nblocks,naddrs;
	struct tree_level_info *ptr_info;
	
	ptr_info=(struct tree_level_info *)malloc((tree_height+1) * \
			sizeof(struct tree_level_info));

	naddrs=bitmapsize;
	for(i=tree_height; i<=0; i++)
	{
		nblocks=(unsigned long)ceil((double)naddrs/PER_BLOCK_DBA);

		ptr_info[i].level=i;
		ptr_info[i].num_of_addresses=naddrs;
		ptr_info[i].num_of_blocks=nblocks;

		naddrs=nblocks;
	}	
	
	return ptr_info;
}


dba_t init_hmamap(int fd, dba_t root, dba_t starting_addr, \
				unsigned long bitmapsize, int tree_height)
{
	dba_t ret_dba;
	struct tree_level_info *tree_info;
	unsigned long i, j, k, last_block_addrs, remainder;
	dba_t block[PER_BLOCK_DBA];
	dba_t addr=starting_addr;
	dba_t dba_write=root;

	tree_info=get_tree_levels_info(bitmapsize, tree_height);
	

	for(i=0; i<=tree_height; i++)
	{
		remainder = tree_info[i].num_of_addresses%PER_BLOCK_DBA;

		last_block_addrs=(remainder==0) ? PER_BLOCK_DBA : remainder;

		for(j=0; j<tree_info[i].num_of_blocks; j++)
		{
			if(j==tree_info[i].num_of_blocks-1)	
			{
				for(k=0; k<last_block_addrs; k++)
				{
					block[k]=addr;	
					DBA(addr)=DBA(addr) + 1;
				}
			}
			else
			{
				for(k=0;k<PER_BLOCK_DBA; k++)
				{
					block[k]=addr;	
					DBA(addr)=DBA(addr) + 1;
				}
			}
			write_block(fd, (char *)block, dba_write);
			DBA(dba_write)=DBA(dba_write) + 1;
		}
	}
	DBA(ret_dba) = DBA(dba_write) - 1;
	return ret_dba;
}


dba_t getmapblockaddr(struct BlockStore *bsp, int blocknum, int tree_height, dba_t root)
{
	dba_t block[PER_BLOCK_DBA];
	dba_t next_level_addr, mapblockaddr;
	
	int cur_level;
	int cur_level_addr_search  = blocknum;
	int next_level_addr_index  = 0;
	int next_level_addr_search = blocknum;

	PDEBUG_MAPBLOCKADDR("Entered getmapblockaddr:blocknum=%d\n",blocknum);
	PDEBUG_MAPBLOCKADDR("blocknum=%d\t%lu\n",blocknum,DBA(root));
	next_level_addr=root;

	for(cur_level = 0; cur_level <= tree_height; cur_level++)
	{
		PDEBUG_MAPBLOCKADDR("FOr: 1 GMAMAP_ROOT:%lu\n",DBA(root));
		cache_blockread(bsp, next_level_addr, (char *)block);

		cur_level_addr_search = next_level_addr_search;
		next_level_addr_index = cur_level_addr_search / \
						    pow((PER_BLOCK_DBA),(tree_height -  cur_level));

		next_level_addr = block[next_level_addr_index];

		next_level_addr_search = cur_level_addr_search % \
							(int)pow((PER_BLOCK_DBA),(tree_height - cur_level));
	}

	mapblockaddr = next_level_addr;

	PDEBUG_MAPBLOCKADDR("Exitting getmapblockaddr:%lu\n",DBA(mapblockaddr));
	return(mapblockaddr);
}


dba_t updatemapblockaddr(struct BlockStore *bsp, int blocknum, int tree_height,\
                         dba_t mapblockaddr, dba_t root)
{
	dba_t curfreedba,nextfreedba, olddba;
	dba_t block[PER_BLOCK_DBA];
	dba_t next_level_addr;
	
	int cur_level=0;
	int cur_level_addr_search  = blocknum;
	int next_level_addr_index  = 0;
	int next_level_addr_search = blocknum;
	bool block_status=DIRTY;

	PDEBUG_MAPBLOCKADDR("Entered\n");
	PDEBUG_MAPBLOCKADDR("INPUT: fd=%d\n",bsp->fd);
	PDEBUG_MAPBLOCKADDR("INPUT: blocknum=%d\n", blocknum);
	PDEBUG_MAPBLOCKADDR("INPUT: tree_height=%d\n", tree_height);
	PDEBUG_MAPBLOCKADDR("INPUT: newblockaddr=%lu\n", DBA(mapblockaddr));
	PDEBUG_MAPBLOCKADDR("INPUT: hmamaproot=%lu\n", DBA(root));

	next_level_addr=root;

	/*
 	*This is the place where root node will be processed 	
 	*/	
	
	cache_blockread(bsp, root, (char *)block);
	
	/*Get free block dba in here. This service will be provided by bitmap
 * module*/
	olddba=root;

	if(  (NULL == cache_lookup(root, &block_status)) ||\
          ((NULL != cache_lookup(root, &block_status))&&\
           (block_status==CLEAN)) )
	{
		PDEBUG_MAPBLOCKADDR("CALLING allocatedba\n");
		curfreedba=allocatedba(bsp);
		PDEBUG_MAPBLOCKADDR("RETURNUNG allocatedba with:%lu\n", DBA(curfreedba));
	
		PDEBUG_MAPBLOCKADDR("CALLING deallocatedba with:%lu\n",DBA(olddba));
		deallocatedba(bsp, olddba);
		PDEBUG_MAPBLOCKADDR("RETURNUNG deallocatedba\n");
		cache_blockdelete(olddba);

		root = curfreedba;
	}
	else
	{
		curfreedba=root;
	}
//Dangerous move undo immediately if did not work out.sn
	cur_level_addr_search = next_level_addr_search;
	next_level_addr_index = cur_level_addr_search / \
					    pow((PER_BLOCK_DBA),(tree_height -  cur_level));

	PDEBUG_MAPBLOCKADDR("Addressable node at this level=%g\n",\
				pow((PER_BLOCK_DBA),(tree_height -  cur_level)));

	next_level_addr = block[next_level_addr_index];
//Dangerous move undo immediately if did not work out.en


	for(cur_level = 0; cur_level < tree_height; cur_level++)
	{
		/*Get free block dba in here. This service will be provided by bitmap
		 * module*/

		if(  (NULL == cache_lookup(block[next_level_addr_index],&block_status)) ||\
			((NULL != cache_lookup(block[next_level_addr_index],\
                 &block_status)) && (block_status==CLEAN)) )
		{
			PDEBUG_MAPBLOCKADDR("CALLING allocatedba\n");
			nextfreedba=allocatedba(bsp);
			PDEBUG_MAPBLOCKADDR("RETURNUNG allcoatedba with: %lu\n", DBA(nextfreedba));

			PDEBUG_MAPBLOCKADDR("CALLING deallocatedba with:%lu\n",\
							DBA(block[next_level_addr_index]));
			deallocatedba(bsp,block[next_level_addr_index]);
			PDEBUG_MAPBLOCKADDR("RETURNUNG deallocatedba\n");
	
			cache_blockdelete(block[next_level_addr_index]);
			block[next_level_addr_index] = nextfreedba;
			cache_blockwrite(bsp, olddba, curfreedba, (char *)block);
			olddba=curfreedba;
			curfreedba=nextfreedba;
		}
			
		/*Get free block dba in here. This service will be provided by bitmap
		 * module*/


//Dangerous move undo immediately if did not work out.sn
		cur_level_addr_search = next_level_addr_search;
		next_level_addr_index = cur_level_addr_search / \
						    pow((PER_BLOCK_DBA),(tree_height -  cur_level));

		next_level_addr = block[next_level_addr_index];
//Dangerous move undo immediately if did not work out.sn


		next_level_addr_search = cur_level_addr_search % \
							(int)pow((PER_BLOCK_DBA),(tree_height - cur_level));

		cache_blockread(bsp, next_level_addr, (char *)block);
		
	}
		
	PDEBUG_MAPBLOCKADDR("next_level_addr_index=%d\n",next_level_addr_index);
	PDEBUG_MAPBLOCKADDR("block[next_level_addr_index]=%lu\n",\
					 DBA(block[next_level_addr_index]));

	deallocatedba(bsp,block[next_level_addr_index]);
	cache_blockdelete(block[next_level_addr_index]);
	block[next_level_addr_index]=mapblockaddr;
	cache_blockwrite(bsp, olddba, curfreedba,(char *)block);
	PDEBUG_MAPBLOCKADDR("Exiting:New Root=%lu\n", DBA(root));
	return root;	
}


void getmapblock(struct BlockStore *bsp, dba_t addr, char *mapblock)
{
	cache_blockread(bsp, addr, mapblock);
	return;
}

