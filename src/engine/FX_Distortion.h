#pragma once

#include "engine/DSP.h"
#include "engine/Effect.h"

namespace fx {

class Distortion : public Effect
{
public:

    enum Params
    {
        DRY = 0,
        WET,
        GAIN,

        NUM_PARAMS
    };

    Distortion();

    void process(const float *inL, const float *inR, float *outL, float *outR, size_t numFrames) override;
};

} // namespace fx
