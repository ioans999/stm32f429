#include "usart.h"

#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_usart.h"

#include "algblock.h"
#include "FreeRTOS.h"
#include "task.h"
#include "modbus.h"
#include "algdef.h"

#include <stdlib.h>
#include <string.h>

extern xSemaphoreHandle xBinarySemaphoreUSART4StartALG;

struct	{
	uint8_t list_cntrl[NUBER_PCB_IN_OUT];
	uint8_t addr_cntrl[NUBER_PCB_IN_OUT];
	uint32_t point_write_value[NUBER_PCB_IN_OUT];
}	RS485_ucntrl_n4;

struct	{
	uint8_t data[128];
	uint8_t count_b;
	uint32_t timeout;
	uint8_t flag;
} RS485_Modbus_n4;


void v_rs485_task_n4(void * pvParameters)
{
  uint32_t all_tx=0, all_rx=0, current_lost=0;
  uint8_t ln=0;	
	uint32_t i=0,count_mc=0;
	uint8_t bf[32];
	
	
  vTaskDelay(50 / portTICK_RATE_MS );
  while(1)
	{
		current_lost = all_tx - all_rx;
		for (count_mc=0; count_mc < NUBER_PCB_IN_OUT; count_mc++)
		{		
			if (RS485_ucntrl_n4.list_cntrl[count_mc]>0)
			{			
        if (all_tx >= 0XFFFFFF00) {
					all_tx=0;
					all_rx=0;
					current_lost = 0;
				}								
				/////////////////////Send packet
				//Get Discret for do16
				if (RS485_ucntrl_n4.list_cntrl[count_mc]==11)
				{
					bf[5]=get_mc_DO_byte_from_buf(RS485_ucntrl_n4.point_write_value[count_mc]);
					bf[6]=get_mc_DO_byte_from_buf(RS485_ucntrl_n4.point_write_value[count_mc]+8);
					for(i=0; i<16; i++)
					{
						uint16_t ao_v;
						ao_v = get_mc_AO_byte_from_buf(RS485_ucntrl_n4.point_write_value[count_mc]);
						bf[7+i*2] = ao_v & 0xFF;
						bf[8+i*2] = (ao_v >> 8) & 0xFF;
					}
				}
				RS485_Modbus_n4.flag=0;
				ln=Create_mb_packet(bf, RS485_ucntrl_n4.list_cntrl[count_mc], RS485_ucntrl_n4.addr_cntrl[count_mc]);
			  rs485_Send_n4(bf,ln);
		    all_tx++;
				
				/////////////////////Recive packet
				for (i=0; i<125; i++)
				{
					vTaskDelay(2 / portTICK_RATE_MS );
					if(RS485_Modbus_n4.flag==3)
					{
						break;
					}
				}
				if(RS485_Modbus_n4.flag==3)
				{
					if (Chk_mb_packet(RS485_Modbus_n4.data, RS485_ucntrl_n4.list_cntrl[count_mc], RS485_ucntrl_n4.addr_cntrl[count_mc], RS485_Modbus_n4.count_b) ==0)
					{
						all_rx++;
						//Write Analog to buff from ad16
						if (RS485_ucntrl_n4.list_cntrl[count_mc]==1){
							write_mc_value_to_buf(RS485_Modbus_n4.data+3, RS485_ucntrl_n4.point_write_value[count_mc], 0, 16, 1);
						}
						//Write Analog to buff from ai200
						if (RS485_ucntrl_n4.list_cntrl[count_mc]==2){
							write_mc_value_to_buf(RS485_Modbus_n4.data+4, RS485_ucntrl_n4.point_write_value[count_mc], 0, 12, 1);
						}
						//Write Discret to buff from dd16
						if (RS485_ucntrl_n4.list_cntrl[count_mc]==3){
							write_mc_value_to_buf(RS485_Modbus_n4.data+3, RS485_ucntrl_n4.point_write_value[count_mc], 0, 16, 2);
						}
						if (RS485_ucntrl_n4.list_cntrl[count_mc]==11){
							write_mc_value_to_buf(RS485_Modbus_n4.data+3, RS485_ucntrl_n4.point_write_value[count_mc], 0, 16, 3);
							write_mc_value_to_buf(RS485_Modbus_n4.data+5, RS485_ucntrl_n4.point_write_value[count_mc], 0, 16, 4);
						}
					}
					else
					{
						//Write Analog lost to buff from ad16
						if (RS485_ucntrl_n4.list_cntrl[count_mc]==1){
							write_mc_value_lost_to_buf(RS485_ucntrl_n4.point_write_value[count_mc],16,1);
						}
						//Write Analog lost to buff from ai200
						if (RS485_ucntrl_n4.list_cntrl[count_mc]==2){
							write_mc_value_lost_to_buf(RS485_ucntrl_n4.point_write_value[count_mc],12,1);
						}
						//Write Discret lost to buff from dd16
						if (RS485_ucntrl_n4.list_cntrl[count_mc]==3){
							write_mc_value_lost_to_buf(RS485_ucntrl_n4.point_write_value[count_mc],16,2);
						}
						if (RS485_ucntrl_n4.list_cntrl[count_mc]==11){
							write_mc_value_lost_to_buf(RS485_ucntrl_n4.point_write_value[count_mc],16,3);
							write_mc_value_lost_to_buf(RS485_ucntrl_n4.point_write_value[count_mc],16,4);
						}
					}
		 	  }
				else
				{
					//Write Analog lost to buff from ad16
					if (RS485_ucntrl_n4.list_cntrl[count_mc]==1){
						write_mc_value_lost_to_buf(RS485_ucntrl_n4.point_write_value[count_mc],16,1);
					}
					//Write Analog lost to buff from ai200
					if (RS485_ucntrl_n4.list_cntrl[count_mc]==2){
						write_mc_value_lost_to_buf(RS485_ucntrl_n4.point_write_value[count_mc],12,1);
					}
					//Write Discret lost to buff from dd16
					if (RS485_ucntrl_n4.list_cntrl[count_mc]==3){
						write_mc_value_lost_to_buf(RS485_ucntrl_n4.point_write_value[count_mc],16,2);
					}
					//Write Discret lost to buff from ad16
					if (RS485_ucntrl_n4.list_cntrl[count_mc]==11){
						write_mc_value_lost_to_buf(RS485_ucntrl_n4.point_write_value[count_mc],16,3);
						write_mc_value_lost_to_buf(RS485_ucntrl_n4.point_write_value[count_mc],16,4);
					}
				}
				vTaskDelay(2 / portTICK_RATE_MS );
			}
		}
		write_modbas_inf(all_tx,1,4);
		write_modbas_inf(all_rx,2,4);
		
		
		if ((all_tx - all_rx) > current_lost)
		{
			AlgLostModbusPacketU4(1);
		} else
		{
			AlgLostModbusPacketU4(0);
		}
		xSemaphoreGive(xBinarySemaphoreUSART4StartALG);
		vTaskDelay(4 / portTICK_RATE_MS );
	}
}




