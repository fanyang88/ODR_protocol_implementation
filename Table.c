#include	<stdlib.h>
#include 	<limits.h>
#include	"ODR.h"
#include	<net/if.h>
#include	<linux/if_ether.h>
#include	"unp.h"
#include	"hw_addrs.h"

int64_t
getTS()
{
   struct timeval tv;
   int ret= gettimeofday(&tv, NULL);
   if(ret==-1)
   return -1;
   return (int64_t)((int64_t) tv.tv_sec*1000+(int64_t)tv.tv_usec/1000);
}


Mtable CreateMT(Mtable mt)
{

	MNode h=NULL;
	printf("create MT!\n");
	mt = (Mtable)malloc(sizeof(Mnode));
	mt->port_number=-1;
	//mt->path_name= (char*)malloc(10);
	mt->path_name= "   ";
	mt->next = NULL;
	return mt;	
}

char* 
get_sun_path(int number, Mtable mt)
{
	int count=0;
  	MNode cur_ptr;
  	cur_ptr= mt;
  	while(cur_ptr != NULL)
  	{
     	if(cur_ptr->port_number==number)
		return cur_ptr->path_name;
     	 cur_ptr=cur_ptr->next;
    
 	 }
  return NULL;
	
}

int get_port_number(char *str, Mtable mt)
{
	int count=0;
  	MNode cur_ptr;
  	cur_ptr= mt;
  	while(cur_ptr != NULL)
  	{
     	if(memcmp(cur_ptr->path_name, str, strlen(str))==0)
		return cur_ptr->port_number;
     	 cur_ptr=cur_ptr->next;

 	 }
  return -1;
	
}

Itable 
CreateIT(Itable it)
{

	INode h=NULL;
	printf("create IT!\n");
	it = (Itable)malloc(sizeof(Inode));

	it->index=-1;
	it->mac_addr= malloc(ETH_ALEN * sizeof(unsigned char));
	memset(&it->mac_addr, 0x00, ETH_ALEN);
	it->next = NULL;
	return it;	
}


int 
Ilength(Itable it)
{
  int count=0;

  INode cur_ptr;
  cur_ptr= it;
  while(cur_ptr != NULL)
  {
     cur_ptr=cur_ptr->next;
     count++;
  }
  return count;
}


Itable    
insertItable(int index, unsigned char *maddr, Itable it)
{
	INode p,q;
	int i=0, pos, m;
	p = it;
	pos= Ilength(it);
	
	while(p!=NULL && i<pos-1)
	{
		p = p->next;
		i++;
	}
	
	q = (Inode *)malloc(sizeof(INode));
	q->index= index;
	q->mac_addr= malloc(ETH_ALEN * sizeof(unsigned char));
	memcpy(q->mac_addr, maddr, IF_HADDR);

	q->next = p->next;
	p->next = q;
	return it;
	
}


Itable
removedup(Itable it)
{
   INode ptr1, ptr2, dup;
   ptr1= it;
   while(ptr1 !=NULL && ptr1->next !=NULL)
   {
        ptr2= ptr1;
	while(ptr2->next!=NULL)
	{
		if(ptr1->index== ptr2->next->index)
		{
			dup=ptr2->next;
			ptr2->next= ptr2->next->next;
			free(dup);
		}
		else
			ptr2=ptr2->next;
	}
	ptr1=ptr1->next;
   }
	return it;
}

void
printItable(Itable it)
{
  int i;
  INode cur_ptr;
  cur_ptr= it;
  cur_ptr= cur_ptr->next;

  while(cur_ptr != NULL)
  {
     printf("[PROCESS] interface index: %d, ", cur_ptr->index);
     printf("mac_addr:"); 
	i = IF_HADDR;
	do {
		printf("%.2x:", cur_ptr->mac_addr[i] & 0xff);
	    } while (--i > 0);
	printf("\n");

     cur_ptr=cur_ptr->next;
  }
}


unsigned char* 
get_mac_addr(Itable it)
{
  	INode cur_ptr;
  	cur_ptr= it;
  	cur_ptr= cur_ptr->next;

  while(cur_ptr != NULL)
  {
     if(cur_ptr->index>0)
	{
	   return cur_ptr->mac_addr;
	}
     	cur_ptr=cur_ptr->next;
  }
	return NULL;
}

int 
Mlength(Mtable mt)
{
  int count=0;

  MNode cur_ptr;
  cur_ptr= mt;
  while(cur_ptr != NULL)
  {
     cur_ptr=cur_ptr->next;
     count++;
  }
  return count;
}


