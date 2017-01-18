/**
  ******************************************************************************
  * @file    stm32f4xx_it.c 
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    31-July-2013
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
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
#include "stm32f4xx_it.h"
#include "main.h"
#include "stm32f4x7_eth.h"
#include "sdio_sd.h"

/* Scheduler includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* lwip includes */
#include "lwip/sys.h"

#include "usart.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern xSemaphoreHandle s_xSemaphore;
extern xSemaphoreHandle ETH_link_xSemaphore;
/* Private function prototypes -----------------------------------------------*/
extern void xPortSysTickHandler(void); 
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function andles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
  xPortSysTickHandler(); 
}

/**
  * @brief  This function handles External line 10 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI15_10_IRQHandler(void)
{
  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

  if(EXTI_GetITStatus(ETH_LINK_EXTI_LINE) != RESET)
  {
    /* Give the semaphore to wakeup LwIP task */
    xSemaphoreGiveFromISR(ETH_link_xSemaphore, &xHigherPriorityTaskWoken ); 
  }
  /* Clear interrupt pending bit */
  EXTI_ClearITPendingBit(ETH_LINK_EXTI_LINE);

  /* Switch tasks if necessary. */	
  if( xHigherPriorityTaskWoken != pdFALSE )
  {
    portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
  }
}

/**
  * @brief  This function handles ethernet DMA interrupt request.
  * @param  None
  * @retval None
  */
void ETH_IRQHandler(void)
{
  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

  /* Frame received */
  if ( ETH_GetDMAFlagStatus(ETH_DMA_FLAG_R) == SET) 
  {
    /* Give the semaphore to wakeup LwIP task */
    xSemaphoreGiveFromISR( s_xSemaphore, &xHigherPriorityTaskWoken );   
  }

  /* Clear the interrupt flags. */
  /* Clear the Eth DMA Rx IT pending bits */
  ETH_DMAClearITPendingBit(ETH_DMA_IT_R);
  ETH_DMAClearITPendingBit(ETH_DMA_IT_NIS);

  /* Switch tasks if necessary. */	
  if( xHigherPriorityTaskWoken != pdFALSE )
  {
    portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
  }
}


void SDIO_IRQHandler(void)
{
  /* Process All SDIO Interrupt Sources */
  SD_ProcessIRQSrc();
}


void DMA2_Stream3_IRQHandler(void)//SD_SDIO_DMA_IRQHANDLER
{
  SD_ProcessDMAIRQ();
} 

/**************************************************
******************    USART
**************************************************/


void DMA2_Stream7_IRQHandler(void)
{ 
  if(DMA_GetITStatus(DMA2_Stream7, DMA_IT_TCIF7)!= RESET)
  {
		//RS-485  #3
		if ((DMA2_Stream7->CR & DMA_SxCR_CHSEL_0) && !(DMA2_Stream7->CR & DMA_SxCR_CHSEL_1) && (DMA2_Stream7->CR & DMA_SxCR_CHSEL_2))
		{
			USART_DMACmd(USART6, USART_DMAReq_Tx, DISABLE);
			DMA_ITConfig(DMA2_Stream7, DMA_IT_TC, DISABLE);
			DMA_Cmd(DMA2_Stream7, DISABLE);
			USART_ITConfig(USART6, USART_IT_TC, ENABLE);
			DMA_ClearITPendingBit(DMA2_Stream7,DMA_IT_TC);
		}
		//RS-485  #1
		if (!(DMA2_Stream7->CR & DMA_SxCR_CHSEL_0) && !(DMA2_Stream7->CR & DMA_SxCR_CHSEL_1) && (DMA2_Stream7->CR & DMA_SxCR_CHSEL_2))
		{
			USART_DMACmd(USART6, USART_DMAReq_Tx, DISABLE);
			DMA_ITConfig(DMA2_Stream7, DMA_IT_TC, DISABLE);
			DMA_Cmd(DMA2_Stream7, DISABLE);
			USART_ITConfig(USART1, USART_IT_TC, ENABLE);
			DMA_ClearITPendingBit(DMA2_Stream7,DMA_IT_TC);
		}
  }
}

