#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include "blockstore.h"
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <libgen.h>

#define DEBUG if (0)

#define ULONG(x) ((unsigned long)(x))

#define CHECKRESULT(result, res, ret) do {		\
    if ((result) != (res)) { \
      fprintf(stderr, "FAIL: [Line %d] Unexpected Result. Expected %s; Obtained %s\n", \
	     __LINE__, result_name((res)), result_name((result))); \
      return ret; \
    } \
  } while(0)

#define SHOULDBE(result, res, fun, ret) do {	\
    (result) = (fun);\
    CHECKRESULT(result, res, ret);			\
  } while(0)

#define SHOULDBEOK(result, fun, ret) SHOULDBE(result, result_OK, fun, ret)

#define CHECKTRANS(t) do {\
    if (t == NULL) { \
      fprintf(stderr, "FAIL: [Line %d] Could not allocate transaction\n", \
              __LINE__); \
    } \
  } while (0);

typedef struct {
  int shouldRemove;		/* should we remove any existing store? */
  dba_t nBlocks;		/* size of store to create */
  const char *storeName;	/* name of store to create */
} testargs_t;
testargs_t args;

int 
process_args(int argc, char *const argv[], testargs_t* args)
{
  const char *const progName = basename(argv[0]);

  // Stuff for argument processing:
  int c;
  extern char *optarg;
  extern int optopt;
  extern int opterr;
  extern int optind;

  args->shouldRemove = 0;	/* until otherwise proven */
  DBA(args->nBlocks) = 0;	/* until otherwise proven */

  opterr = 0;			/* suppress library commentary */

  while ((c = getopt(argc, argv, "rs:")) != -1) {
    switch(c) {
    case ':':
      {
	fprintf(stderr, "Missing option parameter\n");
	exit(1);
      }
    case 's':
      {
	DBA(args->nBlocks) = strtoul(optarg, 0, 0);
	break;
      }
    case 'r':
      {
	args->shouldRemove = 1;
	break;
      }
    case '?':
      {
	fprintf(stderr, "Unknown option %c\n", optopt);
	exit(1);
      }
    }
  }

  argv += optind;
  argc -= optind;

  if (argc != 1) {
    fprintf(stderr, "%s requires a volume filename\n", progName);
    exit(1);
  }

  args->storeName = argv[0];

  if(args->shouldRemove)
    unlink(args->storeName);

  // Step past the repository name argument:
  optind++;
  return argc;
}

void
create_close_test(const char *const storeName, const dba_t nBlocks)
{
  result_t result;
  BlockStore *blockStore;
  struct stat statbuf;
  size_t total_size;

  fprintf(stdout, "Trying to create Block-Store... ");
  blockStore = bl_create(storeName, nBlocks, &result);
  if (result != result_OK) {
    fprintf(stdout, "FAILED\n");
    fprintf(stderr, "Result creating new store: %s\n", result_name(result));
    exit(1);
  }

  if(blockStore == NULL) {
    fprintf(stdout, "FAILED\n");
    fprintf(stderr, "PANIC: blockstore created by bl_create is NULL even");
    fprintf(stderr, " though the result is result_OK\n");
    exit(1);
  } 
  else 
    fprintf(stdout, "OK\n");
  
  if(DBA(nBlocks) == 0) {
    /* I am relying on the fact that for creating a new file
       the min length is 2 blocks for the headers. */
    
    if (stat(storeName, &statbuf) < 0) {
      fprintf(stdout, "FAILED\n");
      fprintf(stderr, "Could not stat new store: %s\n", storeName);
      exit(1);
    }
    total_size = statbuf.st_blksize * statbuf.st_blocks;
    if(total_size != DBA(nBlocks) * BLK_SZ) {
      fprintf(stdout, "FAILED\n");
      fprintf(stderr, "Expected size %lu != Actual Size %lu",
	      DBA(nBlocks) * BLK_SZ, ULONG(total_size));
      exit(1);
    }
  } 

  fprintf(stdout, "Trying to close Block-Store... ");
  result = bl_close(blockStore);
  if (result != result_OK) {
    fprintf(stdout, "FAILED\n");
    fprintf(stderr, "Result closing new store: %s\n", result_name(result));
    exit(1);
  }
  else 
    fprintf(stdout, "OK\n");
}

void
open_close_test(const char *const storeName)
{
  result_t result;
  BlockStore *blockStore;

  fprintf(stdout, "Opening existing store... ");
  blockStore = bl_open(storeName, &result);
  if (result != result_OK) {
    fprintf(stdout, "FAILED\n");
    fprintf(stderr, "Result opening existing store: %s\n", result_name(result));
    exit(1);
  }

  if(blockStore == NULL) {
    fprintf(stdout, "FAILED\n");
    fprintf(stderr, "PANIC: blockstore opened by bl_open is NULL even");
    fprintf(stderr, " though the result is result_OK\n");
    exit(1);
  }
  else 
    fprintf(stdout, "OK\n");

  fprintf(stdout, "Trying to close Block-Store... ");
  result = bl_close(blockStore);
  if (result != result_OK) {
    fprintf(stdout, "FAILED\n");
    fprintf(stderr, "Result closing new store: %s\n", result_name(result));
    exit(1);
  }
  else 
    fprintf(stdout, "OK\n");
}


