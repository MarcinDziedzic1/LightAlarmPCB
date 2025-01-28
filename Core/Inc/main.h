/* USER CODE BEGIN Header */
/*
 * Created by: Marcin Dziedzic
 */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* USER CODE BEGIN ET */
/* USER CODE END ET */

/* USER CODE BEGIN EC */
/* USER CODE END EC */

/* USER CODE BEGIN EM */
/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
void Error_Handler(void);

/* USER CODE BEGIN EFP */
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define B1_EXTI_IRQn EXTI15_10_IRQn
#define LCD_D5_Pin GPIO_PIN_0
#define LCD_D5_GPIO_Port GPIOC
#define LCD_D4_Pin GPIO_PIN_1
#define LCD_D4_GPIO_Port GPIOC
#define LCD_RS_Pin GPIO_PIN_2
#define LCD_RS_GPIO_Port GPIOC
#define LCD_EN_Pin GPIO_PIN_3
#define LCD_EN_GPIO_Port GPIOC
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define LCD_D7_Pin GPIO_PIN_4
#define LCD_D7_GPIO_Port GPIOA
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
#define LCD_D6_Pin GPIO_PIN_0
#define LCD_D6_GPIO_Port GPIOB
#define LB_PWM_CTRL_Pin GPIO_PIN_1
#define LB_PWM_CTRL_GPIO_Port GPIOB
#define ENCODER_BTN_Pin GPIO_PIN_7
#define ENCODER_BTN_GPIO_Port GPIOC
#define USB2_FLT_Pin GPIO_PIN_8
#define USB2_FLT_GPIO_Port GPIOC
#define USB2_EN_Pin GPIO_PIN_9
#define USB2_EN_GPIO_Port GPIOC
#define ENCODER_A_Pin GPIO_PIN_8
#define ENCODER_A_GPIO_Port GPIOA
#define ENCODER_B_Pin GPIO_PIN_9
#define ENCODER_B_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define USB1_FLT_Pin GPIO_PIN_10
#define USB1_FLT_GPIO_Port GPIOC
#define USB1_EN_Pin GPIO_PIN_11
#define USB1_EN_GPIO_Port GPIOC
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define RTC_SCL_Pin GPIO_PIN_8
#define RTC_SCL_GPIO_Port GPIOB
#define RTC_SDA_Pin GPIO_PIN_9
#define RTC_SDA_GPIO_Port GPIOB

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
