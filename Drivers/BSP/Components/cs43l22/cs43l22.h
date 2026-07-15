#ifndef CS43L22_H
#define CS43L22_H

#include <stdint.h>
#include "stm32f4xx_hal.h"

#define CS43L22_OK                0
#define CS43L22_ERROR            -1

#define CS43L22_ADDR              0x4A
#define CS43L22_ID                0xE0

#define CS43L22_REG_ID                    0x01
#define CS43L22_REG_POWER_CTL1            0x02
#define CS43L22_REG_POWER_CTL2            0x03
#define CS43L22_REG_CLOCKING_CTL          0x04
#define CS43L22_REG_INTERFACE_CTL1        0x05
#define CS43L22_REG_INTERFACE_CTL2        0x06
#define CS43L22_REG_PASSTHROUGH_A         0x07
#define CS43L22_REG_PASSTHROUGH_B         0x08
#define CS43L22_REG_ANALOG_ZC_SR          0x09
#define CS43L22_REG_PASSTHROUGH_G         0x0A
#define CS43L22_REG_PLAYBACK_CTL1         0x0B
#define CS43L22_REG_MISC_CTL              0x0C
#define CS43L22_REG_PLAYBACK_CTL2         0x0D
#define CS43L22_REG_PASSTHROUGH_CTL       0x0E

#define CS43L22_POWER_CTL1_PDN            0x01
#define CS43L22_POWER_CTL1_SPK_EN         0x02
#define CS43L22_POWER_CTL1_HP_EN          0x04
#define CS43L22_POWER_CTL1_DAC_EN         0x08

#define CS43L22_INTERFACE_CTL1_I2S        0x04
#define CS43L22_INTERFACE_CTL1_16BIT      0x00
#define CS43L22_INTERFACE_CTL1_20BIT      0x08
#define CS43L22_INTERFACE_CTL1_24BIT      0x10
#define CS43L22_INTERFACE_CTL1_32BIT      0x18

#define CS43L22_CLOCKING_CTL_AUTO         0x80
#define CS43L22_CLOCKING_CTL_MCLK_DIV2    0x40

#define CS43L22_MISC_CTL_DIGITAL_SOFT_RST 0x80

typedef enum {
  CS43L22_OUTPUT_DEVICE_SPEAKER = 1,
  CS43L22_OUTPUT_DEVICE_HEADPHONE = 2,
  CS43L22_OUTPUT_DEVICE_BOTH = 3
} CS43L22_OutputDevice;

typedef enum {
  CS43L22_FREQUENCY_8K  = 8000,
  CS43L22_FREQUENCY_16K = 16000,
  CS43L22_FREQUENCY_22K = 22050,
  CS43L22_FREQUENCY_44K = 44100,
  CS43L22_FREQUENCY_48K = 48000
} CS43L22_AudioFreq;

typedef int32_t CS43L22_StatusTypeDef;

int32_t CS43L22_Init(I2C_HandleTypeDef *hi2c, uint16_t DeviceAddr, uint32_t AudioFreq, uint8_t BitResolution);
int32_t CS43L22_DeInit(I2C_HandleTypeDef *hi2c);
int32_t CS43L22_ReadID(I2C_HandleTypeDef *hi2c, uint16_t DeviceAddr);
int32_t CS43L22_Play(I2C_HandleTypeDef *hi2c, uint16_t DeviceAddr);
int32_t CS43L22_Pause(I2C_HandleTypeDef *hi2c, uint16_t DeviceAddr);
int32_t CS43L22_Stop(I2C_HandleTypeDef *hi2c, uint16_t DeviceAddr, uint8_t CodecPdwnMode);
int32_t CS43L22_SetVolume(I2C_HandleTypeDef *hi2c, uint16_t DeviceAddr, uint8_t Volume);
int32_t CS43L22_SetFrequency(I2C_HandleTypeDef *hi2c, uint32_t AudioFreq);
int32_t CS43L22_SetOutputMode(I2C_HandleTypeDef *hi2c, uint8_t OutputDevice);
int32_t CS43L22_Reset(I2C_HandleTypeDef *hi2c);

#endif