#ifndef __KEY_H
#define __KEY_H

#include "stm32f10x.h"

#define KEY_PORT GPIOA
#define KEY_PIN  GPIO_Pin_0
#define KEY_RCC  RCC_APB2Periph_GPIOA

void KEY_Init(void);
uint8_t KEY_Scan(uint8_t mode);

#endif