void
open_close_detailed_test(const char *const storeName)
{
  result_t result; 
  bl_transaction_t trans;
  BlockStore *blockStore;
  lba_t mylba, templba;
  obByte buf[BLK_SZ], wbuf1[BLK_SZ];
  LBA(mylba) = 10;
  LBA(templba) = 20;

  printf("Trying Read/write survival across open/close ... ");
  blockStore = bl_open(storeName, &result);
  CHECKRESULT(result, result_OK,);
  trans=bl_BeginTransaction(blockStore);
  CHECKTRANS(trans);
  
  result = bl_allocLBA(trans, 25, &mylba);

  memset(wbuf1, '\1', BLK_SZ);
  LBA(templba) = LBA(mylba) + 1;
  SHOULDBEOK(result, bl_write(trans, templba, wbuf1),);
  
  memset(wbuf1, '\7', BLK_SZ);
  LBA(templba) = LBA(mylba) + 7;
  SHOULDBEOK(result, bl_write(trans, templba, wbuf1),);

  memset(wbuf1, '\11', BLK_SZ);
  LBA(templba) = LBA(mylba) + 11;
  SHOULDBEOK(result, bl_write(trans, templba, wbuf1),);

  SHOULDBEOK(result, bl_commit(trans),);
  SHOULDBEOK(result, bl_close(blockStore),);

  blockStore = bl_open(storeName, &result);
  CHECKRESULT(result, result_OK,);
  trans=bl_BeginTransaction(blockStore);
  CHECKTRANS(trans);

  bzero(buf, BLK_SZ);
  memset(wbuf1, '\1', BLK_SZ);
  LBA(templba) = LBA(mylba) + 1;
  SHOULDBEOK(result, bl_read(trans, templba, buf),);
  if(memcmp(wbuf1, buf, BLK_SZ) != 0) {
    fprintf(stdout, "FAILED\n");
    fprintf(stderr, "FAILED read/write data integrity test\n");
    exit(1);
  }

  bzero(buf, BLK_SZ);
  memset(wbuf1, '\7', BLK_SZ);
  LBA(templba) = LBA(mylba) + 7;
  SHOULDBEOK(result, bl_read(trans, templba, buf),);
  if(memcmp(wbuf1, buf, BLK_SZ) != 0) {
    fprintf(stdout, "FAILED\n");
    fprintf(stderr, "FAILED read/write data integrity test\n");
    exit(1);
  }

  bzero(buf, BLK_SZ);
  memset(wbuf1, '\11', BLK_SZ);
  LBA(templba) = LBA(mylba) + 11;
  SHOULDBEOK(result, bl_read(trans, templba, buf),);
  if(memcmp(wbuf1, buf, BLK_SZ) != 0) {
    fprintf(stdout, "FAILED\n");
    fprintf(stderr, "FAILED read/write data integrity test\n");
    exit(1);
  }

  blockStore = bl_open(storeName, &result);
  CHECKRESULT(result, result_OK,);
  trans=bl_BeginTransaction(blockStore);
  CHECKTRANS(trans);

  fprintf(stdout, "OK\n");
}


void
abort_test(const char *const storeName)
{
  result_t result;
  BlockStore *blockStore;
  bl_transaction_t trans;

  blockStore = bl_open(storeName, &result);
  CHECKRESULT(result, result_OK,);

  fprintf(stdout, "Trying to begin first transaction... ");
  trans=bl_BeginTransaction(blockStore);
  CHECKTRANS(trans);

  if(trans == NULL) {
    fprintf(stdout, "FAILED\n");
    fprintf(stderr, "PANIC: bl_BeginTransaction returned NULL\n");
    exit(1);
  }
  else
    fprintf(stdout, "OK\n");
   
  fprintf(stdout, "Trying to Abort the transaction... ");
  SHOULDBE(result, result_TxAbort, bl_abort(trans),);
  if (result != result_TxAbort) {
    fprintf(stdout, "FAILED\n");
    fprintf(stderr, "Result Aborting: %s\n", result_name(result));
    exit(1);
  }
  else
    fprintf(stdout, "OK\n");

  SHOULDBEOK(result, bl_close(blockStore),);
}

