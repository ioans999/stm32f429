#include "algblock.h"

/* ------------------------ STM32F4xx lib----------------------------- */
#include "stm32f4xx.h"
#include "arm_math.h"

/* ------------------------ FreeRTOS includes ----------------------------- */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"                                                             
#include "queue.h"


#include "ff.h"

/* ------------------------ Project includes ------------------------------ */
#include "main.h"
#include "utilities.h"
#include "sys_config.h"
#include "udp_server.h"
#include "usart.h" 

#include <string.h>
#include <stdlib.h>

volatile float alg_bd_fl[128];

uint8_t alg_interval(int a1,int a2,int b,uint8_t a);
uint16_t alg_inc(uint16_t *a);
void Alg_write_data_from_buf(void);
void Alg_write_data_to_buf_read(void);

xSemaphoreHandle xBinarySemaphoreUSART1StartALG;
xSemaphoreHandle xBinarySemaphoreUSART1sendAD_DD;
xSemaphoreHandle xBinarySemaphoreUSART1sendDD;
xSemaphoreHandle xBinarySemaphoreUSART1AndDO;

xSemaphoreHandle xBinarySemaphoreUSART2StartALG;
xSemaphoreHandle xBinarySemaphoreUSART2sendAD_DD;
xSemaphoreHandle xBinarySemaphoreUSART2sendDD;
xSemaphoreHandle xBinarySemaphoreUSART2AndDO;

xSemaphoreHandle xBinarySemaphoreUSART3StartALG;
xSemaphoreHandle xBinarySemaphoreUSART3sendAD_DD;
xSemaphoreHandle xBinarySemaphoreUSART3sendDD;
xSemaphoreHandle xBinarySemaphoreUSART3AndDO;

xSemaphoreHandle xBinarySemaphoreUSART4StartALG;

extern xSemaphoreHandle xMutex_ALGBLOC_RW_DataVar;
extern xSemaphoreHandle xMutex_ALGBLOC_RW_DataVar_Buf;

typedef struct {
	struct {
		float		AD[128];
		uint8_t AD_Mask[16];
		uint8_t DD[16];        //128 bit in discret channal
		uint8_t DD_Mask[16];
		uint8_t DO[16];        //128 bit out discret channal
		uint8_t DO_Mask[16];
		uint16_t AO[128];      //128  integer16 Analog Out
		uint8_t AO_Mask[16];
	} mc_in_out;
	float			RP[512]; //512 Analog send to ARM
	uint8_t		IP[64];  //512 bit discret send to ARM
	uint8_t		LR[512];  //512 bit discret
	uint8_t		IR[512];
	uint16_t	IR2[512];
	uint32_t	IR4[512];	
} ALG_global_value_type;
	
struct {
	uint32_t rs485_1_send;
	uint32_t rs485_1_rec;
	uint32_t rs485_2_send;
	uint32_t rs485_2_rec;
	uint32_t rs485_3_send;
	uint32_t rs485_3_rec;
	uint32_t rs485_4_send;
	uint32_t rs485_4_rec;
	uint32_t udp_send_count;
	uint32_t CiclePeriod;
	uint32_t AlgCount;
} sys_inf_global_value;

struct  {
	float		AD[128];
	uint8_t AD_Mask[16];
	uint8_t DD[16];        //128 bit in discret channal
	uint8_t DD_Mask[16];
	uint8_t DO[16];        //128 bit out discret channal
	uint8_t DO_Mask[16];
	uint16_t AO[128];      //128  integer16 Analog Out
	uint8_t AO_Mask[16];
	float	  RP[512]; //512 Analog send to ARM
	uint8_t RP_Mask[64];
	uint8_t	IP[64];  //512 bit discret send to ARM
	uint8_t IP_Mask[64];
} mc_in_out_buf;

struct  {
	uint8_t DO[16];        //128 bit out discret channal
	uint16_t AO[128];      //128  integer16 Analog Out
} mc_out_buf_read;


struct {
	float func_a[128];
	float func_b[128];
} adc_func_int2float; 


ALG_global_value_type	ALG_global_value;
uint32_t CicleTime;
uint8_t modbus_lost_flag;
uint8_t modbus_lost_flag_u4;

float test_float[128];

