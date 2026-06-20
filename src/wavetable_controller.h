#ifndef WAVETABLE_CONTROLLER_H
#define WAVETABLE_CONTROLLER_H

#include "midi_handler.h"
#include "wavetable_osc.h"
#include <Arduino.h>

// ---------------------------------------------------------------------------
// WavetableController - Manages MIDI note tracking, pitch bend, and legato
// Converts MIDI input to raw frequency and controls the oscillator
// ---------------------------------------------------------------------------

class WavetableController {
public:
  static constexpr int MAX_HELD_NOTES = 16;
  static constexpr float PITCH_BEND_RANGE_SEMITONES = 2.0f;

  enum class PitchBendEncoding {
    Unknown,
    Signed8192,   // -8192..8191, center 0
    Unsigned16384 // 0..16383, center 8192
  };

  WavetableController(WavetableOsc &osc)
      : osc(osc), currentNote(69), pitchBendSemitones(0.0f), heldNoteCount(0),
        active(false), pitchBendEncoding(PitchBendEncoding::Unknown) {}

  WavetableOsc &getOscillator() { return osc; }

  void initializeFrequency() {
    // Ensure initial frequency accounts for current pitch bend state
    updateFrequency();
  }

  void noteOn(int midiNote, int midiVelocity) {
    __disable_irq();

    // Add note to held notes stack if not already there
    bool alreadyHeld = false;
    for (int i = 0; i < heldNoteCount; i++) {
      if (heldNotes[i] == midiNote) {
        alreadyHeld = true;
        break;
      }
    }
    if (!alreadyHeld && heldNoteCount < MAX_HELD_NOTES) {
      heldNotes[heldNoteCount++] = midiNote;
    }

    // Find highest note
    int highestNote = heldNotes[0];
    for (int i = 1; i < heldNoteCount; i++) {
      if (heldNotes[i] > highestNote) {
        highestNote = heldNotes[i];
      }
    }
    currentNote = highestNote;

    // Only activate oscillator on first note
    if (heldNoteCount == 1) {
      active = true;
      osc.setActive(true);
      osc.setVelocity(midiVelocity / 127.0f);
    }

    updateFrequency();
    __enable_irq();
  }

  void noteOff(int midiNote) {
    __disable_irq();

    // Remove note from held notes
    for (int i = 0; i < heldNoteCount; i++) {
      if (heldNotes[i] == midiNote) {
        // Shift remaining notes down
        for (int j = i; j < heldNoteCount - 1; j++) {
          heldNotes[j] = heldNotes[j + 1];
        }
        heldNoteCount--;
        break;
      }
    }

    // If notes remain, switch to highest
    if (heldNoteCount > 0) {
      int highestNote = heldNotes[0];
      for (int i = 1; i < heldNoteCount; i++) {
        if (heldNotes[i] > highestNote) {
          highestNote = heldNotes[i];
        }
      }
      currentNote = highestNote;
      updateFrequency();
    } else {
      // All notes released
      active = false;
      osc.setActive(false);
    }
    __enable_irq();
  }

  void setPitchBend(int value) {
    __disable_irq();

    // Teensy MIDI pitch bend callbacks may provide either signed
    // (-8192..8191) or unsigned (0..16383) values depending on stack.
    // Detect once and default to signed in the ambiguous 0..8191 range,
    // which avoids startup drops when center is reported as 0.
    if (pitchBendEncoding == PitchBendEncoding::Unknown) {
      if (value < 0) {
        pitchBendEncoding = PitchBendEncoding::Signed8192;
      } else if (value > 8191) {
        pitchBendEncoding = PitchBendEncoding::Unsigned16384;
      } else {
        pitchBendEncoding = PitchBendEncoding::Signed8192;
      }
    }

    float normalized = 0.0f;
    if (pitchBendEncoding == PitchBendEncoding::Signed8192) {
      int clamped = constrain(value, -8192, 8191);
      normalized = clamped / 8192.0f;
    } else {
      int clamped = constrain(value, 0, 16383);
      normalized = (clamped - 8192) / 8192.0f;
    }

    pitchBendSemitones = normalized * PITCH_BEND_RANGE_SEMITONES;
    updateFrequency();
    __enable_irq();
  }

private:
  WavetableOsc &osc;
  volatile int currentNote;
  volatile float pitchBendSemitones;
  volatile int heldNotes[MAX_HELD_NOTES];
  volatile int heldNoteCount;
  volatile bool active;
  volatile PitchBendEncoding pitchBendEncoding;

  void updateFrequency() {
    // Convert MIDI note to base frequency
    float freq = midiNoteToFreq(currentNote);

    // Apply pitch bend (already normalized to semitones).
    freq *= powf(2.0f, pitchBendSemitones / 12.0f);

    // Set raw frequency on oscillator
    osc.setFrequency(freq);
  }
};

#endif
