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

constexpr size_t SINE_SIZE = 4096;
static float SINE[SINE_SIZE + 1];

static bool _sine_initd = []() -> bool {
    for (size_t i = 0; i < SINE_SIZE; ++i)
        SINE[i] = sinf(float(i) * math::Constants<float>::twoPi / 4096.0);

    // For linear interpolation
    SINE[SINE_SIZE] = SINE[0];

    return true;
}();

float sineLUT(float p)
{
    const float pp = p * SINE_SIZE;
    const int k = (int)pp;
    const float frac = pp - (float)k;
    return math::lerp(SINE[k], SINE[k+1], frac);
}
/*
float FmVoice::fmProcess(FmOp* op)
{
    op->phase += op->phaseInc;

    if (op->modulator != nullptr)
        op->phase += fmProcess(op->modulator);
    

    // ~fmodf
    while (op->phase > 1.0f)
        op->phase -= 1.0f;

    // ! Due to precision phase may turn out negative
    if (op->phase < 0.0f)
        op->phase = 0.0f;

    return op->amplitude * sineLUT(op->phase);
}
*/
//==============================================================================

FmVoice::FmVoice()
{
}

void FmVoice::trigger (int note, int velocity)
{
    Voice::trigger(note, velocity);

    m_operator[0].phaseInc = DPHASE[note];
    m_operator[0].aeg.trigger({0.0f, 10.0f, 0.0f, 1.0f}, globals::SAMPLE_RATE);

    m_operator[1].phaseInc = 14.0f * DPHASE[note];
    m_operator[1].aeg.trigger({0.0f, 6.0f, 0.2f, 0.5f}, globals::SAMPLE_RATE);

    m_operator[2].phaseInc = DPHASE[note];
    m_operator[2].aeg.trigger({0.0f, 10.0f, 0.0f, 1.0f}, globals::SAMPLE_RATE);

    m_operator[3].phaseInc = 1.0f * DPHASE[note];
    m_operator[3].aeg.trigger({0.0f, 4.0f, 0.3f, 0.5f}, globals::SAMPLE_RATE);

    m_operator[4].phaseInc = DPHASE[note];
    m_operator[4].aeg.trigger({0.0f, 3.0f, 0.0f, 1.0f}, globals::SAMPLE_RATE);

    m_operator[5].phaseInc = DPHASE[note];
    m_operator[5].aeg.trigger({0.0f, 1.0f, 0.0f, 0.0f}, globals::SAMPLE_RATE);

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
}

void FmVoice::process(float* outL, float* outR, size_t numFrames)
{
    for (size_t i = 0; i < numFrames; ++i) {
        float l, r;
        tick(l, r);

        outL[i] = l;
        outR[i] = r;
    }
}

bool FmVoice::shouldRecycle()
{
    return m_operator[0].aeg.state() == Envelope::State::Off
        && m_operator[2].aeg.state() == Envelope::State::Off
        && m_operator[4].aeg.state() == Envelope::State::Off;
}

void FmVoice::tick(float& l, float &r)
{
    constexpr float s = 0.0078125f;

    float a = m_operator[0].tick(s * m_operator[1].tick());
    float b = m_operator[2].tick(s * m_operator[3].tick());
    float c = m_operator[4].tick(s * m_operator[5].tick(s * m_operator[5].value));

    l = 0.04f * a + 0.06f * b + 0.01f * c;
    r = 0.06f * a + 0.04f * b + 0.01f * c;
}

//==============================================================================

FmInstrument::FmInstrument()
    : Parent(NUM_PARAMS)
    , m_lowPass()
    , m_distortion()
    , m_delay()
    , m_reverb()
{
    effects().append(&m_lowPass);
    effects().append(&m_distortion);
    effects().append(&m_delay);
    effects().append(&m_reverb);

    m_lowPass.parameters()[fx::LowPass::FREQUENCY].setValue(3000.0f, true);
    m_lowPass.init();

    m_distortion.parameters()[fx::Distortion::DRY].setValue(0.8f, true);
    m_distortion.parameters()[fx::Distortion::WET].setValue(0.2f, true);
    m_distortion.parameters()[fx::Distortion::GAIN].setValue(6.0f, true);

    m_delay.parameters()[fx::Delay::FEEDBACK].setValue(0.25f, true);
    m_delay.parameters()[fx::Delay::DELAY].setValue(0.14563107f, true);
    m_delay.parameters()[fx::Delay::WET].setValue(0.5f, true);

    m_reverb.parameters()[fx::Reverb::WET].setValue(0.4f, true);
    m_reverb.parameters()[fx::Reverb::ROOM_SIZE].setValue(0.75f, true);
    m_reverb.parameters()[fx::Reverb::WIDTH].setValue(1.0f, true);
    m_reverb.parameters()[fx::Reverb::PITCH].setValue(1.0f, true);
    m_reverb.parameters()[fx::Reverb::FEEDBACK].setValue(0.0f, true);
}
