//general use
#define RECV_BUFSIZE 256

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
