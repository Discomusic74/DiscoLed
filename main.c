#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "stm32_init.h"

uint32_t DMA_buf[LEDS_NUM+3][COLRS][8];
uint32_t LEDS_buf[LEDS_NUM]; 
uint8_t val_sin[LEDS_NUM];
uint8_t sat_sin[LEDS_NUM];

  int main(void)
{
  STM32_init_rcc();
  STM32_init_dma();
  STM32_init_timer();
  STM32_init_gpio();
  
  RCC_ClocksTypeDef  rcc;   // Frequence SysClock
  RCC_GetClocksFreq(&rcc);
  uint32_t sysclk = rcc.SYSCLK_Frequency;
  uint32_t hclk = rcc.HCLK_Frequency;
  uint32_t getsys = RCC_GetSYSCLKSource();
    
  for (uint8_t i=0;i<LEDS_NUM;i++){
    val_sin[i] =(((sin((2*PI*i)/((LEDS_NUM/2))))+1)*5)+53;
    sat_sin[i] =(((sin((2*PI*i)/((LEDS_NUM/2))))+1)*20)+23;
  }

   LEDstrip_init();
   Convert_RGB_to_DMA_buf();
   while (1)
  {
    //Running_LEDs_1();
      Fill_color(240);  
      Moving_leds(LEDS_NUM, 0, 80);
  }

  return 0;

}


void DMA1_Stream7_IRQHandler(void){
 
  //DMA_Cmd(DMA1_Stream7, DISABLE);
  DMA_ClearITPendingBit(DMA1_Stream7, DMA_IT_TCIF7);  // Сброс флага прерывания
  
}

#ifdef USE_FULL_ASSERT

void assert_failed(u8* file, u32 line)
{
}
#endif


