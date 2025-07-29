#ifndef __WS2812_H
#define __WS2812_H

#include "stm32f10x.h"

#define LED_NUM 60  // 灯珠数量

typedef struct {
    uint8_t g;  // 绿色
    uint8_t r;  // 红色
    uint8_t b;  // 蓝色
} RGB_Color;

void WS2812_Init(void);
void WS2812_SetColor(uint16_t index, RGB_Color color);
void WS2812_SetAll(RGB_Color color);
void WS2812_Update(void);
void WS2812_Clear(void);

#endif
