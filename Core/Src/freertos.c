/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "serial_feedback_define.h"
#include "flash.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
union G_FLASH_DS {
    int intAccess[0x400];
    float floatAccess[0x400];
    uint32_t wordAccess[0x400];
    uint8_t byteAccess[0x1000];
};
union G_TASK_DS {
    int intAccess[13];
    float floatAccess[13];
    uint32_t wordAccess[13];
    uint8_t byteAccess[52];
};
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
extern IWDG_HandleTypeDef hiwdg;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;
extern RTC_HandleTypeDef hrtc;
extern CRC_HandleTypeDef hcrc;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define RX_BUFFER_SIZE          100
#define RX_MAIN_BUFFER_SIZE     200

/* StatusReg flag define */
#define modeFlagA               31
#define modeFlagTransparent     30
#define ippFlag                 29
#define taskFlag                28
#define fathometerStatFlag1     27
#define fathometerStatFlag2     26
#define fathometerStatFlag3     25
#define fathometerStatFlag4     24
#define fathometerStatFlag5     23
#define fathometerStatFlag6     22
#define fathometerDataReadyFlag 21
#define flashDataPointerFlag    20
#define uplinkTimeoutFlag       19
#define maintenanceFlag         17
#define voltageFlag             16



/* Config address define */
#define SERIAL_NUMBER_ADDR      0x0807F000
#define BAUD_RATE_ADDR          0x0807E800
#define DFU_FLAG_ADDR           0x0807E000
#define RTC_INIT_FLAG_ADDR      0x0807D800
#define RTC_INIT_FLAG_DATA      0x04020D0A

SYSTEM_RESET_PROTOTYPE {
    HAL_Delay(600);
    __ASM volatile ("cpsid i");
    HAL_NVIC_SystemReset();
}

WRITE_FLASH_PROTOTYPE {
    if (address >= FLASH_START_ADDRESS && address <= FLASH_END_ADDRESS) {
        HAL_FLASH_Unlock();
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, data);
        HAL_FLASH_Lock();
    }
}

ERASE_FLASH_PROTOTYPE {
    int sect = 0;
    uint32_t PageError = 0;
    if (address >= FLASH_START_ADDRESS || address >= FLASH_END_ADDRESS) {
        while (address >= FLASH_START_ADDRESS + FLASH_PAGE_SIZE) {
            sect++;
            address -= FLASH_PAGE_SIZE;
        }
        FLASH_EraseInitTypeDef EraseInitStruct;
        EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
        EraseInitStruct.NbPages = 1;
        EraseInitStruct.PageAddress = FLASH_START_ADDRESS + sect * FLASH_PAGE_SIZE;
        HAL_FLASH_Unlock();
        __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_WRPERR | FLASH_SR_PGERR);
        HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);
        HAL_FLASH_Lock();
    }
}

SN_PRINT_PROTOTYPE {
    uint32_t firstWord = G_READ_FLASH(SERIAL_NUMBER_ADDR);
    uint32_t secondWord = G_READ_FLASH(SERIAL_NUMBER_ADDR + WORD_SIZE);

    if (firstWord == DEFAULT_FLASH && secondWord == DEFAULT_FLASH) {
        printf(SERIAL_FEEDBACK_ERROR_GET_SERIAL_NUMBER_NOT_SET);
    } else {
        char serialNum[9] = {};
        serialNum[8] = '\0';
        int shift = 32;
        for (int i = 0; i < 4; i++) {
            serialNum[i] = (firstWord >> (shift -= 8)) & 0xFF;
            serialNum[i + 4] = (secondWord >> (shift)) & 0xFF;
        }
        if (loc == 1) {
            printf(SERIAL_FEEDBACK_INFO_GET_SERIAL_NUMBER, serialNum);
        } else {
            printf(SERIAL_FEEDBACK_INFO_GET_SERIAL_NUMBER_INITIAL, serialNum);
        }

    }
}

SMS_REFRESH_PROTOTYPE {
    switch (status) {
        case 1: {
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_0_GPIO_Port, RELAY_CONTROL_BIT_0_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_1_GPIO_Port, RELAY_CONTROL_BIT_1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_2_GPIO_Port, RELAY_CONTROL_BIT_2_Pin, GPIO_PIN_RESET);
            break;
        }
        case 2: {
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_0_GPIO_Port, RELAY_CONTROL_BIT_0_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_1_GPIO_Port, RELAY_CONTROL_BIT_1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_2_GPIO_Port, RELAY_CONTROL_BIT_2_Pin, GPIO_PIN_RESET);
            break;
        }
        case 3: {
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_0_GPIO_Port, RELAY_CONTROL_BIT_0_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_1_GPIO_Port, RELAY_CONTROL_BIT_1_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_2_GPIO_Port, RELAY_CONTROL_BIT_2_Pin, GPIO_PIN_RESET);
            break;
        }
        case 4: {
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_0_GPIO_Port, RELAY_CONTROL_BIT_0_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_1_GPIO_Port, RELAY_CONTROL_BIT_1_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_2_GPIO_Port, RELAY_CONTROL_BIT_2_Pin, GPIO_PIN_RESET);
            break;
        }
        case 5: {
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_0_GPIO_Port, RELAY_CONTROL_BIT_0_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_1_GPIO_Port, RELAY_CONTROL_BIT_1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_2_GPIO_Port, RELAY_CONTROL_BIT_2_Pin, GPIO_PIN_SET);
            break;
        }
        case 6: {
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_0_GPIO_Port, RELAY_CONTROL_BIT_0_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_1_GPIO_Port, RELAY_CONTROL_BIT_1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_2_GPIO_Port, RELAY_CONTROL_BIT_2_Pin, GPIO_PIN_SET);
            break;
        }
        case 7: {
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_0_GPIO_Port, RELAY_CONTROL_BIT_0_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_1_GPIO_Port, RELAY_CONTROL_BIT_1_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_2_GPIO_Port, RELAY_CONTROL_BIT_2_Pin, GPIO_PIN_SET);
            break;
        }
        case 8: {
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_0_GPIO_Port, RELAY_CONTROL_BIT_0_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_1_GPIO_Port, RELAY_CONTROL_BIT_1_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_2_GPIO_Port, RELAY_CONTROL_BIT_2_Pin, GPIO_PIN_SET);
            break;
        }
        default: {
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_0_GPIO_Port, RELAY_CONTROL_BIT_0_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_1_GPIO_Port, RELAY_CONTROL_BIT_1_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_2_GPIO_Port, RELAY_CONTROL_BIT_2_Pin, GPIO_PIN_SET);
        }
    }
    HAL_GPIO_WritePin(SMS_ENABLE_GPIO_Port, SMS_ENABLE_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SMS_ENABLE_GPIO_Port, SMS_ENABLE_Pin, GPIO_PIN_RESET);
}

