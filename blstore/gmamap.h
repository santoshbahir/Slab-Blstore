#include "blockstore.h"

int height_of_gmamap;


/*******************************************************************************
 * This function returns the gmamap tree height.                               *
 * INPUT : hashmapsize in terms of number of blocks                            *
 * OUTPUT: Height of the hmamap tree                                           *
 ******************************************************************************/
int get_gmamap_height(int hashmapsize);


/*******************************************************************************
 * This function returns the size of gmamap tree in terms of number of blocks. *
 * Also, it stores the total number of nodes present at given level of gmamap  *
 * tree.                                                                       *
 * INPUT : hashmapsize in terms of number of blocks                            *
 *         Array to store the total number of nodes at particular level        *
 * OUTPUT: size of gmamap in terms of number of blocks                         *
 ******************************************************************************/
int get_gmamapsize(int hashmapsize, int level_nodes[]);


/*******************************************************************************
 * This function initializes the internal nodes and leaf nodes of gmamap tree  *
 * with correct DBA addresses.                                                 *
 * INPUT : File pointer to volume.                                             *
 *         hashmapsize in terms of number of blocks required for bitmap.       *
 *         dba address of root of the gmamap tree.                             *
 * OUTPUT: dba address of the last dba used for the gmamap tree.               *
 ******************************************************************************/
dba_t gmamap_init(int fd, int hashmapsize, dba_t root_gmamap);
