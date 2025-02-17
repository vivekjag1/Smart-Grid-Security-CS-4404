//general use
#define RECV_BUFSIZE 256
#define SERVER 0
#define CLIENT 1

//general PSEM packet info
#define SECURITY_TBL_ID 42 // this is the table that holds passwords in PSEM protocol
#define PSEM_DEFAULT_BAUD 9600
#define PSEM_STP 0xEE
#define PSEM_IDENTITY 0x00
#define PSEM_OK 0x00
#define PSEM_ERR 0x01
#define PSEM_ACK 0x06
#define PSEM_NAK 0x15

//read service PSEM info
#define READ_FULL 0x30
#define READ_OFFSET_REQ 0x3F


//LOGON specific stuff 
//logon service PSEM info 
#define LOGON 0x50
#define LOGON_USERID 1111
#define LOGON_USER 0x0102030405060708090A

//ident info 
#define IDENT 0x20
#define STANDARD 0x00
#define VERSION 2 
#define REV 0 