RELAY_CONTROL_REFRESH_PROTOTYPE {
    switch (status) {
        case 1: {
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_0_GPIO_Port, RELAY_CONTROL_BIT_0_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_1_GPIO_Port, RELAY_CONTROL_BIT_1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_2_GPIO_Port, RELAY_CONTROL_BIT_2_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_0_GPIO_Port, RELAY_CONTROL_0_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(RELAY_CONTROL_1_GPIO_Port, RELAY_CONTROL_1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_2_GPIO_Port, RELAY_CONTROL_2_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_3_GPIO_Port, RELAY_CONTROL_3_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_4_GPIO_Port, RELAY_CONTROL_4_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_5_GPIO_Port, RELAY_CONTROL_5_Pin, GPIO_PIN_RESET);
            break;
        }
        case 2: {
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_0_GPIO_Port, RELAY_CONTROL_BIT_0_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_1_GPIO_Port, RELAY_CONTROL_BIT_1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_2_GPIO_Port, RELAY_CONTROL_BIT_2_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_0_GPIO_Port, RELAY_CONTROL_0_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_1_GPIO_Port, RELAY_CONTROL_1_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(RELAY_CONTROL_2_GPIO_Port, RELAY_CONTROL_2_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_3_GPIO_Port, RELAY_CONTROL_3_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_4_GPIO_Port, RELAY_CONTROL_4_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_5_GPIO_Port, RELAY_CONTROL_5_Pin, GPIO_PIN_RESET);
            break;
        }
        case 3: {
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_0_GPIO_Port, RELAY_CONTROL_BIT_0_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_1_GPIO_Port, RELAY_CONTROL_BIT_1_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_2_GPIO_Port, RELAY_CONTROL_BIT_2_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_0_GPIO_Port, RELAY_CONTROL_0_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_1_GPIO_Port, RELAY_CONTROL_1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_2_GPIO_Port, RELAY_CONTROL_2_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(RELAY_CONTROL_3_GPIO_Port, RELAY_CONTROL_3_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_4_GPIO_Port, RELAY_CONTROL_4_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_5_GPIO_Port, RELAY_CONTROL_5_Pin, GPIO_PIN_RESET);
            break;
        }
        case 4: {
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_0_GPIO_Port, RELAY_CONTROL_BIT_0_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_1_GPIO_Port, RELAY_CONTROL_BIT_1_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_2_GPIO_Port, RELAY_CONTROL_BIT_2_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_0_GPIO_Port, RELAY_CONTROL_0_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_1_GPIO_Port, RELAY_CONTROL_1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_2_GPIO_Port, RELAY_CONTROL_2_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_3_GPIO_Port, RELAY_CONTROL_3_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(RELAY_CONTROL_4_GPIO_Port, RELAY_CONTROL_4_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_5_GPIO_Port, RELAY_CONTROL_5_Pin, GPIO_PIN_RESET);
            break;
        }
        case 5: {
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_0_GPIO_Port, RELAY_CONTROL_BIT_0_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_1_GPIO_Port, RELAY_CONTROL_BIT_1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_2_GPIO_Port, RELAY_CONTROL_BIT_2_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(RELAY_CONTROL_0_GPIO_Port, RELAY_CONTROL_0_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_1_GPIO_Port, RELAY_CONTROL_1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_2_GPIO_Port, RELAY_CONTROL_2_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_3_GPIO_Port, RELAY_CONTROL_3_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_4_GPIO_Port, RELAY_CONTROL_4_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(RELAY_CONTROL_5_GPIO_Port, RELAY_CONTROL_5_Pin, GPIO_PIN_RESET);
            break;
        }
        case 6: {
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_0_GPIO_Port, RELAY_CONTROL_BIT_0_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_1_GPIO_Port, RELAY_CONTROL_BIT_1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_2_GPIO_Port, RELAY_CONTROL_BIT_2_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(RELAY_CONTROL_0_GPIO_Port, RELAY_CONTROL_0_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_1_GPIO_Port, RELAY_CONTROL_1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_2_GPIO_Port, RELAY_CONTROL_2_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_3_GPIO_Port, RELAY_CONTROL_3_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_4_GPIO_Port, RELAY_CONTROL_4_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_5_GPIO_Port, RELAY_CONTROL_5_Pin, GPIO_PIN_SET);
            break;
        }
        default: {
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_0_GPIO_Port, RELAY_CONTROL_BIT_0_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_1_GPIO_Port, RELAY_CONTROL_BIT_1_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(RELAY_CONTROL_BIT_2_GPIO_Port, RELAY_CONTROL_BIT_2_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(RELAY_CONTROL_0_GPIO_Port, RELAY_CONTROL_0_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_1_GPIO_Port, RELAY_CONTROL_1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_2_GPIO_Port, RELAY_CONTROL_2_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_3_GPIO_Port, RELAY_CONTROL_3_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_4_GPIO_Port, RELAY_CONTROL_4_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(RELAY_CONTROL_5_GPIO_Port, RELAY_CONTROL_5_Pin, GPIO_PIN_RESET);
        }
    }
    HAL_GPIO_WritePin(RELAY_CONTROL_ENABLE_GPIO_Port, RELAY_CONTROL_ENABLE_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(RELAY_CONTROL_ENABLE_GPIO_Port, RELAY_CONTROL_ENABLE_Pin, GPIO_PIN_RESET);
}



/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
uint32_t statusReg = DEFAULT_FLASH;
uint32_t SERIAL_EVENT_FLAG = 1;
RTC_DateTypeDef currentDate;
RTC_TimeTypeDef currentTime;
uint8_t RxBuffer[RX_BUFFER_SIZE];
uint8_t RxBufferSlave[RX_BUFFER_SIZE];
uint8_t RxMainBuffer[RX_MAIN_BUFFER_SIZE];
uint8_t RxMainBufferSlave[RX_MAIN_BUFFER_SIZE];
uint32_t UID[3];
uint16_t AD_DMA_BUFFER[2] = {0};
float taskDepthBuffer[6][20] = {0};
int taskCountBuffer[6] = {0};
int currentChannel = 1;
int taskTickCount = 0;
union G_FLASH_DS flashDataStruct;
union G_TASK_DS taskDataStruct;
int flashDataPointerStart;
int flashDataPointerEnd;
int debugFlag = 0;

CONTROL_COMMAND_SEND_PROTOTYPE {
    HAL_UART_DMAStop(&huart1);
    uint8_t test[8] = {'$', 'R', 'u', 'n', '=', '1', '\r', '\n'};
    HAL_GPIO_WritePin(DE485_GPIO_Port, DE485_Pin, GPIO_PIN_SET);
    osDelay(1);
    HAL_UART_Transmit(&huart1, (uint8_t *) &test, 9, 20);
    HAL_GPIO_WritePin(DE485_GPIO_Port, DE485_Pin, GPIO_PIN_RESET);
    while (HAL_UARTEx_ReceiveToIdle_DMA(&huart1, RxBufferSlave, RX_BUFFER_SIZE) != HAL_OK);
    __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
}
/* USER CODE END Variables */
/* Definitions for mainTask */
osThreadId_t mainTaskHandle;
const osThreadAttr_t mainTask_attributes = {
        .name = "mainTask",
        .stack_size = 1024 * 4,
        .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for serialProcess */
osThreadId_t serialProcessHandle;
const osThreadAttr_t serialProcess_attributes = {
        .name = "serialProcess",
        .stack_size = 2048 * 4,
        .priority = (osPriority_t) osPriorityRealtime,
};
/* Definitions for heartbeatTimer */
osTimerId_t heartbeatTimerHandle;
const osTimerAttr_t heartbeatTimer_attributes = {
        .name = "heartbeatTimer"
};
/* Definitions for serialCommandEvent */
osEventFlagsId_t serialCommandEventHandle;
const osEventFlagsAttr_t serialCommandEvent_attributes = {
        .name = "serialCommandEvent"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void readUniqueID(volatile uint32_t *IDArrayPtr);

void intellectualPropertyProtection(void);

void intellectualPropertyProtectionSet(void);

void peripheralInit(void);

void getFlashDPTR(void);

void setFlashDPTR(int start, int end);

/* USER CODE END FunctionPrototypes */

void osMainEntry(void *argument);

void serialCommandProcess(void *argument);

void heartbeatCallback(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
    /* USER CODE BEGIN Init */
    printf(SERIAL_FEEDBACK_INFO_SYSTEM_INIT);
    /* USER CODE END Init */

    /* USER CODE BEGIN RTOS_MUTEX */
    /* add mutexes, ... */
    /* USER CODE END RTOS_MUTEX */

    /* USER CODE BEGIN RTOS_SEMAPHORES */
    /* add semaphores, ... */
    /* USER CODE END RTOS_SEMAPHORES */

    /* Create the timer(s) */
    /* creation of heartbeatTimer */
    heartbeatTimerHandle = osTimerNew(heartbeatCallback, osTimerPeriodic, NULL, &heartbeatTimer_attributes);

    /* USER CODE BEGIN RTOS_TIMERS */
    /* start timers, add new ones, ... */
    /* USER CODE END RTOS_TIMERS */

    /* USER CODE BEGIN RTOS_QUEUES */
    /* add queues, ... */
    /* USER CODE END RTOS_QUEUES */

    /* Create the thread(s) */
    /* creation of mainTask */
    mainTaskHandle = osThreadNew(osMainEntry, NULL, &mainTask_attributes);

    /* creation of serialProcess */
    serialProcessHandle = osThreadNew(serialCommandProcess, NULL, &serialProcess_attributes);

    /* USER CODE BEGIN RTOS_THREADS */
    /* add threads, ... */
    /* USER CODE END RTOS_THREADS */

    /* Create the event(s) */
    /* creation of serialCommandEvent */
    serialCommandEventHandle = osEventFlagsNew(&serialCommandEvent_attributes);

    /* USER CODE BEGIN RTOS_EVENTS */
    /* add events, ... */
    /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_osMainEntry */
/**
  * @brief  Function implementing the mainTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_osMainEntry */
void osMainEntry(void *argument) {
    /* USER CODE BEGIN osMainEntry */
    UNUSED(argument);

    osTimerStart(heartbeatTimerHandle, 100);

    intellectualPropertyProtection();

    peripheralInit();

    /* Infinite loop */
    for (;;) {

        osDelay(1000);
        vTaskSuspendAll();
        if ((flashDataPointerStart == flashDataPointerEnd) && (flashDataPointerStart > 600)) {
            setFlashDPTR(0, 0);
            getFlashDPTR();
        }
        xTaskResumeAll();
        if (GET_MAIN_VOLTAGE(AD_DMA_BUFFER[0]) > 24.5 || GET_MAIN_VOLTAGE(AD_DMA_BUFFER[0]) < 23.5) {
            G_SET_BIT(statusReg, voltageFlag);
        }
        if (BCD_TO_DECIMAL(currentTime.Hours) == 1 && BCD_TO_DECIMAL(currentTime.Minutes) == 45 &&
            BCD_TO_DECIMAL(currentTime.Minutes) <= 50) {
            if (G_GET_BIT(statusReg, taskFlag) == 1) {
                G_SYSTEM_RESET();
            }
        }

    }
    /* USER CODE END osMainEntry */
}

/* USER CODE BEGIN Header_serialCommandProcess */
/**
* @brief Function implementing the serialProcess thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_serialCommandProcess */
void serialCommandProcess(void *argument) {
    /* USER CODE BEGIN serialCommandProcess */
    UNUSED(argument);
    uint32_t flags;
    /* Infinite loop */
    for (;;) {
        /* Infinite loop */
        flags = osEventFlagsWait(serialCommandEventHandle, SERIAL_EVENT_FLAG, osFlagsWaitAll, osWaitForever);
        if (flags == SERIAL_EVENT_FLAG) {
            osDelay(1);
            switch (RxMainBuffer[2]) {
                case 'I': {
                    getFlashDPTR();
                    printf(SERIAL_FEEDBACK_INFO_QUERY_STATUS, statusReg,
                           flashDataPointerEnd - flashDataPointerStart,
                           GET_SYSTEM_VOLTAGE(AD_DMA_BUFFER[1]), GET_MAIN_VOLTAGE(AD_DMA_BUFFER[0]));
                    break;
                }
                case 'M': {
                    switch (RxMainBuffer[3]) {
                        case '0':
                            G_CLEAR_BIT(statusReg, modeFlagA);
                            G_SET_BIT(statusReg, modeFlagTransparent);
                            printf(SERIAL_FEEDBACK_INFO_SWITCH_OPERATING_MODE, RxMainBuffer[3]);
                            break;
                        case '1':
                            G_SET_BIT(statusReg, modeFlagA);
                            G_CLEAR_BIT(statusReg, modeFlagTransparent);
                            printf(SERIAL_FEEDBACK_INFO_SWITCH_OPERATING_MODE, RxMainBuffer[3]);
                            break;
                        case '2':
                            G_SET_BIT(statusReg, modeFlagA);
                            G_CLEAR_BIT(statusReg, modeFlagTransparent);
                            printf(SERIAL_FEEDBACK_INFO_SWITCH_OPERATING_MODE, RxMainBuffer[3]);
                            break;
                        case '3':
                            G_SET_BIT(statusReg, modeFlagA);
                            G_CLEAR_BIT(statusReg, modeFlagTransparent);
                            printf(SERIAL_FEEDBACK_INFO_SWITCH_OPERATING_MODE, RxMainBuffer[3]);
                            break;
                        default:
                            printf(SERIAL_FEEDBACK_ERROR_OPERATING_MODE_INVALID);
                    }
                    break;
                }
                case 'R': {
                    printf(SERIAL_FEEDBACK_INFO_SYSTEM_RESTART_PROCEED);
                    G_SYSTEM_RESET();
                    break;
                }
                case 's': {
                    G_SN_PRINT(1);
                    break;
                }
                case 'S': {
                    char snNew[8];
                    int stdFlag = 0;
                    for (int i = 0; i < 8; ++i) {
                        if (CHAR_ASCII_CRITERION(RxMainBuffer[3 + i])) {
                            snNew[7 - i] = RxMainBuffer[3 + i];
                        } else {
                            stdFlag = 1;
                            break;
                        }
                    }
                    if (stdFlag == 0) {
                        G_ERASE_FLASH(SERIAL_NUMBER_ADDR);
                        uint32_t buf;
                        memcpy(&buf, snNew, WORD_SIZE);
                        G_WRITE_FLASH(SERIAL_NUMBER_ADDR + WORD_SIZE, buf);
                        memcpy(&buf, snNew + WORD_SIZE, WORD_SIZE);
                        G_WRITE_FLASH(SERIAL_NUMBER_ADDR, buf);
                        memcpy(&snNew, snNew, 8);
                        srand(osKernelGetTickCount());
                        for (int i = 2; i < FLASH_PAGE_WORD_COUNT; ++i) {
                            G_WRITE_FLASH(SERIAL_NUMBER_ADDR + i * WORD_SIZE,
                                          rand());// NOLINT(cert-msc30-c, cert-msc50-cpp)
                        }

                        printf(SERIAL_FEEDBACK_INFO_SET_SERIAL_NUMBER, snNew[7], snNew[6],
                               snNew[5], snNew[4], snNew[3], snNew[2], snNew[1], snNew[0]);
                    } else {
                        printf(SERIAL_FEEDBACK_ERROR_ILLEGAL_SERIAL_NUMBER);
                    }
                    break;
                }
                case 'd': {
                    RTC_DATETIME_PRINT(SERIAL_FEEDBACK_INFO_GET_RTC_TIME);
                    break;
                }
                case '7': {
                    setFlashDPTR(47, 98);
                    BSP_W25Qx_Read((uint8_t *) &flashDataStruct, FLASH_SECTOR_ADDRESS(10), 0x1000);
                    for (int i = 0; i < 1024; ++i) {
                        printf("%d ", flashDataStruct.intAccess[i]);
                    }
                    printf("\r\n");
                    BSP_W25Qx_Read((uint8_t *) &flashDataStruct, FLASH_SECTOR_ADDRESS(11), 0x1000);
                    for (int i = 0; i < 1024; ++i) {
                        printf("%d ", flashDataStruct.intAccess[i]);
                    }
                    printf("\r\n");
                    break;
                }
                case '8': {
                    printf("R*EF22789445645678,wert1234,gfds4567,456e4563,gdfgd523,gfds4567,gfds4567,gfds4567\r\n");
                    printf("%lu\r\n", uxTaskGetStackHighWaterMark(serialProcessHandle));
                    taskTickCount = 0;
                    BSP_W25Qx_Erase_Chip();
                    break;
                }
                case 'T': {
                    getFlashDPTR();
                    char taskSetBuffer[14];
                    int stdFlag = 0;
                    for (int i = 0; i < 14; ++i) {
                        if (CHAR_NUMBER_CRITERION(RxMainBuffer[3 + i])) {
                            taskSetBuffer[i] = RxMainBuffer[3 + i];
                            stdFlag++;
                        } else {
                            break;
                        }
                    }
                    if (stdFlag == 14) {
                        int year = CHAR_TO_INT(taskSetBuffer[2]) * 10 +
                                   CHAR_TO_INT(taskSetBuffer[3]);
                        int month = CHAR_TO_INT(taskSetBuffer[4]) * 10 +
                                    CHAR_TO_INT(taskSetBuffer[5]);
                        int day = CHAR_TO_INT(taskSetBuffer[6]) * 10 +
                                  CHAR_TO_INT(taskSetBuffer[7]);
                        int hour = CHAR_TO_INT(taskSetBuffer[8]) * 10 +
                                   CHAR_TO_INT(taskSetBuffer[9]);
                        int minute = CHAR_TO_INT(taskSetBuffer[10]) * 10 +
                                     CHAR_TO_INT(taskSetBuffer[11]);
                        int second = CHAR_TO_INT(taskSetBuffer[12]) * 10 +
                                     CHAR_TO_INT(taskSetBuffer[13]);
                        if (DATETIME_CRITERION(year, month, day, hour, minute, second)) {
                            if (G_GET_BIT(statusReg, taskFlag)) {
                                for (int i = 0; i < 6; ++i) {
                                    for (int j = 0; j < 20; ++j) {
                                        taskDepthBuffer[i][j] = 0;
                                    }
                                    taskCountBuffer[i] = 0;
                                }
                                taskDataStruct.intAccess[0] = year;
                                taskDataStruct.intAccess[1] = month;
                                taskDataStruct.intAccess[2] = day;
                                taskDataStruct.intAccess[3] = hour;
                                taskDataStruct.intAccess[4] = minute;
                                taskDataStruct.intAccess[5] = second;
                                taskDataStruct.intAccess[6] = flashDataPointerEnd + 1;

                                int zMonth = month, zYear = year, zYearPrefix = year / 100;
                                if (month == 1 || month == 2) {
                                    zMonth += 12;
                                    zYear--;
                                }
                                int weekdayJudge = (zYear + zYear / 4 + zYearPrefix / 4 - 2 * zYearPrefix +
                                                    26 * (zMonth + 1) / 10 + day - 1) % 7;
                                RTC_DateTypeDef DateToUpdate = {0};
                                switch (weekdayJudge) {
                                    case 1: {
                                        DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
                                    }
                                    case 2: {
                                        DateToUpdate.WeekDay = RTC_WEEKDAY_TUESDAY;
                                    }
                                    case 3: {
                                        DateToUpdate.WeekDay = RTC_WEEKDAY_WEDNESDAY;
                                    }
                                    case 4: {
                                        DateToUpdate.WeekDay = RTC_WEEKDAY_THURSDAY;
                                    }
                                    case 5: {
                                        DateToUpdate.WeekDay = RTC_WEEKDAY_FRIDAY;
                                    }
                                    case 6: {
                                        DateToUpdate.WeekDay = RTC_WEEKDAY_SATURDAY;
                                    }
                                    case 0: {
                                        DateToUpdate.WeekDay = RTC_WEEKDAY_SUNDAY;
                                    }
                                    default: {
                                        DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
                                    }
                                }
                                DateToUpdate.Month = (month);
                                DateToUpdate.Date = (day);
                                DateToUpdate.Year = (year);
                                RTC_TimeTypeDef sTime = {0};
                                sTime.Hours = DECIMAL_TO_BCD(hour);
                                sTime.Minutes = DECIMAL_TO_BCD(minute);
                                sTime.Seconds = DECIMAL_TO_BCD(second);
                                HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BCD);
                                HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD);
                                printf(SERIAL_FEEDBACK_INFO_TASK_CREATE, taskDataStruct.intAccess[6]);
                                G_CLEAR_BIT(statusReg, taskFlag);
                            } else {
                                printf(SERIAL_FEEDBACK_ERROR_TASK_BUSY);
                            }
                        } else {
                            printf(SERIAL_FEEDBACK_ERROR_TASK_TIME_SYNC_FAIL);
                        }
                    } else {
                        printf(SERIAL_FEEDBACK_ERROR_TASK_TIME_SYNC_FAIL);
                    }
                    break;
                }
                case 'F': {
                    getFlashDPTR();
                    if (flashDataPointerStart == flashDataPointerEnd) {
                        printf(SERIAL_FEEDBACK_ERROR_FLASH__POINTER);
                    } else {
                        BSP_W25Qx_Read((uint8_t *) &flashDataStruct, FLASH_SECTOR_ADDRESS(flashDataPointerStart + 101),
                                       52);
                        printf(SERIAL_FEEDBACK_INFO_FETCH_DATA,
                               flashDataStruct.intAccess[0], flashDataStruct.intAccess[1], flashDataStruct.intAccess[2],
                               flashDataStruct.intAccess[3], flashDataStruct.intAccess[4], flashDataStruct.intAccess[5],
                               flashDataStruct.intAccess[6], flashDataStruct.wordAccess[7],
                               flashDataStruct.wordAccess[8], flashDataStruct.wordAccess[9],
                               flashDataStruct.wordAccess[10], flashDataStruct.wordAccess[11],
                               flashDataStruct.wordAccess[12]);

                    }
                    break;
                }
                case 'C': {
                    int stdFlag = 0;
                    for (int i = 0; i < 8; ++i) {
                        if (CHAR_ASCII_CRITERION(RxMainBuffer[3 + i])) {
                        } else {
                            stdFlag = 1;
                            break;
                        }
                    }
                    if (stdFlag == 0) {
                        getFlashDPTR();
                        if (flashDataPointerStart == flashDataPointerEnd) {
                            printf(SERIAL_FEEDBACK_ERROR_FLASH__POINTER);
                        } else {
                            int crc = CHAR_TO_INT(RxMainBuffer[3]) * 16 * 16 * 16 * 16 * 16 * 16 * 16 +
                                      CHAR_TO_INT(RxMainBuffer[4]) * 16 * 16 * 16 * 16 * 16 * 16 +
                                      CHAR_TO_INT(RxMainBuffer[5]) * 16 * 16 * 16 * 16 * 16 +
                                      CHAR_TO_INT(RxMainBuffer[6]) * 16 * 16 * 16 * 16 +
                                      CHAR_TO_INT(RxMainBuffer[7]) * 16 * 16 * 16 +
                                      CHAR_TO_INT(RxMainBuffer[8]) * 16 * 16 +
                                      CHAR_TO_INT(RxMainBuffer[9]) * 16 +
                                      CHAR_TO_INT(RxMainBuffer[10]);
                            BSP_W25Qx_Read((uint8_t *) &flashDataStruct,
                                           FLASH_SECTOR_ADDRESS(flashDataPointerStart + 101),
                                           52);
                            uint32_t Data_buffer[6] = {flashDataStruct.wordAccess[7], flashDataStruct.wordAccess[8],
                                                       flashDataStruct.wordAccess[9], flashDataStruct.wordAccess[10],
                                                       flashDataStruct.wordAccess[11], flashDataStruct.wordAccess[12]};
                            if (crc == HAL_CRC_Calculate(&hcrc, Data_buffer, 6)) {
                                printf(SERIAL_FEEDBACK_INFO_CRC_PASS, flashDataStruct.intAccess[0],
                                       flashDataStruct.intAccess[1], flashDataStruct.intAccess[2],
                                       flashDataStruct.intAccess[3], flashDataStruct.intAccess[4],
                                       flashDataStruct.intAccess[5]);
                                setFlashDPTR(flashDataPointerStart + 1, flashDataPointerEnd);
                            } else {
                                printf(SERIAL_FEEDBACK_ERROR_CRC_FAIL);
                            }
                        }
                    } else {
                        printf(SERIAL_FEEDBACK_ERROR_CRC_ILLEGAL);
                    }

                    break;
                }
                case 'E': {
                    setFlashDPTR(0, 0);
                    printf(SERIAL_FEEDBACK_INFO_FLASH_ERASE);
                    break;
                }
                case 'D': {
                    if (debugFlag == 0) {
                        debugFlag = 1;
                        printf(SERIAL_FEEDBACK_INFO_DEBUG_ON);
                    } else {
                        debugFlag = 0;
                        printf(SERIAL_FEEDBACK_INFO_DEBUG_OFF);
                    }
                    break;
                }
                default: {
                    printf(SERIAL_FEEDBACK_ERROR_COMMAND_NOT_FOUND);
                }

            }
            G_SMS_REFRESH(currentChannel);
            HAL_UARTEx_ReceiveToIdle_DMA(&huart2, RxBuffer, RX_BUFFER_SIZE);
            __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);
        }
    }
    /* USER CODE END serialCommandProcess */
}

/* heartbeatCallback function */
void heartbeatCallback(void *argument) {
    /* USER CODE BEGIN heartbeatCallback */
    UNUSED(argument);
    HAL_IWDG_Refresh(&hiwdg);

    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, RxBufferSlave, RX_BUFFER_SIZE);
    __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, RxBuffer, RX_BUFFER_SIZE);
    __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);

    HAL_RTC_GetTime(&hrtc, &currentTime, RTC_FORMAT_BCD);
    HAL_RTC_GetDate(&hrtc, &currentDate, RTC_FORMAT_BCD);

    if (!G_GET_BIT(statusReg, taskFlag)) {

        switch (taskTickCount) {
            case 0: {
                G_SET_BIT(statusReg, fathometerDataReadyFlag);
                currentChannel = 1;
                G_RELAY_CONTROL_REFRESH(currentChannel);
                G_SMS_REFRESH(currentChannel);
                break;
            }
            case 1: {
                G_CONTROL_COMMAND_SEND();
                break;
            }
            case 3:
            case 4:
            case 5:
            case 6: {
                if (G_GET_BIT(statusReg, fathometerDataReadyFlag) == 1) {
                    G_CONTROL_COMMAND_SEND();
                    G_SET_BIT(statusReg, fathometerStatFlag1);
                } else {
                    G_CLEAR_BIT(statusReg, fathometerStatFlag1);
                }
                break;
            }
            case 30: {
                G_SET_BIT(statusReg, fathometerDataReadyFlag);
                currentChannel = 2;
                G_RELAY_CONTROL_REFRESH(currentChannel);
                G_SMS_REFRESH(currentChannel);
                break;
            }
            case 31: {
                G_CONTROL_COMMAND_SEND();
                break;
            }
            case 33:
            case 34:
            case 35:
            case 36: {
                if (G_GET_BIT(statusReg, fathometerDataReadyFlag) == 1) {
                    G_CONTROL_COMMAND_SEND();
                    G_SET_BIT(statusReg, fathometerStatFlag2);
                } else {
                    G_CLEAR_BIT(statusReg, fathometerStatFlag2);
                }
                break;
            }
            case 60: {
                G_SET_BIT(statusReg, fathometerDataReadyFlag);
                currentChannel = 3;
                G_RELAY_CONTROL_REFRESH(currentChannel);
                break;
            }
            case 61: {
                G_CONTROL_COMMAND_SEND();
                G_SMS_REFRESH(currentChannel);
                break;
            }
            case 63:
            case 64:
            case 65:
            case 66: {
                if (G_GET_BIT(statusReg, fathometerDataReadyFlag) == 1) {
                    G_CONTROL_COMMAND_SEND();
                    G_SET_BIT(statusReg, fathometerStatFlag3);
                } else {
                    G_CLEAR_BIT(statusReg, fathometerStatFlag3);
                }
                break;
            }
            case 90: {
                G_SET_BIT(statusReg, fathometerDataReadyFlag);
                currentChannel = 4;
                G_RELAY_CONTROL_REFRESH(currentChannel);
                break;
            }
            case 91: {
                G_CONTROL_COMMAND_SEND();
                G_SMS_REFRESH(currentChannel);
                break;
            }
            case 93:
            case 94:
            case 95:
            case 96:{
                if (G_GET_BIT(statusReg, fathometerDataReadyFlag) == 1) {
                    G_CONTROL_COMMAND_SEND();
                    G_SET_BIT(statusReg, fathometerStatFlag4);
                } else {
                    G_CLEAR_BIT(statusReg, fathometerStatFlag4);
                }
                break;
            }
            case 120: {
                G_SET_BIT(statusReg, fathometerDataReadyFlag);
                currentChannel = 5;
                G_RELAY_CONTROL_REFRESH(currentChannel);
                break;
            }
            case 121: {
                G_CONTROL_COMMAND_SEND();
                G_SMS_REFRESH(currentChannel);
                break;
            }
            case 123:
            case 124:
            case 125:
            case 126: {
                if (G_GET_BIT(statusReg, fathometerDataReadyFlag) == 1) {
                    G_CONTROL_COMMAND_SEND();
                    G_SET_BIT(statusReg, fathometerStatFlag5);
                } else {
                    G_CLEAR_BIT(statusReg, fathometerStatFlag5);
                }
                break;
            }
            case 150: {
                G_SET_BIT(statusReg, fathometerDataReadyFlag);
                currentChannel = 6;
                G_RELAY_CONTROL_REFRESH(currentChannel);
                break;
            }
            case 151: {
                G_CONTROL_COMMAND_SEND();
                G_SMS_REFRESH(currentChannel);
                break;
            }
            case 153:
            case 154:
            case 155:
            case 156: {
                if (G_GET_BIT(statusReg, fathometerDataReadyFlag) == 1) {
                    G_CONTROL_COMMAND_SEND();
                    G_SET_BIT(statusReg, fathometerStatFlag6);
                } else {
                    G_CLEAR_BIT(statusReg, fathometerStatFlag6);
                }
                break;
            }
            default: {
                if (taskTickCount >= 180) {
                    G_RELAY_CONTROL_REFRESH(10);
                    taskTickCount = -1;
                    G_SET_BIT(statusReg, taskFlag);
                    for (int i = 0; i < 6; ++i) {
                        float sum = 0;
                        for (int j = 0; j < 20; ++j) {
                            sum += taskDepthBuffer[i][j];
                        }
                        taskDataStruct.floatAccess[7 + i] = taskCountBuffer[i] == 0 ?
                                                            0 : sum / (float) (taskCountBuffer[i] > 20 ? 20
                                                                                                       : taskCountBuffer[i]);
                    }
//                    printf("%d %d %d %d %d %d | %d | %.2f %.2f %.2f %.2f %.2f %.2f\r\n", taskDataStruct.intAccess[0],
//                           taskDataStruct.intAccess[1], taskDataStruct.intAccess[2], taskDataStruct.intAccess[3],
//                           taskDataStruct.intAccess[4], taskDataStruct.intAccess[5], taskDataStruct.intAccess[6],
//                           taskDataStruct.floatAccess[7], taskDataStruct.floatAccess[8], taskDataStruct.floatAccess[9],
//                           taskDataStruct.floatAccess[10], taskDataStruct.floatAccess[11],
//                           taskDataStruct.floatAccess[12]);
                    BSP_W25Qx_Erase_Block(FLASH_SECTOR_ADDRESS(taskDataStruct.intAccess[6] + 100));
                    BSP_W25Qx_Write((uint8_t *) &taskDataStruct,
                                    FLASH_SECTOR_ADDRESS(taskDataStruct.intAccess[6] + 100),
                                    52);
                    setFlashDPTR(flashDataPointerStart, taskDataStruct.intAccess[6]);
                }
            }
        }
        taskTickCount++;
    }

    /* USER CODE END heartbeatCallback */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
    if (huart->Instance == USART2) {
        if (Size == RX_BUFFER_SIZE && RxBuffer[Size - 1] != 0x0A) {
            printf(SERIAL_FEEDBACK_ERROR_COMMAND_OVERFLOW);
            HAL_UARTEx_ReceiveToIdle_DMA(&huart2, RxBuffer, RX_BUFFER_SIZE);
            __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);
        } else {
            memcpy((uint8_t *) RxMainBuffer, RxBuffer, Size);
            if (RxBuffer[0] == 0x47 && RxBuffer[1] == 0x2B && RxBuffer[Size - 2] == 0x0D &&
                RxBuffer[Size - 1] == 0x0A) {
                G_SMS_REFRESH(7);
                osEventFlagsSet(serialCommandEventHandle, SERIAL_EVENT_FLAG);
            } else {
                HAL_UARTEx_ReceiveToIdle_DMA(&huart2, RxBuffer, RX_BUFFER_SIZE);
                __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);
            }
        }
    } else if (huart->Instance == USART1) {
        G_SMS_REFRESH(8);
        memcpy((uint8_t *) RxMainBufferSlave, RxBufferSlave, Size);
        uint16_t SizeBackup = Size;
//        for (int i = 0; i < Size; ++i) {
//            HAL_UART_Transmit(&huart2, (uint8_t *) &RxMainBufferSlave[i], 1, 10);
//        }

        int dollarFlag = -1, crFlag = -1, lfFlag = -1, processFlag = 0;
        for (int i = 0; i < Size; ++i) {
            if (RxMainBufferSlave[i] == 0x24) {
                dollarFlag = i;
                crFlag = -1;
                lfFlag = -1;
            } else if (RxMainBufferSlave[i] == 0x0D) {
                crFlag = i;
            } else if (RxMainBufferSlave[i] == 0x0A) {
                lfFlag = i;
            }
            if (dollarFlag != -1 && crFlag != -1 && lfFlag != -1) {
                break;
            }
        }
        if (dollarFlag != -1 && crFlag != -1 && lfFlag != -1 && lfFlag - crFlag == 1) {
            if (dollarFlag != 0) {
                for (int i = 0; i < Size - dollarFlag; ++i) {
                    RxMainBufferSlave[i] = RxMainBufferSlave[i + dollarFlag];
                }
            }
            Size = lfFlag - dollarFlag + 1;
            if (RxMainBufferSlave[Size - 5] == 0x2A && RxMainBufferSlave[Size - 6] == 0x2C &&
                RxMainBufferSlave[Size - 8] == 0x2C) {
                if (RxMainBufferSlave[Size - 7] == 0x20) {
                    char checkSum = RxMainBufferSlave[1];
                    for (int i = 0; i < Size - 6; ++i) {
                        checkSum = checkSum ^ RxMainBufferSlave[2 + i];
                    }
                    if (CHAR_TO_INT(RxMainBufferSlave[Size - 3]) +
                        CHAR_TO_INT(RxMainBufferSlave[Size - 4]) * 16 == checkSum) {
                        int depthPointer = 0;
                        float depth = 0;
                        for (int i = 0; i < Size; ++i) {
                            if (RxMainBufferSlave[i] == 0x2E) {
                                depthPointer = i;
                                break;
                            }
                        }
                        for (int i = 1; i < depthPointer; ++i) {
                            depth += (float) (CHAR_TO_INT(RxMainBufferSlave[i]) * pow(10, depthPointer - i - 1));
                        }
                        for (int i = depthPointer + 1; i < Size - 8; ++i) {
                            depth += (float) (CHAR_TO_INT(RxMainBufferSlave[i]) * pow(10, depthPointer - i));
                        }
                        taskCountBuffer[currentChannel - 1]++;
                        for (int i = 0; i < 19; ++i) {
                            taskDepthBuffer[currentChannel - 1][i] = taskDepthBuffer[currentChannel - 1][i + 1];
                        }
                        taskDepthBuffer[currentChannel - 1][19] = depth;
                        processFlag = 1;
                        printf("%.2f\r\n", depth);
                        G_CLEAR_BIT(statusReg, fathometerDataReadyFlag);
                    }
                }
            }
        }
        if (processFlag == 0 && debugFlag == 1) {
            printf("[Debug Serial Receive]");
            for (int i = 0; i < SizeBackup; ++i) {
                printf("%c", RxMainBufferSlave[i]);
            }
            printf("[EndDebug]\r\n");
        }
        G_SMS_REFRESH(currentChannel);
        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, RxBufferSlave, RX_BUFFER_SIZE);
        __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
    }
}

