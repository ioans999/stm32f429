#include "usart.h"

#include "stm32f4xx.h"

#include "stm32f4xx_gpio.h"
#include "stm32f4xx_dma.h"

#include "FreeRTOS.h"
#include "task.h"

#include "algblock.h"
#include "FreeRTOS.h"
#include "task.h"
#include "modbus.h"
#include "algdef.h"

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



void modbus_rx_IRQ_n4(uint8_t b_in)
{
	RS485_Modbus_n4.timeout=0;
	if (RS485_Modbus_n4.flag==0)
	{
		RS485_Modbus_n4.count_b=0;
		RS485_Modbus_n4.flag=1;
	}
	if (RS485_Modbus_n4.flag==1)
	{
		if (RS485_Modbus_n4.count_b >127)
		{
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



#define TX_BUFFER_SIZE 64
static uint8_t tx_buffer[TX_BUFFER_SIZE];
static uint8_t tx_rd_index,tx_counter;



void modbus_tx_IRQ_n4(void)
{
  if (tx_counter)
  {
    --tx_counter;
    USART_SendData(USART1, (uint8_t) tx_buffer[tx_rd_index]);
    tx_rd_index++;
  } else
  {
		tx_rd_index=0;
    GPIO_ResetBits(GPIOG, GPIO_Pin_12);
  }
}





void rs485_Send_n4(uint8_t *source, uint16_t size)
{
	uint16_t i;
	
	vTaskSuspendAll();
	{
		GPIO_SetBits(GPIOG, GPIO_Pin_12);
		tx_counter=0;
		//if (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
		USART_SendData(USART1, source[0]);
		for (i=1;i<size;i++)
		{
			tx_buffer[tx_counter]=source[i];
			tx_counter++;
		}
	}
	xTaskResumeAll();
}






void InitUsart_rs485_n4(uint32_t baudrate)
{
	GPIO_InitTypeDef gpio;
	USART_InitTypeDef usart;
	
	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOG, ENABLE );
	gpio.GPIO_OType = GPIO_OType_PP;
  gpio.GPIO_PuPd = GPIO_PuPd_DOWN;
  gpio.GPIO_Mode = GPIO_Mode_OUT;
	gpio.GPIO_Pin = GPIO_Pin_12;
  gpio.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOG, &gpio);

	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	
	
	gpio.GPIO_Mode = GPIO_Mode_AF;
  gpio.GPIO_OType = GPIO_OType_PP;
  gpio.GPIO_Pin = GPIO_Pin_6;
  gpio.GPIO_PuPd = GPIO_PuPd_UP;
  gpio.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &gpio);
	
	gpio.GPIO_Pin = GPIO_Pin_7;
  GPIO_Init(GPIOB, &gpio);
	
	gpio.GPIO_Pin = GPIO_Pin_9;
  GPIO_Init(GPIOA, &gpio);
	
	gpio.GPIO_Pin = GPIO_Pin_10;
  GPIO_Init(GPIOA, &gpio);

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);
	GPIO_PinAFConfig( GPIOA, GPIO_PinSource9, GPIO_AF_USART1 );



  usart.USART_BaudRate = baudrate;
  usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  usart.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  usart.USART_Parity = USART_Parity_No;
  usart.USART_StopBits = USART_StopBits_1;
  usart.USART_WordLength = USART_WordLength_8b;
	
	USART_Init(USART1, &usart);
	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);	
	USART_ITConfig(USART1, USART_IT_TC, ENABLE);

	NVIC_EnableIRQ(USART1_IRQn);

  USART_Cmd(USART1, ENABLE);
}



void uart_print(char *str, uint8_t ln)
{
  //volatile int i = 0;

  //for(i=0;i<ln;i++)
  {
		//while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
		  //USART_SendData(USART1, (uint8_t) str[i]);
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



