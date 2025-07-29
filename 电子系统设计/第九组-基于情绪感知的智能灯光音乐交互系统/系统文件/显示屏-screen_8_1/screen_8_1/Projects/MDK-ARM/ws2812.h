#ifndef __WS2812_H
#define __WS2812_H

#include "stm32f4xx.h"

#define LED_NUM 60
#define USE_PWM_COMPRESSION  // 启用压缩存储
#define PWM_BITS 24     // 每个LED的PWM位数(24bit GRB)

typedef struct {
    uint8_t g;  // WS2812是GRB顺序
    uint8_t r;
    uint8_t b;
} RGB_Color;

void WS2812_Init(void);
void WS2812_SetColor(uint16_t index, RGB_Color color);
void WS2812_SetAll(RGB_Color color);
void WS2812_Update(void);
void WS2812_Clear(void);

#endif