void
commit_test(const char *const storeName)
{
  result_t result;
  BlockStore *blockStore;
  bl_transaction_t trans;

  blockStore = bl_open(storeName, &result);
  CHECKRESULT(result, result_OK,);

  fprintf(stdout, "Trying to begin transaction... ");
  trans = bl_BeginTransaction(blockStore);
  if(trans == NULL) {
    fprintf(stdout, "FAILED\n");
    fprintf(stderr, "PANIC: bl_BeginTransaction returned NULL\n");
    exit(1);
  }
  else
    fprintf(stdout, "OK\n");
   
  fprintf(stdout, "Trying to Commit the transaction... ");
  SHOULDBEOK(result, bl_commit(trans),);
  if (result != result_OK) {
    fprintf(stdout, "FAILED\n");
    fprintf(stderr, "Result Aborting: %s\n", result_name(result));
    exit(1);
  }
  else
    fprintf(stdout, "OK\n");

  SHOULDBEOK(result, bl_close(blockStore),);
}

 
void
alloc_lba_test(const char *const storeName)
{
  result_t result;
  BlockStore *blockStore;
  bl_transaction_t trans;
  lba_t first_pLBA, second_pLBA;

  LBA(first_pLBA) = 10;
  LBA(second_pLBA) = 20;

  blockStore = bl_open(storeName, &result);
  CHECKRESULT(result, result_OK,);
  trans=bl_BeginTransaction(blockStore);
  CHECKTRANS(trans);

  fprintf(stdout, "Trying to acquire 10 LBAs... ");
  result = bl_allocLBA(trans, 10, &first_pLBA);
  if (result != result_OK) {
    fprintf(stdout, "FAILED\n");
    fprintf(stderr, "Result aquiring 10 lbas: %s\n", result_name(result));
    exit(1);
  }
  else 
    fprintf(stdout, "OK\n");

  fprintf(stdout, "Trying to acquire 10 more LBAs... ");
  result = bl_allocLBA(trans, 10, &second_pLBA);
  if (result != result_OK) {
    fprintf(stdout, "FAILED\n");
    fprintf(stderr, "Result acquiring 10 lbas (2nd time): %s\n", result_name(result));
    exit(1);
  }
  else 
    fprintf(stdout, "OK\n");
  
  fprintf(stdout, "Comparing LBAs obtained... ");
  if(LBA(first_pLBA) - LBA(second_pLBA) < 10) {
    fprintf(stdout, "FAILED\n");
    fprintf(stderr, "LBA allocation error: 1st = %llu 2nd = %llu [Diff ~>= 10]\n", 
	    LBA(first_pLBA), LBA(second_pLBA));
    exit(1);
  }
  else
    fprintf(stdout, "OK\n");

  SHOULDBEOK(result, bl_commit(trans),); 
  SHOULDBEOK(result, bl_close(blockStore),);
}


void
alloc_lba_across_open_close_test(const char *const storeName)
{
  result_t result;
  BlockStore *blockStore;
  bl_transaction_t trans;
  lba_t first_pLBA, second_pLBA;

  LBA(first_pLBA) = 10;
  LBA(second_pLBA) = 20;

  blockStore = bl_open(storeName, &result);
  CHECKRESULT(result, result_OK,);
  trans=bl_BeginTransaction(blockStore);
  CHECKTRANS(trans);

  fprintf(stdout, "Testing Comitted LBA allocation across open/close ...");
  SHOULDBEOK(result, bl_allocLBA(trans, 10, &first_pLBA),);

  SHOULDBEOK(result, bl_commit(trans),);
  SHOULDBEOK(result, bl_close(blockStore),);

  blockStore = bl_open(storeName, &result);
  CHECKRESULT(result, result_OK,);
  trans=bl_BeginTransaction(blockStore);
  CHECKTRANS(trans);

  SHOULDBEOK(result, bl_allocLBA(trans, 10, &second_pLBA),);
  
  if(LBA(first_pLBA) - LBA(second_pLBA) < 10) {
    fprintf(stdout, "FAILED\n");
    fprintf(stderr, "LBA allocation error: 1st = %llu 2nd = %llu [Diff ~>= 10]\n", 
	    LBA(first_pLBA), LBA(second_pLBA));
    exit(1);
  }
  else
    fprintf(stdout, "OK\n");

  SHOULDBE(result, result_TxAbort, bl_abort(trans),);
  SHOULDBEOK(result, bl_close(blockStore),);
}

void
drop_non_existent_lba_test(const char *const storeName)
{
  result_t result;
  BlockStore *blockStore;
  bl_transaction_t trans;
  lba_t dead_lba;

  LBA(dead_lba) = 10;

  blockStore = bl_open(storeName, &result);
  CHECKRESULT(result, result_OK,);
  trans=bl_BeginTransaction(blockStore);
  CHECKTRANS(trans);

  SHOULDBEOK(result, bl_allocLBA(trans, 1, &dead_lba),);
  SHOULDBEOK(result, bl_drop(trans, dead_lba),);

  fprintf(stdout, "Testing Drop of non-existant LBA... ");
  result = bl_drop(trans, dead_lba);
  if (result == result_OK) {
    fprintf(stdout, "FAILED\n");
    fprintf(stderr, "Result dropping lba %llu: %s\n", 
	    LBA(dead_lba), result_name(result));
    exit(1);
  }
  else
    fprintf(stdout, "OK\n");
  
  SHOULDBE(result, result_TxAbort, bl_abort(trans),);
  SHOULDBEOK(result, bl_close(blockStore),);
}

