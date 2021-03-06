#include <cmath>
#include <Arduino.h>
#include "engine/FmSynth.h"

#include "lut_sine.inc"
#include "lut_dphase.inc"
#include "lut_velocity.inc"

#define LUT_INTERPOLATE 0

float sineLUT(float p)
{
    const float pp = p * float (SINE_SIZE);
    const int k = (int)pp;

#if LUT_INTERPOLATE
    const float frac = pp - (float)k;
    return math::lerp(SINE[k], SINE[k+1], frac);
#else
    return SINE[k];
#endif
}

//==============================================================================

constexpr float modulationFrequency = 7.0f; // [Hz]
constexpr float modulationDepth = 2e-4f;

FmVoice::FmVoice()
{
    // Prepare static envelopes
    m_operator[1].aeg.prepare({0.0f, 6.0f, 0.2f, 0.5f});
    m_operator[3].aeg.prepare({0.0f, 4.0f, 0.3f, 0.5f});
    m_operator[5].aeg.prepare({0.0f, 1.0f, 0.0f, 0.0f});
}

void FmVoice::trigger (int note, int velocity)
{
    Voice::trigger(note, velocity);

    m_gain = 0.2f + 0.8f * VELOCITY_CURVE[velocity];

    m_modPhase = 0.0f;

    const float v = float(velocity) * (1.0f / 127.0f);
    const float attack = 0.1f / (1.0f + 500.0f * v);
    const float decay = 3.0f + 7.0f * v;

    const float dp = DPHASE[note];

    m_operator[0].phaseInc = dp;
    m_operator[0].aeg.trigger({attack, 0.5f * decay, 0.0f, 0.25f});

    m_operator[1].phaseInc = 14.0f * dp;
    m_operator[1].aeg.trigger();

    m_operator[2].phaseInc = dp;
    m_operator[2].aeg.trigger({attack, decay, 0.0f, 0.25f});

    m_operator[3].phaseInc = 1.0f * dp;
    m_operator[3].aeg.trigger();

    m_operator[4].phaseInc = dp;
    m_operator[4].aeg.trigger({attack, 3.0f, 0.0f, 0.25f});

    m_operator[5].phaseInc = dp;
    m_operator[5].aeg.trigger();
}

void FmVoice::release()
{
    for (size_t i = 0; i < NUM_OPS; ++i)
        m_operator[i].aeg.release();
}

void FmVoice::reset()
{
    for (size_t i = 0; i < NUM_OPS; ++i) {
        m_operator[i].phase = 0.0f;
        m_operator[i].value = 0.0f;
    }

    m_modPhase = 0.0f;
}

void FmVoice::process(float* outL, float* outR, size_t numFrames)
{
    // Tone
    constexpr float s = 0.0078125f;
    const float tone = 2.0f * s * (*params)[FmInstrument::TONE].value();

    // Modulation
    const float modulation = modulationDepth * (*params)[FmInstrument::MODULATION].value();

    for (size_t i = 0; i < numFrames; ++i) {
        const float m = modulation * sineLUT(m_modPhase);

        float a = m_operator[0].tick(tone * (m_operator[1].tick()) + m);    
        float b = m_operator[2].tick(tone * (m_operator[3].tick()) + m);
        float c = m_operator[4].tick(tone * (m_operator[5].tick(s * m_operator[5].value)));
        
        // Update modulation phase
        constexpr float modInc = modulationFrequency * globals::SAMPLE_RATE_R;
        m_modPhase += modInc;

        while (m_modPhase > 1.0f)
            m_modPhase -= 1.0f;

        // Mix operators
        const float l = 0.04f * a + 0.06f * b + 0.01f * c;
        const float r = 0.06f * a + 0.04f * b + 0.01f * c;

        outL[i] += l * m_gain;
        outR[i] += r * m_gain;
    }
}

bool FmVoice::shouldRecycle()
{
    return m_operator[0].aeg.state() == Envelope::State::Off
        && m_operator[2].aeg.state() == Envelope::State::Off
        && m_operator[4].aeg.state() == Envelope::State::Off;
}

float FmVoice::envelopeLevel() const
{
    // Should take the slowest envelope here.
    return m_operator[0].aeg.level();
}

//==============================================================================

FmInstrument::FmInstrument()
    : PolyphonicInstrument(NUM_PARAMS)
    , m_reverb()
{
    effects().append(&m_reverb);

    parameters()[MODULATION].setValue(0.0f, true);
    parameters()[TONE].setValue(0.5f, true);

    mapCC(MidiMessage::CC_Modulation, MODULATION);
    mapCC(16, TONE);

    m_reverb.parameters()[fx::Reverb::DRY].setValue(1.0f, true);
    m_reverb.parameters()[fx::Reverb::WET].setValue(0.4f, true);
    m_reverb.parameters()[fx::Reverb::ROOM_SIZE].setValue(0.87f, true);
    m_reverb.parameters()[fx::Reverb::WIDTH].setValue(1.0f, true);
    m_reverb.parameters()[fx::Reverb::PITCH].setValue(1.0f, true);
    m_reverb.parameters()[fx::Reverb::FEEDBACK].setValue(0.0f, true);
}
