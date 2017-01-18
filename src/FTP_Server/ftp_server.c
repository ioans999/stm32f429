

/* ------------------------ STM32F4xx lib----------------------------- */
#include "stm32f4xx.h"

/* ------------------------ FreeRTOS includes ----------------------------- */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"                                                             
#include "queue.h"

/* ------------------------ lwIP includes --------------------------------- */
#include "api.h"
#include "lwip/mem.h"

/* ------------------------ Project includes ------------------------------ */
//#include "SD.h"         /* SD Card Driver (SPI mode) */
#include "ff.h"
#include "diskio.h"
#include "sdio_sd.h"

/* ------------------------ Project includes ------------------------------ */
#include "ftp_server.h"
#include "utilities.h"
#include "sys_config.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>


#define FTP_USERNAME          ""
#define FTP_PASSWORD          ""

 
typedef union
{
	uint8_t  bytes[4];
	uint32_t lword;		
}T32_8;

typedef union
{
	uint8_t  u8[2];
	uint16_t u16;		
}T16_8;


extern char system_login[16];
extern char system_password[16];
extern xQueueHandle xQueueFTPr;
extern xQueueHandle xQueueFTPs;

static char *ftp_path;
static unsigned char *buffer_ftp;


/********************Private Functions ***************************************/
uint8_t
FTPTestWriteDisk(void);
/**
 * FTP Server Main Control Socket Parser: requests and responses

 */
