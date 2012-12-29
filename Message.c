#include	"ODR.h"
#include	<net/if.h>
#include	<linux/if_ether.h>
#include	"unp.h"
#include 	<stdio.h>
#include 	<ctype.h>


char* 
trim(char *str)
{
  int dest, src=0, len=strlen(str);
  while(isspace(str[src]))
   src++;
  for(dest=0; src<len;dest++, src++)
    str[dest]= str[src];
  for(dest=len-1; isspace(str[dest]); --dest)
    str[dest]='\0';
  return str;
}


char* 
InttoString(int a, int bytes)
{
 int len=0, i=0, k=0, rest;
 char temp[bytes], *str;
 str=malloc(bytes * sizeof(char));
 sprintf(temp, "%d", a);
  len= strlen(temp);
  if(bytes< len)  
  {printf("bytes exceeds limits!\n"); return NULL;}

   rest= bytes-len;
  while(rest>0)
  {
     str[i]='0';
     rest--; i++;
  }
  while(len>0)
 {
    str[i]= temp[k];
    k++;   len--; i++;
 }
	return str;
}


struct RREQ*
CreateRREQ(struct RREQ* req, int id, char *saddr, char *daddr)
{
	req = Calloc(1, sizeof(struct RREQ));
  	req->type = 0;
  	req->broadcast_id = id;
	req->hop_cnt=0;
	req->rrepsent=0;
	req->discovery=0;
	req->sport= 0;
	req->dport= 0;

	req->src_addr= malloc(strlen(saddr) * sizeof(char));
	strcpy(req->src_addr, saddr); 
	
	req->dest_addr= malloc(strlen(daddr) * sizeof(char));
	strcpy(req->dest_addr, daddr); 
	
	return req;
}

void     
freeRREQ(struct RREQ *p)
{
        p->type = 0;
	p->broadcast_id = 0;
	p->hop_cnt = 0;
	if(p->src_addr!=NULL)
	   free(p->src_addr);
	if(p->dest_addr!=NULL)
	  free(p->dest_addr);
	p->sport=0;
	p->dport=0;	
	p->rrepsent = 0;
	p->discovery =0;
        free(p);		        
}

struct RREQ* 
StrtoRREQ(char *str, struct RREQ *req)
{
	char temp[5];
	req = Calloc(1, sizeof(struct RREQ));

	req->rrepsent=str[1]-'0';
	req->discovery=str[2]-'0';
	
	sprintf(temp, "%.*s", 5, str+3);
	req->broadcast_id= atoi(temp);

	memset(&temp, '0', sizeof(temp));
	sprintf(temp, "%.*s", 5, str+8);
	req->hop_cnt= atoi(temp);
	
	req->src_addr= (char *)malloc(20 * sizeof(char));
	sprintf(req->src_addr, "%.*s", 20, str+13);
	req->src_addr= trim(req->src_addr);

	req->dest_addr= (char *)malloc(20 * sizeof(char));
	sprintf(req->dest_addr, "%.*s", 20, str+33);
	req->dest_addr= trim(req->dest_addr);

	memset(&temp, '0', sizeof(temp));
	sprintf(temp, "%.*s", 5, str+53);
	req->sport= atoi(temp);
	
	memset(&temp, '0', sizeof(temp));
	sprintf(temp, "%.*s", 5, str+58);
	req->dport= atoi(temp);
	
	return req;
}

char* 
RREQtoStr(struct RREQ *req)
{
	 char str[100];
	 int i;
	 for(i=0;i<100;i++)
		str[i]=' ';
	 str[0]='0';
	 str[1]= req->rrepsent+ 48; 
	 str[2]= req->discovery+ 48;
	
	 strncpy(str+3, InttoString(req->broadcast_id,  5), 5); 
  	 strncpy(str+8, InttoString(req->hop_cnt,  5), 5);
  	 strncpy(str+13, req->src_addr, strlen(req->src_addr));
	 strncpy(str+33, req->dest_addr, strlen(req->dest_addr));
	 strncpy(str+53, InttoString(req->sport,  5), 5); 
  	 strncpy(str+58, InttoString(req->dport,  5), 5);
  	 
 	return str;
	 	
}

		
struct ODR*
CreateODR(struct ODR* d)
{	int i=0;
	d = Calloc(1, sizeof(struct ODR));
  	d->broadcast_id=0;
	d->rt= CreateRT(d->rt);	
	d->bt= CreateBT(d->bt);
	d->mt= CreateMT(d->mt);
	d->it= CreateIT(d->it);
	d->pfd=0;
	d->ufd=0;
	printf("create ODR success!\n");
	
	return d;
}

