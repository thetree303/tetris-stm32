#include "cs43l22.h"

#define CS43L22_I2C_TIMEOUT  1000

static CS43L22_StatusTypeDef CS43L22_WriteReg(I2C_HandleTypeDef *hi2c, uint16_t DeviceAddr, uint8_t Reg, uint8_t Value)
{
  HAL_StatusTypeDef status = HAL_I2C_Mem_Write(hi2c, DeviceAddr, Reg, I2C_MEMADD_SIZE_8BIT, &Value, 1, CS43L22_I2C_TIMEOUT);
  return (status == HAL_OK) ? CS43L22_OK : CS43L22_ERROR;
}

static CS43L22_StatusTypeDef CS43L22_ReadReg(I2C_HandleTypeDef *hi2c, uint16_t DeviceAddr, uint8_t Reg, uint8_t *Value)
{
  HAL_StatusTypeDef status = HAL_I2C_Mem_Read(hi2c, DeviceAddr, Reg, I2C_MEMADD_SIZE_8BIT, Value, 1, CS43L22_I2C_TIMEOUT);
  return (status == HAL_OK) ? CS43L22_OK : CS43L22_ERROR;
}

CS43L22_StatusTypeDef CS43L22_Init(I2C_HandleTypeDef *hi2c, uint16_t DeviceAddr, uint32_t AudioFreq, uint8_t BitResolution)
{
  uint8_t device_id = 0;
  uint8_t reg_val = 0;

  if (CS43L22_ReadReg(hi2c, DeviceAddr, CS43L22_REG_ID, &device_id) != CS43L22_OK)
    return CS43L22_ERROR;

  if (device_id != CS43L22_ID)
    return CS43L22_ERROR;

  /* Power Down */
  reg_val = CS43L22_POWER_CTL1_PDN;
  if (CS43L22_WriteReg(hi2c, DeviceAddr, CS43L22_REG_POWER_CTL1, reg_val) != CS43L22_OK)
    return CS43L22_ERROR;

  HAL_Delay(10);

  /* Power Control 1: Enable Headphone, Speaker, DAC */
  reg_val = CS43L22_POWER_CTL1_HP | CS43L22_POWER_CTL1_SPEAKER | CS43L22_POWER_CTL1_DAC;
  if (CS43L22_WriteReg(hi2c, DeviceAddr, CS43L22_REG_POWER_CTL1, reg_val) != CS43L22_OK)
    return CS43L22_ERROR;

  /* Clocking Control: Auto-detect, MCLK from I2S */
  reg_val = CS43L22_CLOCKING_CTL_AUTO | CS43L22_CLOCKING_CTL_MCLK;
  if (CS43L22_WriteReg(hi2c, DeviceAddr, CS43L22_REG_CLOCKING_CTL, reg_val) != CS43L22_OK)
    return CS43L22_ERROR;

  /* Interface Control 1: I2S, 16-bit, Slave */
  reg_val = CS43L22_INTERFACE_CTL1_I2S | CS43L22_INTERFACE_CTL1_16B | CS43L22_INTERFACE_CTL1_SLAVE;
  if (CS43L22_WriteReg(hi2c, DeviceAddr, CS43L22_REG_INTERFACE_CTL1, reg_val) != CS43L22_OK)
    return CS43L22_ERROR;

  /* Playback Control 1: Enable Speaker and Headphone */
  reg_val = CS43L22_PLAYBACK_CTL1_SPK | CS43L22_PLAYBACK_CTL1_HP;
  if (CS43L22_WriteReg(hi2c, DeviceAddr, CS43L22_REG_PLAYBACK_CTL1, reg_val) != CS43L22_OK)
    return CS43L22_ERROR;

  /* Misc Control: Soft ramp, no de-emphasis */
  reg_val = CS43L22_MISC_CTL_DAC_SOFT | CS43L22_MISC_CTL_DAC_RAMP;
  if (CS43L22_WriteReg(hi2c, DeviceAddr, CS43L22_REG_MISC_CTL, reg_val) != CS43L22_OK)
    return CS43L22_ERROR;

  /* Master Volume: ~50% */
  reg_val = 0x30;
  if (CS43L22_WriteReg(hi2c, DeviceAddr, CS43L22_REG_MASTER_VOL_A, reg_val) != CS43L22_OK)
    return CS43L22_ERROR;
  if (CS43L22_WriteReg(hi2c, DeviceAddr, CS43L22_REG_MASTER_VOL_B, reg_val) != CS43L22_OK)
    return CS43L22_ERROR;

  /* Speaker Volume: ~50% */
  reg_val = 0x30;
  if (CS43L22_WriteReg(hi2c, DeviceAddr, CS43L22_REG_SPEAKER_VOL, reg_val) != CS43L22_OK)
    return CS43L22_ERROR;

  /* Charge Pump Frequency */
  reg_val = 0x0C;
  if (CS43L22_WriteReg(hi2c, DeviceAddr, CS43L22_REG_CHARGE_PUMP_FREQ, reg_val) != CS43L22_OK)
    return CS43L22_ERROR;

  /* Beep Config: Disable */
  reg_val = 0x00;
  if (CS43L22_WriteReg(hi2c, DeviceAddr, CS43L22_REG_BEEP_CTL, reg_val) != CS43L22_OK)
    return CS43L22_ERROR;

  /* Thermal Control */
  reg_val = 0x00;
  if (CS43L22_WriteReg(hi2c, DeviceAddr, CS43L22_REG_THERMAL_CTL, reg_val) != CS43L22_OK)
    return CS43L22_ERROR;

  HAL_Delay(10);

  return CS43L22_OK;
}

