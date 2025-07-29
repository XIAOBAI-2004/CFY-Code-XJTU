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
    EFFECT_JOY,      // 新增-喜
    EFFECT_ANGER,    // 新增-怒
    EFFECT_SORROW,   // 新增-哀
    EFFECT_DELIGHT,  // 新增-乐
    EFFECT_COMPLEX,  // 新增-复杂
    EFFECT_MAX
} EffectType;

// 新增数据结构
typedef struct {
    float position;  // 粒子位置
    float speed;     // 粒子速度
    RGB_Color color; // 粒子颜色
} Particle;

void Effects_Init(void);
void Effects_SetMode(EffectType mode);
void Effects_Run(void);
void Effects_Transition(EffectType new_mode); // 新增平滑过渡函数

#endif
