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
static Logging_T* logging;

#define SG_STACK_SIZE 2000
static StaticTask_t taskBuffer;
static StackType_t taskStack[SG_STACK_SIZE];

// Task data
static TaskHandle_t taskHandle;

// ------------------- Private methods -------------------
static void SigGen_TaskMain(void* pvParameters)
{
  logPrintS(logging, "SigGen_TaskMain begin\n", LOGGING_DEFAULT_BUFF_LEN);
  char logBuffer[LOGGING_DEFAULT_BUFF_LEN];

  const TickType_t blockTime = 10 / portTICK_PERIOD_MS; // 10ms
  uint32_t notifiedValue;

  while (1) {
    // Wait for notification to wake up
    notifiedValue = ulTaskNotifyTake(pdTRUE, blockTime);
    if (notifiedValue > 0) {
      // ready to process

      HAL_GPIO_TogglePin(GPIO1_GPIO_Port, GPIO1_Pin);
    }

  }
}

// ------------------- Public methods -------------------
SigGen_Status_T SigGen_Init(
    Logging_T* logger)
{
  logging = logger;
  logPrintS(logging, "SigGen_Init begin\n", LOGGING_DEFAULT_BUFF_LEN);

  // Write direction
  HAL_GPIO_WritePin(GPIO1_DIR_GPIO_Port, GPIO1_DIR_Pin, GPIO_PIN_RESET);  // data flows B->A (output from ECU)

  // create main task
  taskHandle = xTaskCreateStatic(
      SigGen_TaskMain,
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

  logPrintS(logging, "SigGen_Init complete\n", LOGGING_DEFAULT_BUFF_LEN);
  return SIGGEN_STATUS_OK;
}
