/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  	START
	  ↓
	sensor address + WRITE
	  ↓
	register address
	  ↓
	REPEATED START
	  ↓
	sensor address + READ
	  ↓
	data bytes
	  ↓
	 STOP

  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <ssd1306.h>
#include <ah4931q.h>
#include <ssd1306_fonts.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TX_ID          (0x111)   /* TX CAN message identifier    */
#define RX_ID          (0x111)   /* RX CAN message identifier    */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#ifndef COUNTOF
#define COUNTOF(BUFFER) (sizeof((BUFFER)) / sizeof(*(BUFFER)))
#endif
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

FDCAN_HandleTypeDef hfdcan1;

I2C_HandleTypeDef hi2c2;

/* USER CODE BEGIN PV */

#define SENSOR_PERIOD_MS   5
#define DISPLAY_PERIOD_MS  25
#define CAN_PERIOD_MS      200

FDCAN_RxHeaderTypeDef rxHeader;
FDCAN_TxHeaderTypeDef txHeader;
uint8_t rxData[16U];

/*
    Byte 0: gear enum + validity bits
	Byte 1: rolling counter
	Byte 2: checksum / CRC
	Bytes 3-7: reserved or other status
*/
static uint8_t txData[] = { 0x00, 0x00, 0x54, 0x76, 0x98, 0x00, 0x11, 0x22 };

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C2_Init(void);
static void MX_FDCAN1_Init(void);
static void MX_ICACHE_Init(void);
/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
//static uint16_t Buffercmp(uint8_t *pBuffer1, uint8_t *pBuffer2, uint16_t BufferLength);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int _write(int file, char *ptr, int len)
{
    for (int i = 0; i < len; i++)
    {
    	ITM_SendChar(*ptr++);
    }
    return len;
}

void printI2Cdevices(void)
{
	for (uint8_t addr = 1; addr < 128; addr++)
	{
	    if (HAL_I2C_IsDeviceReady(&hi2c2, addr << 1, 1, 10) == HAL_OK)
	    {
	    	printf("I2C device found at 0x%02X\r\n", addr);
	    }
	}
	return;
}

typedef enum
{
    SCREEN_RAW,
    SCREEN_POSITION,
    SCREEN_STATUS
} ScreenMode_t;

void UpdateGearPosition(SystemState_t *system_state)
{
	system_state -> gear = GEAR_DRIVE; //(3)GEAR_DRIVE
}

static void DrawRawScreen(const SystemState_t *state)
{
    char buf[24];

    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("RAW", Font_11x18, White);

    snprintf(buf, sizeof(buf), "X:%+5d", state->x);
    ssd1306_SetCursor(0, 22);
    ssd1306_WriteString(buf, Font_7x10, White);

    snprintf(buf, sizeof(buf), "Y:%+5d", state->y);
    ssd1306_SetCursor(0, 34);
    ssd1306_WriteString(buf, Font_7x10, White);

    snprintf(buf, sizeof(buf), "Z:%+5d", state->z);
    ssd1306_SetCursor(0, 46);
    ssd1306_WriteString(buf, Font_7x10, White);
}

static void DrawPositionScreen(const SystemState_t *state)
{
    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("POSITION", Font_7x10, White);

    switch (state->gear)
    {
        case GEAR_PARK:
            ssd1306_SetCursor(55, 25);
            ssd1306_WriteString("P", Font_16x26, White);
            break;

        case GEAR_REVERSE:
        	ssd1306_SetCursor(55, 25);
			ssd1306_WriteString("R", Font_16x26, White);
            break;

        case GEAR_NEUTRAL:
        	ssd1306_SetCursor(55, 25);
			ssd1306_WriteString("N", Font_16x26, White);
            break;

        case GEAR_DRIVE:
        	ssd1306_SetCursor(55, 25);
			ssd1306_WriteString("D", Font_16x26, White);
            break;

        case GEAR_INVALID:
			ssd1306_SetCursor(45, 25);
			ssd1306_WriteString("X", Font_16x26, White);
			break;

        default:
            ssd1306_SetCursor(45, 25);
            ssd1306_WriteString("?", Font_16x26, White);
            break;
    }
}