void
write_test(const char *const storeName)
{
  result_t result;
  BlockStore *blockStore;
  bl_transaction_t trans;
  lba_t mylba;
  obByte wbuf1[BLK_SZ];

  LBA(mylba) = 10;

  blockStore = bl_open(storeName, &result);
  CHECKRESULT(result, result_OK,);
  trans=bl_BeginTransaction(blockStore);
  CHECKTRANS(trans);
 
  result = bl_allocLBA(trans, 1, &mylba);
  
  memset(wbuf1, '\1', BLK_SZ);
  fprintf(stdout, "Trying to write 1 block of 1s to LBA %llu... ",
	  LBA(mylba));
  result = bl_write(trans, mylba, wbuf1);
  if (result != result_OK) {
    fprintf(stdout, "FAILED\n");
    fprintf(stderr, "Result writing block at LBA %llu: %s\n", 
	    LBA(mylba), result_name(result));
    exit(1);
  }
  else
    fprintf(stdout, "OK\n");
  
  SHOULDBEOK(result, bl_commit(trans),);
  SHOULDBEOK(result, bl_close(blockStore),);
}

void
read_test(const char *const storeName)
{
  result_t result;
  BlockStore *blockStore;
  bl_transaction_t trans;
  lba_t mylba;
  obByte buf[BLK_SZ], wbuf1[BLK_SZ];

  LBA(mylba) = 10;

  bzero(buf, BLK_SZ);
  memset(wbuf1, '\1', BLK_SZ);
 
  blockStore = bl_open(storeName, &result);
  CHECKRESULT(result, result_OK,);
  trans=bl_BeginTransaction(blockStore);
  CHECKTRANS(trans);

  SHOULDBEOK(result, bl_allocLBA(trans, 1, &mylba),);
  
  SHOULDBEOK(result, bl_write(trans, mylba, wbuf1),);

  fprintf(stdout, "Trying to read ... ");
  result = bl_read(trans, mylba, buf);
  if (result != result_OK) {
    fprintf(stdout, "FAILED\n");
    fprintf(stderr, "Result reading LBA %llu: %s\n", 
	    LBA(mylba), result_name(result));
    exit(1);
  }
  else
    fprintf(stdout, "OK\n");
  
  fprintf(stdout, "Checking for Data Integrity... ");
  if(memcmp(wbuf1, buf, BLK_SZ) == 0)
    fprintf(stdout, "OK\n");
  else {
    fprintf(stdout, "FAILED\n");
    fprintf(stderr, "FAILED read/write data integrity test\n");
    exit(1);
  }

  SHOULDBEOK(result, bl_commit(trans),);
  SHOULDBEOK(result, bl_close(blockStore),);
}

void
drop_existent_lba_test(const char *const storeName)
{
  result_t result;
  BlockStore *blockStore;
  bl_transaction_t trans;
  lba_t mylba;

  LBA(mylba) = 10;
  dba_t mydba;

  obByte wbuf1[BLK_SZ];

  memset(wbuf1, '\1', BLK_SZ);

  blockStore = bl_open(storeName, &result);
  CHECKRESULT(result, result_OK,);
  trans=bl_BeginTransaction(blockStore);
  CHECKTRANS(trans);

  SHOULDBEOK(result, bl_allocLBA(trans, 1, &mylba),);
  SHOULDBEOK(result, bl_write(trans, mylba, wbuf1),);

  fprintf(stdout, "Testing Drop of existing, written-to LBA... ");
  result = bl_drop(trans, mylba);
  if (result != result_OK) {
    fprintf(stdout, "FAILED\n");
    fprintf(stderr, "Result dropping lba %llu: %s\n", 
	    LBA(mylba), result_name(result));
    exit(1);
  }

  result = bl_lookup_lba(trans, mylba, &mydba);
  if (result != result_NotFound) {
    fprintf(stdout, "FAILED\n");
    fprintf(stderr, "Result looking up lba %llu: %s. The alleged dba is %lu.\n", 
	    LBA(mylba), result_name(result), DBA(mydba));
    exit(1);
  }
  else
    fprintf(stdout, "OK\n");
  
  SHOULDBE(result, result_TxAbort, bl_abort(trans),);
  SHOULDBEOK(result, bl_close(blockStore),);
}