static uint8_t
vFTPConnection(struct netconn *connfd, char *alloc_rq)
{
	PasvData SendCmd;
	PasvData RecCmd;
	portBASE_TYPE xStatus;
	uint8_t stat_data = 0;
	struct netconn *conn_data;
  T32_8 ip_address;
  T16_8 ftp_port;    
  CHAR *end;
  CHAR i;
	
  uint8_t correct_login = FALSE;

	stat_data = 0;
  netconn_write(connfd,FTP_WELCOME_RESPONSE,str_len(FTP_WELCOME_RESPONSE),NULL);
  
  do
  {
    /*if reception is OK: wait for REQUEST from client*/
    netconn_rcv_req((void *)connfd, alloc_rq, NULL,NULL);
    if(connfd->last_err != ERR_OK)
    {
      break;
    }
    /*authentication required*/           
    if( correct_login == TRUE)
    {
       if( strstr(alloc_rq,FTP_DELE_REQUEST) != NULL )
       {
          if ( 0 == 0) //Delete file
          {
             netconn_write(connfd,FTP_DELE_OK_RESPONSE,str_len(FTP_DELE_OK_RESPONSE),NULL);            
          }
          else
          {
             netconn_write(connfd,FTP_WRITE_FAIL_RESPONSE,str_len(FTP_WRITE_FAIL_RESPONSE),NULL);             
          }
       }
			 else if( strstr(alloc_rq,"PWD") != NULL )
       {
          FTP_PWD(connfd);
			 }
			 else if( (strstr(alloc_rq,"syst") || strstr(alloc_rq,"SYST")) != NULL )
       {
				  netconn_write(connfd,"215 UNIX Type: L8\r\n",str_len("215 UNIX Type: L8\r\n"),NULL);
			 }
			 else if( strstr(alloc_rq,"opts utf8 on") != NULL )
       {
				  netconn_write(connfd,"200 OK, UTF-8 enabled\r\n",str_len("200 OK, UTF-8 enabled\r\n"),NULL);
			 }
       else if( strstr(alloc_rq,"CWD") != NULL )
       {
				  FTP_CWD(connfd,&alloc_rq[sizeof("CWD")]);
			 }
			 else if( strstr(alloc_rq,"TYPE A") != NULL )
       {
				  netconn_write(connfd,"200 TYPE is now ASCII\r\n",str_len("200 TYPE is now ASCII\r\n"),NULL);
			 }
			 else if( strstr(alloc_rq,"TYPE I") != NULL )
       {
				  netconn_write(connfd,"200 TYPE is now binary\r\n",str_len("200 TYPE is now binary\r\n"),NULL);
			 }
			 else if( strstr(alloc_rq,"NOOP") != NULL || strstr(alloc_rq,"noop") != NULL)
       {
				  netconn_write(connfd,"200 OK\r\n",str_len("200 OK\r\n"),NULL);
			 }
			 else if( strstr(alloc_rq,"REBOOT") != NULL || strstr(alloc_rq,"reboot") != NULL)
       {
				  netconn_write(connfd,"200 REBOOT OK\r\n",str_len("200 REBOOT OK\r\n"),NULL);
			    NVIC_SystemReset();
			 }
			 else if( strstr(alloc_rq,FTP_PORT_REQUEST) != NULL )
			 {
				  stat_data = 1;
				  //Get IP: four parts
				  alloc_rq = &alloc_rq[sizeof(FTP_PORT_REQUEST)];
          for(i=0;i<sizeof(ip_address);i++)
          {
             end = (CHAR *)strchr((const CHAR *)alloc_rq,',');        
             ip_address.bytes[i] = (uint8_t ) strtoul(alloc_rq,(char **)&end,10);
             alloc_rq = (char *)end + 1;      
          }
          //Get FTP Port:first part
          end = (CHAR *)strchr((const CHAR *)alloc_rq,',');        
          ftp_port.u8[1] = (uint8_t)strtoul(alloc_rq,(char **)&end,10);
          alloc_rq = (char *)end + 1;
          //Get FTP Port:second part
          end = (char *)strchr((const char *)alloc_rq,'\r');        
          ftp_port.u8[0] = (uint8_t)strtoul(alloc_rq,(char **)&end,10); 
   
          netconn_write(connfd,FTP_PORT_OK_RESPONSE,str_len(FTP_PORT_OK_RESPONSE),NULL);
			 }
			 else if( strstr(alloc_rq,"LIST") != NULL )
       {
				  if (stat_data ==1)  //PORT mode
				  {
						 if( (conn_data = FTP_OpenDataPort((struct ip_addr *)&(ip_address.lword), ftp_port.u16)) != NULL )
             {
                netconn_write(connfd,FTP_LIST_OK_RESPONSE,str_len(FTP_LIST_OK_RESPONSE),NULL);        
                if (FTP_Read_List_Of_Files(conn_data) != 0)
								{
									 netconn_write(connfd,FTP_LIST_FAIL_RESPONSE,str_len(FTP_LIST_FAIL_RESPONSE),NULL);
								}	
								else
								{
									 netconn_write(connfd,FTP_TRANSFER_OK_RESPONSE,str_len(FTP_TRANSFER_OK_RESPONSE),NULL);
								}
                FTP_CloseDataPort(conn_data);                        
             }
             else
             {
                netconn_write(connfd,FTP_DATA_PORT_FAILED,str_len(FTP_DATA_PORT_FAILED),NULL);        
     
             }						 
				  }
          stat_data = 0;					
			 }
       else if( strstr(alloc_rq,FTP_STOR_REQUEST) != NULL )
       {
				  if (stat_data ==1)  //PORT mode
				  {
             if( (conn_data = FTP_OpenDataPort((struct ip_addr *)&(ip_address.lword),ftp_port.u16)) != NULL )
             {
                netconn_write(connfd,FTP_STOR_OK_RESPONSE,str_len(FTP_STOR_OK_RESPONSE),NULL);          
                if (FTP_Write_File(conn_data,&alloc_rq[sizeof(FTP_STOR_REQUEST)]) != 0)
								{
								   netconn_write(connfd,FTP_WRITE_FAIL_RESPONSE,str_len(FTP_WRITE_FAIL_RESPONSE),NULL);   
								}									
                FTP_CloseDataPort(conn_data);         
                netconn_write(connfd,FTP_TRANSFER_OK_RESPONSE,str_len(FTP_TRANSFER_OK_RESPONSE),NULL);
             }
             else
             {
               netconn_write(connfd,FTP_DATA_PORT_FAILED,str_len(FTP_DATA_PORT_FAILED),NULL); 
             }  
					}
					stat_data = 0;	
       }
			 else if( strstr(alloc_rq,FTP_RETR_REQUEST) != NULL )
       {
				  if (stat_data ==1)  //PORT mode
				  {
             if( (conn_data = FTP_OpenDataPort((struct ip_addr *)&(ip_address.lword),ftp_port.u16)) != NULL )
             {
                netconn_write(connfd,FTP_RETR_OK_RESPONSE,str_len(FTP_RETR_OK_RESPONSE),NULL);          
                if (FTP_Read_File(conn_data, &alloc_rq[sizeof(FTP_RETR_REQUEST)]) != 0 )
								{
								   netconn_write(connfd,FTP_WRITE_FAIL_RESPONSE,str_len(FTP_WRITE_FAIL_RESPONSE),NULL);   
								}		
                else
								{
									 netconn_write(connfd,FTP_TRANSFER_OK_RESPONSE,str_len(FTP_TRANSFER_OK_RESPONSE),NULL);
								}									
                FTP_CloseDataPort(conn_data);                          
             }
             else
             {
               netconn_write(connfd,FTP_DATA_PORT_FAILED,str_len(FTP_DATA_PORT_FAILED),NULL); 
             }  
					}						
					stat_data = 0;	
       }			 
       else 
       {
          if( FTP_QUIT_OR_WRONG_REQUEST(connfd,alloc_rq) )
          {
             break;/*QUIT command*/
          }
       }
    }
    else//not logged in
    {
       if( strstr(alloc_rq,FTP_USER_REQUEST) != NULL )
       {          
          /*authentication process: username matchs exactly?*/
          if( !strncmp(&alloc_rq[sizeof(FTP_USER_REQUEST)],FTP_USERNAME,str_len(FTP_USERNAME)) )
          {
              netconn_write(connfd,FTP_USER_RESPONSE,str_len(FTP_USER_RESPONSE),NULL);            
              netconn_rcv_req((void *)connfd, alloc_rq, NULL, NULL);
              if(connfd->last_err != ERR_OK)
              {
                  break;
              }             
              if( strstr(alloc_rq,FTP_PASS_REQUEST) != NULL )
              {
                  if( !strncmp(&alloc_rq[sizeof(FTP_PASS_REQUEST)],FTP_PASSWORD,str_len(FTP_PASSWORD)) )
                  {
                    netconn_write(connfd,FTP_PASS_OK_RESPONSE,str_len(FTP_PASS_OK_RESPONSE),NULL);                    
                    correct_login = TRUE;
                  }
                  else  {
                    netconn_write(connfd,FTP_PASS_FAIL_RESPONSE,str_len(FTP_PASS_FAIL_RESPONSE),NULL);
                  }
              }
              else {
                  netconn_write(connfd,FTP_BAD_SEQUENCE_RESPONSE,str_len(FTP_BAD_SEQUENCE_RESPONSE),NULL);
              }
          }
          else {
              netconn_write(connfd,FTP_PASS_FAIL_RESPONSE,str_len(FTP_PASS_FAIL_RESPONSE),NULL);
          }
       }
       else
       {
          if( FTP_QUIT_OR_WRONG_REQUEST(connfd,alloc_rq) )
          {
             break;
          }
       }
    }
  }while(1);      

  netconn_close(connfd);
  netconn_delete(connfd);

  return 1;/*default close value*/
}







