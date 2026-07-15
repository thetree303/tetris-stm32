#include "cs43l22.h"

#define CS43L22_I2C_TIMEOUT  100

static CS43L22_StatusTypeDef CS43L22_WriteReg(I2C_HandleTypeDef *hi2c, uint16_t DeviceAddr, uint8_t Reg, uint8_t Value)
{
  return (HAL_I2C_Mem_Write(hi2c, DeviceAddr, Reg, I2C_MEMADD_SIZE_8BIT, &Value, 1, CS43L22_I2C_TIMEOUT) == HAL_OK) ? CS43L22_OK : CS43L22_ERROR;
}

static CS43L22_StatusTypeDef CS43L22_ReadReg(I2C_HandleTypeDef *hi2c, uint16_t DeviceAddr, uint8_t Reg, uint8_t *Value)
{
  return (HAL_I2C_Mem_Read(hi2c, DeviceAddr, Reg, I2C_MEMADD_SIZE_8BIT, Value, 1, CS43L22_I2C_TIMEOUT) == HAL_OK) ? CS43L22_OK : CS43L22_ERROR;
}

CS43L22_StatusTypeDef CS43L22_Init(I2C_HandleTypeDef *hi2c, uint16_t DeviceAddr, uint32_t AudioFreq, uint8_t BitResolution)
{
  uint8_t device_id = 0;
  uint8_t reg_val = 0;

  (void)AudioFreq;
  (void)BitResolution;

  /* Read device ID (must be 0xE0) */
  if (CS43L22_ReadReg(hi2c, DeviceAddr, CS43L22_REG_ID, &device_id) != CS43L22_OK)
    return CS43L22_ERROR;

  if (device_id != 0xE0)
    return CS43L22_ERROR;

  /* Power down before configuration */
  CS43L22_WriteReg(hi2c, DeviceAddr, CS43L22_REG_POWER_CTL1, CS43L22_POWER_CTL1_PDN);
  HAL_Delay(10);

  /* Power Control 1: enable HP, Speaker, DAC */
  reg_val = CS43L22_POWER_CTL1_HP_EN | CS43L22_POWER_CTL1_SPK_EN | CS43L22_POWER_CTL1_DAC_EN;
  CS43L22_WriteReg(hi2c, DeviceAddr, CS43L22_REG_POWER_CTL1, reg_val);

  /* Clocking: Auto-detect clock ratio */
  CS43L22_WriteReg(hi2c, DeviceAddr, CS43L22_REG_CLOCKING_CTL, CS43L22_CLOCKING_CTL_AUTO);

  /* Interface: I2S, 16-bit, Slave mode */
  reg_val = CS43L22_INTERFACE_CTL1_I2S | CS43L22_INTERFACE_CTL1_16BIT | CS43L22_INTERFACE_CTL1_SLAVE;
  CS43L22_WriteReg(hi2c, DeviceAddr, CS43L22_REG_INTERFACE_CTL1, reg_val);

  /* Master Volume A/B: 0 dB (0x00) */
  CS43L22_WriteReg(hi2c, DeviceAddr, CS43L22_REG_MASTER_VOL_A, 0x00);
  CS43L22_WriteReg(hi2c, DeviceAddr, CS43L22_REG_MASTER_VOL_B, 0x00);

  /* Speaker Volume: ~50% */
  CS43L22_WriteReg(hi2c, DeviceAddr, CS43L22_REG_SPEAKER_VOL, 0x00);

  /* Charge Pump: auto */
  CS43L22_WriteReg(hi2c, DeviceAddr, CS43L22_REG_CHARGE_PUMP, 0x99);

  HAL_Delay(10);

  /* Release power down */
  CS43L22_WriteReg(hi2c, DeviceAddr, CS43L22_REG_POWER_CTL1, 0x00);
  HAL_Delay(10);

  return CS43L22_OK;
}