void vAlg_block_Task(void * pvParameters)
{
	volatile static uint32_t (*program)(ALG_global_value_type*, uint32_t);
		
	FATFS   fs; 
  FIL     file;
  FRESULT ferr; 
	char *buff;
	uint32_t  j,len,n,i;
	uint8_t *addr;
	volatile uint8_t alg_file_load=1;

	
  vSemaphoreCreateBinary(xBinarySemaphoreUSART1StartALG);
	vSemaphoreCreateBinary(xBinarySemaphoreUSART2StartALG);
	vSemaphoreCreateBinary(xBinarySemaphoreUSART3StartALG);
//	vSemaphoreCreateBinary(xBinarySemaphoreUSART4StartALG);
	
	vSemaphoreCreateBinary(xBinarySemaphoreUSART1AndDO);
	vSemaphoreCreateBinary(xBinarySemaphoreUSART2AndDO);
	vSemaphoreCreateBinary(xBinarySemaphoreUSART3AndDO);
	
	program = (int(*)(int, char **))0x60000001;
	
	
	if( (buff =(char *)pvPortMalloc(1500)) == NULL) {    
      vPortFree(buff);
      vTaskDelete( NULL ); 
  }
		/*
	ferr = f_mount(0, &fs);
  if (ferr == FR_OK)
	{
			ferr = f_open(&file, ("0:alg.bin"),  FA_READ);
			if (ferr == FR_OK)
			{			
				for (j = 0;;)
				{
					len=0;
					ferr = f_read(&file, buff, sizeof(buff), &len);  
					if (ferr != FR_OK)  
					{
						alg_file_load = 0;
						break;
					}
          addr = (uint8_t *)0x60000000;					
					for(n = 0; n < len; n++ )
					{
					   addr = 0x60000000 + (uint8_t *)j+n;
					   *addr = buff[n];	  				
					}
					j += len;					
					if (len < sizeof(buff)) break;  //error or eof 				
				}
			  f_close(&file);
			}
			else
			{
				alg_file_load = 0;
			}
			f_mount(0, NULL);
	}
	else
	{
		 alg_file_load = 0;
	}
	*/
	xSemaphoreTake(xBinarySemaphoreUSART1StartALG, portMAX_DELAY);
	xSemaphoreTake(xBinarySemaphoreUSART2StartALG, portMAX_DELAY);
	xSemaphoreTake(xBinarySemaphoreUSART3StartALG, portMAX_DELAY);
	
	//xSemaphoreTake(xBinarySemaphoreUSART4StartALG, portMAX_DELAY);
	//xSemaphoreTake(xBinarySemaphoreUSART4StartALG, portMAX_DELAY);
	for (;;)
  {
		xSemaphoreTake(xBinarySemaphoreUSART1AndDO, portMAX_DELAY);
		xSemaphoreTake(xBinarySemaphoreUSART2AndDO, portMAX_DELAY);
		xSemaphoreTake(xBinarySemaphoreUSART3AndDO, portMAX_DELAY);
		for(j=0;j<16;j++)
		{
			ALG_global_value.mc_in_out.AD_Mask[j]=0xFF;
			ALG_global_value.mc_in_out.DD_Mask[j]=0xFF;
		}
		xSemaphoreGive(xBinarySemaphoreUSART1sendAD_DD);
		xSemaphoreGive(xBinarySemaphoreUSART2sendAD_DD);
		xSemaphoreGive(xBinarySemaphoreUSART3sendAD_DD);
		xSemaphoreTake(xBinarySemaphoreUSART1StartALG, portMAX_DELAY);
		xSemaphoreTake(xBinarySemaphoreUSART2StartALG, portMAX_DELAY);
		xSemaphoreTake(xBinarySemaphoreUSART3StartALG, portMAX_DELAY);
		
		
		Alg_write_data_to_buf_read();
		Alg_write_data_from_buf();

		sys_inf_global_value.CiclePeriod = CicleTime;
		CicleTime = 0;
		
		//xSemaphoreTake(xMutex_ALGBLOC_RW_DataVar, portMAX_DELAY );
		{
			if(modbus_lost_flag == 0 && modbus_lost_flag_u4 == 0 && alg_file_load == 1)
			{
				sys_inf_global_value.AlgCount++;
				//(*program)(&ALG_global_value, sys_inf_global_value.CiclePeriod);
			}
			modbus_lost_flag = 0;
		
			buff[0]=1; buff[1]=0; buff[2]=0; buff[3]=0;
			memcpy(buff+4, &(ALG_global_value.mc_in_out), sizeof(ALG_global_value.mc_in_out));
			memcpy(buff+sizeof(ALG_global_value.mc_in_out)+4, &sys_inf_global_value, sizeof(sys_inf_global_value));
			//UDP_send((uint8_t *) buff, sizeof(ALG_global_value.mc_in_out)+sizeof(sys_inf_global_value)+4); 
			sys_inf_global_value.udp_send_count++;
		
			buff[0]=2; buff[1]=0; buff[2]=0; buff[3]=0;
			memcpy(buff+4, ALG_global_value.RP, sizeof(ALG_global_value.RP)/2);
			//UDP_send((uint8_t *) buff, sizeof(ALG_global_value.RP)/2+4); 
			sys_inf_global_value.udp_send_count++;
		
			buff[0]=3; buff[1]=0; buff[2]=0; buff[3]=0;
			memcpy(buff+4, ALG_global_value.RP + 256, sizeof(ALG_global_value.RP)/2);
			memcpy(buff+sizeof(ALG_global_value.RP)/2 + 4, ALG_global_value.IP, sizeof(ALG_global_value.IP));
			//UDP_send((uint8_t *) buff, sizeof(ALG_global_value.RP)/2 + sizeof(ALG_global_value.IP)+4);
			sys_inf_global_value.udp_send_count++;
		
			if (sys_inf_global_value.udp_send_count > 0xFFFFFF00)	sys_inf_global_value.udp_send_count=0;
		
			for (i=0; i<128; i++)
			{
				ALG_global_value.mc_in_out.AD_Mask[i/8] &= ~(1 << (i%8));
				ALG_global_value.mc_in_out.DD_Mask[i/8] &= ~(1 << (i%8));
				ALG_global_value.mc_in_out.AO_Mask[i/8] &= ~(1 << (i%8));
			}
		}
		//xSemaphoreGive(xMutex_ALGBLOC_RW_DataVar);
		
		xSemaphoreGive(xBinarySemaphoreUSART1sendDD);
		xSemaphoreGive(xBinarySemaphoreUSART2sendDD);
		xSemaphoreGive(xBinarySemaphoreUSART3sendDD);
	}
}