void
read_a_random_lba_test(const char *const storeName)
{
  result_t result;
  BlockStore *blockStore;
  bl_transaction_t trans;
  lba_t dead_lba;

  LBA(dead_lba) = 10;

  blockStore = bl_open(storeName, &result);
  CHECKRESULT(result, result_OK,);
  trans=bl_BeginTransaction(blockStore);
  CHECKTRANS(trans);

  obByte buf[BLK_SZ];
  
  fprintf(stdout, "Read a random LBA test ... ");
  LBA(dead_lba) = 0xdeadface;
  result = bl_read(trans, dead_lba, buf);
  if (result == result_OK) {
    fprintf(stdout, "FAILED\n");
    fprintf(stderr, "Result reading a random LBA %llu: %s\n", 
	    LBA(dead_lba), result_name(result));
    exit(1);
  }
  else
    fprintf(stdout, "OK\n");
  
  SHOULDBE(result, result_TxAbort, bl_abort(trans),); 
  SHOULDBEOK(result, bl_close(blockStore),);
}

void
re_write_test(const char *const storeName)
{
  result_t result;
  BlockStore *blockStore;
  bl_transaction_t trans;
  lba_t mylba;
  dba_t first_dba, second_dba, third_dba;
  obByte wbuf1[BLK_SZ];


  LBA(mylba) = 10;

  fprintf(stdout, "Re-Write test ... ");
  blockStore = bl_open(storeName, &result);
  CHECKRESULT(result, result_OK,);
  trans=bl_BeginTransaction(blockStore);
  CHECKTRANS(trans);
  
  SHOULDBEOK(result, bl_allocLBA(trans, 1, &mylba),);
  
  memset(wbuf1, '\1', BLK_SZ);
  SHOULDBEOK(result, bl_write(trans, mylba, wbuf1),);
  
  SHOULDBEOK(result, bl_lookup_lba(trans, mylba, &first_dba),);
  if(bl_dba_is_now_allocated(trans, first_dba) == 0) {
    fprintf(stdout, "FAILED\n");
    fprintf(stderr, "DBA %lu written for LBA %llu is NOW still free\n", 
	    DBA(first_dba), LBA(mylba));
    exit(1);
  }
  
  SHOULDBEOK(result, bl_commit(trans),);

  /* Now we know that there should be something at first_dba */
  trans=bl_BeginTransaction(blockStore);
  CHECKTRANS(trans);

  /* This one should allocate a new dba: */
  memset(wbuf1, '\2', BLK_SZ);
  SHOULDBEOK(result, bl_write(trans, mylba, wbuf1),);

  SHOULDBEOK(result, bl_lookup_lba(trans, mylba, &second_dba),);

  /* This one should re-use second_dba: */
  memset(wbuf1, '\3', BLK_SZ);
  SHOULDBEOK(result, bl_write(trans, mylba, wbuf1),);
  SHOULDBEOK(result, bl_lookup_lba(trans, mylba, &third_dba),);

  if (DBA(first_dba) == DBA(second_dba)) {
    fprintf(stdout, "FAILED. Overwrote LBA %llu at DBA %lu\n", LBA(mylba), DBA(second_dba));
    exit(1);
  }
  
  if (DBA(second_dba) != DBA(third_dba)) {
    fprintf(stdout, "FAILED. Did not over write newly allocated block.\n");
    fprintf(stdout, "\tLBA = %llu, New DBA = %lu. Another new DBA = %lu\n", LBA(mylba), DBA(second_dba), DBA(third_dba));
    exit(1);
  }

  if(bl_dba_was_allocated(trans, first_dba) == 0) {
    fprintf(stdout, "FAILED\n");
    fprintf(stderr, "DBA %lu written for LBA %llu in previous trans \"WAS\" still free\n",
	    DBA(first_dba), LBA(mylba));
    exit(1);
  }

  if(bl_dba_is_now_allocated(trans, second_dba) == 0) {
    fprintf(stdout, "FAILED\n");
    fprintf(stderr, "New DBA %lu written for LBA %llu IS still free\n",
	    DBA(second_dba), LBA(mylba));
    exit(1);
  }

  if(bl_dba_is_now_allocated(trans, first_dba) != 0) {
    fprintf(stdout, "FAILED\n");
    fprintf(stderr, "Old DBA %lu written for LBA %llu IS still free\n",
	    DBA(first_dba), LBA(mylba));
    exit(1);
  }
  fprintf(stdout, "OK\n");

  SHOULDBEOK(result, bl_commit(trans),);
  SHOULDBEOK(result, bl_close(blockStore),);
}

