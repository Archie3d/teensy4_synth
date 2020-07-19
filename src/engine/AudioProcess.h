#pragma once

#include <AudioStream.h>
#include "engine/Engine.h"

class AudioProcess : public AudioStream
{
public:

    constexpr static float  sampleRate         = AUDIO_SAMPLE_RATE;
    constexpr static float  sampleRate_r       = 1.0f / sampleRate;
    constexpr static size_t audioBlockSize     = AUDIO_BLOCK_SAMPLES;
    constexpr static float  audioBlockMicros   = 10e6f * float (AUDIO_BLOCK_SAMPLES) / AUDIO_SAMPLE_RATE;
    constexpr static float  audioBlockMicros_r = 1.0f / audioBlockMicros;

    AudioProcess();
    ~AudioProcess();

    float dspLoadPercent() const noexcept { return m_dspLoadPercent; }
    int numActiveVoices() const noexcept;
    float amplitudeL() const noexcept { return m_amplitudeL; }
    float amplitudeR() const noexcept { return m_amplitudeR; }

    void update() override;

    //
    void noteOn(int channel, int note, int velocity);
    void noteOff(int channel, int node, int velocity);
    void controlChange(int channel, int control, int value);

private:

    static void globalInitialize();

    Engine m_audioEngine;

    audio_block_t* m_audioData[2];
    float m_audioBuffer[audioBlockSize * 2]; // Stereo audio buffer
    float m_dspLoadPercent;
    float m_amplitudeL;
    float m_amplitudeR;
};
