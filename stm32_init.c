#include "stm32_init.h"

void STM32_init_rcc(){

	RCC_DeInit();
  RCC_HSICmd(ENABLE);
  while (!(RCC_GetFlagStatus(RCC_FLAG_HSIRDY)));
    
  FLASH_SetLatency(FLASH_Latency_2);
  RCC_PLLConfig(RCC_PLLSource_HSI, 8, 84, 2, 4);
  RCC_PLLCmd(ENABLE);
    					//PLL selected as system clock
  while((RCC->CR & RCC_CR_PLLRDY) == 0) { } 

  RCC_ClockSecuritySystemCmd(ENABLE);
  RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
  RCC_HCLKConfig(RCC_SYSCLK_Div1);
  RCC_MCO2Config(RCC_MCO2Source_SYSCLK, RCC_MCO2Div_1);
  RCC_HCLKConfig(RCC_SYSCLK_Div1);
  RCC_PCLK1Config(RCC_HCLK_Div2);

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);  // Timer2
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);  // DMA
 
}

void STM32_init_gpio(){

 GPIO_InitTypeDef gp;
  gp.GPIO_Pin = GPIO_Pin_13;
  gp.GPIO_Mode = GPIO_Mode_OUT;
  gp.GPIO_Speed = GPIO_Speed_100MHz;
  gp.GPIO_OType = GPIO_OType_OD;
  gp.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOC,&gp);
    
 
  GPIO_InitTypeDef gp2;     // PA1 out for Timer2_Channel2
  gp2.GPIO_Pin = GPIO_Pin_3;
  gp2.GPIO_Mode = GPIO_Mode_AF;
  gp2.GPIO_Speed = GPIO_Speed_100MHz;
  gp2.GPIO_OType = GPIO_OType_PP;
  gp2.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOA,&gp2);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_TIM2);
}

void STM32_init_timer(){

	TIM_DeInit(TIM2);
  TIM_TimeBaseInitTypeDef tim2_base;
  tim2_base.TIM_Prescaler = 0;
  tim2_base.TIM_CounterMode = TIM_CounterMode_Up;
  tim2_base.TIM_Period = 103;
  tim2_base.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseInit(TIM2,&tim2_base);

  TIM_OCInitTypeDef tim2;
  tim2.TIM_OCMode = TIM_OCMode_PWM1;
  tim2.TIM_OutputState = TIM_OutputState_Enable;
  tim2.TIM_Pulse = 30;
  tim2.TIM_OCPolarity = TIM_OCPolarity_Low;
  TIM_OC4Init(TIM2, &tim2);
  TIM_ARRPreloadConfig(TIM2, ENABLE);
  TIM_DMAConfig(TIM2, TIM_DMABase_CCR4, TIM_DMABurstLength_1Transfer);

  
  TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);

  TIM_DMACmd(TIM2, TIM_DMA_Update, ENABLE);        // InterrupsTIM_DMA_Update
	TIM_Cmd(TIM2,ENABLE);
}

void STM32_init_dma(){

	DMA_DeInit(DMA1_Stream7);
  while (DMA_GetCmdStatus(DMA1_Stream7));
  DMA_InitTypeDef dm;
  dm.DMA_Channel = DMA_Channel_3;
  dm.DMA_PeripheralBaseAddr = (uint32_t)&(TIM2->DMAR);
  dm.DMA_Memory0BaseAddr = (uint32_t)&DMA_buf;
  dm.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  dm.DMA_BufferSize = (LEDS_NUM + 4) * COLRS * 8;
  dm.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  dm.DMA_MemoryInc = DMA_MemoryInc_Enable;
  dm.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
  dm.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
  dm.DMA_Mode = DMA_Mode_Circular;
  //dm.DMA_Mode = DMA_Mode_Normal;
  dm.DMA_Priority = DMA_Priority_High;
  dm.DMA_FIFOMode = DMA_FIFOMode_Disable;
  //dm.DMA_FIFOThreshold = DMA_FIFOThreshold_3QuartersFull;
  dm.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  dm.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    
      
  DMA_Init(DMA1_Stream7, &dm);
  DMA_ITConfig(DMA1_Stream7, DMA_IT_TC, ENABLE);
  DMA_Cmd(DMA1_Stream7, ENABLE);
    
  //NVIC_EnableIRQ(TIM2_IRQn);
  //NVIC_EnableIRQ(DMA1_Stream7_IRQn);
   // __enable_irq();

}
