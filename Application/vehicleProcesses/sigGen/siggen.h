/*
 * siggen.h
 *
 * PWM sine wave generator
 *
 *  Created on: 12 Aug 2021
 *      Author: Liam Flaherty
 */

#ifndef VEHICLEPROCESSES_SIGGEN_SIGGEN_H
#define VEHICLEPROCESSES_SIGGEN_SIGGEN_H

#include "lib/logging/logging.h"

typedef enum
{
  SIGGEN_STATUS_OK     = 0x00U,
  SIGGEN_STATUS_ERROR  = 0x01U
} SigGen_Status_T;

/**
 * @brief Initialize the process
 * @param logger Pointer to system logger
 */
SigGen_Status_T SigGen_Init(
    Logging_T* logger);

#endif /* VEHICLEPROCESSES_SIGGEN_SIGGEN_H */
