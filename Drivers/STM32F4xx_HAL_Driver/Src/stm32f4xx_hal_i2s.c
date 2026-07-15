/**
  ******************************************************************************
  * @file    stm32f4xx_hal_i2s.c
  * @author  MCD Application Team
  * @brief   I2S HAL module driver.
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

#include "stm32f4xx_hal.h"

#ifdef HAL_I2S_MODULE_ENABLED

#define I2S_TIMEOUT_FLAG          100U

static void I2S_DMATxCplt(DMA_HandleTypeDef *hdma);
static void I2S_DMATxHalfCplt(DMA_HandleTypeDef *hdma);
static void I2S_DMARxCplt(DMA_HandleTypeDef *hdma);
static void I2S_DMARxHalfCplt(DMA_HandleTypeDef *hdma);
static void I2S_DMAError(DMA_HandleTypeDef *hdma);
static void I2S_Transmit_IT(I2S_HandleTypeDef *hi2s);
static void I2S_Receive_IT(I2S_HandleTypeDef *hi2s);
static void I2S_IRQHandler(I2S_HandleTypeDef *hi2s);
static HAL_StatusTypeDef I2S_WaitFlagStateUntilTimeout(I2S_HandleTypeDef *hi2s, uint32_t Flag, FlagStatus State, uint32_t Timeout);

HAL_StatusTypeDef HAL_I2S_Init(I2S_HandleTypeDef *hi2s)
{
  uint32_t i2sdiv;
  uint32_t i2sodd;
  uint32_t packetlength;
  uint32_t tmp;
  uint32_t i2sclk;

  if (hi2s == NULL) return HAL_ERROR;

  assert_param(IS_I2S_ALL_INSTANCE(hi2s->Instance));
  assert_param(IS_I2S_MODE(hi2s->Init.Mode));
  assert_param(IS_I2S_STANDARD(hi2s->Init.Standard));
  assert_param(IS_I2S_DATA_FORMAT(hi2s->Init.DataFormat));
  assert_param(IS_I2S_MCLK_OUTPUT(hi2s->Init.MCLKOutput));
  assert_param(IS_I2S_AUDIO_FREQ(hi2s->Init.AudioFreq));
  assert_param(IS_I2S_CPOL(hi2s->Init.CPOL));
  assert_param(IS_I2S_CLOCKSOURCE(hi2s->Init.ClockSource));

  if (hi2s->State == HAL_I2S_STATE_RESET)
  {
    hi2s->Lock = HAL_UNLOCKED;
    hi2s->IrqHandlerISR = I2S_IRQHandler;
    HAL_I2S_MspInit(hi2s);
  }

  hi2s->State = HAL_I2S_STATE_BUSY;

  CLEAR_BIT(hi2s->Instance->I2SCFGR, (SPI_I2SCFGR_CHLEN | SPI_I2SCFGR_DATLEN | SPI_I2SCFGR_CKPOL | \
                                       SPI_I2SCFGR_I2SSTD | SPI_I2SCFGR_PCMSYNC | SPI_I2SCFGR_I2SCFG | \
                                       SPI_I2SCFGR_I2SE | SPI_I2SCFGR_I2SMOD));
  hi2s->Instance->I2SPR = 0x0002U;

  if (hi2s->Init.AudioFreq != I2S_AUDIOFREQ_DEFAULT)
  {
    if (hi2s->Init.DataFormat == I2S_DATAFORMAT_16B)
      packetlength = 16U;
    else
      packetlength = 32U;

    if (hi2s->Init.Standard <= I2S_STANDARD_LSB)
      packetlength = packetlength * 2U;

    i2sclk = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_I2S);

    if (hi2s->Init.MCLKOutput == I2S_MCLKOUTPUT_ENABLE)
    {
      if (hi2s->Init.DataFormat != I2S_DATAFORMAT_16B)
        tmp = (uint32_t)(((((i2sclk / (packetlength * 4U)) * 10U) / hi2s->Init.AudioFreq)) + 5U);
      else
        tmp = (uint32_t)(((((i2sclk / (packetlength * 8U)) * 10U) / hi2s->Init.AudioFreq)) + 5U);
    }
    else
    {
      tmp = (uint32_t)(((((i2sclk / packetlength) * 10U) / hi2s->Init.AudioFreq)) + 5U);
    }

    tmp = tmp / 10U;
    i2sodd = (uint32_t)(tmp & (uint32_t)1U);
    i2sdiv = (uint32_t)((tmp - i2sodd) / 2U);
    i2sodd = (uint32_t)(i2sodd << 8U);
  }
  else
  {
    i2sdiv = 2U;
    i2sodd = 0U;
  }

  if ((i2sdiv < 2U) || (i2sdiv > 0xFFU))
  {
    SET_BIT(hi2s->ErrorCode, HAL_I2S_ERROR_PRESCALER);
    return HAL_ERROR;
  }

  hi2s->Instance->I2SPR = (uint32_t)((uint32_t)i2sdiv | (uint32_t)(i2sodd | (uint32_t)hi2s->Init.MCLKOutput));

  MODIFY_REG(hi2s->Instance->I2SCFGR, (SPI_I2SCFGR_CHLEN | SPI_I2SCFGR_DATLEN | \
                                        SPI_I2SCFGR_CKPOL | SPI_I2SCFGR_I2SSTD | \
                                        SPI_I2SCFGR_PCMSYNC | SPI_I2SCFGR_I2SCFG | \
                                        SPI_I2SCFGR_I2SE  | SPI_I2SCFGR_I2SMOD), \
             (SPI_I2SCFGR_I2SMOD | hi2s->Init.Mode | hi2s->Init.Standard | hi2s->Init.DataFormat | hi2s->Init.CPOL));

  hi2s->ErrorCode = HAL_I2S_ERROR_NONE;
  hi2s->State = HAL_I2S_STATE_READY;

  return HAL_OK;
}

HAL_StatusTypeDef HAL_I2S_DeInit(I2S_HandleTypeDef *hi2s)
{
  if (hi2s == NULL) return HAL_ERROR;
  assert_param(IS_I2S_ALL_INSTANCE(hi2s->Instance));

  hi2s->State = HAL_I2S_STATE_BUSY;
  __HAL_I2S_DISABLE(hi2s);
  HAL_I2S_MspDeInit(hi2s);

  hi2s->ErrorCode = HAL_I2S_ERROR_NONE;
  hi2s->State = HAL_I2S_STATE_RESET;
  __HAL_UNLOCK(hi2s);
  return HAL_OK;
}

__weak void HAL_I2S_MspInit(I2S_HandleTypeDef *hi2s) { UNUSED(hi2s); }
__weak void HAL_I2S_MspDeInit(I2S_HandleTypeDef *hi2s) { UNUSED(hi2s); }

HAL_StatusTypeDef HAL_I2S_Transmit(I2S_HandleTypeDef *hi2s, uint16_t *pData, uint16_t Size, uint32_t Timeout)
{
  uint32_t tmpreg_cfgr;
  if ((pData == NULL) || (Size == 0U)) return HAL_ERROR;
  if (hi2s->State != HAL_I2S_STATE_READY) return HAL_BUSY;

  __HAL_LOCK(hi2s);
  hi2s->State = HAL_I2S_STATE_BUSY_TX;
  hi2s->ErrorCode = HAL_I2S_ERROR_NONE;
  hi2s->pTxBuffPtr = pData;

  tmpreg_cfgr = hi2s->Instance->I2SCFGR & (SPI_I2SCFGR_DATLEN | SPI_I2SCFGR_CHLEN);
  if ((tmpreg_cfgr == I2S_DATAFORMAT_24B) || (tmpreg_cfgr == I2S_DATAFORMAT_32B))
  { hi2s->TxXferSize = (Size << 1U); hi2s->TxXferCount = (Size << 1U); }
  else
  { hi2s->TxXferSize = Size; hi2s->TxXferCount = Size; }

  tmpreg_cfgr = hi2s->Instance->I2SCFGR;
  if ((hi2s->Instance->I2SCFGR & SPI_I2SCFGR_I2SE) != SPI_I2SCFGR_I2SE) __HAL_I2S_ENABLE(hi2s);

  if (I2S_WaitFlagStateUntilTimeout(hi2s, I2S_FLAG_TXE, SET, Timeout) != HAL_OK)
  { SET_BIT(hi2s->ErrorCode, HAL_I2S_ERROR_TIMEOUT); hi2s->State = HAL_I2S_STATE_READY; __HAL_UNLOCK(hi2s); return HAL_ERROR; }

  while (hi2s->TxXferCount > 0U)
  {
    hi2s->Instance->DR = (*hi2s->pTxBuffPtr);
    hi2s->pTxBuffPtr++; hi2s->TxXferCount--;
    if (I2S_WaitFlagStateUntilTimeout(hi2s, I2S_FLAG_TXE, SET, Timeout) != HAL_OK)
    { SET_BIT(hi2s->ErrorCode, HAL_I2S_ERROR_TIMEOUT); hi2s->State = HAL_I2S_STATE_READY; __HAL_UNLOCK(hi2s); return HAL_ERROR; }
    if (__HAL_I2S_GET_FLAG(hi2s, I2S_FLAG_UDR) == SET)
    { __HAL_I2S_CLEAR_UDRFLAG(hi2s); SET_BIT(hi2s->ErrorCode, HAL_I2S_ERROR_UDR); }
  }

  if (((tmpreg_cfgr & SPI_I2SCFGR_I2SCFG) == I2S_MODE_SLAVE_TX) || ((tmpreg_cfgr & SPI_I2SCFGR_I2SCFG) == I2S_MODE_SLAVE_RX))
  {
    if (I2S_WaitFlagStateUntilTimeout(hi2s, I2S_FLAG_BSY, RESET, Timeout) != HAL_OK)
    { SET_BIT(hi2s->ErrorCode, HAL_I2S_ERROR_TIMEOUT); hi2s->State = HAL_I2S_STATE_READY; __HAL_UNLOCK(hi2s); return HAL_ERROR; }
  }

  hi2s->State = HAL_I2S_STATE_READY;
  __HAL_UNLOCK(hi2s);
  return HAL_OK;
}

HAL_StatusTypeDef HAL_I2S_Receive(I2S_HandleTypeDef *hi2s, uint16_t *pData, uint16_t Size, uint32_t Timeout)
{
  uint32_t tmpreg_cfgr;
  if ((pData == NULL) || (Size == 0U)) return HAL_ERROR;
  if (hi2s->State != HAL_I2S_STATE_READY) return HAL_BUSY;

  __HAL_LOCK(hi2s);
  hi2s->State = HAL_I2S_STATE_BUSY_RX;
  hi2s->ErrorCode = HAL_I2S_ERROR_NONE;
  hi2s->pRxBuffPtr = pData;

  tmpreg_cfgr = hi2s->Instance->I2SCFGR & (SPI_I2SCFGR_DATLEN | SPI_I2SCFGR_CHLEN);
  if ((tmpreg_cfgr == I2S_DATAFORMAT_24B) || (tmpreg_cfgr == I2S_DATAFORMAT_32B))
  { hi2s->RxXferSize = (Size << 1U); hi2s->RxXferCount = (Size << 1U); }
  else
  { hi2s->RxXferSize = Size; hi2s->RxXferCount = Size; }

  if ((hi2s->Instance->I2SCFGR & SPI_I2SCFGR_I2SE) != SPI_I2SCFGR_I2SE) __HAL_I2S_ENABLE(hi2s);
  if ((hi2s->Instance->I2SCFGR & SPI_I2SCFGR_I2SCFG) == I2S_MODE_MASTER_RX) __HAL_I2S_CLEAR_OVRFLAG(hi2s);

  while (hi2s->RxXferCount > 0U)
  {
    if (I2S_WaitFlagStateUntilTimeout(hi2s, I2S_FLAG_RXNE, SET, Timeout) != HAL_OK)
    { SET_BIT(hi2s->ErrorCode, HAL_I2S_ERROR_TIMEOUT); hi2s->State = HAL_I2S_STATE_READY; __HAL_UNLOCK(hi2s); return HAL_ERROR; }
    (*hi2s->pRxBuffPtr) = (uint16_t)hi2s->Instance->DR;
    hi2s->pRxBuffPtr++; hi2s->RxXferCount--;
    if (__HAL_I2S_GET_FLAG(hi2s, I2S_FLAG_OVR) == SET)
    { __HAL_I2S_CLEAR_OVRFLAG(hi2s); SET_BIT(hi2s->ErrorCode, HAL_I2S_ERROR_OVR); }
  }

  hi2s->State = HAL_I2S_STATE_READY;
  __HAL_UNLOCK(hi2s);
  return HAL_OK;
}

HAL_StatusTypeDef HAL_I2S_Transmit_IT(I2S_HandleTypeDef *hi2s, uint16_t *pData, uint16_t Size)
{
  uint32_t tmpreg_cfgr;
  if ((pData == NULL) || (Size == 0U)) return HAL_ERROR;
  if (hi2s->State != HAL_I2S_STATE_READY) return HAL_BUSY;

  __HAL_LOCK(hi2s);
  hi2s->State = HAL_I2S_STATE_BUSY_TX;
  hi2s->ErrorCode = HAL_I2S_ERROR_NONE;
  hi2s->pTxBuffPtr = pData;

  tmpreg_cfgr = hi2s->Instance->I2SCFGR & (SPI_I2SCFGR_DATLEN | SPI_I2SCFGR_CHLEN);
  if ((tmpreg_cfgr == I2S_DATAFORMAT_24B) || (tmpreg_cfgr == I2S_DATAFORMAT_32B))
  { hi2s->TxXferSize = (Size << 1U); hi2s->TxXferCount = (Size << 1U); }
  else
  { hi2s->TxXferSize = Size; hi2s->TxXferCount = Size; }

  __HAL_UNLOCK(hi2s);
  __HAL_I2S_ENABLE_IT(hi2s, (I2S_IT_TXE | I2S_IT_ERR));
  if ((hi2s->Instance->I2SCFGR & SPI_I2SCFGR_I2SE) != SPI_I2SCFGR_I2SE) __HAL_I2S_ENABLE(hi2s);
  return HAL_OK;
}

HAL_StatusTypeDef HAL_I2S_Receive_IT(I2S_HandleTypeDef *hi2s, uint16_t *pData, uint16_t Size)
{
  uint32_t tmpreg_cfgr;
  if ((pData == NULL) || (Size == 0U)) return HAL_ERROR;
  if (hi2s->State != HAL_I2S_STATE_READY) return HAL_BUSY;

  __HAL_LOCK(hi2s);
  hi2s->State = HAL_I2S_STATE_BUSY_RX;
  hi2s->ErrorCode = HAL_I2S_ERROR_NONE;
  hi2s->pRxBuffPtr = pData;

  tmpreg_cfgr = hi2s->Instance->I2SCFGR & (SPI_I2SCFGR_DATLEN | SPI_I2SCFGR_CHLEN);
  if ((tmpreg_cfgr == I2S_DATAFORMAT_24B) || (tmpreg_cfgr == I2S_DATAFORMAT_32B))
  { hi2s->RxXferSize = (Size << 1U); hi2s->RxXferCount = (Size << 1U); }
  else
  { hi2s->RxXferSize = Size; hi2s->RxXferCount = Size; }

  __HAL_UNLOCK(hi2s);
  __HAL_I2S_ENABLE_IT(hi2s, (I2S_IT_RXNE | I2S_IT_ERR));
  if ((hi2s->Instance->I2SCFGR & SPI_I2SCFGR_I2SE) != SPI_I2SCFGR_I2SE) __HAL_I2S_ENABLE(hi2s);
  return HAL_OK;
}

HAL_StatusTypeDef HAL_I2S_Transmit_DMA(I2S_HandleTypeDef *hi2s, uint16_t *pData, uint16_t Size)
{
  uint32_t tmpreg_cfgr;
  if ((pData == NULL) || (Size == 0U)) return HAL_ERROR;
  if (hi2s->State != HAL_I2S_STATE_READY) return HAL_BUSY;

  __HAL_LOCK(hi2s);
  hi2s->State = HAL_I2S_STATE_BUSY_TX;
  hi2s->ErrorCode = HAL_I2S_ERROR_NONE;
  hi2s->pTxBuffPtr = pData;

  tmpreg_cfgr = hi2s->Instance->I2SCFGR & (SPI_I2SCFGR_DATLEN | SPI_I2SCFGR_CHLEN);
  if ((tmpreg_cfgr == I2S_DATAFORMAT_24B) || (tmpreg_cfgr == I2S_DATAFORMAT_32B))
  { hi2s->TxXferSize = (Size << 1U); hi2s->TxXferCount = (Size << 1U); }
  else
  { hi2s->TxXferSize = Size; hi2s->TxXferCount = Size; }

  hi2s->hdmatx->XferHalfCpltCallback = I2S_DMATxHalfCplt;
  hi2s->hdmatx->XferCpltCallback = I2S_DMATxCplt;
  hi2s->hdmatx->XferErrorCallback = I2S_DMAError;

  if (HAL_OK != HAL_DMA_Start_IT(hi2s->hdmatx, (uint32_t)hi2s->pTxBuffPtr, (uint32_t)&hi2s->Instance->DR, hi2s->TxXferSize))
  { SET_BIT(hi2s->ErrorCode, HAL_I2S_ERROR_DMA); hi2s->State = HAL_I2S_STATE_READY; __HAL_UNLOCK(hi2s); return HAL_ERROR; }

  __HAL_UNLOCK(hi2s);
  if (HAL_IS_BIT_CLR(hi2s->Instance->CR2, SPI_CR2_TXDMAEN)) SET_BIT(hi2s->Instance->CR2, SPI_CR2_TXDMAEN);
  if (HAL_IS_BIT_CLR(hi2s->Instance->I2SCFGR, SPI_I2SCFGR_I2SE)) __HAL_I2S_ENABLE(hi2s);
  return HAL_OK;
}

HAL_StatusTypeDef HAL_I2S_Receive_DMA(I2S_HandleTypeDef *hi2s, uint16_t *pData, uint16_t Size)
{
  uint32_t tmpreg_cfgr;
  if ((pData == NULL) || (Size == 0U)) return HAL_ERROR;
  if (hi2s->State != HAL_I2S_STATE_READY) return HAL_BUSY;

  __HAL_LOCK(hi2s);
  hi2s->State = HAL_I2S_STATE_BUSY_RX;
  hi2s->ErrorCode = HAL_I2S_ERROR_NONE;
  hi2s->pRxBuffPtr = pData;

  tmpreg_cfgr = hi2s->Instance->I2SCFGR & (SPI_I2SCFGR_DATLEN | SPI_I2SCFGR_CHLEN);
  if ((tmpreg_cfgr == I2S_DATAFORMAT_24B) || (tmpreg_cfgr == I2S_DATAFORMAT_32B))
  { hi2s->RxXferSize = (Size << 1U); hi2s->RxXferCount = (Size << 1U); }
  else
  { hi2s->RxXferSize = Size; hi2s->RxXferCount = Size; }

  hi2s->hdmarx->XferHalfCpltCallback = I2S_DMARxHalfCplt;
  hi2s->hdmarx->XferCpltCallback = I2S_DMARxCplt;
  hi2s->hdmarx->XferErrorCallback = I2S_DMAError;

  if ((hi2s->Instance->I2SCFGR & SPI_I2SCFGR_I2SCFG) == I2S_MODE_MASTER_RX) __HAL_I2S_CLEAR_OVRFLAG(hi2s);

  if (HAL_OK != HAL_DMA_Start_IT(hi2s->hdmarx, (uint32_t)&hi2s->Instance->DR, (uint32_t)hi2s->pRxBuffPtr, hi2s->RxXferSize))
  { SET_BIT(hi2s->ErrorCode, HAL_I2S_ERROR_DMA); hi2s->State = HAL_I2S_STATE_READY; __HAL_UNLOCK(hi2s); return HAL_ERROR; }

  __HAL_UNLOCK(hi2s);
  if (HAL_IS_BIT_CLR(hi2s->Instance->CR2, SPI_CR2_RXDMAEN)) SET_BIT(hi2s->Instance->CR2, SPI_CR2_RXDMAEN);
  if ((hi2s->Instance->I2SCFGR & SPI_I2SCFGR_I2SE) != SPI_I2SCFGR_I2SE) __HAL_I2S_ENABLE(hi2s);
  return HAL_OK;
}

HAL_StatusTypeDef HAL_I2S_DMAPause(I2S_HandleTypeDef *hi2s)
{
  __HAL_LOCK(hi2s);
  if (hi2s->State == HAL_I2S_STATE_BUSY_TX) CLEAR_BIT(hi2s->Instance->CR2, SPI_CR2_TXDMAEN);
  else if (hi2s->State == HAL_I2S_STATE_BUSY_RX) CLEAR_BIT(hi2s->Instance->CR2, SPI_CR2_RXDMAEN);
  __HAL_UNLOCK(hi2s);
  return HAL_OK;
}

HAL_StatusTypeDef HAL_I2S_DMAResume(I2S_HandleTypeDef *hi2s)
{
  __HAL_LOCK(hi2s);
  if (hi2s->State == HAL_I2S_STATE_BUSY_TX) SET_BIT(hi2s->Instance->CR2, SPI_CR2_TXDMAEN);
  else if (hi2s->State == HAL_I2S_STATE_BUSY_RX) SET_BIT(hi2s->Instance->CR2, SPI_CR2_RXDMAEN);
  if (HAL_IS_BIT_CLR(hi2s->Instance->I2SCFGR, SPI_I2SCFGR_I2SE)) __HAL_I2S_ENABLE(hi2s);
  __HAL_UNLOCK(hi2s);
  return HAL_OK;
}

HAL_StatusTypeDef HAL_I2S_DMAStop(I2S_HandleTypeDef *hi2s)
{
  __HAL_LOCK(hi2s);
  CLEAR_BIT(hi2s->Instance->CR2, SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN);
  if (hi2s->hdmatx != NULL) { HAL_DMA_Abort(hi2s->hdmatx); }
  if (hi2s->hdmarx != NULL) { HAL_DMA_Abort(hi2s->hdmarx); }
  __HAL_I2S_DISABLE(hi2s);
  hi2s->State = HAL_I2S_STATE_READY;
  __HAL_UNLOCK(hi2s);
  return HAL_OK;
}

__weak void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s) { UNUSED(hi2s); }
__weak void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s) { UNUSED(hi2s); }
__weak void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s) { UNUSED(hi2s); }
__weak void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s) { UNUSED(hi2s); }
__weak void HAL_I2S_ErrorCallback(I2S_HandleTypeDef *hi2s) { UNUSED(hi2s); }

HAL_I2S_StateTypeDef HAL_I2S_GetState(const I2S_HandleTypeDef *hi2s) { return hi2s->State; }
uint32_t HAL_I2S_GetError(const I2S_HandleTypeDef *hi2s) { return hi2s->ErrorCode; }

void HAL_I2S_IRQHandler(I2S_HandleTypeDef *hi2s)
{
  if (hi2s->IrqHandlerISR != NULL) hi2s->IrqHandlerISR(hi2s);
}

static void I2S_IRQHandler(I2S_HandleTypeDef *hi2s)
{
  uint32_t sr = hi2s->Instance->SR;
  uint32_t cr2 = hi2s->Instance->CR2;

  if (I2S_CHECK_FLAG(sr, I2S_FLAG_OVR) && I2S_CHECK_IT_SOURCE(cr2, I2S_IT_ERR))
  { __HAL_I2S_CLEAR_OVRFLAG(hi2s); HAL_I2S_ErrorCallback(hi2s); }

  if (I2S_CHECK_FLAG(sr, I2S_FLAG_UDR) && I2S_CHECK_IT_SOURCE(cr2, I2S_IT_ERR))
  { __HAL_I2S_CLEAR_UDRFLAG(hi2s); HAL_I2S_ErrorCallback(hi2s); }

  if (I2S_CHECK_FLAG(sr, I2S_FLAG_FRE) && I2S_CHECK_IT_SOURCE(cr2, I2S_IT_ERR))
  { __HAL_I2S_FLUSH_RX_DR(hi2s); HAL_I2S_ErrorCallback(hi2s); }

  if (I2S_CHECK_FLAG(sr, I2S_FLAG_TXE) && I2S_CHECK_IT_SOURCE(cr2, I2S_IT_TXE))
  { I2S_Transmit_IT(hi2s); }

  if (I2S_CHECK_FLAG(sr, I2S_FLAG_RXNE) && I2S_CHECK_IT_SOURCE(cr2, I2S_IT_RXNE))
  { I2S_Receive_IT(hi2s); }
}

static void I2S_Transmit_IT(I2S_HandleTypeDef *hi2s)
{
  if (hi2s->TxXferCount == 0U)
  {
    __HAL_I2S_DISABLE_IT(hi2s, I2S_IT_TXE);
    HAL_I2S_TxCpltCallback(hi2s);
  }
  else
  {
    hi2s->Instance->DR = (*hi2s->pTxBuffPtr);
    hi2s->pTxBuffPtr++; hi2s->TxXferCount--;
  }
}

static void I2S_Receive_IT(I2S_HandleTypeDef *hi2s)
{
  (*hi2s->pRxBuffPtr) = (uint16_t)hi2s->Instance->DR;
  hi2s->pRxBuffPtr++;
  hi2s->RxXferCount--;

  if (hi2s->RxXferCount == 0U)
  {
    __HAL_I2S_DISABLE_IT(hi2s, I2S_IT_RXNE);
    HAL_I2S_RxCpltCallback(hi2s);
  }
}

static void I2S_DMATxCplt(DMA_HandleTypeDef *hdma)
{
  I2S_HandleTypeDef *hi2s = (I2S_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;
  if (HAL_IS_BIT_CLR(hi2s->Instance->CR2, SPI_CR2_TXDMAEN)) return;
  HAL_I2S_TxCpltCallback(hi2s);
}

static void I2S_DMATxHalfCplt(DMA_HandleTypeDef *hdma)
{
  I2S_HandleTypeDef *hi2s = (I2S_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;
  HAL_I2S_TxHalfCpltCallback(hi2s);
}

static void I2S_DMARxCplt(DMA_HandleTypeDef *hdma)
{
  I2S_HandleTypeDef *hi2s = (I2S_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;
  HAL_I2S_RxCpltCallback(hi2s);
}

static void I2S_DMARxHalfCplt(DMA_HandleTypeDef *hdma)
{
  I2S_HandleTypeDef *hi2s = (I2S_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;
  HAL_I2S_RxHalfCpltCallback(hi2s);
}

static void I2S_DMAError(DMA_HandleTypeDef *hdma)
{
  I2S_HandleTypeDef *hi2s = (I2S_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;
  SET_BIT(hi2s->ErrorCode, HAL_I2S_ERROR_DMA);
  HAL_I2S_ErrorCallback(hi2s);
}

static HAL_StatusTypeDef I2S_WaitFlagStateUntilTimeout(I2S_HandleTypeDef *hi2s, uint32_t Flag, FlagStatus State, uint32_t Timeout)
{
  uint32_t tickstart = HAL_GetTick();
  while ((__HAL_I2S_GET_FLAG(hi2s, Flag) ? SET : RESET) != State)
  { if (Timeout != HAL_MAX_DELAY) { if ((HAL_GetTick() - tickstart) > Timeout) return HAL_ERROR; } }
  return HAL_OK;
}

#endif /* HAL_I2S_MODULE_ENABLED */