void write_modbas_inf(uint32_t val, uint8_t t, uint8_t nm)
{
	if(nm==1) {
		if (t==1)
		  sys_inf_global_value.rs485_1_send=val;
		if (t==2)
		  sys_inf_global_value.rs485_1_rec=val;
	}
	if(nm==2) {
		if (t==1)
		  sys_inf_global_value.rs485_2_send=val;
		if (t==2)
		  sys_inf_global_value.rs485_2_rec=val;
	}
	if(nm==3) {
		if (t==1)
		  sys_inf_global_value.rs485_3_send=val;
		if (t==2)
		  sys_inf_global_value.rs485_3_rec=val;
	}
	if(nm==4) {
		if (t==1)
		  sys_inf_global_value.rs485_4_send=val;
		if (t==2)
		  sys_inf_global_value.rs485_4_rec=val;
	}	
}



/*******************************************************************************************************************************
*@
                 =============================================================================================
                                            Functions of Read Write to data MC 
                 =============================================================================================
*@ Write to Data
*@ Read from Data
************************************************************/


void write_mc_value(uint8_t *val, uint32_t p,uint8_t ln,uint8_t m)
{
	uint8_t i;

	//xSemaphoreTake(xMutex_ALGBLOC_RW_DataVar, portMAX_DELAY );
	if (m==1)
		for (i=0;i<ln;i++) 
	  {
			ALG_global_value.mc_in_out.AD[p+i] = (val[i*2+1]*0x100 + val[i*2])*adc_func_int2float.func_a[p+i]+adc_func_int2float.func_b[p+i];
			ALG_global_value.mc_in_out.AD_Mask[(p+i)/8] |= (1 << ((p+i)%8));
		}
	if (m==2)
	  for (i=0;i<ln;i++) 
	  {
			ALG_global_value.mc_in_out.DD[(p+i)/8] &= ~(1 << (p+i)%8);
			if ((p+i)%8 >= i%8)
			{
				ALG_global_value.mc_in_out.DD[(p+i)/8] |= (val[i/8] << (((p+i)%8)-(i%8))) & (1 << ((p+i)%8));
			}
			else
			{
				ALG_global_value.mc_in_out.DD[(p+i)/8] |= (val[i/8] >> ((i%8)-((p+i)%8))) & (1 << ((p+i)%8));
			}
			ALG_global_value.mc_in_out.DD_Mask[(p+i)/8] |= (1 << (p+i)%8);
		}
	if (m==3)
		for (i=0;i<ln;i++)
	  {
			ALG_global_value.mc_in_out.DO[(p+i)/8] &= ~(1 << (p+i)%8);
			if ((p+i)%8 >= i%8)
			{
				ALG_global_value.mc_in_out.DO[(p+i)/8] |= (val[i/8] << (((p+i)%8)-(i%8))) & (1 << ((p+i)%8));
			}
			else
			{
				ALG_global_value.mc_in_out.DO[(p+i)/8] |= (val[i/8] >> ((i%8)-((p+i)%8))) & (1 << ((p+i)%8));
			}
			ALG_global_value.mc_in_out.DO_Mask[(p+i)/8] |= (1 << (p+i)%8);
		}
	if (m==4)
		for (i=0;i<ln;i++) 
	  {
			ALG_global_value.mc_in_out.AO[p+i] = val[i*2+1]*0x100 + val[i*2];
			ALG_global_value.mc_in_out.AO_Mask[(p+i)/8] |= (1 << (p+i)%8);
		}
	//xSemaphoreGive(xMutex_ALGBLOC_RW_DataVar);
}


