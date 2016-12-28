#ifndef _UDP_SERVER_H_
#define _UDP_SERVER_H_

#include "stm32f4xx.h"

void vUDP_serv_recv(void * pvParameters);
uint32_t UDP_send(uint8_t *bf, uint16_t ln);
void vUDP_serv_send(void * pvParameters);
void SetDestPort(uint16_t p);

#endif
