#include	"ODR.h"
#include	<linux/if_ether.h>
#include	<sys/socket.h>
#include	<linux/if_packet.h>
#include	"hw_addrs.h"

int process_rreq(void* , struct ODR *, int );
int process_rrep(void* , struct ODR *, int , int );
int process_pmsg(void* , struct ODR *, int );
int process_umsg(struct Msg * , struct ODR *, int , void* );
int sendRREQ(int , char * , struct ODR * , void* );
int sendRREP(int , char * ,  unsigned char * , int , void* ,struct ODR * );
int sendMsg(int , char * ,  unsigned char * , int , void* , struct ODR * );


int
main(int argc, char **argv)
{
	socklen_t		len;
	int			k=0, n, m, i, prflag, maxfd=0, st, from_index, rv, pnum;
	fd_set			socks;
	struct sockaddr_un	hostaddr, peeraddr;
	struct Msg		*msg;
	struct UMsg		*u_msg;
	struct hostent		*serv, *cli;
	struct hwa_info		*hwa, *hwahead;
	struct sockaddr		*sa;
	char			src_addr[20], res[STLEN], recv_ubuf[STLEN];
	unsigned char		mac_addr[ETH_ALEN];
	struct sockaddr_ll	from;
	struct	ODR		*odr;
	void* recv_buf = (void*)malloc(ETH_FRAME_LEN);
	unsigned char* data= recv_buf+14;
	struct timeval 		tim;
	FILE			*f;
	
	printf("[PROCESS] start_ODR!\n");
	st= 60000;  //staleness parameters
		
	odr= CreateODR(odr);
	odr->mt= insertMtable( PORT_NUM, DEST_SUN_PATH, odr->mt);
	odr->mt= insertMtable( ODR_NUM, ODR_SUN_PATH, odr->mt);
	odr->mt= insertMtable( CLI_NUM, CLI_SUN_PATH, odr->mt);
	
	printf("[PROCESS] Create a unix domain socket!\n");
	f= fopen(ODR_SUN_PATH, "W+");
	
	unlink(ODR_SUN_PATH);
	odr->ufd = Socket(AF_LOCAL, SOCK_DGRAM, 0);
	bzero(&hostaddr, sizeof(hostaddr));
	hostaddr.sun_family = AF_LOCAL;
	strcpy(hostaddr.sun_path, ODR_SUN_PATH);

	if((rv=bind(odr->ufd, (struct sockaddr*)&hostaddr, sizeof(hostaddr)))<0)
	{perror("[PROCESS] Bind error!\n"); exit(0);}
	printf("[PROCESS] Bind sucess!\n");

	//get hwaddr 
	for (hwahead = hwa = Get_hw_addrs(); hwa != NULL; hwa = hwa->hwa_next) {
	   
		if ( (sa = hwa->ip_addr) != NULL)
			{
				 if(strcmp(hwa->if_name,"eth0")==0)
			  {
		odr->ip_addr= malloc(strlen(Sock_ntop_host(sa, sizeof(*sa))) * sizeof(char));
		memcpy(odr->ip_addr, Sock_ntop_host(sa, sizeof(*sa)), strlen(Sock_ntop_host(sa, sizeof(*sa))));
			  
			  }
			}
				
		prflag = 0;
		i = 0;
		do {
			if (hwa->if_haddr[i] != '\0') {
				prflag = 1;
				break;
			}
		} while (++i < IF_HADDR);

		if (prflag) {
			//printf("HW addr! ");
			memset(mac_addr, 0x00, ETH_ALEN);
			memcpy(mac_addr, hwa->if_haddr, IF_HADDR);
			
		}
		
		if(strcmp(hwa->if_name,"eth0")!=0 && strcmp(hwa->if_name,"lo")!=0)
                  	{ 
			     // printf("insert node in itable!\n");
			      odr->it=insertItable(hwa->if_index,hwa->if_haddr,odr->it);
			}	
		 
		
	}//for end
	free_hwa_info(hwahead);

	odr->it=removedup(odr->it);
	printItable(odr->it);
	printf("[PROCESS] ETH0 IP ADDRESS:%s\n", odr->ip_addr); 
	
	//create PF_packet socket	
	odr->pfd= socket(PF_PACKET, SOCK_RAW, htons(ODR_PRO));
	if(odr->pfd==-1)
	{
		perror("[PROCESS] PF_PACKET create error!");
		exit(1);
	}

	maxfd= odr->pfd > odr->ufd? odr->pfd: odr->ufd;
	for(;;)
	{
		printf("\n[PROCESS] ODR listening!\n");
	
		FD_ZERO(&socks);
		FD_SET(odr->pfd, &socks);
		FD_SET(odr->ufd, &socks);
		
		if(select(maxfd+1, &socks, NULL, NULL, NULL)<0)
		{
			if(errno!=EINTR)
			{ perror("[PROCESS] Select error!");  exit(-1);}
		}
		if(FD_ISSET(odr->pfd, &socks))
		{
			socklen_t fromlen;
			fromlen= sizeof(from);
			len=recvfrom(odr->pfd, recv_buf, ETH_FRAME_LEN, 0, (struct sockaddr*)&from, &fromlen);
			if(len<0)
			{perror("[PROCESS] Error in recvfrom PF_PACKET!"); exit(-1);}
			from_index= from.sll_ifindex;
			//printf("\n interface index=%d, type=%d\n", from_index, data[0]-'0');
			switch(data[0]-'0')
			{
		 	case 0: process_rreq(recv_buf, odr,from_index); break;
			case 1: process_rrep(recv_buf,odr,from_index, st); break;
			case 2: process_pmsg(recv_buf, odr, st); break;
			default: break;
			}

		}//end if
		
		else if(FD_ISSET(odr->ufd, &socks))
		{
			printf("[PROCESS] Receive msg from domain socket of client!\n");
			 if((rv=Recvfrom(odr->ufd, recv_ubuf, STLEN, 0, NULL, NULL))<0)
			{  perror("[PROCESS] Recv msg error!");  exit(1);}

			//printf("recv from domain socket of client: %s\n", recv_ubuf);
			u_msg= StrtoUMsg(recv_ubuf, u_msg);//recv_buf has port_string
		if(findMtable(u_msg->sport, odr->mt)<0) //no such sun_path in mt
				{
				   // printf("insert!\n");
				    pnum= Max_number(odr->mt);
				   // printf("insert MT with #: %d, fname=%s\n", pnum+1,u_msg->sport);
			            odr->mt= insertMtable(pnum+1, trim(u_msg->sport), odr->mt);
				}

			 msg= CreateMsg(msg, u_msg->src_addr,u_msg->dest_addr);
			 msg->dport= u_msg->dport;
			 msg->sport= get_port_number(u_msg->sport, odr->mt);
			 msg->buflen=strlen(u_msg->buffer);
			 msg->buffer = malloc(strlen(u_msg->buffer) * sizeof(char));
			 strcpy(msg->buffer, u_msg->buffer);
			// strcpy(res, MsgtoStr(msg));
			// freeMsg(msg);
			 freeUMsg(u_msg);
			 process_umsg(msg, odr,st, recv_buf); 				
		}
		else  
			break;
	}

	unlink(ODR_SUN_PATH);
	close(odr->pfd);
	close(odr->ufd);
	
	exit(0);
}

