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
#include "stm32f1xx_hal_rcc_ex.h"
#include "stm32f1xx_hal_tim.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "oled.h"    /* OLED 显示屏驱动 */
#include "stdio.h"   /* 标准输入输出库，用于 sprintf */
#include "string.h"  /* 字符串操作库，用于 strlen */
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
/* HC-SR04 超声波测距：TIM1_CH3 捕获上升沿，CH4 捕获下降沿，差值即为脉冲宽度 */
int upEdge = 0;      /* 上升沿 TIM 计数值 */
int downEdge = 0;    /* 下降沿 TIM 计数值 */
float distance = 0;  /* 计算出的距离（cm） */

/* TIM1 输入捕获回调：CH4 捕获到下降沿时触发 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim){
  if(htim == &htim1 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4){
    upEdge = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);   /* 读取上升沿时间 */
    downEdge = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_4); /* 读取下降沿时间 */
    distance = (downEdge - upEdge) * 0.034 / 2;  /* 脉冲宽度 × 声速(0.034cm/μs) ÷ 2 = 距离(cm) */
  }
}
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
  /* USER CODE BEGIN 2 */
  char message[20] = "";   /* OLED 显示缓冲区 */

  HAL_Delay(20);                              /* 等待 OLED 上电稳定 */
  OLED_Init();                                /* 初始化 OLED */
  HAL_TIM_Base_Start(&htim1);                 /* 启动 TIM1 基时（Prescaler=72，1MHz=1μs/计数） */
  HAL_TIM_IC_Start(&htim1, TIM_CHANNEL_3);    /* 启动 CH3 输入捕获（上升沿） */
  HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_4); /* 启动 CH4 输入捕获（下降沿，中断方式） */
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* 发送 Trig 脉冲（≥10μs）触发 HC-SR04 测距 */
    HAL_GPIO_WritePin(Trig_GPIO_Port, Trig_Pin, GPIO_PIN_SET);   /* Trig 拉高 */
    HAL_Delay(1);                                                 /* 保持 1ms 高电平 */
    HAL_GPIO_WritePin(Trig_GPIO_Port, Trig_Pin, GPIO_PIN_RESET); /* Trig 拉低，传感器开始发送超声波 */
    __HAL_TIM_SET_COUNTER(&htim1, 0);                             /* 计数器归零，准备测量回波脉宽 */
    HAL_Delay(20);                                                /* 等待测距完成 */

    OLED_NewFrame();                                              /* 开始新一帧绘制 */
    sprintf(message, "距离: %.2f cm", distance);                   /* 格式化距离字符串 */
    OLED_PrintString(0, 0, message, &font16x16, OLED_COLOR_NORMAL); /* OLED 显示距离 */
    OLED_ShowFrame();                                             /* 刷新显示 */
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
