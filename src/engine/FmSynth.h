#pragma once

#include <array>

#include "engine/Voice.h"
#include "engine/Envelope.h"
#include "engine/Instrument.h"

#include "engine/FX_LowPass.h"
#include "engine/FX_Distortion.h"
#include "engine/FX_Delay.h"
#include "engine/FX_Reverb.h"

float sineLUT(float p);

class FmVoice : public Voice,
                public ListItem<FmVoice>
{
public:
    struct FmOp
    {
        float phase = 0.0f;
        float phaseInc = 0.0f;

        Envelope aeg;

        float value = 0.0f;

        inline float tick(float pmod = 0.0f)
        {
            phase += phaseInc + pmod;
   
            // ~fmodf
            while (phase > 1.0f)
                phase -= 1.0f;

            // ! Due to precision phase may turn out negative
            if (phase < 0.0f)
                phase = 0.0f;

            value = aeg.next() * sineLUT(phase);
            return value;
        }
    };

    static float fmProcess(FmOp* op);

    FmVoice();

    void trigger(int note, int velocity) override;
    void release() override;
    void reset() override;
    void process(float* outL, float* outR, size_t numFrames) override;
    bool shouldRecycle() override;

private:

    void tick(float& l, float& r);

    Envelope m_adsr;

    constexpr static size_t NUM_OPS = 6;
    FmOp m_operator[NUM_OPS];
};

//==============================================================================

class FmInstrument : public Instrument<FmVoice, 32>
{
public:

    enum Params
    {
        ADSR_ATTACK = 0,
        ADSR_DECAY,
        ADSR_RELEASE,
        ADSR_SUSTAIN,

        NUM_PARAMS
    };

    FmInstrument();

private:

    using Parent = Instrument<FmVoice, 32>;

    fx::LowPass m_lowPass;
    fx::Distortion m_distortion;
    fx::Delay m_delay;
    fx::Reverb m_reverb;
};
