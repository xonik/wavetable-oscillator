#include <Arduino.h>
#include <Audio.h>

#include "midi_handler.h"
#include "wavetable_controller.h"
#include "wavetable_osc.h"
#include "wavetables.h"

// Forward declarations of MIDI callbacks defined in midi_callbacks.cpp
void onNoteOn(byte channel, byte note, byte velocity);
void onNoteOff(byte channel, byte note, byte velocity);
void onControlChange(byte channel, byte control, byte value);
void onPitchChange(byte channel, int bend);

// ---------------------------------------------------------------------------
// Audio routing and instantiation
// ---------------------------------------------------------------------------

WavetableOsc osc;
WavetableController controller(osc);
AudioOutputI2S i2sOut;
AudioConnection patchL(osc, 0, i2sOut, 0);
AudioConnection patchR(osc, 0, i2sOut, 1);

// ---------------------------------------------------------------------------
// Wavetable bank management
// ---------------------------------------------------------------------------

static const int MAX_WAVETABLES = 128;
const int16_t *wavetableBank[MAX_WAVETABLES];
static int numTables = 0;

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);

  // Generate all wavetables
  generateWavetables();

  // Populate wavetable bank
  wavetableBank[0] = waveTable0;
  wavetableBank[1] = waveTable1;
  wavetableBank[2] = waveTable2;
  wavetableBank[3] = waveTable3;
  wavetableBank[4] = waveTable4;
  wavetableBank[5] = waveTable5;
  wavetableBank[6] = waveTable6;
  wavetableBank[7] = waveTable7;
  numTables = 8;

  // Initialize audio memory
  AudioMemory(8);

  // Configure oscillator
  osc.setWavetables(wavetableBank, numTables, TABLE_SIZE);
  controller.initializeFrequency();
  osc.setMorphPos(0.0f);

  // Register MIDI callbacks
  usbMIDI.setHandleNoteOn(onNoteOn);
  usbMIDI.setHandleNoteOff(onNoteOff);
  usbMIDI.setHandleControlChange(onControlChange);
  usbMIDI.setHandlePitchChange(onPitchChange);
}

// ---------------------------------------------------------------------------
// Main loop
// ---------------------------------------------------------------------------

void loop() {
  // Process incoming MIDI messages
  usbMIDI.read();
}