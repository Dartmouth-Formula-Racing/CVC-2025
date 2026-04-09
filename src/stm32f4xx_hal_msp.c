/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file         stm32f4xx_hal_msp.c
 * @brief        This file provides code for the MSP Initialization
 *               and de-Initialization codes.
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
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN Define */

/* USER CODE END Define */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN Macro */

/* USER CODE END Macro */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
extern DMA_HandleTypeDef hdma_adc1;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* External functions --------------------------------------------------------*/
/* USER CODE BEGIN ExternalFunctions */

/* USER CODE END ExternalFunctions */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef* htim);
/**
 * Initializes the Global MSP.
 */
void HAL_MspInit(void) {
    /* USER CODE BEGIN MspInit 0 */

    /* USER CODE END MspInit 0 */

    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    /* System interrupt init*/
    /* PendSV_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(PendSV_IRQn, 15, 0);

    /* USER CODE BEGIN MspInit 1 */

    /* USER CODE END MspInit 1 */
}

/**
 * @brief ADC MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hadc: ADC handle pointer
 * @retval None
 */
void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (hadc->Instance == ADC1) {
        /* USER CODE BEGIN ADC1_MspInit 0 */

        /* USER CODE END ADC1_MspInit 0 */
        /* Peripheral clock enable */
        __HAL_RCC_ADC1_CLK_ENABLE();

        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**ADC1 GPIO Configuration
        PC0     ------> ADC1_IN10
        PC1     ------> ADC1_IN11
        PC2     ------> ADC1_IN12
        PA0-WKUP     ------> ADC1_IN0
        PA1     ------> ADC1_IN1
        PA3     ------> ADC1_IN3
        PA4     ------> ADC1_IN4
        PA5     ------> ADC1_IN5
        PA6     ------> ADC1_IN6
        PA7     ------> ADC1_IN7
        PB0     ------> ADC1_IN8
        PB1     ------> ADC1_IN9
        */
        GPIO_InitStruct.Pin = DCDC_Voltage_Pin | Brake_Pressure_Pin | GLV_Bus_Voltage_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = APPS_1_Pin | APPS_2_Pin | Current_Sensor_Pin | Steering_Angle_Pin | Inverter1_CS_Pin | Inverter2_CS_Pin | Pump1_CS_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = Pump2_CS_Pin | Battery_Voltage_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        /* ADC1 DMA Init */
        __HAL_RCC_DMA2_CLK_ENABLE();
        hdma_adc1.Instance = DMA2_Stream0;
        hdma_adc1.Init.Channel = DMA_CHANNEL_0;
        hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
        hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
        hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
        hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
        hdma_adc1.Init.Mode = DMA_CIRCULAR;
        hdma_adc1.Init.Priority = DMA_PRIORITY_HIGH;
        hdma_adc1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
        if (HAL_DMA_Init(&hdma_adc1) != HAL_OK) {
            Error_Handler();
        }

        __HAL_LINKDMA(hadc, DMA_Handle, hdma_adc1);

        /* ADC1 DMA interrupt Init */
        HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

        /* ADC1 interrupt Init */
        HAL_NVIC_SetPriority(ADC_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(ADC_IRQn);
        /* USER CODE BEGIN ADC1_MspInit 1 */

        /* USER CODE END ADC1_MspInit 1 */
    }
}

/**
 * @brief ADC MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hadc: ADC handle pointer
 * @retval None
 */
