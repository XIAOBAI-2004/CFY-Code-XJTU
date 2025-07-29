#include "ws2812.h"
#include "delay.h"

#define WS2812_RESET_PULSE 50  // 复位脉冲时间(us)
#define WS2812_0_CODE 0xC0     // '0'码 (SPI 8位数据)
#define WS2812_1_CODE 0xF8     // '1'码 (SPI 8位数据)

static uint8_t ws2812_buffer[24 * LED_NUM + 32]; // DMA发送缓冲区
static RGB_Color led_colors[LED_NUM];            // 颜色数据缓存

void WS2812_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef SPI_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO | RCC_APB2Periph_SPI1, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    
    // 配置SPI引脚
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;  // MOSI(PA7)
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // SPI配置
    SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;  // 9MHz @72MHz
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &SPI_InitStructure);
    
    // DMA配置
    DMA_DeInit(DMA1_Channel3);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&SPI1->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ws2812_buffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = sizeof(ws2812_buffer);
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel3, &DMA_InitStructure);
    
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
    SPI_Cmd(SPI1, ENABLE);
    
    WS2812_Clear();
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
    uint16_t i, j;
    uint8_t *ptr = ws2812_buffer;
    
    for(i = 0; i < LED_NUM; i++) {
        uint32_t color = ((uint32_t)led_colors[i].g << 16) | 
                         ((uint32_t)led_colors[i].r << 8) | 
                         led_colors[i].b;
        
        for(j = 0; j < 24; j++) {
            *ptr++ = (color & (1 << (23 - j))) ? WS2812_1_CODE : WS2812_0_CODE;
        }
    }
    
    for(i = 0; i < 32; i++) {
        *ptr++ = 0;
    }
    
    DMA_Cmd(DMA1_Channel3, DISABLE);
    DMA1_Channel3->CNDTR = sizeof(ws2812_buffer);
    DMA1_Channel3->CMAR = (uint32_t)ws2812_buffer;
    DMA_Cmd(DMA1_Channel3, ENABLE);
    
    while(DMA_GetFlagStatus(DMA1_FLAG_TC3) == RESET);
    DMA_ClearFlag(DMA1_FLAG_TC3);
}

void WS2812_Clear(void) {
    RGB_Color black = {0, 0, 0};
    WS2812_SetAll(black);
    WS2812_Update();
}
