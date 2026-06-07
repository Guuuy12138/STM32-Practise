/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"     /* OLED 显示屏驱动 */
#include"string.h"    /* 字符串操作库 */
#include"stdio.h"     /* 标准输入输出库，用于 sprintf */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  int channel_index = 0;                                    /* 当前PWM通道索引(0=CH1,1=CH2,2=CH3) */
  uint32_t channels[3] = {TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3}; /* 三个PWM通道供切换 */
  HAL_Delay(20);                                            /* 等待OLED上电稳定 */
  OLED_Init();                                              /* 初始化OLED显示屏 */
  HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_ALL);           /* 启动TIM1编码器模式读取旋转编码器 */
  HAL_TIM_PWM_Start(&htim3, channels[channel_index]);       /* 启动TIM3当前通道PWM输出 */
  int count = 0;                                            /* 编码器计数值 */
  char message[20] = "";                                    /* OLED显示缓冲区 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    OLED_NewFrame();                                                          /* 开始新一帧绘制 */
    count = __HAL_TIM_GET_COUNTER(&htim1);                                    /* 读取编码器当前计数值 */
    if(count > 6000){                                                         /* 超过6000则归零（防溢出） */
      count = 0;
      __HAL_TIM_SET_COUNTER(&htim1, 0);
    }else if(count >100){                                                     /* 上限钳位到100（进度条范围） */
      count = 100;
      __HAL_TIM_SET_COUNTER(&htim1, 100);
    }
    sprintf(message, "Count: %d", count);                                     /* 格式化计数值 */
    OLED_PrintString(13, 0, message, &font16x16, OLED_COLOR_NORMAL);          /* 显示数字 */
    OLED_DrawRectangle(13, 25, 101, 12, OLED_COLOR_NORMAL);                   /* 绘制进度条外框 */
    OLED_DrawFilledRectangle(14, 26, count, 11, OLED_COLOR_NORMAL);           /* 绘制进度条填充 */
    if(HAL_GPIO_ReadPin(Key_GPIO_Port, Key_Pin) == GPIO_PIN_RESET){           /* 按键按下（低电平有效） */
      HAL_Delay(20);                                                          /* 消抖 */
      if(HAL_GPIO_ReadPin(Key_GPIO_Port, Key_Pin) == GPIO_PIN_RESET){         /* 再次确认按下 */
        HAL_TIM_PWM_Stop(&htim3, channels[channel_index]);                    /* 停止当前PWM通道 */
        channel_index = (channel_index + 1) % 3;                              /* 切换到下一个通道(0→1→2→0) */
        HAL_TIM_PWM_Start(&htim3, channels[channel_index]);                   /* 启动新通道PWM */
      }
      while(HAL_GPIO_ReadPin(Key_GPIO_Port, Key_Pin) == GPIO_PIN_RESET);      /* 等待按键释放 */
    }
    __HAL_TIM_SET_COMPARE(&htim3, channels[channel_index], count);            /* 设置PWM占空比=计数值 */
    OLED_ShowFrame();                                                         /* 刷新显示 */
    HAL_Delay(100);                                                           /* 每100ms更新一次 */
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
