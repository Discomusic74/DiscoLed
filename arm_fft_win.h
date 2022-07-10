#ifndef __ARM_FFT_WIN
#define __ARM_FFT_WIN

#include "stm32_init.h"
#include <math.h>

float32_t fCoeff[LENGTH_SAMPLES];
float32_t dTmpFloat = 2.0 * PI / (LENGTH_SAMPLES - 1.0);

void arm_fft_window(uint8_t wind);


#endif