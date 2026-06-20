#ifndef MIDI_HANDLER_H
#define MIDI_HANDLER_H

#include <Arduino.h>
#include <math.h>

// ---------------------------------------------------------------------------
// MIDI utilities
// ---------------------------------------------------------------------------

// MIDI note to frequency conversion (A4 = MIDI 69 = 440Hz)
inline float midiNoteToFreq(int note) {
  return 440.0f * powf(2.0f, (note - 69) / 12.0f);
}

// MIDI CC constants
static const int CC_WAVEMORPH = 11; // CC for wavetable morphing

#endif