void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc) {
    if (hadc->Instance == ADC1) {
        /* USER CODE BEGIN ADC1_MspDeInit 0 */

        /* USER CODE END ADC1_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_ADC1_CLK_DISABLE();

        /**ADC1 GPIO Configuration
        PC0     ------> ADC1_IN10
        PC1     ------> ADC1_IN11
        PC2     ------> ADC1_IN12
        PA0-WKUP     ------> ADC1_IN0
        PA1     ------> ADC1_IN1
        PA3     ------> ADC1_IN3
        PA4     ------> ADC1_IN4
        PA5     ------> ADC1_IN5
        PA6     ------> ADC1_IN6
        PA7     ------> ADC1_IN7
        PB0     ------> ADC1_IN8
        PB1     ------> ADC1_IN9
        */
        HAL_GPIO_DeInit(GPIOC, DCDC_Voltage_Pin | Brake_Pressure_Pin | GLV_Bus_Voltage_Pin);

        HAL_GPIO_DeInit(GPIOA, APPS_1_Pin | APPS_2_Pin | Current_Sensor_Pin | Steering_Angle_Pin | Inverter1_CS_Pin | Inverter2_CS_Pin | Pump1_CS_Pin);

        HAL_GPIO_DeInit(GPIOB, Pump2_CS_Pin | Battery_Voltage_Pin);

        /* ADC1 DMA DeInit */
        HAL_DMA_DeInit(hadc->DMA_Handle);

        /* ADC1 DMA interrupt DeInit */
        HAL_NVIC_DisableIRQ(DMA2_Stream0_IRQn);

        /* ADC1 interrupt DeInit */
        HAL_NVIC_DisableIRQ(ADC_IRQn);
        /* USER CODE BEGIN ADC1_MspDeInit 1 */

        /* USER CODE END ADC1_MspDeInit 1 */
    }
}

static uint32_t HAL_RCC_CAN1_CLK_ENABLED = 0;

/**
 * @brief CAN MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hcan: CAN handle pointer
 * @retval None
 */
void HAL_CAN_MspInit(CAN_HandleTypeDef* hcan) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (hcan->Instance == CAN1) {
        /* USER CODE BEGIN CAN1_MspInit 0 */

        /* USER CODE END CAN1_MspInit 0 */
        /* Peripheral clock enable */
        HAL_RCC_CAN1_CLK_ENABLED++;
        if (HAL_RCC_CAN1_CLK_ENABLED == 1) {
            __HAL_RCC_CAN1_CLK_ENABLE();
        }

        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**CAN1 GPIO Configuration
        PA11     ------> CAN1_RX
        PA12     ------> CAN1_TX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* CAN1 interrupt Init */
        HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
        /* USER CODE BEGIN CAN1_MspInit 1 */

        /* USER CODE END CAN1_MspInit 1 */
    } else if (hcan->Instance == CAN2) {
        /* USER CODE BEGIN CAN2_MspInit 0 */

        /* USER CODE END CAN2_MspInit 0 */
        /* Peripheral clock enable */
        __HAL_RCC_CAN2_CLK_ENABLE();
        HAL_RCC_CAN1_CLK_ENABLED++;
        if (HAL_RCC_CAN1_CLK_ENABLED == 1) {
            __HAL_RCC_CAN1_CLK_ENABLE();
        }

        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**CAN2 GPIO Configuration
        PB12     ------> CAN2_RX
        PB13     ------> CAN2_TX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF9_CAN2;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        /* CAN2 interrupt Init */
        HAL_NVIC_SetPriority(CAN2_RX0_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(CAN2_RX0_IRQn);
        /* USER CODE BEGIN CAN2_MspInit 1 */

        /* USER CODE END CAN2_MspInit 1 */
    }
}

/**
 * @brief CAN MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hcan: CAN handle pointer
 * @retval None
 */
void HAL_CAN_MspDeInit(CAN_HandleTypeDef* hcan) {
    if (hcan->Instance == CAN1) {
        /* USER CODE BEGIN CAN1_MspDeInit 0 */

        /* USER CODE END CAN1_MspDeInit 0 */
        /* Peripheral clock disable */
        HAL_RCC_CAN1_CLK_ENABLED--;
        if (HAL_RCC_CAN1_CLK_ENABLED == 0) {
            __HAL_RCC_CAN1_CLK_DISABLE();
        }

        /**CAN1 GPIO Configuration
        PA11     ------> CAN1_RX
        PA12     ------> CAN1_TX
        */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11 | GPIO_PIN_12);

        /* CAN1 interrupt DeInit */
        HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
        /* USER CODE BEGIN CAN1_MspDeInit 1 */

        /* USER CODE END CAN1_MspDeInit 1 */
    } else if (hcan->Instance == CAN2) {
        /* USER CODE BEGIN CAN2_MspDeInit 0 */

        /* USER CODE END CAN2_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_CAN2_CLK_DISABLE();
        HAL_RCC_CAN1_CLK_ENABLED--;
        if (HAL_RCC_CAN1_CLK_ENABLED == 0) {
            __HAL_RCC_CAN1_CLK_DISABLE();
        }

        /**CAN2 GPIO Configuration
        PB12     ------> CAN2_RX
        PB13     ------> CAN2_TX
        */
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12 | GPIO_PIN_13);

        /* CAN2 interrupt DeInit */
        HAL_NVIC_DisableIRQ(CAN2_RX0_IRQn);
        /* USER CODE BEGIN CAN2_MspDeInit 1 */

        /* USER CODE END CAN2_MspDeInit 1 */
    }
}

