#include "ds3231.h"
#include "OLED.h"  // 复用OLED的I2C函数

// BCD转十进制
static uint8_t BCD2DEC(uint8_t bcd) {
    return (bcd >> 4) * 10 + (bcd & 0x0F);
}

// 十进制转BCD
static uint8_t DEC2BCD(uint8_t dec) {
    return ((dec / 10) << 4) | (dec % 10);
}

// 读取多个寄存器（复用OLED的I2C函数）
static void DS3231_ReadReg(uint8_t reg, uint8_t *buf, uint8_t len) 
{
    OLED_I2C_Start();
    OLED_I2C_SendByte(DS3231_ADDR);     // 写模式
    OLED_I2C_SendByte(reg);
    OLED_I2C_Start();                   // 重复启动
    OLED_I2C_SendByte(DS3231_ADDR | 1); // 读模式
    
    while(len--) {
        *buf++ = OLED_I2C_ReceiveByte();
        if(len) OLED_I2C_SendAck(0);    // 非最后一个字节发ACK
        else OLED_I2C_SendAck(1);       // 最后一个字节发NACK
    }
    OLED_I2C_Stop();
}

// 写入多个寄存器
static void DS3231_WriteReg(uint8_t reg, uint8_t *buf, uint8_t len) {
    OLED_I2C_Start();
    OLED_I2C_SendByte(DS3231_ADDR);
    OLED_I2C_SendByte(reg);
    while(len--) {
        OLED_I2C_SendByte(*buf++);
    }
    OLED_I2C_Stop();
}

// 初始化（实际无需硬件初始化，共用OLED的I2C）
void DS3231_Init(void) {
    // 空函数，共用OLED的I2C初始化
}

// 获取时间
void DS3231_GetTime(RTC_Time *time) {
    uint8_t buf[7];
    DS3231_ReadReg(0x00, buf, 7);  // 读取0x00-0x06寄存器
    
    time->seconds = BCD2DEC(buf[0] & 0x7F);
    time->minutes = BCD2DEC(buf[1]);
    time->hours   = BCD2DEC(buf[2] & 0x3F);  // 24小时模式
    time->day     = BCD2DEC(buf[3]);
    time->date    = BCD2DEC(buf[4]);
    time->month   = BCD2DEC(buf[5] & 0x1F);  // 忽略世纪位
    time->year    = BCD2DEC(buf[6]) + 2000;
}

// 设置时间
void DS3231_SetTime(RTC_Time *time) {
    uint8_t buf[7] = {
        DEC2BCD(time->seconds),
        DEC2BCD(time->minutes),
        DEC2BCD(time->hours),
        DEC2BCD(time->day),
        DEC2BCD(time->date),
        DEC2BCD(time->month),
        DEC2BCD(time->year % 100)
    };
    DS3231_WriteReg(0x00, buf, 7);
}