void
vanish_after_abort_test(const char *const storeName)
{
  result_t result;
  BlockStore *blockStore;
  bl_transaction_t trans;
  lba_t mylba;
  dba_t mydbas[5], mydba;
  obByte wbuf1[BLK_SZ];
  int i;
 
  LBA(mylba) = 10;

  printf("Testing non-persistance of LBAs and DBAs across Abort ... ");
  blockStore = bl_open(storeName, &result);
  CHECKRESULT(result, result_OK,);
  trans=bl_BeginTransaction(blockStore);
  CHECKTRANS(trans);

  SHOULDBEOK(result, bl_allocLBA(trans, 5, &mylba),);
   
  for(i=0; i<5; i++) {
    memset(wbuf1, i+2, BLK_SZ); 
    SHOULDBEOK(result, bl_write(trans, mylba, wbuf1),);
    SHOULDBEOK(result, bl_lookup_lba(trans, mylba, &mydbas[i]),);

    if(bl_dba_is_now_allocated(trans, mydbas[i]) == 0) {
      fprintf(stdout, "FAILED\n");
      fprintf(stderr, "DBA %lu written for LBA %llu is NOW still free\n", 
	      DBA(mydbas[i]), LBA(mylba));
      exit(1);
    }    
    LBA(mylba)++;
  }
  LBA(mylba) -= i;

  SHOULDBE(result, result_TxAbort, bl_abort(trans),);

  trans=bl_BeginTransaction(blockStore);
  CHECKTRANS(trans);
  
  for(i=0; i<5; i++) {
    result = bl_lookup_lba(trans, mylba, &mydba);
    if(result == result_OK) {
      fprintf(stdout, "FAILED\n");
      fprintf(stderr, "LBA %llu-> DBA %lu persistent across aborts\n", 
	      LBA(mylba), DBA(mydba));
      exit(1);
    }

    if(bl_dba_was_allocated(trans, mydbas[i]) != 0) {
      fprintf(stdout, "FAILED\n");
      fprintf(stderr, "DBA %lu newly written for LBA %llu in the aborted transaction WAS Allocated\n", 
	      DBA(mydbas[i]), LBA(mylba));
      exit(1);
    }    
    
    LBA(mylba)++;
  }
  fprintf(stdout, "OK\n");

  SHOULDBE(result, result_TxAbort, bl_abort(trans),);
  SHOULDBEOK(result, bl_close(blockStore),);
}


void
lba_dba_map_creation_test(const char *const storeName)
{
  result_t result;
  BlockStore *blockStore;
  bl_transaction_t trans;
  lba_t mylba;
  dba_t mydba;
  obByte wbuf1[BLK_SZ];
  int i;

  LBA(mylba) = 10;

  printf("Testing lba->dba map creation ... ");
 
  blockStore = bl_open(storeName, &result);
  CHECKRESULT(result, result_OK,);
  trans=bl_BeginTransaction(blockStore);
  CHECKTRANS(trans);

  SHOULDBEOK(result, bl_allocLBA(trans, 5, &mylba),);

  for(i=0; i<5; i++) {
    memset(wbuf1, i, BLK_SZ); 
    SHOULDBEOK(result, bl_write(trans, mylba, wbuf1),);
    LBA(mylba)++;
  }
  LBA(mylba) -= i;

  for(i=0; i<5; i++) {
    result = bl_lookup_lba(trans, mylba, &mydba);
    if(result != result_OK) {
      fprintf(stdout, "FAILED\n");
      fprintf(stderr, "LBA %llu-> DBA ??? mapping does not exists after write\n", 
	      LBA(mylba));
      exit(1);
    }
    LBA(mylba)++;
  }

  fprintf(stdout, "OK\n");

  SHOULDBEOK(result, bl_commit(trans),);
  SHOULDBEOK(result, bl_close(blockStore),);
}

unsigned long
space_availablity_test(const char *const storeName)
{
  result_t result;
  BlockStore *blockStore;
  bl_transaction_t trans;
  unsigned long nFreeBlocks=0;
  lba_t mylba;
  obByte wbuf1[BLK_SZ];

  LBA(mylba) = 10;

  fprintf(stdout, "Space Availability test ... ");

  memset(wbuf1, '\1', BLK_SZ);

  blockStore = bl_open(storeName, &result);
  CHECKRESULT(result, result_OK, 0);
  trans=bl_BeginTransaction(blockStore);
  CHECKTRANS(trans);

  while(1) {
    result = bl_allocLBA(trans, 1, &mylba);
    if(result == result_NoSpace) {
      break;
    }
    else if(result != result_OK) {
      fprintf(stdout, "Space Availability test ... FAIL. [%s]\n",
	      result_name(result));
      return 0;
    }

    result = bl_write(trans, mylba, wbuf1);    
    if(result == result_NoSpace) {
      break;
    }
    else if(result != result_OK) {
      fprintf(stdout, "Space Availability test ... FAIL. [%s]\n",
	      result_name(result));
      return 0;
    }

    nFreeBlocks++;
    if (nFreeBlocks && (nFreeBlocks % 100) == 0)
      fprintf(stdout, "\rSpace Availability test ...  %lu", 
	      nFreeBlocks);
  }
  
  SHOULDBE(result, result_TxAbort, bl_abort(trans), 0);
  SHOULDBEOK(result, bl_close(blockStore), 0);

  fprintf(stdout, "Space Availability test ...  OK. [nFreeBlocks = %lu (approx)]\n",
	  nFreeBlocks);

  return nFreeBlocks;
}