/**
 * @brief CRC MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hcrc: CRC handle pointer
 * @retval None
 */
void HAL_CRC_MspInit(CRC_HandleTypeDef* hcrc) {
    if (hcrc->Instance == CRC) {
        /* USER CODE BEGIN CRC_MspInit 0 */

        /* USER CODE END CRC_MspInit 0 */
        /* Peripheral clock enable */
        __HAL_RCC_CRC_CLK_ENABLE();
        /* USER CODE BEGIN CRC_MspInit 1 */

        /* USER CODE END CRC_MspInit 1 */
    }
}

/**
 * @brief CRC MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hcrc: CRC handle pointer
 * @retval None
 */
void HAL_CRC_MspDeInit(CRC_HandleTypeDef* hcrc) {
    if (hcrc->Instance == CRC) {
        /* USER CODE BEGIN CRC_MspDeInit 0 */

        /* USER CODE END CRC_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_CRC_CLK_DISABLE();
        /* USER CODE BEGIN CRC_MspDeInit 1 */

        /* USER CODE END CRC_MspDeInit 1 */
    }
}

/**
 * @brief RTC MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hrtc: RTC handle pointer
 * @retval None
 */
void HAL_RTC_MspInit(RTC_HandleTypeDef* hrtc) {
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    if (hrtc->Instance == RTC) {
        /* USER CODE BEGIN RTC_MspInit 0 */

        /* USER CODE END RTC_MspInit 0 */

        /** Initializes the peripherals clock
         */
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
        PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
            Error_Handler();
        }

        /* Peripheral clock enable */
        __HAL_RCC_RTC_ENABLE();
        /* USER CODE BEGIN RTC_MspInit 1 */

        /* USER CODE END RTC_MspInit 1 */
    }
}

/**
 * @brief RTC MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hrtc: RTC handle pointer
 * @retval None
 */
void HAL_RTC_MspDeInit(RTC_HandleTypeDef* hrtc) {
    if (hrtc->Instance == RTC) {
        /* USER CODE BEGIN RTC_MspDeInit 0 */

        /* USER CODE END RTC_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_RTC_DISABLE();
        /* USER CODE BEGIN RTC_MspDeInit 1 */

        /* USER CODE END RTC_MspDeInit 1 */
    }
}