/****************************************
  RS-485  #1
*/
void DMA1_Stream7_IRQHandler(void)
{ 
  if(DMA_GetITStatus(DMA1_Stream6, DMA_IT_TCIF6)!= RESET)
  {
		DMA_ITConfig(DMA1_Stream6,DMA_IT_TC,DISABLE );
    DMA_Cmd(DMA1_Stream6, DISABLE);	
		USART_ITConfig(USART2, USART_IT_TC, ENABLE);	
		DMA_ClearITPendingBit(DMA1_Stream6,DMA_IT_TC);
  }
}

void USART1_IRQHandler(void)
{
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
		modbus_IRQ_n2(USART1->DR);
	}
	if (USART_GetITStatus(USART1, USART_IT_TC) != RESET)
	{
		USART_ClearITPendingBit(USART1, USART_IT_TC);
		USART_time_reset_n1();
		GPIO_ResetBits(GPIOA, GPIO_Pin_4);
		USART_ITConfig(USART1, USART_IT_TC, DISABLE);
	}
}
/****************************************
  RS-485  #2
*/
void DMA1_Stream6_IRQHandler(void)
{ 
  if(DMA_GetITStatus(DMA1_Stream6, DMA_IT_TCIF6)!= RESET)
  {
		DMA_ITConfig(DMA1_Stream6,DMA_IT_TC,DISABLE );
    DMA_Cmd(DMA1_Stream6, DISABLE);	
		USART_ITConfig(USART2, USART_IT_TC, ENABLE);	
		DMA_ClearITPendingBit(DMA1_Stream6,DMA_IT_TC);
  }
}

void USART2_IRQHandler(void)
{
	if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
	{
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
		modbus_IRQ_n2(USART2->DR);
	}
	if (USART_GetITStatus(USART2, USART_IT_TC) != RESET)
	{
		USART_ClearITPendingBit(USART2, USART_IT_TC);
		USART_time_reset_n2();
		GPIO_ResetBits(GPIOG, GPIO_Pin_9);
		USART_ITConfig(USART2, USART_IT_TC, DISABLE);
	}
}


/****************************************
  RS-485  #3
*/


void USART6_IRQHandler(void)
{
	uint32_t i;
	
	if (USART_GetITStatus(USART6, USART_IT_RXNE) != RESET)
	{
		USART_ClearITPendingBit(USART6, USART_IT_RXNE);
		modbus_IRQ_n3(USART6->DR);
	}
	if (USART_GetITStatus(USART6, USART_IT_TC) != RESET)
	{
		USART_ClearITPendingBit(USART6, USART_IT_TC);
		GPIO_ResetBits(GPIOH, GPIO_Pin_7);
		USART_ITConfig(USART6, USART_IT_TC, DISABLE);
	}
}


/****************************************
  RS-485  #4
*/
void DMA1_Stream1_IRQHandler(void)
{ 
  if(DMA_GetITStatus(DMA1_Stream1, DMA_IT_TCIF1)!= RESET)
  {
		USART_DMACmd(UART7, USART_DMAReq_Tx, DISABLE);
		DMA_ITConfig(DMA1_Stream1, DMA_IT_TC, DISABLE);
    DMA_Cmd(DMA1_Stream1, DISABLE);
		USART_ITConfig(UART7, USART_IT_TC, ENABLE);
		DMA_ClearITPendingBit(DMA1_Stream1,DMA_IT_TC);
  }
}


void UART7_IRQHandler(void)
{
	uint32_t i;
	
	if (USART_GetITStatus(UART7, USART_IT_RXNE) != RESET)
	{
		USART_ClearITPendingBit(UART7, USART_IT_RXNE);
		modbus_IRQ_n4(UART7->DR);
	}
	if (USART_GetITStatus(UART7, USART_IT_TC) != RESET)
	{
		USART_ClearITPendingBit(UART7, USART_IT_TC);
		GPIO_ResetBits(GPIOF, GPIO_Pin_8);
		USART_ITConfig(UART7, USART_IT_TC, DISABLE);
	}
}



/******************************
RS-485  TIMER
*/
void TIM6_DAC_IRQHandler(void)
{
	static uint8_t tc;
	
	if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET) 
	{
		timer_IRQ_n1();
		timer_IRQ_n2();
		timer_IRQ_n3();
		timer_IRQ_n4();
		if (tc > 9)
		{
			//AlgCicleTimeInc();
			tc=0;
		} tc++;
	  TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
  }
}

/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/
/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
