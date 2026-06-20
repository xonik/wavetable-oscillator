#ifndef WAVETABLE_OSC_H
#define WAVETABLE_OSC_H

#include <Arduino.h>
#include <Audio.h>
#include <math.h>

struct OscProfileSnapshot {
  uint32_t blockCount;
  uint64_t totalCycles;
  uint32_t maxCyclesPerBlock;
};

// ---------------------------------------------------------------------------
// WavetableOsc - Core wavetable oscillator with wavetable morphing
// Operates on raw frequency input (pitch bend calculations handled externally)
// ---------------------------------------------------------------------------

class WavetableOsc : public AudioStream {
public:
  static constexpr float SAMPLE_RATE = AUDIO_SAMPLE_RATE_EXACT;

  WavetableOsc()
      : AudioStream(0, nullptr), tableBank(nullptr), tableCount(0),
        tableSize(1024), tableSizeMask(1023), phase(0), increment(0),
        morphPos(0.0f), morphTargetPos(0.0f), active(false), velocity(1.0f),
        profileBlockCount(0), profileTotalCycles(0), profileMaxCycles(0) {}

  void setWavetables(const int16_t **tables, int count, int size) {
    __disable_irq();
    tableBank = tables;
    tableCount = count;
    // Ensure size is power of two for bitmask
    tableSize = size;
    tableSizeMask = size - 1;
    // Reset morphPos indices on table change
    morphPos = constrain(morphPos, 0.0f, (float)(tableCount - 1));
    morphTargetPos = constrain(morphTargetPos, 0.0f, (float)(tableCount - 1));
    __enable_irq();
  }

  int getTableCount() const { return tableCount; }

  void setFrequency(float freq) {
    increment = (uint32_t)((freq / SAMPLE_RATE) * tableSize * 65536.0f);
  }

  void setActive(bool shouldBeActive) {
    __disable_irq();
    active = shouldBeActive;
    __enable_irq();
  }

  void setVelocity(float vel) {
    __disable_irq();
    velocity = vel;
    __enable_irq();
  }

  void setMorphPos(float pos) {
    __disable_irq();
    morphTargetPos = constrain(pos, 0.0f, (float)(tableCount - 1));
    __enable_irq();
  }

  OscProfileSnapshot consumeProfileSnapshot() {
    __disable_irq();
    OscProfileSnapshot snapshot = {
        profileBlockCount,
        profileTotalCycles,
        profileMaxCycles,
    };
    profileBlockCount = 0;
    profileTotalCycles = 0;
    profileMaxCycles = 0;
    __enable_irq();
    return snapshot;
  }

  virtual void update() override {
    audio_block_t *block = allocate();
    if (!block || !tableBank) {
      return;
    }

    const uint32_t startCycles = ARM_DWT_CYCCNT;

    // Smooth morphPos towards target
    static constexpr float MORPH_ALPHA = 0.1f;
    morphPos += (morphTargetPos - morphPos) * MORPH_ALPHA;

    if (!active) {
      memset(block->data, 0, sizeof(block->data));
      transmit(block, 0);
      release(block);
      recordProfileBlock(startCycles);
      return;
    }

    float pos = morphPos;
    int idxA = (int)pos;
    float blend = pos - idxA;
    int idxB = idxA + 1;

    idxA = constrain(idxA, 0, tableCount - 1);
    idxB = constrain(idxB, 0, tableCount - 1);

    const int16_t *tableA = tableBank[idxA];
    const int16_t *tableB = tableBank[idxB];
    float vel = velocity;

    if (blend < 0.001f) {
      for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
        uint32_t idx = (phase >> 16) & tableSizeMask;
        block->data[i] = (int16_t)(tableA[idx] * vel);
        phase += increment;
      }
    } else {
      for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
        uint32_t idx = (phase >> 16) & tableSizeMask;
        float s = tableA[idx] + (tableB[idx] - tableA[idx]) * blend;
        block->data[i] = (int16_t)(s * vel);
        phase += increment;
      }
    }

    transmit(block, 0);
    release(block);
    recordProfileBlock(startCycles);
  }

private:
  void recordProfileBlock(uint32_t startCycles) {
    uint32_t elapsedCycles = ARM_DWT_CYCCNT - startCycles;
    profileBlockCount++;
    profileTotalCycles += elapsedCycles;
    if (elapsedCycles > profileMaxCycles) {
      profileMaxCycles = elapsedCycles;
    }
  }

  const int16_t **tableBank;
  volatile int tableCount;
  volatile int tableSize;
  volatile uint32_t tableSizeMask;
  uint32_t phase;
  uint32_t increment;
  volatile float morphPos;
  volatile float morphTargetPos;
  volatile bool active;
  volatile float velocity;
  volatile uint32_t profileBlockCount;
  volatile uint64_t profileTotalCycles;
  volatile uint32_t profileMaxCycles;
};

#endif
