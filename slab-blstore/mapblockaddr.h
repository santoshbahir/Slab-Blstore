/*   This is the genric librari which will work for both HMAMAP and BITMA    */

/******************************************************************************
 * This is the datasturcture which stores the information about a particular  *
 * level of hmamap/gmamap tree. For all the level info linked list of this    *
 * datastructure is maintained.                                               *
 *****************************************************************************/
struct tree_level_info{
	int level;
	unsigned long num_of_blocks;
	unsigned long num_of_addresses;
};


/*******************************************************************************
 * This is the fucntion which calculates the tree-level information for hmamap *
 * and gmamap. It will calculate number of block, nad number of addresses to be*
 * stored at each level.                                                       *
 * Input :-  bitmapsize/hmapsize, height of a tree                             *
 * output:-  base pointer of data structure which holds the tree-level info    *
 ******************************************************************************/
struct tree_level_info * get_tree_levels_info(unsigned long bitmapsize,\
	  								 int tree_height);


/*******************************************************************************
 * This function will initiate the hmamap and gmamap tree. The blocks in the   *
 * tree contains only DBA address of the next level block-nodes in the tree.   *
 * In the beginning, all the tree blocks will be stroed in the contiguous way, *
 * so starting address of the tree is sufficient.                              *
 ******************************************************************************/
dba_t init_hmamap(int fd, dba_t root, dba_t starting_addr,\
				unsigned long bitmapsize, int tree_height);


/******************************************************************************
 * This function will return the dba_address for the sequential block number  *
 * of bitmap/hashmap received as a input.                                     *
 *****************************************************************************/
dba_t getmapblockaddr(struct BlockStore *bsp, int blocknum, \
					int tree_height, dba_t root);


/******************************************************************************
* This function updates the block address of the map block number blocknum    *
* with address mapblockaddr sent as input. This will return the new dba for   *
* the root for given tree.                                                    *
* this is very crucial function as this updates the whole hamamap/gmamap tree *
******************************************************************************/
dba_t updatemapblockaddr(struct BlockStore *bsp, int blocknum, int tree_height,\
					dba_t mapblockaddr, dba_t root);

/******************************************************************************
 * This fglknction is internal function to this module. This is a helper      *
 * function. It read the block from cache and cast the character stream to    *
 * sequence of the dba addressess. This will avoid the need to play with ptrs *
 * every time we read the block from the volume.                              *
 *****************************************************************************/
void getmapblock(struct BlockStore *bsp, dba_t addr, char *mapblock);
