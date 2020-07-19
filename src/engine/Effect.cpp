#include "engine/Effect.h"

Effect::Effect(size_t numParams)
    : params(numParams)
{
}

//==============================================================================

EffectChain::EffectChain()
{
}

void EffectChain::append(Effect* fx)
{
    m_effects.push_back(fx);
}

void EffectChain::process(const float* inL, const float* inR,
                          float* outL, float* outR, size_t numFrames)
{
    if (m_effects.size() == 0) {
        /* Empty chain */
        if (inL != outL)
            ::memcpy(outL, inL, sizeof(float) * numFrames);
        if (inR != outR)
            ::memcpy(outR, inR, sizeof(float) * numFrames);        
    } else if (m_effects.size() == 1) {
        /* Single effect */
        m_effects.front()->process(inL, inR, outL, outR, numFrames);
    } else {
        const float* inBufL = inL;
        const float* inBufR = inR;

        const bool evenNumberOfEffects = (0 == m_effects.size() % 2);

        float* outBufL = outL;
        float* outBufR = outR;
        float* outNextBufL = m_mixBufL.data();
        float* outNextBufR = m_mixBufR.data();

        if (evenNumberOfEffects) {
            std::swap (outBufL, outNextBufL);
            std::swap (outBufR, outNextBufR);
        }

        auto it = m_effects.begin();
        (*it)->process (inBufL, inBufR, outBufL, outBufR, numFrames);
        ++it;

        // This will end up with final effect outputing to the target buffer
        while (it != m_effects.end())
        {
            (*it)->process (outBufL, outBufR, outNextBufL, outNextBufR, numFrames);

            std::swap (outBufL, outNextBufL);
            std::swap (outBufR, outNextBufR);

            ++it;
        }
    }
}

void EffectChain::reset()
{
    for (auto* fx : m_effects)
        fx->reset();
}