#pragma once

#include <array>
#include <Arduino.h>

#include "engine/FastList.h"
#include "engine/Parameter.h"

/**
 * @brief Abstract voice.
 * 
 * This class must be specialized for a specific voices.
 */
class Voice
{
public:
    Voice()
        : params(nullptr)
        , m_key(0)
        , m_velocity(0)
    {
    }

    virtual ~Voice() = default;

    void setParametersPool(ParameterPool* p)
    {
        params = p;
    }

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

protected:

    // Shared parameters
    ParameterPool* params;

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

    void setParametersPool(ParameterPool* p)
    {
        // Shared parameters accessible from all the voices
        for (auto& v : m_voices)
            v.setParametersPool(p);
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