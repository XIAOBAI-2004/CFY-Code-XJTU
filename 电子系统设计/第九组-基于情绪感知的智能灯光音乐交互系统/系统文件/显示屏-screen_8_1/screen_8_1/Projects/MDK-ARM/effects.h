#ifndef __EFFECTS_H
#define __EFFECTS_H

#include "ws2812.h"

typedef enum {
    EFFECT_FLOW = 0,
    EFFECT_GRADIENT,
    EFFECT_BLINK,
    EFFECT_RAINBOW,
	EFFECT_ON,
	EFFECT_OFF,
    EFFECT_JOY,      // ����-ϲ
    EFFECT_ANGER,    // ����-ŭ
    EFFECT_SORROW,   // ����-��
    EFFECT_DELIGHT,  // ����-��
    EFFECT_COMPLEX,  // ����-����
    EFFECT_MAX
} EffectType;

// �������ݽṹ
typedef struct {
    float position;  // ����λ��
    float speed;     // �����ٶ�
    RGB_Color color; // ������ɫ
} Particle;

void Effects_Init(void);
void Effects_SetMode(EffectType mode);
void Effects_Run(void);
void Effects_Transition(EffectType new_mode); // ����ƽ�����ɺ���

#endif
