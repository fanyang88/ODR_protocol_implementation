#ifndef	__ODR_h
#define	__ODR_h

#include	<net/if.h>
#include	<linux/if_ether.h>
#include	"unp.h"

#define	ARPHRD_ETHER	1
#define ODR_SUN_PATH	"/tmp/ODR_PATH"
#define ODR_NUM		95
#define	DEST_SUN_PATH	"/tmp/SERV_PATH"
#define PORT_NUM	93
#define	CLI_SUN_PATH	"/tmp/CLI_PATH"	
#define CLI_NUM		94
#define	TIMEOUTVAL	10
#define CLEN		100  //char sequence length
#define ODR_PRO		93359
#define STLEN		200



typedef struct node * PNode;
typedef struct node
{
	char		*dest_node;	
	unsigned char	*next_hop;
	int		hop_cnt;
	int		index;
	int64_t		ts;
	PNode 		next;
	
}Node,* Rtable;

Rtable CreateRT(Rtable );
int findEntry(char *, Rtable );
int get_hop_cnt(char *,Rtable );
int get_index(char *,Rtable );
unsigned char* get_next_hop(char *,Rtable );
int length(Rtable );
Rtable insertEntry(char *,unsigned char *, int , int, int64_t , Rtable );
Rtable updateEntry(char *,unsigned char *, int , int, int64_t , Rtable );
int stale(char *, Rtable , int );
int deleteEntry(char *, Rtable );
void printRT(Rtable );

typedef struct mnode * MNode;
typedef struct mnode
{
	char		*path_name;	
	int		port_number;
	MNode 		next;
	
}Mnode,* Mtable;

Mtable CreateMT(Mtable );
char* get_sun_path(int , Mtable );
int Mlength(Mtable );
Mtable insertMtable(int , char *, Mtable );
int findMtable(char * , Mtable );
int Max_number(Mtable );
int get_port_number(char *, Mtable );

typedef struct inode * INode;
typedef struct inode
{
	unsigned char	*mac_addr;	
	int		index;
	INode 		next;
	
}Inode,* Itable;

Itable removedup(Itable );
void printItable(Itable );
unsigned char* get_mac_addr(Itable );
Itable insertItable(int , unsigned char *, Itable );
int Ilength(Itable );
Itable CreateIT(Itable );


typedef struct bnode * BNode;
typedef struct bnode
{
	int		bid;
	char		*ip_addr;
	BNode		next;
	
}Bnode,* Btable;
Btable CreateBT(Btable );
int findBtable(int , char *, Btable );
Btable insertBtable(int , char *, Btable );
int Blength(Btable );

struct RREQ
{
  	int		type;
	int		broadcast_id;
	int		hop_cnt;
	u_short		rrepsent:1;	
	u_short		discovery:1;
	char		*src_addr;
	char		*dest_addr;
	int		sport;
	int		dport;
	
};
struct RREQ* CreateRREQ(struct RREQ * , int id, char *, char *);
void freeRREQ(struct RREQ*);
struct RREQ* StrtoRREQ(char *, struct RREQ *);
char* RREQtoStr(struct RREQ *);

struct RREP
{
  	int		type;
	int		hop_cnt;
	char		*src_addr;
	char		*dest_addr;
	int		sport;
	int		dport;	
	u_short		discovery:1;
};
struct RREP* CreateRREP(struct RREP*, char *, char *);
void freeRREP(struct RREP *);
struct RREP* StrtoRREP(char *, struct RREP *);
char* RREPtoStr(struct RREP *);

struct Msg
{
	int		type;
	int		id;
	int		hop_cnt;
	int		buflen;
	char		*src_addr;
	char		*dest_addr;
	int		sport;
	int		dport;
	char		*buffer;
};
struct Msg* CreateMsg(struct Msg* , char *, char *);
void freeMsg(struct Msg *);
struct Msg* StrtoMsg(char *str, struct Msg *);
char* MsgtoStr(struct Msg *);


struct UMsg
{
	char		*src_addr;
	char		*dest_addr;
	char		*sport;
	int		dport;
	char		*buffer;
};
struct UMsg*
CreateUMsg(struct UMsg* ,char * ,char * ,char * ,int , char *);
void freeUMsg(struct UMsg *);
struct UMsg* StrtoUMsg(char *str, struct UMsg *);
char* UMsgtoStr(struct UMsg *);


 static struct ODR
{
	Rtable		rt;	
	Btable		bt;
	Mtable		mt;
	Itable		it;
	char		*ip_addr;
	unsigned char	*mac_addr;
	int		pfd;
	int		ufd;
	int		broadcast_id;
	
};

struct ODR* CreateODR(struct ODR* );

char* trim(char *str);
char* InttoString(int , int );
int64_t getTS();


#endif

	
