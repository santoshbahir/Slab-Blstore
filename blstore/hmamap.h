#include "blockstore.h"

int height_of_hmamap;


/*******************************************************************************
 * This function returns the hmamap tree height.                               *
 * INPUT : bitmapsize in terms of number of blocks                             *
 * OUTPUT: Height of the hmamap tree                                           *
 ******************************************************************************/
int get_hmamap_height(int bitmapsize);


/*******************************************************************************
 * This function returns the size of hmamap tree in terms of number of blocks. *
 * Also, it stores the total number of nodes present at given level of hmamap  *
 * tree.                                                                       *
 * INPUT : bitmapsize in terms of number of blocks                             *
 *         Array to store the total number of nodes at particular level        *
 * OUTPUT: size of hmamap size in terms of number of blocks                    *
 ******************************************************************************/
int get_hmamapsize(int bitmapsize, int level_nodes[]);


/*******************************************************************************
 * This function initializes the internal nodes and leaf nodes of hmamap tree  *
 * with correct DBA addresses.                                                 *
 * INPUT : File pointer to volume.                                             *
 *         bitmapsize in terms of number of blocks required for bitmap.        *
 *         dba address of root of the hmamap tree.                             *
 * OUTPUT: dba address of the last dba used for the hmamap tree.               *
 ******************************************************************************/
dba_t hmamap_init(int fd, int bitmapsize, dba_t root_hmamap);