static void DrawStatusScreen(const SystemState_t *state)
{
    char buf[24];

    ssd1306_SetCursor(0, 0);
    ssd1306_WriteString("STATUS", Font_7x10, White);

    snprintf(buf, sizeof(buf),
             "SNS: %s",
             state->sensor_ok ? "OK" : "ERR");

    ssd1306_SetCursor(0, 16);
    ssd1306_WriteString(buf, Font_7x10, White);

    snprintf(buf, sizeof(buf),
             "CAN: %s",
             state->can_ok ? "OK" : "ERR");

    ssd1306_SetCursor(0, 28);
    ssd1306_WriteString(buf, Font_7x10, White);
}

static void DrawErrorScreen()
{
	char string[] = "ERROR";
    ssd1306_SetCursor(0, 16);
    ssd1306_WriteString(string, Font_7x10, White);
}

void UpdateDisplay(const SystemState_t *state, ScreenMode_t mode)
{
    ssd1306_Fill(Black);

    switch (mode)
    {
        case SCREEN_RAW:
            DrawRawScreen(state);
            break;

        case SCREEN_POSITION:
            DrawPositionScreen(state);
            break;

        case SCREEN_STATUS:
            DrawStatusScreen(state);
            break;

        default:
            DrawErrorScreen();
            break;
    }
    ssd1306_UpdateScreen();
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C2_Init();
  MX_FDCAN1_Init();
  MX_ICACHE_Init();
  /* USER CODE BEGIN 2 */
	/* Configure LD2 */
	BSP_LED_Init(LD2);
	BSP_PB_Init(BUTTON_USER,BUTTON_MODE_GPIO); 				/* Configure User push-button */

	MX_GPIO_Init();

	while (HAL_I2C_GetState(&hi2c2) != HAL_I2C_STATE_READY) {}

//  uint32_t clock = HAL_RCC_GetHCLKFreq();

	/*
	int16_t x;
    int16_t y;
    int16_t z;
    Gear_t gear; enum: (0)GEAR_PARK, (1)GEAR_REVERSE, (2)GEAR_NEUTRAL, (3)GEAR_DRIVE, (4)GEAR_INVALID
    uint8_t sensor_ok;
	uint8_t can_ok;
	uint32_t fault_flags;
	 */
	SystemState_t system_state = {0};
	ScreenMode_t  screen_mode  = SCREEN_RAW; //(0)SCREEN_RAW, (1)SCREEN_POSITION, (2)SCREEN_STATUS
	printI2Cdevices();
	ssd1306_Init();
	if (AH4931Q_Init(&hi2c2)) { Error_Handler();}  // returns HAL status
	HAL_Delay(100);
	// Press button to initialize AH4931Q I2C peripheral
	printf("Screen and sensor initialized. Press user button to start sampling\r\n");
	while (BSP_PB_GetState(BUTTON_USER) != GPIO_PIN_SET  ) {}
	while (BSP_PB_GetState(BUTTON_USER) != GPIO_PIN_RESET) {}
	uint32_t last_sensor  = HAL_GetTick();
	uint32_t last_display = HAL_GetTick();
	uint32_t last_can     = HAL_GetTick();
	uint32_t now 		  = HAL_GetTick();
//	uint32_t last_can = HAL_GetTick();

	/* Configure reception filter to Rx FIFO 0 */
	  FDCAN_FilterTypeDef        sFilterConfig;
	  sFilterConfig.IdType       = FDCAN_STANDARD_ID;
	  sFilterConfig.FilterIndex  = 0U;
	  sFilterConfig.FilterType   = FDCAN_FILTER_MASK;
	  sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
	  sFilterConfig.FilterID1    = TX_ID;
	  sFilterConfig.FilterID2    = 0x7FF;
	  if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig) != HAL_OK) { Error_Handler(); }

	  /**
	    *  Configure global filter:
	    *    - Filter all remote frames with STD and EXT ID
	    *    - Reject non matching frames with STD ID and EXT ID
	    */
	  if (HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_REJECT, FDCAN_REJECT, FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE) != HAL_OK) { Error_Handler(); }

	  /* Activate Rx FIFO 0 new message notification */
	  if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0U) != HAL_OK) { Error_Handler(); }

	  /* Prepare Tx message Header */
	  txHeader.Identifier          = TX_ID;
	  txHeader.IdType              = FDCAN_STANDARD_ID;
	  txHeader.TxFrameType         = FDCAN_DATA_FRAME;
	  txHeader.DataLength          = FDCAN_DLC_BYTES_8;
	  txHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	  txHeader.BitRateSwitch       = FDCAN_BRS_OFF;
	  txHeader.FDFormat            = FDCAN_CLASSIC_CAN;
	  txHeader.TxEventFifoControl  = FDCAN_NO_TX_EVENTS;
	  txHeader.MessageMarker       = 0U;

	  if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK) { Error_Handler(); } /* Start FDCAN controller */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

	while(1) ///////////////////////////////////////////////////////////////////////////////////////////
	{
		now = HAL_GetTick();
		if ((now - last_sensor) >= SENSOR_PERIOD_MS)
		{
			if(ReadHallSensor(&system_state) != HAL_OK) { Error_Handler();}
			UpdateGearPosition(&system_state);
			last_sensor = HAL_GetTick();
		}

	    if ((now - last_can) >= CAN_PERIOD_MS)
	    {
			/* 	txData:
			    Byte 0: gear enum + validity bits
				Byte 1: rolling counter
				Byte 2: checksum / CRC
				Bytes 3-7: reserved or other status
			*/
	    	txData[1]++;
	    	txData[1] &= 0x0F;
	    	HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &txHeader, txData); //  SendCANStatus(&system_state);
	        last_can = now;
	    }

		if ((now - last_display) >= DISPLAY_PERIOD_MS)
		{
//			printf("display start\r\n");	//DEBUG
			UpdateDisplay(&system_state, screen_mode);
			last_display = HAL_GetTick();
//			printf("display done\r\n");		//DEBUG
		}
	}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the System Power Supply
  */
  if (HAL_PWREx_ConfigSupply(PWR_SMPS_SUPPLY) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSIS;
  RCC_OscInitStruct.MSISState = RCC_MSI_ON;
  RCC_OscInitStruct.MSISSource = RCC_MSI_RC0;
  RCC_OscInitStruct.MSISDiv = RCC_MSI_DIV8;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_PCLK3;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSIS;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief FDCAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_FDCAN1_Init(void)
{

  /* USER CODE BEGIN FDCAN1_Init 0 */

  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */

  /* USER CODE END FDCAN1_Init 1 */
  hfdcan1.Instance = FDCAN1;
  hfdcan1.Init.ClockDivider = FDCAN_CLOCK_DIV1;
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan1.Init.AutoRetransmission = DISABLE;
  hfdcan1.Init.TransmitPause = ENABLE;
  hfdcan1.Init.ProtocolException = DISABLE;
  hfdcan1.Init.NominalPrescaler = 1;
  hfdcan1.Init.NominalSyncJumpWidth = 30;
  hfdcan1.Init.NominalTimeSeg1 = 90;
  hfdcan1.Init.NominalTimeSeg2 = 30;
  hfdcan1.Init.DataPrescaler = 1;
  hfdcan1.Init.DataSyncJumpWidth = 3;
  hfdcan1.Init.DataTimeSeg1 = 8;
  hfdcan1.Init.DataTimeSeg2 = 3;
  hfdcan1.Init.StdFiltersNbr = 1;
  hfdcan1.Init.ExtFiltersNbr = 0;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */

  /* USER CODE END FDCAN1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */
  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */
  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x40000A0B;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */
  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief ICACHE Initialization Function
  * @param None
  * @retval None
  */
static void MX_ICACHE_Init(void)
{

  /* USER CODE BEGIN ICACHE_Init 0 */

  /* USER CODE END ICACHE_Init 0 */

  /* USER CODE BEGIN ICACHE_Init 1 */

  /* USER CODE END ICACHE_Init 1 */

  /** Enable instruction cache (default 2-ways set associative cache)
  */
  if (HAL_ICACHE_Enable() != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ICACHE_Init 2 */

  /* USER CODE END ICACHE_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
//static uint16_t Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength)
//{
//  while (BufferLength--) {
//    if ((*pBuffer1) != *pBuffer2) {
//    	return BufferLength;
//    }
//    pBuffer1++;
//    pBuffer2++;
//  }
//  return 0;
//}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while(1){}
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
