#include "engine/FX_LowPass.h"

namespace fx {

static void updateFilter (dsp::BiquadFilter::Spec& spec, float f, float q)
{
    spec.freq = f;
    spec.q = q;
    dsp::BiquadFilter::updateSpec (spec);
}

LowPass::LowPass()
    : Effect (NUM_PARAMS)
{
    filterSpec.type = dsp::BiquadFilter::LowPass;

    params[FREQUENCY].setRange (1.0f, 20000.0f);
    params[FREQUENCY].setValue (15000.0f, true);

    params[Q_FACTOR].setRange (0.0f, 10.0f);
    params[Q_FACTOR].setValue (0.7071f, true);

    init();
}

void LowPass::init()
{
    filterSpec.sampleRate = globals::SAMPLE_RATE;
    updateFilter(filterSpec, params[FREQUENCY].value(), params[Q_FACTOR].value());

    dsp::BiquadFilter::resetState(filterSpec, filterL);
    dsp::BiquadFilter::resetState(filterSpec, filterR);
}

void LowPass::process (const float *inL, const float *inR, float *outL, float *outR, size_t numFrames)
{
    float f = params[FREQUENCY].value();
    float q = params[Q_FACTOR].value();

    while ((params[FREQUENCY].isSmoothing() || params[Q_FACTOR].isSmoothing()) && numFrames > 0)
    {
        f = params[FREQUENCY].nextValue();
        q = params[Q_FACTOR].nextValue();
        updateFilter (filterSpec, f, q);

        *(outL++) = dsp::BiquadFilter::tick(filterSpec, filterL, *(inL++));
        *(outR++) = dsp::BiquadFilter::tick(filterSpec, filterR, *(inR++));

        --numFrames;
    }

    dsp::BiquadFilter::process(filterSpec, filterL, inL, outL, numFrames);
    dsp::BiquadFilter::process(filterSpec, filterR, inR, outR, numFrames);
}

} // namespace fx
