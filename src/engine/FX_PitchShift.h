#pragma once

#include "engine/DSP.h"
#include "engine/Effect.h"

namespace fx {

class PitchShift : public Effect
{
public:

    enum Params
    {
        DRY = 0,
        WET,
        PITCH,
        NUM_PARAMS
    };

    PitchShift();

    void process(const float *inL, const float *inR, float *outL, float *outR, size_t numFrames) override;

private:

    void init();

    void updateFilter();

    dsp::BiquadFilter::Spec filterSpec;
    dsp::BiquadFilter::State filterL;
    dsp::BiquadFilter::State filterR;

    dsp::DelayLine delayL;
    dsp::DelayLine delayR;

    dsp::DCBlocker::Spec dcBlockSpec;
    dsp::DCBlocker::State dcBlockL;
    dsp::DCBlocker::State dcBlockR;

    float dA;
    float dB;
    float w;    // Window constant.
};


} // namespace fx