struct RREP*
CreateRREP(struct RREP* rep, char *saddr, char *daddr)
{
	rep = Calloc(1, sizeof(struct RREP));
  	rep->type = 1;
  	rep->hop_cnt=0;
	rep->sport= 0;
	rep->dport= 0;
	rep->discovery=0;
	rep->src_addr= malloc(strlen(saddr) * sizeof(char));
	strcpy(rep->src_addr, saddr); 

	rep->dest_addr= malloc(strlen(daddr) * sizeof(char));
	strcpy(rep->dest_addr, daddr); 

	return rep;
}


void     
freeRREP(struct RREP *p)
{
        p->type = 1;
	p->hop_cnt = 0;
	if(p->src_addr!=NULL)
	   free(p->src_addr);
	if(p->dest_addr!=NULL)
	  free(p->dest_addr);
	p->sport=0;
	p->dport=0;
	p->discovery =0;
        free(p);		        
}

struct RREP* 
StrtoRREP(char *str, struct RREP *rep)
{
	char temp[5];
	rep = Calloc(1, sizeof(struct RREP));
 	rep->discovery=str[1]-'0';

	sprintf(temp, "%.*s", 5, str+2);
	rep->hop_cnt= atoi(temp);
	
	rep->src_addr= malloc(20 *sizeof(char));
	sprintf(rep->src_addr, "%.*s", 20, str+7);
	rep->src_addr= trim(rep->src_addr);

	rep->dest_addr= malloc(20* sizeof(char));
	sprintf(rep->dest_addr, "%.*s", 20, str+27);
	rep->dest_addr= trim(rep->dest_addr);

	memset(&temp, '0', sizeof(temp));
	sprintf(temp, "%.*s", 5, str+47);
	rep->sport= atoi(temp);
	
	memset(&temp, '0', sizeof(temp));
	sprintf(temp, "%.*s", 5, str+52);
	rep->dport= atoi(temp);
	
	return rep;
}

char* 
RREPtoStr(struct RREP *rep)
{
	char str[60];
	 int i;
	for(i=0;i<60;i++)
		str[i]=' ';

	 str[0]='1';
	 str[1]= rep->discovery+ 48;
	 strncpy(str+2,InttoString(rep->hop_cnt,  5), 5); 
  	 strncpy(str+7, rep->src_addr, strlen(rep->src_addr));
	 strncpy(str+27,rep->dest_addr, strlen(rep->dest_addr));

	 strncpy(str+47,InttoString(rep->sport,  5), 5); 
  	 strncpy(str+52,InttoString(rep->dport,  5), 5); 
  	 
  	return str;
}


struct Msg*
CreateMsg(struct Msg* msg, char *saddr, char *daddr)
{

	msg = Calloc(1, sizeof(struct Msg));
  	msg->type = 2;
	msg->id=0;
  	msg->buflen = 0;
	msg->hop_cnt=0;
	
	msg->src_addr= malloc(strlen(saddr) * sizeof(char));
	strcpy(msg->src_addr, saddr); 

	msg->dest_addr= malloc(strlen(daddr) * sizeof(char));
	strcpy(msg->dest_addr, daddr); 

	msg->sport= 0;
	msg->dport= 0;
	
	return msg;
}



void     
freeMsg(struct Msg *p)
{
        p->type = 2;
	p->id=0;
	p->buflen = 0;
	p->hop_cnt = 0;
	if(p->src_addr!=NULL)
	   free(p->src_addr);
	if(p->dest_addr!=NULL)
	  free(p->dest_addr);
	p->sport=0;
	p->dport=0;
	if(p->buffer!=NULL)
	   free(p->buffer);
	 free(p);		        
}


