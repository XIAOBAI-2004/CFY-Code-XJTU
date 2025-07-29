#ifndef __DS3231_H
#define __DS3231_H

#include "stm32f10x.h"

// DS3231 I2C地址（左移1位后的地址）
#define DS3231_ADDR 0xD0  // 0x68 << 1

// 时间结构体定义（与OLED共用PB8/PB9引脚）
typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day;
    uint8_t date;
    uint8_t month;
    uint16_t year;
} RTC_Time;

// 函数声明
void DS3231_Init(void);
void DS3231_GetTime(RTC_Time *time);
void DS3231_SetTime(RTC_Time *time);

#endif
