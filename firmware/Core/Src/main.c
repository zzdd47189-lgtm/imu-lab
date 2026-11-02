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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include<stdio.h>
#include <math.h>
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
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
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
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  uint8_t id=0;
  HAL_StatusTypeDef status;
  status = HAL_I2C_Mem_Read(&hi2c1,0x6A<<1,0x0F,I2C_MEMADD_SIZE_8BIT,&id,1,100);
  uint8_t accel_config=0x40;
  HAL_StatusTypeDef accel_write_status;
  accel_write_status = HAL_I2C_Mem_Write(&hi2c1,0x6A<<1,0x10,I2C_MEMADD_SIZE_8BIT,&accel_config,1,100);
  uint8_t gyro_config=0x40;
HAL_StatusTypeDef gyro_write_status;
gyro_write_status = HAL_I2C_Mem_Write(&hi2c1,0x6A<<1,0x11,I2C_MEMADD_SIZE_8BIT,&gyro_config,1,100);
 

uint8_t msg[] = "STM32 UART OK\r\n";
HAL_UART_Transmit(&huart1, msg, sizeof(msg) - 1, 100);// Send a test message over UART to verify it is working
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
   uint8_t accel_data[6]={0};
  int16_t ax_raw=0;
  float ax=0.0f;
  int16_t ay_raw=0;
  float ay=0.0f;
  int16_t az_raw=0;
  float az=0.0f;
  HAL_StatusTypeDef accel_status;

  uint8_t gyro_data[6]={0};
  int16_t gx_raw=0;
  float gx=0.0f;
  int16_t gy_raw=0;
  float gy=0.0f;
  int16_t gz_raw=0;
  float gz=0.0f;
  HAL_StatusTypeDef gyro_status;

  float roll_acc = 0.0f;
float pitch_acc = 0.0f;

float roll_gyro = 0.0f;
float pitch_gyro = 0.0f;

float roll_fused = 0.0f;
float pitch_fused = 0.0f;
uint32_t previous_time = HAL_GetTick();
  while (1)
{
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    static uint32_t last_send = 0;
    static uint32_t tilt_start = 0;
    static int tilt_timing = 0;
    static int alarm = 0;

    accel_status = HAL_I2C_Mem_Read(
        &hi2c1, 0x6A << 1, 0x28,
        I2C_MEMADD_SIZE_8BIT, accel_data, 6, 100);

    gyro_status = HAL_I2C_Mem_Read(
        &hi2c1, 0x6A << 1, 0x22,
        I2C_MEMADD_SIZE_8BIT, gyro_data, 6, 100);

    uint32_t current_time = HAL_GetTick();
    float dt = (current_time - previous_time) / 1000.0f;
    previous_time = current_time;

    // 读取失败：中断本次超限计时，不用旧数据继续判断
    if (accel_status != HAL_OK || gyro_status != HAL_OK)
    {
        tilt_timing = 0;
        alarm = 0;
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
        HAL_Delay(10);
        continue;
    }

    // 加速度，单位 g
    ax_raw = (int16_t)((accel_data[1] << 8) | accel_data[0]);
    ay_raw = (int16_t)((accel_data[3] << 8) | accel_data[2]);
    az_raw = (int16_t)((accel_data[5] << 8) | accel_data[4]);

    ax = ax_raw * 0.000061f;
    ay = ay_raw * 0.000061f;
    az = az_raw * 0.000061f;

    roll_acc = atan2f(ay, az) * 180.0f / 3.1415926f;
    pitch_acc = atan2f(-ax, sqrtf(ay * ay + az * az))
                * 180.0f / 3.1415926f;

    // 角速度，单位 °/s
    gx_raw = (int16_t)((gyro_data[1] << 8) | gyro_data[0]);
    gy_raw = (int16_t)((gyro_data[3] << 8) | gyro_data[2]);
    gz_raw = (int16_t)((gyro_data[5] << 8) | gyro_data[4]);

    gx = gx_raw * 0.00875f;
    gy = gy_raw * 0.00875f;
    gz = gz_raw * 0.00875f;

    roll_gyro += gx * dt;
    pitch_gyro += gy * dt;

    roll_fused = 0.98f * (roll_fused + gx * dt)
                 + 0.02f * roll_acc;
    pitch_fused = 0.98f * (pitch_fused + gy * dt)
                  + 0.02f * pitch_acc;

    // 连续超限 2 秒才报警
    int tilt_over = (roll_fused > 20.0f ||
                     roll_fused < -20.0f ||
                     pitch_fused > 20.0f ||
                     pitch_fused < -20.0f);

    if (tilt_over)
    {
        if (!tilt_timing)
        {
            tilt_start = current_time;
            tilt_timing = 1;
        }

        if (current_time - tilt_start >= 2000)
        {
            alarm = 1;
        }
    }
    else
    {
        tilt_timing = 0;
        alarm = 0;
    }

    // 每 100ms 发送：时间、Roll、Pitch、报警状态
    if (current_time - last_send >= 100)
    {
        last_send = current_time;

        char msg[64];
        int len = snprintf(msg, sizeof(msg),
                           "%lu,%d,%d,%d\r\n",
                           (unsigned long)current_time,
                           (int)roll_fused,
                           (int)pitch_fused,
                           alarm);

        if (len > 0 && len < (int)sizeof(msg))
        {
            HAL_UART_Transmit(
                &huart1, (uint8_t *)msg, (uint16_t)len, 100);
        }
    }

    HAL_Delay(10);
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
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
