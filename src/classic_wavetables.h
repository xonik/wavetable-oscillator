#ifndef CLASSIC_WAVETABLES_H
#define CLASSIC_WAVETABLES_H

#include <Arduino.h>
#include <math.h>

// ---------------------------------------------------------------------------
// Classic procedural waveforms kept separately from AIFF-converted tables
// ---------------------------------------------------------------------------

static const int CLASSIC_TABLE_SIZE = 1024;

int16_t classicWaveSine[CLASSIC_TABLE_SIZE];
int16_t classicWaveTriangle[CLASSIC_TABLE_SIZE];
int16_t classicWaveSaw[CLASSIC_TABLE_SIZE];
int16_t classicWaveSquare[CLASSIC_TABLE_SIZE];

void generateClassicWavetables() {
  for (int i = 0; i < CLASSIC_TABLE_SIZE; i++) {
    float phase = (float)i / CLASSIC_TABLE_SIZE;

    classicWaveSine[i] = (int16_t)(sinf(phase * 2.0f * M_PI) * 32767.0f);
    classicWaveTriangle[i] = (int16_t)((phase < 0.5f ? (4.0f * phase - 1.0f)
                                                     : (3.0f - 4.0f * phase)) *
                                       32767.0f);
    classicWaveSaw[i] = (int16_t)((phase * 2.0f - 1.0f) * 32767.0f);
    classicWaveSquare[i] = (phase > 0.5f) ? 32767 : -32767;
  }
}

#endif
