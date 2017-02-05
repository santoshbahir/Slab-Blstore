#include "block_cache.h"
#include "phyblockrw.h"
#include "bitmap.h"

struct cache_buffer *cache_header;

void cache_init()
{
	cache_header=NULL;
	return;
}


bool cache_empty(struct cache_buffer *cache_header)
{
	if(NULL==cache_header)
		return true;	
	
	return false;
}


struct cache_buffer *alloc_cache_buffer()
{
	PDEBUG_BLOCKCACHE("Entered\n");
	struct cache_buffer *cb;
	
	PDEBUG_BLOCKCACHE("Entered:-->>>%lu\n",(unsigned long)cb);
	cb=malloc(sizeof *cb);
	return(cb);
}


struct cache_buffer *cache_lookup(dba_t address, bool *status)
{
	struct cache_buffer *tmp_buf;

	PDEBUG_BLOCKCACHE("Entered: Looking for DBA:%lu\n",DBA(address));
	if(cache_empty(cache_header))
		return NULL;

	tmp_buf=cache_header;

	do{	
		PDEBUG_BLOCKCACHE("%lu\n",DBA(tmp_buf->block_addr));	

		if(DBA(tmp_buf->block_addr) == DBA(address))
		{
			*status=tmp_buf->status;
			return tmp_buf;
		}
		tmp_buf=tmp_buf->next;

	}while(tmp_buf != cache_header);

	PDEBUG_BLOCKCACHE("Exiting as DBA %lu not found\n",DBA(address));
	return NULL;
}


void cache_blockadd(struct cache_buffer *buffer)
{
	//This tmp_buf will point to first element in circular linked list.
	struct cache_buffer *tmp_buf; 
	
	tmp_buf=cache_header;
	if(cache_empty(cache_header))
	{
		buffer->next=buffer;
		buffer->prev=buffer;

		cache_header=buffer;
	}
	else
	{
		buffer->next=tmp_buf;
		buffer->prev=tmp_buf->prev;
		tmp_buf->prev->next=buffer;
		tmp_buf->prev=buffer;

		cache_header=buffer;
	}
	return;
}


void cache_blockdelete(dba_t dba_addr)
{
	struct cache_buffer *tmp_buf; 
	bool block_status;
	
	tmp_buf=cache_lookup(dba_addr, &block_status);

	if(NULL==tmp_buf)	
		return; //No action is required as dba does not exists

	if((tmp_buf->next==cache_header) && (tmp_buf->prev==cache_header))
	{
		cache_header=NULL;
		free(tmp_buf);
		return;
	}
	
	tmp_buf->prev->next=tmp_buf->next;	
	tmp_buf->next->prev=tmp_buf->prev;	

	if(tmp_buf == cache_header)
		cache_header=tmp_buf->next;

	free(tmp_buf);

	return;
}


size_t cache_blockread(struct BlockStore *bsp, dba_t address, char *block)
{
	struct cache_buffer *tmp_buf;
	int i;
	bool block_status;
	size_t read_bytes=BLK_SZ;

	PDEBUG_BLOCKCACHE("Entered cache_blockread:readDBA=%lu\n",DBA(address));
	tmp_buf=cache_lookup(address, &block_status);

	if(NULL == tmp_buf)
	{
		PDEBUG_BLOCKCACHE("Inside IF block\n");
		tmp_buf=alloc_cache_buffer();
		PDEBUG_BLOCKCACHE("Inside IF block:0\n");
		read_bytes=read_block(bsp->fd, tmp_buf->block, address);
		PDEBUG_BLOCKCACHE("Inside IF block:1 :read_bytes=%d\n",(int)read_bytes);
		PDEBUG_BLOCKCACHE("Inside IF block:1 :ADDRESS=%lu\n",DBA(address));
		PDEBUG_BLOCKCACHE("Inside IF block:1 :tmp_buf=%lu\n",(unsigned long)tmp_buf);
		PDEBUG_BLOCKCACHE("1 :tmp_buf->next=%lu\n",(unsigned long)tmp_buf->next);
		PDEBUG_BLOCKCACHE("1 :tmp_buf->block=%lu\n",(unsigned long)tmp_buf->block);
		tmp_buf->block_addr=address;
		PDEBUG_BLOCKCACHE("Inside IF block:2\n");
		tmp_buf->status=CLEAN;
		PDEBUG_BLOCKCACHE("Inside IF block:3\n");

		PDEBUG_BLOCKCACHE("Inside IF block:4\n");
		cache_blockadd(tmp_buf);
		PDEBUG_BLOCKCACHE("Inside IF block:5\n");
	}

	PDEBUG_BLOCKCACHE("Before copying from the cache\n");
	for(i=0; i<BLK_SZ; i++)
		*(block+i) = tmp_buf->block[i];
	PDEBUG_BLOCKCACHE("Exitting:%lu\n",DBA(address));
	return read_bytes;
}


