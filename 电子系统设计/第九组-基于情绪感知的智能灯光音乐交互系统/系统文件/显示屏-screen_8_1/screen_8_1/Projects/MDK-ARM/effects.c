#include "effects.h"
#include "ws2812.h"
#include "./SYSTEM/delay/delay.h"
#include <math.h>
#include <stdlib.h>

#define PARTICLE_COUNT 20  // ��������

static EffectType current_effect = EFFECT_JOY;
static EffectType target_effect = EFFECT_JOY;
static float transition_progress = 1.0f;
static Particle particles[PARTICLE_COUNT];
static uint8_t effect_running = 0;

// ��ǿ��HSVתRGB���Ż�ɫ��ƽ���ȣ�
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
        (uint8_t)((g + m) * 255), // ע��WS2812��GRB˳��
        (uint8_t)((r + m) * 255),
        (uint8_t)((b + m) * 255)
    };
}

// ��ʼ������ϵͳ
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

// ƽ�����ɵ���Ч��
void Effects_Transition(EffectType new_mode) {
    target_effect = new_mode;
    transition_progress = 0.0f;
}

// ϲ��Ч�� - �ʺ����ӿ�
static void Effect_Joy(void) {
    static float hue = 0;
    
    // ��������
    for(int i = 0; i < PARTICLE_COUNT; i++) {
        particles[i].position += particles[i].speed;
        if(particles[i].position >= LED_NUM) {
            particles[i].position = 0;
            particles[i].color = HSVtoRGB(hue + i * 18, 1.0f, 1.0f);
        }
    }
    
    // ��Ⱦ������β
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

// ��ŭЧ�� - �������+����
static void Effect_Anger(void) {
    static float pulse = 0;
    static uint8_t flash_counter = 0;
    
    pulse += 0.1f;
    float intensity = (sinf(pulse * 5) + 1) / 2 * 0.7f + 0.3f;
    
    // ������ɫ����
    RGB_Color base_color = {0, (uint8_t)(255 * intensity), 0};
    WS2812_SetAll(base_color);
    
    // �����ɫ����
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

// ����Ч�� - ��ɫ�������
static void Effect_Sorrow(void) {
    static float drop_pos[3] = {0};
    static float drop_speed[3] = {0.3f, 0.5f, 0.4f};
    
    WS2812_Clear();
    
    // �������λ��
    for(int i = 0; i < 3; i++) {
        drop_pos[i] += drop_speed[i];
        if(drop_pos[i] >= LED_NUM) {
            drop_pos[i] = 0;
            drop_speed[i] = 0.2f + (float)rand() / RAND_MAX * 0.3f;
        }
        
        // ��Ⱦ���
        int pos = (int)drop_pos[i];
        for(int j = 0; j < 5; j++) {
            if(pos - j >= 0) {
                float val = 1.0f - j * 0.2f;
                WS2812_SetColor(pos - j, (RGB_Color){0, 0, (uint8_t)(100 * val)});
            }
        }
    }
    
    // �����������
    if(rand() % 30 == 0) {
        drop_pos[rand() % 3] = 0;
    }
    
    WS2812_Update();
    delay_ms(80);
}

// ����Ч�� - ������ӱ�ը
static void Effect_Delight(void) {
    static float explosion_pos = LED_NUM / 2;
    static float explosion_radius = 0;
    static uint8_t explosion_color = 0;
    
    // ����ը�뾶��������ʱ�������±�ը
    if(explosion_radius > LED_NUM / 2) {
        explosion_pos = 10 + rand() % (LED_NUM - 20); // ����̫������Ե
        explosion_radius = 0;
        explosion_color = rand() % 2; // 0=��ɫ, 1=��ɫ
    }
    
    explosion_radius += 0.7f; // �ӿ������ٶ�
    
    WS2812_Clear();
    for(int i = (int)(explosion_pos - explosion_radius); 
        i <= (int)(explosion_pos + explosion_radius); i++) {
        if(i >= 0 && i < LED_NUM) {
            float dist = fabsf(i - explosion_pos) / explosion_radius;
            float intensity = powf(1.0f - dist, 1.5f); // ����1.5�η����߸���Ȼ
            
            RGB_Color c;
            if(explosion_color == 0) {
                // ��ɫ (ע��WS2812��GRB˳��)
                c = (RGB_Color){
                    (uint8_t)(150 * intensity),  // G
                    (uint8_t)(200 * intensity),  // R
                    (uint8_t)(50 * intensity)    // B
                };
            } else {
                // ��ȷ�ķ�ɫ (R=255, G=105, B=180) -> GRB˳��
                c = (RGB_Color){
                    (uint8_t)(105 * intensity),  // G
                    (uint8_t)(255 * intensity),  // R
                    (uint8_t)(180 * intensity)   // B
                };
            }
            // ������������������15%��
            float flicker = 0.85f + (rand() % 30) / 100.0f;
            c.g = (uint8_t)(c.g * flicker);
            c.r = (uint8_t)(c.r * flicker);
            c.b = (uint8_t)(c.b * flicker);
            
            WS2812_SetColor(i, c);
        }
    }
    
    // ��Ӳ�������Ч��
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
    delay_ms(25); // �ӿ�ˢ����
}

// �������� - ��̬����Ч��
static void Effect_Complex(void) {
    static EffectType sub_effect = EFFECT_JOY;
    static uint32_t last_change = 0;
    static uint32_t counter = 0;
    
    counter++;
    
    // ÿ10���л���Ч��
    if(counter - last_change > 300) {
        last_change = counter;
        sub_effect = (sub_effect + 1) % (EFFECT_COMPLEX - 1);
    }
    
    // ���ݵ�ǰ��Ч�����ö�Ӧ����
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
    
    // Ч�����ɴ���
    if(transition_progress < 1.0f) {
        transition_progress += 0.02f;
        if(transition_progress >= 1.0f) {
            current_effect = target_effect;
        }
    }
    switch(current_effect) {
		case EFFECT_JOY:     Effect_Joy();     break; // ϲ
        case EFFECT_ANGER:   Effect_Anger();   break; // ŭ
        case EFFECT_SORROW:  Effect_Sorrow();  break; // ��
        case EFFECT_DELIGHT: Effect_Delight(); break; // ��
        case EFFECT_COMPLEX: Effect_Complex(); break; // ����
        case EFFECT_FLOW: {
            // ��ˮЧ��
            WS2812_Clear();
            uint16_t pos = counter % LED_NUM;
            RGB_Color color = {0, 0, 255}; // ��ɫ��ˮ
            WS2812_SetColor(pos, color);
            WS2812_Update();
            delay_ms(50);
            counter++;
            break;
        }
        
        case EFFECT_GRADIENT: {
            // ��ɫ����Ч��
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
            // ��˸Ч��
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
            // �ʺ�ѭ��Ч��
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
            // ����Ч��
            blink_state = !blink_state;
            RGB_Color color = blink_state ? (RGB_Color){255, 255, 255} : (RGB_Color){0, 0, 0};
            WS2812_SetAll(color);
            WS2812_Update();
			break;
        }
		case EFFECT_OFF: {
            // �ص�Ч��
                WS2812_Clear();
            break;
        }
        default:
            break;
    }
}
