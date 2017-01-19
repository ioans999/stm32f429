#include "usart.h"

#include "stm32f4xx.h"

#include "stm32f4xx_gpio.h"
#include "stm32f4xx_dma.h"

#include "algblock.h"
#include "FreeRTOS.h"
#include "task.h"
#include "modbus.h"
#include "algdef.h"

#include <stdlib.h>
#include <string.h>

extern xSemaphoreHandle xBinarySemaphoreUSART1StartALG;
extern xSemaphoreHandle xBinarySemaphoreUSART1sendAD_DD;
extern xSemaphoreHandle xBinarySemaphoreUSART1sendDD;
extern xSemaphoreHandle xBinarySemaphoreUSART1AndDO;
extern xSemaphoreHandle xMutex_USART1_USART3_DmaSend;

void delete_modbas_buf(void);

struct	{
	uint8_t list_cntrl[NUBER_PCB_IN_OUT];
	uint8_t addr_cntrl[NUBER_PCB_IN_OUT];
	uint32_t point_write_value[NUBER_PCB_IN_OUT];
}	RS485_ucntrl_n1;
	
struct	{
	uint8_t data[128];
	uint8_t count_b;
	uint32_t timeout;
	uint8_t flag;
} RS485_Modbus_n1;






void v_rs485_task_n1(void * pvParameters)
{
  uint32_t all_tx=0, all_rx=0, current_lost=0;
  uint8_t ln=0;
	uint32_t i=0,n=0,count_mc=0;
  
	uint8_t bf[44];
	
	//Init LED1
	GPIO_InitTypeDef gpio;
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOH, ENABLE );
	gpio.GPIO_OType = GPIO_OType_PP;
  gpio.GPIO_Mode = GPIO_Mode_OUT;
	gpio.GPIO_Pin = GPIO_Pin_3;
  gpio.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOH, &gpio);
	
 	vSemaphoreCreateBinary(xBinarySemaphoreUSART1sendAD_DD);
 	vSemaphoreCreateBinary(xBinarySemaphoreUSART1sendDD);
	xSemaphoreTake(xBinarySemaphoreUSART1sendAD_DD, portMAX_DELAY);
	xSemaphoreTake(xBinarySemaphoreUSART1sendDD,portMAX_DELAY);
	
  while(1)
	{
		xSemaphoreTake(xBinarySemaphoreUSART1sendAD_DD, portMAX_DELAY);
		current_lost = all_tx - all_rx;
		for (count_mc=0; count_mc<NUBER_PCB_IN_OUT; count_mc++)
		{		
			if (RS485_ucntrl_n1.list_cntrl[count_mc]>0 && RS485_ucntrl_n1.list_cntrl[count_mc]<10)
			{		
        if 	(all_tx >= 0xFFFFFF00) {
					all_tx=0;
					all_rx=0;
					current_lost = 0;
				}							
				/////////////////////Send packet
				RS485_Modbus_n1.flag=0;
				ln=Create_mb_packet(bf, RS485_ucntrl_n1.list_cntrl[count_mc], RS485_ucntrl_n1.addr_cntrl[count_mc]);
			  rs485_DMASend_n1(bf,ln,0);
		    all_tx++;
				
				/////////////////////Recive packet
				for (i=0; i<125; i++)
				{
					vTaskDelay(2 / portTICK_RATE_MS );
					if(RS485_Modbus_n1.flag==3)
					{
						break;
					}
				}
				if(RS485_Modbus_n1.flag==3)
				{
					GPIO_SetBits(GPIOH, GPIO_Pin_3);
					if (Chk_mb_packet(RS485_Modbus_n1.data, RS485_ucntrl_n1.list_cntrl[count_mc], RS485_ucntrl_n1.addr_cntrl[count_mc], RS485_Modbus_n1.count_b) ==0)
					{
						//Write Analog from ad16
						if (RS485_ucntrl_n1.list_cntrl[count_mc]==1){
							  write_mc_value(RS485_Modbus_n1.data+3, RS485_ucntrl_n1.point_write_value[count_mc],16,1);
						}
						//Write Analog from ai200
						if (RS485_ucntrl_n1.list_cntrl[count_mc]==2){
							  write_mc_value(RS485_Modbus_n1.data+4, RS485_ucntrl_n1.point_write_value[count_mc],12,1);
						}
						//Write Discret from dd16
						if (RS485_ucntrl_n1.list_cntrl[count_mc]==3){
							  write_mc_value(RS485_Modbus_n1.data+3, RS485_ucntrl_n1.point_write_value[count_mc],16,2);
						}
						all_rx++;
					}
		 	  } 
				vTaskDelay(2/ portTICK_RATE_MS );
				GPIO_ResetBits(GPIOH, GPIO_Pin_3);
			}
		}
		write_modbas_inf(all_tx,1,1);
		write_modbas_inf(all_rx,2,1);
		if ((all_tx - all_rx) > current_lost)
		{
			AlgLostModbusPacket();
		}
		xSemaphoreGive(xBinarySemaphoreUSART1StartALG);
		

		xSemaphoreTake(xBinarySemaphoreUSART1sendDD,portMAX_DELAY);
		current_lost = all_tx - all_rx;
		for (count_mc=0; count_mc<NUBER_PCB_IN_OUT; count_mc++)
		{
			if (RS485_ucntrl_n1.list_cntrl[count_mc]>10)
			{
				if 	(all_tx >= 0xFFFFFF00) 
				{
					all_tx=0;
					all_rx=0;
				}			
				/////////////////Send packet			
				//Get Discret for do16
				if (RS485_ucntrl_n1.list_cntrl[count_mc]==11)
				{
					bf[5]=get_mc_DO_byte(RS485_ucntrl_n1.point_write_value[count_mc]);
					bf[6]=get_mc_DO_byte(RS485_ucntrl_n1.point_write_value[count_mc]+8);
					for(i=0; i<16; i++)
					{
						uint16_t ao_v;
						ao_v = get_mc_AO_byte(RS485_ucntrl_n1.point_write_value[count_mc]);
						bf[7+i*2] = ao_v & 0xFF;
						bf[8+i*2] = (ao_v >> 8) & 0xFF;
					}
				}
				RS485_Modbus_n1.flag=0;
				ln=Create_mb_packet(bf, RS485_ucntrl_n1.list_cntrl[count_mc], RS485_ucntrl_n1.addr_cntrl[count_mc]);
				rs485_DMASend_n1(bf,ln,0);
				all_tx++;
				
				///////////////Recive packet
				for (i=0; i<25; i++)
				{
					vTaskDelay(2 / portTICK_RATE_MS );
					if(RS485_Modbus_n1.flag==3)
					{
						 break;
					}
				}
				if(RS485_Modbus_n1.flag==3)
				{
					GPIO_SetBits(GPIOH, GPIO_Pin_3);
					if (Chk_mb_packet(RS485_Modbus_n1.data, RS485_ucntrl_n1.list_cntrl[count_mc],RS485_ucntrl_n1.addr_cntrl[count_mc], RS485_Modbus_n1.count_b) ==0)
					{
						 if (RS485_ucntrl_n1.list_cntrl[count_mc]==11){
							 write_mc_value(RS485_Modbus_n1.data+3, RS485_ucntrl_n1.point_write_value[count_mc],16,3);
							 write_mc_value(RS485_Modbus_n1.data+5, RS485_ucntrl_n1.point_write_value[count_mc],16,4);
						 }
						 all_rx++;
					}
		 	  }
				vTaskDelay(2/ portTICK_RATE_MS );
				GPIO_ResetBits(GPIOH, GPIO_Pin_3);
			}
		}
		if ((all_tx - all_rx) > current_lost)
		{
			AlgLostModbusPacket();
		}
		//vTaskDelay(4/ portTICK_RATE_MS );
		xSemaphoreGive(xBinarySemaphoreUSART1AndDO);
	}
}





