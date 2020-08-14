#pragma once

#include <array>
#include <bitset>
#include <atomic>
#include <map>
#include "engine/Globals.h"
#include "engine/Parameter.h"
#include "engine/MidiMessage.h"
#include "engine/Voice.h"
#include "engine/Effect.h"

template <class VoiceType, size_t Polyphony>
class Instrument
{
public:

    Instrument(size_t numParameters = 0)
        : m_parameters(numParameters)
    {
        m_sustained = false;
        m_numActiveVoices = 0;

        m_voicePool.setParametersPool(&m_parameters);
    }

    virtual ~Instrument() = default;

    int numActiveVoices() const noexcept { return m_numActiveVoices; }

    void process(float* outL, float* outR, size_t numFrames)
    {
        auto* voice = m_activeVoices.first();

        while (voice != nullptr) {
            voice->process(outL, outR, numFrames);

            if (voice->shouldRecycle()) {
                auto* nextVoice = m_activeVoices.removeAndReturnNext(voice);
                m_numActiveVoices -=1;
                m_voicePool.recycle(voice);
                voice = nextVoice;
            } else {
                voice = voice->next();
            }
        }

        m_effects.process(outL, outR, outL, outR, numFrames);

        updateParameters();
    }

    void processMidiMessage(const MidiMessage& msg)
    {
        switch (msg.type())
        {
            case MidiMessage::Type::NoteOn:
                if (msg.velocity() > 0)
                    noteOn(msg);
                else
                    noteOff(msg);
                break;
            case MidiMessage::Type::NoteOff:
                noteOff(msg);
                break;
            case MidiMessage::Type::ControlChange:
                controlChange(msg.cc(), msg.value());
                break;
            default:
                break;
        }
    }

    ParameterPool& parameters() { return m_parameters; }

    void mapCC(int cc, int param)
    {
        m_ccToParamMap[cc] = param;
    }

    EffectChain& effects() { return m_effects; }

protected:

    virtual void updateParameters()
    {
        // Advance all parameters
        for (size_t i = 0; i < m_parameters.size(); ++i)
            m_parameters[i].nextValue();
    }

private:

    void noteOn(const MidiMessage& msg)
    {
        m_keysState[msg.note()] = true;

        if (auto* voice = m_voicePool.trigger(msg.note(), msg.velocity())) {
            m_activeVoices.append(voice);
            m_numActiveVoices += 1;
        }
    }

    void noteOff(const MidiMessage& msg)
    {
        m_keysState[msg.note()] = false;

        if (m_sustained)
            return;

        auto* voice = m_activeVoices.first();

        while (voice != nullptr) {
            if (voice->key() == msg.note())
                voice->release();

            voice = voice->next();
        }
    }

    void controlChange(int control, int value)
    {        
        if (control == MidiMessage::CC_SustainPedal) {
            bool wasSustained = m_sustained;
            m_sustained = (value >= 64);

            if (wasSustained && (! m_sustained)) {
                releaseSustained();
            }
        }

        const auto it = m_ccToParamMap.find(control);

        if (it != m_ccToParamMap.end()) {
            // Set parameter tagret value. The actualt value will
            // be updated when updateParameters() gets called.
            const float v = float(value) * (1.0f / 127.0f);
            m_parameters[it->second].setValue(v);

        }
    }

    void releaseSustained()
    {
        auto* voice = m_activeVoices.first();

        while (voice != nullptr) {
            if (! m_keysState[voice->key()])
                voice->release();

            voice = voice->next();
        }
    }


    ParameterPool m_parameters;

    VoicePool<VoiceType, Polyphony> m_voicePool;
    List<VoiceType> m_activeVoices;

    std::atomic<int> m_numActiveVoices;

    std::bitset<128> m_keysState;
    bool m_sustained;

    EffectChain m_effects;

    std::map<int, int> m_ccToParamMap;
};
