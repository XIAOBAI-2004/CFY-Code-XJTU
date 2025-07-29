#include "ws2812.h"
#include "./SYSTEM/delay/delay.h"

#define RESET_PULSES  80      // 50us 低电平复位脉冲（80 * 0.625us）

#ifdef USE_PWM_COMPRESSION
static uint16_t pwm_buffer[(24*LED_NUM + 80)/2]; // 压缩存储
#else
__attribute__((section(".ccmram"))) 
static uint16_t pwm_buffer[24*LED_NUM + 80];    // 完整存储到CCM RAM
#endif

//static uint16_t pwm_buffer[PWM_BITS * LED_NUM + RESET_PULSES]; // PWM 缓冲区
static RGB_Color led_colors[LED_NUM];                          // 颜色缓存

// PWM 占空比定义（24MHz 时钟，800kHz PWM 频率）
#define WS2812_0_PULSE  8   // 0 码：高电平 0.4us (8/24MHz)
#define WS2812_1_PULSE  16  // 1 码：高电平 0.8us (16/24MHz)

void WS2812_Init(void) {
    // 1. 使能时钟
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;  // 使能 GPIOA
    RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;   // 使能 TIM5
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;   // 使能 DMA1

    // 2. 配置 GPIO PA3（TIM5_CH4）为复用功能
    GPIOA->MODER   &= ~(0x3 << (3 * 2));      // 清除 PA3 模式位
    GPIOA->MODER   |=  (0x2 << (3 * 2));      // 复用功能模式（AF2）
    GPIOA->OSPEEDR |=  (0x3 << (3 * 2));      // 高速模式（100MHz）
    GPIOA->AFR[0]  &= ~(0xF << (3 * 4));      // 清除 AFRL3
    GPIOA->AFR[0]  |=  (0x2 << (3 * 4));      // AF2（TIM5_CH4）

    // 3. 配置 TIM5（24MHz 时钟，800kHz PWM）
    TIM5->CR1  = 0;                          // 禁用定时器
    TIM5->PSC  = 0;                          // 无分频（84MHz / (0+1) = 84MHz）
    TIM5->ARR  = 29;                         // PWM 周期 = (29+1)/84MHz ≈ 0.357us（实际调整见下文）
    TIM5->CCMR2 |= (0x6 << 4);               // PWM 模式 1（CH4）
    TIM5->CCER  |= TIM_CCER_CC4E;            // 使能 CH4 输出
    TIM5->CCR4  = 0;                         // 初始占空比为 0

    // 4. 配置 DMA1 Stream6（TIM5_CH4 触发）
    DMA1_Stream6->CR = 0;
    DMA1_Stream6->CR |= (6 << 25);           // Channel 6（TIM5_CH4）
    DMA1_Stream6->CR |= DMA_SxCR_DIR_0;      // 内存到外设
    DMA1_Stream6->CR |= DMA_SxCR_MINC;       // 内存地址递增
    DMA1_Stream6->CR |= DMA_SxCR_PSIZE_1;    // 外设数据大小：16 位（TIM_CCR4 是 16 位寄存器）
    DMA1_Stream6->CR |= DMA_SxCR_MSIZE_1;    // 内存数据大小：16 位
    DMA1_Stream6->CR |= DMA_SxCR_PL;         // 高优先级
    DMA1_Stream6->PAR  = (uint32_t)&TIM5->CCR4;  // 目标地址：TIM5 CCR4
    DMA1_Stream6->M0AR = (uint32_t)pwm_buffer;   // 源地址：PWM 缓冲区
    DMA1_Stream6->NDTR = sizeof(pwm_buffer) / 2; // 传输数据量（16 位为单位）

    // 5. 使能 DMA 请求
    TIM5->DIER |= TIM_DIER_CC4DE;            // 使能 CH4 DMA 请求
    TIM5->CR2  |= TIM_CR2_CCDS;              // DMA 请求选择比较事件

    // 6. 清除 DMA 标志
    DMA1->HIFCR = DMA_HIFCR_CTCIF6 | DMA_HIFCR_CHTIF6 | DMA_HIFCR_CTEIF6;

    // 初始化 LED 为关闭状态
    //WS2812_Clear();

}

void WS2812_SetColor(uint16_t index, RGB_Color color) {
    if(index >= LED_NUM) return;
    led_colors[index] = color;
}

void WS2812_SetAll(RGB_Color color) {
    for(uint16_t i = 0; i < LED_NUM; i++) {
        led_colors[i] = color;
    }
}

void WS2812_Update(void) {
    uint16_t *ptr = pwm_buffer;
    uint32_t color;
    
    // 1. 填充PWM缓冲区
    for(uint16_t i = 0; i < LED_NUM; i++) {
        color = ((uint32_t)led_colors[i].g << 16) | 
                ((uint32_t)led_colors[i].r << 8) | 
                led_colors[i].b;
        
        for(uint8_t j = 0; j < 24; j++) {
            *ptr++ = (color & (1 << (23 - j))) ? WS2812_1_PULSE : WS2812_0_PULSE;
        }
    }
    
    // 2. 填充复位信号(50us低电平)
    for(uint16_t i = 0; i < 80; i++) {
        *ptr++ = 0;
    }
    
    // 3. 启动DMA传输
    DMA1_Stream6->CR &= ~DMA_SxCR_EN;
    while(DMA1_Stream6->CR & DMA_SxCR_EN);
    
    DMA1->HIFCR = DMA_HIFCR_CTCIF6;
    DMA1_Stream6->NDTR = sizeof(pwm_buffer)/2;
    DMA1_Stream6->M0AR = (uint32_t)pwm_buffer;
    DMA1_Stream6->CR |= DMA_SxCR_EN;
    
    // 4. 启动TIM5
    TIM5->CR1 |= TIM_CR1_CEN;
    
    // 5. 等待传输完成
    while(!(DMA1->HISR & DMA_HISR_TCIF6));
    DMA1->HIFCR = DMA_HIFCR_CTCIF6;
    
    // 6. 停止TIM5
    TIM5->CR1 &= ~TIM_CR1_CEN;
}

void WS2812_Clear(void) {
    RGB_Color black = {0, 0, 0};
    WS2812_SetAll(black);
    WS2812_Update();
}
