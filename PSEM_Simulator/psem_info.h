//general use
#define RECV_BUFSIZE 256
#define SERVER 0
#define CLIENT 1

//general PSEM packet info
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

// Terminate info
#define TERMINATE 0x21

// TODO: Terminate for client and server cleanup

// Table info
#define TOTAL_TABLES 82 // Total number of tables supported (all besides one are not in use)
#define ENTRY_SIZE 32 // Each table entry should be 32 bytes
#define PASSWORD_TBL_ID 51 // this is the table that holds passwords in PSEM protocol
#define PASSWORD_LEN 21 // The password is 21 bytes long
#define ACCESS_READ_ONLY 1 // Readonly user permissions
#define ACCESS_READ_WRITE 2 // Read/write user permissions
#define ACCESS_ADMIN 3 // Admin user permission
#define MAX_LOGIN_ATTEMPTS 3 // Maximum number of attempts before user is locked out
#define NUM_USERS 3 // Total number of users to store in the database for testing

// Structure for compact datetimes in password entry (7 bytes in size)
struct datetime {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};

// Structure for an entry in the password table (32 bytes in size)
struct password_entry {
    uint8_t user_id;
    uint8_t password[21];
    uint8_t access_level;
    uint8_t max_attempts;
    uint8_t lock_status;
    struct datetime dt;
};