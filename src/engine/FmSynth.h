#pragma once

#include <array>

#include "engine/Voice.h"
#include "engine/Envelope.h"
#include "engine/Instrument.h"

#include "engine/FX_LowPass.h"
#include "engine/FX_Reverb.h"

class FmVoice : public Voice,
                public ListItem<FmVoice>
{
public:
    struct FmOp
    {
        float phase = 0;
        float phaseInc = 0;
        float amplitude = 1.0f; // TODO: use adsr here?

        FmOp* modulator = nullptr;
    };

    static float fmProcess(FmOp* op);

    FmVoice();

    void trigger(int note, int velocity) override;
    void release() override;
    void reset() override;
    void process(float* outL, float* outR, size_t numFrames) override;
    bool shouldRecycle() override;

private:
    Envelope m_adsr;
    FmOp m_operator[2];
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

    fx::Reverb m_reverb;
};
