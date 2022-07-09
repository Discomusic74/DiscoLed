#define __FPU_PRESENT 1

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
//#include "ws2812b.h" 
#include "stm32_init.h"

#define CHANNELS 8
#define LEDPERCANEL 18

extern const uint8_t BrightnessTable[64];
extern const uint16_t BrightnessTable2[64];

uint32_t DMA_buf[LEDS_NUM+2][COLRS][8];
q31_t ADC_buf[LENGTH_SAMPLES];
float32_t ADC_arr[LENGTH_SAMPLES];
float32_t ADC_fft[LENGTH_SAMPLES*2];
float32_t Spectr_fft[LENGTH_SAMPLES];
q31_t Spectr_fft_q31[LENGTH_SAMPLES];
uint32_t capture[8]; 
uint32_t LEDS_buf[LEDS_NUM]; 
uint8_t val_sin[LEDS_NUM];
uint8_t sat_sin[LEDS_NUM];
uint8_t ADC_flag_fft = 0;
static uint16_t pos= 0;
volatile uint8_t peak[CHANNELS]; // массив для значений текущей яркости по каналам

volatile static uint16_t fade = 0; // счетчик для затухания
volatile static uint16_t fadespeed = 30; // скорость затухания огней по умолчанию = fadeinitial * faderate

extern float32_t testInput_f32_10khz[2048]; 

