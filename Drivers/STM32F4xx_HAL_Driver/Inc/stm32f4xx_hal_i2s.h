/**
  ******************************************************************************
  * @file    stm32f4xx_hal_i2s.h
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32F4xx_HAL_I2S_H
#define STM32F4xx_HAL_I2S_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal_def.h"

/** @addtogroup STM32F4xx_HAL_Driver
  * @{
  */

/** @addtogroup I2S
  * @{
  */

/* Exported types ------------------------------------------------------------*/
/** @defgroup I2S_Exported_Types I2S Exported Types
  * @{
  */

/**
  * @brief I2S Init structure definition
  */
typedef struct
{
  uint32_t Mode;
  uint32_t Standard;
  uint32_t DataFormat;
  uint32_t MCLKOutput;
  uint32_t AudioFreq;
  uint32_t CPOL;
  uint32_t ClockSource;
  uint32_t FullDuplexMode;
} I2S_InitTypeDef;

typedef enum
{
  HAL_I2S_STATE_RESET      = 0x00U,
  HAL_I2S_STATE_READY      = 0x01U,
  HAL_I2S_STATE_BUSY       = 0x02U,
  HAL_I2S_STATE_BUSY_TX    = 0x03U,
  HAL_I2S_STATE_BUSY_RX    = 0x04U,
  HAL_I2S_STATE_BUSY_TX_RX = 0x05U,
  HAL_I2S_STATE_TIMEOUT    = 0x06U,
  HAL_I2S_STATE_ERROR      = 0x07U
} HAL_I2S_StateTypeDef;

typedef struct __I2S_HandleTypeDef
{
  SPI_TypeDef                *Instance;
  I2S_InitTypeDef            Init;
  uint16_t                   *pTxBuffPtr;
  __IO uint16_t              TxXferSize;
  __IO uint16_t              TxXferCount;
  uint16_t                   *pRxBuffPtr;
  __IO uint16_t              RxXferSize;
  __IO uint16_t              RxXferCount;
  void (*IrqHandlerISR)(struct __I2S_HandleTypeDef *hi2s);
  DMA_HandleTypeDef          *hdmatx;
  DMA_HandleTypeDef          *hdmarx;
  __IO HAL_LockTypeDef       Lock;
  __IO HAL_I2S_StateTypeDef  State;
  __IO uint32_t              ErrorCode;

#if (USE_HAL_I2S_REGISTER_CALLBACKS == 1U)
  void (* TxCpltCallback)(struct __I2S_HandleTypeDef *hi2s);
  void (* RxCpltCallback)(struct __I2S_HandleTypeDef *hi2s);
  void (* TxRxCpltCallback)(struct __I2S_HandleTypeDef *hi2s);
  void (* TxHalfCpltCallback)(struct __I2S_HandleTypeDef *hi2s);
  void (* RxHalfCpltCallback)(struct __I2S_HandleTypeDef *hi2s);
  void (* TxRxHalfCpltCallback)(struct __I2S_HandleTypeDef *hi2s);
  void (* ErrorCallback)(struct __I2S_HandleTypeDef *hi2s);
  void (* MspInitCallback)(struct __I2S_HandleTypeDef *hi2s);
  void (* MspDeInitCallback)(struct __I2S_HandleTypeDef *hi2s);
#endif
} I2S_HandleTypeDef;

#define HAL_I2S_ERROR_NONE               (0x00000000U)
#define HAL_I2S_ERROR_TIMEOUT            (0x00000001U)
#define HAL_I2S_ERROR_OVR                (0x00000002U)
#define HAL_I2S_ERROR_UDR                (0x00000004U)
#define HAL_I2S_ERROR_DMA                (0x00000008U)
#define HAL_I2S_ERROR_PRESCALER          (0x00000010U)
#define HAL_I2S_ERROR_INVALID_CALLBACK   (0x00000020U)
#define HAL_I2S_ERROR_BUSY_LINE_RX       (0x00000040U)

#define I2S_MODE_SLAVE_TX                (0x00000000U)
#define I2S_MODE_SLAVE_RX                (SPI_I2SCFGR_I2SCFG_0)
#define I2S_MODE_MASTER_TX               (SPI_I2SCFGR_I2SCFG_1)
#define I2S_MODE_MASTER_RX               ((SPI_I2SCFGR_I2SCFG_0 | SPI_I2SCFGR_I2SCFG_1))

#define I2S_STANDARD_PHILIPS             (0x00000000U)
#define I2S_STANDARD_MSB                 (SPI_I2SCFGR_I2SSTD_0)
#define I2S_STANDARD_LSB                 (SPI_I2SCFGR_I2SSTD_1)
#define I2S_STANDARD_PCM_SHORT           ((SPI_I2SCFGR_I2SSTD_0 | SPI_I2SCFGR_I2SSTD_1))
#define I2S_STANDARD_PCM_LONG            ((SPI_I2SCFGR_I2SSTD_0 | SPI_I2SCFGR_I2SSTD_1 | SPI_I2SCFGR_PCMSYNC))