void readUniqueID(volatile uint32_t *IDArrayPtr) {
    volatile uint32_t Addr = 0x20000006;
    Addr -= 0x800;
    IDArrayPtr[0] = G_READ_FLASH(Addr -= 0x1E);
    IDArrayPtr[1] = G_READ_FLASH(Addr += WORD_SIZE);
    IDArrayPtr[2] = G_READ_FLASH(Addr += WORD_SIZE);
}

void intellectualPropertyProtection(void) {
    readUniqueID((volatile uint32_t *) UID);
    volatile uint32_t baseAddr = IPP_BASE_ADDRESS;
    if (G_READ_FLASH(baseAddr -= 0x17F80804) == IPP_CRITERION_1) {
        uint32_t ippData = G_READ_FLASH(baseAddr += WORD_SIZE);
        ippData &= G_READ_FLASH(baseAddr += WORD_SIZE);
        if ((ippData |= G_READ_FLASH(baseAddr += WORD_SIZE)) == IPP_CRITERION_2) {
            intellectualPropertyProtectionSet();
        }
    }
    baseAddr = IPP_BASE_ADDRESS;
    if (G_READ_FLASH(baseAddr -= 0x17F807E0) == (UID[0] & (UID[1] | UID[2]))) {
        G_CLEAR_BIT(statusReg, ippFlag);
    } else {
        G_SET_BIT(statusReg, ippFlag);
        printf(SERIAL_FEEDBACK_ERROR_IPP_TRIGGERED);
        printf(SERIAL_FEEDBACK_WARNING_IPP_LEGAL_NOTICE);
        if (osThreadTerminate(serialProcessHandle) == osOK) {
            printf(SERIAL_FEEDBACK_INFO_SHUTDOWN_SERIAL);
        }
        if (osTimerStop(heartbeatTimerHandle) == osOK) {
            printf(SERIAL_FEEDBACK_INFO_SHUTDOWN_HEARTBEAT);
        }
        printf(SERIAL_FEEDBACK_INFO_CONTACT_INFO);

        int dotCount = 0;
        while (1) {
            osDelay(100);
            HAL_IWDG_Refresh(&hiwdg);
            printf(".");
            fflush(stdout);
            dotCount++;
            if (dotCount == 10) {
                //TODO: Self-Destruct in Bootloader
                for (int i = 0; i < 255; ++i) {
                    G_ERASE_FLASH(FLASH_END_ADDRESS - (i + 1) * FLASH_PAGE_SIZE);
                    HAL_IWDG_Refresh(&hiwdg);
                }
            }
        }
    }
}

