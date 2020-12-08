/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
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
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

// hacky
extern RTC_HandleTypeDef hrtc;

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

// #define PROFILING_ENABLED
#ifdef PROFILING_ENABLED

#define PROFILING_INIT(_t_name) \
        RTC_TimeTypeDef _t_name##_t0; \
        RTC_TimeTypeDef _t_name##_t1;

#define PROFILING_MEASURE(_t) \
        do { \
          RTC_DateTypeDef _sDate; \
          uint32_t Format = RTC_FORMAT_BIN; \
          HAL_StatusTypeDef ret = HAL_RTC_GetTime(&hrtc, &_t, Format); \
          HAL_RTC_GetDate(&hrtc, &_sDate, Format); \
        } while (0)

#define PROFILING_START(_t_name) PROFILING_MEASURE(_t_name##_t0)
#define PROFILING_END(_t_name) PROFILING_MEASURE(_t_name##_t1)
#define PROFILING_DIFF(_t_name) \
        (\
          (((_t_name##_t1).SecondFraction - (_t_name##_t1).SubSeconds) - ((_t_name##_t0).SecondFraction - (_t_name##_t0).SubSeconds)) \
          + \
          (((_t_name##_t1).Seconds - (_t_name##_t0).Seconds)) \
        )

#else

#define PROFILING_INIT(_t_name)
#define PROFILING_MEASURE(_t)
#define PROFILING_START(_t_name)
#define PROFILING_END(_t_name)
#define PROFILING_DIFF(_t_name)

#endif

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

void GW_EnterDeepSleep(void);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define GPIO_Speaker_enable_Pin GPIO_PIN_3
#define GPIO_Speaker_enable_GPIO_Port GPIOE
#define BTN_PAUSE_Pin GPIO_PIN_13
#define BTN_PAUSE_GPIO_Port GPIOC
#define BTN_GAME_Pin GPIO_PIN_1
#define BTN_GAME_GPIO_Port GPIOC
#define BTN_PWR_Pin GPIO_PIN_0
#define BTN_PWR_GPIO_Port GPIOA
#define BTN_TIME_Pin GPIO_PIN_5
#define BTN_TIME_GPIO_Port GPIOC
#define BTN_A_Pin GPIO_PIN_9
#define BTN_A_GPIO_Port GPIOD
#define BTN_Left_Pin GPIO_PIN_11
#define BTN_Left_GPIO_Port GPIOD
#define BTN_Down_Pin GPIO_PIN_14
#define BTN_Down_GPIO_Port GPIOD
#define BTN_Right_Pin GPIO_PIN_15
#define BTN_Right_GPIO_Port GPIOD
#define BTN_Up_Pin GPIO_PIN_0
#define BTN_Up_GPIO_Port GPIOD
#define BTN_B_Pin GPIO_PIN_5
#define BTN_B_GPIO_Port GPIOD
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
