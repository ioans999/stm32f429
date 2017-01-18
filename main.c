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
#include "ftp_server.h"

#include <string.h>
#include <math.h>


xSemaphoreHandle xMutex_ALGBLOC_RW_DataVar;
xSemaphoreHandle xMutex_ALGBLOC_RW_DataVar_Buf;

void Main_task(void * pvParameters);
void tasktest(void * pvParameters);

int main(void)
{
	xMutex_ALGBLOC_RW_DataVar = xSemaphoreCreateMutex();
	xMutex_ALGBLOC_RW_DataVar_Buf = xSemaphoreCreateMutex();

  /* Init task */
  xTaskCreate(Main_task,(int8_t *)"Main", configMINIMAL_STACK_SIZE * 12, NULL,tskIDLE_PRIORITY + 2, NULL);

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
	portBASE_TYPE err_task;
	
	uart_print( "\r\nStart STM32F429\r\n",strlen("\r\nStart STM32F429\r\n"));	
	
		/* configure global parameters (usart, IP, MAC, ADC converter)*/ ;
	rc_config_start();

  /* configure Ethernet (GPIOs, clocks, MAC, DMA) */ ;
  ETH_BSP_Config();
	
	LwIP_Init();
	
	//xTaskCreate(tasktest,(int8_t *)"tasktest", configMINIMAL_STACK_SIZE * 8, NULL, tskIDLE_PRIORITY + 1, NULL);
	
	err_task = xTaskCreate(vBasicFTPServer, "vBasicFTPServ", configMINIMAL_STACK_SIZE * 10, NULL, 8, NULL);
	if (err_task == pdTRUE)  
		uart_print( "Create vBasicFTPServer task\r\n",strlen("Create vBasicFTPServer task\r\n"));
  else 
		uart_print( "Error create vBasicFTPServer task\r\n",strlen("Error create vBasicFTPServer task\r\n"));
	
	err_task = xTaskCreate(v_rs485_task_n1, "rs485_task_n1", configMINIMAL_STACK_SIZE * 6, NULL, 6, NULL);
	if (err_task == pdTRUE)  
		 uart_print( "Create rs485 n1 task\r\n", strlen("Create rs485 n1 task\r\n"));
  else 
	 	 uart_print( "Error create rs485 n1 task\r\n", strlen("Error create rs485 n1 task\r\n"));		
	
	err_task = xTaskCreate(v_rs485_task_n2, "rs485_task_n2", configMINIMAL_STACK_SIZE * 2, NULL, 6, NULL);
	if (err_task == pdTRUE)  
		 uart_print( "Create rs485 n2 task\r\n", strlen("Create rs485 n2 task\r\n"));
  else 
	 	 uart_print( "Error create rs485 n2 task\r\n", strlen("Error create rs485 n2 task\r\n"));	

	err_task = xTaskCreate(v_rs485_task_n3, "rs485_task_n3", configMINIMAL_STACK_SIZE * 2, NULL, 6, NULL);
	if (err_task == pdTRUE)  
		 uart_print( "Create rs485 n3 task\r\n", strlen("Create rs485 n3 task\r\n"));
  else 
	 	 uart_print( "Error create rs485 n3 task\r\n", strlen("Error create rs485 n3 task\r\n"));	
	/*
	err_task = xTaskCreate(v_rs485_task_n4, "rs485_task_n4", configMINIMAL_STACK_SIZE * 6, NULL, 5, NULL);
	if (err_task == pdTRUE)  
		 uart_print( "Create rs485 n4 task\r\n", strlen("Create rs485 n4 task\r\n"));
  else 
	 	 uart_print( "Error create rs485 n4 task\r\n", strlen("Error create rs485 n4 task\r\n"));		
	*/
	err_task = xTaskCreate(vAlg_block_Task, "Star Algoblock Task", configMINIMAL_STACK_SIZE * 4, NULL, 4, NULL);
	if (err_task == pdTRUE)  
		uart_print( "Create Algoblock task\r\n", strlen("Create Algoblock task\r\n"));
  else 
		uart_print( "Error create vBasicFTPServer task\r\n", strlen("Error create vBasicFTPServer task\r\n"));
	
  //Ptr16=(uint16_t *)0xC0000000;
	for( ;; )
  {
		vTaskDelete(NULL);
  }
}

	#define SIZE_PACKEG  1400
	#define SIZE_COUNT  10
void tasktest(void * pvParameters)
{
	FATFS   fscnf;
  FIL     filecnf;
  FRESULT ferr; 
	uint8_t bufw[SIZE_PACKEG];
	uint8_t bufr[SIZE_PACKEG];
	uint32_t n,i,ln,rlen,count;
  volatile uint8_t err = 0; 
  uint8_t file[] = "0:/test2.txt";
	
		
	for( ;; ) vTaskDelay(100);
  /*{
		count++;
		for (i = 0; i < SIZE_PACKEG; i++)
		{
			bufw[i]='0';
		}
		ferr = f_mount(0, &fscnf);
		ferr = f_open(&filecnf,(TCHAR*)file,  FA_WRITE| FA_CREATE_ALWAYS);
		if (ferr != FR_OK) {		
			f_close(&filecnf);
			f_mount(0, NULL);
			vTaskDelay(100);
			err = 1;
			break;
		}
		for (i=0;i<SIZE_COUNT;i++)
		{
			ferr = f_write(&filecnf, bufw, SIZE_PACKEG, &ln);
			if (ferr != FR_OK ) {
				f_close(&filecnf);
				f_mount(0, NULL);
				vTaskDelay(100);
				err = 2;
				break;
			}
			if (ln < SIZE_PACKEG) err = 3;
		}
		f_close(&filecnf);
		
		if (err > 0)
		{
			vTaskDelay(1);
			break;
		}		
		
		ferr = f_open(&filecnf,(TCHAR*)file,  FA_READ );
		if (ferr != FR_OK) {		
				f_close(&filecnf);
				f_mount(0, NULL);
				vTaskDelay(100);
				err = 1;
		}
		rlen = 0;
		for (i=0;i<SIZE_COUNT + 1000;i++)
		{
			ferr = f_read(&filecnf, bufr, SIZE_PACKEG, &ln);
			if (ferr != FR_OK) {
				f_close(&filecnf);
				f_mount(0, NULL);
				vTaskDelay(100);
				err = 1;
				break;
			}
			rlen += ln;
			for (n=0; n<ln; n++)
			{
				if (bufr[n] != '0')
				{
					err = 2;
					break;
				}
			}
			if (err == 2) break;
			if (ln < SIZE_PACKEG)
			{
				if (rlen == SIZE_PACKEG*SIZE_COUNT)
				{
					err = 0;
					break;
				}
				err = 3;
				break;
			}
			err = 4;
		}
		f_close(&filecnf);
		f_mount(0, NULL);
		
		if (err > 0)
		{
			vTaskDelay(1);
		}
  }*/
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
   vTaskDelay(nCount);
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