int
process_rreq(void* buffer, struct ODR *odr, int from_index)
{
	struct ethhdr		*eh;
	struct RREQ		*req;
	struct RREP		*rep;
	int64_t			ts= getTS();
	unsigned char		Ether_saddr[6], *Ether_daddr;
	char			rreq_msg[100],rrep_msg[60];
	int			flag=0, count, ind, slen, i;
	eh= (struct ethhdr *) buffer;

	printf("[PROCESS] Get a RREQ from interface index %d, MAC ADDRESS: ", from_index);
	i = IF_HADDR;
	do {
		printf("%.2x:", eh->h_source[i] & 0xff);
	    } while (--i > 0);
	printf("\n");

	req=StrtoRREQ(buffer+14, req);
	
	//printf("req: bid=%d, saddr=%s, daddr=%s, sport=%d, dport=%d, hc=%d, rrepsent=%d, discovery=%d\n", req->broadcast_id, req->src_addr, req->dest_addr, req->sport, req->dport, req->hop_cnt, req->rrepsent, req->discovery);

	if(findBtable(req->broadcast_id, trim(req->src_addr), odr->bt)==1)
	  {printf("[PROCESS] This RREQ already been processed\n"); return 1;}

	if(req->rrepsent==1)
	 return 0;
	
	//insert broadcast_id into bt 
	 odr->bt= insertBtable(req->broadcast_id, trim(req->src_addr), odr->bt);  

	if(findEntry(req->src_addr, odr->rt)<0)  //such entry not exist	
		odr->rt=insertEntry(req->src_addr, eh->h_source,req->hop_cnt+1,from_index, ts, odr->rt);
	 	
	else
	{
	  //if(get_hop_cnt(req->src_addr,odr->rt)> req->hop_cnt ||(get_hop_cnt(req->src_addr,odr->rt)== req->hop_cnt && strncmp(get_next_hop(req->src_addr,odr->rt), eh->h_source,ETH_ALEN)!=0 )|| req->discovery==1)
	if(get_hop_cnt(req->src_addr,odr->rt)> req->hop_cnt || req->discovery==1)
	
		//update route table
		odr->rt=updateEntry(req->src_addr,eh->h_source, req->hop_cnt+1, from_index, ts, odr->rt);
	}

//////////////////////////////////////////////////////////////////////////////
	if(findEntry(trim(req->dest_addr), odr->rt)==1 || strcmp(trim(req->dest_addr), trim(odr->ip_addr))==0)
	{	
		//printf("[PROCESS] at dest or intermidate node has route to dest!\n");
		if(findEntry(req->dest_addr, odr->rt)==1)
		{
			printf("[PROCESS] RREQ reached at intermediate node has route to Destination!\n");
			if(req->discovery == 0)
			{
		 		req->rrepsent= 1; 
				flag=1;  //set hop_cnt for RREP to be 1
			}
		printf("[PROCESS] keep flooding the RREQ!\n"); 
		req->hop_cnt++;
		strcpy(rreq_msg, RREQtoStr(req));
		memset(buffer,'\0', ETH_FRAME_LEN);
		sendRREQ(odr->pfd, rreq_msg, odr, buffer);
		return 1;
		}
		
		else  //at destination
			{printf("[PROCESS] RREQ reached at destination!\n");  flag=2;} 

			//update routing table
  			count=0;
		      if(flag==1 || flag==2)
			{
			//send RREP
			 if(flag==1)   count++;
			rep=CreateRREP(rep, req->src_addr, req->dest_addr);
			rep->hop_cnt= count;
			rep->discovery= req->discovery;
			rep->sport= req->sport;
			rep->dport= req->dport;
			Ether_daddr= get_next_hop(trim(req->src_addr), odr->rt);
			printf("[PROCESS] the next_hop found in routing table:");
			i = IF_HADDR;
			do {
				printf("%.2x:", Ether_daddr[i] & 0xff);
	   		   } while (--i > 0);
			printf("\n");

			//printf("rep: hc=%d, discovery=%d, saddr=%s, daddr=%s, sport=%d, dport=%d\n", rep->hop_cnt, rep->discovery, rep->src_addr, rep->dest_addr, rep->sport, rep->dport);

			ind= get_index(trim(req->src_addr), odr->rt);
			printf("[PROCESS] send a RREP to interface index=%d\n", ind);
			
			strcpy(rrep_msg, RREPtoStr(rep));
			//printf("rreq_msg=%s\n", rrep_msg);
			freeRREQ(req);
			freeRREP(rep);	
			sendRREP(odr->pfd, rrep_msg, Ether_daddr, ind, buffer, odr);
				return 1;
			}
	}

	else
	{
		printf("[PROCESS] Update and keep sending RREQ!\n");
		req->hop_cnt=req->hop_cnt+1;
		strcpy(rreq_msg, RREQtoStr(req));
		//printf("new RREQ: %s\n", rreq_msg);
		freeRREQ(req);
		//memset(buffer,'\0', ETH_FRAME_LEN);
		sendRREQ(odr->pfd, rreq_msg, odr,buffer);
		return 1;
	}
	
	
	return 1;
}


