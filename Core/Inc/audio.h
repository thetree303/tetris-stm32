#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AUDIO_SAMPLE_RATE 16000
#define AUDIO_BUFFER_SIZE 1024

typedef enum {
  SFX_BGM,
  SFX_MOVE,
  SFX_ROTATE,
  SFX_ROW_CLEAR,
  SFX_ROW_CLEAR_4,
  SFX_GAME_OVER,
  SFX_COUNT
} SFX_ID;

extern volatile uint8_t bgm_volume;
extern volatile uint8_t sfx_volume;

void Audio_Init(void);
void Audio_StopAll(void);
void Audio_StopSFX(void);

void Audio_PlaySFX(SFX_ID id);
uint8_t Audio_IsSFXPlaying(void);

void Audio_StartBGM(void);
void Audio_StopBGM(void);
uint8_t Audio_IsBGMPlaying(void);

void Audio_I2S_TxCpltCallback(void);

#ifdef __cplusplus
}
#endif

#endif
