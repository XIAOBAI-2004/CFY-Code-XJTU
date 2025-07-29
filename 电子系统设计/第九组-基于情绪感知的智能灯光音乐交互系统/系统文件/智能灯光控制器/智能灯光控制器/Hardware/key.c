#include "key.h"
#include "delay.h"

void KEY_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_APB2PeriphClockCmd(KEY_RCC, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = KEY_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // …œ¿≠ ‰»Î
    GPIO_Init(KEY_PORT, &GPIO_InitStructure);
}

uint8_t KEY_Scan(uint8_t mode) {
    static uint8_t key_up = 1;
    if(mode) key_up = 1;
    
    if(key_up && (GPIO_ReadInputDataBit(KEY_PORT, KEY_PIN) == 0)) {
        delay_ms(10);
        key_up = 0;
        if(GPIO_ReadInputDataBit(KEY_PORT, KEY_PIN) == 0) {
            return 1;
        }
    } else if(GPIO_ReadInputDataBit(KEY_PORT, KEY_PIN) == 1) {
        key_up = 1;
    }
    return 0;
}
