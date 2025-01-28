/*
 * Created by: Marcin Dziedzic
 */

/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "r_encoder.h"
#include "RTC.h"
#include "lcd.h"
#include "menu.h"
#include "light_sen.h"
#include "fade.h"
#include "menu_state_handlers.h"
#include "alarm.h"
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
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim3;
UART_HandleTypeDef huart2;
LedFadeHandle_t g_fadeHandle;

/* USER CODE BEGIN PV */
uint32_t lastEncMove    = 0;
uint32_t lastBtnPress   = 0;
uint32_t lastTimeUpdate = 0;
REncoder_HandleTypeDef henc;
extern int menuCount;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM1_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM3_Init(void);

/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/**
  * @brief  Główna funkcja programu (punkt startu).
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* 1. Inicjalizacja biblioteki HAL */
  HAL_Init();

  /* 2. Konfiguracja zegara systemowego (CubeMX) */
  SystemClock_Config();

  /* USER CODE BEGIN Init */
  RTC_Init(&hi2c1);
  /* USER CODE END Init */

  /* Inicjalizacja wygenerowanych peryferiów */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  MX_I2C1_Init();
  MX_TIM3_Init();

  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, htim3.Init.Period);
  l_BulbOnOff = 2; // 2 = OFF

  REncoder_Init(&henc, &htim1, GPIOC, GPIO_PIN_7);
  LightSen_Init(&hi2c1);

  // Inicjalizacja LCD
  Lcd_PortType ports[] = { GPIOC, GPIOC, GPIOB, GPIOA };
  Lcd_PinType  pins[]  = { GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_0, GPIO_PIN_4 };
  Lcd_HandleTypeDef lcd = Lcd_create(
      ports,
      pins,
      GPIOC, GPIO_PIN_2,  // RS
      GPIOC, GPIO_PIN_3,  // EN
      LCD_4_BIT_MODE
  );

  // Wyświetlenie menu głównego
  Menu_Display(&lcd, menuIndex, true);
  AlarmPreSet();
  /* USER CODE END 2 */

  /* USER CODE BEGIN WHILE */
  while (1)
  {
    LedFade_Process(&g_fadeHandle);

    int val = REncoder_Update(&henc);
    uint32_t now = HAL_GetTick();

    // Odczyt RTC
    RTC_TimeTypeDef rtc_info;
    RTC_ReadTime(&rtc_info);

    CheckAlarmTrigger(&rtc_info);

    // Główny automat stanów menu
    switch (gState)
    {
      case MENU_STATE:
        HandleMenuState(val, now, &lcd);
        break;
      case OPTION_STATE:
        HandleOptionState(val, now, &lcd);
        break;
      case SUBMENU_2:
        HandleSubMenu2State(val, now, &lcd);
        break;
      case SUBMENU_2B:
        HandleSubMenu2BState(val, now, &lcd);
        break;
      case SUBMENU_L_BULB:
        HandleSubMenuLBState(val, now, &lcd);
        break;
      case SUBMENU_ALARM:
        HandleSubMenuAlarmState(val, now, &lcd);
        break;
      case SUBMENU_ALARM_SET:
        HandleSubMenuAlarmSetState(val, now, &lcd);
        break;
      case ALARM_TRIGGERED:
        HandleAlarmTriggered(val, now, &lcd);
        break;
      case SUBMENU_ALARM_LSENSOR:
        HandleSubMenuAlarmLSensorState(val, now, &lcd);
        break;
      default:
        break;
    }

    // Opóźnienie w pętli (odciążenie CPU)
    HAL_Delay(10);
  }
  /* USER CODE END WHILE */
  /* USER CODE BEGIN 3 */
}
/* USER CODE END 3 */

