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

  WavetableController(WavetableOsc &osc)
      : osc(osc), currentNote(69), pitchBendValue(8192), heldNoteCount(0),
        active(false) {}

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
    pitchBendValue = value;
    updateFrequency();
    __enable_irq();
  }

private:
  WavetableOsc &osc;
  volatile int currentNote;
  volatile int pitchBendValue;
  volatile int heldNotes[MAX_HELD_NOTES];
  volatile int heldNoteCount;
  volatile bool active;

  void updateFrequency() {
    // Convert MIDI note to base frequency
    float freq = midiNoteToFreq(currentNote);

    // Apply pitch bend: range 0-16383, center is 8192
    // Standard range is +/- 2 semitones
    float bendSemitones = (pitchBendValue - 8192) / 4096.0f;
    freq *= powf(2.0f, bendSemitones / 12.0f);

    // Set raw frequency on oscillator
    osc.setFrequency(freq);
  }
};

#endif