Mtable    
insertMtable(int num, char *name, Mtable mt)
{
	MNode p,q;
	int i=0, pos;
	p = mt;

	pos= Mlength(mt);
	
	while(p!=NULL && i<pos-1)
	{
		p = p->next;
		i++;
	}
	
	q = (Mnode *)malloc(sizeof(MNode));
	q->port_number= num;
	q->path_name= malloc(strlen(name) * sizeof(char));
	strcpy(q->path_name, name);
	q->next = p->next;
	p->next = q;
	return mt;
	
}

int 
findMtable(char *pname, Mtable mt)
{
	MNode cur_ptr;
	cur_ptr= mt;
  	while(cur_ptr!= NULL)
  	{
	    
     	if(strncmp(cur_ptr->path_name, trim(pname), strlen(trim(pname)))==0)
		return 1;
     	 cur_ptr=cur_ptr->next;
    
 	 }
  return -1;
}


int 
Max_number(Mtable mt)
{
	int max= INT_MIN;
	MNode cur_ptr;
  	cur_ptr= mt;
  	while(cur_ptr != NULL)
  	{
     	if(max< cur_ptr->port_number)
		max= cur_ptr->port_number;
     	 cur_ptr=cur_ptr->next;
    
 	 }
  return max;
}

Btable 
CreateBT(Btable bt)
{

	BNode h=NULL;
	printf("create BT!");
	bt = (Btable)malloc(sizeof(Bnode));
	bt->bid=-1;
	bt->ip_addr="256.256.256.256";
	bt->next = NULL;
	return bt;	
}

int 
findBtable(int id, char *addr, Btable bt)
{
	int count=0;
  	BNode cur_ptr;
  	cur_ptr= bt;
  	while(cur_ptr != NULL)
  	{
     	if(cur_ptr->bid==id && strcmp(cur_ptr->ip_addr, addr)==0)
		return 1;
     	 cur_ptr=cur_ptr->next;
    
 	 }
  return -1;	
}

Btable    
insertBtable(int id, char *addr, Btable bt)
{
	BNode p,q;
	int i=0, pos;
	p = bt;

	pos= Blength(bt);
	
	while(p!=NULL && i<pos-1)
	{
		p = p->next;
		i++;
	}
	
	q = (Bnode *)malloc(sizeof(Bnode));
	q->bid= id;
	q->ip_addr= malloc(strlen(addr) * sizeof(char));
	strcpy(q->ip_addr, addr);
	q->next = p->next;
	p->next = q;
	return bt;
	
}

int 
Blength(Btable bt)
{
  int count=0;
  BNode cur_ptr;
  cur_ptr= bt;
  while(cur_ptr != NULL)
  {
     cur_ptr=cur_ptr->next;
     count++;
  }
  return count;
}


Rtable 
CreateRT(Rtable rt)
{
	PNode p=NULL;
	rt = (Rtable)malloc(sizeof(Node));
	rt->hop_cnt=0;
	rt->index=0;
	rt->dest_node= "256.256.256.256";
	rt->next_hop= NULL;
	rt->ts= getTS();
	rt->next = NULL;
	return rt;	
}


int 
findEntry(char *addr, Rtable rt)
{
	int count=0;
  	PNode cur_ptr;
  	cur_ptr= rt;
  	while(cur_ptr != NULL)
  	{
     	if(strcmp(addr, cur_ptr->dest_node)==0)
		return 1;
     	 cur_ptr=cur_ptr->next;
    
 	 }
  return -1;	
}

int 
get_hop_cnt(char *addr,Rtable rt)
{
	int count=0;
  	PNode cur_ptr;
  	cur_ptr= rt;
  	while(cur_ptr != NULL)
  	{
     	if(strcmp(addr, cur_ptr->dest_node)==0)
		return cur_ptr->hop_cnt;
     	 cur_ptr=cur_ptr->next;
    
 	 }
  return -1;
}

int 
get_index(char *addr,Rtable rt)
{
	int count=0;
  	PNode cur_ptr;
  	cur_ptr= rt;
  	while(cur_ptr != NULL)
  	{
     	if(strcmp(addr, cur_ptr->dest_node)==0)
		return cur_ptr->index;
     	 cur_ptr=cur_ptr->next;
    
 	 }
  return -1;
}


void 
printRT(Rtable rt)
{
	int i;
  PNode cur_ptr;
  cur_ptr= rt;
	printf("print Routing table!\n");
 while(cur_ptr != NULL)
  	{

     	printf("next_hop:"); 
	i = IF_HADDR;
	do {
		printf("%.2x:", cur_ptr->next_hop[i] & 0xff);
	    } while (--i > 0);
	printf(" dest: %s!", cur_ptr->dest_node);
	printf("\n");
     	 cur_ptr=cur_ptr->next;
    
 	 }
}


