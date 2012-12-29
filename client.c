#include	"unp.h"
#include	"ODR.h"
#include	<sys/socket.h>
#include 	"hw_addrs.h"

char* get_eth0_addr();

int
main(int argc, char **argv)
{
	int			filedes=-1, sockfd, flag=0, rv, p=1, slen, enter=0;
	struct sockaddr_un	cliaddr, servaddr;			
	struct hostent 		*serv, *cli, *he;
	struct Msg		*msg;
	struct UMsg		*u_msg;
	struct in_addr		temp_addr;
	struct timeval 		tim;
        fd_set 			socks;
	char			*pname,recv_msg[200], *dest_addr, src_addr[20], odr_msg[STLEN], line[MAXLINE], port_number[20], recvline[100], *message;
	FILE			*f;
	
	f= fopen(CLI_SUN_PATH, "W+");
	
	unlink(CLI_SUN_PATH);
	printf("[PROCESS] Please input which machine want to connect(e.g: vm2)\n");
	sockfd = Socket(AF_LOCAL, SOCK_DGRAM, 0);
	bzero(&cliaddr, sizeof(cliaddr));		/* bind an address for us */
	cliaddr.sun_family = AF_LOCAL;

	//pname= "93359_CLI_PATH";
	
	//memset(pname, '\0', sizeof(pname));
	//strncpy(pname, "myTmpFile-XXXXXX", 30);
	//filedes= mkstemp(pname);
	
	//printf("bind to file:%s\n", CLI_SUN_PATH);
	strcpy(cliaddr.sun_path, CLI_SUN_PATH);
	Bind(sockfd, (SA *) &cliaddr, sizeof(cliaddr));
	
	bzero(&servaddr, sizeof(servaddr));	/* fill in server's address */
	servaddr.sun_family = AF_LOCAL;
	strcpy(servaddr.sun_path, ODR_SUN_PATH);  //change to a well-known port number
	strcpy(src_addr, get_eth0_addr());
	
	//if(listen(sockfd, 10)==-1) {perror("listen error!"); exit(1);}


	for(;;)
        {
		if (Fgets(line, MAXLINE, stdin) != NULL) 
		{
			serv= gethostbyname(trim(line));
                        if(serv==NULL)
				{printf("[PROCESS] No such machine!\n"); break;}
			memcpy(&(temp_addr),serv->h_addr, serv->h_length);
			dest_addr= inet_ntoa(temp_addr);
			printf("[PROCESS] ask to connect to %s with IP ADDRESS: %s\n", serv->h_name, dest_addr);

			message="hello";
			u_msg= CreateUMsg(u_msg, dest_addr, src_addr, cliaddr.sun_path, PORT_NUM, message);
			slen= strlen(UMsgtoStr(u_msg));
			//printf("str len=%d\n", slen);
			
			memset(&odr_msg, '\0', sizeof(odr_msg));
			strcpy(odr_msg, UMsgtoStr(u_msg));
			printf("[PROCESS] Send message: %s to %s\n", message, serv->h_name);
	Sendto(sockfd, odr_msg, STLEN, 0, &servaddr, sizeof(struct sockaddr_un));
			freeUMsg(u_msg);

					//wait msg to send back
		  	while(1)
	     	 	{    
				printf("\n[PROCESS] Client listening!\n");
				FD_ZERO(&socks);
				FD_SET(sockfd, &socks);
				tim.tv_sec = TIMEOUTVAL;
		    	if (select(sockfd+1, &socks, NULL, NULL, &tim))
		  	 {
				
			 if(rv=recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL)<0) 
		       		{  perror("[PROCESS] recv msg error!");  exit(1);}
				//printf("received msg: %s\n", recvline);
				msg= StrtoMsg(recvline, msg);
			printf("[PROCESS] received message: '%s' from server!\n", trim(msg->buffer));
				//he=gethostbyaddr(trim(msg->dest_addr), strlen(msg->dest_addr), AF_INET);
				//if(he!=NULL)
			//printf("msg:'%s' recived from %s\n",msg->buffer, he->h_name);
					//check the message type then process
			
			if(msg->id==2 || msg->id==3)		
						//send msg again
				{
				printf("[PROCESS] Client get a RREP from server, so need to send the message '%s' again\n", message);
				Sendto(sockfd, odr_msg, STLEN, 0, &servaddr, sizeof(struct sockaddr_un));
				}
		
			}
		   	else
		  	 {
				printf( "Timeout!\n");
				//flag=1; 
				break;
		  	 }      
	      	
	}//end while
     }//end if
}//end for

       unlink(cliaddr.sun_path);
	exit(0);
       
}



char*
get_eth0_addr()
{
	struct hwa_info	*hwa, *hwahead;
	struct sockaddr	*sa;
	char   *ptr, ip_addr[20];
	int    i, prflag;

	//ip_addr=[20];
	memset(&ip_addr, '\0', sizeof(ip_addr));
			
	for (hwahead = hwa = Get_hw_addrs(); hwa != NULL; hwa = hwa->hwa_next) {
		//printf("type: %s :%s", hwa->if_name, ((hwa->ip_alias) == IP_ALIAS) ? " (alias)\n" : "\n");
		
		if ( (sa = hwa->ip_addr) != NULL)
			{
				//if(memcmp(hwa->if_name,"eth0", 4)==0)
				//printf("type: %s, ip_addr: %s\n", hwa->if_name,Sock_ntop_host(sa, sizeof(*sa)) );
				if(strcmp(hwa->if_name,"eth0")==0)
			  {
		memcpy(ip_addr, Sock_ntop_host(sa, sizeof(*sa)), strlen(Sock_ntop_host(sa, sizeof(*sa))));
			  printf("[PROCESS] ETH0 IP address = %s\n", ip_addr);}
			}
			
		//printf("\ninterface index = %d\n\n", hwa->if_index);
	}

	free_hwa_info(hwahead);
	return ip_addr;
}