int
process_rrep(void* buffer,struct ODR *odr, int from_index, int staleness)
{
	struct sockaddr_un	servaddr;
	struct ethhdr		*eh;
	struct RREQ		*req;
	struct RREP		*rep;
	struct Msg		*msg;
	int64_t			ts= getTS();
	char			rreq_msg[100],  rrep_msg[60], odr_msg[STLEN];
	int			slen, ind, i, rv;
	char			*messge;
	unsigned char		*Ether_daddr;
	eh= (struct ethhdr*) buffer;

	printf("[PROCESS] Get an RREP from: ");
	i = IF_HADDR;
	do {
		printf("%.2x:", eh->h_source[i] & 0xff);
	    } while (--i > 0);
	printf("\n");
	rep=StrtoRREP(buffer+14, rep);
	//printf("rep: saddr=%s, daddr=%s, sport=%d, dport=%d, hc=%d, discovery=%d\n", rep->src_addr, rep->dest_addr, rep->sport, rep->dport, rep->hop_cnt, rep->discovery);

	if(findEntry(rep->dest_addr, odr->rt)<0)  //such entry not exist
		odr->rt=insertEntry(rep->dest_addr, eh->h_source ,rep->hop_cnt+1, from_index, ts, odr->rt);

	else 
	{
	  if(stale(rep->dest_addr, odr->rt, staleness)==1)  //if the entry is stale
	  {
		printf("[PROCESS] This entry has dest_node:%s is stale!\n", rep->dest_addr);
		odr->rt=deleteEntry(rep->dest_addr, odr->rt);
		//flood a RREQ with discoery flag=1 from src_addr to dest_addr
		odr->broadcast_id++;
		req= CreateRREQ(req, odr->broadcast_id, rep->dest_addr, rep->src_addr);
		req->discovery= 1;
		req->rrepsent= 0;
		req->hop_cnt= rep->hop_cnt+1;
		//memcpy(req->dest_addr, rep->src_addr, strlen(rep->src_addr));
		//memcpy(req->src_addr, rep->dest_addr, strlen(rep->dest_addr));
		req->sport= rep->dport;
		req->dport= rep->sport; 
	odr->bt= insertBtable(req->broadcast_id, rep->dest_addr, odr->bt);  //check if it exist for not
		strcpy(rreq_msg, RREQtoStr(req));
		freeRREQ(req);
		sendRREQ(odr->pfd, rreq_msg, odr, buffer);
   		return 1;
	  }
	  
		if(get_hop_cnt(rep->dest_addr,odr->rt)> rep->hop_cnt || rep->discovery==1)
		{printf("[PROCESS] Update route table!\n");
		odr->rt=updateEntry(rep->dest_addr, eh->h_source, rep->hop_cnt+1, from_index, ts, odr->rt);}

	}
	
	if(strcmp(trim(rep->src_addr), trim(odr->ip_addr))==0)
	{
			printf("[PROCESS] Reached at destination node, send to Domain socket!\n");
			bzero(&servaddr, sizeof(servaddr));	
			servaddr.sun_family = AF_LOCAL;
			
			msg= CreateMsg(msg,rep->src_addr, rep->dest_addr);
			msg->hop_cnt=0;
			msg->sport= rep->sport;
			msg->dport= rep->dport;
			//printf("client domain file:%s\n", get_sun_path(rep->sport, odr->mt));
			strcpy(servaddr.sun_path, get_sun_path(rep->sport, odr->mt));  
			
                if(rep->discovery==1) //at destion received RREP
		{
			printf("[PROCESS] route has been established, destination node ask client to send message again!\n");
			msg->id= 1;
			msg->buffer= "Rediscovery done!";
			msg->buflen= strlen(msg);
		}
		else//client
		{
			printf("[PROCESS] Client got RREP from destination node!\n");
			msg->id= 2;
			msg->buffer= "client get RREP";
			msg->buflen= strlen(msg);
	    	}

			strcpy(odr_msg, MsgtoStr(msg));
			freeMsg(msg);
			freeRREP(rep);
			printf("[PROCESS] send msg to domain client!\n");
			Sendto(odr->ufd, odr_msg, 100 , 0, &servaddr, sizeof(struct sockaddr_un));
			return 1;
		
	}

	else
	{
		printf("[PROCESS] update and keep sending RREP!\n"); 	
		rep->hop_cnt= rep->hop_cnt++;
		Ether_daddr= get_next_hop(rep->src_addr, odr->rt);
		ind= get_index(rep->src_addr, odr->rt);
		printf("[PROCESS] Send to interface index=%d\n", ind);
		strcpy(rrep_msg, RREPtoStr(rep));
		//printf("RREP=%s\n", rrep_msg);
		freeRREP(rep);
		memset(buffer, '\0', ETH_FRAME_LEN);
		sendRREP(odr->pfd, rrep_msg, Ether_daddr, ind, buffer, odr);
		return 1;
	}
	
	return 1;
}



