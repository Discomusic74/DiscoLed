#include "stm32_init.h"

void SysTick_Init(void)
{
  SysTick->LOAD &= ~SysTick_LOAD_RELOAD_Msk; //сбрасываем возможные старые значения интервала счета в 0
  SysTick->LOAD = SYSCLOCK/1000 - 1; //правильнее без скобки, досадная очепятка. Правильно так.
  SysTick->VAL &= ~SysTick_VAL_CURRENT_Msk; //сбрасываем текущие значения счетчика в 0
  SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;//запуск счетчика
  //выбрали частоту синхронизации - от тактирования процессора(АНВ),разрешили прерывания и включили счетчик.
}

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
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);  // Timer3 RCC_APB1Periph_TIM3
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);  // DMA 
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); // ADC1
 
}

void STM32_init_gpio(){

 GPIO_InitTypeDef gp;
  gp.GPIO_Pin = GPIO_Pin_14;
  gp.GPIO_Mode = GPIO_Mode_IN;
  gp.GPIO_Speed = GPIO_Speed_100MHz;
  //gp.GPIO_OType = GPIO_OType_OD;
  gp.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOC,&gp);
    
 
  GPIO_InitTypeDef gp2;     // PA1 out for Timer2_Channel2
  gp2.GPIO_Pin = GPIO_Pin_3;
  gp2.GPIO_Mode = GPIO_Mode_AF;
  gp2.GPIO_Speed = GPIO_Speed_100MHz;
  gp2.GPIO_OType = GPIO_OType_PP;
  gp2.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOA,&gp2);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_TIM2);

  GPIO_InitTypeDef gp3;     // PA7 for ADC1
  gp3.GPIO_Pin = GPIO_Pin_7;
  gp3.GPIO_Mode = GPIO_Mode_AN;
  gp3.GPIO_Speed = GPIO_Speed_100MHz;
  //gp3.GPIO_OType = GPIO_OType_PP;
  //gp3.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA,&gp3);
  //GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_TIM2);


}

void ISR_init_timer(){

  TIM_DeInit(TIM3);
  TIM_TimeBaseInitTypeDef tim3_base;
  tim3_base.TIM_Prescaler = 0;
  tim3_base.TIM_CounterMode = TIM_CounterMode_Up;
  tim3_base.TIM_Period = 21000;
  tim3_base.TIM_ClockDivision = TIM_CKD_DIV4;
  TIM_TimeBaseInit(TIM3,&tim3_base);
    
  TIM_ARRPreloadConfig(TIM3, ENABLE);
  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
  NVIC_EnableIRQ(TIM3_IRQn);
  TIM_Cmd(TIM3,ENABLE);

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

void STM32_init_dma_timer(){
  
  DMA_DeInit(DMA1_Stream7);
  while (DMA_GetCmdStatus(DMA1_Stream7));
  DMA_InitTypeDef dm;
  dm.DMA_Channel = DMA_Channel_3;
  dm.DMA_PeripheralBaseAddr = (uint32_t)&(TIM2->DMAR);
  dm.DMA_Memory0BaseAddr = (uint32_t)&DMA_buf;
  dm.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  dm.DMA_BufferSize = (LEDS_NUM + 2) * COLRS * 8;
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

void STM32_init_adc(){

    ADC_DeInit();
    ADC_InitTypeDef adc;
    adc.ADC_Resolution = ADC_Resolution_12b;
    adc.ADC_ScanConvMode = DISABLE;
    adc.ADC_ContinuousConvMode = DISABLE;
    adc.ADC_DataAlign = ADC_DataAlign_Right;
    adc.ADC_NbrOfConversion = ADC_Channel_7;
    ADC_Init(ADC1, &adc);

    ADC_CommonInitTypeDef adc1;
    adc1.ADC_Mode = ADC_Mode_Independent;
    adc1.ADC_Prescaler = ADC_Prescaler_Div6;    // 84 Мгц / 6 = 14 МГц
    adc1.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
    adc1.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_20Cycles;
    ADC_CommonInit(&adc1);

    //ADC_ContinuousModeCmd(ADC1, ENABLE);
    //ADC_DiscModeCmd(ADC1, ENABLE);

    ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
    ADC_DMACmd(ADC1, ENABLE);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 1, ADC_SampleTime_480Cycles); // 14 МГц / 480 = 29166 Гц Частота дискретизации
                                                                                // По теореме Котельникова граничная частота отображения 29166 / 2 = 14583 Гц
    ADC_ITConfig(ADC1, ADC_IT_OVR, ENABLE);                     
    ADC_EOCOnEachRegularChannelCmd(ADC1, ENABLE);

    ADC_AnalogWatchdogCmd(ADC1, DISABLE);

    ADC_Cmd(ADC1, ENABLE);

    ADC_SoftwareStartConv(ADC1);

    //NVIC_EnableIRQ(ADC_IRQn);
}

void STM32_init_dma_adc(){

    DMA_DeInit(DMA2_Stream0);
  while (DMA_GetCmdStatus(DMA2_Stream0));
  DMA_InitTypeDef dm;
  dm.DMA_Channel = DMA_Channel_0;
  dm.DMA_PeripheralBaseAddr = (uint32_t)&(ADC1->DR);
  dm.DMA_Memory0BaseAddr = (uint32_t)&ADC_buf;
  dm.DMA_DIR = DMA_DIR_PeripheralToMemory;
  dm.DMA_BufferSize = LENGTH_SAMPLES;
  dm.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  dm.DMA_MemoryInc = DMA_MemoryInc_Enable;
  dm.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
  dm.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
  dm.DMA_Mode = DMA_Mode_Circular;
  //dm.DMA_Mode = DMA_Mode_Normal;
  dm.DMA_Priority = DMA_Priority_High;
  dm.DMA_FIFOMode = DMA_FIFOMode_Enable;
  dm.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  dm.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  dm.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    
      
  DMA_Init(DMA2_Stream0, &dm);
  DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE);
  DMA_Cmd(DMA2_Stream0, ENABLE);
    
  NVIC_EnableIRQ(DMA2_Stream0_IRQn);
}