#pragma once

#include <array>
#include <Arduino.h>

#include "engine/FastList.h"

/**
 * @brief Abstract voice.
 * 
 * This class must be specialized for a specific voices.
 */
class Voice
{
public:
    Voice() {}
    virtual ~Voice() = default;

    virtual void trigger(int key, int velocity)
    {
        m_key = key;
        m_velocity = velocity;
    }

    int key() const noexcept { return m_key; }

    virtual void release() = 0;
    virtual void reset() = 0;
    virtual void process(float* outL, float* outR, size_t numFrames) = 0;
    virtual bool shouldRecycle() = 0;

private:
    int m_key;
    int m_velocity;
};

//==============================================================================

template <class VoiceType, size_t PoolSize>
class VoicePool final
{
public:

    constexpr static size_t size = PoolSize;

    VoicePool()
        : m_voices()
    {
        for (auto& voice : m_voices)
            m_idleVoices.append(&voice);
    }

    VoiceType* trigger(int note, int velocity)
    {
        if (auto* voice = m_idleVoices.first())
        {
            m_idleVoices.remove(voice);
            voice->trigger(note, velocity);
            return voice;
        }

        return nullptr;
    }

    void recycle (VoiceType* voice)
    {
        voice->reset();
        m_idleVoices.append(voice);
    }

private:
    std::array<VoiceType, size> m_voices;
    List<VoiceType> m_idleVoices;
};