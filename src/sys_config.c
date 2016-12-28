#include "sys_config.h"


#include "ff.h"
#include "ethernetif.h"
#include "netconf.h"
#include "stm32f4x7_eth_bsp.h"

#include "utilities.h"
#include "main.h"
#include "usart.h"
#include "algblock.h"
#include "udp_server.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
	uint8_t	IP4_ADDR_cnf[4] ;
	uint8_t	MAC_addrs_cnf[6]; 
  char login[10];
	char password[10];
	uint32_t rs485_speed_1;
	uint32_t rs485_speed_2;
	uint32_t rs485_speed_3;
	uint32_t rs485_speed_4;
	uint16_t dst_port;
	struct {
		uint8_t mc[NUBER_PCB_IN_OUT];
		uint8_t addr[NUBER_PCB_IN_OUT];
		uint8_t valpoint[NUBER_PCB_IN_OUT];
		uint32_t speed;
	} rs485_1;
	struct {
		uint8_t mc[NUBER_PCB_IN_OUT];
		uint8_t addr[NUBER_PCB_IN_OUT];
	  uint8_t valpoint[NUBER_PCB_IN_OUT];
		uint32_t speed;
	} rs485_2;
	struct {
		uint8_t mc[NUBER_PCB_IN_OUT];
		uint8_t addr[NUBER_PCB_IN_OUT];
		uint8_t valpoint[NUBER_PCB_IN_OUT];
	  uint32_t speed;
	} rs485_3;
	struct {
		uint8_t mc[NUBER_PCB_IN_OUT];
		uint8_t addr[NUBER_PCB_IN_OUT];
		uint8_t valpoint[NUBER_PCB_IN_OUT];
		uint32_t speed;
	} rs485_4;
} rc_config_initialization;

void read_cmd(char* str,char* w1,char* w2,char* w3);
uint8_t read_config(rc_config_initialization *rc_cnf);



uint8_t rc_config_start(void) 
{
	uint8_t err,i;
	rc_config_initialization rc_conf;
	
	for (i=0;i<20;i++) 
	{
		rc_conf.rs485_1.mc[i]=0;
		rc_conf.rs485_2.mc[i]=0;
		rc_conf.rs485_3.mc[i]=0;
		rc_conf.rs485_4.mc[i]=0;
	}
	rc_conf.MAC_addrs_cnf[0] = 0x02;
	rc_conf.MAC_addrs_cnf[1] = 0x02;
	rc_conf.MAC_addrs_cnf[2] = 0x02;
	rc_conf.MAC_addrs_cnf[3] = 0x02;
	rc_conf.MAC_addrs_cnf[4] = 0x02;
	rc_conf.MAC_addrs_cnf[5] = 0x02;
	
	rc_conf.IP4_ADDR_cnf[0]=192;
	rc_conf.IP4_ADDR_cnf[1]=168;
	rc_conf.IP4_ADDR_cnf[2]=0;
	rc_conf.IP4_ADDR_cnf[3]=254;
	
	rc_conf.rs485_1.speed=115200;
	rc_conf.rs485_2.speed=115200;
	rc_conf.rs485_3.speed=115200;
	rc_conf.rs485_4.speed=115200;
	
  err=read_config(&rc_conf);
	if (err>0){
		if (err == 1)
		  uart_print("Error file system\r\n", strlen("Error file system\r\n"));
		if (err == 2)
		  uart_print("Error open file rc.cnf\r\n", strlen("Error open file rc.cnf\r\n"));
		if (err == 3)
		  uart_print("Error open file rs485_1.cnf\r\n", strlen("Error open file rs485_1.cnf\r\n"));
		if (err == 4)
		  uart_print("Error open file rs485_2.cnf\r\n", strlen("Error open file rs485_2.cnf\r\n"));
		if (err == 5)
		  uart_print("Error open file rs485_3.cnf\r\n", strlen("Error open file rs485_3.cnf\r\n"));
		if (err == 6)
		  uart_print("Error open file rs485_4.cnf\r\n", strlen("Error open rs485_4.cnf\r\n"));
		if (err == 7)
		  uart_print("Error open file adc_i2f.cnf\r\n", strlen("Error open file adc_i2f.cnf\r\n"));
	}
	
	//set_mac_addr_config(rc_conf.MAC_addrs_cnf);
  //set_ipv4_config(rc_conf.IP4_ADDR_cnf);
	set_rs485_config_v1(rc_conf.rs485_1.mc, rc_conf.rs485_1.addr, rc_conf.rs485_1.valpoint);
	set_rs485_config_v2(rc_conf.rs485_2.mc, rc_conf.rs485_2.addr, rc_conf.rs485_2.valpoint);
	//set_rs485_config_v3(rc_conf.rs485_3.mc, rc_conf.rs485_3.addr, rc_conf.rs485_3.valpoint);
	set_rs485_config_v4(rc_conf.rs485_4.mc, rc_conf.rs485_4.addr, rc_conf.rs485_4.valpoint);

	//ETH_BSP_Config();
  //LwIP_Init();
	//InitUsart_rs485_n1(rc_conf.rs485_1.speed);
	//InitUsart_rs485_n2(rc_conf.rs485_2.speed);
  //InitUsart_rs485_n3(rc_conf.rs485_3.speed);
	//InitUsart_rs485_n4(rc_conf.rs485_4.speed);
	//SetDestPort(rc_conf.dst_port);
	
	return 0;
}













