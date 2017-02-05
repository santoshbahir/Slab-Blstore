#include <stdio.h>
#include "blstoremsg.h"
#include "blockstore.h"

int no_blks_for_hashmap;
int no_blks_for_bitmap;
int no_of_gma_per_blk;
int no_of_hma_per_blk;
int blocks_per_level[MAX_LEVEL];

void cal_global_parameter(dba_t nBlocks);

size_t read_block(int fd, char *buffer, dba_t dba_addr);
size_t write_block(int fd, char *buffer, dba_t dba_addr);

void test_read(int fd);
