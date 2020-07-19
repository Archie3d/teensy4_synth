#include "engine/FX_Delay.h"

namespace fx {

Delay::Delay()
    : Effect(NUM_PARAMS)
    , delayL()
    , delayR()
    , delayToSampleIndex(0.0f)
{
    params[DRY].setValue(1.0f, true);
    params[WET].setValue(0.5f, true);
    params[DELAY].setRange(0.0f, 10.0f);
    params[MAXDELAY].setValue(5.0f, true);
    params[FEEDBACK].setValue(0.5f, true);

    init();
}

void Delay::init()
{
    const auto maxDelaySamples = (int)ceilf(globals::SAMPLE_RATE * params[MAXDELAY].target());

    delayL.resize(maxDelaySamples);
    delayR.resize(maxDelaySamples);

    delayToSampleIndex = (float)delayL.size() / params[MAXDELAY].value();
}

void Delay::process(const float *inL, const float *inR, float *outL, float *outR, size_t numFrames)
{
    for (size_t i = 0; i < numFrames; ++i) {
        const auto dry = params[DRY].nextValue();
        const auto wet = params[WET].nextValue();
        const auto delay = params[DELAY].nextValue() * delayToSampleIndex;
        const auto fb = params[FEEDBACK].nextValue();
        const auto l = delayL.read(delay);
        const auto r = delayR.read(delay);

        delayL.write(l * fb + inL[i]);
        delayR.write(r * fb + inR[i]);
        outL[i] = l * wet + inL[i] * dry;
        outR[i] = r * wet + inR[i] * dry;
    }
}

} // namespace fx