/*****************************************************
 * Closes or Leave session depending on client request
 
 */
uint8_t
FTP_QUIT_OR_WRONG_REQUEST(struct netconn *connfd, char *alloc_rq)
{
   if( strstr(alloc_rq,FTP_QUIT_REQUEST) != NULL )
   {
      //****RESPONSE CLOSING SESSION: BYE
      netconn_write(connfd,FTP_QUIT_RESPONSE,str_len(FTP_QUIT_RESPONSE),NULL); 
      return 1;/*close session*/
   }
	 netconn_write(connfd,FTP_UNKNOWN_RESPONSE,str_len(FTP_UNKNOWN_RESPONSE),NULL);
	 return 0;
}

/**************************************************
 * Open data socket: ftp server connects to client.
 */
struct netconn *
FTP_OpenDataPort(struct ip_addr *add, uint16_t port)
{
  struct netconn *conn_data;
	
  if( (conn_data = netconn_new(NETCONN_TCP)) == NULL )
  {
     return NULL;/*error*/
  }
  if( netconn_connect(conn_data,add,port) != ERR_OK )
  {
     return NULL;/*error*/
  }    
  //netconn_set_timeout((void *)conn_data,4000/*timeout*/);  
	netconn_set_recvtimeout (conn_data, FTP_TCP_TIMEOUT_DATA_PORT );
  return conn_data;  
}

