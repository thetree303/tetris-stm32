#include "audio.h"
#include "sfx_data.h"
#include "main.h"

extern I2S_HandleTypeDef hi2s3;

typedef enum {
  AUDIO_IDLE,
  AUDIO_BGM,
  AUDIO_SFX
} AudioState;

static volatile AudioState state = AUDIO_IDLE;

static const uint16_t *bgm_data = NULL;
static uint32_t bgm_total = 0;
static uint32_t bgm_pos = 0;

static const uint16_t *sfx_ptr = NULL;
static uint32_t sfx_total = 0;

static uint32_t bgm_resume_pos = 0;
static uint8_t bgm_resume_flag = 0;

static void send_bgm_chunk(void) {
  uint32_t remaining = bgm_total - bgm_pos;
  uint16_t chunk = (remaining > AUDIO_BUFFER_SIZE) ? AUDIO_BUFFER_SIZE : remaining;
  HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t *)(bgm_data + bgm_pos), chunk);
  bgm_pos += chunk;
}

void Audio_Init(void) {
  bgm_data = sfx_info[SFX_BGM].data;
  bgm_total = sfx_info[SFX_BGM].length;
  state = AUDIO_IDLE;
  bgm_resume_flag = 0;
}

void Audio_StopAll(void) {
  HAL_I2S_DMAStop(&hi2s3);
  state = AUDIO_IDLE;
  bgm_resume_flag = 0;
}

void Audio_PlaySFX(SFX_ID id) {
  if (id >= SFX_COUNT || id == SFX_BGM)
    return;

  if (state == AUDIO_BGM) {
    HAL_I2S_DMAStop(&hi2s3);
    bgm_resume_pos = bgm_pos;
    bgm_resume_flag = 1;
  } else {
    HAL_I2S_DMAStop(&hi2s3);
    bgm_resume_flag = 0;
  }

  sfx_ptr = sfx_info[id].data;
  sfx_total = sfx_info[id].length;

  state = AUDIO_SFX;
  HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t *)sfx_ptr, sfx_total);
}

uint8_t Audio_IsSFXPlaying(void) {
  return (state == AUDIO_SFX);
}

void Audio_StartBGM(void) {
  if (state != AUDIO_IDLE)
    return;

  bgm_pos = 0;
  state = AUDIO_BGM;
  send_bgm_chunk();
}

void Audio_StopBGM(void) {
  if (state == AUDIO_BGM) {
    HAL_I2S_DMAStop(&hi2s3);
    state = AUDIO_IDLE;
  }
  bgm_resume_flag = 0;
}

uint8_t Audio_IsBGMPlaying(void) {
  return (state == AUDIO_BGM);
}

void Audio_I2S_TxCpltCallback(void) {
  if (state == AUDIO_BGM) {
    if (bgm_pos >= bgm_total)
      bgm_pos = 0;
    send_bgm_chunk();
  } else if (state == AUDIO_SFX) {
    state = AUDIO_IDLE;
    if (bgm_resume_flag) {
      bgm_pos = bgm_resume_pos;
      bgm_resume_flag = 0;
      if (bgm_pos >= bgm_total)
        bgm_pos = 0;
      state = AUDIO_BGM;
      send_bgm_chunk();
    }
  }
}