__IO uint32_t SysTick_CNT = 0; //обьявляем и иницализируем в 0 значение нашего счетчика SysTick
void delay_mS(uint32_t mS);
static void DoFFT();


  int main(void)
{
  STM32_init_rcc();
  STM32_init_dma_timer();
  STM32_init_timer();
  STM32_init_gpio();
  STM32_init_adc();
  STM32_init_dma_adc();
  ISR_init_timer();
    
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
     uint16_t spectr;
     
   //DoFFT();
   
   while (1)
  {
    if (ADC_flag_fft)
   {
     DoFFT();
       
     //чистка массива перед повторным использованием
     memset (capture, 0, sizeof(capture));   
     
     //==================================================+
     // Разложение спектра на каналы						|
     //==================================================+		
    
      for (uint16_t n = 2; n < LENGTH_SAMPLES / 2; n++) {
              spectr = Spectr_fft_q31[n];
              if (spectr <= 3000) spectr = 0;
              switch (n)
                      {	
                      case 2 ... 20: capture[0] += spectr;// низкие частоты
                      break;
                      case 21 ... 41:capture[1] += spectr;
                      break;
                      case 42 ... 70: capture[2] += spectr;
                      break;
                      case 81 ... 150: capture[3] += spectr;
                      break;
                      case 151 ... 299: capture[4] += spectr;
                      break;
                      case 300 ... 339: capture[5] += spectr;
                      break;
                      case 400 ... 448: capture[6] += spectr;
                      break;
                      case 449 ... 512: capture[7] += spectr;	// высокие частоты
                      break;
      
                      }
       }
         
       for (uint8_t y = 0; y < CHANNELS; y++) {
         capture[y] = capture[y] / 9375 ;
         if (capture[y] >= 64) capture[y] = 64;
         if(capture[y] >= peak[y]) peak[y] = capture[y];        // установка нового пика и счетчика паузы затухания	
       }
    
      //============================================================+
      // Заполнение массива цветом и яркостью для каждого	из ws2812b|
      //============================================================+		
              
      uint8_t currentled = 0;
   uint8_t currentledp = LEDPERCANEL;
      
    for(uint8_t y = 0; y < CHANNELS; y++) { // для каждого канала
          for(uint8_t x = 0; x < currentledp; x++) { // для каждого светодиода
            
            
           uint8_t hue = 150;
           uint8_t sat = 255;
           
           LEDS_buf[currentled] = HSV_to_RGB(hue, sat, BrightnessTable2[peak[y]]);
           currentled++;
          if (currentled > LEDS_NUM) currentled = 0;
          }
         
    }
                
     Convert_RGB_to_DMA_buf();                             
     ADC_flag_fft = 0; 
    } else 
   {
     ADC_SoftwareStartConv(ADC1);
   }
      //uint16_t i = 0;
    //Running_LEDs_1();
      //Fill_color(240);  
      //Moving_leds(LEDS_NUM, 0, 80);
         //GPIO_SetBits(GPIOC, GPIO_Pin_13);
      //delay_mS(1000);
      //GPIO_ResetBits(GPIOC, GPIO_Pin_13);
      //delay_mS(1000);
      //i = ADC_GetConversionValue(ADC1);
      //printf ("%d\n",hclk);
     // ADC_SoftwareStartConv(ADC1);
     
     //Convert_RGB_to_DMA_buf();
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

void TIM3_IRQHandler(void) // Частота прерывания 1 мс
{
  if( fade >= fadespeed) { // счетчик циклов, при сробатывании обнуляется. чем он меньше, тем чаще будет происходить "затухание" при отсутствии нового "пика"
          for(uint8_t y = 0; y < CHANNELS; y++) if(peak[y]) peak[y]--;
          fade = 0;
  }
  fade++; // счетчик для затухания
  TIM_ClearFlag(TIM3, TIM_FLAG_Update);
}

void DMA2_Stream0_IRQHandler(void)
{
   //uint32_t val;
  //memcpy(ADC_arr, ADC_buf, sizeof(ADC_buf));
         // 1. Подготовка входных данных
  arm_q31_to_float(ADC_buf, ADC_arr, LENGTH_SAMPLES);
  for (uint16_t i = 0; i < LENGTH_SAMPLES; i++)
  {
    ADC_fft[pos] = ADC_arr[i];
    ADC_fft[pos+1] = 0;
    pos += 2;
  }
    pos =0;
  //memset (ADC_buf, 0, sizeof(ADC_buf));
  ADC_flag_fft = 1;
  DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0);  // Сброс флага прерывания

}

void delay_mS(uint32_t mS)
{
  SysTick->VAL &= ~SysTick_VAL_CURRENT_Msk;//сбрасываем старые возможные значения текущего счета в 0
  SysTick->VAL = SYSCLOCK / 1000 - 1; // скобки убраны, очепятка. Правильнее в таком формате
  SysTick_CNT = mS;
  while(SysTick_CNT) {}// как только будет 0, то выходим из цикла и задержка заканчивается
}

static void DoFFT()
{
  uint16_t spectr;
  uint32_t ifftFlag = 0; 
  uint32_t doBitReverse = 1; 
  uint32_t refIndex = 213, testIndex = 0; 
    arm_status status; 
      
  //float32_t maxValue; 
        
      //arm_rfft_instance_f32 S; 
      arm_cfft_radix4_instance_f32 S;

    if (arm_cfft_radix4_init_f32(&S, LENGTH_SAMPLES,0, 1) == ARM_MATH_SUCCESS)
    {
      memset (Spectr_fft, 0, sizeof(Spectr_fft));
      arm_cfft_radix4_f32(&S, ADC_fft);         // Вычисляем FFT
        //arm_cfft_radix4_f32(&S, testInput_f32_10khz);
          arm_cmplx_mag_f32(ADC_fft, Spectr_fft, LENGTH_SAMPLES);       // Преобразуем значение FFT в комлексный формат float32
      //arm_cmplx_mag_f32(testInput_f32_10khz, Spectr_fft, LENGTH_SAMPLES);
        arm_float_to_q31(Spectr_fft, Spectr_fft_q31, LENGTH_SAMPLES);
            
    }

              
      //memset (ADC_arr, 0, sizeof(ADC_arr));
    //status = arm_cfft_radix4_init_f32(&S, fftSize,  ifftFlag, doBitReverse); 
	 
	/* Process the data through the CFFT/CIFFT module */ 
	//arm_cfft_radix4_f32(&S, ADC_arr); 
	 
	// memset (ADC_fft, 0, sizeof(ADC_fft));
	/* Process the data through the Complex Magnitude Module for  
	calculating the magnitude at each bin */ 
	//arm_cmplx_mag_f32(ADC_arr, ADC_fft, fftSize);    
        
         //memset (ADC_arr, 0, sizeof(ADC_arr));

}