size_t cache_blockwrite(struct BlockStore *bsp, dba_t oldaddr, \
					dba_t newaddr, char *block)
{
	struct cache_buffer *tmp_buf;
	int i;
	bool block_status;

	PDEBUG_BLOCKCACHE("Entered\n");

	tmp_buf=cache_lookup(oldaddr, &block_status);

	if(NULL == tmp_buf)
	{
		PDEBUG_BLOCKCACHE("IF BLOCK: Entered\n");
		tmp_buf=alloc_cache_buffer();
		PDEBUG_BLOCKCACHE("IF BLOCK: 0\n");
		tmp_buf->block_addr=newaddr;

		PDEBUG_BLOCKCACHE("IF BLOCK: 1\n");
		cache_blockadd(tmp_buf);
		PDEBUG_BLOCKCACHE("IF BLOCK: 2\n");
		PDEBUG_BLOCKCACHE("IF BLOCK: Exiting\n");
	}
	else
	{
		/* Not a superblock &&
 		 * not a block read from volume*/ 
		if( (!( DBA(oldaddr)==DBA(newaddr) ) ) && \
		( DBA(oldaddr)!=0 && DBA(newaddr)!=1 ))
		{
//			deallocatedba(bsp, oldaddr);
			tmp_buf->block_addr=newaddr;
		}
	}

	for(i=0; i<BLK_SZ; i++)
		tmp_buf->block[i] = *(block+i);
	tmp_buf->status=DIRTY;	

	PDEBUG_BLOCKCACHE("Exiting:%lu\n",DBA(newaddr));
	return((size_t)BLK_SZ);
}

void cache_fflush(int fd)
{
	struct cache_buffer *tmp_buf, *tmp;
	size_t write_bytes;
	int i=0;	

	PDEBUG_BLOCKCACHE("Entered\n");
	if(cache_empty(cache_header))
	{
		PDEBUG_BLOCKCACHE("Bull-shit\n");
		return;
	}
	tmp_buf=cache_header;

	do{		
		i++;
		PDEBUG_BLOCKCACHE("i=%d\n",i);
		if(tmp_buf->status == DIRTY)
		{
		PDEBUG_BLOCKCACHE("cache memory block:%lu\tvolume block:%lu\n"\
                           ,(unsigned long)tmp_buf,DBA(tmp_buf->block_addr));
			write_bytes=write_block(fd, tmp_buf->block, tmp_buf->block_addr);
		}

		tmp=tmp_buf;
		tmp_buf=tmp_buf->next;
		free(tmp);

	}while(tmp_buf != cache_header);

	cache_header=NULL;
	PDEBUG_BLOCKCACHE("Exiting\n");
	return;
}


void cache_abort(int fd)
{
     struct cache_buffer *tmp_buf, *tmp;
     int i=0;

	PDEBUG_BLOCKCACHE("Entered\n");

	if(cache_empty(cache_header))
     {
          PDEBUG_BLOCKCACHE("Bull-shit\n");
          return;
     }
     tmp_buf=cache_header;	

     do{
          i++;
          PDEBUG_BLOCKCACHE("i=%d\n",i);
          tmp=tmp_buf;
          tmp_buf=tmp_buf->next;
          free(tmp);

     }while(tmp_buf != cache_header);

     cache_header=NULL;
     PDEBUG_BLOCKCACHE("Exiting\n");
     return;
}