/**
 * @brief TIM_Base MSP Initialization
 * This function configures the hardware resources used in this example
 * @param htim_base: TIM_Base handle pointer
 * @retval None
 */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (htim_base->Instance == TIM1) {
        /* USER CODE BEGIN TIM1_MspInit 0 */

        /* USER CODE END TIM1_MspInit 0 */
        /* Peripheral clock enable */
        __HAL_RCC_TIM1_CLK_ENABLE();

        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**TIM1 GPIO Configuration
        PA8     ------> TIM1_CH1
        */
        GPIO_InitStruct.Pin = Wheel_Speed_FL_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
        HAL_GPIO_Init(Wheel_Speed_FL_GPIO_Port, &GPIO_InitStruct);

        /* USER CODE BEGIN TIM1_MspInit 1 */

        /* USER CODE END TIM1_MspInit 1 */
    } else if (htim_base->Instance == TIM3) {
        /* USER CODE BEGIN TIM3_MspInit 0 */

        /* USER CODE END TIM3_MspInit 0 */
        /* Peripheral clock enable */
        __HAL_RCC_TIM3_CLK_ENABLE();

        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**TIM3 GPIO Configuration
        PB4     ------> TIM3_CH1
        */
        GPIO_InitStruct.Pin = Wheel_Speed_FR_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
        HAL_GPIO_Init(Wheel_Speed_FR_GPIO_Port, &GPIO_InitStruct);

        /* USER CODE BEGIN TIM3_MspInit 1 */

        /* USER CODE END TIM3_MspInit 1 */
    } else if (htim_base->Instance == TIM4) {
        /* USER CODE BEGIN TIM4_MspInit 0 */

        /* USER CODE END TIM4_MspInit 0 */
        /* Peripheral clock enable */
        __HAL_RCC_TIM4_CLK_ENABLE();

        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**TIM4 GPIO Configuration
        PB6     ------> TIM4_CH1
        */
        GPIO_InitStruct.Pin = Wheel_Speed_RL_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
        HAL_GPIO_Init(Wheel_Speed_RL_GPIO_Port, &GPIO_InitStruct);

        /* USER CODE BEGIN TIM4_MspInit 1 */

        /* USER CODE END TIM4_MspInit 1 */
    } else if (htim_base->Instance == TIM8) {
        /* USER CODE BEGIN TIM8_MspInit 0 */

        /* USER CODE END TIM8_MspInit 0 */
        /* Peripheral clock enable */
        __HAL_RCC_TIM8_CLK_ENABLE();

        __HAL_RCC_GPIOC_CLK_ENABLE();
        /**TIM8 GPIO Configuration
        PC6     ------> TIM8_CH1
        */
        GPIO_InitStruct.Pin = Wheel_Speed_RR_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF3_TIM8;
        HAL_GPIO_Init(Wheel_Speed_RR_GPIO_Port, &GPIO_InitStruct);

        /* TIM8 interrupt Init */
        HAL_NVIC_SetPriority(TIM8_TRG_COM_TIM14_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(TIM8_TRG_COM_TIM14_IRQn);
        /* USER CODE BEGIN TIM8_MspInit 1 */

        /* USER CODE END TIM8_MspInit 1 */
    } else if (htim_base->Instance == TIM10) {
        /* USER CODE BEGIN TIM10_MspInit 0 */

        /* USER CODE END TIM10_MspInit 0 */
        /* Peripheral clock enable */
        __HAL_RCC_TIM10_CLK_ENABLE();
        /* USER CODE BEGIN TIM10_MspInit 1 */

        /* USER CODE END TIM10_MspInit 1 */
    } else if (htim_base->Instance == TIM11) {
        /* USER CODE BEGIN TIM11_MspInit 0 */

        /* USER CODE END TIM11_MspInit 0 */
        /* Peripheral clock enable */
        __HAL_RCC_TIM11_CLK_ENABLE();
        /* USER CODE BEGIN TIM11_MspInit 1 */

        /* USER CODE END TIM11_MspInit 1 */
    } else if (htim_base->Instance == TIM14) {
        /* USER CODE BEGIN TIM14_MspInit 0 */

        /* USER CODE END TIM14_MspInit 0 */
        /* Peripheral clock enable */
        __HAL_RCC_TIM14_CLK_ENABLE();
        /* TIM14 interrupt Init */
        HAL_NVIC_SetPriority(TIM8_TRG_COM_TIM14_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(TIM8_TRG_COM_TIM14_IRQn);
        /* USER CODE BEGIN TIM14_MspInit 1 */

        /* USER CODE END TIM14_MspInit 1 */
    }
}

/**
 * @brief TIM_PWM MSP Initialization
 * This function configures the hardware resources used in this example
 * @param htim_pwm: TIM_PWM handle pointer
 * @retval None
 */
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* htim_pwm) {
    if (htim_pwm->Instance == TIM9) {
        /* USER CODE BEGIN TIM9_MspInit 0 */

        /* USER CODE END TIM9_MspInit 0 */
        /* Peripheral clock enable */
        __HAL_RCC_TIM9_CLK_ENABLE();
        /* USER CODE BEGIN TIM9_MspInit 1 */

        /* USER CODE END TIM9_MspInit 1 */
    } else if (htim_pwm->Instance == TIM12) {
        /* USER CODE BEGIN TIM12_MspInit 0 */

        /* USER CODE END TIM12_MspInit 0 */
        /* Peripheral clock enable */
        __HAL_RCC_TIM12_CLK_ENABLE();
        /* USER CODE BEGIN TIM12_MspInit 1 */

        /* USER CODE END TIM12_MspInit 1 */
    }
}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef* htim) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (htim->Instance == TIM9) {
        /* USER CODE BEGIN TIM9_MspPostInit 0 */

        /* USER CODE END TIM9_MspPostInit 0 */
        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**TIM9 GPIO Configuration
        PA2     ------> TIM9_CH1
        */
        GPIO_InitStruct.Pin = Servo_1_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF3_TIM9;
        HAL_GPIO_Init(Servo_1_GPIO_Port, &GPIO_InitStruct);

        /* USER CODE BEGIN TIM9_MspPostInit 1 */

        /* USER CODE END TIM9_MspPostInit 1 */
    } else if (htim->Instance == TIM10) {
        /* USER CODE BEGIN TIM10_MspPostInit 0 */

        /* USER CODE END TIM10_MspPostInit 0 */

        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**TIM10 GPIO Configuration
        PB8     ------> TIM10_CH1
        */
        GPIO_InitStruct.Pin = Servo_2_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF3_TIM10;
        HAL_GPIO_Init(Servo_2_GPIO_Port, &GPIO_InitStruct);

        /* USER CODE BEGIN TIM10_MspPostInit 1 */

        /* USER CODE END TIM10_MspPostInit 1 */
    } else if (htim->Instance == TIM11) {
        /* USER CODE BEGIN TIM11_MspPostInit 0 */

        /* USER CODE END TIM11_MspPostInit 0 */

        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**TIM11 GPIO Configuration
        PB9     ------> TIM11_CH1
        */
        GPIO_InitStruct.Pin = Pump_1_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF3_TIM11;
        HAL_GPIO_Init(Pump_1_GPIO_Port, &GPIO_InitStruct);

        /* USER CODE BEGIN TIM11_MspPostInit 1 */

        /* USER CODE END TIM11_MspPostInit 1 */
    } else if (htim->Instance == TIM12) {
        /* USER CODE BEGIN TIM12_MspPostInit 0 */

        /* USER CODE END TIM12_MspPostInit 0 */

        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**TIM12 GPIO Configuration
        PB14     ------> TIM12_CH1
        */
        GPIO_InitStruct.Pin = Pump_2_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF9_TIM12;
        HAL_GPIO_Init(Pump_2_GPIO_Port, &GPIO_InitStruct);

        /* USER CODE BEGIN TIM12_MspPostInit 1 */

        /* USER CODE END TIM12_MspPostInit 1 */
    }
}
/**
 * @brief TIM_Base MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param htim_base: TIM_Base handle pointer
 * @retval None
 */
