//// voice_ctrl.h
//#ifndef __VOICE_CTRL_H
//#define __VOICE_CTRL_H

//#include "ucos_ii.h"
//#include "audioplay.h"

///* ָ�����Ͷ��� */
//typedef enum {
//    VOICE_PLAY_PAUSE,
//    VOICE_PREV,
//    VOICE_NEXT,
//    VOICE_STOP
//} VoiceCmdType;

///* ��Ϣ�ṹ�� */
//typedef struct {
//    VoiceCmdType cmd;
//} VoiceMsg;

///* �ⲿ���� */
//extern OS_EVENT *VoiceMsgQ;
//extern OS_STK VoiceTaskStk[];
//extern void *VoiceMsgQTbl[10];

///* �궨�� */
//#define VOICE_MSG_Q_SIZE 10
//#define VOICE_TASK_PRIO  4
//#define VOICE_STK_SIZE   256

///* ȫ�ֱ������� */
//extern OS_EVENT *audiombox;  // ���extern����

///* �������� */
//void VoiceTask(void *pdata);
//void VoiceCtrl_Init(void);

//#endif