#define I2S_DATAFORMAT_16B               (0x00000000U)
#define I2S_DATAFORMAT_16B_EXTENDED      (SPI_I2SCFGR_CHLEN)
#define I2S_DATAFORMAT_24B               ((SPI_I2SCFGR_CHLEN | SPI_I2SCFGR_DATLEN_0))
#define I2S_DATAFORMAT_32B               ((SPI_I2SCFGR_CHLEN | SPI_I2SCFGR_DATLEN_1))

#define I2S_MCLKOUTPUT_ENABLE            (SPI_I2SPR_MCKOE)
#define I2S_MCLKOUTPUT_DISABLE           (0x00000000U)

#define I2S_AUDIOFREQ_192K               (192000U)
#define I2S_AUDIOFREQ_96K                (96000U)
#define I2S_AUDIOFREQ_48K                (48000U)
#define I2S_AUDIOFREQ_44K                (44100U)
#define I2S_AUDIOFREQ_32K                (32000U)
#define I2S_AUDIOFREQ_22K                (22050U)
#define I2S_AUDIOFREQ_16K                (16000U)
#define I2S_AUDIOFREQ_11K                (11025U)
#define I2S_AUDIOFREQ_8K                 (8000U)
#define I2S_AUDIOFREQ_DEFAULT            (2U)

#define I2S_FULLDUPLEXMODE_DISABLE       (0x00000000U)
#define I2S_FULLDUPLEXMODE_ENABLE        (0x00000001U)

#define I2S_CPOL_LOW                     (0x00000000U)
#define I2S_CPOL_HIGH                    (SPI_I2SCFGR_CKPOL)

#define I2S_IT_TXE                       SPI_CR2_TXEIE
#define I2S_IT_RXNE                      SPI_CR2_RXNEIE
#define I2S_IT_ERR                       SPI_CR2_ERRIE

#define I2S_FLAG_TXE                     SPI_SR_TXE
#define I2S_FLAG_RXNE                    SPI_SR_RXNE
#define I2S_FLAG_UDR                     SPI_SR_UDR
#define I2S_FLAG_OVR                     SPI_SR_OVR
#define I2S_FLAG_FRE                     SPI_SR_FRE
#define I2S_FLAG_CHSIDE                  SPI_SR_CHSIDE
#define I2S_FLAG_BSY                     SPI_SR_BSY
#define I2S_FLAG_MASK                   (SPI_SR_RXNE | SPI_SR_TXE | SPI_SR_UDR | SPI_SR_OVR | SPI_SR_FRE | SPI_SR_CHSIDE | SPI_SR_BSY)

#define I2S_CLOCK_PLL                    (0x00000000U)
#define I2S_CLOCK_EXTERNAL               (0x00000001U)

#define __HAL_I2S_RESET_HANDLE_STATE(__HANDLE__) ((__HANDLE__)->State = HAL_I2S_STATE_RESET)
#define __HAL_I2S_ENABLE(__HANDLE__)    (SET_BIT((__HANDLE__)->Instance->I2SCFGR, SPI_I2SCFGR_I2SE))
#define __HAL_I2S_DISABLE(__HANDLE__) (CLEAR_BIT((__HANDLE__)->Instance->I2SCFGR, SPI_I2SCFGR_I2SE))
#define __HAL_I2S_ENABLE_IT(__HANDLE__, __INTERRUPT__)    (SET_BIT((__HANDLE__)->Instance->CR2,(__INTERRUPT__)))
#define __HAL_I2S_DISABLE_IT(__HANDLE__, __INTERRUPT__) (CLEAR_BIT((__HANDLE__)->Instance->CR2,(__INTERRUPT__)))
#define __HAL_I2S_GET_IT_SOURCE(__HANDLE__, __INTERRUPT__) ((((__HANDLE__)->Instance->CR2 & (__INTERRUPT__)) == (__INTERRUPT__)) ? SET : RESET)
#define __HAL_I2S_GET_FLAG(__HANDLE__, __FLAG__) ((((__HANDLE__)->Instance->SR) & (__FLAG__)) == (__FLAG__))
#define __HAL_I2S_CLEAR_OVRFLAG(__HANDLE__) do{ __IO uint32_t tmpreg_ovr = 0x00U; tmpreg_ovr = (__HANDLE__)->Instance->DR; tmpreg_ovr = (__HANDLE__)->Instance->SR; UNUSED(tmpreg_ovr); }while(0U)
#define __HAL_I2S_CLEAR_UDRFLAG(__HANDLE__) do{ __IO uint32_t tmpreg_udr = 0x00U; tmpreg_udr = ((__HANDLE__)->Instance->SR); UNUSED(tmpreg_udr); }while(0U)
#define __HAL_I2S_FLUSH_RX_DR(__HANDLE__)  do{ __IO uint32_t tmpreg_dr = 0x00U; tmpreg_dr = ((__HANDLE__)->Instance->DR); UNUSED(tmpreg_dr); }while(0U)

