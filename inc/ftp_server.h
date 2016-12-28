#ifndef _FTP_SERVER_H_
#define _FTP_SERVER_H_

#include "stm32f4xx.h"

enum
{
  FTP_CLOSED,
  FTP_OPEN
};
#define FTP_DATA_USE_PASV          0   // 1: use pasive mode, 2: not to use pasive mode 

#define FTP_TASK_PRIORITY          tskIDLE_PRIORITY + 3

#define FTP_TCP_CONTROL_PORT       21
#define FTP_TCP_TIMEOUT_REC_PORT    5000
#define FTP_TCP_TIMEOUT_DATA_PORT   5000

#define FTP_REQUEST_SPACE          150/*Client request should not be longer*/
#define FTP_LIST_BUFF_SEND         550

/*FTP Server Requests*/
#define FTP_USER_REQUEST           "USER"
#define FTP_PASS_REQUEST           "PASS"              
#define FTP_QUIT_REQUEST           "QUIT"
#define FTP_PORT_REQUEST           "PORT"
#define FTP_LIST_REQUEST           "LIST"
#define FTP_STOR_REQUEST           "STOR"
#define FTP_RETR_REQUEST           "RETR"
#define FTP_DELE_REQUEST           "DELE"

/*FTP Server Response*/
#define FTP_WELCOME_RESPONSE        "220 Service Ready\r\n"
#define FTP_USER_RESPONSE           "331 USER OK. PASS needed\r\n"
#define FTP_PASS_FAIL_RESPONSE      "530 NOT LOGGUED IN\r\n"
#define FTP_PASS_OK_RESPONSE        "230 USR LOGGUED IN\r\n"
#define FTP_PORT_OK_RESPONSE        "200 PORT OK\r\n"
#define FTP_LIST_OK_RESPONSE        "150 LIST OK\r\n"
#define FTP_RETR_OK_RESPONSE        "150 RETR OK\r\n"
#define FTP_STOR_OK_RESPONSE        "150 STOR OK\r\n"
#define FTP_DELE_OK_RESPONSE        "150 DELE OK\r\n"
#define FTP_QUIT_RESPONSE           "221 BYE OK\r\n"
#define FTP_TRANSFER_OK_RESPONSE    "226 Transfer OK\r\n"
#define FTP_WRITE_FAIL_RESPONSE     "550 File unavailable\r\n"
#define FTP_CMD_NOT_IMP_RESPONSE    "502 Command Unimplemented\r\n"
#define FTP_DATA_PORT_FAILED        "425 Cannot open Data Port\r\n"
#define FTP_UNKNOWN_RESPONSE        "502 Unrecognized Command\r\n"
#define FTP_BAD_SEQUENCE_RESPONSE   "503 Bad Sequence of Commands\r\n"
#define FTP_LIST_FAIL_RESPONSE      "426 error LIST\r\n"

/********Prototype Functions************************************/

 
typedef struct
{
unsigned char cmdVal[32];
unsigned char cmdType; 
} PasvData;

static const char* month_array[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

static uint8_t
vFTPConnection(struct netconn *connfd, char *alloc_rq);

uint8_t
FTP_QUIT_OR_WRONG_REQUEST(struct netconn *connfd, char *alloc_rq);

static uint8_t
FTP_CloseDataPort(struct netconn *conn_data);

struct netconn *
FTP_OpenDataPort(struct ip_addr *add, uint16_t port);

uint8_t
FTP_Read_List_Of_Files(struct netconn *connfd);

uint8_t
FTP_Read_File(struct netconn *connfd,  char *alloc_rq);

uint8_t
FTP_Write_File(struct netconn *connfd, char *data);

#if	 FTP_DATA_USE_PASV 
void 
vFTPServerPasiv( void *pvParameters );
#endif

void 
FTP_PWD(struct netconn *connfd);

void
FTP_CWD(struct netconn *connfd, char *alloc_rq);

void 
vBasicFTPServer( void *pvParameters );

#endif