uint8_t read_config(rc_config_initialization *rc_cnf)
{
  static FATFS   fscnf;
  static FIL     filecnf;
  static FRESULT ferr; 
	uint8_t str[60];
	uint32_t n,i; 
  uint8_t w1[20];
	uint8_t w2[20];
	uint8_t w3[20];
 
	ferr = f_mount(0, &fscnf);
  if (ferr != FR_OK) {
		return 1;
	}
  strcpy(w1,"0:/RC.CNF");
	ferr = f_open(&filecnf,w1,  FA_READ);
	if (ferr != FR_OK) {		
		return 2;
	}
	for ( n=0; n < 255; n++) 
	{
		str[0]=0;
		w1[0]='\0';w2[0]='\0';w3[0]='\0';
	  if (f_gets(str, sizeof(str), &filecnf)==0)
		{
		  if ( f_error(&filecnf) != 0) {
			  return 2;
	    }		
			if ( f_eof(&filecnf) != 0) {
			  break;
	    }	
      return 2;			
	  }
		uart_print(str, strlen(str));
	  read_cmd(str,w1,w2,w3);
		if (strncmp(w1,"ifconfig",strlen("ifconfig"))==0)
		{
			if (strncmp(w2,"-ipv4",strlen("-ipv4"))==0)
			{
					read_ip4_addr(w3, rc_cnf->IP4_ADDR_cnf);
			}
		  if (strncmp(w2,"-mac",strlen("-mac"))==0)
			{
					read_mac_addr(w3, rc_cnf->MAC_addrs_cnf);
			}
		}
		if (strncmp(w1,"rs485-1",strlen("rs485-1"))==0)
		{
			 rc_cnf->rs485_1.speed = atoi(w2);
		}
		if (strncmp(w1,"rs485-2",strlen("rs485-2"))==0)
		{
			 rc_cnf->rs485_2.speed = atoi(w2);
		}
		if (strncmp(w1,"rs485-3",strlen("rs485-3"))==0)
		{
			 rc_cnf->rs485_3.speed = atoi(w2);
		}
		if (strncmp(w1,"rs485-4",strlen("rs485-4"))==0)
		{
			 rc_cnf->rs485_4.speed = atoi(w2);
		}
		if (strncmp(w1,"dest_port",strlen("dest_port"))==0)
		{
			 rc_cnf->dst_port = atoi(w2);
		}
	}
	f_close(&filecnf);
	
	
	
	
	strcpy(w1,"0:/rs485_1.cnf");
	ferr = f_open(&filecnf,w1,  FA_READ);
	if (ferr != FR_OK) {		
		return 3;
	}
	i=0;
	for ( n=0; n < 255; n++) 
	{
		str[0]=0;
		w1[0]='\0';w2[0]='\0';w3[0]='\0';
	  if (f_gets(str, sizeof(str), &filecnf)==0)
		{
		  if ( f_error(&filecnf) != 0) {
			  return 3;
	    }		
			if ( f_eof(&filecnf) != 0) {
			  break;
	    }	
      return 3;			
	  }
		read_cmd(str,w1,w2,w3);
		if  (strncmp(w1,"ad16",strlen("ad16"))==0)
		{
			rc_cnf->rs485_1.mc[i]=1;
			rc_cnf->rs485_1.addr[i]=atoi(w2);
			rc_cnf->rs485_1.valpoint[i]=atoi(w3);
			i++;
			if (i> (NUBER_PCB_IN_OUT-1)) break;
		}
		if  (strncmp(w1,"ai200",strlen("ai200"))==0)
		{
			rc_cnf->rs485_1.mc[i]=2;
			rc_cnf->rs485_1.addr[i]=atoi(w2);
			rc_cnf->rs485_1.valpoint[i]=atoi(w3);
			i++;
			if (i> (NUBER_PCB_IN_OUT-1)) break;
		}
		if  (strncmp(w1,"dd16",strlen("dd16"))==0)
		{
			rc_cnf->rs485_1.mc[i]=3;
			rc_cnf->rs485_1.addr[i]=atoi(w2);
			rc_cnf->rs485_1.valpoint[i]=atoi(w3);
			i++;
			if (i> (NUBER_PCB_IN_OUT-1)) break;
		}
		if  (strncmp(w1,"do16",strlen("do16"))==0)
		{
			rc_cnf->rs485_1.mc[i]=11;
			rc_cnf->rs485_1.addr[i]=atoi(w2);
			rc_cnf->rs485_1.valpoint[i]=atoi(w3);
			i++;
			if (i> (NUBER_PCB_IN_OUT-1)) break;
		}
	}
	f_close(&filecnf);
	
	
	
	strcpy(w1,"0:/rs485_2.cnf");
	ferr = f_open(&filecnf,w1,  FA_READ);
	if (ferr != FR_OK) {		
		return 4;
	} 
	i=0;
	for ( n=0; n < 255; n++) 
	{
		str[0]='\0';
		w1[0]='\0';w2[0]='\0';w3[0]='\0';
	  if (f_gets(str, sizeof(str), &filecnf)==0)
		{
		  if ( f_error(&filecnf) != 0) {
			  return 4;
	    }		
			if ( f_eof(&filecnf) != 0) {
			  break;
	    }	
      return 4;			
	  }
		read_cmd(str,w1,w2,w3);
    if  (strncmp(w1,"ad16",strlen("ad16"))==0)
		{
			rc_cnf->rs485_2.mc[i]=1;
			rc_cnf->rs485_2.addr[i]=atoi(w2);
			rc_cnf->rs485_2.valpoint[i]=atoi(w3);
			i++;
			if (i> (NUBER_PCB_IN_OUT-1)) break;
		}
		if  (strncmp(w1,"ai200",strlen("ai200"))==0)
		{
			rc_cnf->rs485_2.mc[i]=2;
			rc_cnf->rs485_2.addr[i]=atoi(w2);
			rc_cnf->rs485_2.valpoint[i]=atoi(w3);
			i++;
			if (i> (NUBER_PCB_IN_OUT-1)) break;
		}
		if  (strncmp(w1,"dd16",strlen("dd16"))==0)
		{
			rc_cnf->rs485_2.mc[i]=3;
			rc_cnf->rs485_2.addr[i]=atoi(w2);
			rc_cnf->rs485_2.valpoint[i]=atoi(w3);
			i++;
			if (i> (NUBER_PCB_IN_OUT-1)) break;
		}
		if  (strncmp(w1,"do16",strlen("do16"))==0)
		{
			rc_cnf->rs485_2.mc[i]=11;
			rc_cnf->rs485_2.addr[i]=atoi(w2);
			rc_cnf->rs485_2.valpoint[i]=atoi(w3);
			i++;
			if (i> (NUBER_PCB_IN_OUT-1)) break;
		}
		if (strncmp(w1,"dest_port",strlen("dest_port"))==0)
		{
			 rc_cnf->dst_port = atoi(w2);
		}
	}
	f_close(&filecnf);
	
	
	
	strcpy(w1,"0:/rs485_3.cnf");
	ferr = f_open(&filecnf,w1,  FA_READ);
	if (ferr != FR_OK) {		
		return 5;
	}
	i=0;
	for ( n=0; n < 255; n++) 
	{
		str[0]='\0';
		w1[0]='\0';w2[0]='\0';w3[0]='\0';
	  if (f_gets(str, sizeof(str), &filecnf)==0)
		{
		  if ( f_error(&filecnf) != 0) {
			  return 5;
	    }		
			if ( f_eof(&filecnf) != 0) {
			  break;
	    }	
      return 5;			
	  }
		read_cmd(str,w1,w2,w3);
		if  (strncmp(w1,"ad16",strlen("ad16"))==0)
		{
			rc_cnf->rs485_3.mc[i]=1;
			rc_cnf->rs485_3.addr[i]=atoi(w2);
			rc_cnf->rs485_3.valpoint[i]=atoi(w3);
			i++;
			if (i> (NUBER_PCB_IN_OUT-1)) break;
		}
		if  (strncmp(w1,"ai200",strlen("ai200"))==0)
		{
			rc_cnf->rs485_3.mc[i]=2;
			rc_cnf->rs485_3.addr[i]=atoi(w2);
			rc_cnf->rs485_3.valpoint[i]=atoi(w3);
			i++;
			if (i> (NUBER_PCB_IN_OUT-1)) break;
		}
		if  (strncmp(w1,"dd16",strlen("dd16"))==0)
		{
			rc_cnf->rs485_3.mc[i]=3;
			rc_cnf->rs485_3.addr[i]=atoi(w2);
			rc_cnf->rs485_3.valpoint[i]=atoi(w3);
			i++;
			if (i> (NUBER_PCB_IN_OUT-1)) break;
		}
		if  (strncmp(w1,"do16",strlen("do16"))==0)
		{
			rc_cnf->rs485_3.mc[i]=11;
			rc_cnf->rs485_3.addr[i]=atoi(w2);
			rc_cnf->rs485_3.valpoint[i]=atoi(w3);
			i++;
			if (i> (NUBER_PCB_IN_OUT-1)) break;
		}
	}
	f_close(&filecnf);
	
	
	
	strcpy(w1,"0:/rs485_4.cnf");
	ferr = f_open(&filecnf,w1,  FA_READ);
	if (ferr != FR_OK) {		
		return 6;
	}
	i=0;
	for ( n=0; n < 255; n++) 
	{
		str[0]=0;
		w1[0]='\0';w2[0]='\0';w3[0]='\0';
	  if (f_gets(str, sizeof(str), &filecnf)==0)
		{
		  if ( f_error(&filecnf) != 0) {
			  return 6;
	    }		
			if ( f_eof(&filecnf) != 0) {
			  break;
	    }	
      return 6;			
	  }
		read_cmd(str,w1,w2,w3);
		if  (strncmp(w1,"ad16",strlen("ad16"))==0)
		{
			rc_cnf->rs485_4.mc[i]=1;
			rc_cnf->rs485_4.addr[i]=atoi(w2);
			rc_cnf->rs485_4.valpoint[i]=atoi(w3);
			i++;
			if (i> (NUBER_PCB_IN_OUT-1)) break;
		}
		if  (strncmp(w1,"ai200",strlen("ai200"))==0)
		{
			rc_cnf->rs485_4.mc[i]=2;
			rc_cnf->rs485_4.addr[i]=atoi(w2);
			rc_cnf->rs485_4.valpoint[i]=atoi(w3);
			i++;
			if (i> (NUBER_PCB_IN_OUT-1)) break;
		}
		if  (strncmp(w1,"dd16",strlen("dd16"))==0)
		{
			rc_cnf->rs485_4.mc[i]=3;
			rc_cnf->rs485_4.addr[i]=atoi(w2);
			rc_cnf->rs485_4.valpoint[i]=atoi(w3);
			i++;
			if (i> (NUBER_PCB_IN_OUT-1)) break;
		}
		if  (strncmp(w1,"do16",strlen("do16"))==0)
		{
			rc_cnf->rs485_4.mc[i]=11;
			rc_cnf->rs485_4.addr[i]=atoi(w2);
			rc_cnf->rs485_4.valpoint[i]=atoi(w3);
			i++;
			if (i> (NUBER_PCB_IN_OUT-1)) break;
		}
	}
	f_close(&filecnf);
	
	
	
	strcpy(w1,"0:/adc_i2f.cnf");
	ferr = f_open(&filecnf,w1,  FA_READ);
	if (ferr != FR_OK) {
		return 7;
	}
	for ( n=0; n < 1024; n++)
	{
		str[0]=0;
	  if (f_gets(str, sizeof(str), &filecnf)==0)
		{
		  if ( f_error(&filecnf) != 0) {
			  return 7;
	    }		
			if ( f_eof(&filecnf) != 0) {
			  break;
	    }	
      return 7;			
	  }
		w1[0]=0; w2[0]=0; w3[0]=0;
		read_cmd(str,w1,w2,w3);
		read_cnf_adc_str2float(w1,w2,w3);
	}
	f_close(&filecnf);
	
	f_mount(0, NULL);
	return 0;
}






