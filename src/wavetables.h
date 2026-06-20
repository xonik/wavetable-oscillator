#ifndef WAVETABLES_H
#define WAVETABLES_H

#include <Arduino.h>
#include <math.h>

// ---------------------------------------------------------------------------
// Wavetable data and generation
// ---------------------------------------------------------------------------

static const int TABLE_SIZE = 1024;

int16_t waveSine[TABLE_SIZE];
int16_t waveTriangle[TABLE_SIZE];
int16_t waveSaw[TABLE_SIZE];
int16_t waveSquare[TABLE_SIZE];
int16_t waveTable4[TABLE_SIZE];
int16_t waveTable5[TABLE_SIZE];
int16_t waveTable6[TABLE_SIZE];
int16_t waveTable7[TABLE_SIZE];

void generateWavetables() {
  for (int i = 0; i < TABLE_SIZE; i++) {
    float phase = (float)i / TABLE_SIZE;

    waveSine[i] = (int16_t)(sinf(phase * 2.0f * M_PI) * 32767.0f);
    waveTriangle[i] = (int16_t)((phase < 0.5f ? (4.0f * phase - 1.0f)
                                              : (3.0f - 4.0f * phase)) *
                                32767.0f);
    waveSaw[i] = (int16_t)((phase * 2.0f - 1.0f) * 32767.0f);
    waveSquare[i] = (phase > 0.5f) ? 32767 : -32767;
    waveTable4[i] = waveSine[i];
    waveTable5[i] = waveSine[i];
    waveTable6[i] = waveSine[i];
    waveTable7[i] = waveSine[i];
  }
}

#endif
