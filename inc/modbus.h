#ifndef __modbus_h
#define __modbus_h

#include "stm32f4xx.h"

uint8_t Chk_mb_packet(uint8_t* buf, uint8_t type, uint8_t addr, uint8_t ln);
uint8_t Create_mb_packet(uint8_t* buf, uint8_t type, uint8_t addr);
uint16_t  Chk_CRC(uint8_t* buf,uint8_t len);

#endif
