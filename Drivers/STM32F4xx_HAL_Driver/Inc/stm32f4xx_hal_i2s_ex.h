/**
  ******************************************************************************
  * @file    stm32f4xx_hal_i2s_ex.h
  * @author  MCD Application Team
  * @brief   Header file of I2S HAL module.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#ifndef STM32F4xx_HAL_I2S_EX_H
#define STM32F4xx_HAL_I2S_EX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal_def.h"

#if defined(SPI_I2S_FULLDUPLEX_SUPPORT)

#define I2SxEXT(__INSTANCE__) ((__INSTANCE__) == (SPI2)? (SPI_TypeDef *)(I2S2ext_BASE): (SPI_TypeDef *)(I2S3ext_BASE))
#define __HAL_I2SEXT_ENABLE(__HANDLE__) (I2SxEXT((__HANDLE__)->Instance)->I2SCFGR |= SPI_I2SCFGR_I2SE)
#define __HAL_I2SEXT_DISABLE(__HANDLE__) (I2SxEXT((__HANDLE__)->Instance)->I2SCFGR &= ~SPI_I2SCFGR_I2SE)
#define __HAL_I2SEXT_ENABLE_IT(__HANDLE__, __INTERRUPT__) (I2SxEXT((__HANDLE__)->Instance)->CR2 |= (__INTERRUPT__))
#define __HAL_I2SEXT_DISABLE_IT(__HANDLE__, __INTERRUPT__) (I2SxEXT((__HANDLE__)->Instance)->CR2 &= ~(__INTERRUPT__))
#define __HAL_I2SEXT_GET_IT_SOURCE(__HANDLE__, __INTERRUPT__) (((I2SxEXT((__HANDLE__)->Instance)->CR2 & (__INTERRUPT__)) == (__INTERRUPT__)) ? SET : RESET)
#define __HAL_I2SEXT_GET_FLAG(__HANDLE__, __FLAG__) (((I2SxEXT((__HANDLE__)->Instance)->SR) & (__FLAG__)) == (__FLAG__))
#define __HAL_I2SEXT_CLEAR_OVRFLAG(__HANDLE__) do{ __IO uint32_t tmpreg_ovr = 0x00U; tmpreg_ovr = I2SxEXT((__HANDLE__)->Instance)->DR; tmpreg_ovr = I2SxEXT((__HANDLE__)->Instance)->SR; UNUSED(tmpreg_ovr); }while(0U)
#define __HAL_I2SEXT_CLEAR_UDRFLAG(__HANDLE__) do{ __IO uint32_t tmpreg_udr = 0x00U; tmpreg_udr = I2SxEXT((__HANDLE__)->Instance)->SR; UNUSED(tmpreg_udr); }while(0U)
#define __HAL_I2SEXT_FLUSH_RX_DR(__HANDLE__) do{ __IO uint32_t tmpreg_dr = 0x00U; tmpreg_dr = I2SxEXT((__HANDLE__)->Instance)->DR; tmpreg_dr = ((__HANDLE__)->Instance->DR); UNUSED(tmpreg_dr); }while(0U)

HAL_StatusTypeDef HAL_I2SEx_TransmitReceive(I2S_HandleTypeDef *hi2s, uint16_t *pTxData, uint16_t *pRxData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef HAL_I2SEx_TransmitReceive_IT(I2S_HandleTypeDef *hi2s, uint16_t *pTxData, uint16_t *pRxData, uint16_t Size);
HAL_StatusTypeDef HAL_I2SEx_TransmitReceive_DMA(I2S_HandleTypeDef *hi2s, uint16_t *pTxData, uint16_t *pRxData, uint16_t Size);
void HAL_I2SEx_FullDuplex_IRQHandler(I2S_HandleTypeDef *hi2s);
void HAL_I2SEx_TxRxHalfCpltCallback(I2S_HandleTypeDef *hi2s);
void HAL_I2SEx_TxRxCpltCallback(I2S_HandleTypeDef *hi2s);

#endif /* SPI_I2S_FULLDUPLEX_SUPPORT */

#ifdef __cplusplus
}
#endif

#endif /* STM32F4xx_HAL_I2S_EX_H */
