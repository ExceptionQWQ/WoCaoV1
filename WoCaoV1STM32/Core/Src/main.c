/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#include "string.h"
#include "stdio.h"
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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

//Mecanum

struct WheelPWM
{
    double speed;
    double target;
    double minPWM;
    double maxPWM;
    double kp;
    double ki;
    double kd;
    double error;
    double lastError;
    double errorI;
    double pwm;
}wheelPWM1, wheelPWM2, wheelPWM3, wheelPWM4;

struct RobotInfo
{
    double xPos;
    double yPos;
    double angle;
    double wheel1Speed;
    double wheel2Speed;
    double wheel3Speed;
    double wheel4Speed;
    double rXSpeed;
    double rYSpeed;
}robotInfo;

void ClearSpeed()
{
    robotInfo.wheel1Speed = 0;
    robotInfo.wheel2Speed = 0;
    robotInfo.wheel3Speed = 0;
    robotInfo.wheel4Speed = 0;
    robotInfo.rXSpeed = 0;
    robotInfo.rYSpeed = 0;
}

void MoveForward(double speed)
{
    robotInfo.wheel1Speed += speed;
    robotInfo.wheel2Speed += -speed;
    robotInfo.wheel3Speed += -speed;
    robotInfo.wheel4Speed += speed;
}

void MoveBackward(double speed)
{
    robotInfo.wheel1Speed += -speed;
    robotInfo.wheel2Speed += speed;
    robotInfo.wheel3Speed += speed;
    robotInfo.wheel4Speed += -speed;
}

void MoveLeft(double speed)
{
    robotInfo.wheel1Speed += -speed;
    robotInfo.wheel2Speed += -speed;
    robotInfo.wheel3Speed += speed;
    robotInfo.wheel4Speed += speed;
}

void MoveRight(double speed)
{
    robotInfo.wheel1Speed += speed;
    robotInfo.wheel2Speed += speed;
    robotInfo.wheel3Speed += -speed;
    robotInfo.wheel4Speed += -speed;
}

void SpinLeft(double speed)
{
    robotInfo.wheel1Speed += -speed;
    robotInfo.wheel2Speed += -speed;
    robotInfo.wheel3Speed += -speed;
    robotInfo.wheel4Speed += -speed;
}

void SpinRight(double speed)
{
    robotInfo.wheel1Speed += speed;
    robotInfo.wheel2Speed += speed;
    robotInfo.wheel3Speed += speed;
    robotInfo.wheel4Speed += speed;
}

void CommitSpeed()
{
    wheelPWM1.target = robotInfo.wheel1Speed;
    wheelPWM2.target = robotInfo.wheel2Speed;
    wheelPWM3.target = robotInfo.wheel3Speed;
    wheelPWM4.target = robotInfo.wheel4Speed;
}

void PID_Init()
{
    wheelPWM1.target = 0;
    wheelPWM1.minPWM = -1000;
    wheelPWM1.maxPWM = 1000;
    wheelPWM1.kp = 15;
    wheelPWM1.ki = 4;
    wheelPWM1.kd = 0;

    wheelPWM2.target = 0;
    wheelPWM2.minPWM = -1000;
    wheelPWM2.maxPWM = 1000;
    wheelPWM2.kp = 15;
    wheelPWM2.ki = 4;
    wheelPWM2.kd = 0;

    wheelPWM3.target = 0;
    wheelPWM3.minPWM = -1000;
    wheelPWM3.maxPWM = 1000;
    wheelPWM3.kp = 15;
    wheelPWM3.ki = 4;
    wheelPWM3.kd = 0;

    wheelPWM4.target = 0;
    wheelPWM4.minPWM = -1000;
    wheelPWM4.maxPWM = 1000;
    wheelPWM4.kp = 15;
    wheelPWM4.ki = 4;
    wheelPWM4.kd = 0;
}

void Do_PID(struct WheelPWM* wheelPWM)
{
    wheelPWM->lastError = wheelPWM->error;
    wheelPWM->error = wheelPWM->target - wheelPWM->speed;
    wheelPWM->errorI += wheelPWM->error;
    double pwm = wheelPWM->kp * wheelPWM->error + wheelPWM->ki * wheelPWM->errorI + wheelPWM->kd * (wheelPWM->error - wheelPWM->lastError);
    if (pwm > wheelPWM->maxPWM) pwm = wheelPWM->maxPWM;
    if (pwm < wheelPWM->minPWM) pwm = wheelPWM->minPWM;
    wheelPWM->pwm = pwm;
}