void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base) {
    if (htim_base->Instance == TIM1) {
        /* USER CODE BEGIN TIM1_MspDeInit 0 */

        /* USER CODE END TIM1_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_TIM1_CLK_DISABLE();

        /**TIM1 GPIO Configuration
        PA8     ------> TIM1_CH1
        */
        HAL_GPIO_DeInit(Wheel_Speed_FL_GPIO_Port, Wheel_Speed_FL_Pin);

        /* USER CODE BEGIN TIM1_MspDeInit 1 */

        /* USER CODE END TIM1_MspDeInit 1 */
    } else if (htim_base->Instance == TIM3) {
        /* USER CODE BEGIN TIM3_MspDeInit 0 */

        /* USER CODE END TIM3_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_TIM3_CLK_DISABLE();

        /**TIM3 GPIO Configuration
        PB4     ------> TIM3_CH1
        */
        HAL_GPIO_DeInit(Wheel_Speed_FR_GPIO_Port, Wheel_Speed_FR_Pin);

        /* USER CODE BEGIN TIM3_MspDeInit 1 */

        /* USER CODE END TIM3_MspDeInit 1 */
    } else if (htim_base->Instance == TIM4) {
        /* USER CODE BEGIN TIM4_MspDeInit 0 */

        /* USER CODE END TIM4_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_TIM4_CLK_DISABLE();

        /**TIM4 GPIO Configuration
        PB6     ------> TIM4_CH1
        */
        HAL_GPIO_DeInit(Wheel_Speed_RL_GPIO_Port, Wheel_Speed_RL_Pin);

        /* USER CODE BEGIN TIM4_MspDeInit 1 */

        /* USER CODE END TIM4_MspDeInit 1 */
    } else if (htim_base->Instance == TIM8) {
        /* USER CODE BEGIN TIM8_MspDeInit 0 */

        /* USER CODE END TIM8_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_TIM8_CLK_DISABLE();

        /**TIM8 GPIO Configuration
        PC6     ------> TIM8_CH1
        */
        HAL_GPIO_DeInit(Wheel_Speed_RR_GPIO_Port, Wheel_Speed_RR_Pin);

        /* TIM8 interrupt DeInit */
        /* USER CODE BEGIN TIM8:TIM8_TRG_COM_TIM14_IRQn disable */
        /**
         * Uncomment the line below to disable the "TIM8_TRG_COM_TIM14_IRQn" interrupt
         * Be aware, disabling shared interrupt may affect other IPs
         */
        /* HAL_NVIC_DisableIRQ(TIM8_TRG_COM_TIM14_IRQn); */
        /* USER CODE END TIM8:TIM8_TRG_COM_TIM14_IRQn disable */

        /* USER CODE BEGIN TIM8_MspDeInit 1 */

        /* USER CODE END TIM8_MspDeInit 1 */
    } else if (htim_base->Instance == TIM10) {
        /* USER CODE BEGIN TIM10_MspDeInit 0 */

        /* USER CODE END TIM10_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_TIM10_CLK_DISABLE();
        /* USER CODE BEGIN TIM10_MspDeInit 1 */

        /* USER CODE END TIM10_MspDeInit 1 */
    } else if (htim_base->Instance == TIM11) {
        /* USER CODE BEGIN TIM11_MspDeInit 0 */

        /* USER CODE END TIM11_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_TIM11_CLK_DISABLE();
        /* USER CODE BEGIN TIM11_MspDeInit 1 */

        /* USER CODE END TIM11_MspDeInit 1 */
    } else if (htim_base->Instance == TIM14) {
        /* USER CODE BEGIN TIM14_MspDeInit 0 */

        /* USER CODE END TIM14_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_TIM14_CLK_DISABLE();

        /* TIM14 interrupt DeInit */
        /* USER CODE BEGIN TIM14:TIM8_TRG_COM_TIM14_IRQn disable */
        /**
         * Uncomment the line below to disable the "TIM8_TRG_COM_TIM14_IRQn" interrupt
         * Be aware, disabling shared interrupt may affect other IPs
         */
        /* HAL_NVIC_DisableIRQ(TIM8_TRG_COM_TIM14_IRQn); */
        /* USER CODE END TIM14:TIM8_TRG_COM_TIM14_IRQn disable */

        /* USER CODE BEGIN TIM14_MspDeInit 1 */

        /* USER CODE END TIM14_MspDeInit 1 */
    }
}

/**
 * @brief TIM_PWM MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param htim_pwm: TIM_PWM handle pointer
 * @retval None
 */
void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef* htim_pwm) {
    if (htim_pwm->Instance == TIM9) {
        /* USER CODE BEGIN TIM9_MspDeInit 0 */

        /* USER CODE END TIM9_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_TIM9_CLK_DISABLE();
        /* USER CODE BEGIN TIM9_MspDeInit 1 */

        /* USER CODE END TIM9_MspDeInit 1 */
    } else if (htim_pwm->Instance == TIM12) {
        /* USER CODE BEGIN TIM12_MspDeInit 0 */

        /* USER CODE END TIM12_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_TIM12_CLK_DISABLE();
        /* USER CODE BEGIN TIM12_MspDeInit 1 */

        /* USER CODE END TIM12_MspDeInit 1 */
    }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