/****************************************
 * Close data socket
 */
static uint8_t
FTP_CloseDataPort(struct netconn *conn_data)
{
  /*delete TCP connection*/
  netconn_close(conn_data);
  netconn_delete(conn_data);  
  
  return 0;
}


/****************************************
Returns the current directory
 */
void
FTP_PWD(struct netconn *connfd)
{
	buffer_ftp[0] = 0;
	strncat(	buffer_ftp, "257 \"",strlen("257 \""));
	strncat(	buffer_ftp, ftp_path,strlen(ftp_path));
	strncat(	buffer_ftp, "\"\r\n",strlen("\"\r\n"));
	netconn_write(connfd, buffer_ftp, strlen(buffer_ftp), NULL);
}

/****************************************
 Changes the current directory 
 */
void
FTP_CWD(struct netconn *connfd, char *alloc_rq)
{
	uint32_t i;
	
	if ((strncmp(alloc_rq,"../",strlen("../")) == 0) && (strstr(ftp_path,"/") != NULL) && strlen(ftp_path) == 1  )  		
    netconn_write(connfd,"250 OK. Current directory is /\r\n",strlen("250 OK. Current directory is /\r\n"),NULL);
	else if ((strncmp(alloc_rq,"../",strlen("../")) == 0) || (strncmp(alloc_rq,"..",strlen("..")) == 0) \
	|| (strncmp(alloc_rq,"/../",strlen("/../")) == 0))
	{
		 for (i=0;i<10; i++)
		 {
			  if (ftp_path[strlen(ftp_path) -1] == '/' )
				{
					 if (strlen(ftp_path) > 1)
					    ftp_path[strlen(ftp_path) -1] = '\0';
           break;
				}
				else ftp_path[strlen(ftp_path) -1] = '\0';
		 }
		 netconn_write(connfd,"250 OK.\r\n",strlen("250 OK.\r\n"),NULL);
	}
	else if (strlen(alloc_rq) > 0)
	{
		if (strncmp(alloc_rq,"/",strlen("/")) == 0 )
		{
	     ftp_path[0] = 0;
		}
		else if (strlen(ftp_path) > 1 && strlen(alloc_rq) > 1)
		{
			 strncat(	ftp_path, "/",strlen("/"));
		}
		
		strncat(	ftp_path, alloc_rq,strlen(alloc_rq));
		if (ftp_path[strlen(ftp_path) - 1] == '\n')
  	{
		   ftp_path[strlen(ftp_path) - 1] = '\0';			
	  }
		if (ftp_path[strlen(ftp_path) - 1] == '\r')
		{
			 ftp_path[strlen(ftp_path) - 1] = '\0';
		}
		if (ftp_path[strlen(ftp_path) - 1] == '/' && strlen(ftp_path) > 1)
		{
			 ftp_path[strlen(ftp_path) - 1] = '\0';
		}
	  netconn_write(connfd,"250 OK.\r\n",strlen("250 OK.\r\n"),NULL);
	} 
}




/**********************************************
Send list of files in the current directory
 */
