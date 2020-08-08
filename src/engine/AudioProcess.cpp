#include <Arduino.h>
#include <cmath>
#include "engine/Globals.h"
#include "engine/AudioProcess.h"

AudioProcess::AudioProcess()
    : AudioStream(0, nullptr)
    , m_audioEngine()
    , m_audioData { nullptr, nullptr }
    , m_dspLoadPercent(0.0f)
{
    globalInitialize();

    m_audioData[0] = allocate();
    m_audioData[1] = allocate();
}

AudioProcess::~AudioProcess()
{
    release(m_audioData[0]);
    release(m_audioData[1]);
}

int AudioProcess::numActiveVoices() const noexcept
{
    return m_audioEngine.numActiveVoices();
}

void AudioProcess::update()
{
    const auto beginUpdate_us = micros();

    float* outL = m_audioBuffer;
    float* outR = &m_audioBuffer[globals::AUDIO_BLOCK_SIZE];

    m_audioEngine.process(outL, outR, globals::AUDIO_BLOCK_SIZE);

    float maxL = 0.0f;
    float maxR = 0.0f;

    // Convert to 16-bit integer
    for (size_t i = 0; i < globals::AUDIO_BLOCK_SIZE; ++i) {
        const float l = math::clamp(-1.0f, 1.0f, outL[i]);
        const float r = math::clamp(-1.0f, 1.0f, outR[i]);

        maxL = std::max(maxL, fabsf(l));
        maxR = std::max(maxR, fabsf(r));

        m_audioData[0]->data[i] = (int16_t) (l * 32767.0f);
        m_audioData[1]->data[i] = (int16_t) (r * 32767.0f);
    }

    m_amplitudeL = maxL;
    m_amplitudeR = maxR;

    transmit(m_audioData[0], 0);
    transmit(m_audioData[1], 1);

    const auto endUpdate_us = micros();
    const float load = 100.0f * (endUpdate_us - beginUpdate_us) * globals::AUDIO_BLOCK_US_R;

    if (load > m_dspLoadPercent)
        m_dspLoadPercent = load;
    else
        m_dspLoadPercent = 0.9f * m_dspLoadPercent + 0.1f * load;
}

void AudioProcess::noteOn(int channel, int note, int velocity)
{
    //Serial.printf("noteOn %d\r\n", note);
    m_audioEngine.noteOn(channel, note, velocity);
}

void AudioProcess::noteOff(int channel, int note, int velocity)
{
    //Serial.printf("noteOff %d\r\n", note);
    m_audioEngine.noteOff(channel, note, velocity);
}

void AudioProcess::controlChange(int channel, int control, int value)
{
    //Serial.printf("CC %d = %d\r\n", control, value);
    m_audioEngine.controlChange(channel, control, value);
}

void AudioProcess::globalInitialize()
{
    static bool initialized = false;

    constexpr size_t globalNumBuffers = 8;
    static DMAMEM audio_block_t globalAudioData[globalNumBuffers];

    if (!initialized) {
        AudioStream::initialize_memory(globalAudioData, globalNumBuffers);
        initialized = true;
    }
}