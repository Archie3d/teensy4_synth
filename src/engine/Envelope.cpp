#include <cmath>
#include "engine/Envelope.h"

// Ported from
// https://www.earlevel.com/main/2013/06/03/envelope-generators-adsr-code/

const float Envelope::logAttackTR = -logf((1.0f + Envelope::AttackTargetRatio) / Envelope::AttackTargetRatio);
const float Envelope::logDecayReleaseTR = -logf((1.0f + Envelope::DecayReleaseTargetRatio) / Envelope::DecayReleaseTargetRatio);

Envelope::Envelope()
    : currentState (Off)
    , currentLevel (0.0f)
    , attackRate (0.0f)
    , attackCoef (0.0f)
    , attackBase (0.0f)
    , decayRate (0.0f)
    , decayCoef (0.0f)
    , decayBase (0.0f)
    , releaseRate (0.0f)
    , releaseCoef (0.0f)
    , releaseBase (0.0f)
    , sustainLevel (0.0f)
{
}

void Envelope::trigger(const Envelope::Trigger& trig)
{
    prepare(trig);
    trigger();
}

void Envelope::prepare(const Envelope::Trigger& trigger)
{
    sustainLevel = trigger.sustain;

    attackRate = trigger.attack * globals::SAMPLE_RATE;
    
    attackCoef = calculate2 (attackRate, logAttackTR);
    attackBase = (1.0f + AttackTargetRatio) * (1.0f - attackCoef);

    decayRate = trigger.decay * globals::SAMPLE_RATE;
    decayCoef = calculate2 (decayRate, logDecayReleaseTR);
    decayBase = (sustainLevel - DecayReleaseTargetRatio) * (1.0f - decayCoef);

    releaseRate = trigger.release * globals::SAMPLE_RATE;
    releaseCoef = calculate2 (releaseRate, logDecayReleaseTR);
    releaseBase = -DecayReleaseTargetRatio * (1.0f - releaseCoef);
}

void Envelope::trigger()
{
    currentState = Attack;
    currentLevel = 0.0f;
}

void Envelope::release()
{
    currentState = Release;
}

void Envelope::release(float t)
{
    releaseRate = t * globals::SAMPLE_RATE;
    releaseCoef = calculate2 (releaseRate, logDecayReleaseTR);
    releaseBase = -DecayReleaseTargetRatio * (1.0f - releaseCoef);

    currentState = Release;
}

float Envelope::next()
{
    switch (currentState)
    {
    case Off:
        break;
    case Attack:
        currentLevel = attackBase + currentLevel * attackCoef;

        if (currentLevel >= 1.0f) {
            currentLevel = 1.0f;
            currentState = Decay;
        }
        break;
    case Decay:
        currentLevel = decayBase + currentLevel * decayCoef;

        if (currentLevel <= sustainLevel) {
            currentLevel = sustainLevel;

            if (currentLevel > 0.0f)
                currentState = Sustain;
            else
                currentState = Off;
        }
        break;
    case Sustain:
        break;
    case Release:
        currentLevel = releaseBase + currentLevel * releaseCoef;

        if (currentLevel <= 0.0f) {
            currentLevel = 0.0f;
            currentState = Off;
        }
        break;
    default:
        break;
    }

    return currentLevel;
}

float Envelope::calculate(float rate, float targetRatio)
{
    return rate <= 0 ? 0.0f : expf(-logf((1.0f + targetRatio) / targetRatio) / rate);
}

float Envelope::calculate2(float rate, float logtr)
{
    if (rate <= 0.0f)
        return 0.0f;
    
    const float x = logtr / rate;

    // Polynomial approximation of exp(x)
    return 1.0f + x * (1.0f + 0.5 * x);
}