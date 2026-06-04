#include "aht20.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_i2c.h"
#define  AHT20_ADDR 0x70

static uint8_t shujv[6];


//初始化函数
void AHT20_init(){
    uint8_t chushi[3] = {0xBE, 0x08, 0x00};
    uint8_t cmd[8];
    HAL_Delay(40);
    HAL_I2C_Master_Receive(&hi2c1, AHT20_ADDR, cmd, 8,100);
    if((cmd[3] & 0x08) == 0x00){
        HAL_I2C_Master_Transmit(&hi2c1, AHT20_ADDR, chushi, 3,100);
    }
}

//发送指令函数
void AHT20_fasong(){
    static uint8_t celiang[3] = {0xAC, 0x33, 0x00};
    HAL_I2C_Master_Transmit_IT(&hi2c1, AHT20_ADDR, celiang, 3);
}

//接收数据函数
void AHT20_jieshou(){
    HAL_Delay(75);
    HAL_I2C_Master_Receive_IT(&hi2c1, AHT20_ADDR, shujv, 6);
}

//计算数据函数
void AHT20_jisuan(float *wendu, float *shidu){
    if((shujv[0] & 0x80)  == 0x00){
        uint32_t shidu_data = (uint32_t)shujv[1]<<12|(uint32_t)shujv[2]<< 4|(uint32_t)shujv[3]>>4;
        uint32_t wendu_data = ((uint32_t)shujv[3] & 0x0F)<<16|(uint32_t)shujv[4]<<8|(uint32_t)shujv[5];
        *shidu = (shidu_data * 100.0f) / (1 <<20);
        *wendu = ((wendu_data * 200.0f) / (1 <<20)) - 50.0f;
    }
}