int
process_umsg(struct Msg	*msg, struct ODR *odr, int staleness, void* e_buf)
{
	int			slen, ind, rv, i, len;
	struct sockaddr_un	servaddr;
	struct RREQ		*req;
	int64_t			ts= getTS();
	char			rreq_msg[100], msg_msg[STLEN];
	unsigned char		*Ether_daddr;
		
	  printf("[PROCESS] Received a msg from unix domain socket!\n");
	 	strcpy(msg_msg, MsgtoStr(msg));
		//printf("msg_msg=%s\n", msg_msg);
		
	//  printf("msg id=%d, hc=%d,saddr=%s, daddr=%s,sport=%d, dport=%d, buffer=%s, buflen=%d\n", msg->id, msg->hop_cnt, msg->src_addr, msg->dest_addr,msg->sport, msg->dport, msg->buffer, msg->buflen);
	
	//if the dest already reached, give it to domain socket and return
	if(strcmp(odr->ip_addr, msg->dest_addr)==0)	
	{
	printf("[PROCESS] msg reached at Destination, send to domain socket!\n");
	bzero(&servaddr, sizeof(servaddr));	/* fill in server's address */
	servaddr.sun_family = AF_LOCAL;
	//printf("sun_path_name=%s\n", get_sun_path(msg->dport, odr->mt));
	strcpy(servaddr.sun_path, get_sun_path(msg->dport, odr->mt)); 
	freeMsg(msg);
	Sendto(odr->ufd, msg_msg, strlen(msg_msg), 0, &servaddr, sizeof(struct sockaddr_un));
		return 1;
	}
	
	
	if(stale(msg->dest_addr, odr->rt, staleness)==1 || findEntry(trim(msg->dest_addr), odr->rt)<0)  
	{
		printf("[PROCESS] Issue a RREQ due to Entry to %s in routing table is stale or not exist!\n", msg->dest_addr);
		if(stale(msg->dest_addr, odr->rt, staleness)==1)
			odr->rt=deleteEntry(msg->dest_addr, odr->rt);
		printf("[PROCESS] Issue a RREQ!\n");
		odr->broadcast_id= odr->broadcast_id+1;
		req= CreateRREQ(req, odr->broadcast_id, msg->src_addr, msg->dest_addr);
		req->discovery= 0;
		
		if(stale(msg->dest_addr, odr->rt, staleness)==1)
		 	req->discovery= 1;
		req->rrepsent= 0;
		req->hop_cnt= 0;
		req->sport= msg->sport;
		req->dport= msg->dport;
		odr->bt= insertBtable(req->broadcast_id, req->src_addr, odr->bt);  
		//printf("rreq bid=%d,hc=%d,saddr=%s, daddr=%s,sport=%d, dport=%d, resent=%d, discovery=%d\n", req->broadcast_id, req->hop_cnt, req->src_addr, req->dest_addr,req->sport, req->dport, req->rrepsent, req->discovery);
		
		strcpy(rreq_msg, RREQtoStr(req));
		freeRREQ(req);
		//printf("rreq:%s\n", rreq_msg);
		freeMsg(msg);
		rv= sendRREQ(odr->pfd, rreq_msg, odr, e_buf);
		return 1;
	}

	else
	//look up in rtable to find next_hop and forward it
	{
		msg->hop_cnt= msg->hop_cnt++;
		ind= get_index(msg->dest_addr, odr->rt);
		printf("[PROCESS] Send msg to interface index= %d\n", ind);
		
		memset(e_buf, '\0', ETH_FRAME_LEN);
		
		//printf("get next_hop!\n");
		Ether_daddr= get_next_hop(trim(msg->dest_addr), odr->rt);
			printf("[PROCESS] the next_hop found in routing table to %s:", msg->dest_addr);
			i = IF_HADDR;
			do {
				printf("%.2x:", Ether_daddr[i] & 0xff);
	   		   } while (--i > 0);
			printf("\n");

		freeMsg(msg);
		sendMsg(odr->pfd, msg_msg, Ether_daddr, ind, e_buf, odr);
		return 1;
	}
	
	return 1;
}