uint8_t  get_mc_DO_byte(uint8_t  n)
{
	uint8_t i, b;
	b=0;
	//xSemaphoreTake(xMutex_ALGBLOC_RW_DataVar, portMAX_DELAY );
	for ( i=0; i<8; i++)
	{
		b |= ((ALG_global_value.mc_in_out.DO[(n+i)/8] >> (n+i)%8) & 1) << i;
	}
	//xSemaphoreGive(xMutex_ALGBLOC_RW_DataVar);
  return b;
}

uint16_t  get_mc_AO_byte(uint8_t  n)
{
	uint16_t b;
	//xSemaphoreTake(xMutex_ALGBLOC_RW_DataVar, portMAX_DELAY );
	b = ALG_global_value.mc_in_out.AO[n];
	//xSemaphoreGive(xMutex_ALGBLOC_RW_DataVar);
	return b;
}
/*********
*@
*@
*@
**************************************************************************************************************/


/**************************************************************************************************************
*@
                 =============================================================================================
                                            Functions of Read Write to data buffer 
                 =============================================================================================
*@ Write to DataBuf from periphery
*@ Read from DataBuf and Write to DataMc
*@ 
*********/

void Alg_write_data_to_buf_read(void)
{
	volatile uint32_t i;
	
	//xSemaphoreTake(xMutex_ALGBLOC_RW_DataVar_Buf, portMAX_DELAY );
	//xSemaphoreTake(xMutex_ALGBLOC_RW_DataVar, portMAX_DELAY );
	for (i=0; i<128; i++) 
	{
		if ((ALG_global_value.mc_in_out.DO[i/8] & (1 << i%8)) == (1 << i%8))
		{
			mc_out_buf_read.DO[i/8] |= 1 << i%8;
		}
		else
		{
			mc_out_buf_read.DO[i/8] &= ~(1 << i%8);
		}

		mc_out_buf_read.AO[i] = ALG_global_value.mc_in_out.AO[i];
	}
	//xSemaphoreGive(xMutex_ALGBLOC_RW_DataVar);
	//xSemaphoreGive(xMutex_ALGBLOC_RW_DataVar_Buf);
}
/***********************
*
************************/
uint8_t  get_mc_DO_byte_from_buf(uint8_t  n)
{
	uint8_t i, b;
	b=0;
	//xSemaphoreGive(xMutex_ALGBLOC_RW_DataVar_Buf);
	for ( i=0; i<8; i++)
	{
		b |= ((mc_out_buf_read.DO[(n+i)/8] >> (n+i)%8) & 1) << i;
	}
	//xSemaphoreGive(xMutex_ALGBLOC_RW_DataVar_Buf);
  return b;
}
/***********************
*
************************/
uint16_t  get_mc_AO_byte_from_buf(uint8_t  n)
{
	uint16_t b;
	//xSemaphoreGive(xMutex_ALGBLOC_RW_DataVar_Buf);
	b = mc_out_buf_read.AO[n];
	//xSemaphoreGive(xMutex_ALGBLOC_RW_DataVar_Buf);
	return b;
}
/***********************
*
************************/

