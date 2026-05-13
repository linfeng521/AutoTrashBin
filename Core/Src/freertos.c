/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "oled.h"
#include "servo.h"
#include "kalman_1d_filter.h"
#include "hcsr04.h"
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
/* USER CODE BEGIN Variables */
// 共享变量：用于任务间通信
volatile float g_distance_filtered = 0.0;
volatile uint8_t g_lid_state = 0; // 0=关闭, 1=打开
volatile uint32_t g_lid_open_time = 0; // 开盖时间戳
/* USER CODE END Variables */
/* Definitions for servoTask */
osThreadId_t servoTaskHandle;
const osThreadAttr_t servoTask_attributes = {
    .name = "servoTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};
/* Definitions for ultrasonicTask */
osThreadId_t ultrasonicTaskHandle;
const osThreadAttr_t ultrasonicTask_attributes = {
    .name = "ultrasonicTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};
/* Definitions for oledDSTask */
osThreadId_t oledDSTaskHandle;
const osThreadAttr_t oledDSTask_attributes = {
    .name = "oledDSTask",
    .stack_size = 256 * 4,
    .priority = (osPriority_t)osPriorityNormal,  // 改为 Normal，和超声波任务同级
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartServoTask(void *argument);
void StartUltrasonicTask(void *argument);
void StartOledTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void)
{
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of servoTask */
  servoTaskHandle = osThreadNew(StartServoTask, NULL, &servoTask_attributes);

  /* creation of ultrasonicTask */
  ultrasonicTaskHandle = osThreadNew(StartUltrasonicTask, NULL, &ultrasonicTask_attributes);

  /* creation of oledDSTask */
  oledDSTaskHandle = osThreadNew(StartOledTask, NULL, &oledDSTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */
}

/* USER CODE BEGIN Header_StartServoTask */
/**
 * @brief  Function implementing the servoTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartServoTask */
void StartServoTask(void *argument)
{
  /* USER CODE BEGIN StartServoTask */
  // PWM 已在 main.c 中启动，初始化舵机位置
  set_degree(0);

  /* Infinite loop */
  for (;;)
  {
    // 舵机任务暂时空闲，当前由超声波任务直接控制
    // TODO: uart命令/按键中断队列
    osDelay(1000);
  }
  /* USER CODE END StartServoTask */
}

/* USER CODE BEGIN Header_StartUltrasonicTask */
/**
 * @brief Function implementing the ultrasonicTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartUltrasonicTask */
void StartUltrasonicTask(void *argument)
{
  /* USER CODE BEGIN StartUltrasonicTask */
  float distance_raw = 0.0;
  float distance_filtered = 0.0;
  static uint8_t lidIsOpen = 0;

  // 定时器输入捕获已在 main.c 中启动

  /* Infinite loop */
  for (;;)
  {
    // 1. 触发超声波测距
    HCSR04_Trigger();

    // 2. 等待测距完成（超声波最大测距时间约 38ms）
    osDelay(60);

    // 3. 获取原始距离
    distance_raw = HCSR04_GetDistance();

    // 4. 只有数据有效时才处理
    if (distance_raw > 0) {
      // 卡尔曼滤波（用于控制逻辑和显示）
      distance_filtered = Kalman_Filter(distance_raw);

      // 更新共享变量（OLED 显示滤波值）
      g_distance_filtered = distance_filtered;

      // 5. 控制逻辑 小于5cm
      if (distance_filtered < 5.0 && !lidIsOpen)
      {
        // 检测到人靠近，开盖
        set_degree(90);
        lidIsOpen = 1;
        g_lid_state = 1;
        g_lid_open_time = osKernelGetTickCount();
      }
      else if (lidIsOpen && (osKernelGetTickCount() - g_lid_open_time > 3000))
      {
        // 保持 3 秒后关盖
        set_degree(0);
        lidIsOpen = 0;
        g_lid_state = 0;
      }
			//fix: 修复上一版人离开后不能关盖的bug
      else if (lidIsOpen && distance_filtered < 5.0)
      {
        // 如果开盖期间又检测到人，重置计时
        g_lid_open_time = osKernelGetTickCount();
      }
    }
    // 6. 延时 100ms 后继续下一次测距
    osDelay(100);
  }
  /* USER CODE END StartUltrasonicTask */
}

/* USER CODE BEGIN Header_StartOledTask */
/**
 * @brief Function implementing the oledDSTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartOledTask */
void StartOledTask(void *argument)
{
  /* USER CODE BEGIN StartOledTask */
  char buf[30];
  float distance_copy = 0.0;
  uint8_t lid_state_copy = 0;
  // 使用 UTF-8 字节数组定义中文字符串（不知道为啥中文编译Error，这里改为编码）
  const char title[] = "\xE6\x99\xBA\xE8\x83\xBD\xE5\x9E\x83\xE5\x9C\xBE\xE6\xA1\xB6RTOS"; // "智能垃圾桶"
  const char state_open[] = "\xE7\x8A\xB6\xE6\x80\x81:\xE5\xBC\x80\xE7\x9B\x96"; // "状态:开盖"
  const char state_close[] = "\xE7\x8A\xB6\xE6\x80\x81:\xE5\x85\xB3\xE9\x97\xAD"; // "状态:关闭"
	const char * message = "\xE7\xA7\x92\xE5\x90\x8E\xE5\x85\xB3\xE9\x97\xAD\xE7\x9B\x96"; //秒后关闭盖
  osDelay(20);
  OLED_Init();

  /* Infinite loop */
  for (;;)
  {
    // 读取共享变量（简单读取，无需互斥锁）
    distance_copy = g_distance_filtered;
    lid_state_copy = g_lid_state;

    OLED_NewFrame();

    // 第1行：标题
    OLED_PrintString(0, 0, (char*)title, &font16x16, OLED_COLOR_NORMAL);

    // 第2行：距离信息
    sprintf(buf, "Dis:%.1fcm", distance_copy);
    OLED_PrintASCIIString(0, 16, buf, &afont16x8, OLED_COLOR_NORMAL);

    // 第3行：状态信息
    if (lid_state_copy == 1) {
      OLED_PrintString(0, 32, (char*)state_open, &font16x16, OLED_COLOR_NORMAL);
			// 第4行：显示还有*秒后关闭
			uint32_t elapsed = (osKernelGetTickCount() - g_lid_open_time) / 1000;
			uint8_t remain = (elapsed < 3) ? (3 - elapsed) : 0;
			sprintf(buf, "%d%s", remain, message);
			OLED_PrintString(0, 48, buf, &font16x16, OLED_COLOR_NORMAL);
    } else {
      OLED_PrintString(0, 32, (char*)state_close, &font16x16, OLED_COLOR_NORMAL);
    }
		
    OLED_ShowFrame();

    osDelay(100);  // 100ms 刷新一次，更快响应
  }
  /* USER CODE END StartOledTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