int
process_pmsg(void* buffer, struct ODR *odr, int staleness)
{
	int			slen, ind, i, rv;
	struct sockaddr_un	servaddr;
	struct ethhdr		*eh;
	struct RREQ		*req;
	struct Msg		*msg;
	int64_t			ts= getTS();
	unsigned char		*Ether_daddr;
	char			rreq_msg[100], msg_msg[STLEN], umsg[STLEN];
	
	printf("[PROCESS] Received a msg from PF_packet, MAC ADDRESS:");	  
	eh= (struct ethhdr*) buffer;
	
	i = IF_HADDR;
	do {
		printf(":%.2x", eh->h_source[i] & 0xff);
	    } while (--i > 0);
	printf(" to ");
	i = IF_HADDR;
	do {
		printf(":%.2x", eh->h_dest[i] & 0xff);
	    } while (--i > 0);

	printf("\n");
	msg=StrtoMsg(buffer+14, msg);

	//  printf("msg id=%d, hc=%d,saddr=%s, daddr=%s,sport=%d, dport=%d, buffer=%s, buflen=%d\n", msg->id, msg->hop_cnt, msg->src_addr, msg->dest_addr,msg->sport, msg->dport, msg->buffer, msg->buflen);
	//if the dest already reached, give it to domain socket and return

	if(strcmp(odr->ip_addr, msg->dest_addr)==0)	
	{
	printf("[PROCESS] msg reached at Destination, send to domain socket!\n");
	  strcpy(umsg, MsgtoStr(msg));
	  //printf("send msg:%s\n", umsg);
		
	bzero(&servaddr, sizeof(servaddr));	
/* fill in server's address */
	servaddr.sun_family = AF_LOCAL;
	//printf("sun_path_name=%s\n", get_sun_path(msg->dport, odr->mt));
	strcpy(servaddr.sun_path, get_sun_path(msg->dport, odr->mt)); 
	freeMsg(msg);
	Sendto(odr->ufd, umsg, 100, 0, &servaddr, sizeof(struct sockaddr_un));
		return 1;
	}
	
	if(stale(msg->dest_addr, odr->rt, staleness)==1 || findEntry(msg->dest_addr, odr->rt)<0)  
	{
		if(stale(msg->dest_addr, odr->rt, staleness)==1)
			{
			printf("[PROCESS] Entry is stale!\n");
			odr->rt=deleteEntry(msg->dest_addr, odr->rt);
			}
		printf("[PROCESS] Issue a RREQ due to Entry to %s in routing table is stale or not exist!\n", msg->dest_addr);

		odr->broadcast_id= odr->broadcast_id+1;
		req= CreateRREQ(req, odr->broadcast_id, msg->src_addr, msg->dest_addr);
		req->discovery= 0;
		
		if(stale(msg->dest_addr, odr->rt, staleness)==1)
		 	req->discovery= 1;
		req->rrepsent= 0;
		req->hop_cnt= 0;
		req->sport= msg->sport;
		req->dport= msg->dport;
		
		odr->bt= insertBtable(req->broadcast_id, req->src_addr, odr->bt);  //check if it exist for not
		//printf("rreq bid=%d,hc=%d,saddr=%s, daddr=%s,sport=%d, dport=%d, resent=%d, discovery=%d\n", req->broadcast_id, req->hop_cnt, req->src_addr, req->dest_addr,req->sport, req->dport, req->rrepsent, req->discovery);
		
		strcpy(rreq_msg, RREQtoStr(req));
		freeRREQ(req);
		//printf("rreq:%s\n", rreq_msg);
		freeMsg(msg);
		rv= sendRREQ(odr->pfd, rreq_msg, odr, buffer);
		return 0;
	}

	else
	//look up in rtable to find next_hop and forward it
	{
		msg->hop_cnt= msg->hop_cnt++;
		ind= get_index(msg->dest_addr, odr->rt);
		printf("[PROCESS] Send msg to interface index= %d\n", ind);
		strcpy(msg_msg, MsgtoStr(msg));
		//printf("send msg:%s\n", msg_msg);
		//printf("get next_hop!\n");
		Ether_daddr= get_next_hop(trim(msg->dest_addr), odr->rt);
			//printf("the next hop found in routing table:");
			printf("[PROCESS] the next_hop found in routing table to %s ", msg->dest_addr);
			i = IF_HADDR;
			do {
				printf(":%.2x", Ether_daddr[i] & 0xff);
	   		   } while (--i > 0);
			printf("\n");

		freeMsg(msg);
		sendMsg(odr->pfd, msg_msg, Ether_daddr, ind, buffer, odr);
	}
	
	return 1;

}


