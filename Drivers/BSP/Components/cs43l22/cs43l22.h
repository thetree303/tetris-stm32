#ifndef CS43L22_H
#define CS43L22_H

#include "stm32f4xx_hal.h"

#define CS43L22_OK                   0
#define CS43L22_ERROR               -1

#define CS43L22_ADDR                 0x94  /* HAL 8-bit address (7-bit 0x4A << 1) */

/* CS43L22 Register Map */
#define CS43L22_REG_ID               0x01
#define CS43L22_REG_POWER_CTL1       0x02
#define CS43L22_REG_POWER_CTL2       0x03
#define CS43L22_REG_CLOCKING_CTL     0x04
#define CS43L22_REG_INTERFACE_CTL1   0x05
#define CS43L22_REG_INTERFACE_CTL2   0x06
#define CS43L22_REG_PLAYBACK_CTL1    0x0B
#define CS43L22_REG_MISC_CTL        0x0E
#define CS43L22_REG_PLAYBACK_CTL2    0x0F
#define CS43L22_REG_MASTER_VOL_A    0x20
#define CS43L22_REG_MASTER_VOL_B    0x21
#define CS43L22_REG_SPEAKER_VOL     0x24
#define CS43L22_REG_CHARGE_PUMP     0x32

/* Power Control 1 bits */
#define CS43L22_POWER_CTL1_PDN       0x80
#define CS43L22_POWER_CTL1_SPK_EN    0x20
#define CS43L22_POWER_CTL1_HP_EN     0x10
#define CS43L22_POWER_CTL1_DAC_EN    0x08

/* Interface Control 1 bits */
#define CS43L22_INTERFACE_CTL1_I2S    0x04
#define CS43L22_INTERFACE_CTL1_16BIT  0x00
#define CS43L22_INTERFACE_CTL1_SLAVE  0x02

/* Clocking Control bits */
#define CS43L22_CLOCKING_CTL_AUTO     0x80
#define CS43L22_CLOCKING_CTL_MCLK2   0x40

typedef int32_t CS43L22_StatusTypeDef;

CS43L22_StatusTypeDef CS43L22_Init(I2C_HandleTypeDef *hi2c, uint16_t DeviceAddr, uint32_t AudioFreq, uint8_t BitResolution);

#endif
