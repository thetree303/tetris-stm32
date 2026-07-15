#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AUDIO_SAMPLE_RATE 48000
#define AUDIO_BUFFER_SIZE 1024

void Audio_Init(void);
void Audio_Play(const uint16_t *buffer, size_t size);
void Audio_Stop(void);
uint8_t Audio_IsPlaying(void);
void Audio_I2S_TxCpltCallback(void);

void Audio_PlayTone(uint16_t frequency_hz, uint16_t duration_ms);
void Audio_PlaySine(uint16_t frequency_hz);

#ifdef __cplusplus
}
#endif

#endif
