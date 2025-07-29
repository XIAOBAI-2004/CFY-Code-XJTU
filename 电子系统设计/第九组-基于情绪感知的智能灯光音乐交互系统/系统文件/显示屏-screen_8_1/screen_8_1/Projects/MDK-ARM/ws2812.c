#include "ws2812.h"
#include "./SYSTEM/delay/delay.h"

#define RESET_PULSES  80      // 50us �͵�ƽ��λ���壨80 * 0.625us��

#ifdef USE_PWM_COMPRESSION
static uint16_t pwm_buffer[(24*LED_NUM + 80)/2]; // ѹ���洢
#else
__attribute__((section(".ccmram"))) 
static uint16_t pwm_buffer[24*LED_NUM + 80];    // �����洢��CCM RAM
#endif

//static uint16_t pwm_buffer[PWM_BITS * LED_NUM + RESET_PULSES]; // PWM ������
static RGB_Color led_colors[LED_NUM];                          // ��ɫ����

// PWM ռ�ձȶ��壨24MHz ʱ�ӣ�800kHz PWM Ƶ�ʣ�
#define WS2812_0_PULSE  8   // 0 �룺�ߵ�ƽ 0.4us (8/24MHz)
#define WS2812_1_PULSE  16  // 1 �룺�ߵ�ƽ 0.8us (16/24MHz)

void WS2812_Init(void) {
    // 1. ʹ��ʱ��
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;  // ʹ�� GPIOA
    RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;   // ʹ�� TIM5
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;   // ʹ�� DMA1

    // 2. ���� GPIO PA3��TIM5_CH4��Ϊ���ù���
    GPIOA->MODER   &= ~(0x3 << (3 * 2));      // ��� PA3 ģʽλ
    GPIOA->MODER   |=  (0x2 << (3 * 2));      // ���ù���ģʽ��AF2��
    GPIOA->OSPEEDR |=  (0x3 << (3 * 2));      // ����ģʽ��100MHz��
    GPIOA->AFR[0]  &= ~(0xF << (3 * 4));      // ��� AFRL3
    GPIOA->AFR[0]  |=  (0x2 << (3 * 4));      // AF2��TIM5_CH4��

    // 3. ���� TIM5��24MHz ʱ�ӣ�800kHz PWM��
    TIM5->CR1  = 0;                          // ���ö�ʱ��
    TIM5->PSC  = 0;                          // �޷�Ƶ��84MHz / (0+1) = 84MHz��
    TIM5->ARR  = 29;                         // PWM ���� = (29+1)/84MHz �� 0.357us��ʵ�ʵ��������ģ�
    TIM5->CCMR2 |= (0x6 << 4);               // PWM ģʽ 1��CH4��
    TIM5->CCER  |= TIM_CCER_CC4E;            // ʹ�� CH4 ���
    TIM5->CCR4  = 0;                         // ��ʼռ�ձ�Ϊ 0

    // 4. ���� DMA1 Stream6��TIM5_CH4 ������
    DMA1_Stream6->CR = 0;
    DMA1_Stream6->CR |= (6 << 25);           // Channel 6��TIM5_CH4��
    DMA1_Stream6->CR |= DMA_SxCR_DIR_0;      // �ڴ浽����
    DMA1_Stream6->CR |= DMA_SxCR_MINC;       // �ڴ��ַ����
    DMA1_Stream6->CR |= DMA_SxCR_PSIZE_1;    // �������ݴ�С��16 λ��TIM_CCR4 �� 16 λ�Ĵ�����
    DMA1_Stream6->CR |= DMA_SxCR_MSIZE_1;    // �ڴ����ݴ�С��16 λ
    DMA1_Stream6->CR |= DMA_SxCR_PL;         // �����ȼ�
    DMA1_Stream6->PAR  = (uint32_t)&TIM5->CCR4;  // Ŀ���ַ��TIM5 CCR4
    DMA1_Stream6->M0AR = (uint32_t)pwm_buffer;   // Դ��ַ��PWM ������
    DMA1_Stream6->NDTR = sizeof(pwm_buffer) / 2; // ������������16 λΪ��λ��

    // 5. ʹ�� DMA ����
    TIM5->DIER |= TIM_DIER_CC4DE;            // ʹ�� CH4 DMA ����
    TIM5->CR2  |= TIM_CR2_CCDS;              // DMA ����ѡ��Ƚ��¼�

    // 6. ��� DMA ��־
    DMA1->HIFCR = DMA_HIFCR_CTCIF6 | DMA_HIFCR_CHTIF6 | DMA_HIFCR_CTEIF6;

    // ��ʼ�� LED Ϊ�ر�״̬
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
    
    // 1. ���PWM������
    for(uint16_t i = 0; i < LED_NUM; i++) {
        color = ((uint32_t)led_colors[i].g << 16) | 
                ((uint32_t)led_colors[i].r << 8) | 
                led_colors[i].b;
        
        for(uint8_t j = 0; j < 24; j++) {
            *ptr++ = (color & (1 << (23 - j))) ? WS2812_1_PULSE : WS2812_0_PULSE;
        }
    }
    
    // 2. ��临λ�ź�(50us�͵�ƽ)
    for(uint16_t i = 0; i < 80; i++) {
        *ptr++ = 0;
    }
    
    // 3. ����DMA����
    DMA1_Stream6->CR &= ~DMA_SxCR_EN;
    while(DMA1_Stream6->CR & DMA_SxCR_EN);
    
    DMA1->HIFCR = DMA_HIFCR_CTCIF6;
    DMA1_Stream6->NDTR = sizeof(pwm_buffer)/2;
    DMA1_Stream6->M0AR = (uint32_t)pwm_buffer;
    DMA1_Stream6->CR |= DMA_SxCR_EN;
    
    // 4. ����TIM5
    TIM5->CR1 |= TIM_CR1_CEN;
    
    // 5. �ȴ��������
    while(!(DMA1->HISR & DMA_HISR_TCIF6));
    DMA1->HIFCR = DMA_HIFCR_CTCIF6;
    
    // 6. ֹͣTIM5
    TIM5->CR1 &= ~TIM_CR1_CEN;
}

void WS2812_Clear(void) {
    RGB_Color black = {0, 0, 0};
    WS2812_SetAll(black);
    WS2812_Update();
}
