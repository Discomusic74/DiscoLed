#include "ws2812b.h"
#include "stm32_init.h"

void Delay_ms(uint32_t n)
{
  uint32_t i;
  for (i=0;i<(n*4);i++)
  {
    memcpy((void*)buf1, (void*)buf2, ASIZE); // Задержка 250 мкс
  }
}

// Sloppy delay function (not accurate)
void Delay(__IO uint32_t nCount) { while(nCount--) { } }

/*------------------------------------------------------------------------------
  ??????????? ??????????? ? ???????? ??????? ? ???????? ????????? ? ???????? ?????????? ?????
------------------------------------------------------------------------------*/
void Moving_leds(uint32_t n, uint32_t dir, uint32_t del)
{
  uint32_t i, k;
  uint32_t tmpLED;

  for (k = 0; k < n; k++)
  {
    //Convert_RGB_to_DMA_buf();
    //Convert_RGB_to_DMA_buf_Mirror();
      // ?????????? ????
      tmpLED = LEDS_buf[0];
      for (i = 0; i < LEDS_NUM; i++)
      {
        if ( i != (LEDS_NUM - 1) )
        {
          LEDS_buf[i] = LEDS_buf[i + 1];
        }
        else
        {
          LEDS_buf[i] = tmpLED;
        }
          shift();
      }
        Convert_RGB_to_DMA_buf_Mirror();
        Delay_ms(del);
            //shift();
    }
}


/*------------------------------------------------------------------------------
  ????????? ??????? 
 ------------------------------------------------------------------------------*/
static void Fill_rainbow(uint32_t sat, uint32_t val)
{
  uint32_t i;
  uint32_t hue;

    // ?????????? ? ????? ????????? ??????????????????
    for (i = 0; i < LEDS_NUM; i++)
    {
       
      hue = ((360 * i) / LEDS_NUM);
			
        LEDS_buf[i] = HSV_to_RGB(hue, sat, BrightnessTable[val_sin[i]]);
	
         // LEDS_buf[i] = HSV_to_RGB(hue, sat, val);
            //shift();						
    }
}

void Fill_color(uint32_t color)
{
  uint32_t i;
  uint32_t hue;

    // ?????????? ? ????? ????????? ??????????????????
    for (i = 0; i < LEDS_NUM; i++)
    {
       
      hue = color ;		
      LEDS_buf[i] = HSV_to_RGB(hue, BrightnessTable[sat_sin[i]], BrightnessTable[val_sin[i]]);
      //LEDS_buf[i] = HSV_to_RGB(hue, BrightnessTable[sat_sin[i]], 255);
         // LEDS_buf[i] = HSV_to_RGB(hue, sat, val);
            shift();						
    }
}

/*------------------------------------------------------------------------------
  ??????? ?????? 
 ------------------------------------------------------------------------------*/
void Running_LEDs_1(void)
{
  
    
    Fill_rainbow(255, 255);
    Moving_leds(LEDS_NUM, 0, 100);
    //Fill_rainbow(255, 255, 1);
    //Moving_leds(LEDS_NUM * 2, 0, 1000);

}

void LEDstrip_init(void)
{
  uint32_t i, j, k;

  for (i = 0; i < LEDS_NUM; i++)
  {
    for (j = 0; j < COLRS; j++)
    {
      for (k = 0; k < 8; k++)
      {
        DMA_buf[i][j][k] = LED_LEV0;
      }
    }
  }
  // ?????? ??????
  for (i = LEDS_NUM; i < LEDS_NUM + 2; i++)
  {
    for (j = 0; j < COLRS; j++)
    {
      for (k = 0; k < 8; k++)
      {
        DMA_buf[i][j][k] = 0;
      }
    }
  }
}


/*------------------------------------------------------------------------------
  ?????????? ?? HSV ? RGB ? ????????????? ??????????
 
  hue        : 0..360
  saturation : 0..255
  value      : 0..255
 ------------------------------------------------------------------------------*/
