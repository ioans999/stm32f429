
#include "stm32f4xx.h"

/* ------------------------ FreeRTOS includes ----------------------------- */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* ------------------------ lwIP includes --------------------------------- */
#include "api.h"
#include "lwip/mem.h"

#include "utilities.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/*
** Translation Table as described in RFC1113
*/
static const 
char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const 
MIME_TYPE MIME_TYPE_ARRAY[]  = 
{
  MIME_TYPE_HTM,
  MIME_TYPE_SHTML,
  MIME_TYPE_CSS,
  MIME_TYPE_TXT,
  MIME_TYPE_FSL,
  MIME_TYPE_JPG,
  MIME_TYPE_JPEG,
  MIME_TYPE_GIF,
  MIME_TYPE_BMP
};

/*************************************************
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
uint16_t 
netconn_rcv_req(void *connec, char *alloc_rq, void **nbuffer, uint8_t flag)
{ 
  /*joining temp pbuf*/
  struct netbuf *inbuf;
  struct pbuf *q;
  
  struct netconn *conn = (struct netconn *)connec;

  
  /*temporal len*/
  uint16_t len = 0;

  /*FSL: receive the packet*/
  //inbuf = netconn_recv(conn,inbuf);
  
  /*receiving from the buffer*/
  if( netconn_recv(conn, &inbuf) == ERR_OK )
  {
    /*if receiver is expecting a big rx packet, use it directly from the network buffers*/
    if(flag)
    {
      /*use buffer directly from lwIP network buffers*/
      len = inbuf->ptr->tot_len;
      *nbuffer = (void *)inbuf;
      return len;     
    }
    
    /*if not you can copy it to a small buffer*/    
    /*start segment index*/
    q = inbuf->ptr;
    do
    {
        memcpy( &alloc_rq[len], q->payload, q->len );
        len += q->len;
    }
    while( ( q = q->next ) != NULL );            
		
    /*NULL char terminator. Useful for ASCII transfers*/
    alloc_rq[len] = '\0';
    /*free pbuf memory*/
    netbuf_delete(inbuf);
  }  
  return len;/*return value*/
}



/***************************
 *  uint32_t to string
 */
 void uitoa(uint32_t n, char s[])
 {
     uint32_t i, j, sign;
     char c;
	 
     i = 0;
     do {      
         s[i++] = n % 10 + '0';   
     } while ((n /= 10) > 0);    
     s[i] = '\0';
		 for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
         c = s[i];
         s[i] = s[j];
         s[j] = c;
     }
 }
 
 
 
 
 
void read_cmd(char* str, char* w1,char* w2,char* w3)
{ 
	uint8_t *a = w1, *b = w2,*c = w3; 
	uint8_t i = 0;
	uint8_t k;
	
	for (k = 0 ; k < 100; k++) {
		if ( ((str[k] == '\r')&&(str[k+1] == '\n')) || (str[k] == '/' && str[k+1] == '/')) 
		{	
			if (i == 0 || i == 1 )
				*a++ = '\0';	
			if (i == 2 || i == 3 )
				*b++ = '\0';	
			if (i == 4 || i == 5 )
        *c++ = '\0';			
			break;
		}
		if ((str[k] > 0x20) && (str[k] < 0x7B) ) {
			if (i == 0 || i == 1 ) {
			  	i = 1;
	        *a++ = str[k];
			}
		  if (i == 2 || i == 3 ) {
	  			i = 3;
	        *b++ = str[k];
			}
			if (i == 4 || i == 5 ) {
		  		i = 5;
	        *c++ = str[k];
			}			
	  }	
		else
		{
			if (i == 1) {
		  		i = 2;
			  	*a++ = '\0';;
			}
			if (i == 3) {
			  	i = 4;
			  	*b++ = '\0';;
			}
			if (i == 5) {
			  	i = 6;
			  	*c++ = '\0';
			}
		}
	}
}






uint8_t read_ip4_addr(char* str,uint8_t *ip)
{
  char a[4];
  volatile char *p;
	volatile uint8_t h;
	volatile uint8_t i=0,n=0,l=0;
	
  p=a;
	for (i = 0; i <=15; i++) {
		if ((str[i] != '.') && (str[i] != '\0') && (l<4)) {
		  a[l] = str[i];
			a[l+1]='\0';
			++l;
	  }	
		else {
			n++;
			if (n>4) break;
			l=0;
			h=atoi(a);
			ip[n-1]=h;		
		}
	}
	return 0;
}




uint8_t read_mac_addr(char* str,uint8_t *MAC_addr)
{
	char  mac_str[4];
	char *p; 
	
	if ((str[2] == '-') && (str[5] == '-') && (str[8] == '-') && (str[11] == '-') && (str[14] == '-'))
		{
				p=mac_str;
			  *p = '0';
			  *++p = 'x';
			  *++p = str[0];
			  *++p = str[1];
		    *MAC_addr++ = strtol(mac_str,(char **) NULL,16);
				p=mac_str + 2;
			  *p = str[3];
			  *++p = str[4];
				*MAC_addr++ = strtol(mac_str,(char **) NULL,16);
				p=mac_str + 2;
			  *p = str[6];
			  *++p = str[7];
				*MAC_addr++ = strtol(mac_str,(char **) NULL,16);
				p=mac_str+2;
			  *p = str[9];
			  *++p = str[10];
				*MAC_addr++ = strtol(mac_str,(char **) NULL,16);
				p=mac_str+2;
			  *p = str[12];
			  *++p = str[13];
				*MAC_addr++ = strtol(mac_str,(char **) NULL,16);
				p=mac_str+2;
			  *p = str[15];
			  *++p = str[16];
				*MAC_addr++ = strtol(mac_str,(char **) NULL,16);
	  }	else return 1;
}




 
 