int 
sendRREQ(int pfd, char *msg, struct ODR *odr, void* buffer)
{
		int			rv, i, *p;
		INode 			cur_ptr;
		struct sockaddr_ll	broadcast_addr;
		unsigned char* etherhead= buffer;
		unsigned char* data= buffer+14;
		struct ethhdr *eh =(struct ethhdr *)etherhead;
  		unsigned char src_mac[6]= {0x00,0x00,0x00,0x00,0x00,0x00};
		unsigned char dest_mac[6]= {0xff,0xff,0xff,0xff,0xff,0xff};
		printf("[PROCESS] Send a RREQ ");
		memset(buffer,'\0', ETH_FRAME_LEN);
		
		broadcast_addr.sll_family   = PF_PACKET;	
		broadcast_addr.sll_protocol = htons(ODR_PRO);	
		broadcast_addr.sll_hatype   = ARPHRD_ETHER;
		broadcast_addr.sll_pkttype  = PACKET_OTHERHOST;
		broadcast_addr.sll_halen    = ETH_ALEN;		
		memset(broadcast_addr.sll_addr,0xff, ETH_ALEN);
		broadcast_addr.sll_addr[6]  = 0x00;/*not used*/
		broadcast_addr.sll_addr[7]  = 0x00;/*not used*/
		
		memcpy((void*)buffer, (void*)dest_mac, ETH_ALEN);
		
		eh->h_proto = htons(ODR_PRO);
		memcpy(data, msg, strlen(msg));
		//printf("data=%s\n", data);
		
  		cur_ptr=odr->it->next;
  		while(cur_ptr!= NULL && cur_ptr->index>0)
  		{
     			printf("from interface index=%d, ", cur_ptr->index);
			broadcast_addr.sll_ifindex  = cur_ptr->index;
     			printf("MAC ADDRESS "); 

			i = IF_HADDR;
			do {
				src_mac[i]= cur_ptr->mac_addr[i];
				printf(":%.2x", src_mac[i] & 0xff);
	    		} while (--i > 0);
			printf("\n");

			memcpy((void*)(buffer+ETH_ALEN), (void*)src_mac, ETH_ALEN);
			rv = sendto(pfd, buffer, ETH_FRAME_LEN, 0, (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
			if (rv == -1) 
		   	{ perror("[PROCESS] Sending Ethernet frame error!");  exit(1); }
     		
     			cur_ptr=cur_ptr->next;
 		 }

		return 1;
}


int
sendRREP(int pfd, char *msg, unsigned char *Edaddr, int index, void* buffer, struct ODR *odr)
{
		//int			rv, i;
		int			rv, i, *p;
		INode 			cur_ptr;
		struct ethhdr		*eh;
		struct sockaddr_ll	unicast_addr;
		unsigned char		*data;
		unsigned char src_mac[6]= {0x00,0x00,0x00,0x00,0x00,0x00};
		
		eh= (struct ethhdr *)buffer;	
		
		data = buffer + 14;
		memset(buffer, '\0', ETH_FRAME_LEN);
		printf("[PROCESS] Send a RREP ");
		unicast_addr.sll_family   = PF_PACKET;	
		unicast_addr.sll_protocol = htons(ODR_PRO);	
		unicast_addr.sll_ifindex  = index;
		unicast_addr.sll_hatype   = ARPHRD_ETHER;
		unicast_addr.sll_pkttype  = PACKET_OTHERHOST;
		unicast_addr.sll_halen    = ETH_ALEN;	
		memcpy(unicast_addr.sll_addr, Edaddr, IF_HADDR);	
		unicast_addr.sll_addr[6]  = 0x00;/*not used*/
		unicast_addr.sll_addr[7]  = 0x00;/*not used*/

		eh->h_proto = htons(ODR_PRO);
		memcpy(data, msg, strlen(msg));
		//printf("data=%s\n", data);

		printf("to interface index %d, MAC ADDRESS ", index);
		i = IF_HADDR;
	do {
		eh->h_dest[i]= Edaddr[i];
		printf(":%.2x", eh->h_dest[i] & 0xff);
	    } while (--i > 0);
	printf("\n");

		
		cur_ptr=odr->it->next;
  		while(cur_ptr!= NULL && cur_ptr->index>0)
  		{	printf("[PROCESS] Send RREP from interface index=%d, ", cur_ptr->index);
			unicast_addr.sll_ifindex  = cur_ptr->index;
     			printf("MAC ADDRESS "); 

			i = IF_HADDR;
			do {
				src_mac[i]= cur_ptr->mac_addr[i];
				printf(":%.2x", src_mac[i] & 0xff);
	    		} while (--i > 0);
			printf("\n");

			memcpy((void*)(buffer+ETH_ALEN), (void*)src_mac, ETH_ALEN);
			rv = sendto(pfd, buffer, ETH_FRAME_LEN, 0, (struct sockaddr*)&unicast_addr, sizeof(unicast_addr));
			if (rv == -1) 
		   	{ perror("[PROCESS] Sending Ethernet frame error!");  exit(1); }
     		
     			cur_ptr=cur_ptr->next;
 		 }
		return 1;
}



int
sendMsg(int pfd, char *msg, unsigned char *Edaddr, int index, void* buffer, struct ODR *odr)
{
		int			rv, i;
		struct ethhdr		*eh;
		struct sockaddr_ll	unicast_addr;
		unsigned char		*data;
		eh= (struct ethhdr *)buffer;	
		
		memset(buffer, '\0', ETH_FRAME_LEN);
		//printf("get msg:%s\n", msg);
		unicast_addr.sll_family   = PF_PACKET;	
		unicast_addr.sll_protocol = htons(ODR_PRO);	
		unicast_addr.sll_ifindex  = index;
		unicast_addr.sll_hatype   = ARPHRD_ETHER;
		unicast_addr.sll_pkttype  = PACKET_OTHERHOST;
		unicast_addr.sll_halen    = ETH_ALEN;	
		memcpy(unicast_addr.sll_addr, Edaddr, IF_HADDR);	
		unicast_addr.sll_addr[6]  = 0x00;/*not used*/
		unicast_addr.sll_addr[7]  = 0x00;/*not used*/

		memcpy((void*)buffer, (void*)Edaddr, ETH_ALEN);
		memcpy((void*)(buffer+ETH_ALEN), (void*)get_mac_addr(odr->it), ETH_ALEN);
			
		printf("[PROCESS] Send message from ");
		i = IF_HADDR;
	do {
		printf(":%.2x", eh->h_source[i] & 0xff);
	    } while (--i > 0);
	printf(" to ");
		i = IF_HADDR;
	do {
		printf(":%.2x", eh->h_dest[i] & 0xff);
	    } while (--i > 0);
	printf("\n");

		eh->h_proto = htons(ODR_PRO);
		data = buffer + 14;
		memcpy(data, msg, strlen(msg));
		//printf("data=%s\n", data);
		rv = sendto(pfd, buffer, ETH_FRAME_LEN, 0, (struct sockaddr*)&unicast_addr, sizeof(unicast_addr));
		if (rv == -1) 
			{ perror("[PROCESS] Sending Ethernet frame error!");  exit(1); }
		return rv;
}