void intellectualPropertyProtectionSet(void) {
    volatile uint32_t baseAddr = IPP_BASE_ADDRESS;
    baseAddr -= 0x17F807E0;
    G_ERASE_FLASH(baseAddr);
    G_WRITE_FLASH(baseAddr, UID[0] & (UID[1] | UID[2]));

    srand(osKernelGetTickCount());
    for (int i = 1; i < IPP_BLANK_AFTER; ++i) {
        G_WRITE_FLASH(baseAddr + i * WORD_SIZE, rand());// NOLINT(cert-msc30-c, cert-msc50-cpp)
    }
    for (int i = 1; i < IPP_BLANK_BEFORE; ++i) {
        G_WRITE_FLASH(baseAddr - i * WORD_SIZE, rand());// NOLINT(cert-msc30-c, cert-msc50-cpp)
    }


    G_CLEAR_BIT(statusReg, ippFlag);
    printf(SERIAL_FEEDBACK_INFO_IPP_SET);

//    FLASH_OBProgramInitTypeDef OBInit;
//    __HAL_FLASH_PREFETCH_BUFFER_DISABLE();
//    HAL_FLASHEx_OBGetConfig(&OBInit);
//    if (OBInit.RDPLevel == OB_RDP_LEVEL_0) {
//        printf("[Info] Factory Mode: Readout Protection is now set, RDP option byte programmed\r\n");
//        for (int i = 0; i < 3; ++i) {
//            printf("[Warning] Factory Mode: Please restart device......\r\n");
//        }
//        printf("\r\n");
//
//        OBInit.OptionType = OPTIONBYTE_RDP;
//        OBInit.RDPLevel = OB_RDP_LEVEL_1;
//        HAL_FLASH_Unlock();
//        HAL_FLASH_OB_Unlock();
//        HAL_FLASHEx_OBProgram(&OBInit);
//        HAL_FLASH_OB_Launch();
//        HAL_FLASH_OB_Lock();
//        HAL_FLASH_Lock();
//    }
//    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();

    G_SYSTEM_RESET();
}

