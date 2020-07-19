#pragma once

#include "engine/DSP.h"
#include "engine/Effect.h"

namespace fx {
/**
 * @brief Simple delay effect with feedback.
 */
class Delay : public Effect
{
public:

    enum Params
    {
        DRY = 0,
        WET,
        DELAY,
        MAXDELAY,
        FEEDBACK,

        NUM_PARAMS
    };

    Delay();

    void init();

    void process(const float *inL, const float *inR, float *outL, float *outR, size_t numFrames) override;

private:

    dsp::DelayLine delayL;
    dsp::DelayLine delayR;

    float delayToSampleIndex;
};


} // namespace fx
