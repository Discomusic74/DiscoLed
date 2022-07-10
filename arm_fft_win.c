#include "arm_fft_win.h"

void arm_fft_window(uint8_t wind)
{
      for (uint16_t iCnt = 0; iCnt < LENGTH_SAMPLES; iCnt++)
      {
         switch (wind)
            {
          case 0: fCoeff[iCnt] = 1.0; 
          break;
          case 1: fCoeff[iCnt] = sinf(dTmpFloat * iCnt);//sin window
          break;
          case 2: fCoeff[iCnt] = (0.42 - 0.5 * cosf(dTmpFloat * iCnt) + 0.08 * cosf(2.0 * dTmpFloat * iCnt));//????????
          break;
          case 3: fCoeff[iCnt] = (0.54 - 0.46 * cosf(dTmpFloat * iCnt));//????????
          break;
          case 4: fCoeff[iCnt] = (0.5 - 0.5*cosf(dTmpFloat * iCnt));//????????
          break;
          case 5: fCoeff[iCnt] = (0.35875 - 0.48829*cosf(dTmpFloat * iCnt) + 0.14128*cosf(2.0 * dTmpFloat * iCnt) - 0.01168*cosf(3.0 * dTmpFloat * iCnt));//????????-???????
          break;
          case 6: fCoeff[iCnt] = (0.355768 - 0.487396*cosf(dTmpFloat * iCnt) + 0.144232*cosf(2.0 * dTmpFloat * iCnt) - 0.012604*cosf(3.0 * dTmpFloat * iCnt));//???????
          break;
          case 7: fCoeff[iCnt] = (0.3635819 - 0.4891775*cosf(dTmpFloat * iCnt) + 0.1365995*cosf(2.0 * dTmpFloat * iCnt) - 0.0106411*cosf(3.0 * dTmpFloat * iCnt));//????????-???????
          break;
            }
      }
}