void PID_Tick()
{
    wheelPWM1.speed = (short) __HAL_TIM_GET_COUNTER(&htim1);
    __HAL_TIM_SET_COUNTER(&htim1, 0);

    wheelPWM2.speed = (short) __HAL_TIM_GET_COUNTER(&htim2);
    __HAL_TIM_SET_COUNTER(&htim2, 0);

    wheelPWM3.speed = (short) __HAL_TIM_GET_COUNTER(&htim3);
    __HAL_TIM_SET_COUNTER(&htim3, 0);

    wheelPWM4.speed = (short) __HAL_TIM_GET_COUNTER(&htim4);
    __HAL_TIM_SET_COUNTER(&htim4, 0);

    Do_PID(&wheelPWM1);
    if (wheelPWM1.pwm > 0) {
        HAL_GPIO_WritePin(WHEEL1_A_GPIO_Port, WHEEL1_A_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(WHEEL1_B_GPIO_Port, WHEEL1_B_Pin, GPIO_PIN_SET);
        __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, wheelPWM1.pwm);
    } else if (wheelPWM1.pwm < 0) {
        HAL_GPIO_WritePin(WHEEL1_A_GPIO_Port, WHEEL1_A_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(WHEEL1_B_GPIO_Port, WHEEL1_B_Pin, GPIO_PIN_RESET);
        __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, -wheelPWM1.pwm);
    } else {
        HAL_GPIO_WritePin(WHEEL1_A_GPIO_Port, WHEEL1_A_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(WHEEL1_B_GPIO_Port, WHEEL1_B_Pin, GPIO_PIN_RESET);
        __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, 0);
    }

    Do_PID(&wheelPWM2);
    if (wheelPWM2.pwm > 0) {
        HAL_GPIO_WritePin(WHEEL2_A_GPIO_Port, WHEEL2_A_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(WHEEL2_B_GPIO_Port, WHEEL2_B_Pin, GPIO_PIN_SET);
        __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, wheelPWM2.pwm);
    } else if (wheelPWM2.pwm < 0) {
        HAL_GPIO_WritePin(WHEEL2_A_GPIO_Port, WHEEL2_A_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(WHEEL2_B_GPIO_Port, WHEEL2_B_Pin, GPIO_PIN_RESET);
        __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, -wheelPWM2.pwm);
    } else {
        HAL_GPIO_WritePin(WHEEL2_A_GPIO_Port, WHEEL2_A_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(WHEEL2_B_GPIO_Port, WHEEL2_B_Pin, GPIO_PIN_RESET);
        __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, 0);
    }

    Do_PID(&wheelPWM3);
    if (wheelPWM3.pwm > 0) {
        HAL_GPIO_WritePin(WHEEL3_A_GPIO_Port, WHEEL3_A_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(WHEEL3_B_GPIO_Port, WHEEL3_B_Pin, GPIO_PIN_SET);
        __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, wheelPWM3.pwm);
    } else if (wheelPWM3.pwm < 0) {
        HAL_GPIO_WritePin(WHEEL3_A_GPIO_Port, WHEEL3_A_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(WHEEL3_B_GPIO_Port, WHEEL3_B_Pin, GPIO_PIN_RESET);
        __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, -wheelPWM3.pwm);
    } else {
        HAL_GPIO_WritePin(WHEEL3_A_GPIO_Port, WHEEL3_A_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(WHEEL3_B_GPIO_Port, WHEEL3_B_Pin, GPIO_PIN_RESET);
        __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, 0);
    }

    Do_PID(&wheelPWM4);
    if (wheelPWM4.pwm > 0) {
        HAL_GPIO_WritePin(WHEEL4_A_GPIO_Port, WHEEL4_A_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(WHEEL4_B_GPIO_Port, WHEEL4_B_Pin, GPIO_PIN_SET);
        __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_4, wheelPWM4.pwm);
    } else if (wheelPWM4.pwm < 0) {
        HAL_GPIO_WritePin(WHEEL4_A_GPIO_Port, WHEEL4_A_Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(WHEEL4_B_GPIO_Port, WHEEL4_B_Pin, GPIO_PIN_RESET);
        __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_4, -wheelPWM4.pwm);
    } else {
        HAL_GPIO_WritePin(WHEEL4_A_GPIO_Port, WHEEL4_A_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(WHEEL4_B_GPIO_Port, WHEEL4_B_Pin, GPIO_PIN_RESET);
        __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_4, 0);
    }

}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
    if (htim->Instance == TIM6) {
        PID_Tick();
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
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_TIM6_Init();
  MX_USART1_UART_Init();
  MX_TIM8_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_UART4_Init();
  MX_UART5_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */

    PID_Init();
    HAL_TIM_Base_Start_IT(&htim6);

    HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_1 | TIM_CHANNEL_2);
    HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_1 | TIM_CHANNEL_2);
    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_1 | TIM_CHANNEL_2);
    HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_1 | TIM_CHANNEL_2);

    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_4);


    ClearSpeed();
    CommitSpeed();





  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

      char buff[64] = {0};
      snprintf(buff, 64, "speed:%.2lf pwm:%.2lf\r\n", wheelPWM2.speed, wheelPWM2.pwm);
      HAL_UART_Transmit(&huart1, buff, strlen(buff), 100);
      HAL_Delay(50);

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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
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

#ifdef  USE_FULL_ASSERT
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

