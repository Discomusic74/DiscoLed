#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "stm32_init.h"

uint32_t DMA_buf[LEDS_NUM+2][COLRS][8];
uint32_t ADC_buf[1024];
uint32_t LEDS_buf[LEDS_NUM]; 
uint8_t val_sin[LEDS_NUM];
uint8_t sat_sin[LEDS_NUM];

__IO uint32_t SysTick_CNT = 0; //обьявляем и иницализируем в 0 значение нашего счетчика SysTick
void delay_mS(uint32_t mS);

  int main(void)
{
  STM32_init_rcc();
  STM32_init_dma_timer();
  STM32_init_timer();
  STM32_init_gpio();
  STM32_init_adc();
  STM32_init_dma_adc();
    
  SysTick_Init();
  NVIC_EnableIRQ(SysTick_IRQn);
  
  RCC_ClocksTypeDef  rcc;   // Frequence SysClock
  RCC_GetClocksFreq(&rcc);
  uint32_t sysclk = rcc.SYSCLK_Frequency;
  uint32_t hclk = rcc.PCLK2_Frequency;
  uint32_t getsys = RCC_GetSYSCLKSource();
    
  for (uint8_t i=0;i<LEDS_NUM;i++){
    val_sin[i] =(((sin((2*PI*i)/((LEDS_NUM/2))))+1)*5)+53;
    sat_sin[i] =(((sin((2*PI*i)/((LEDS_NUM/2))))+1)*20)+23;
  }

   LEDstrip_init();
   Convert_RGB_to_DMA_buf();
   while (1)
  {
      uint16_t i = 0;
    //Running_LEDs_1();
      //Fill_color(240);  
      //Moving_leds(LEDS_NUM, 0, 80);
      GPIO_SetBits(GPIOC, GPIO_Pin_13);
      delay_mS(1000);
      GPIO_ResetBits(GPIOC, GPIO_Pin_13);
      delay_mS(1000);
      //i = ADC_GetConversionValue(ADC1);
      //printf ("%d\n",hclk);
     // ADC_SoftwareStartConv(ADC1);
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


void SysTick_Handler(void)
{
  if(SysTick_CNT > 0)  SysTick_CNT--;
}

void ADC_IRQHandler(void)
{
  //ADC_ITConfig(ADC1, ADC_IT_EOC, DISABLE);
  //ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
  ADC_ClearFlag(ADC1, ADC_FLAG_OVR);
  //ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
      
  ADC_SoftwareStartConv(ADC1);
}

void DMA2_Stream0_IRQHandler(void)
{
  DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0);  // Сброс флага прерывания
}

void delay_mS(uint32_t mS)
{
  SysTick->VAL &= ~SysTick_VAL_CURRENT_Msk;//сбрасываем старые возможные значения текущего счета в 0
  SysTick->VAL = SYSCLOCK / 1000 - 1; // скобки убраны, очепятка. Правильнее в таком формате
  SysTick_CNT = mS;
  while(SysTick_CNT) {}// как только будет 0, то выходим из цикла и задержка заканчивается
}