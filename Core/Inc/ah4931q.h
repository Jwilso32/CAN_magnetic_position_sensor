/**
  ******************************************************************************
  * @file           : ah4931q.h
  * @brief          : Header for ah4931.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Author: John Wilson
  * 2026
  *
  ******************************************************************************
  */

#ifndef AH4931Q_H
#define AH4931Q_H

#include <stddef.h>
#include <stdint.h>
#include <_ansi.h>

// Choose a microcontroller family
//#define STM32F0
//#define STM32F1
//#define STM32F4
//#define STM32L0
//#define STM32L1
//#define STM32L4
//#define STM32F3
//#define STM32H7
//#define STM32F7
//#define STM32G0
//#define STM32C0
//#define STM32U5
#define STM32U3

#if defined(STM32WB)
#include "stm32wbxx_hal.h"
#elif defined(STM32F0)
#include "stm32f0xx_hal.h"
#elif defined(STM32F1)
#include "stm32f1xx_hal.h"
#elif defined(STM32F4)
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#elif defined(STM32L0)
#include "stm32l0xx_hal.h"
#elif defined(STM32L1)
#include "stm32l1xx_hal.h"
#elif defined(STM32L4)
#include "stm32l4xx_hal.h"
#elif defined(STM32L5)
#include "stm32l5xx_hal.h"
#elif defined(STM32F3)
#include "stm32f3xx_hal.h"
#elif defined(STM32H7)
#include "stm32h7xx_hal.h"
#elif defined(STM32F7)
#include "stm32f7xx_hal.h"
#elif defined(STM32G0)
#include "stm32g0xx_hal.h"
#elif defined(STM32G4)
#include "stm32g4xx_hal.h"
#elif defined(STM32C0)
#include "stm32c0xx_hal.h"
#elif defined(STM32U5)
#include "stm32u5xx_hal.h"
#elif defined(STM32U3)
#include "stm32u3xx_hal.h"
#include "stm32u3xx_hal_i2c.h"
#else
#error "SSD1306 library was tested only on STM32F0, STM32F1, STM32F3, STM32F4, STM32F7, STM32L0, STM32L1, STM32L4, STM32H7, STM32G0, STM32G4, STM32WB, STM32C0, STM32U5 MCU families. Please modify ssd1306.h if you know what you are doing. Also please send a pull request if it turns out the library works on other MCU's as well!"
#endif

/* I2C config */

#define AH4931Q_I2C_PORT &hi2c2
extern I2C_HandleTypeDef hi2c2;

#ifndef AH4931Q_ADDR
#define AH4931Q_ADDR (0x5E << 1)  // The lsb after shift is for r/w and is set by HAL in the tx/rx functions
#endif

typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;
} AH4931Q_MagData;

typedef enum
{
	GEAR_PARK,
	GEAR_REVERSE,
	GEAR_NEUTRAL,
	GEAR_DRIVE,
	GEAR_INVALID
}Gear_t;

typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;
    Gear_t gear;
    uint8_t sensor_ok;
	uint8_t can_ok;
	uint32_t fault_flags;
} SystemState_t;

HAL_StatusTypeDef AH4931Q_Init(I2C_HandleTypeDef *hi2c2);
int16_t sign_extend_12(uint16_t value);
HAL_StatusTypeDef ReadHallSensor(SystemState_t *system_state);

#endif
