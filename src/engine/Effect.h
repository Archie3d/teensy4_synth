#pragma once

#include <array>
#include "engine/Globals.h"
#include "engine/Parameter.h"

class Effect
{
public:

    Effect(size_t numParams = 0);

    virtual void process(const float* inL, const float* inR,
                         float* outL, float* outR, size_t numFrames) = 0;

    virtual void reset() {}

    ParameterPool& parameters() { return params; }

protected:

    ParameterPool params;
};

//==============================================================================

class EffectChain
{
public:
    EffectChain();

    void append(Effect* fx);

    void process(const float* inL, const float* inR,
                 float* outL, float* outR, size_t numFrames);

    void reset();

private:
    std::vector<Effect*> m_effects;
    std::array<float, globals::AUDIO_BLOCK_SIZE> m_mixBufL;
    std::array<float, globals::AUDIO_BLOCK_SIZE> m_mixBufR;
};