/**
  * @brief System Clock Configuration
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState           = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue= RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState       = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource      = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL         = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                     |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  */
static void MX_I2C1_Init(void)
{
  hi2c1.Instance              = I2C1;
  hi2c1.Init.ClockSpeed       = 100000;
  hi2c1.Init.DutyCycle        = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1      = 0;
  hi2c1.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2      = 0;
  hi2c1.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM1 Initialization Function
  */
static void MX_TIM1_Init(void)
{
  TIM_Encoder_InitTypeDef sConfig       = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim1.Instance               = TIM1;
  htim1.Init.Prescaler         = 0;
  htim1.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim1.Init.Period            = 65535;
  htim1.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

  sConfig.EncoderMode          = TIM_ENCODERMODE_TI1;
  sConfig.IC1Polarity          = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection         = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler         = TIM_ICPSC_DIV1;
  sConfig.IC1Filter            = 0;
  sConfig.IC2Polarity          = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection         = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler         = TIM_ICPSC_DIV1;
  sConfig.IC2Filter            = 0;

  if (HAL_TIM_Encoder_Init(&htim1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM3 Initialization Function
  */
static void MX_TIM3_Init(void)
{
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC         = {0};

  htim3.Instance               = TIM3;
  htim3.Init.Prescaler         = 0;
  htim3.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim3.Init.Period            = 65535;
  htim3.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }

  sConfigOC.OCMode      = TIM_OCMODE_PWM1;
  sConfigOC.Pulse       = 0;
  sConfigOC.OCPolarity  = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode  = TIM_OCFAST_DISABLE;

  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_TIM_MspPostInit(&htim3);
}

/**
  * @brief USART2 Initialization Function
  */
static void MX_USART2_UART_Init(void)
{
  huart2.Instance             = USART2;
  huart2.Init.BaudRate        = 115200;
  huart2.Init.WordLength      = UART_WORDLENGTH_8B;
  huart2.Init.StopBits        = UART_STOPBITS_1;
  huart2.Init.Parity          = UART_PARITY_NONE;
  huart2.Init.Mode            = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl       = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling    = UART_OVERSAMPLING_16;

  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC,
                    LCD_D5_Pin|LCD_D4_Pin|LCD_RS_Pin|LCD_EN_Pin
                    |USB2_EN_Pin|USB1_EN_Pin,
                    GPIO_PIN_RESET);

  /* Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA,
                    LCD_D7_Pin|LD2_Pin,
                    GPIO_PIN_RESET);

  /* Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_D6_GPIO_Port,
                    LCD_D6_Pin,
                    GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin  = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_D5_Pin LCD_D4_Pin LCD_RS_Pin LCD_EN_Pin
                           USB2_EN_Pin USB1_EN_Pin */
  GPIO_InitStruct.Pin   = LCD_D5_Pin|LCD_D4_Pin|LCD_RS_Pin|LCD_EN_Pin
                          |USB2_EN_Pin|USB1_EN_Pin;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_D7_Pin LD2_Pin */
  GPIO_InitStruct.Pin   = LCD_D7_Pin|LD2_Pin;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_D6_Pin */
  GPIO_InitStruct.Pin   = LCD_D6_Pin;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_D6_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ENCODER_BTN_Pin */
  GPIO_InitStruct.Pin   = ENCODER_BTN_Pin;
  GPIO_InitStruct.Mode  = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull  = GPIO_PULLUP;
  HAL_GPIO_Init(ENCODER_BTN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : USB2_FLT_Pin USB1_FLT_Pin */
  GPIO_InitStruct.Pin   = USB2_FLT_Pin|USB1_FLT_Pin;
  GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* EXTI interrupt init */
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/**
  * @brief Funkcja wywoływana w przypadku błędu.
  */
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  Raportuje nazwę pliku źródłowego i numer linii,
  *         w której wystąpił błąd assert_param.
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* Przykładowa implementacja do debugowania:
     printf("Wrong parameters value: file %s on line %d\r\n", file, line);
  */
}
#endif /* USE_FULL_ASSERT */
