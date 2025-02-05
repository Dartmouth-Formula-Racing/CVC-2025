/* USER CODE BEGIN Header */
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define DCDC_Voltage_Pin GPIO_PIN_0
#define DCDC_Voltage_GPIO_Port GPIOC
#define Brake_Pressure_Pin GPIO_PIN_1
#define Brake_Pressure_GPIO_Port GPIOC
#define APPS_1_Pin GPIO_PIN_0
#define APPS_1_GPIO_Port GPIOA
#define APPS_2_Pin GPIO_PIN_1
#define APPS_2_GPIO_Port GPIOA
#define Servo_Control_1_Pin GPIO_PIN_2
#define Servo_Control_1_GPIO_Port GPIOA
#define Current_Sensor_Pin GPIO_PIN_3
#define Current_Sensor_GPIO_Port GPIOA
#define Steering_Angle_Pin GPIO_PIN_4
#define Steering_Angle_GPIO_Port GPIOA
#define Inverter1_CS_Pin GPIO_PIN_5
#define Inverter1_CS_GPIO_Port GPIOA
#define Inverter2_CS_Pin GPIO_PIN_6
#define Inverter2_CS_GPIO_Port GPIOA
#define Pump1_CS_Pin GPIO_PIN_7
#define Pump1_CS_GPIO_Port GPIOA
#define Pump2_CS_Pin GPIO_PIN_0
#define Pump2_CS_GPIO_Port GPIOB
#define Battery_Voltage_Pin GPIO_PIN_1
#define Battery_Voltage_GPIO_Port GPIOB
#define Pump_2_Pin GPIO_PIN_14
#define Pump_2_GPIO_Port GPIOB
#define Wheel_Speed_RR_Pin GPIO_PIN_6
#define Wheel_Speed_RR_GPIO_Port GPIOC
#define Wheel_Speed_FL_Pin GPIO_PIN_8
#define Wheel_Speed_FL_GPIO_Port GPIOA
#define Wheel_Speed_FR_Pin GPIO_PIN_4
#define Wheel_Speed_FR_GPIO_Port GPIOB
#define Wheel_Speed_RL_Pin GPIO_PIN_6
#define Wheel_Speed_RL_GPIO_Port GPIOB
#define Servo_Control_2_Pin GPIO_PIN_8
#define Servo_Control_2_GPIO_Port GPIOB
#define Pump_1_Pin GPIO_PIN_9
#define Pump_1_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