#include "stm32f4xx_hal_i2s_ex.h"

HAL_StatusTypeDef HAL_I2S_Init(I2S_HandleTypeDef *hi2s);
HAL_StatusTypeDef HAL_I2S_DeInit(I2S_HandleTypeDef *hi2s);
void HAL_I2S_MspInit(I2S_HandleTypeDef *hi2s);
void HAL_I2S_MspDeInit(I2S_HandleTypeDef *hi2s);

HAL_StatusTypeDef HAL_I2S_Transmit(I2S_HandleTypeDef *hi2s, uint16_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef HAL_I2S_Receive(I2S_HandleTypeDef *hi2s, uint16_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef HAL_I2S_Transmit_IT(I2S_HandleTypeDef *hi2s, uint16_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_I2S_Receive_IT(I2S_HandleTypeDef *hi2s, uint16_t *pData, uint16_t Size);
void HAL_I2S_IRQHandler(I2S_HandleTypeDef *hi2s);
HAL_StatusTypeDef HAL_I2S_Transmit_DMA(I2S_HandleTypeDef *hi2s, uint16_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_I2S_Receive_DMA(I2S_HandleTypeDef *hi2s, uint16_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_I2S_DMAPause(I2S_HandleTypeDef *hi2s);
HAL_StatusTypeDef HAL_I2S_DMAResume(I2S_HandleTypeDef *hi2s);
HAL_StatusTypeDef HAL_I2S_DMAStop(I2S_HandleTypeDef *hi2s);

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s);
void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s);
void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s);
void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s);
void HAL_I2S_ErrorCallback(I2S_HandleTypeDef *hi2s);

HAL_I2S_StateTypeDef HAL_I2S_GetState(const I2S_HandleTypeDef *hi2s);
uint32_t HAL_I2S_GetError(const I2S_HandleTypeDef *hi2s);

#define IS_I2S_MODE(__MODE__) (((__MODE__) == I2S_MODE_SLAVE_TX) || ((__MODE__) == I2S_MODE_SLAVE_RX) || ((__MODE__) == I2S_MODE_MASTER_TX) || ((__MODE__) == I2S_MODE_MASTER_RX))
#define IS_I2S_STANDARD(__STANDARD__) (((__STANDARD__) == I2S_STANDARD_PHILIPS) || ((__STANDARD__) == I2S_STANDARD_MSB) || ((__STANDARD__) == I2S_STANDARD_LSB) || ((__STANDARD__) == I2S_STANDARD_PCM_SHORT) || ((__STANDARD__) == I2S_STANDARD_PCM_LONG))
#define IS_I2S_DATA_FORMAT(__FORMAT__) (((__FORMAT__) == I2S_DATAFORMAT_16B) || ((__FORMAT__) == I2S_DATAFORMAT_16B_EXTENDED) || ((__FORMAT__) == I2S_DATAFORMAT_24B) || ((__FORMAT__) == I2S_DATAFORMAT_32B))
#define IS_I2S_MCLK_OUTPUT(__OUTPUT__) (((__OUTPUT__) == I2S_MCLKOUTPUT_ENABLE) || ((__OUTPUT__) == I2S_MCLKOUTPUT_DISABLE))
#define IS_I2S_AUDIO_FREQ(__FREQ__) ((((__FREQ__) >= I2S_AUDIOFREQ_8K) && ((__FREQ__) <= I2S_AUDIOFREQ_192K)) || ((__FREQ__) == I2S_AUDIOFREQ_DEFAULT))
#define IS_I2S_FULLDUPLEX_MODE(MODE) (((MODE) == I2S_FULLDUPLEXMODE_DISABLE) || ((MODE) == I2S_FULLDUPLEXMODE_ENABLE))
#define IS_I2S_CPOL(__CPOL__) (((__CPOL__) == I2S_CPOL_LOW) || ((__CPOL__) == I2S_CPOL_HIGH))
#define IS_I2S_CLOCKSOURCE(CLOCK) (((CLOCK) == I2S_CLOCK_EXTERNAL) || ((CLOCK) == I2S_CLOCK_PLL))

#ifdef __cplusplus
}
#endif

#endif /* STM32F4xx_HAL_I2S_H */
