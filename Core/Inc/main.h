/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#define DRDY_4_Pin GPIO_PIN_13
#define DRDY_4_GPIO_Port GPIOC
#define STB_1_Pin GPIO_PIN_14
#define STB_1_GPIO_Port GPIOC
#define STB_2_Pin GPIO_PIN_15
#define STB_2_GPIO_Port GPIOC
#define WATER_1_Pin GPIO_PIN_0
#define WATER_1_GPIO_Port GPIOC
#define WATER_2_Pin GPIO_PIN_1
#define WATER_2_GPIO_Port GPIOC
#define A_MO_Pin GPIO_PIN_2
#define A_MO_GPIO_Port GPIOC
#define A_LL_HH_Pin GPIO_PIN_3
#define A_LL_HH_GPIO_Port GPIOC
#define ADC_LEVEL_1_Pin GPIO_PIN_0
#define ADC_LEVEL_1_GPIO_Port GPIOA
#define ACD_LEVEL_2_Pin GPIO_PIN_1
#define ACD_LEVEL_2_GPIO_Port GPIOA
#define RS485_TX_Pin GPIO_PIN_2
#define RS485_TX_GPIO_Port GPIOA
#define RS485_RD_Pin GPIO_PIN_4
#define RS485_RD_GPIO_Port GPIOA
#define SPI1_CS2_Pin GPIO_PIN_6
#define SPI1_CS2_GPIO_Port GPIOA
#define SPI1_CS3_Pin GPIO_PIN_7
#define SPI1_CS3_GPIO_Port GPIOA
#define A_PT_Pin GPIO_PIN_4
#define A_PT_GPIO_Port GPIOC
#define A_HT_Pin GPIO_PIN_5
#define A_HT_GPIO_Port GPIOC
#define BUZZER_Pin GPIO_PIN_0
#define BUZZER_GPIO_Port GPIOB
#define SPI1_CS1_Pin GPIO_PIN_1
#define SPI1_CS1_GPIO_Port GPIOB
#define LIVE_Pin GPIO_PIN_2
#define LIVE_GPIO_Port GPIOB
#define CARD_DET_Pin GPIO_PIN_6
#define CARD_DET_GPIO_Port GPIOC
#define B_MO_Pin GPIO_PIN_7
#define B_MO_GPIO_Port GPIOC
#define B_LL_HH_Pin GPIO_PIN_8
#define B_LL_HH_GPIO_Port GPIOC
#define B_PT_Pin GPIO_PIN_9
#define B_PT_GPIO_Port GPIOC
#define SPI1_CS4_Pin GPIO_PIN_8
#define SPI1_CS4_GPIO_Port GPIOA
#define CLK_4_Pin GPIO_PIN_11
#define CLK_4_GPIO_Port GPIOA
#define DIO_4_Pin GPIO_PIN_12
#define DIO_4_GPIO_Port GPIOA
#define DRDY_1_Pin GPIO_PIN_15
#define DRDY_1_GPIO_Port GPIOA
#define B_HT_Pin GPIO_PIN_10
#define B_HT_GPIO_Port GPIOC
#define DRDY_2_Pin GPIO_PIN_11
#define DRDY_2_GPIO_Port GPIOC
#define DRDY_3_Pin GPIO_PIN_12
#define DRDY_3_GPIO_Port GPIOC
#define CLK_3_Pin GPIO_PIN_8
#define CLK_3_GPIO_Port GPIOB
#define DIO_3_Pin GPIO_PIN_9
#define DIO_3_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
