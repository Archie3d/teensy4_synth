#include <cmath>
#include <Arduino.h>
#include "engine/FmSynth.h"

static float DPHASE[128];

static bool _dphase_initd = []() -> bool {
    for (size_t i = 0; i < 128; ++i) {
        const float f = (440.0f / 32.0f) * powf(2.0f, static_cast<float>(i) * (1.0f/12.0f));
        DPHASE[i] = f * globals::SAMPLE_RATE_R;
    }

    return true;
}();


float FmVoice::fmProcess(FmOp* op)
{
    op->phase += op->phaseInc;

    if (op->modulator != nullptr)
        op->phase += fmProcess(op->modulator);

    while (op->phase > 1.0f)
        op->phase -= 1.0f;

    return op->amplitude * sinf(op->phase * math::Constants<float>::twoPi);
}

//==============================================================================

FmVoice::FmVoice()
{
    m_operator[0].modulator = &m_operator[1];
}

void FmVoice::trigger (int note, int velocity)
{
    Voice::trigger(note, velocity);

    Envelope::Trigger env;
    env.attack = 0.01f;
    env.decay = 2.5f;
    env.sustain = 0.0f;
    env.release = 0.1f;
    m_adsr.trigger (env, globals::SAMPLE_RATE);

    m_operator[0].phaseInc = DPHASE[note];
    m_operator[0].amplitude = float(velocity) * (1.0f/127.0f);

    m_operator[1].phaseInc = 0.5f * DPHASE[note];
    m_operator[1].amplitude = 0.01f;
}

void FmVoice::release()
{
    m_adsr.release();
}

void FmVoice::reset()
{
    m_operator[0].phase = 0.0f;
    m_operator[1].phase = 0.0f;
}

void FmVoice::process(float* outL, float* outR, size_t numFrames)
{
    for (size_t i = 0; i < numFrames; ++i) {
        float x = 0.1f * fmProcess(&m_operator[0]) * m_adsr.next();
        
        outL[i] = x;
        outR[i] = x;
    }
}

bool FmVoice::shouldRecycle()
{
    return m_adsr.state() == Envelope::State::Off;    
}

//==============================================================================

FmInstrument::FmInstrument()
    : Parent(NUM_PARAMS)
    , m_delay()
    , m_reverb()
{
    effects().append(&m_delay);
    effects().append(&m_reverb);

    m_delay.parameters()[fx::Delay::FEEDBACK].setValue(0.6f, true);
    m_delay.parameters()[fx::Delay::DELAY].setValue(0.2f, true);
    m_delay.parameters()[fx::Delay::WET].setValue(0.75f, true);

    m_reverb.parameters()[fx::Reverb::ROOM_SIZE].setValue(0.9f, true);
    m_reverb.parameters()[fx::Reverb::PITCH].setValue(2.0f, true);
    m_reverb.parameters()[fx::Reverb::FEEDBACK].setValue(0.01f, true);
}