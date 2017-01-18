#ifndef _ALGBLOCK_H_
#define _ALGBLOCK_H_
#include "stm32f4xx.h"


void vAlg_block_Task(void * pvParameters);

void write_modbas_inf(uint32_t val, uint8_t t, uint8_t nm);
void write_mc_value(uint8_t *val, uint32_t p,uint8_t ln,uint8_t m);
uint8_t  get_mc_DO_byte(uint8_t  n);
uint16_t  get_mc_AO_byte(uint8_t  n);

void write_mc_value_to_buf(uint8_t *val, uint32_t p, uint32_t s,uint8_t ln,uint8_t m);
void write_mc_value_lost_to_buf(uint32_t p,uint8_t ln,uint8_t m);

void AlgCicleTimeInc(void);
void AlgLostModbusPacket(void);
void AlgLostModbusPacketU4(uint8_t n);
void WriteCnfAdcFloat(uint16_t num, float a, float b);
#endif