void timer_IRQ_n4(void)
{
  RS485_Modbus_n4.timeout++;
	if ((RS485_Modbus_n4.timeout>11 )&&(RS485_Modbus_n4.flag==1))
		RS485_Modbus_n4.flag=2;
	if ((RS485_Modbus_n4.timeout>22) && (RS485_Modbus_n4.flag==2))
		RS485_Modbus_n4.flag=3;
}





void modbus_IRQ_n4(uint8_t b_in)
{
	RS485_Modbus_n4.timeout=0;
	if (RS485_Modbus_n4.flag==0)
	{
		RS485_Modbus_n4.count_b=0;
		RS485_Modbus_n4.flag=1;
	}
	if (RS485_Modbus_n4.flag==1)
	{
		if (RS485_Modbus_n4.count_b >127) {
			RS485_Modbus_n4.count_b=0;
		}
    RS485_Modbus_n4.data[RS485_Modbus_n4.count_b] = b_in;
    RS485_Modbus_n4.count_b++;
	}
	if (RS485_Modbus_n4.flag==2)
	{
		RS485_Modbus_n4.flag=4;
	}
}


void rs485_Send_n42(uint8_t *source, uint16_t size)
{
  volatile int i = 0;
  GPIO_SetBits(GPIOF, GPIO_Pin_8);
  for(i=0;i<size;i++)
  {
		while(USART_GetFlagStatus(UART7, USART_FLAG_TXE) == RESET);
		  USART_SendData(UART7, (uint8_t) source[i]);
  }

} 


