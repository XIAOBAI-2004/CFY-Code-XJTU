#include "delay.h"

static uint8_t  fac_us = 0;
static uint16_t fac_ms = 0;

void Delay_Init(void) {
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8); // 选择外部时钟  HCLK/8
    fac_us = SystemCoreClock / 8000000;                   // 1us需要的时钟数
    fac_ms = (uint16_t)fac_us * 1000;                    // 1ms需要的时钟数
}

void delay_us(uint32_t nus) {
    uint32_t temp;
    SysTick->LOAD = nus * fac_us;                         // 加载计数值
    SysTick->VAL = 0x00;                                  // 清空计数器
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;             // 启动计数器
    do {
        temp = SysTick->CTRL;
    } while((temp & 0x01) && !(temp & (1 << 16)));        // 等待时间到达
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;            // 关闭计数器
    SysTick->VAL = 0X00;                                  // 清空计数器
}

void delay_ms(uint32_t nms) {
    uint32_t temp;
    SysTick->LOAD = (uint32_t)nms * fac_ms;               // 加载计数值
    SysTick->VAL = 0x00;                                  // 清空计数器
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;             // 启动计数器
    do {
        temp = SysTick->CTRL;
    } while((temp & 0x01) && !(temp & (1 << 16)));        // 等待时间到达
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;            // 关闭计数器
    SysTick->VAL = 0X00;                                  // 清空计数器
}