CS43L22_StatusTypeDef CS43L22_DeInit(I2C_HandleTypeDef *hi2c)
{
  uint8_t reg_val = CS43L22_POWER_CTL1_PDN;
  return CS43L22_WriteReg(hi2c, CS43L22_ADDR, CS43L22_REG_POWER_CTL1, reg_val);
}

CS43L22_StatusTypeDef CS43L22_ReadID(I2C_HandleTypeDef *hi2c, uint16_t DeviceAddr, uint8_t *DeviceID)
{
  return CS43L22_ReadReg(hi2c, DeviceAddr, CS43L22_REG_ID, DeviceID);
}

CS43L22_StatusTypeDef CS43L22_Play(I2C_HandleTypeDef *hi2c)
{
  uint8_t reg_val = 0;
  if (CS43L22_ReadReg(hi2c, CS43L22_ADDR, CS43L22_REG_POWER_CTL1, &reg_val) != CS43L22_OK)
    return CS43L22_ERROR;

  reg_val &= ~CS43L22_POWER_CTL1_PDN;
  return CS43L22_WriteReg(hi2c, CS43L22_ADDR, CS43L22_REG_POWER_CTL1, reg_val);
}

CS43L22_StatusTypeDef CS43L22_Pause(I2C_HandleTypeDef *hi2c)
{
  uint8_t reg_val = 0;
  if (CS43L22_ReadReg(hi2c, CS43L22_ADDR, CS43L22_REG_POWER_CTL1, &reg_val) != CS43L22_OK)
    return CS43L22_ERROR;

  reg_val |= CS43L22_POWER_CTL1_PDN;
  return CS43L22_WriteReg(hi2c, CS43L22_ADDR, CS43L22_REG_POWER_CTL1, reg_val);
}

CS43L22_StatusTypeDef CS43L22_Stop(I2C_HandleTypeDef *hi2c, uint32_t Cmd)
{
  return CS43L22_Pause(hi2c);
}

CS43L22_StatusTypeDef CS43L22_SetVolume(I2C_HandleTypeDef *hi2c, uint8_t Volume)
{
  if (Volume > 0xFF) Volume = 0xFF;
  if (CS43L22_WriteReg(hi2c, CS43L22_ADDR, CS43L22_REG_MASTER_VOL_A, Volume) != CS43L22_OK)
    return CS43L22_ERROR;
  if (CS43L22_WriteReg(hi2c, CS43L22_ADDR, CS43L22_REG_MASTER_VOL_B, Volume) != CS43L22_OK)
    return CS43L22_ERROR;
  if (CS43L22_WriteReg(hi2c, CS43L22_ADDR, CS43L22_REG_SPEAKER_VOL, Volume) != CS43L22_OK)
    return CS43L22_ERROR;
  return CS43L22_OK;
}

CS43L22_StatusTypeDef CS43L22_SetMute(I2C_HandleTypeDef *hi2c, uint32_t Cmd)
{
  uint8_t reg_val = 0;
  if (CS43L22_ReadReg(hi2c, CS43L22_ADDR, CS43L22_REG_PLAYBACK_CTL1, &reg_val) != CS43L22_OK)
    return CS43L22_ERROR;

  if (Cmd != 0)
    reg_val |= CS43L22_PLAYBACK_CTL1_MUTE;
  else
    reg_val &= ~CS43L22_PLAYBACK_CTL1_MUTE;

  return CS43L22_WriteReg(hi2c, CS43L22_ADDR, CS43L22_REG_PLAYBACK_CTL1, reg_val);
}

CS43L22_StatusTypeDef CS43L22_SetFrequency(I2C_HandleTypeDef *hi2c, uint32_t AudioFreq)
{
  (void)hi2c;
  (void)AudioFreq;
  return CS43L22_OK;
}

CS43L22_StatusTypeDef CS43L22_Reset(I2C_HandleTypeDef *hi2c)
{
  return CS43L22_DeInit(hi2c);
}