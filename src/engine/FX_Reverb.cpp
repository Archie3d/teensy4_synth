#include "engine/FX_Reverb.h"

namespace fx {

Reverb::Reverb()
    : Effect(NUM_PARAMS)
{
    params[DRY].setValue (DefaultDry, 0.5f, true);
    params[WET].setValue (DefaultWet, 0.5f, true);
    params[ROOM_SIZE].setValue (DefaultRoomSize, 0.5f, true);
    params[DAMP].setValue (DefaultDamp, 0.5f, true);
    params[WIDTH].setValue (DefaultWidth, 0.5f, true);

    params[PITCH].setRange (0.0f, 2.0f);
    params[PITCH].setValue (DefaultPitch, 0.5f, true);

    params[FEEDBACK].setRange (0.0f, 1.0f);
    params[FEEDBACK].setValue (DefaultFeedback, 0.5f, true);

    reverbLSpec.roomsize = DefaultRoomSize;
    reverbLSpec.damp = DefaultDamp;

    reverbRSpec.roomsize = DefaultRoomSize;
    reverbRSpec.damp = DefaultDamp;

    init();
}

void Reverb::init()
{
    ReverbL::updateSpec (reverbLSpec);
    ReverbR::updateSpec (reverbRSpec);

    ReverbL::resetState (reverbLSpec, reverbLState);
    ReverbR::resetState (reverbRSpec, reverbRState);

    ::memset(m_mixBufL.data(), 0, sizeof(float) * m_mixBufL.size());
    ::memset(m_mixBufR.data(), 0, sizeof(float) * m_mixBufR.size());

    pitchShift.parameters()[PitchShift::DRY].setValue (0.0f, true);
    pitchShift.parameters()[PitchShift::WET].setValue (1.0f, true);
    pitchShift.parameters()[PitchShift::PITCH].setValue (params[PITCH].value(), true);
}

void Reverb::process (const float *inL, const float *inR, float *outL, float *outR, size_t numFrames)
{
    updateParams();

    float* tmpL = m_mixBufL.data();
    float* tmpR = m_mixBufR.data();

    const auto pitch = params[PITCH].value();
    auto feedback = params[FEEDBACK].value();

    if (feedback > 0.0f && pitch != 1.0f) {
        // Shimmer reverb
        pitchShift.parameters()[PitchShift::PITCH].setValue (pitch, true);
        pitchShift.process (tmpL, tmpR, tmpL, tmpR, numFrames);
        
        for (size_t i = 0; i < numFrames; ++i) {
            tmpL[i] = inL[i] + feedback * tmpL[i];
            tmpR[i] = inR[i] + feedback * tmpR[i];
        }        

        ReverbL::process(reverbLSpec, reverbLState, tmpL, tmpL, numFrames);
        ReverbR::process(reverbRSpec, reverbRState, tmpR, tmpR, numFrames);
    
    } else {
        // Normal reverb
        ReverbL::process(reverbLSpec, reverbLState, inL, tmpL, numFrames);
        ReverbR::process(reverbRSpec, reverbRState, inR, tmpR, numFrames);
    }
    
    // Dry/wet mixing
    for (size_t i = 0; i < numFrames; ++i) {
        // Advance smoothed parameters
        params[PITCH].nextValue();
        params[FEEDBACK].nextValue();

        const auto width = 1.0f; //params[WIDTH].nextValue();
        const auto dry = params[DRY].nextValue();
        const auto wet = params[WET].nextValue();
        const auto wet1 = wet * (width * 0.5f + 0.5f);
        const auto wet2 = wet * (0.5f * (1.0f - width));

        outL[i] = tmpL[i] * wet1 + tmpR[i] * wet2 + inL[i] * dry;
        outR[i] = tmpR[i] * wet1 + tmpL[i] * wet2 + inR[i] * dry;
    }
}

void Reverb::updateParams()
{
    reverbLSpec.roomsize = params[ROOM_SIZE].target();
    reverbLSpec.damp = params[DAMP].target();

    reverbRSpec.roomsize = params[ROOM_SIZE].target();
    reverbRSpec.damp = params[DAMP].target();

    ReverbL::updateSpec(reverbLSpec);
    ReverbR::updateSpec(reverbRSpec);
}

} // namespace fx