void peripheralInit(void) {

    //Firmware Version
    printf(SERIAL_FEEDBACK_INFO_GET_FIRMWARE_VERSION, "v1.0.0");

    //Serial Number
    G_SN_PRINT(0);
    HAL_GPIO_WritePin(RS485_232_GPIO_Port, RS485_232_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(DE485_GPIO_Port, DE485_Pin, GPIO_PIN_RESET);

    //Baud Rate
    printf(SERIAL_FEEDBACK_INFO_GET_BAUD_RATE);

    //SMS/POST Init
    for (int i = 1; i < 9; i++) {
        G_SMS_REFRESH(i);
        HAL_Delay(20);
    }
    for (int i = 7; i > 0; i--) {
        G_SMS_REFRESH(i);
        HAL_Delay(20);
    }

    //RTC Init
    if (G_READ_FLASH(RTC_INIT_FLAG_ADDR) == RTC_INIT_FLAG_DATA) {
        HAL_RTC_GetTime(&hrtc, &currentTime, RTC_FORMAT_BCD);
        HAL_RTC_GetDate(&hrtc, &currentDate, RTC_FORMAT_BCD);
        RTC_DATETIME_PRINT(SERIAL_FEEDBACK_INFO_RTC_INIT);
    }

    //ADC Init
    HAL_ADCEx_Calibration_Start(&hadc1);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *) &AD_DMA_BUFFER, 2);
    HAL_Delay(10);
    printf(SERIAL_FEEDBACK_INFO_ADC_INIT, GET_SYSTEM_VOLTAGE(AD_DMA_BUFFER[1]), GET_MAIN_VOLTAGE(AD_DMA_BUFFER[0]));

    //W25Q64 Init
    uint8_t ID[4];
    BSP_W25Qx_Init();
    BSP_W25Qx_Read_ID(ID);
    if ((ID[0] == 0xEF) && (ID[1] == 0x16)) {
        printf(SERIAL_FEEDBACK_INFO_FLASH_INIT);
    }
    getFlashDPTR();
    G_CLEAR_BIT(statusReg, flashDataPointerFlag);
    printf(SERIAL_FEEDBACK_INFO_RECORDS_ALERT, flashDataPointerEnd - flashDataPointerStart);

    //Serial Terminal Init
    while (HAL_UARTEx_ReceiveToIdle_DMA(&huart2, RxBuffer, RX_BUFFER_SIZE) != HAL_OK);
    __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);
    while (HAL_UARTEx_ReceiveToIdle_DMA(&huart1, RxBufferSlave, RX_BUFFER_SIZE) != HAL_OK);
    __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);

    G_CLEAR_BIT(statusReg, voltageFlag);
}

