#define __FPU_PRESENT 1

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "stm32_init.h"

#define CHANNELS 8
#define LEDPERCANEL 18

extern const uint8_t BrightnessTable[64];
extern const uint16_t BrightnessTable2[64];
extern float32_t fCoeff[LENGTH_SAMPLES];
extern void arm_fft_window(uint8_t wind);

enum {
  SOUNDSPECTR = 0,
  COLORMUSIC = 1,
  VUMETR = 2
} mode;

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
volatile uint8_t vpeak[LEDS_NUM]; // массив для значений текущей яркости по каналам
volatile uint16_t min_value ; // минимальное пороговое значение каналов
volatile uint16_t max_value ; // максимальное пороговое значение каналов
volatile uint32_t vmetr = 0;
volatile static uint8_t cmumode = VUMETR; 
volatile static uint16_t fade = 0; // счетчик для затухания
volatile static uint16_t fadespeed = 30; // скорость затухания огней по умолчанию = fadeinitial * faderate
volatile static uint16_t rainbow; // переменная текущего цвета Hue радуги
volatile static uint8_t currentled = 0; // переменная для цму - текущий светодиод для обработки
volatile static uint8_t rainbowspeed; // счетчик для регулировки скорости смены Hue цветов радуги
volatile static uint8_t rainbowChannel; // счетчик для регулировки скорости смены Hue цветов радуги
volatile uint8_t fourChannels = 0; //4 канала для 2 режима
volatile uint16_t devider; // делитель
volatile uint8_t v_peakspeed;
volatile static uint8_t tempeffone = 0;
uint8_t v_speed = 20;
uint16_t shift_teak;
uint16_t shift_speed= 100;
volatile static uint8_t volfade = 0; // переменная для скорости затухания в режиме vu-meter

void shift (void);
__IO uint32_t SysTick_CNT = 0; //обьявляем и иницализируем в 0 значение нашего счетчика SysTick
void delay_mS(uint32_t mS);
void SetValueMode(uint8_t mode);
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
    sat_sin[i] =(((sin((2*PI*i)/((LEDS_NUM/3))))+1)*25)+13; 
  }

   LEDstrip_init();
   Convert_RGB_to_DMA_buf();
     uint16_t spectr;
   SetValueMode(VUMETR);
   arm_fft_window(4); // Создание окна Хеннинга
   CF_HSV_TypeDef HSV;
     
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
              if (spectr <= min_value) spectr = 0;
              switch (n)
                      {	
                      case 5 ... 20: capture[0] += spectr;// низкие частоты
                      break;
                      case 21 ... 45:capture[1] += spectr;
                      break;
                      case 46 ... 75: capture[2] += spectr;
                      break;
                      case 76 ... 120: capture[3] += spectr;
                      break;
                      case 121 ... 170: capture[4] += spectr;
                      break;
                      case 181 ... 220: capture[5] += spectr;
                      break;
                      case 221 ... 270: capture[6] += spectr;
                      break;
                      case 271 ... 512: capture[7] += spectr;	// высокие частоты
                      break;
      
                      }
       }
       vmetr = 0; 
       for (uint8_t y = 0; y < CHANNELS; y++) {
         capture[y] = capture[y] / devider ;
         if (capture[y] >= 64) capture[y] = 64;
         if(capture[y] >= peak[y]) peak[y] = capture[y];        // установка нового пика и счетчика паузы затухания	
         if (cmumode == 2) vmetr += capture[y];
       }
    
      //============================================================+
      // Заполнение массива цветом и яркостью для каждого	из ws2812b|
      //============================================================+		
              
      uint8_t currentled = 0;
   uint8_t currentledp = LEDPERCANEL;
      if (cmumode == 2)  {
            vmetr = vmetr / 4;
              if(vmetr > (LEDS_NUM)) vmetr = LEDS_NUM;
      }
      uint8_t hue = 150;
      uint8_t sat = 255;
        uint8_t vled =0;
          HSV.S = HSV_SAT_MAX;
    for(uint8_t y = 0; y < CHANNELS; y++) { // для каждого канала
          for(uint8_t x = 0; x < currentledp; x++) { // для каждого светодиода
            
            switch (cmumode) //режим работы цму
            {
            case 0: // постоянный цвет
               
               HSV.H = rainbow;
               LEDS_buf[currentled] = HSV_to_RGB(HSV.H, HSV.S, BrightnessTable2[peak[y]]);
              
            break;
              
            case 2:     // VUMETR
              volfade = 2;
              if (vmetr > LEDS_NUM/4) volfade = 3;	
              vled = currentled + ((LEDS_NUM)/2);
              if (vled > LEDS_NUM) vled = (LEDS_NUM-1);
                      if (currentled <= vmetr) {
                              
                              HSV.H = ABS((HSV_HUE_MAX - currentled*3)+rainbow);
                              //HSV.H = rainbow;
                              if (HSV.H > HSV_HUE_MAX) HSV.H -= HSV_HUE_MAX;
                              HSV.V = 250;
                              vpeak[currentled]=63;
                         
                              } 
                            LEDS_buf[vled]=HSV_to_RGB(HSV.H, HSV.S, BrightnessTable2[vpeak[currentled] & sat_sin[LEDS_NUM - currentled]]);
                            LEDS_buf[LEDS_NUM-vled-1]=HSV_to_RGB(HSV.H, HSV.S, BrightnessTable2[vpeak[currentled]& sat_sin[LEDS_NUM - currentled]]);
                              //shift();
              break;
          }
           currentled++;
           if (currentled > LEDS_NUM) currentled = 0;
          }
         
    }
                
     Convert_RGB_to_DMA_buf();                             
     ADC_flag_fft = 0; 
       //shift();
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

