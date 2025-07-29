#include "ds3231.h"
#include "OLED.h"  // ����OLED��I2C����

// BCDתʮ����
static uint8_t BCD2DEC(uint8_t bcd) {
    return (bcd >> 4) * 10 + (bcd & 0x0F);
}

// ʮ����תBCD
static uint8_t DEC2BCD(uint8_t dec) {
    return ((dec / 10) << 4) | (dec % 10);
}

// ��ȡ����Ĵ���������OLED��I2C������
static void DS3231_ReadReg(uint8_t reg, uint8_t *buf, uint8_t len) 
{
    OLED_I2C_Start();
    OLED_I2C_SendByte(DS3231_ADDR);     // дģʽ
    OLED_I2C_SendByte(reg);
    OLED_I2C_Start();                   // �ظ�����
    OLED_I2C_SendByte(DS3231_ADDR | 1); // ��ģʽ
    
    while(len--) {
        *buf++ = OLED_I2C_ReceiveByte();
        if(len) OLED_I2C_SendAck(0);    // �����һ���ֽڷ�ACK
        else OLED_I2C_SendAck(1);       // ���һ���ֽڷ�NACK
    }
    OLED_I2C_Stop();
}

// д�����Ĵ���
static void DS3231_WriteReg(uint8_t reg, uint8_t *buf, uint8_t len) {
    OLED_I2C_Start();
    OLED_I2C_SendByte(DS3231_ADDR);
    OLED_I2C_SendByte(reg);
    while(len--) {
        OLED_I2C_SendByte(*buf++);
    }
    OLED_I2C_Stop();
}

// ��ʼ����ʵ������Ӳ����ʼ��������OLED��I2C��
void DS3231_Init(void) {
    // �պ���������OLED��I2C��ʼ��
}

// ��ȡʱ��
void DS3231_GetTime(RTC_Time *time) {
    uint8_t buf[7];
    DS3231_ReadReg(0x00, buf, 7);  // ��ȡ0x00-0x06�Ĵ���
    
    time->seconds = BCD2DEC(buf[0] & 0x7F);
    time->minutes = BCD2DEC(buf[1]);
    time->hours   = BCD2DEC(buf[2] & 0x3F);  // 24Сʱģʽ
    time->day     = BCD2DEC(buf[3]);
    time->date    = BCD2DEC(buf[4]);
    time->month   = BCD2DEC(buf[5] & 0x1F);  // ��������λ
    time->year    = BCD2DEC(buf[6]) + 2000;
}

// ����ʱ��
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