void rs485_Send_n4(uint8_t *source, uint16_t size)
{
	 DMA_InitTypeDef dma;
	 NVIC_InitTypeDef dma_nvic;
	
	 GPIO_SetBits(GPIOF, GPIO_Pin_8);
   
   DMA_DeInit(DMA1_Stream1);
	
	 dma.DMA_BufferSize = size;
   dma.DMA_Channel = DMA_Channel_5;
   dma.DMA_DIR = DMA_DIR_MemoryToPeripheral;
   dma.DMA_FIFOMode = DMA_FIFOMode_Disable;
   dma.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
   dma.DMA_Memory0BaseAddr = (uint32_t)source;
   dma.DMA_MemoryBurst = DMA_MemoryBurst_Single;
   dma.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
   dma.DMA_MemoryInc = DMA_MemoryInc_Enable;
   dma.DMA_Mode = DMA_Mode_Normal;
   dma.DMA_PeripheralBaseAddr = (uint32_t)&(UART7->DR);
   dma.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
   dma.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
   dma.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
   dma.DMA_Priority = DMA_Priority_Low;

   dma_nvic.NVIC_IRQChannel = DMA1_Stream1_IRQn;
   dma_nvic.NVIC_IRQChannelCmd = ENABLE;
   dma_nvic.NVIC_IRQChannelPreemptionPriority = 1;
   dma_nvic.NVIC_IRQChannelSubPriority = 0;
   DMA_Init(DMA1_Stream1, &dma);
	 DMA_ITConfig(DMA1_Stream1, DMA_IT_TC, ENABLE);
	 NVIC_EnableIRQ(DMA1_Stream1_IRQn);
	 
	 
	 DMA_Cmd(DMA1_Stream1, ENABLE);
	 
   NVIC_Init(&dma_nvic);
	 USART_ClearFlag(UART7, USART_FLAG_TC);
   USART_DMACmd(UART7, USART_DMAReq_Tx, ENABLE);

   return;
}






void InitUsart_rs485_n4(uint32_t baudrate)
{
	GPIO_InitTypeDef gpio;
	USART_InitTypeDef usart;
	
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOF, ENABLE );
	gpio.GPIO_OType = GPIO_OType_PP;
  gpio.GPIO_PuPd = GPIO_PuPd_DOWN;
  gpio.GPIO_Mode = GPIO_Mode_OUT;
	gpio.GPIO_Pin = GPIO_Pin_8;
  gpio.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOF, &gpio);

	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART7, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	
	gpio.GPIO_Mode = GPIO_Mode_AF;
  gpio.GPIO_OType = GPIO_OType_PP;
  gpio.GPIO_Pin = GPIO_Pin_6;
  gpio.GPIO_PuPd = GPIO_PuPd_UP;
  gpio.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOF, &gpio);
	
	gpio.GPIO_Pin = GPIO_Pin_7;
  GPIO_Init(GPIOF, &gpio);
	
	GPIO_PinAFConfig(GPIOF, GPIO_PinSource6, GPIO_AF_UART7);
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource7, GPIO_AF_UART7);

  usart.USART_BaudRate = baudrate;
  usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  usart.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  usart.USART_Parity = USART_Parity_No;
  usart.USART_StopBits = USART_StopBits_1;
  usart.USART_WordLength = USART_WordLength_8b;
	
	USART_Init(UART7, &usart);
	USART_ITConfig(UART7, USART_IT_RXNE, ENABLE);
	USART_ClearFlag(UART7, USART_FLAG_TC);
	
	NVIC_EnableIRQ(UART7_IRQn);
  USART_Cmd(UART7, ENABLE);
}



void uart_print(char *str, uint8_t ln)
{
  volatile int i = 0;

  for(i=0;i<ln;i++)
  {
	//	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	//	  USART_SendData(USART1, (uint8_t) str[i]);
  }

} 




void set_rs485_config_v4(uint8_t *mc, uint8_t *addr, uint8_t *point)
{
	uint8_t i;
	
	for(i=0;i<NUBER_PCB_IN_OUT;i++)
	{
		RS485_ucntrl_n4.list_cntrl[i]=mc[i];
		RS485_ucntrl_n4.addr_cntrl[i]=addr[i];
		RS485_ucntrl_n4.point_write_value[i]=point[i];
	}
}



