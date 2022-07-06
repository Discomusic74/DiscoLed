#ifndef __STM32_INIT
#define __STM32_INIT

#include "stm32f4xx.h"
#include "core_cm4.h"

#define SYSCLOCK 84000000U // макрос нашей системной частоты


#define LEDS_NUM    144
#define COLRS       3

#define LED_LEV0  (33)
#define LED_LEV1  (67)
extern uint32_t DMA_buf[LEDS_NUM+2][COLRS][8];
extern uint32_t ADC_buf[1024];
extern uint32_t LEDS_buf[LEDS_NUM]; 
extern uint8_t val_sin[LEDS_NUM];

extern uint8_t sat_sin[LEDS_NUM];
#define PI 3.14159265

void STM32_init_dma_timer();
void STM32_init_timer();
void STM32_init_gpio();
void STM32_init_rcc();
void STM32_init_adc();
void STM32_init_dma_adc();

#endif