void getFlashDPTR(void) {
    BSP_W25Qx_Read((uint8_t *) &flashDataStruct, FLASH_SECTOR_ADDRESS(10), 0x1000);
    for (int i = 0x3FF; i >= 0; --i) {
        if (flashDataStruct.wordAccess[i] != 0xFFFFFFFF) {
            flashDataPointerStart = flashDataStruct.intAccess[i];
            break;
        }
        if (i == 0) {
            printf(SERIAL_FEEDBACK_INFO_FLASH_D_PTR_START_INIT);
            flashDataPointerStart = 0;
            flashDataStruct.intAccess[0] = 0;
            BSP_W25Qx_Write((uint8_t *) &flashDataStruct, FLASH_SECTOR_ADDRESS(10), 0x4);
        }
    }
    BSP_W25Qx_Read((uint8_t *) &flashDataStruct, FLASH_SECTOR_ADDRESS(11), 0x1000);
    for (int i = 0x3FF; i >= 0; --i) {
        if (flashDataStruct.wordAccess[i] != 0xFFFFFFFF) {
            flashDataPointerEnd = flashDataStruct.intAccess[i];
            break;
        }
        if (i == 0) {
            printf(SERIAL_FEEDBACK_INFO_FLASH_D_PTR_END_INIT);
            flashDataPointerEnd = 0;
            flashDataStruct.intAccess[0] = 0;
            BSP_W25Qx_Write((uint8_t *) &flashDataStruct, FLASH_SECTOR_ADDRESS(11), 0x4);
        }
    }
}

