#include "stm32f10x.h"
#include "ws2812.h"
#include "key.h"
#include "effects.h"
#include "delay.h"
#include "Serial_2.h"
#include "Serial.h"
#include "OLED.h"
#include "ds3231.h"

int main(void) {
    RTC_Time currentTime;
	SystemInit();
    Delay_Init();
    KEY_Init();
    WS2812_Init();
    Effects_Init();
    Serial_Init();		//串口初始化
	Serial2_Init();
	OLED_Init();	
    EffectType current_mode = EFFECT_BLINK;
	OLED_ShowString(4, 1, "MOOD:NONE");
    Effects_SetMode(current_mode);
    DS3231_Init();  // 实际初始化在OLED_Init中完成
    
    // 首次使用设置时间（示例）
//    RTC_Time initTime = {10, 02, 20, 3, 14, 5, 2025};
//    DS3231_SetTime(&initTime);
//    
//    OLED_Clear();
//    OLED_ShowString(1, 1, "Date:2025-05-24");
//    OLED_ShowString(3, 1, "Time:20-02-10");
//    
    while(1) {
        DS3231_GetTime(&currentTime);
		OLED_ShowString(1, 1, "Date:");
		OLED_ShowString(3, 1, "Time:");
		//OLED_ClearArea(1, 45, 51);  // 清空第1行6-15列
		/* 更新日期显示（YYYY-MM-DD）*/
		OLED_ShowNum(1, 6,  currentTime.year, 4);    // 年
		OLED_ShowString(1, 10, "-");                // 添加分隔符
		OLED_ShowNum(1, 11, currentTime.month, 2);  // 月（两位显示）
		OLED_ShowString(1, 13, "-");                // 添加分隔符
		OLED_ShowNum(1, 14, currentTime.date, 2);   // 日（两位显示）
		
		//OLED_ClearArea(3, 13, 14);  // 清空第3行6-13列
		/* 更新时间显示（HH:MM:SS）*/
		OLED_ShowNum(3, 6,  currentTime.hours, 2);  // 时（两位）
		OLED_ShowString(3, 8, ":");                 // 添加冒号
		OLED_ShowNum(3, 9,  currentTime.minutes, 2);// 分（两位）
		OLED_ShowString(3, 11, ":");                // 添加冒号
		OLED_ShowNum(3, 12, currentTime.seconds, 2);// 秒（两位）
    
		    /* 显示星期 */
		char *week[] = {"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saterday"};
		OLED_ShowString(2, 1, week[currentTime.day % 7]);
		
		
		if (Serial_RxFlag == 1)		//如果接收到数据包
		{
			
			/*将收到的数据包与预设的指令对比，以此决定将要执行的操作*/
			if (strcmp(Serial_RxPacket, "liushui") == 0)			
			{
				current_mode = 0;
				Effects_SetMode(current_mode);
				delay_ms(300); // 防抖延时
			}
			else if (strcmp(Serial_RxPacket, "qisejianbian") == 0)	
			{
				current_mode = 1;
				Effects_SetMode(current_mode);
				delay_ms(300); // 防抖延时
			}
			else if (strcmp(Serial_RxPacket, "shanshuo") == 0)	
			{
				current_mode = 2;
				Effects_SetMode(current_mode);
				delay_ms(300); // 防抖延时
			}
			else if (strcmp(Serial_RxPacket, "xunhuan") == 0)	
			{
				current_mode = 3;
				Effects_SetMode(current_mode);
				delay_ms(300); // 防抖延时
			}	
			else if (strcmp(Serial_RxPacket, "kaideng") == 0)	
			{
				current_mode = 4;
				Effects_SetMode(current_mode);
				delay_ms(300); // 防抖延时
			}	
			else if (strcmp(Serial_RxPacket, "guandeng") == 0)	
			{
				current_mode = 5;
				Effects_SetMode(current_mode);
				delay_ms(300); // 防抖延时
			}
			else if (strcmp(Serial_RxPacket, "kaixin") == 0)			
			{
				current_mode = 6;
				Effects_SetMode(current_mode);
				delay_ms(300); // 防抖延时
				OLED_ShowString(4, 1, "MOOD:HAPPY  ");
				
			}
			else if (strcmp(Serial_RxPacket, "shengqi") == 0)			
			{
				current_mode = 7;
				Effects_SetMode(current_mode);
				delay_ms(300); // 防抖延时
				OLED_ShowString(4, 1, "MOOD:ANGRY  ");
			}
			else if (strcmp(Serial_RxPacket, "beishang") == 0)			
			{
				current_mode = 8;
				Effects_SetMode(current_mode);
				delay_ms(300); // 防抖延时
				OLED_ShowString(4, 1, "MOOD:SAD    ");
			}	
			else if (strcmp(Serial_RxPacket, "kuaile") == 0)			
			{
				current_mode = 9;
				Effects_SetMode(current_mode);
				delay_ms(300); // 防抖延时
			}
			else if (strcmp(Serial_RxPacket, "fuza") == 0)			
			{
				current_mode = 10;
				Effects_SetMode(current_mode);
				delay_ms(300); // 防抖延时
			}
			else if (strcmp(Serial_RxPacket, "zengwu") == 0)			
			{
				current_mode = 11;
				Effects_SetMode(current_mode);
				delay_ms(300); // 防抖延时
				OLED_ShowString(4, 1, "MOOD:DISGUST");
			}
			else if (strcmp(Serial_RxPacket, "jingxi") == 0)			
			{
				current_mode = 12;
				Effects_SetMode(current_mode);
				delay_ms(300); // 防抖延时
				OLED_ShowString(4, 1, "MOOD:SURPRISE");
			}
			else if (strcmp(Serial_RxPacket, "zhongli") == 0)			
			{
				current_mode = 13;
				Effects_SetMode(current_mode);
				delay_ms(300); // 防抖延时
				OLED_ShowString(4, 1, "MOOD:NEUTRAL");
			}
			else if (strcmp(Serial_RxPacket, "kongju") == 0)			
			{
				current_mode = 14;
				Effects_SetMode(current_mode);
				delay_ms(300); // 防抖延时
				OLED_ShowString(4, 1, "MOOD:FEAR   ");
			}
			Serial_RxFlag = 0;			//处理完成后，需要将接收数据包标志位清零，否则将无法接收后续数据包
		}
		
		if (Serial2_RxFlag == 1)		//如果接收到数据包
		{			
			/*将收到的数据包与预设的指令对比，以此决定将要执行的操作*/
			if (strcmp(Serial2_RxPacket, "SONG:1") == 0)			
			{
				current_mode = 7;
				Effects_SetMode(current_mode);
				delay_ms(300); // 防抖延时
				OLED_ShowString(4, 1, "MOOD:ANGRY  ");
			}
			else if (strcmp(Serial2_RxPacket, "SONG:3") == 0)			
			{
				current_mode = 11;
				Effects_SetMode(current_mode);
				delay_ms(300); // 防抖延时
				OLED_ShowString(4, 1, "MOOD:DISGUST");
			}
			else if (strcmp(Serial2_RxPacket, "SONG:4") == 0)			
			{
				current_mode = 14;
				Effects_SetMode(current_mode);
				delay_ms(300); // 防抖延时
				OLED_ShowString(4, 1, "MOOD:FEAR   ");
			}
			else if (strcmp(Serial2_RxPacket, "SONG:2") == 0)			
			{
				current_mode = 6;
				Effects_SetMode(current_mode);
				delay_ms(300); // 防抖延时
				OLED_ShowString(4, 1, "MOOD:HAPPY  ");
			}
			else if (strcmp(Serial2_RxPacket, "SONG:5") == 0)			
			{
				current_mode = 8;
				Effects_SetMode(current_mode);
				delay_ms(300); // 防抖延时
				OLED_ShowString(4, 1, "MOOD:SAD    ");
			}
			else if (strcmp(Serial2_RxPacket, "SONG:6") == 0)			
			{
				current_mode = 12;
				Effects_SetMode(current_mode);
				delay_ms(300); // 防抖延时
				OLED_ShowString(4, 1, "MOOD:SURPRISE");
			}
			else if (strcmp(Serial2_RxPacket, "zanting") == 0)			
			{
				current_mode = 2;
				Effects_SetMode(current_mode);
				delay_ms(300); // 防抖延时
				OLED_ShowString(4, 1, "MOOD:NEUTRAL");
			}
		
			Serial2_RxFlag = 0;			//处理完成后，需要将接收数据包标志位清零，否则将无法接收后续数据包
		}
        // 运行当前效果
      Effects_Run(); 
	}
}
