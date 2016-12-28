/**
  ******************************************************************************
  * @file    main.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    31-July-2013
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4x7_eth.h"
#include "netconf.h"
#include "main.h"
#include "tcpip.h"
#include "stm32f4x7_eth_bsp.h"
#include "task.h"
#include "ff.h"

#include "sys_config.h"
#include "algblock.h"
#include "usart.h"
#include "udp_server.h"
#include <string.h>

/*--------------- Tasks Priority -------------*/
#define MAIN_TASK_PRIO   ( tskIDLE_PRIORITY + 1 )
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void Main_task(void * pvParameters);
void SystemInit_FmcSdram(void);
void tasktest(void * pvParameters);
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{
	uint32_t i;
  /* Configures the priority grouping: 4 bits pre-emption priority */
	  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
  /* Init task */
  xTaskCreate(Main_task,(int8_t *)"Main", configMINIMAL_STACK_SIZE * 1, NULL,tskIDLE_PRIORITY + 2, NULL);

  /* Start scheduler */
  vTaskStartScheduler();

  /* We should never get here as control is now taken by the scheduler */
  for( ;; );

}

/**
  * @brief  Main task
  * @param  pvParameters not used
  * @retval None
  */
void Main_task(void * pvParameters)
{
	volatile uint8_t * Ptr8;
	volatile uint16_t * Ptr16;
	int i;
	
	//rc_config_start();
	//uart_print( "\r\nStart STM32F417\r\n",strlen("\r\nStart STM32F417\r\n"));	
  /* configure Ethernet (GPIOs, clocks, MAC, DMA) */ 
  vTaskDelay(1000);
  ETH_BSP_Config();
	LwIP_Init();
	xTaskCreate(tasktest,(int8_t *)"tasktest", configMINIMAL_STACK_SIZE * 5, NULL,MAIN_TASK_PRIO, NULL);
  //Ptr8=(uint8_t *)0xC0000000;
	//for(i=0;i<1000;i++)
	//{
	//	*Ptr8=(uint8_t ) i;
	//	Ptr8++;
	//}
	
 // Ptr16=(uint16_t *)0xC0000000;
//for(i=0;i<16777216;i++)
//	{
//		*Ptr16=(uint16_t ) i;
//		Ptr16++;
//	}
	

  for( ;; )
  {
      vTaskDelete(NULL);
  }
}


void tasktest(void * pvParameters)
{
	FATFS   fscnf;
  FIL     filecnf;
  volatile FRESULT ferr;
	volatile uint8_t w1[20];
	volatile uint8_t w2[20];
	volatile uint8_t w3[20];
	volatile uint8_t str[60];
  for( ;; )
  {
		ferr = f_mount(0, &fscnf);
		strcpy((char*)w1,"0:/RC.CNF");
		ferr = f_open(&filecnf,(TCHAR*)w1,  FA_READ);
		if (f_gets((TCHAR*)str, sizeof(str), &filecnf)==0)
		{
			
		}
		f_close(&filecnf);
		vTaskDelay(1000);
  }
}

void vApplicationStackOverflowHook ( xTaskHandle pxTask, signed char *pcTaskName )
{
	 int i; 
	 signed char str[17];
	 for (i=0; i <16; i++)
   {
		 	str[i] = *pcTaskName++;
	 } 
}
/**
  * @brief  Inserts a delay time.
  * @param  nCount: number of Ticks to delay.
  * @retval None
  */
void Delay(uint32_t nCount) 
{
   //vTaskDelay(nCount);
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
