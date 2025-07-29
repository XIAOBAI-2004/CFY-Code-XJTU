//// voice_ctrl.c
//#include "voice_ctrl.h"
//#include "common.h"
//#include "settings.h"
//#include "audioplay.h"
//#include "os.h"
//// ��ϵͳͷ�ļ�/includes.h�����:
//#include <stdlib.h>  // ����malloc/free�ȱ�׼����

//// ״̬����
//#define PLAYING  (1 << 0)
//#define PAUSED   (0)

///* ����ȫ�ֱ��� */
//OS_EVENT *VoiceMsgQ = NULL;
//void *VoiceMsgQTbl[VOICE_MSG_Q_SIZE];
//OS_STK VoiceTaskStk[VOICE_STK_SIZE];

///* ������������ */
//void VoiceTask(void *pdata)
//{
//    INT8U err;
//    VoiceMsg *msg;
//    OS_CPU_SR cpu_sr = 0; // �޸�cpu_srδ�������
//    
//    while(1) {
//        msg = (VoiceMsg*)OSQPend(VoiceMsgQ, 0, &err);
//        if(err == OS_ERR_NONE) {
//            OS_ENTER_CRITICAL();
//            switch(msg->cmd) {
//                case VOICE_PLAY_PAUSE:
//                    if(g_audiodev.status & (1 << 0)) {
//                        audio_stop();
//                        g_audiodev.status &= ~(1 << 0);
//                    } else {
//                        audio_start();
//                        g_audiodev.status |= (1 << 0);
//                    }
//                    break;
//                case VOICE_PREV:
//                    audio_stop_req(&g_audiodev);
//                    if(systemset.audiomode == 1) {
//                        g_audiodev.curindex = app_get_rand(g_audiodev.mfilenum);
//                    } else {
//                        if(g_audiodev.curindex) g_audiodev.curindex--;
//                        else g_audiodev.curindex = g_audiodev.mfilenum-1;
//                    }
//                    OSMboxPost(audiombox, (void*)(g_audiodev.curindex + 1));
//                    break;
//                case VOICE_NEXT:
//                    audio_stop_req(&g_audiodev);
//                    if(systemset.audiomode == 1) {
//                        g_audiodev.curindex = app_get_rand(g_audiodev.mfilenum);
//                    } else {
//                        if(g_audiodev.curindex < (g_audiodev.mfilenum-1)) 
//                            g_audiodev.curindex++;
//                        else 
//                            g_audiodev.curindex = 0;
//                    }
//                    OSMboxPost(audiombox, (void*)(g_audiodev.curindex + 1));
//                    break;
//                case VOICE_STOP:
//                    audio_task_delete();
//                    break;
//            }
//            OS_EXIT_CRITICAL();
//            myfree(SRAMIN, msg);
//        }
//    }
//}

///* ��ʼ������ */
//void VoiceCtrl_Init(void)
//{
//    VoiceMsgQ = OSQCreate(&VoiceMsgQTbl[0], VOICE_MSG_Q_SIZE);
//    OSTaskCreateExt(VoiceTask, 
//                  (void *)0,
//                  &VoiceTaskStk[VOICE_STK_SIZE-1],
//                  VOICE_TASK_PRIO,
//                  VOICE_TASK_PRIO,
//                  VoiceTaskStk,
//                  VOICE_STK_SIZE,
//                  (void *)0,
//                  OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
//}