void setFlashDPTR(int start, int end) {
    BSP_W25Qx_Read((uint8_t *) &flashDataStruct, FLASH_SECTOR_ADDRESS(10), 0x1000);
    for (int i = 0; i < 0x400; ++i) {
        if (flashDataStruct.wordAccess[i] == 0xFFFFFFFF) {
            flashDataStruct.intAccess[0] = start;
            BSP_W25Qx_Write((uint8_t *) &flashDataStruct, FLASH_SECTOR_ADDRESS(10) + i * 4, 0x4);
            break;
        }
        if (i == 0x3FF) {
            BSP_W25Qx_Erase_Block(FLASH_SECTOR_ADDRESS(10));
            flashDataStruct.intAccess[0] = start;
            BSP_W25Qx_Write((uint8_t *) &flashDataStruct, FLASH_SECTOR_ADDRESS(10), 0x4);
        }
    }

    BSP_W25Qx_Read((uint8_t *) &flashDataStruct, FLASH_SECTOR_ADDRESS(11), 0x1000);
    for (int i = 0; i < 0x400; ++i) {
        if (flashDataStruct.wordAccess[i] == 0xFFFFFFFF) {
            flashDataStruct.intAccess[0] = end;
            BSP_W25Qx_Write((uint8_t *) &flashDataStruct, FLASH_SECTOR_ADDRESS(11) + i * 4, 0x4);
            break;
        }
        if (i == 0x3FF) {
            BSP_W25Qx_Erase_Block(FLASH_SECTOR_ADDRESS(11));
            flashDataStruct.intAccess[0] = end;
            BSP_W25Qx_Write((uint8_t *) &flashDataStruct, FLASH_SECTOR_ADDRESS(11), 0x4);
        }
    }
}
/* USER CODE END Application */