void modbus_IRQ_n1(uint8_t b_in)
{
	RS485_Modbus_n1.timeout=0;
	if (RS485_Modbus_n1.flag==0)
	{
		RS485_Modbus_n1.count_b=0;
		RS485_Modbus_n1.flag=1;
	}
	if (RS485_Modbus_n1.flag==1)
	{
		if (RS485_Modbus_n1.count_b >127)
		{
			RS485_Modbus_n1.count_b=0;
		}
    RS485_Modbus_n1.data[RS485_Modbus_n1.count_b] = b_in;
    RS485_Modbus_n1.count_b++;
	}
	if (RS485_Modbus_n1.flag==2)
	{
		RS485_Modbus_n1.flag=4;
	}
}


void timer_IRQ_n1(void)
{
  RS485_Modbus_n1.timeout++;
	if ((RS485_Modbus_n1.timeout>11 )&&(RS485_Modbus_n1.flag==1))
		RS485_Modbus_n1.flag=2;
	if ((RS485_Modbus_n1.timeout>22) && (RS485_Modbus_n1.flag==2))
		RS485_Modbus_n1.flag=3;
}



void USART_time_reset_n1(void)
{
	RS485_Modbus_n1.timeout=0;
}


void rs485_DMASend_n12(uint8_t *source, uint16_t size,uint8_t cut_after_newline)
{
	volatile int i = 0;
	
  GPIO_SetBits(GPIOA, GPIO_Pin_4);
  for(i=0;i<size;i++)
  {
		while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
		  USART_SendData(USART1, (uint8_t) source[i]);
  }
}

