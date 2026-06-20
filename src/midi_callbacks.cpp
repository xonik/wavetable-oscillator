#include "wavetable_controller.h"

// ---------------------------------------------------------------------------
// MIDI Callbacks
// ---------------------------------------------------------------------------

extern WavetableController controller; // Defined in main.cpp

void onNoteOn(byte channel, byte note, byte velocity) {
  if (velocity == 0) {
    // Note-on with velocity 0 is a note-off per MIDI spec
    controller.noteOff(note);
    return;
  }
  controller.noteOn(note, velocity);
}

void onNoteOff(byte channel, byte note, byte velocity) {
  controller.noteOff(note);
}

void onControlChange(byte channel, byte control, byte value) {
  if (control == CC_WAVEMORPH) {
    // CC value is 0..127, map to 0.0..(tableCount-1)
    // Access oscillator through controller for morphing
    int count = controller.getOscillator().getTableCount();
    if (count > 0) {
      float pos = (value / 127.0f) * (count - 1);
      controller.getOscillator().setMorphPos(pos);
    }
    Serial.print("CC ");
    Serial.print(control);
    Serial.print(" = ");
    Serial.println(value);
  }
}

void onPitchChange(byte channel, int bend) {
  // bend may be signed (-8192..8191) or unsigned (0..16383).
  controller.setPitchBend(bend);
  Serial.print("Pitch Bend = ");
  Serial.println(bend);
}
