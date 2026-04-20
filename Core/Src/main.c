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
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "ssd1306.h"
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
// 超声波相关全局变量
uint32_t upEdge = 0;   // 上升沿捕获的时间（us）
uint32_t downEdge = 0; // 下降沿捕获的时间（us）
float distance = 0.0;
char buf[20]; // oled显示
void set_degree(uint8_t degree)
{
  // 限制角度范围 0-180
  if (degree > 180)
    degree = 180;
  // 0~180° -> 占空比2.5%~12.5% -> 比较器 5~25
  uint16_t duration = (uint16_t)degree * 20 / 180 + 5;
  __HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, duration);
  // printf("Set Degree: %d, Duration: %d\r\n", degree, duration); // 调试用
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM2 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
  {

    upEdge = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
    downEdge = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
    distance = (float)((downEdge - upEdge) * 0.034 / 2);
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
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_I2C2_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_IC_Start(&htim2, TIM_CHANNEL_1);
  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);
  uint8_t degree = 0;
  int8_t dir = 5;
  char message[20] = "";
  const uint8_t title[] = {4, 5, 6, 7, 8}; // "智能垃圾桶"
  const uint8_t open[] = {0, 1};           // "打开"
  const uint8_t close[] = {2, 3};          // "关闭"
  if (OLED_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  OLED_Clear();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    HAL_GPIO_TogglePin(Led_GPIO_Port, Led_Pin);
    HAL_Delay(100);
    // set_degree(degree);
    // degree += dir;
    // if (degree >= 180)
    //   dir = -5;
    // if (degree <= 0)
    //   dir = 5;
    // OLED_ShowString(2, 0, "Auto TrashBin", 16, 0);
    OLED_ShowCHinese_Array(2, 0, 1, 5, title);
    HAL_GPIO_WritePin(Trig_GPIO_Port, Trig_Pin, GPIO_PIN_SET);
    HAL_Delay(1); // 延时了1ms
    HAL_GPIO_WritePin(Trig_GPIO_Port, Trig_Pin, GPIO_PIN_RESET);
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    HAL_Delay(200);

    sprintf(message, "dis:%fcm\r\n", distance);
    sprintf(buf, "dis:%3.1fcm", distance);
    OLED_ShowString(0, 2, buf, 16, 0);
    HAL_UART_Transmit(&huart1, (uint8_t *)message, strlen(message), 200);
    if (distance <= 5.0)
    {
      OLED_ShowString(0, 4, "State:Opening", 16, 0);
      // x,y,state,num,array
      OLED_ShowCHinese_Array(40, 6, 1, 2, open); // 从第20列开始显示
      set_degree(90);
      sprintf(message, "distance low\r\n");
      HAL_UART_Transmit(&huart1, (uint8_t *)message, strlen(message), 200);
    }
    else
    {
      OLED_ShowString(0, 4, "State:Closed ", 16, 0);
      OLED_ShowCHinese_Array(40, 6, 1, 2, close);
      // OLED_ShowCHinese(56 - 16, 6, 2, 1);
      // OLED_ShowCHinese(56, 6, 3, 1);
      sprintf(message, "distance high\r\n");
      HAL_UART_Transmit(&huart1, (uint8_t *)message, strlen(message), 200);
      set_degree(0);
    }
    // x,y,index,0/1

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
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
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
