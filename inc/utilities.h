#ifndef _UTILITIES_H_
#define _UTILITIES_H_

//#include "ff.h"
#include "diskio.h"
#include "sdio_sd.h"


/*FSL: flag used to request a memalloc inside netconn_rcv_req(...)*/
#define NETCONN_RCV_NETBUFFER         1
#define IP_DELIMITATOR                '.'
#define MAC_DELIMITATOR               IP_DELIMITATOR/*':'*/
#define STRING_END                    "\r\n"
#define STRING_DOUBLE_END             "\r\n\r\n"

#define str_len(x)        sizeof(x)-1

/*FSL: Stack space neede by each Task*/
#define WEBSERVER_STACK_SPACE		  160//OK
#define UART_BRIDGE_STACK_SPACE   144//OK
#define SPI_BRIDGE_STACK_SPACE    144//??
#define DHCP_STACK_SPACE          112//OK
#define LED_STACK_SPACE            32//NO
#define EMAILCLIENT_STACK_SPACE		 96//OK
#define SRL_BRIDGE_BUFFER_LIMIT    96//???
#define TERMINAL_STACK_SPACE      144//OK
#define SDCARD_STACK_SPACE        224//???
#define FTPSERVER_STACK_SPACE     144//OK

/*MIME Media Types*/
#define CONTENT_HTML      "text/html"                 /*HTM, HTML, SHTML*/
#define CONTENT_CSS       "text/css"                  /*CSS*/
#define CONTENT_TEXT      "text/plain"                /*TXT, FSL*/
#define CONTENT_JPG       "image/jpeg"                /*JPG, JPEG*/
#define CONTENT_GIF       "image/gif"                 /*GIF*/
#define CONTENT_BMP       "image/bmp"                 /*BMP*/
#define CONTENT_UNKNOWN   "application/octet-stream"  /*unknown MIME type*/

typedef const struct
{
    char *  mime_extension;
    char *  mime_type;
} MIME_TYPE;

/*
 * Macros for MIME Types
 */
#ifndef MIME_TYPE_HTM
#define MIME_TYPE_HTM    \
    {".HTM",CONTENT_HTML}
#endif

#ifndef MIME_TYPE_SHTML
#define MIME_TYPE_SHTML    \
    {".SHTML",CONTENT_HTML}
#endif

#ifndef MIME_TYPE_CSS
#define MIME_TYPE_CSS    \
    {".CSS",CONTENT_CSS}
#endif

#ifndef MIME_TYPE_TXT
#define MIME_TYPE_TXT    \
    {".TXT",CONTENT_TEXT}
#endif

#ifndef MIME_TYPE_FSL
#define MIME_TYPE_FSL    \
    {".FSL",CONTENT_TEXT}
#endif

#ifndef MIME_TYPE_JPG
#define MIME_TYPE_JPG    \
    {".JPG",CONTENT_JPG}
#endif

#ifndef MIME_TYPE_JPEG
#define MIME_TYPE_JPEG    \
    {".JPEG",CONTENT_JPG}
#endif

#ifndef MIME_TYPE_GIF
#define MIME_TYPE_GIF    \
    {".GIF",CONTENT_GIF}
#endif

#ifndef MIME_TYPE_BMP
#define MIME_TYPE_BMP    \
    {".BMP",CONTENT_BMP}
#endif  
		

#define MIME_MAX_TYPES       		sizeof(MIME_TYPE_ARRAY)/sizeof(MIME_TYPE )

/**
 * Receives tcp/udp information copying to a static
 *  array or use network buffer directly depending on flag var
 *  Info is received thru tcp/udp/raw connection descriptor
 *  Features: reentrant
 *
 * @param connection descriptor
 * @param static array to be used to copy network buffers
 * @param selector from apps array or use directly from lwIP network buffers
 * @param network buffer pointer of pointer
 * @return length of buffer. Read conn->err for details: 
 *    OK, (ERR_OK) CLSD (ERR_CLSD), TIMEOUT (ERR_TIMEOUT), OUT OF MEM (ERR_MEM)
 */
uint16_t netconn_rcv_req(void *connec, char *alloc_rq, void **nbuffer, uint8_t flag);

/**
 * Set timeout for selected connection. Call it after netconn_listen(...)
 *   for client or netconn_new(...) for server
 *
 * @param connection descriptor
 * @param wait time for connection until is time out'ed
 * @return none
 */
 void uitoa(uint32_t n, char s[]);


#endif