struct Msg* 
StrtoMsg(char *str, struct Msg *msg)
{
 	char temp[5];
	msg = Calloc(1, sizeof(struct Msg));
	msg->type= 2;
 	msg->id= str[1]-48;
	
	memset(&temp, '0', sizeof(temp));
	sprintf(temp, "%.*s", 5, str+2);
	msg->buflen= atoi(temp);
	
	memset(&temp, '0', sizeof(temp));
	sprintf(temp, "%.*s", 5, str+7);
	msg->hop_cnt= atoi(temp);
	
	msg->src_addr= malloc(20* sizeof(char));
	sprintf(msg->src_addr, "%.*s", 20, str+12);
	msg->src_addr= trim(msg->src_addr);

	msg->dest_addr= malloc(20* sizeof(char));
	sprintf(msg->dest_addr, "%.*s", 20, str+32);
	msg->dest_addr= trim(msg->dest_addr);

	memset(&temp, '0', sizeof(temp));
	sprintf(temp, "%.*s", 5, str+52);
	msg->sport= atoi(temp);
	
	memset(&temp, '0', sizeof(temp));
	sprintf(temp, "%.*s", 5, str+57);
	msg->dport= atoi(temp);
	
	msg->buffer= malloc((strlen(str)-62)* sizeof(char));
	sprintf(msg->buffer, "%.*s", strlen(str)-62, str+62);
	
	return msg;
}


char* 
MsgtoStr(struct Msg *msg)
{
	char str[100];
	memset(&str, ' ', sizeof(str));
	 
	 str[0]='2';//type
	 str[1]=msg->id + 48;//id
	 strncpy(str+2,InttoString(msg->buflen,  5), 5); 
  	 strncpy(str+7,InttoString(msg->hop_cnt,  5), 5);
  	 strncpy(str+12, msg->src_addr, strlen(msg->src_addr));
	 strncpy(str+32,msg->dest_addr, strlen(msg->dest_addr));

	 strncpy(str+52,InttoString(msg->sport,  5), 5); 
  	 strncpy(str+57,InttoString(msg->dport,  5), 5);
	 strncpy(str+62,msg->buffer, 20);
	 return str;
}


struct UMsg*
CreateUMsg(struct UMsg* umsg,char *daddr,char *saddr,char *spstr,int dport, char *msg)
{
	umsg = Calloc(1, sizeof(struct UMsg));
	umsg->src_addr= malloc(strlen(saddr) * sizeof(char));
	strcpy(umsg->src_addr, saddr); 

	umsg->dest_addr= malloc(strlen(daddr) * sizeof(char));
	strcpy(umsg->dest_addr, daddr); 

	umsg->sport= malloc(strlen(spstr) * sizeof(char));
	strcpy(umsg->sport, spstr); 
	umsg->dport= dport;

	umsg->buffer=malloc(20 * sizeof(char));
	memcpy(umsg->buffer, msg, 20); 

	return umsg;
}


void     
freeUMsg(struct UMsg *p)
{
	if(p->src_addr!=NULL)
	   free(p->src_addr);
	if(p->dest_addr!=NULL)
	  free(p->dest_addr);
	if(p->sport!=NULL)
	  free(p->sport);
	p->dport=0;
	if(p->buffer!=NULL)
	   free(p->buffer);
	 free(p);		        
}


struct UMsg* 
StrtoUMsg(char *str, struct UMsg *msg)
{
	char temp[5];
	msg = Calloc(1, sizeof(struct UMsg));
 	
	msg->src_addr= malloc(20 * sizeof(char));  //(char*)malloc(20);
	sprintf(msg->src_addr, "%.*s", 20, str);
	msg->src_addr= trim(msg->src_addr);

	msg->dest_addr= malloc(20* sizeof(char));
	sprintf(msg->dest_addr, "%.*s", 20, str+20);
	msg->dest_addr= trim(msg->dest_addr);

	msg->sport= malloc(100* sizeof(char));
	sprintf(msg->sport, "%.*s", 100, str+40);
	msg->sport= trim(msg->sport);

	memset(&temp, '0', sizeof(temp));
	sprintf(temp, "%.*s", 5, str+140);
	msg->dport= atoi(temp);
	
	msg->buffer= malloc((strlen(str)-145)* sizeof(char));
	sprintf(msg->buffer, "%.*s", strlen(str)-145, str+145);
	msg->buffer= trim(msg->buffer);
	
	return msg;
}


char* 
UMsgtoStr(struct UMsg *msg)
{
	 char	s[200];
	 memset(&s, ' ', sizeof(s));
	 strncpy(s+0, msg->src_addr, strlen(msg->src_addr));
	 strncpy(s+20,msg->dest_addr, strlen(msg->dest_addr));
	 strncpy(s+40,msg->sport, strlen(msg->sport));
	 strncpy(s+140,InttoString(msg->dport,  5), 5);
	 strncpy(s+145,msg->buffer, 20);
	 
  	return s;
}
