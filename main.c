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
    
 
  GPIO_InitTypeDef gp2;     // PA1 out for Timer2_Channel2
  gp2.GPIO_Pin = GPIO_Pin_3;
  gp2.GPIO_Mode = GPIO_Mode_AF;
  gp2.GPIO_Speed = GPIO_Speed_100MHz;
  gp2.GPIO_OType = GPIO_OType_PP;
  gp2.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA,&gp2);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_TIM2);

  RCC_ClocksTypeDef  rcc;   // Frequence SysClock
  RCC_GetClocksFreq(&rcc);
  uint32_t sysclk = rcc.SYSCLK_Frequency;
  uint32_t hclk = rcc.HCLK_Frequency;
  uint32_t getsys = RCC_GetSYSCLKSource();

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);  // Timer2
  //RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);  // Timer1

  RCC_PCLK1Config(RCC_HCLK_Div2);

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
  //TIM2->ARR = 105 - 1;          // Перегрузка таймера каждые 1.25 мкс
  //TIM_SetAutoreload(TIM2, 103);  // Перегрузка таймера каждые 1.25 мкс
  TIM_OC4Init(TIM2, &tim2);
  //TIM_CounterModeConfig(TIM2, TIM_CounterMode_Up);
  TIM_ARRPreloadConfig(TIM2, ENABLE);
  //TIM2->DCR &= ~TIM_DCR_DBA;
  //TIM2->DCR &= ~TIM_DCR_DBL;
  TIM_DMAConfig(TIM2, TIM_DMABase_CCR4, TIM_DMABurstLength_1Transfer);
  TIM_SelectCCDMA(TIM2, ENABLE);
  TIM_UpdateRequestConfig(TIM2, TIM_UpdateSource_Regular);
  TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);
    //TIM2->CCMR1 &= ~TIM_CCMR1_CC1S;
  //TIM_PrescalerConfig(TIM2,0, TIM_PSCReloadMode_Immediate);
    
      //TIM2->EGR |= TIM_EGR_UG;
    // Ожидание сброса бита UG:
    //while((TIM1->EGR & TIM_EGR_UG) == SET){}
    // Разрешение UEV установкой бита UG для загрузки данных
    // из предзагрузки в активные регистры:
    //TIM2->EGR |= TIM_EGR_UG;
        
//TIM_DIER_UIE
            
    //TIM2->EGR |= TIM_EGR_UG;//генерировать событие UPDATE
    //TIM_GenerateEvent(TIM2, TIM_EventSource_CC2);  // TIM_EventSource_CC2
    //TIM_GenerateEvent(TIM2, TIM_EventSource_Update);
      //while((TIM2->EGR & TIM_EGR_UG) == 1){}
    //TIM_Cmd(TIM2,ENABLE);
  //TIM_CCxCmd(TIM2, TIM_Channel_4, TIM_CCx_Enable);

  //TIM_DMACmd(TIM2, TIM_DMA_CC4, ENABLE);        // Interrups
  TIM_DMACmd(TIM2, TIM_DMA_Update, ENABLE);        // InterrupsTIM_DMA_Update
  //TIM_ITConfig(TIM2, TIM_IT_CC2, ENABLE);
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM2,ENABLE);
  //NVIC_EnableIRQ(TIM2_IRQn);
  //NVIC_EnableIRQ(DMA1_Stream6_IRQn);
    //TIM2->CCMR1 |= TIM_CCMR1_OC2PE;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);  // DMA
  //--
  uint32_t DMA_buf = 10;
  //uint16_t DMA_buf[1] = {50};
   //for (int i = 0; i < (sizeof(DMA_buf)/sizeof(uint16_t)); i++)
   //{
   //  DMA_buf[i]=i;
   //}
  //DMA_ITConfig(DMA1_Stream7, DMA_IT_TC, ENABLE);
   // TIM2->DMAR = 0x01;
  DMA_DeInit(DMA1_Stream7);
  while (DMA_GetCmdStatus(DMA1_Stream7));
  DMA_InitTypeDef dm;
  dm.DMA_Channel = DMA_Channel_3;
  dm.DMA_PeripheralBaseAddr = (unsigned int)&(TIM2->DMAR);
  dm.DMA_Memory0BaseAddr = (uint32_t)&DMA_buf;
  dm.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  dm.DMA_BufferSize = 1;
  dm.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  dm.DMA_MemoryInc = DMA_MemoryInc_Disable;
  dm.DMA_PeripheralDataSize = DMA_MemoryDataSize_Word;
  dm.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
  //dm.DMA_Mode = DMA_Mode_Circular;
  dm.DMA_Mode = DMA_Mode_Normal;
  dm.DMA_Priority = DMA_Priority_High;
  //dm.DMA_FIFOMode = DMA_FIFOMode_Disable;
  //dm.DMA_FIFOThreshold = DMA_FIFOThreshold_3QuartersFull;
  dm.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  dm.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA1_Stream7, &dm);
  DMA_Cmd(DMA1_Stream7, ENABLE);
    
  //NVIC_EnableIRQ(TIM2_IRQn);
  //NVIC_EnableIRQ(DMA1_Stream7_IRQn);
   // __enable_irq();



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

void DMA1_Stream7_IRQHandler(void){
  DMA_TypeDef *dma = DMA1;

  dma->HIFCR = (1<<27); // CTCIF7: Stream 7 clear transfer complete interrupt flag. Сброс флага прерывания
}

void TIM2_IRQHandler(void){
  TIM2->SR = 0;
}

#ifdef USE_FULL_ASSERT

void assert_failed(u8* file, u32 line)
{
}
#endif


