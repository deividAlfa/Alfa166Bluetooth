/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#include <stdint.h>
#include <stdbool.h>
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
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define BTN_CALL_Pin GPIO_PIN_0
#define BTN_CALL_GPIO_Port GPIOB
#define BTN_PLAY_PAUSE_Pin GPIO_PIN_1
#define BTN_PLAY_PAUSE_GPIO_Port GPIOB
#define BTN_PREV_Pin GPIO_PIN_10
#define BTN_PREV_GPIO_Port GPIOB
#define BTN_NEXT_Pin GPIO_PIN_11
#define BTN_NEXT_GPIO_Port GPIOB
#define POS_0_Pin GPIO_PIN_12
#define POS_0_GPIO_Port GPIOB
#define POS_1_2_Pin GPIO_PIN_13
#define POS_1_2_GPIO_Port GPIOB
#define POS_2_5_Pin GPIO_PIN_14
#define POS_2_5_GPIO_Port GPIOB
#define L_minus_Pin GPIO_PIN_15
#define L_minus_GPIO_Port GPIOB
#define L_plus_Pin GPIO_PIN_8
#define L_plus_GPIO_Port GPIOA
#define MT_RVS_Pin GPIO_PIN_9
#define MT_RVS_GPIO_Port GPIOA
#define H_SPEED_Pin GPIO_PIN_10
#define H_SPEED_GPIO_Port GPIOA
#define MT_FWD_Pin GPIO_PIN_11
#define MT_FWD_GPIO_Port GPIOA
#define PHOTO_F_Pin GPIO_PIN_6
#define PHOTO_F_GPIO_Port GPIOB
#define PHOTO_R_Pin GPIO_PIN_7
#define PHOTO_R_GPIO_Port GPIOB
#define POLARITY_Pin GPIO_PIN_8
#define POLARITY_GPIO_Port GPIOB
#define ALWAYS_HIGH_Pin GPIO_PIN_9
#define ALWAYS_HIGH_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
