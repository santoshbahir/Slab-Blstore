#include <stdio.h>
#include "bllbarw.h"
#include "virtptr.h"
#include "dslabdefine.h"


void null_init()
{
	LBA(vnull.lba)=0; vnull.index=0;
	LBA(vcnull.lba)=0; vcnull.index=0;
	LBA(vsnull.lba)=0; vsnull.index=0;
	LBA(vbnull.lba)=0; vbnull.index=0;
	return;
}

void inc(vptr *addr, unsigned long long nplaces)
{
	int div, rem;
	
	if(nplaces >= BLK_SZ)
	{
		div=(addr->index+nplaces)/BLK_SZ;
		rem=(addr->index+nplaces)%BLK_SZ;
	}
	else
	{
		div=0;
		rem=nplaces;	
	}

	LBA(addr->lba)=LBA(addr->lba)+div;
	addr->index=addr->index+rem;
}


void inc_pc(vptr_cache *addr, unsigned long long nplaces)
{
	int div, rem;
	
	div=((addr->index+(nplaces*sizeof(struct dmem_cache)))-1)/BLK_SZ;

	if(div==0)
		rem=nplaces*sizeof(struct dmem_cache);
	else
		rem=((addr->index+(nplaces*sizeof(struct dmem_cache)))-1)%BLK_SZ;

	LBA(addr->lba)=LBA(addr->lba)+div;

	addr->index=addr->index+rem;
}


void inc_ps(vptr_slab *addr, unsigned long long nplaces)
{
	int div, rem;
	
	div=((addr->index+(nplaces*sizeof(struct dmem_slab)))-1)/BLK_SZ;

	if(div==0)
		rem=nplaces*sizeof(struct dmem_slab);
	else
		rem=((addr->index+(nplaces*sizeof(struct dmem_slab)))-1)%BLK_SZ;

	LBA(addr->lba)=LBA(addr->lba)+div;
	addr->index=addr->index+rem;
}


void inc_pb(vptr_bufctl *addr, unsigned long long nplaces)
{
	int div, rem;
	
	div=((addr->index+(nplaces*sizeof(struct dmem_bufctl)))-1)/BLK_SZ;

	if(div==0)
		rem=nplaces*sizeof(struct dmem_bufctl);
	else
		rem=((addr->index+(nplaces*sizeof(struct dmem_bufctl)))-1)%BLK_SZ;

	LBA(addr->lba)=LBA(addr->lba)+div;
	addr->index=addr->index+rem;
}

/*int main()
{*/
	/*Testing of casting the types*/
/*     vptr_cache c;	vptr_slab s; vptr_bufctl b;
     lba_t l;     LBA(l)=9;
     s.lba=l;     s.index=4;
     CAST_CACHE(c,s);
     printf("c.lba:%llu\tc.index:%hu\n",LBA(c.lba),c.index);*/

	/*Testing of pointer increment/decrement*/
/*	inc_ps(&s,1);
     printf("s.lba:%llu\ts.index:%hu\n",LBA(s.lba),s.index);
     CAST_CACHE(b,s);
	inc_pb(&b,-1);
     printf("b.lba:%llu\tb.index:%hu\n",LBA(b.lba),b.index);*/

	/*Testing of the assignment of two pointers*/
/*	vptr_slab a, b;
	lba_t l;	LBA(l)=99;
	a.lba=l;	a.index=1000;

	b=a;
     printf("b.lba:%llu\tb.index:%hu\n",LBA(b.lba),b.index);*/
	
	/*Testing of load and store procedure*/
/*	unsigned char buffer[BLK_SZ];
	vptr v;
	CAST_VPTR(v,c);
	load(v, 10, buffer);*/

	/*Testing of dmem structures procedure*/
/*	struct dmem_slab *ob;
	ob=(struct dmem_slab *)malloc(sizeof(struct dmem_slab));
	return 0;
}*/
