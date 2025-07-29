//// voice_ctrl.h
//#ifndef __VOICE_CTRL_H
//#define __VOICE_CTRL_H

//#include "ucos_ii.h"
//#include "audioplay.h"

///* 指令类型定义 */
//typedef enum {
//    VOICE_PLAY_PAUSE,
//    VOICE_PREV,
//    VOICE_NEXT,
//    VOICE_STOP
//} VoiceCmdType;

///* 消息结构体 */
//typedef struct {
//    VoiceCmdType cmd;
//} VoiceMsg;

///* 外部声明 */
//extern OS_EVENT *VoiceMsgQ;
//extern OS_STK VoiceTaskStk[];
//extern void *VoiceMsgQTbl[10];

///* 宏定义 */
//#define VOICE_MSG_Q_SIZE 10
//#define VOICE_TASK_PRIO  4
//#define VOICE_STK_SIZE   256

///* 全局变量声明 */
//extern OS_EVENT *audiombox;  // 添加extern声明

///* 函数声明 */
//void VoiceTask(void *pdata);
//void VoiceCtrl_Init(void);

//#endif