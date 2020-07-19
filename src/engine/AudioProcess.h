#pragma once

#include <AudioStream.h>
#include "engine/Globals.h"
#include "engine/Engine.h"

class AudioProcess : public AudioStream
{
public:
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
    float m_audioBuffer[globals::AUDIO_BLOCK_SIZE * 2]; // Stereo audio buffer
    float m_dspLoadPercent;
    float m_amplitudeL;
    float m_amplitudeR;
};
