#include "audio.h"
#include "sfx_data.h"
#include "main.h"
#include <string.h>

extern I2S_HandleTypeDef hi2s3;

// Ping-Pong buffer size (2048 elements = 2 halves of 1024)
#define I2S_BUFFER_SIZE 2048
static uint16_t i2s_buffer[I2S_BUFFER_SIZE];

// Playback state variables
static volatile uint8_t bgm_playing = 0;
static volatile uint8_t sfx_playing = 0;
static volatile uint8_t dma_active = 0;

volatile uint8_t bgm_volume = 50; // Default 50%
volatile uint8_t sfx_volume = 80; // Default 80%

static const uint16_t *bgm_data = NULL;
static uint32_t bgm_total = 0;
static volatile uint32_t bgm_pos = 0;

static const uint16_t *sfx_ptr = NULL;
static uint32_t sfx_total = 0;
static volatile uint32_t sfx_pos = 0;

// Internal mixer function - Đã sửa lỗi phát nhanh gấp đôi
static void fill_buffer_half(uint16_t *dest, uint32_t size) {
  // size ở đây là 1024. Chúng ta duyệt i += 2 để xử lý từng cặp kênh (Trái - Phải)
  for (uint32_t i = 0; i < size; i += 2) {
    int32_t mixed_sample = 0;
    
    // 1. Cộng mẫu BGM (Mono) với cấu hình âm lượng
    if (bgm_playing && bgm_data != NULL) {
      int32_t bgm_sample = (int16_t)bgm_data[bgm_pos];
      bgm_sample = (bgm_sample * bgm_volume) / 100;
      mixed_sample += bgm_sample;
      bgm_pos++;
      if (bgm_pos >= bgm_total) {
        bgm_pos = 0; // Loop BGM
      }
    }
    
    // 2. Cộng mẫu SFX (Mono) với cấu hình âm lượng
    if (sfx_playing && sfx_ptr != NULL) {
      int32_t sfx_sample = (int16_t)sfx_ptr[sfx_pos];
      sfx_sample = (sfx_sample * sfx_volume) / 100;
      mixed_sample += sfx_sample;
      sfx_pos++;
      if (sfx_pos >= sfx_total) {
        sfx_playing = 0;
        sfx_ptr = NULL;
        sfx_pos = 0;
      }
    }
    
    // Giới hạn biên độ (clipping) để tránh méo tiếng
    if (mixed_sample > 32767) {
      mixed_sample = 32767;
    } else if (mixed_sample < -32768) {
      mixed_sample = -32768;
    }
    
    // Nhân bản mẫu đơn (Mono) ra cả hai kênh Trái & Phải của Stereo
    dest[i]     = (uint16_t)mixed_sample; // Kênh Trái
    dest[i + 1] = (uint16_t)mixed_sample; // Kênh Phải
  }
}

void Audio_Init(void) {
  bgm_data = sfx_info[SFX_BGM].data;
  bgm_total = sfx_info[SFX_BGM].length;
  bgm_playing = 0;
  sfx_playing = 0;
  dma_active = 0;
  bgm_pos = 0;
  sfx_pos = 0;
  memset(i2s_buffer, 0, sizeof(i2s_buffer));
}

void Audio_StartBGM(void) {
  __disable_irq();
  if (bgm_playing) {
    __enable_irq();
    return;
  }
  bgm_playing = 1;
  bgm_pos = 0;
  
  if (!dma_active) {
    dma_active = 1;
    fill_buffer_half(&i2s_buffer[0], I2S_BUFFER_SIZE / 2);
    fill_buffer_half(&i2s_buffer[I2S_BUFFER_SIZE / 2], I2S_BUFFER_SIZE / 2);
    __enable_irq();
    HAL_I2S_Transmit_DMA(&hi2s3, i2s_buffer, I2S_BUFFER_SIZE);
  } else {
    __enable_irq();
  }
}

void Audio_StopBGM(void) {
  __disable_irq();
  bgm_playing = 0;
  bgm_pos = 0;
  if (!bgm_playing && !sfx_playing) {
    if (dma_active) {
      dma_active = 0;
      __enable_irq();
      HAL_I2S_DMAStop(&hi2s3);
      return;
    }
  }
  __enable_irq();
}

void Audio_PlaySFX(SFX_ID id) {
  if (id >= SFX_COUNT || id == SFX_BGM) return;

  __disable_irq();
  sfx_ptr = sfx_info[id].data;
  sfx_total = sfx_info[id].length;
  sfx_pos = 0;
  sfx_playing = 1;

  if (!dma_active) {
    dma_active = 1;
    fill_buffer_half(&i2s_buffer[0], I2S_BUFFER_SIZE / 2);
    fill_buffer_half(&i2s_buffer[I2S_BUFFER_SIZE / 2], I2S_BUFFER_SIZE / 2);
    __enable_irq();
    HAL_I2S_Transmit_DMA(&hi2s3, i2s_buffer, I2S_BUFFER_SIZE);
  } else {
    __enable_irq();
  }
}

void Audio_StopAll(void) {
  __disable_irq();
  bgm_playing = 0;
  sfx_playing = 0;
  sfx_ptr = NULL;
  bgm_pos = 0;
  sfx_pos = 0;
  if (dma_active) {
    dma_active = 0;
    __enable_irq();
    HAL_I2S_DMAStop(&hi2s3);
    return;
  }
  __enable_irq();
}

void Audio_StopSFX(void) {
  __disable_irq();
  sfx_playing = 0;
  sfx_ptr = NULL;
  sfx_pos = 0;
  __enable_irq();
}

uint8_t Audio_IsSFXPlaying(void) {
  return sfx_playing;
}

uint8_t Audio_IsBGMPlaying(void) {
  return bgm_playing;
}

// Dummy to satisfy audio.h
void Audio_I2S_TxCpltCallback(void) {
}

// Interrupt handlers called by HAL DMA interrupts
void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s) {
  if (hi2s->Instance == SPI3) {
    fill_buffer_half(&i2s_buffer[0], I2S_BUFFER_SIZE / 2);
    if (!bgm_playing && !sfx_playing && dma_active) {
      dma_active = 0;
      HAL_I2S_DMAStop(&hi2s3);
    }
  }
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s) {
  if (hi2s->Instance == SPI3) {
    fill_buffer_half(&i2s_buffer[I2S_BUFFER_SIZE / 2], I2S_BUFFER_SIZE / 2);
    if (!bgm_playing && !sfx_playing && dma_active) {
      dma_active = 0;
      HAL_I2S_DMAStop(&hi2s3);
    }
  }
}