#include	"unp.h"
#include	"ODR.h"
#include	<linux/if_ether.h>
#include	<sys/socket.h>

int
main(int argc, char **argv)
{
	int			sockfd, n,rv, tport;
	struct sockaddr_un	servaddr, cliaddr;
	struct Msg		*msg;
	struct UMsg		*u_msg;
	socklen_t		len;
	char			temp[20],port_number[10], snd_msg[20], recvline[MAXLINE], odr_msg[STLEN], *message;
	struct hostent		*serv, *cli;
	FILE			*f;
	struct timeval 		tim;
        fd_set 			socks;
	
	printf("[PROCESS] Start server!\n");
	f= fopen(DEST_SUN_PATH, "W+");
	unlink(DEST_SUN_PATH);
	sockfd = Socket(AF_LOCAL, SOCK_DGRAM, 0);
	
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sun_family = AF_LOCAL;
	strcpy(servaddr.sun_path, DEST_SUN_PATH);
	
	if((rv=bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)))<0)
	{perror("bind error!\n"); exit(0);}
	
	len = sizeof(cliaddr);	
	bzero(&cliaddr, sizeof(cliaddr));	/* fill in server's address */
	cliaddr.sun_family = AF_LOCAL;
	strcpy(cliaddr.sun_path, ODR_SUN_PATH);  //change to a well-known port number
 	printf("\n[PROCESS] SERVER listening!\n");
	while(1){

		FD_ZERO(&socks);
		FD_SET(sockfd, &socks);
		tim.tv_sec = TIMEOUTVAL;
		if (select(sockfd+1, &socks, NULL, NULL, &tim))
		{
		if(rv=recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL)<0) 
		{  perror("[PROCESS] receive message error!");  exit(1);}
		
		//printf("received msg: %s\n", recvline);
		msg= StrtoMsg(recvline, msg);
		//printf("msg id=%d, hc=%d,saddr=%s, daddr=%s,sport=%d, dport=%d, buffer=%s, buflen=%d\n", msg->id, msg->hop_cnt, msg->src_addr, msg->dest_addr,msg->sport, msg->dport, msg->buffer, msg->buflen);
		printf("[PROCESS] receive message: '%s' from client!\n", trim(msg->buffer));
		//cli=gethostbyaddr(msg->src_addr, strlen(msg->src_addr), AF_INET);
		//if(cli!=NULL)
			//printf("msg:'%s' recived from %s\n",msg->buffer, cli->h_name);
		
		if(msg->id==1)
		  message= "send again";  
		else
		message="hi back!";
		
		//if(serv!=NULL)
		printf("[PROCESS] Server repsonding to client by sending message: %s\n", message);//,  cli->h_name);
		u_msg= CreateUMsg(u_msg, msg->src_addr, msg->dest_addr, servaddr.sun_path, msg->sport, message);
		
		memset(&odr_msg, '\0', sizeof(odr_msg));
		strcpy(odr_msg, UMsgtoStr(u_msg));
		//printf("send msg: %s\n", odr_msg);
		freeMsg(msg);
		freeUMsg(u_msg);
	Sendto(sockfd, odr_msg, STLEN, 0, &cliaddr, sizeof(struct sockaddr_un));
		}	

	}

return 1;
}
