#include "effects.h"
#include "ws2812.h"
#include "./SYSTEM/delay/delay.h"
#include <math.h>
#include <stdlib.h>

#define PARTICLE_COUNT 20  // 粒子数量

static EffectType current_effect = EFFECT_JOY;
static EffectType target_effect = EFFECT_JOY;
static float transition_progress = 1.0f;
static Particle particles[PARTICLE_COUNT];
static uint8_t effect_running = 0;

// 增强型HSV转RGB（优化色彩平滑度）
static RGB_Color HSVtoRGB(float h, float s, float v) {
    h = fmodf(h, 360.0f);
    float c = v * s;
    float x = c * (1 - fabsf(fmodf(h / 60.0f, 2) - 1));
    float m = v - c;
    
    float r, g, b;
    if(h < 60)      { r = c; g = x; b = 0; }
    else if(h < 120) { r = x; g = c; b = 0; }
    else if(h < 180) { r = 0; g = c; b = x; }
    else if(h < 240) { r = 0; g = x; b = c; }
    else if(h < 300) { r = x; g = 0; b = c; }
    else             { r = c; g = 0; b = x; }
    
    return (RGB_Color){
        (uint8_t)((g + m) * 255), // 注意WS2812是GRB顺序
        (uint8_t)((r + m) * 255),
        (uint8_t)((b + m) * 255)
    };
}

// 初始化粒子系统
static void InitParticles(void) {
    for(int i = 0; i < PARTICLE_COUNT; i++) {
        particles[i].position = (float)rand() / RAND_MAX * LED_NUM;
        particles[i].speed = 0.1f + (float)rand() / RAND_MAX * 0.3f;
        particles[i].color = HSVtoRGB(rand() % 360, 1.0f, 1.0f);
    }
}

void Effects_Init(void) {
    InitParticles();
}

// 平滑过渡到新效果
void Effects_Transition(EffectType new_mode) {
    target_effect = new_mode;
    transition_progress = 0.0f;
}

// 喜悦效果 - 彩虹粒子狂欢
static void Effect_Joy(void) {
    static float hue = 0;
    
    // 更新粒子
    for(int i = 0; i < PARTICLE_COUNT; i++) {
        particles[i].position += particles[i].speed;
        if(particles[i].position >= LED_NUM) {
            particles[i].position = 0;
            particles[i].color = HSVtoRGB(hue + i * 18, 1.0f, 1.0f);
        }
    }
    
    // 渲染粒子拖尾
    WS2812_Clear();
    for(int i = 0; i < PARTICLE_COUNT; i++) {
        int pos = (int)particles[i].position;
        for(int j = -2; j <= 2; j++) {
            if(pos + j >= 0 && pos + j < LED_NUM) {
                float intensity = 1.0f - fabsf(j) * 0.3f;
                RGB_Color c = particles[i].color;
                c.r *= intensity;
                c.g *= intensity;
                c.b *= intensity;
                WS2812_SetColor(pos + j, c);
            }
        }
    }
    
    hue += 1.0f;
    WS2812_Update();
    delay_ms(30);
}

// 愤怒效果 - 红白脉冲+闪电
static void Effect_Anger(void) {
    static float pulse = 0;
    static uint8_t flash_counter = 0;
    
    pulse += 0.1f;
    float intensity = (sinf(pulse * 5) + 1) / 2 * 0.7f + 0.3f;
    
    // 基础红色脉冲
    RGB_Color base_color = {0, (uint8_t)(255 * intensity), 0};
    WS2812_SetAll(base_color);
    
    // 随机白色闪电
    if(flash_counter == 0 && rand() % 20 == 0) {
        flash_counter = 5;
        uint8_t start = rand() % (LED_NUM / 2);
        uint8_t length = 5 + rand() % 10;
        for(uint8_t i = start; i < start + length && i < LED_NUM; i++) {
            WS2812_SetColor(i, (RGB_Color){255, 255, 255});
        }
    }
    
    if(flash_counter > 0) flash_counter--;
    
    WS2812_Update();
    delay_ms(50);
}

// 哀伤效果 - 蓝色泪滴下落
static void Effect_Sorrow(void) {
    static float drop_pos[3] = {0};
    static float drop_speed[3] = {0.3f, 0.5f, 0.4f};
    
    WS2812_Clear();
    
    // 更新泪滴位置
    for(int i = 0; i < 3; i++) {
        drop_pos[i] += drop_speed[i];
        if(drop_pos[i] >= LED_NUM) {
            drop_pos[i] = 0;
            drop_speed[i] = 0.2f + (float)rand() / RAND_MAX * 0.3f;
        }
        
        // 渲染泪滴
        int pos = (int)drop_pos[i];
        for(int j = 0; j < 5; j++) {
            if(pos - j >= 0) {
                float val = 1.0f - j * 0.2f;
                WS2812_SetColor(pos - j, (RGB_Color){0, 0, (uint8_t)(100 * val)});
            }
        }
    }
    
    // 随机添加新泪滴
    if(rand() % 30 == 0) {
        drop_pos[rand() % 3] = 0;
    }
    
    WS2812_Update();
    delay_ms(80);
}

