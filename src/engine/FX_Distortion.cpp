#include "engine/FX_Distortion.h"

namespace fx {

inline static float distort(float x)
{
    return x / (1.0f + x*x);
}

Distortion::Distortion()
    : Effect (NUM_PARAMS)
{
    params[DRY].setValue(0.0f, true);
    params[WET].setValue(1.0f, true);
    params[GAIN].setRange(1.0f, 10.0f);
    params[GAIN].setValue(1.0f, true);
}

void Distortion::process (const float *inL, const float *inR, float *outL, float *outR, size_t numFrames)
{
    float dry = params[DRY].value();
    float wet = params[WET].value();
    float gain = params[GAIN].nextValue();

    while ((params[DRY].isSmoothing() || params[WET].isSmoothing() || params[GAIN].isSmoothing()) && numFrames > 0)
    {
        dry = params[DRY].nextValue();
        wet = params[WET].nextValue();
        gain = params[GAIN].nextValue();

        const float l = *(inL++);
        const float r = *(inR++);

        *(outL++) = dry * l + wet * distort(math::clamp(-1.0f, 1.0f, l * gain));
        *(outR++) = dry * r + wet * distort(math::clamp(-1.0f, 1.0f, r * gain));

        --numFrames;
    }

    while (numFrames > 0) {
        const float l = *(inL++);
        const float r = *(inR++);
        *(outL++) = dry * l + wet * distort(math::clamp(-1.0f, 1.0f, l * gain));
        *(outR++) = dry * r + wet * distort(math::clamp(-1.0f, 1.0f, r * gain));
        --numFrames;
    }
}

} // namespace fx
