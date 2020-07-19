#pragma once

#include "engine/DSP.h"
#include "engine/Effect.h"

namespace fx {

class LowPass : public Effect
{
public:

    enum Params
    {
        FREQUENCY = 0,
        Q_FACTOR,

        NUM_PARAMS
    };

    LowPass();

    void init();

    void process(const float *inL, const float *inR, float *outL, float *outR, size_t numFrames) override;

private:

    dsp::BiquadFilter::Spec filterSpec;
    dsp::BiquadFilter::State filterL;
    dsp::BiquadFilter::State filterR;
};

} // namespace fx
