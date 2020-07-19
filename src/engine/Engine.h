#pragma once

#include "engine/Globals.h"
#include "engine/MidiMessage.h"

#include "engine/FmSynth.h"

/**
 * Audio engine control class.
 */
class Engine final
{
public:
    // Helper structure to enable/disable audio interrupt
    struct AudioLock
    {
        static inline void enable()  { NVIC_ENABLE_IRQ(IRQ_SOFTWARE); }
        static inline void disable() { NVIC_DISABLE_IRQ(IRQ_SOFTWARE); }

        AudioLock()  { AudioLock::disable(); }
        ~AudioLock() { AudioLock::enable();  }
    };

    Engine();
    ~Engine();

    int numActiveVoices() const noexcept;

    void noteOn(int channel, int note, int velocity);
    void noteOff(int channel, int node, int velocity);
    void controlChange(int channel, int control, int value);

    void process(float* outL, float* outR, size_t numFrames);

private:

    void processMidi();
    void processMidiMessage(const MidiMessage& msg);

    MidiQueue<64> m_midiQueue;

    FmInstrument m_instrument;

};