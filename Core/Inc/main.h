/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define WORD_SIZE                               0x4
#define DEFAULT_FLASH                           0xFFFFFFFF
#define FLASH_START_ADDRESS                     0x08000000
#define FLASH_END_ADDRESS                       0x0807FFFF
#define FLASH_PAGE_WORD_COUNT                   512
#define IPP_BASE_ADDRESS                        0x20000004
#define IPP_CRITERION_1                         0xADC20424
#define IPP_CRITERION_2                         0xE79F6A04
#define IPP_BLANK_BEFORE                        10
#define IPP_BLANK_AFTER                         503
#define G_SET_BIT(x, y)                         x |= (1<<y)
#define G_CLEAR_BIT(x, y)                       x &= ~(1<<y)
#define G_GET_BIT(x, y)                         ((x>>y)&1)
#define G_TOGGLE_BIT(x, y)                      x ^= (1<<y)
#define G_READ_FLASH(x)                         *(__IO uint32_t*)(x)
#define WRITE_FLASH_PROTOTYPE                   void G_WRITE_FLASH(uint32_t address, uint32_t data)
#define ERASE_FLASH_PROTOTYPE                   void G_ERASE_FLASH(uint32_t address)
#define SYSTEM_RESET_PROTOTYPE                  void G_SYSTEM_RESET()
#define SN_PRINT_PROTOTYPE                      void G_SN_PRINT(uint32_t loc)
#define SMS_REFRESH_PROTOTYPE                   void G_SMS_REFRESH(int status)
#define RELAY_CONTROL_REFRESH_PROTOTYPE         void G_RELAY_CONTROL_REFRESH(int status)
#define CONTROL_COMMAND_SEND_PROTOTYPE          void G_CONTROL_COMMAND_SEND()
#define RTC_DATETIME_PRINT(x)                   printf(x, 2000 + BCD_TO_DECIMAL(currentDate.Year), BCD_TO_DECIMAL(currentDate.Month), BCD_TO_DECIMAL(currentDate.Date), BCD_TO_DECIMAL(currentTime.Hours), BCD_TO_DECIMAL(currentTime.Minutes), BCD_TO_DECIMAL(currentTime.Seconds))
#define CHAR_ASCII_CRITERION(x)                 (x >= 48 && x <= 57) || (x >= 65 && x <= 90) || (x >= 97 && x <= 122)
#define CHAR_NUMBER_CRITERION(x)                (x >= 48 && x <= 57)
#define GET_SYSTEM_VOLTAGE(x)                   ((x * 3.3 / 4096) * 2)
#define GET_MAIN_VOLTAGE(x)                     ((x * 3.3 / 4096) * 9.33)
#define DECIMAL_TO_BCD(x)                       (((x / 10) << 4) | (x % 10))
#define BCD_TO_DECIMAL(x)                       (((x >> 4) * 10) + (x & 0x0F))
#define CHAR_TO_INT(x)                          (x <= 57 ? (x - 48) : (x - 55))
#define FLASH_SECTOR_ADDRESS(x)                 (x * 4096)
#define DATETIME_CRITERION(a, b, c, d, e, f)    (a >= 0 && a <= 99 && b >= 1 && b <= 12 && c >= 1 && c <= 31 && d >= 0 && d <= 23 && e >= 0 && e <= 59 && f >= 0 && f <= 59)
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_SYSTEM_Pin GPIO_PIN_4
#define LED_SYSTEM_GPIO_Port GPIOC
#define SMS_ENABLE_Pin GPIO_PIN_5
#define SMS_ENABLE_GPIO_Port GPIOC
#define nFLASH_CS_Pin GPIO_PIN_12
#define nFLASH_CS_GPIO_Port GPIOB
#define FATHOMETER_ENABLE_Pin GPIO_PIN_13
#define FATHOMETER_ENABLE_GPIO_Port GPIOB
#define RS485_232_Pin GPIO_PIN_6
#define RS485_232_GPIO_Port GPIOC
#define DE485_Pin GPIO_PIN_7
#define DE485_GPIO_Port GPIOC
#define LED_ERROR_Pin GPIO_PIN_8
#define LED_ERROR_GPIO_Port GPIOC
#define LED_COM_Pin GPIO_PIN_9
#define LED_COM_GPIO_Port GPIOC
#define RELAY_CONTROL_BIT_0_Pin GPIO_PIN_10
#define RELAY_CONTROL_BIT_0_GPIO_Port GPIOC
#define RELAY_CONTROL_BIT_1_Pin GPIO_PIN_11
#define RELAY_CONTROL_BIT_1_GPIO_Port GPIOC
#define RELAY_CONTROL_BIT_2_Pin GPIO_PIN_12
#define RELAY_CONTROL_BIT_2_GPIO_Port GPIOC
#define RELAY_CONTROL_0_Pin GPIO_PIN_3
#define RELAY_CONTROL_0_GPIO_Port GPIOB
#define RELAY_CONTROL_1_Pin GPIO_PIN_4
#define RELAY_CONTROL_1_GPIO_Port GPIOB
#define RELAY_CONTROL_2_Pin GPIO_PIN_5
#define RELAY_CONTROL_2_GPIO_Port GPIOB
#define RELAY_CONTROL_3_Pin GPIO_PIN_6
#define RELAY_CONTROL_3_GPIO_Port GPIOB
#define RELAY_CONTROL_4_Pin GPIO_PIN_7
#define RELAY_CONTROL_4_GPIO_Port GPIOB
#define RELAY_CONTROL_5_Pin GPIO_PIN_8
#define RELAY_CONTROL_5_GPIO_Port GPIOB
#define RELAY_CONTROL_ENABLE_Pin GPIO_PIN_9
#define RELAY_CONTROL_ENABLE_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