void
parent_child_test(const char *const storeName)
{
  result_t result;
  BlockStore *blockStore;
  bl_transaction_t trans;
  lba_t lba1, lba2;
  dba_t mydba;
  obByte zero[BLK_SZ], buf[BLK_SZ], wbuf1[BLK_SZ], wbuf2[BLK_SZ];
  int pid, status, res;

  fprintf(stdout, "Parent/Child test ... ");
  LBA(lba1) = 10;
  LBA(lba2) = 20;

  memset(zero,  '\0', BLK_SZ);
  memset(buf,   '\0', BLK_SZ);
  memset(wbuf1, '\1', BLK_SZ);
  memset(wbuf2, '\2', BLK_SZ);
  
  blockStore = bl_open(storeName, &result);
  CHECKRESULT(result, result_OK,);
  trans=bl_BeginTransaction(blockStore);
  CHECKTRANS(trans);

  SHOULDBEOK(result, bl_allocLBA(trans, 1, &lba1),);
  SHOULDBEOK(result, bl_allocLBA(trans, 1, &lba2),);

  SHOULDBEOK(result, bl_write(trans, lba1, zero),);
  SHOULDBEOK(result, bl_write(trans, lba2, zero),);

  SHOULDBEOK(result, bl_commit(trans),);
  SHOULDBEOK(result, bl_close(blockStore),);

  if((pid = fork()) == -1) {
    fprintf(stderr, "Could NOT fork()\n");
    exit(1);
  }
  else if(pid == 0) {
    /* Child */
    blockStore = bl_open(storeName, &result);
    CHECKRESULT(result, result_OK,);
    trans=bl_BeginTransaction(blockStore);
    CHECKTRANS(trans);
    SHOULDBEOK(result, bl_write(trans, lba1, wbuf1),);
    SHOULDBEOK(result, bl_commit(trans),);
    
    trans=bl_BeginTransaction(blockStore);
    CHECKTRANS(trans);
    SHOULDBEOK(result, bl_write(trans, lba2, wbuf2),);
    
    exit(1); /* The client failure */
  }
  else {
    /* Parent */
    res = wait(&status);
    if(res == -1) {
      fprintf(stderr, "wait failed\n");
      exit(1);
    }
    blockStore = bl_open(storeName, &result);
    CHECKRESULT(result, result_OK,);
    trans=bl_BeginTransaction(blockStore);
    CHECKTRANS(trans);
    
    result = bl_lookup_lba(trans, lba1, &mydba);
    if(result != result_OK) {
      fprintf(stdout, "FAILED\n");
      fprintf(stderr, "Result lookingup of Written-to, commited LBA = %llu is %s\n", 
	      LBA(lba1), result_name(result));
      exit(1);
    }
    
    SHOULDBEOK(result, bl_read(trans, lba1, buf),);
    if(memcmp(wbuf1, buf, BLK_SZ) != 0) {
      fprintf(stdout, "FAILED\n");
      fprintf(stderr, "FAILED read of committed block\n");
      exit(1);
    }

    SHOULDBEOK(result, bl_read(trans, lba2, buf),);
    if(memcmp(wbuf2, buf, BLK_SZ) == 0) {
      fprintf(stdout, "FAILED\n");
      fprintf(stderr, "FAILED updates to uncommitted blocks are visible across failure\n");
      exit(1);
    }
    if(memcmp(zero, buf, BLK_SZ) != 0) {
      fprintf(stdout, "FAILED\n");
      fprintf(stderr, "FAILED uncommitted blocks changed across failure\n");
      exit(1);
    }

    SHOULDBEOK(result, bl_commit(trans),);
    fprintf(stdout, "OK\n");
  }
}

inline static void
set(obByte *buf, const unsigned long val)
{
  unsigned long len = BLK_SZ/sizeof(unsigned long), i;
  unsigned long *arr = (unsigned long *) buf;
  for(i=0; i<len; i++)
    arr[i] = val;
} 

inline static result_t
cmp(const obByte *buf, const unsigned long val)
{
  unsigned long len = BLK_SZ/sizeof(unsigned long), i;
  unsigned long *arr = (unsigned long *) buf;
  for(i=0; i<len; i++)
    if(arr[i] != val) {
      fprintf(stderr, "Comparison for data integrity [%lu] Failed\n", val);
      exit(1);
    }
  return result_OK;
}

