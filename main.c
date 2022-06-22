//#include "stm32f4xx_conf.h"
#include "stm32f4xx.h"
#include "core_cm4.h"
#include <stdio.h>


typedef struct a
{
  uint32_t ab;
} a; 

int main(void)
{


  //SystemInit();
  /*-Настройка тактирования
  -Инициализация (разрешения) прерываний
  -Инициализация периферии
  -Прием данных по SPI и складывание в массив при помощи DMA
  -Преобразование FFT
  -Масштабирование данных FFT в нужный диапазон
  -Связывание данных с массивом светодиодов (Режим/цвет/яркость)
  -Отправка данных на светодиодную ленту*/
  RCC_DeInit();
  RCC_HSICmd(ENABLE);
  while (!(RCC_GetFlagStatus(RCC_FLAG_HSIRDY)));
    
  FLASH_SetLatency(FLASH_Latency_2);
  RCC_PLLConfig(RCC_PLLSource_HSI, 8, 84, 2, 4);
  RCC_PLLCmd(ENABLE);
    
  //RCC->CFGR |= RCC_CFGR_SW_PLL; 					//PLL selected as system clock
  while((RCC->CR & RCC_CR_PLLRDY) == 0) { } 
  //while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) {} 	//wait for PLL is used
  //RCC_PLLCmd(ENABLE);
    RCC_ClockSecuritySystemCmd(ENABLE);
  RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
  RCC_HCLKConfig(RCC_SYSCLK_Div1);
  RCC_MCO2Config(RCC_MCO2Source_SYSCLK, RCC_MCO2Div_1);
  RCC_HCLKConfig(RCC_SYSCLK_Div1);
    
  //SystemCoreClockUpdate();


  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

  GPIO_InitTypeDef gp;
  gp.GPIO_Pin = GPIO_Pin_13;
  gp.GPIO_Mode = GPIO_Mode_OUT;
  gp.GPIO_Speed = GPIO_Speed_100MHz;
  gp.GPIO_OType = GPIO_OType_OD;
  gp.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOC,&gp);
    
 
  GPIO_InitTypeDef gp2;
  gp2.GPIO_Pin = GPIO_Pin_1;
  gp2.GPIO_Mode = GPIO_Mode_AF;
  gp2.GPIO_Speed = GPIO_Speed_100MHz;
  gp2.GPIO_OType = GPIO_OType_PP;
  gp2.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOA,&gp2);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_TIM2);

  RCC_ClocksTypeDef  rcc;
  RCC_GetClocksFreq(&rcc);
  uint32_t sysclk = rcc.SYSCLK_Frequency;
  uint32_t hclk = rcc.HCLK_Frequency;
  uint32_t getsys = RCC_GetSYSCLKSource();

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
  RCC_PCLK1Config(RCC_HCLK_Div2);
  TIM_OCInitTypeDef tim2;
  tim2.TIM_OCMode = TIM_OCMode_PWM1;
  tim2.TIM_OutputState = TIM_OutputState_Enable;
  tim2.TIM_Pulse = 30;
  tim2.TIM_OCPolarity = TIM_OCPolarity_High;
  //TIM2->ARR = 105 - 1;          // Перегрузка таймера каждые 1.25 мкс
  TIM_SetAutoreload(TIM2, 103);  // Перегрузка таймера каждые 1.25 мкс
  TIM_OC2Init(TIM2, &tim2);
  TIM_CounterModeConfig(TIM2, TIM_CounterMode_Up);
TIM_Cmd(TIM2,ENABLE);
  TIM_CCxCmd(TIM2, TIM_Channel_2, TIM_CCx_Enable);

  a b;
   while (1)
  {
    uint8_t ad = 10;
      uint8_t ab =TIM_GetCounter(TIM2);
        printf("%i\n", sysclk);
          printf("%i\n", hclk);
        printf("%i\n", ab);

    //GPIO_ToggleBits(GPIOC,GPIO_Pin_13);
  }
  b.ab = 10;
  return 0;

}

#ifdef USE_FULL_ASSERT

void assert_failed(u8* file, u32 line)
{
}
#endif
