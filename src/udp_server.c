#include "udp_server.h"

/* ------------------------ STM32F4xx lib----------------------------- */
#include "stm32f4xx.h"

/* ------------------------ LwIP includes ----------------------------- */
#include "lwip/udp.h"
#include "lwip/netbuf.h"
#include "lwip/pbuf.h"
#include "lwip/api.h"
/* ------------------------ FreeRTOS includes ----------------------------- */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"                                                             
#include "queue.h"

/* ------------------------ Project includes ------------------------------ */
#include "main.h"
#include "algblock.h"
#include "utilities.h"
#include "sys_config.h"
#include <string.h>

#define MAX_CONNECT_UDP_OUT 2

void Write_data_in_mask_DO(uint8_t *v, uint8_t *m);
void Write_data_in_mask_AO(uint8_t *v, uint8_t *m);
void Write_data_in_mask_RP(uint8_t *v, uint8_t *m, uint8_t t);
void Write_data_in_mask_IP(uint8_t *v, uint8_t *m);

extern volatile float alg_bd_fl;

struct 
{
	struct ip_addr  DestIPaddr[MAX_CONNECT_UDP_OUT];
	char Dest_ip_mask[MAX_CONNECT_UDP_OUT];
	uint32_t last_rec[MAX_CONNECT_UDP_OUT];
} RemoteClient;

struct netconn *conn;
struct netbuf *buf;

uint16_t dest_port_out = 11001;
struct udp_pcb* upcb_send;


volatile xSemaphoreHandle xBinarySemaphoreUDPServDestIP;




void vUDP_serv_recv(void * pvParameters)
{
	err_t err;
	uint8_t i,n,ip_ex;
	uint32_t  *p1,*p2;
	uint8_t *buffer_data;
	u16_t len;
	
  xBinarySemaphoreUDPServDestIP = xSemaphoreCreateRecursiveMutex();
	
	if((buffer_data =(unsigned char *)pvPortMalloc(1500)) == NULL )
  {    
      vPortFree(buffer_data);
      vTaskDelete( NULL );      
  }   

	upcb_send = udp_new();
  conn = netconn_new(NETCONN_UDP);
	if (conn!= NULL)
  {
		err = netconn_bind(conn, IP_ADDR_ANY, 11000);
		if (err == ERR_OK)
    {
			while(1)
			{
				err = netconn_recv(conn, &buf);
				if (err == ERR_OK)
				{
			  	if (buf!= NULL)
          {					
						ip_ex=0;
						p1=(uint32_t *) netbuf_fromaddr(buf);
						for (i=0; i<MAX_CONNECT_UDP_OUT; i++){
							p2=(uint32_t *)&RemoteClient.DestIPaddr[i];
							if ((RemoteClient.Dest_ip_mask[i]==1) && (*p2 == *p1)) {
								ip_ex=1;
								RemoteClient.last_rec[i]=0;
						  }
						}
						if (ip_ex==0)
						{
						  for (i=0; i<MAX_CONNECT_UDP_OUT; i++)
							{
						  	if (RemoteClient.Dest_ip_mask[i]==0){
									xSemaphoreTakeRecursive(xBinarySemaphoreUDPServDestIP, portMAX_DELAY);
								  RemoteClient.DestIPaddr[i]= *netbuf_fromaddr(buf);
								  RemoteClient.Dest_ip_mask[i]=1;
									RemoteClient.last_rec[i]=0;
									xSemaphoreGiveRecursive(xBinarySemaphoreUDPServDestIP);	
									ip_ex = 1;
								  break;
							  }
							}
					  }
						if ( ip_ex == 1)
						{
							netbuf_data(buf, &buffer_data, &len);
							
							if (buffer_data[4] == 1)
							{
								Write_data_in_mask_DO(buffer_data+8, buffer_data+8+16);
								Write_data_in_mask_AO(buffer_data+8+32, buffer_data+8+32+256);
								Write_data_in_mask_RP(buffer_data+8+304, buffer_data+8+304+1024, 1);
							}
							if (buffer_data[4] == 2)
							{
								Write_data_in_mask_RP(buffer_data+8, buffer_data+8+1024, 2);
								Write_data_in_mask_IP(buffer_data+8+1056, buffer_data+8+1056+64);
							}
							netconn_send(conn,buf);
						}
            netbuf_delete(buf);
					}
        }
				vTaskDelay(0);
			}
			udp_remove(upcb_send);
		}
	}
}






uint32_t UDP_send(uint8_t *bf, uint16_t ln)
{
  uint8_t i=0;
	static volatile uint8_t errt=0;
  err_t	err;
	struct pbuf* p;		
	struct ip_addr  ip;

	xSemaphoreTakeRecursive(xBinarySemaphoreUDPServDestIP, portMAX_DELAY);
	for (i=0;i<MAX_CONNECT_UDP_OUT;i++)
	{
		if (RemoteClient.last_rec[i] > 300){
			RemoteClient.last_rec[i]=0;
			RemoteClient.Dest_ip_mask[i]=0;
		}
		RemoteClient.last_rec[i]++;
	  if (RemoteClient.Dest_ip_mask[i]!=0)
	  {
	    if ((p = pbuf_alloc(PBUF_TRANSPORT,ln, PBUF_RAM)) == NULL)
			{
				errt++;
				return errt;
			}
	    memcpy(p->payload, bf,  ln);	
			if(udp_sendto(upcb_send, p, &RemoteClient.DestIPaddr[i], dest_port_out) != ERR_OK)
			{
				errt++;
				vTaskDelay(1);
			}
			pbuf_free(p);
	  }
	}
	xSemaphoreGiveRecursive(xBinarySemaphoreUDPServDestIP);
	return errt;
}


void SetDestPort(uint16_t p)
{ 
	dest_port_out = p;
}





void Write_data_in_mask_DO(uint8_t *v, uint8_t *m)
{
	uint32_t i;
	
	for(i=0; i<128; i++) 
	{
		if(((m[i/8] >> i%8) & 1) == 1)
		{
			write_mc_value_to_buf(v, i, i, 1, 3);
		}
	}
}
void Write_data_in_mask_AO(uint8_t *v, uint8_t *m)
{
	uint32_t i;
	
	for(i=0; i<128; i++) 
	{
		if(((m[i/8] >> i%8) & 1) == 1)
		{
			write_mc_value_to_buf(v, i, i, 1, 4);
		}
	}
}

void Write_data_in_mask_RP(uint8_t *v, uint8_t *m, uint8_t t)
{
	uint32_t i;
	
	for(i=0; i<256; i++) 
	{
		if(((m[i/8] >> i%8) & 1) == 1)
		{
			if (t==1)
				write_mc_value_to_buf(v, i, i, 1, 5);
			if (t==2)
				write_mc_value_to_buf(v, i + 256, i, 1, 5);
		}
	}
}
void Write_data_in_mask_IP(uint8_t *v, uint8_t *m)
{
	uint32_t i;
	
	for(i=0; i<512; i++) 
	{
		if(((m[i/8] >> i%8) & 1) == 1)
		{
			write_mc_value_to_buf(v, i, i, 1, 6);
		}
	}
}