static bool
sparsity_test_helper(BlockStore *blockStore, 
		     const unsigned long nFreeBlocks,
		     const lba_t *const lbaList,
		     const unsigned long long rand_max)
{
  result_t result;
  bl_transaction_t trans;
  unsigned long i, j, k;
  unsigned long no_updates_per_pass=nFreeBlocks/4;
  lba_t mylba;
  dba_t mydba;
  obByte buf[BLK_SZ], wbuf[BLK_SZ];

  for(j=0; j<4; j++) {
    trans=bl_BeginTransaction(blockStore);
    CHECKTRANS(trans);
    
    for(i=0; i<no_updates_per_pass; i++) {
      k = lrand48() % nFreeBlocks;
      mylba = lbaList[k];
      DEBUG printf("1. k = %lu LBA = %llu\n", k, LBA(lbaList[k]));
      
      SHOULDBEOK(result, bl_read(trans, mylba, buf), false);
      SHOULDBEOK(result, cmp(buf, k), false);
      
      set(wbuf, k);
      SHOULDBEOK(result, bl_write(trans, mylba, wbuf), false);
      SHOULDBEOK(result, bl_lookup_lba(trans, mylba, &mydba), false);  
    }
    SHOULDBEOK(result, bl_commit(trans), false);
  }

  return true;
}

void
sparsity_test(const char *const storeName, unsigned long nFreeBlocks)
{
  result_t result;
  BlockStore *blockStore;
  bl_transaction_t trans;
  lba_t mylba;
  dba_t mydba;
  obByte wbuf[BLK_SZ];
  lba_t *lbaList;
  unsigned long long rand_max = 1<<24; /* I would have said 64,
					  but lrand48() will be unhappy */
  unsigned long i = 0;
  bool p1OK = false;
  bool p2OK = false;

  fprintf(stdout, "Trying Sparsity test ... ");
  
  DEBUG printf("nFreeBlocks = %lu ", nFreeBlocks); 
  
  nFreeBlocks = (nFreeBlocks * 4) / 10;
  // Cap nFreeBlocks at 2048 for testing purposes:
  if (nFreeBlocks > 2048) nFreeBlocks = 2048;
  
  DEBUG printf("n = %lu\n", nFreeBlocks); 
  lbaList = (lba_t *)malloc(nFreeBlocks * sizeof(lba_t));

  blockStore = bl_open(storeName, &result);
  CHECKRESULT(result, result_OK,);
  trans=bl_BeginTransaction(blockStore);
  CHECKTRANS(trans);
  
  for(i=0; i<nFreeBlocks; i++) {
    LBA(lbaList[i]) = ((unsigned long long)lrand48()) % rand_max;
    SHOULDBEOK(result, bl_allocLBA(trans, 1, &lbaList[i]),);
    
    set(wbuf, i);
    mylba = lbaList[i];
    SHOULDBEOK(result, bl_write(trans, mylba, wbuf),);
    SHOULDBEOK(result, bl_lookup_lba(trans, mylba, &mydba),);
    DEBUG printf("Wrote [%lu] LBA = %llu to dba %lu\n", i, LBA(lbaList[i]), DBA(mydba));
  }
  SHOULDBEOK(result, bl_commit(trans),);
  
  /* First set of passes */
  p1OK = sparsity_test_helper(blockStore, nFreeBlocks, 
			      lbaList, rand_max);

  /* Second set of passes */
  p2OK = sparsity_test_helper(blockStore, nFreeBlocks, 
			      lbaList, rand_max);

  SHOULDBEOK(result, bl_close(blockStore),);
    
  if(p1OK && p2OK)
    fprintf(stdout, "OK\n");
  else
    fprintf(stdout, "FAIL\n");    
}

void
generic(const char *const storeName)
{
  result_t result;
  BlockStore *blockStore;
  bl_transaction_t trans;
 
  blockStore = bl_open(storeName, &result);
  CHECKRESULT(result, result_OK,);
  trans=bl_BeginTransaction(blockStore);
  CHECKTRANS(trans);


  SHOULDBE(result, result_TxAbort, bl_abort(trans),);
  SHOULDBEOK(result, bl_commit(trans),);
  SHOULDBEOK(result, bl_close(blockStore),);
}

int main(int argc, char *argv[])
{
  extern int optind;

  const char *progName = argv[0];

  // Various temporary working variables:
  unsigned long nFreeBlocks=0;

  setbuf(stdout, 0);		/* turn off buffering on stdout */

  process_args(argc, argv, &args);

  argv += optind;
  argc -= optind;

  if (argc != 0) {
    fprintf(stderr, "%s: too many arguments\n", progName);
    exit(1);
  }
  
  create_close_test(args.storeName, args.nBlocks);

  open_close_test(args.storeName);
  
  abort_test(args.storeName);
  
  commit_test(args.storeName);
  
  alloc_lba_test(args.storeName);
  
  alloc_lba_across_open_close_test(args.storeName);

  drop_non_existent_lba_test(args.storeName);

  write_test(args.storeName);
  
  read_test(args.storeName);
  
  open_close_detailed_test(args.storeName);

  drop_existent_lba_test(args.storeName);
 
  read_a_random_lba_test(args.storeName);
 
  re_write_test(args.storeName);
 
  lba_dba_map_creation_test(args.storeName);
  
  vanish_after_abort_test(args.storeName);  
  
  nFreeBlocks = space_availablity_test(args.storeName);
  
  parent_child_test(args.storeName);

  sparsity_test(args.storeName, nFreeBlocks);

  exit(0);
}