uint8_t
FTP_Read_List_Of_Files(struct netconn *connfd)
{
	FATFS   fs; 
	DIR     dir;
	FILINFO fi;
	FRESULT res;
	char  buff[12];
	char  *atr_f;
  char  *alloc_send;
	uint8_t  i;
	uint8_t  n;
	
	if( (alloc_send=(char *)pvPortMalloc(64)) == NULL )
  {    
     vPortFree(alloc_send);
     return 1;
  }  
	*alloc_send = 0;
	
	res = f_mount(0, &fs);
		if ( res != FR_OK ) 	{
		return 1;			
	}
	res = f_opendir(&dir, ftp_path);
	if ( res != FR_OK )  	{
		return 1;			
	}
  for ( i=0; i < 255; i++) 
  {
		res = f_readdir(&dir, &fi);
		if (res != FR_OK || fi.fname[0] == 0 )
		{
			f_mount(0, NULL);
			break; 
		}
		if (fi.fattrib & AM_DIR) 
		{
  		strcat(alloc_send,"drw-rw-rw-  1 ftp  ftp  ");
		}
		else 
		{
  		strcat(alloc_send,"-rw-rw-rw-  1 ftp  ftp  ");
		}
		//file size attribute 
		atr_f = buff;
		uitoa((uint32_t ) fi.fsize, buff);
		for (n=0; n<=(10 -strlen(atr_f));n++)
		{
			strcat(alloc_send," ");
		}
		strncat(alloc_send,atr_f,strlen(atr_f));
		
		//attribute month file date
		strcat(alloc_send," ");
		//strcat(alloc_send,(month_array[((fi.fdate >> 5) & 15) -1]));
		
		//attribute day file date
		atr_f = buff;
		uitoa((uint32_t ) (fi.fdate & 31), buff);
		if ((fi.fdate & 31) < 10) strcat(alloc_send," 0");
		else strcat(alloc_send," ");
		strncat(alloc_send, atr_f, strlen(atr_f));
		
		//attribute hour file date		 
		atr_f = buff;
		uitoa((uint32_t ) ((fi.ftime >> 11) & 31), buff);
		if (((fi.ftime>> 11) & 31) < 10) strcat(alloc_send," 0");
		else strcat(alloc_send," ");
		strncat(alloc_send, atr_f, strlen(atr_f));
		
		//attribute minute file date
		atr_f = buff;
		uitoa((uint32_t ) ((fi.ftime >> 5) & 63), buff);
		if (((fi.ftime >> 5) & 63) < 10) strcat(alloc_send,":0");
		else strcat(alloc_send,":");
		strncat(alloc_send, atr_f, strlen(atr_f));		
		
		//file name
		strcat(alloc_send," ");
		strncat(alloc_send, fi.fname, strlen(fi.fname));
  	strcat(alloc_send,"\r\n");
		
		if (strlen(alloc_send) > (FTP_LIST_BUFF_SEND - 52))
		{
			netconn_write(connfd, alloc_send, strlen(alloc_send), NETCONN_COPY);
			alloc_send[0] = 0;
		}
  } 
	netconn_write(connfd, alloc_send, strlen(alloc_send), NETCONN_COPY);
  f_mount(0, NULL);
	vPortFree(alloc_send);
	return 0;/*OK*/
}



/******************************************
 Read File
 */
uint8_t
FTP_Read_File(struct netconn *connfd,  char *alloc_rq)
{
	FATFS   fs; 
	FIL     file;
	UINT    len;
	FRESULT res;
  UINT    len_send;	
	
	
  buffer_ftp[0] = 0;
	res = f_mount(0, &fs);
	if ( res != FR_OK ) 	{	
		return 1;	   	
	}
	strncat(buffer_ftp, ftp_path, strlen(ftp_path));
	strncat(buffer_ftp, "/", strlen("/"));
	strncat(buffer_ftp, alloc_rq, strlen(alloc_rq));
  res = f_open(&file, buffer_ftp,  FA_READ);
	if (res != FR_OK) {	
		f_mount(0, NULL);
		return 1;
  }
	buffer_ftp[0] = 0;
	while(1){
	  res = f_read(&file, buffer_ftp,1300, &len);
		if (res != FR_OK) {
			 f_close(&file);
		   f_mount(0, NULL);
			 return 1;
    }
		if (len > 0)
			 netconn_write_partly(connfd, buffer_ftp, len, NETCONN_COPY, &len_send);
		if  (len < 1300)
		{
			 f_close(&file);
		   f_mount(0, NULL);
			 break;
		}
  }	
	
	return 0;
}