uint32_t HSV_to_RGB(int hue, int sat, int val) 
{
  int    r;
  int    g;
  int    b;
  int    base;
  uint32_t rgb;

  val = dim_curve[val];
  sat = 255 - dim_curve[255 - sat];


  if ( sat == 0 ) // Acromatic color (gray). Hue doesn't mind.
  {
    rgb = val | (val<<8) | (val <<16);
  }
  else
  {
    base = ((255 - sat) * val) >> 8;
    switch (hue / 60)
    {
    case 0:
      r = val;
      g = (((val - base) * hue) / 60) + base;
      b = base;
      break;
    case 1:
      r = (((val - base) * (60 - (hue % 60))) / 60) + base;
      g = val;
      b = base;
      break;
    case 2:
      r = base;
      g = val;
      b = (((val - base) * (hue % 60)) / 60) + base;
      break;
    case 3:
      r = base;
      g = (((val - base) * (60 - (hue % 60))) / 60) + base;
      b = val;
      break;
    case 4:
      r = (((val - base) * (hue % 60)) / 60) + base;
      g = base;
      b = val;
      break;
    case 5:
      r = val;
      g = base;
      b = (((val - base) * (60 - (hue % 60))) / 60) + base;
      break;
    }
    rgb = ((r & 0xFF)<<16) | ((g & 0xFF)<<8) | (b & 0xFF);
  }
  return rgb;
}


void LEDstrip_set_led_state(uint16_t ledn, uint32_t color)
{
  uint32_t i;
  if ( ledn >= LEDS_NUM ) return;
  for (i = 0; i < 8; i++)
  {
    // ???????
    if ( ((color >> 8) >> (7 - i)) & 1 ) DMA_buf[ledn][0][i] = LED_LEV1;
    else DMA_buf[ledn][0][i] = LED_LEV0;
    // ???????
    if ( ((color >> 16) >> (7 - i)) & 1 ) DMA_buf[ledn][1][i] = LED_LEV1;
    else DMA_buf[ledn][1][i] = LED_LEV0;
    // ?????
    if ( ((color >> 0) >> (7 - i)) & 1 ) DMA_buf[ledn][2][i] = LED_LEV1;
    else DMA_buf[ledn][2][i] = LED_LEV0;
  }
}


/*------------------------------------------------------------------------------
   ???????????? RGB ?????? ? DMA ??????
 ------------------------------------------------------------------------------*/
void Convert_RGB_to_DMA_buf(void)
{
  uint32_t i;
  for (i = 0; i < LEDS_NUM; i++)
  {
    LEDstrip_set_led_state(i, LEDS_buf[i]);
  }
}

/*------------------------------------------------------------------------------
   ???????????? RGB ?????? ? DMA ?????? ?????????
 ------------------------------------------------------------------------------*/
void Convert_RGB_to_DMA_buf_Mirror(void)
{
  uint32_t i;
  for (i = 0; i < LEDS_NUM/2; i++)
  {
    LEDstrip_set_led_state(i, LEDS_buf[i]);
    LEDstrip_set_led_state(LEDS_NUM - i, LEDS_buf[i]);
  }
}


uint32_t HSV_to_RGB_double(uint32_t hue, uint32_t saturation, uint32_t value) 
{
    uint8_t rgb[3]; 
  	double r, g, b;
	
    double h =  (double)hue/360.0;
	double s = (double)saturation/255.0;
  	double v = (double)value/255.0;
	
    int i = (int)(h * 6);
    double f = h * 6 - i;
    double p = v * (1 - s);
    double q = v * (1 - f * s);
    double t = v * (1 - (1 - f) * s);

    switch(i % 6){
        case 0: r = v, g = t, b = p; break;
        case 1: r = q, g = v, b = p; break;
        case 2: r = p, g = v, b = t; break;
        case 3: r = p, g = q, b = v; break;
        case 4: r = t, g = p, b = v; break;
        case 5: r = v, g = p, b = q; break;
    }

    rgb[0] = r * 255;
    rgb[1] = g * 255;
    rgb[2] = b * 255;
		return ((rgb[0] & 0xFF)<<16) | ((rgb[1] & 0xFF)<<8) | (rgb[2] & 0xFF);
}
