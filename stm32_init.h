#ifndef __STM32_INIT
#define __STM32_INIT

#include "stm32f4xx.h"
#include "core_cm4.h"
#include "arm_math.h" 

#define SYSCLOCK 84000000U // макрос нашей системной частоты
#define LENGTH_SAMPLES 1024

#define LEDS_NUM    144
#define COLRS       3

#define HSV_HUE_MAX		360
#define HSV_SAT_MAX		255
#define HSV_VAL_MAX		255

#define LED_LEV0  (33)
#define LED_LEV1  (67)
extern uint32_t DMA_buf[LEDS_NUM+2][COLRS][8];
extern q31_t ADC_buf[LENGTH_SAMPLES];
extern uint32_t LEDS_buf[LEDS_NUM]; 
extern uint8_t val_sin[LEDS_NUM];

extern uint8_t sat_sin[LEDS_NUM];
//#define PI 3.14159265

typedef struct CF_HSV_TypeDef{
  uint8_t H;
  uint8_t S;
  uint8_t V;
} CF_HSV_TypeDef;

#define ABS(x) ((x) < 0 ? -(x) : (x)) // модуль числа

void STM32_init_dma_timer();
void STM32_init_timer();
void STM32_init_gpio();
void STM32_init_rcc();
void STM32_init_adc();
void STM32_init_dma_adc();
void ISR_init_timer();

#endif