/*******************************************
Overwrites the file
 */
uint8_t
FTP_Write_File(struct netconn *connfd, char *alloc_rq)
{
	FATFS   fs; 
	FIL  file;
	FRESULT res;
	struct netbuf *inbuf;
  struct pbuf *q;
	UINT lenr;

	
	buffer_ftp[0] = 0;
	res = f_mount(0, &fs);
	if ( res != FR_OK ) 	{
		return 1;	   	
	}
	strncat(buffer_ftp, ftp_path, strlen(ftp_path));
	if (strlen(ftp_path) > 1 ) {
	    strncat(buffer_ftp, "/", strlen("/"));
	}
	strncat(buffer_ftp, alloc_rq, strlen(alloc_rq));
	
  res = f_open(&file, buffer_ftp, FA_CREATE_ALWAYS | FA_WRITE);
	if (res != FR_OK) {
		f_mount(0, NULL);
		return 1;
  }
	do
  {   
     netconn_rcv_req((void *)connfd, NULL, (void **)&inbuf, NETCONN_RCV_NETBUFFER);
     if( connfd->last_err == ERR_OK )
     {
        q = inbuf->ptr;        
        do
        {    				
           res = f_write(&file, q->payload, q->len, &lenr);			
           if (res != FR_OK ) {
	      		  f_close(&file);
		          f_mount(0, NULL);
						  return 1;
					 }
        } while(( q = q->next ) != NULL );
				f_sync(&file);
        netbuf_delete(inbuf);
     }
     else {
			  f_close(&file);
	      f_mount(0, NULL);
        break;
     }       
  }while(1);
	vTaskDelay(1000);
	return 0;
}







/*********************************************************************
 * Start an embedded FTP server Task: 1 client and 1 file per transfer
 */
void
vBasicFTPServer( void *pvParameters )
{   
    struct netconn *conn, *connection;
    char *alloc_rq;

    uint8_t i = FTP_CLOSED;
    ( void )pvParameters;
		
	  if( (buffer_ftp =(unsigned char *)pvPortMalloc(1600)) == NULL )
    {    
      vPortFree(buffer_ftp);
      vTaskDelete( NULL );      
    }   
	  if( (ftp_path =(char *)pvPortMalloc(40)) == NULL )
    {    
      vPortFree(ftp_path);
      vTaskDelete( NULL );      
    }    
    if( (alloc_rq=(char *)pvPortMalloc( FTP_REQUEST_SPACE )) == NULL )
    {    
      vPortFree(alloc_rq);
      vTaskDelete( NULL );      
    }    
    
    /* Create a new TCP connection handle. */
    conn = netconn_new(NETCONN_TCP);
    netconn_bind(conn, NULL, FTP_TCP_CONTROL_PORT); 
    netconn_listen(conn);
		
		
    for(;;)
    {
      if( i == FTP_CLOSED )/*FTP_CLOSE*/
      {
         if(  netconn_accept(conn, &connection) == ERR_OK )
         {
            i = FTP_OPEN;
					  /*set timeout for this connection*/
					  netconn_set_recvtimeout ( connection, FTP_TCP_TIMEOUT_REC_PORT );
                         
         }
      }
      else/*FTP_OPEN*/
      {
				 //start PATH is "/"
				 ftp_path[0] = 0 ;
				 strncat(ftp_path,"/", str_len("/"));
				
         if( vFTPConnection( connection, alloc_rq ) )
         {
            i = FTP_CLOSED;

         }
      }
    }

}


