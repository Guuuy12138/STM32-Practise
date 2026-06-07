#include "aht20.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_i2c.h"
#define  AHT20_ADDR 0x70     /* AHT20 I2C 地址（左移1位，含读写位） */

static uint8_t shujv[6];     /* AHT20 返回的原始6字节数据缓冲区 */


//初始化函数：检查校准位，未校准则发送初始化命令
void AHT20_init(){
    uint8_t chushi[3] = {0xBE, 0x08, 0x00};   /* 初始化命令：0xBE(初始化) 0x08(上电校准) 0x00(保留) */
    uint8_t cmd[8];
    HAL_Delay(40);                              /* 等待 AHT20 上电稳定（至少 20ms） */
    HAL_I2C_Master_Receive(&hi2c1, AHT20_ADDR, cmd, 8,100);  /* 读取状态寄存器（8字节） */
    if((cmd[3] & 0x08) == 0x00){                /* 检查 bit[3] 校准使能位，为0表示未校准 */
        HAL_I2C_Master_Transmit(&hi2c1, AHT20_ADDR, chushi, 3,100); /* 发送初始化校准命令 */
    }
}

//发送指令函数：触发 AHT20 测量（中断方式）
void AHT20_fasong(){
    static uint8_t celiang[3] = {0xAC, 0x33, 0x00};  /* 测量命令：0xAC(触发测量) 0x33(普通模式) 0x00(保留) */
    HAL_I2C_Master_Transmit_IT(&hi2c1, AHT20_ADDR, celiang, 3);  /* 中断方式发送，发送完成回调设状态=2 */
}

//接收数据函数：等待测量完成，读取6字节原始数据（中断方式）
void AHT20_jieshou(){
    HAL_Delay(75);                               /* 等待 AHT20 测量完成（通常约 80ms） */
    HAL_I2C_Master_Receive_IT(&hi2c1, AHT20_ADDR, shujv, 6);  /* 中断方式接收，接收完成回调设状态=4 */
}

//计算数据函数：将原始6字节数据换算为温湿度
//数据格式：[状态] [湿度高8] [湿度中8] [湿度低4+温度高4] [温度中8] [温度低8]
void AHT20_jisuan(float *wendu, float *shidu){
    if((shujv[0] & 0x80)  == 0x00){              /* 检查 bit[7] 忙状态位，0=空闲（数据有效） */
        /* 湿度 = 20位湿度原始值 / 2^20 * 100% */
        uint32_t shidu_data = (uint32_t)shujv[1]<<12|(uint32_t)shujv[2]<< 4|(uint32_t)shujv[3]>>4;
        /* 温度 = 20位温度原始值 / 2^20 * 200 - 50℃ */
        uint32_t wendu_data = ((uint32_t)shujv[3] & 0x0F)<<16|(uint32_t)shujv[4]<<8|(uint32_t)shujv[5];
        *shidu = (shidu_data * 100.0f) / (1 <<20);
        *wendu = ((wendu_data * 200.0f) / (1 <<20)) - 50.0f;
    }
}