#include <Arduino.h>
#include <Audio.h>

#include "classic_wavetables.h"
#include "midi_handler.h"
#include "prophet_vs_wavetables.h"
#include "wavetable_controller.h"
#include "wavetable_osc.h"

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
static uint32_t loopIterations = 0;
static uint32_t lastProfilerReportMs = 0;

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);

  // Enable DWT cycle counter for lightweight runtime profiling.
  ARM_DEMCR |= ARM_DEMCR_TRCENA;
  ARM_DWT_CTRL |= ARM_DWT_CTRL_CYCCNTENA;
  ARM_DWT_CYCCNT = 0;

  // Generate all wavetables
  generateClassicWavetables();

  // Populate wavetable bank
  wavetableBank[0] = classicWaveSine;
  wavetableBank[1] = classicWaveTriangle;
  wavetableBank[2] = classicWaveSaw;
  wavetableBank[3] = classicWaveSquare;
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
  loopIterations++;

  // Process incoming MIDI messages
  usbMIDI.read();

  uint32_t nowMs = millis();
  uint32_t elapsedMs = nowMs - lastProfilerReportMs;
  if (elapsedMs >= 1000) {
    OscProfileSnapshot profile = osc.consumeProfileSnapshot();

    float seconds = elapsedMs / 1000.0f;
    float loopsPerSecond = loopIterations / seconds;
    float blocksPerSecond = profile.blockCount / seconds;
    float avgCyclesPerBlock =
        (profile.blockCount > 0)
            ? ((float)profile.totalCycles / (float)profile.blockCount)
            : 0.0f;
    float oscCpuPercent =
        ((profile.totalCycles / seconds) / (float)F_CPU_ACTUAL) * 100.0f;

    Serial.print("Profiler: loops/s=");
    Serial.print(loopsPerSecond, 1);
    Serial.print(" blocks/s=");
    Serial.print(blocksPerSecond, 1);
    Serial.print(" avg_cycles/block=");
    Serial.print(avgCyclesPerBlock, 1);
    Serial.print(" max_cycles/block=");
    Serial.print(profile.maxCyclesPerBlock);
    Serial.print(" osc_cpu_est=%");
    Serial.println(oscCpuPercent, 2);

    loopIterations = 0;
    lastProfilerReportMs = nowMs;
  }
}