unsigned char* 
get_next_hop(char *addr,Rtable rt)
{
	
  	int count=0;
  	PNode cur_ptr;
	//printf("[PROCESS] next_hop has dest node in routing table:%s\n", addr);
  	cur_ptr= rt;
  	while(cur_ptr != NULL)
  	{
     	if(strcmp(addr, cur_ptr->dest_node)==0)
		return cur_ptr->next_hop;
     	 cur_ptr=cur_ptr->next;
    
 	 }
  return NULL;
}

int 
length(Rtable rt)
{
  int count=0;
  PNode cur_ptr;
  cur_ptr= rt;
  while(cur_ptr != NULL)
  {
     cur_ptr=cur_ptr->next;
     count++;
  }
  return count;
}

Rtable 
insertEntry(char *addr,unsigned char *Ether_addr, int hc, int index, int64_t ts, Rtable rt)
{
	PNode p,q;
	int i=0, pos;
	p = rt;

	pos= length(rt);
	
	while(p!=NULL && i<pos-1)
	{
		p = p->next;
		i++;
	}
	
	q = (Node *)malloc(sizeof(Node));
	q->hop_cnt= hc;
	q->ts= ts;
	q->index= index;
	
	q->next_hop= malloc(ETH_ALEN * sizeof(unsigned char));
	q->next_hop[0]= Ether_addr[0];
	q->next_hop[1]= Ether_addr[1];
	q->next_hop[2]= Ether_addr[2];
	q->next_hop[3]= Ether_addr[3];
	q->next_hop[4]= Ether_addr[4];
	q->next_hop[5]= Ether_addr[5];

	//memcpy(q->next_hop, Ether_addr, strlen(Ether_addr));
	printf("[PROCESS] Insert into routing table: next_hop"); 
	i = IF_HADDR;
	do {
		printf(":%.2x", q->next_hop[i] & 0xff);
	    } while (--i > 0);
	
	
	q->dest_node= malloc(strlen(addr) * sizeof(char));
	strcpy(q->dest_node, addr);
	printf(", dest node:%s, hop_cnt#: %d\n", q->dest_node, hc);
	q->next = p->next;
	p->next = q;
	//printf("insert node with hop_cnt#: %d success!\n", hc);
	return rt;
}

Rtable 
updateEntry(char *addr,unsigned char *Ether_addr, int hc,int index, int64_t ts, Rtable rt)
{
	int count=0;
  	PNode cur_ptr;
  	cur_ptr= rt;
  	while(cur_ptr != NULL)
  	{
     		if(strcmp(addr, cur_ptr->dest_node)==0)
		{
			memset(&cur_ptr->next_hop, '\0',sizeof(cur_ptr->next_hop));
			memcpy(cur_ptr->next_hop, Ether_addr, strlen(Ether_addr));
			cur_ptr->hop_cnt= hc;
			cur_ptr->ts= ts;
			cur_ptr->index= index;
			return rt;
		}
     	 	cur_ptr=cur_ptr->next;
 	 }
  	return rt;	
}

//0 - no such entry
//1 - entry is stale
//-1- entry is not stale
int 
stale(char *addr, Rtable rt, int staleness)
{
	int count=0;
	int64_t cur_ts=getTS();
  	PNode cur_ptr;
  	cur_ptr= rt;
  	while(cur_ptr != NULL)
  	{
     		if(strcmp(addr, cur_ptr->dest_node)==0)
		{
			if((cur_ts- cur_ptr->ts)<= staleness)
			return -1;
			else
			return 1;
		}
     	 	cur_ptr=cur_ptr->next;
 	 }
  	return 0;
}


int 
deleteEntry(char *addr, Rtable rt)
{
	PNode p,q, prev;
	int i=0, flag=0;
	p = rt;
	if(p==NULL)
        {printf("routing table is empty!\n"); return rt;}

	if(strcmp(rt->dest_node, addr)==0)
        {
		q = rt;
		rt = p->next;
		free(q);
		return rt;
        }


	while(p!=NULL)
	{	prev=p;
	        p = p->next;
		if(strcmp(p->dest_node, addr)==0)
			{flag=1; break;}
	}
	
	if(flag==0)
	   {printf("[PROCESS] No such address exist in rt!\n"); return rt; } //num not exit;

	q = p;
	prev->next = p->next;
	free(q);
	printf("deleted the dest_node with address: %s in the routing table\n", addr);
	return rt;
}

