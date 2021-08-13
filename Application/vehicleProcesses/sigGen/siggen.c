/*
 * siggen.c
 *
 *  Created on: 12 Aug 2021
 *      Author: Liam Flaherty
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <vehicleProcesses/sigGen/siggen.h>

#include "FreeRTOS.h"
#include "task.h"
#include "stm32f7xx_hal.h"

#include "time/tasktimer/tasktimer.h"
#include "time/rtc/rtc.h"

#include "vehicleInterface/deviceMapping/deviceMapping.h"

// ------------------- Private data -------------------
static Logging_T* log;

#define SG_STACK_SIZE 2000
static StaticTask_t taskBuffer;
static StackType_t taskStack[SG_STACK_SIZE];

// Task data
static TaskHandle_t taskHandle;

// ------------------- Private methods -------------------
static void SigGen_TaskMain(void* pvParameters)
{
  logPrintS(log, "SigGen_TaskMain begin\n", LOGGING_DEFAULT_BUFF_LEN);
  char logBuffer[LOGGING_DEFAULT_BUFF_LEN];

  const TickType_t blockTime = 10 / portTICK_PERIOD_MS; // 10ms
  uint32_t notifiedValue;

  while (1) {
    // Wait for notification to wake up
    notifiedValue = ulTaskNotifyTake(pdTRUE, blockTime);
    if (notifiedValue > 0) {
      // ready to process


    }

  }
}

// ------------------- Public methods -------------------
SigGen_Status_T SigGen_Init(
    Logging_T* logger)
{
  log = logger;
  logPrintS(log, "SigGen_Init begin\n", LOGGING_DEFAULT_BUFF_LEN);

  // Write direction
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET); // 0 (RESET) => pull up
  // ADC2_PUP
//  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);   // 1 (SET) => pull down

  // create main task
  taskHandle = xTaskCreateStatic(
      SigTem_TaskMain,
      "SigGenTask",
      SG_STACK_SIZE,   /* Stack size */
      NULL,  /* Parameter passed as pointer */
      tskIDLE_PRIORITY,
      taskStack,
      &taskBuffer);

  // Register the task for timer notifications every 1ms
  uint16_t timerDivider = 1 * TASKTIMER_BASE_PERIOD_MS;
  TaskTimer_Status_T statusTimer = TaskTimer_RegisterTask(&taskHandle, timerDivider);
  if (TASKTIMER_STATUS_OK != statusTimer) {
    return SIGGEN_STATUS_ERROR;
  }

  logPrintS(log, "SigGen_Init complete\n", LOGGING_DEFAULT_BUFF_LEN);
  return SIGGEN_STATUS_OK;
}
