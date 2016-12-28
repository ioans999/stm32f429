/*sys_config.h*/
#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#include "stm32f4xx.h"

uint8_t get_mac_addrs_cnf(uint8_t numb_mac);
uint8_t rc_config_start(void);
uint8_t read_mac_addr(char* str,char *MAC_addr);
uint8_t read_ip4_addr(char* str,uint8_t *ip);
 
#endif
