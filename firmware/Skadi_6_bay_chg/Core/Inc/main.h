/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BAY1_CHG_EN_Pin GPIO_PIN_4
#define BAY1_CHG_EN_GPIO_Port GPIOA
#define BAY2_CHG_EN_Pin GPIO_PIN_5
#define BAY2_CHG_EN_GPIO_Port GPIOA
#define BAY3_CHG_EN_Pin GPIO_PIN_6
#define BAY3_CHG_EN_GPIO_Port GPIOA
#define BAY4_CHG_EN_Pin GPIO_PIN_7
#define BAY4_CHG_EN_GPIO_Port GPIOA
#define BAY5_CHG_EN_Pin GPIO_PIN_0
#define BAY5_CHG_EN_GPIO_Port GPIOB
#define BAY6_CHG_EN_Pin GPIO_PIN_1
#define BAY6_CHG_EN_GPIO_Port GPIOB
#define BAY6_DIS_EN_Pin GPIO_PIN_12
#define BAY6_DIS_EN_GPIO_Port GPIOB
#define BAY5_DIS_EN_Pin GPIO_PIN_13
#define BAY5_DIS_EN_GPIO_Port GPIOB
#define BAY4_DIS_EN_Pin GPIO_PIN_14
#define BAY4_DIS_EN_GPIO_Port GPIOB
#define BAY3_DIS_EN_Pin GPIO_PIN_15
#define BAY3_DIS_EN_GPIO_Port GPIOB
#define BAY2_DIS_EN_Pin GPIO_PIN_8
#define BAY2_DIS_EN_GPIO_Port GPIOA
#define BAY1_DIS_EN_Pin GPIO_PIN_9
#define BAY1_DIS_EN_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