void Alg_write_data_from_buf(void)
{
	volatile uint32_t i;

	//xSemaphoreTake(xMutex_ALGBLOC_RW_DataVar_Buf, portMAX_DELAY );
	//xSemaphoreTake(xMutex_ALGBLOC_RW_DataVar, portMAX_DELAY );
	for (i=0; i<128; i++) 
	{
		if ((mc_in_out_buf.AD_Mask[i/8] & (1 << (i%8))) == (1 << (i%8)))
		{
			ALG_global_value.mc_in_out.AD[i] = mc_in_out_buf.AD[i];
			ALG_global_value.mc_in_out.AD_Mask[i/8] |= 1 << (i%8);
			mc_in_out_buf.AD_Mask[i/8] &= ~(1 << i%8);
		}
		
		if ((mc_in_out_buf.DD_Mask[i/8] & (1 << (i%8))) == (1 << (i%8)))
		{
			if ((mc_in_out_buf.DD[i/8] & (1 << (i%8))) == (1 << (i%8)))
			{
				ALG_global_value.mc_in_out.DD[i/8] |= 1 << (i%8);
			}
			else
			{
				ALG_global_value.mc_in_out.DD[i/8] &= ~(1 << (i%8));
			}
			ALG_global_value.mc_in_out.DD_Mask[i/8] |= 1 << (i%8);
			mc_in_out_buf.DD_Mask[i/8] &= ~(1 << i%8);
		}
		
		if ((mc_in_out_buf.DO_Mask[i/8] & (1 << i%8)) == (1 << i%8))
		{
			if ((mc_in_out_buf.DO[i/8] & (1 << i%8)) == (1 << i%8))
			{
				ALG_global_value.mc_in_out.DO[i/8] |= 1 << i%8;
			}
			else
			{
				ALG_global_value.mc_in_out.DO[i/8] &= ~(1 << i%8);
			}
			ALG_global_value.mc_in_out.DO_Mask[i/8] |= 1 << i%8;
			mc_in_out_buf.DO_Mask[i/8] &= ~(1 << i%8);
		}
		
		if ((mc_in_out_buf.AO_Mask[i/8] & (1 << (i%8))) == (1 << (i%8)))
		{
			ALG_global_value.mc_in_out.AO[i] = mc_in_out_buf.AO[i];
			ALG_global_value.mc_in_out.AO_Mask[i/8] |= 1 << (i%8);
			mc_in_out_buf.AO_Mask[i/8] &= ~(1 << i%8);
		}
	}
	
	for (i=0; i<512; i++) 
	{	
		if ((mc_in_out_buf.RP_Mask[i/8] & (1 << (i%8))) == (1 << (i%8)))
		{
			ALG_global_value.RP[i] = mc_in_out_buf.RP[i];
			mc_in_out_buf.RP_Mask[i/8] &= ~(1 << i%8);
		}
		
		if ((mc_in_out_buf.IP_Mask[i/8] & (1 << (i%8))) == (1 << (i%8)))
		{
			if ((mc_in_out_buf.IP[i/8] & (1 << i%8)) == (1 << i%8))
			{
				ALG_global_value.IP[i/8] |= 1 << i%8;
			}
			else
			{
				ALG_global_value.IP[i/8] &= ~(1 << i%8);
			}
			mc_in_out_buf.IP_Mask[i/8] &= ~(1 << i%8);
		}
	}
	//xSemaphoreGive(xMutex_ALGBLOC_RW_DataVar);
	//xSemaphoreGive(xMutex_ALGBLOC_RW_DataVar_Buf);
}

/***********************
*
************************/