void rs485_DMASend_n1(uint8_t *source, uint16_t size,uint8_t cut_after_newline)
{
	 DMA_InitTypeDef dma;
	 NVIC_InitTypeDef dma_nvic;
	
	 GPIO_SetBits(GPIOA, GPIO_Pin_4);
	
	 xSemaphoreTake(xMutex_USART1_USART3_DmaSend, portMAX_DELAY );
	 
	 DMA_DeInit(DMA2_Stream7);
  
   dma.DMA_BufferSize = size;
   dma.DMA_Channel = DMA_Channel_4;
   dma.DMA_DIR = DMA_DIR_MemoryToPeripheral;
   dma.DMA_FIFOMode = DMA_FIFOMode_Disable;
   dma.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
   dma.DMA_Memory0BaseAddr = (uint32_t)source;
   dma.DMA_MemoryBurst = DMA_MemoryBurst_Single;
   dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
   dma.DMA_MemoryInc = DMA_MemoryInc_Enable;
   dma.DMA_Mode = DMA_Mode_Normal;
   dma.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->DR);
   dma.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
   dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
   dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
   dma.DMA_Priority = DMA_Priority_Low;

   dma_nvic.NVIC_IRQChannel = DMA2_Stream7_IRQn;
   dma_nvic.NVIC_IRQChannelCmd = ENABLE;
   dma_nvic.NVIC_IRQChannelPreemptionPriority = 1;
   dma_nvic.NVIC_IRQChannelSubPriority = 0;
   DMA_Init(DMA2_Stream7, &dma);
	 DMA_ITConfig(DMA2_Stream7, DMA_IT_TC, ENABLE);
	 NVIC_EnableIRQ(DMA2_Stream7_IRQn);
	 DMA_Cmd(DMA2_Stream7, ENABLE);
	 
   //NVIC_Init(&dma_nvic);
	 USART_ClearFlag(USART1, USART_FLAG_TC);	 
   USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
   return;
}






void InitUsart_rs485_n1(uint32_t baudrate)
{
	GPIO_InitTypeDef gpio;
	USART_InitTypeDef usart;
	
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA, ENABLE );
	gpio.GPIO_OType = GPIO_OType_PP;
  gpio.GPIO_PuPd = GPIO_PuPd_DOWN;
  gpio.GPIO_Mode = GPIO_Mode_OUT;
	gpio.GPIO_Pin = GPIO_Pin_4;
  gpio.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &gpio);

	

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	
	gpio.GPIO_Mode = GPIO_Mode_AF;
  gpio.GPIO_OType = GPIO_OType_PP;
  gpio.GPIO_Pin = GPIO_Pin_6;
  gpio.GPIO_PuPd = GPIO_PuPd_UP;
  gpio.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &gpio);
	
	gpio.GPIO_Pin = GPIO_Pin_7;
  GPIO_Init(GPIOB, &gpio);
	
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);

  usart.USART_BaudRate = baudrate;
  usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  usart.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  usart.USART_Parity = USART_Parity_No;
  usart.USART_StopBits = USART_StopBits_1;
  usart.USART_WordLength = USART_WordLength_8b;
	
	USART_Init(USART1, &usart);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
  USART_ClearFlag(USART1, USART_FLAG_TC);

	NVIC_EnableIRQ(USART1_IRQn);
  USART_Cmd(USART1, ENABLE);
	return;
}






void set_rs485_config_v1(uint8_t *mc, uint8_t *addr, uint8_t *point)
{
	uint8_t i;
	
	for(i=0;i<20;i++)
	{
		RS485_ucntrl_n1.list_cntrl[i]=mc[i];
		RS485_ucntrl_n1.addr_cntrl[i]=addr[i];
		RS485_ucntrl_n1.point_write_value[i]=point[i];
	}
}
