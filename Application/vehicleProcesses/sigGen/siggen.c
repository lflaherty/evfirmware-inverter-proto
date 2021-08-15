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

static uint32_t duty_cycle = 0;
static uint64_t counter = 0;
static float frequency = 100.0f; // Hz
static const float TWO_PI = (float)M_2_PI;
static float TWO_PI_F;

// ------------------- Private methods -------------------
static void SigGen_TaskMain(void* pvParameters)
{
  logPrintS(logging, "SigGen_TaskMain begin\n", LOGGING_DEFAULT_BUFF_LEN);

  TWO_PI_F = TWO_PI * frequency;

  while (1) {
    // Wait for notification to wake up
    uint32_t notifiedValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    if (notifiedValue > 0) {
      // ready to process

      // Toggle debug indicator pin
      HAL_GPIO_TogglePin(DEBUG_OUT_GPIO_Port, DEBUG_OUT_Pin);

      // Hard set PWM to 50% for now
//      duty_cycle++;
//      if (duty_cycle > 19999) {
//        duty_cycle = 0;
//      }
//      TIM8->CCR3 = 19999/5; // duty_cycle;

      counter++;

      float tSec = counter * 0.0001f;
      float theta = TWO_PI_F * tSec;
      float sine = 0.5f * sin(theta) + 0.5f; // produce sine value and offset (so it is bounded [0,1])
      duty_cycle = 19999 * sine;  // scale to duty cycle values

      TIM8->CCR3 = duty_cycle;
    }

  }
}

// ------------------- Public methods -------------------
SigGen_Status_T SigGen_Init(
    Logging_T* logger)
{
  logging = logger;
  logPrintS(logging, "SigGen_Init begin\n", LOGGING_DEFAULT_BUFF_LEN);

  // Write direction (because of the weird prototype chip that's attached to PC7
  HAL_GPIO_WritePin(GPIO1_DIR_GPIO_Port, GPIO1_DIR_Pin, GPIO_PIN_RESET);  // data flows B->A (output from ECU)

  // Start the PWM
  HAL_TIM_PWM_Start(Mapping_GetTimerPhA(), MAPPING_TIM_CHANNEL_PHA_HIGH);

  // create main task
  taskHandle = xTaskCreateStatic(
      SigGen_TaskMain,
      "SigGenTask",
      SG_STACK_SIZE,   /* Stack size */
      NULL,  /* Parameter passed as pointer */
      SIGGEN_RTOS_PRIORITY,
      taskStack,
      &taskBuffer);

  // Register the task for timer notifications every 100us
  uint32_t timerDivider = 1 * TASKTIMER_BASE_PERIOD_100US;
  TaskTimer_Status_T statusTimer = TaskTimer_RegisterTask(&taskHandle, timerDivider);
  if (TASKTIMER_STATUS_OK != statusTimer) {
    return SIGGEN_STATUS_ERROR;
  }

  logPrintS(logging, "SigGen_Init complete\n", LOGGING_DEFAULT_BUFF_LEN);
  return SIGGEN_STATUS_OK;
}
