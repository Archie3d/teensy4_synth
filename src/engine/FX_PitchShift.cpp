#include "engine/FX_PitchShift.h"

namespace fx {

constexpr float MaxDelay = 0.035f; // [s]
constexpr float FilterOpenFreq = 22000.0f; // [Hz]

PitchShift::PitchShift()
    : Effect(NUM_PARAMS)
    , delayL()
    , delayR()
    , dA(0.0f)
    , dB(0.0f)
    , w(0.0f)
{
    params[DRY].setValue (0.0f, true);
    params[WET].setValue (1.0f, true);

    params[PITCH].setRange (0.0f, 4.0f);
    params[PITCH].setValue (1.0f, true);

    init();
}

void PitchShift::init()
{
    filterSpec.type = dsp::BiquadFilter::LowPass;
    filterSpec.q = 0.7071f;
    filterSpec.freq = FilterOpenFreq;
    filterSpec.sampleRate = globals::SAMPLE_RATE;
    updateFilter();

    dsp::BiquadFilter::resetState (filterSpec, filterL);
    dsp::BiquadFilter::resetState (filterSpec, filterR);

    dcBlockSpec.alpha = 0.995f;
    dsp::DCBlocker::resetState (dcBlockSpec, dcBlockL);
    dsp::DCBlocker::resetState (dcBlockSpec, dcBlockR);

    const auto maxDelaySamples = (size_t) (globals::SAMPLE_RATE * MaxDelay);

    delayL.resize (maxDelaySamples);
    delayR.resize (maxDelaySamples); 

    dA = 0.0f;
    dB = 0.5f * delayL.size();

    w = math::Constants<float>::pi / (float) delayL.size();
}

void PitchShift::process(const float *inL, const float *inR, float *outL, float *outR, size_t numFrames)
{
    if (params[PITCH].isSmoothing())
        updateFilter();

    for (size_t i = 0; i < numFrames; ++i)
    {
        const auto dry = params[DRY].nextValue();
        const auto wet = params[WET].nextValue();
        const auto p = 1.0f - params[PITCH].nextValue();

        delayL.write (dsp::BiquadFilter::tick (filterSpec, filterL,
                                               dsp::DCBlocker::tick(dcBlockSpec, dcBlockL, inL[i])));
        delayR.write (dsp::BiquadFilter::tick (filterSpec, filterR,
                                               dsp::DCBlocker::tick(dcBlockSpec, dcBlockR, inR[i])));

        const float wa = sinf(w * dA);
        const float wb = sinf(w * dB);

        const float l = 0.5f * (delayL.read(dA) * wa + delayL.read(dB) * wb);
        const float r = 0.5f * (delayR.read(dA) * wa + delayR.read(dB) * wb);

        dA += p;
        dB += p;

        if (dA < 0.0f)
            dA += delayL.size();
        else if (dA > delayL.size())
            dA -= delayL.size();

        if (dB < 0.0f)
            dB += delayR.size();
        else if (dB > delayR.size())
            dB -= delayR.size();


        outL[i] = l * wet + inL[i] * dry;
        outR[i] = r * wet + inR[i] * dry;
    }
}

void PitchShift::updateFilter()
{
    const auto p = params[PITCH].value();
    filterSpec.freq = p > 1.0f ? (0.5f * filterSpec.sampleRate / p) : FilterOpenFreq;
    dsp::BiquadFilter::updateSpec (filterSpec);
}

} // namespace fx