// 快乐效果 - 金粉粒子爆炸
static void Effect_Delight(void) {
    static float explosion_pos = LED_NUM / 2;
    static float explosion_radius = 0;
    static uint8_t explosion_color = 0;
    
    // 当爆炸半径超过限制时，生成新爆炸
    if(explosion_radius > LED_NUM / 2) {
        explosion_pos = 10 + rand() % (LED_NUM - 20); // 避免太靠近边缘
        explosion_radius = 0;
        explosion_color = rand() % 2; // 0=金色, 1=粉色
    }
    
    explosion_radius += 0.7f; // 加快扩张速度
    
    WS2812_Clear();
    for(int i = (int)(explosion_pos - explosion_radius); 
        i <= (int)(explosion_pos + explosion_radius); i++) {
        if(i >= 0 && i < LED_NUM) {
            float dist = fabsf(i - explosion_pos) / explosion_radius;
            float intensity = powf(1.0f - dist, 1.5f); // 改用1.5次方曲线更自然
            
            RGB_Color c;
            if(explosion_color == 0) {
                // 金色 (注意WS2812是GRB顺序)
                c = (RGB_Color){
                    (uint8_t)(150 * intensity),  // G
                    (uint8_t)(200 * intensity),  // R
                    (uint8_t)(50 * intensity)    // B
                };
            } else {
                // 正确的粉色 (R=255, G=105, B=180) -> GRB顺序
                c = (RGB_Color){
                    (uint8_t)(105 * intensity),  // G
                    (uint8_t)(255 * intensity),  // R
                    (uint8_t)(180 * intensity)   // B
                };
            }
            // 添加亮度随机波动（±15%）
            float flicker = 0.85f + (rand() % 30) / 100.0f;
            c.g = (uint8_t)(c.g * flicker);
            c.r = (uint8_t)(c.r * flicker);
            c.b = (uint8_t)(c.b * flicker);
            
            WS2812_SetColor(i, c);
        }
    }
    
    // 添加残留粒子效果
    if(rand() % 5 == 0 && explosion_radius > 5) {
        uint16_t spark_pos = explosion_pos + (rand() % 10 - 5);
        if(spark_pos >= 0 && spark_pos < LED_NUM) {
            WS2812_SetColor(spark_pos, (RGB_Color){
                (uint8_t)(200 * 0.8f),
                (uint8_t)(255 * 0.8f),
                (uint8_t)(150 * 0.8f)
            });
        }
    }
    
    WS2812_Update();
    delay_ms(25); // 加快刷新率
}

// 复杂情绪 - 动态过渡效果
static void Effect_Complex(void) {
    static EffectType sub_effect = EFFECT_JOY;
    static uint32_t last_change = 0;
    static uint32_t counter = 0;
    
    counter++;
    
    // 每10秒切换子效果
    if(counter - last_change > 300) {
        last_change = counter;
        sub_effect = (sub_effect + 1) % (EFFECT_COMPLEX - 1);
    }
    
    // 根据当前子效果调用对应函数
    switch(sub_effect) {
        case EFFECT_JOY:    Effect_Joy(); break;
        case EFFECT_ANGER:  Effect_Anger(); break;
        case EFFECT_SORROW: Effect_Sorrow(); break;
        case EFFECT_DELIGHT:Effect_Delight(); break;
        default: break;
    }
}


void Effects_SetMode(EffectType mode) {
    if(mode < EFFECT_MAX) {
        current_effect = mode;
    }
}

void Effects_Run(void) {
    static uint16_t counter = 0;
    static uint8_t blink_state = 0;
    static float hue = 0;
    static uint32_t last_time = 0;
    
    // 效果过渡处理
    if(transition_progress < 1.0f) {
        transition_progress += 0.02f;
        if(transition_progress >= 1.0f) {
            current_effect = target_effect;
        }
    }
    switch(current_effect) {
		case EFFECT_JOY:     Effect_Joy();     break; // 喜
        case EFFECT_ANGER:   Effect_Anger();   break; // 怒
        case EFFECT_SORROW:  Effect_Sorrow();  break; // 哀
        case EFFECT_DELIGHT: Effect_Delight(); break; // 乐
        case EFFECT_COMPLEX: Effect_Complex(); break; // 复杂
        case EFFECT_FLOW: {
            // 流水效果
            WS2812_Clear();
            uint16_t pos = counter % LED_NUM;
            RGB_Color color = {0, 0, 255}; // 蓝色流水
            WS2812_SetColor(pos, color);
            WS2812_Update();
            delay_ms(50);
            counter++;
            break;
        }
        
        case EFFECT_GRADIENT: {
            // 七色渐变效果
            float progress = (counter % 100) / 100.0f;
            hue = progress * 360.0f;
            RGB_Color color = HSVtoRGB(hue, 1.0f, 1.0f);
            WS2812_SetAll(color);
            WS2812_Update();
            delay_ms(50);
            counter++;
            break;
        }
        
        case EFFECT_BLINK: {
            // 闪烁效果
            if(counter % 20 == 0) {
                blink_state = !blink_state;
                RGB_Color color = blink_state ? (RGB_Color){255, 255, 255} : (RGB_Color){0, 0, 0};
                WS2812_SetAll(color);
                WS2812_Update();
            }
            delay_ms(50);
            counter++;
            break;
        }
        
        case EFFECT_RAINBOW: {
            // 彩虹循环效果
            for(uint16_t i = 0; i < LED_NUM; i++) {
                float led_hue = ((float)i / LED_NUM * 360.0f + hue);
                RGB_Color color = HSVtoRGB(led_hue, 1.0f, 1.0f);
                WS2812_SetColor(i, color);
            }
            WS2812_Update();
            hue += 1.0f;
            if(hue >= 360.0f) hue = 0;
            delay_ms(30);
            break;
        }
        case EFFECT_ON: {
            // 开灯效果
            blink_state = !blink_state;
            RGB_Color color = blink_state ? (RGB_Color){255, 255, 255} : (RGB_Color){0, 0, 0};
            WS2812_SetAll(color);
            WS2812_Update();
			break;
        }
		case EFFECT_OFF: {
            // 关灯效果
                WS2812_Clear();
            break;
        }
        default:
            break;
    }
}
