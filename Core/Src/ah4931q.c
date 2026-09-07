



#include "ah4931q.h"
#include <stdio.h>

/*
Additionally, several flags can be checked to ensure the data values are consistent and the ADC was not running at the time of read-out.
*/


// REGISTER DEFINITIONS

// Read Regs
	// Bx(0H) Bx[11:4] Bx MSB[11:4]
	// By(1H) By[11:4] By MSB[11:4]
	// Bz(2H) Bz[11:4] Bz MSB[11:4]
	// Temp(3H) Temp[11:8] FRM[1:0] CH[1:0] Temp MSB[11:8],  Frame number, CH number
	// Bx2y2(4H) Bx[3:0] By[3:0] Bx LSB[3:0], By LSB[3:0]
	// Bz2(5H) Rsvd PD Bz[3:0] PD, Bz LSB[3:0]
	// Temp2(6H) Temp[7:0] Temp LSB[7:0]
	// WU_XH(7H) WU_XH[10:3] WU_X high threshold[10:3]
	// WU_XL(8H) WU_XL[10:3] WU X low threshold[10:3]
	// WU_YH(9H) WU_YH[10:3] WU_Y high threshold[10:3]
	// WU_YL(AH) WU_YL[10:3] WU Y low threshold[10:3]
	// WU_ZH(BH) WU_ZH[10:3] WU_Z high threshold[10:3]
	// WU_ZL(CH) WU_ZL[10:3] WU Z low threshold[10:3]
	// WU_XLSB(DH) Rsvd XL[2:0] XH[2:0] WU X high/low threshold[2:0]
	// WU_YLSB(EH) Rsvd YL[2:0] YH[2:0] WU Y high/low threshold[2:0]
	// WU_ZLSB(FH) Rsvd ZL[2:0] ZH[2:0] WU Z high/low threshold[2:0]

// Write Regs
	//Rsvd(0H) Rsvd Reserved
	//MOD1(1H) Rsvd  IICAddr[1:0] WUE Rsvd INT FAST LOW IICAddr, Wake-up enable, INT, Continuous (FAST mode), LOW power mode
	//Rsvd(2H) Rsvd Reserved
	//MOD2(3H) Rsvd LP Rsvd CA Rsvd Low Power, Collision Avoidance
	//Rsvd(4H) Rsvd Reserved Rsvd(5H) Rsvd Reserved
	//Rsvd(6H) Rsvd Reserved
	//WU_XH(7H) WU_XH[10:3] WU_X high threshold[10:3]
	//WU_XL(8H) WU_XL[10:3] WU X low threshold[10:3]
	//WU_YH(9H) WU_YH[10:3] WU_Y high threshold[10:3]
	//WU_YL(AH) WU_YL[10:3] WU Y low threshold[10:3]
	//WU_ZH(BH) WU_ZH[10:3] WU_Z high threshold[10:3]
	//WU_ZL(CH) WU_ZL[10:3] WU Z low threshold[10:3]
	//WU_XLSB(DH) Rsvd XL[2:0] XH[2:0] WU X high/low threshold[2:0]
	//WU_YLSB(EH) Rsvd YL[2:0] YH[2:0] WU Y high/low threshold[2:0]
	//WU_ZLSB(FH) Rsvd ZL[2:0] ZH[2:0] WU Z high/low threshold[2:0]

HAL_StatusTypeDef AH4931Q_Init(I2C_HandleTypeDef *hi2c2)
{
	HAL_StatusTypeDef status;
	uint8_t rxData[7];
	uint8_t txData[7];
	txData[0] = 0b0;
	txData[1] = 0b00000001;
	txData[2] = 0b0;
	txData[3] = 0b01000000;
	// read regs default values
	status = HAL_I2C_Master_Receive(hi2c2, (0x5E << 1), rxData, 7, 1000 ); //(I2C_HandleTypeDef, uint16 Addr, uint8 *pData, uint16 Size_in_bytes, uint32 Timeout)
	printf("status = %d, error = 0x%08lX\r\n", status, HAL_I2C_GetError(hi2c2));
	if (status){return status;}
	printf("R0 = 0x%02X\r\n", rxData[0]);
	printf("R1 = 0x%02X\r\n", rxData[1]);
	printf("R2 = 0x%02X\r\n", rxData[2]);
	printf("R3 = 0x%02X\r\n", rxData[3]);
	printf("R4 = 0x%02X\r\n", rxData[4]);
	printf("R5 = 0x%02X\r\n", rxData[5]);
	printf("R6 = 0x%02X\r\n", rxData[6]);
	// Configure Sensor MOD1 & MOD2 register
	status = HAL_I2C_Master_Transmit(hi2c2, (0x5E << 1), txData, 4, 1000); //(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout)
	return status;
}


int16_t sign_extend_12(uint16_t value)
{
    value &= 0x0FFF;
    if (value & 0x0800) { value |= 0xF000; }
    return (int16_t)value;
}

HAL_StatusTypeDef ReadHallSensor(SystemState_t *system_state)
{
	HAL_StatusTypeDef status;
	uint8_t rxData[7];
	uint16_t bx_raw;
	uint16_t by_raw;
	uint16_t bz_raw;
	status = HAL_I2C_Master_Receive(AH4931Q_I2C_PORT, (0x5E << 1), rxData, 7, 1000 ); //(I2C_HandleTypeDef, uint16 Addr, uint8 *pData, uint16 Size_in_bytes, uint32 Timeout)
	if (status){printf("status = %d, error = 0x%08lX\r\n", status, HAL_I2C_GetError(&hi2c2));}
	if (status){return status;}
	bx_raw = ((uint16_t)rxData[0] << 4) | ((rxData[4] >> 4) & 0x0F);
	by_raw = ((uint16_t)rxData[1] << 4) | (rxData[4] & 0x0F);
	bz_raw = ((uint16_t)rxData[2] << 4) | (rxData[5] & 0x0F);
	system_state -> x = sign_extend_12(bx_raw);
	system_state -> y = sign_extend_12(by_raw);
	system_state -> z = sign_extend_12(bz_raw);
//    printf("Bx = %d, By = %d, Bz = %d\r\n", system_state -> x, system_state -> y, system_state -> z);
    return status;
}
