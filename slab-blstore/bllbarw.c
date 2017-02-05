#include "dslabdefine.h"
#include "debugmsg.h"
#include "bllbarw.h"
#include "blockstore.h"

void load(vptr addr, size_t len, unsigned char *buffer)
{
	unsigned long nread=0;
	unsigned char *lbuf;	
	int i=0;
	result_t result;
	lba_t readlba;


	PDEBUG_BLLBARW("Entered\n");
	PRINTA_BLLBARW(addr);

	nread=__totlbarw(addr, len);
	lbuf=(unsigned char *)malloc((nread*BLK_SZ)*sizeof(unsigned char));

	for(i=0; i<nread; i++)
	{
		LBA(readlba)=LBA(addr.lba)+i;
		result=bl_read(slabtrans, readlba, lbuf+(i*BLK_SZ));
	}
	
	for(i=0; i<len; i++)
	{
		*(buffer+i)=*(lbuf+addr.index+i);
	}
	
	free(lbuf);

	PDEBUG_BLLBARW("Exiting\n");

	if(result !=result_OK)
	{
		printf("Failed to load the data: babyeeee\n");
		exit(0);
	}
	else
		return;
}

void loadc(vptr_cache addr, size_t len, unsigned char *buffer)
{
	vptr tmp_vp;

	CAST_VPTR(tmp_vp, addr);
	load(tmp_vp, len, buffer);
	return;
}

void loads(vptr_slab addr, size_t len, unsigned char *buffer)
{
	vptr tmp_vp;

	CAST_VPTR(tmp_vp, addr);
	load(tmp_vp, len, buffer);
	return;
}

void loadb(vptr_bufctl addr, size_t len, unsigned char *buffer)
{
	vptr tmp_vp;

	CAST_VPTR(tmp_vp, addr);
	load(tmp_vp, len, buffer);
	return;
}


void store(vptr addr, size_t len, unsigned char *buffer)
{
     unsigned long nwrite=0;
     int i=0;
     result_t result;
     lba_t writelba;

	PDEBUG_BLLBARW("Entered\n");

     nwrite=__totlbarw(addr, len);

 	int 			j=0;
	size_t		bufwp=0;
	size_t		wrem=len;
	size_t		iwlen=0;
	size_t		sind=addr.index;
	unsigned char 	tmp_buf[BLK_SZ];

     for(i=0; i<nwrite; i++)
     {   
		//Calculate the parameters for write operaions
		wrem=wrem-iwlen;
		iwlen=(wrem >= BLK_SZ)?(BLK_SZ-sind):wrem;
	
          LBA(writelba)=LBA(addr.lba)+i;
          result=bl_read(slabtrans, writelba, tmp_buf);

		if(result != result_OK)
		{
			printf("Failed to read the block for writing: babyeeee\n");
			exit(0);
		}

		for(j=0; j<iwlen; j++)
		{
		//	PDEBUG_BLLBARW("Entered:j=%d\tsind+j=%d\tbufwp=%d\n",j,(sind+j),bufwp);
			*(tmp_buf+sind+j) = *(buffer+bufwp);
			bufwp=bufwp+1;
		}
		sind=0;
				
          result=bl_write(slabtrans, writelba, tmp_buf);
     }   

     if(result !=result_OK)
	{
		printf("Failed to store the data: babyeeee\n");
		exit(0);
	}
	else
	{
		PDEBUG_BLLBARW("Exiting\n");
		return;
	}

}


void storec(vptr_cache addr, size_t len, unsigned char *buffer)
{
	vptr tmp_vp;

	CAST_VPTR(tmp_vp, addr);
	store(tmp_vp, len, buffer);
	return;
}

void stores(vptr_slab addr, size_t len, unsigned char *buffer)
{
	vptr tmp_vp;

	CAST_VPTR(tmp_vp, addr);
	store(tmp_vp, len, buffer);
	return;
}

void storeb(vptr_bufctl addr, size_t len, unsigned char *buffer)
{
	vptr tmp_vp;

	CAST_VPTR(tmp_vp, addr);
	store(tmp_vp, len, buffer);
	return;
}

unsigned long __totlbarw(vptr addr, size_t len)
{
	unsigned long nlba=0;

	PDEBUG_BLLBARW("Entered:\n");
	PDEBUG_BLLBARW("lba:%llu\tlen:%d\n", LBA(addr.lba),len);
	nlba= 1+((addr.index+len-1)/BLK_SZ);
	PDEBUG_BLLBARW("Exiting:nlba:%lu\n",nlba);
	return nlba;
}