void SysTick_Handler(void)
{
  if(SysTick_CNT > 0)  SysTick_CNT--;
}

void TIM3_IRQHandler(void) // Частота прерывания 1 мс
{
  if (shift_teak)
    shift_teak--;
  else{
    shift();
      shift_teak = shift_speed;
  }
  if (rainbowspeed) rainbowspeed--;
	if (rainbowspeed == 0) {
		rainbow++;
                  if (rainbow > HSV_HUE_MAX) {
                    rainbow = 0;
                      
                  }
		rainbowspeed = rainbowChannel;	
                  //shift();
	}
  
  if( fade >= fadespeed) { // счетчик циклов, при сробатывании обнуляется. чем он меньше, тем чаще будет происходить "затухание" при отсутствии нового "пика"
          for(uint8_t y = 0; y < CHANNELS; y++) if(peak[y]) peak[y]--;
          fade = 0;
  }
    
  // затухание для режима громкость
  if ((cmumode == VUMETR)) {
      for(uint8_t y = LEDS_NUM; y > 0; y--) {
        if (vpeak[y] > 2) {
          vpeak[y] -= 2;
           break;
        }
        else continue;

        //if (vpeak[y] >= volfade) vpeak[y]--; 
        //else vpeak[y] = 0;
      }
  }
  v_peakspeed=0;
  fade++; // счетчик для затухания
  TIM_ClearFlag(TIM3, TIM_FLAG_Update);
}

void DMA2_Stream0_IRQHandler(void)
{
  // Подготовка входных данных
  arm_q31_to_float(ADC_buf, ADC_arr, LENGTH_SAMPLES);
  for (uint16_t i = 0; i < LENGTH_SAMPLES; i++)
  {
    ADC_fft[pos] = ADC_arr[i]*fCoeff[i];        // Применение окна Хеннинга
    ADC_fft[pos+1] = 0;
    pos += 2;
  }
  pos =0;
  //shift();
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
  uint32_t ifftFlag = 0; 
  uint32_t doBitReverse = 1; 
  arm_cfft_radix4_instance_f32 S;

  if (arm_cfft_radix4_init_f32(&S, LENGTH_SAMPLES,ifftFlag, doBitReverse) == ARM_MATH_SUCCESS)
  {
    memset (Spectr_fft, 0, sizeof(Spectr_fft));
    arm_cfft_radix4_f32(&S, ADC_fft);         // Вычисляем FFT
    arm_cmplx_mag_f32(ADC_fft, Spectr_fft, LENGTH_SAMPLES);       // Преобразуем значение FFT в комлексный формат float32
    arm_float_to_q31(Spectr_fft, Spectr_fft_q31, LENGTH_SAMPLES);     
  }
}

void SetValueMode(uint8_t mode){
	
	if (mode == 0)
	{
		devider = 9370;
		min_value = 2200;
		fourChannels = 0;
	}
	if (mode == 1) {
		devider = 150;
		min_value = 2000;
		fourChannels = 4;
		rainbowChannel = 18;
	}
	if (mode == 2){
		devider = 7000;
		min_value = 2000;
		fourChannels = 0;
		rainbowChannel = 50;
	}
	
}

void shift (void) {
		tempeffone = sat_sin[0];
		for (uint8_t y = 0; y < (LEDS_NUM-1); y++)	{
			sat_sin[y] = sat_sin[y + 1];
		}
		sat_sin[(LEDS_NUM-1)] = tempeffone;
}
#ifdef USE_FULL_ASSERT

void assert_failed(u8* file, u32 line)
{
}
#endif