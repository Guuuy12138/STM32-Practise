#ifndef __INC_AHT20_H_
#define __INC_AHT20_H_


#include "i2c.h"
void AHT20_init();
void AHT20_fasong();
void AHT20_jieshou();
void AHT20_jisuan(float *wendu, float *shidu);



#endif /*__INC_AHT20_H_*/ 

