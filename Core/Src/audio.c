#include "audio.h"
#include "main.h"

extern I2S_HandleTypeDef hi2s3;

static volatile uint8_t audio_playing = 0;

static uint16_t sine_buffer[AUDIO_BUFFER_SIZE];

#define SINE_TABLE_SIZE 256
static const uint16_t sine_table[SINE_TABLE_SIZE] = {
  32768, 33572, 34375, 35174, 35968, 36755, 37533, 38299,
  39053, 39792, 40514, 41217, 41899, 42559, 43194, 43803,
  44384, 44935, 45456, 45944, 46400, 46820, 47205, 47554,
  47865, 48138, 48372, 48566, 48720, 48833, 48904, 48935,
  48924, 48873, 48779, 48645, 48469, 48252, 47995, 47698,
  47360, 46983, 46567, 46113, 45622, 45094, 44529, 43929,
  43295, 42627, 41927, 41195, 40433, 39642, 38823, 37978,
  37108, 36215, 35300, 34365, 33412, 32442, 31457, 30459,
  29450, 28432, 27407, 26377, 25344, 24311, 23279, 22251,
  21229, 20215, 19212, 18221, 17245, 16286, 15346, 14426,
  13530, 12659, 11816, 11002, 10219,  9470,  8755,  8077,
   7437,  6836,  6276,  5758,  5283,  4852,  4465,  4124,
   3828,  3578,  3374,  3215,  3102,  3033,  3008,  3027,
   3089,  3193,  3338,  3523,  3746,  4006,  4301,  4629,
   4989,  5378,  5795,  6236,  6700,  7185,  7689,  8209,
   8743,  9290,  9846, 10410, 10980, 11553, 12127, 12701,
  13273, 13839, 14400, 14952, 15494, 16025, 16542, 17045,
  17531, 18000, 18451, 18881, 19291, 19678, 20043, 20383,
  20699, 20989, 21253, 21490, 21700, 21883, 22038, 22165,
  22263, 22333, 22375, 22388, 22373, 22330, 22259, 22161,
  22035, 21883, 21704, 21499, 21269, 21014, 20735, 20433,
  20108, 19761, 19393, 19005, 18597, 18172, 17729, 17270,
  16796, 16308, 15808, 15296, 14775, 14245, 13707, 13163,
  12615, 12063, 11509, 10954, 10400, 9848,  9300,  8757,
  8221,  7693,  7174,  6666,  6169,  5686,  5216,  4762,
  4323,  3902,  3498,  3113,  2748,  2403,  2079,  1777,
  1497,  1240,  1006,   795,   609,   447,   310,   197,
   110,    47,    10,     0,    15,    56,   123,   215,
   333,   476,   645,   838,  1057,  1300,  1567,  1858,
  2172,  2509,  2868,  3248,  3649,  4071,  4511,  4970,
  5447,  5941,  6450,  6975,  7514,  8066,  8631,  9207,
  9794, 10391, 10997, 11610, 12231, 12857, 13489, 14125,
};

static void generate_tone(uint16_t *buffer, size_t size, uint16_t frequency) {
  uint32_t phase = 0;
  uint32_t step = (uint32_t)frequency * SINE_TABLE_SIZE / AUDIO_SAMPLE_RATE;
  for (size_t i = 0; i < size; i++) {
    buffer[i] = sine_table[phase];
    phase += step;
    if (phase >= SINE_TABLE_SIZE) phase -= SINE_TABLE_SIZE;
  }
}

void Audio_Init(void) {
  audio_playing = 0;
}

void Audio_Play(const uint16_t *buffer, size_t size) {
  HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t *)buffer, size);
  audio_playing = 1;
}

void Audio_Stop(void) {
  HAL_I2S_DMAStop(&hi2s3);
  audio_playing = 0;
}

uint8_t Audio_IsPlaying(void) {
  return audio_playing;
}

void Audio_I2S_TxCpltCallback(void) {
  audio_playing = 0;
}

void Audio_PlayTone(uint16_t frequency_hz, uint16_t duration_ms) {
  generate_tone(sine_buffer, AUDIO_BUFFER_SIZE, frequency_hz);
  Audio_Play(sine_buffer, AUDIO_BUFFER_SIZE);
}

void Audio_PlaySine(uint16_t frequency_hz) {
  generate_tone(sine_buffer, AUDIO_BUFFER_SIZE, frequency_hz);
  Audio_Play(sine_buffer, AUDIO_BUFFER_SIZE);
}
