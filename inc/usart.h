#ifndef __usart_n2_h
#define __usart_n2_h
#include "stm32f4xx.h"

#define NUBER_PCB_IN_OUT 20

void rs485_DMASend_n1(uint8_t *source, uint16_t size,uint8_t cut_after_newline);
void InitUsart_rs485_n1(uint32_t baudrate);
void v_rs485_task_n1(void * pvParameters);
void modbus_IRQ_n1(uint8_t b_in);
void timer_IRQ_n1(void );
void USART_time_reset_n1(void);
void set_rs485_config_v1(uint8_t *mc, uint8_t *addr, uint8_t *point);



void rs485_DMASend_n2(uint8_t *source, uint16_t size,uint8_t cut_after_newline);
void InitUsart_rs485_n2(uint32_t baudrate);
void v_rs485_task_n2(void * pvParameters);
void modbus_IRQ_n2(uint8_t b_in);
void timer_IRQ_n2(void );
void USART_time_reset_n2(void);
void set_rs485_config_v2(uint8_t *mc, uint8_t *addr, uint8_t *point);


void InitTIM6(void);
void rs485_DMASend_n3(uint8_t *source, uint8_t size,uint8_t cut_after_newline);
void InitUsart_rs485_n3(uint32_t baudrate);
void v_rs485_task_n3(void * pvParameters);
void modbus_IRQ_n3(uint8_t b_in);
void timer_IRQ_n3(void );
void USART_time_reset_n3(void);
void set_rs485_config_v3(uint8_t *mc, uint8_t *addr, uint8_t *point);


void rs485_Send_n4(uint8_t *source, uint16_t size);
void InitUsart_rs485_n4(uint32_t baudrate);
void v_rs485_task_n4(void * pvParameters);
void timer_IRQ_n4(void );
void uart_print(char *str, uint8_t ln);
void modbus_rx_IRQ_n4(uint8_t b_in);
void modbus_tx_IRQ_n4(void);
void set_rs485_config_v4(uint8_t *mc, uint8_t *addr, uint8_t *point);

#endif
