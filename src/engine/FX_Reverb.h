#pragma once

#include <array>
#include "engine/DSP.h"
#include "engine/Effect.h"
#include "engine/FX_PitchShift.h"

namespace fx {

class Reverb : public Effect
{
public:

    enum Params
    {
        DRY = 0,
        WET,
        ROOM_SIZE,
        DAMP,
        WIDTH,
        PITCH,
        FEEDBACK,
        
        NUM_PARAMS
    };

    // Default parameters set on reverb creation
    constexpr static float DefaultDry      = 0.8f;
    constexpr static float DefaultWet      = 0.4f;
    constexpr static float DefaultRoomSize = 0.98f;
    constexpr static float DefaultDamp     = 0.2f;
    constexpr static float DefaultWidth    = 1.0f;
    constexpr static float DefaultPitch    = 2.0f;
    constexpr static float DefaultFeedback = 0.02f;

    static const char* Type;

    Reverb();

    void process(const float *inL, const float *inR, float *outL, float *outR, size_t numFrames) override;

private:

    void init();

    void updateParams();

    static constexpr int stereoSpread = 23;

    using ReverbL = dsp::Reverb<0>;
    using ReverbR = dsp::Reverb<stereoSpread>;

    ReverbL::Spec reverbLSpec;
    ReverbR::Spec reverbRSpec;

    ReverbL::State reverbLState;
    ReverbR::State reverbRState;

    std::array<float, globals::AUDIO_BLOCK_SIZE> m_mixBufL;
    std::array<float, globals::AUDIO_BLOCK_SIZE> m_mixBufR;

    PitchShift pitchShift;
};

} // namespace fx
