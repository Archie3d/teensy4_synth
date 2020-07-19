#pragma once

#include <array>
#include <bitset>
#include <atomic>
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
    }

    virtual ~Instrument() = default;

    int numActiveVoices() const noexcept { return m_numActiveVoices; }

    void process(float* outL, float* outR, size_t numFrames)
    {
        auto* voice = m_activeVoices.first();

        if (voice != nullptr) {
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

        while (voice != nullptr) {
            voice->process(m_mixBufL.data(), m_mixBufR.data(), numFrames);

            // Mix
            for (size_t i = 0; i < numFrames; ++i) {
                outL[i] += m_mixBufL[i];
                outR[i] += m_mixBufR[i];
            }

            if (voice->shouldRecycle()) {
                auto* nextVoice = m_activeVoices.removeAndReturnNext(voice);
                m_numActiveVoices -=1;
                m_voicePool.recycle(voice);
                voice = nextVoice;
            } else {
                voice = voice->next();
            }
        }

        // TODO: pass via mix buffer?
        m_effects.process(outL, outR, outL, outR, numFrames);
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

    EffectChain& effects() { return m_effects; }

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

    std::array<float, globals::AUDIO_BLOCK_SIZE> m_mixBufL;
    std::array<float, globals::AUDIO_BLOCK_SIZE> m_mixBufR;

};