void write_mc_value_to_buf(uint8_t *val, uint32_t p, uint32_t s,uint8_t ln,uint8_t m)
{
	uint8_t i;
	typedef union 
	{
		float fl;
		uint8_t b[4];
	} floatbyte_type;
	uint32_t pv,pd;
	floatbyte_type floatbyte;
	
	//xSemaphoreTake(xMutex_ALGBLOC_RW_DataVar_Buf, portMAX_DELAY );
	if (m==1)
		for (i=0;i<ln;i++) {
			mc_in_out_buf.AD[p+i] = (val[s*2 + i*2+1]*0x100 + val[s*2 + i*2])*adc_func_int2float.func_a[p+i]+adc_func_int2float.func_b[p+i];
			mc_in_out_buf.AD_Mask[(p+i)/8] |= 1 << ((p+i)%8);
		}
	if (m==2)
		for (i=0;i<ln;i++) 
	  {
			pv = i + s;
			pd = i + p;
			mc_in_out_buf.DD[pd/8] &= ~(1 << pd % 8);
			if (pd % 8 >= pv % 8)
			{
				mc_in_out_buf.DD[pd/8] |= (val[pv/8] << p%8-s%8) & (1 << pd%8);
			}
			else
			{
				mc_in_out_buf.DD[pd/8] |= (val[pv/8] >> 8-p%8+s%8) & (1 << pd%8);
			}
			mc_in_out_buf.DD_Mask[pd/8] |= (1 << pd%8);
		}
	if (m==3)
		for (i=0;i<ln;i++) 
	  {
			pv = i + s;
			pd = i + p;
			mc_in_out_buf.DO[pd/8] &= ~(1 << pd%8);
			if ((pd % 8) >= (pv % 8))
			{
				mc_in_out_buf.DO[pd/8] |= (val[pv/8] << (p%8-s%8)) & (1 << (pd%8));
			}
			else
			{
				mc_in_out_buf.DO[pd/8] |= (val[pv/8] >> (8-p%8+s%8)) & (1 << (pd%8));
			}
			mc_in_out_buf.DO_Mask[pd/8] |= (1 << pd%8);
		}
	if (m==4)
		for (i=0;i<ln;i++) 
	  {
			mc_in_out_buf.AO[p+i] = val[s*2+i*2+1]*0x100 + val[s*2 + i*2];
			mc_in_out_buf.AO_Mask[(p+i)/8] |= (1 << (p+i)%8);
		}
	if (m==5)
		for (i=0;i<ln;i++) 
	  {
			floatbyte.b[0] = val[s*4 + i*4];
			floatbyte.b[1] = val[s*4 + i*4 + 1];
			floatbyte.b[2] = val[s*4 + i*4 + 2];
			floatbyte.b[3] = val[s*4 + i*4 + 3];
			mc_in_out_buf.RP[p+i] = floatbyte.fl;
			mc_in_out_buf.RP_Mask[(p+i)/8] |= (1 << (p+i)%8);
		}
	if (m==6)
		for (i=0;i<ln;i++) 
	  {
			pv = i + s;
			pd = i + p;
			mc_in_out_buf.IP[pd/8] &= ~(1 << pd % 8);
			if (pd % 8 >= pv % 8)
			{
				mc_in_out_buf.IP[pd/8] |= (val[pv/8] << p%8-s%8) & (1 << pd%8);
			}
			else
			{
				mc_in_out_buf.IP[pd/8] |= (val[pv/8] >> 8-p%8+s%8) & (1 << pd%8);
			}
			mc_in_out_buf.IP_Mask[pd/8] |= (1 << pd%8);
		}
	//xSemaphoreGive(xMutex_ALGBLOC_RW_DataVar_Buf);
}

/***********************
*
************************/


void write_mc_value_lost_to_buf(uint32_t p,uint8_t ln,uint8_t m)
{
	uint8_t i;

	///xSemaphoreTake(xMutex_ALGBLOC_RW_DataVar_Buf, portMAX_DELAY );
	if (m==1)
		for (i=0;i<ln;i++) {
			mc_in_out_buf.AD[p+i] = 0;
			mc_in_out_buf.AD_Mask[(p+i)/8] &= ~(1 << (p+i)%8);
		}
	if (m==2)
		for (i=0;i<ln;i++) {
			mc_in_out_buf.DD[(p+i)/8] &= ~(1 << ((p+i)%8));
			mc_in_out_buf.DD_Mask[(p+i)/8] &= ~(1 << (p+i)%8);
		}
	if (m==4)
		for (i=0;i<ln;i++) {
			mc_in_out_buf.DD[(p+i)/8] &= ~(1 << (p+i)%8);
			mc_in_out_buf.DO_Mask[(p+i)/8] &= ~(1 << (p+i)%8);
		}
	if (m==4)
		for (i=0;i<ln;i++) {
			mc_in_out_buf.AO[p+i] = 0;
			mc_in_out_buf.AO_Mask[(p+i)/8] &= ~(1 << (p+i)%8);
		}
	///xSemaphoreGive(xMutex_ALGBLOC_RW_DataVar_Buf);
}
/***********************
*@
*@
*@
**************************************************************************************************************/






void Server_Data_in()
{
	//xSemaphoreTake(xMutex_ALGBLOC_RW_DataVar, portMAX_DELAY );
	//xSemaphoreGive(xMutex_ALGBLOC_RW_DataVar);
}

void AlgCicleTimeInc(void)
{
	CicleTime++;
}

void AlgLostModbusPacket(void)
{
	modbus_lost_flag = 1;
}
void AlgLostModbusPacketU4(uint8_t n)
{
	modbus_lost_flag_u4 = n;
}

void WriteCnfAdcFloat(uint16_t num, float a, float b)
{
	adc_func_int2float.func_a[num]=a;
	adc_func_int2float.func_b[num]=b;
}
