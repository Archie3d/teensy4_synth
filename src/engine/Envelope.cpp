#include <cmath>
#include "engine/Envelope.h"

// Ported from
// https://www.earlevel.com/main/2013/06/03/envelope-generators-adsr-code/

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

void Envelope::trigger (const Envelope::Trigger& trigger, float sampleRate)
{
    sustainLevel = trigger.sustain;

    attackRate = trigger.attack * sampleRate;
    attackCoef = calculate (attackRate, AttackTargetRatio);
    attackBase = (1.0f + AttackTargetRatio) * (1.0f - attackCoef);

    decayRate = trigger.decay * sampleRate;
    decayCoef = calculate (decayRate, DecayReleaseTargetRatio);
    decayBase = (sustainLevel - DecayReleaseTargetRatio) * (1.0f - decayCoef);

    releaseRate = trigger.release * sampleRate;
    releaseCoef = calculate (releaseRate, DecayReleaseTargetRatio);
    releaseBase = -DecayReleaseTargetRatio * (1.0f - releaseCoef);

    currentState = Attack;
    currentLevel = 0.0f;
}

void Envelope::release()
{
    currentState = Release;
}

void Envelope::release (float t, float sampleRate)
{
    releaseRate = t * sampleRate;
    releaseCoef = calculate (releaseRate, DecayReleaseTargetRatio);
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

        if (currentLevel >= 1.0f)
        {
            currentLevel = 1.0f;
            currentState = Decay;
        }
        break;
    case Decay:
        currentLevel = decayBase + currentLevel * decayCoef;

        if (currentLevel <= sustainLevel)
        {
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

        if (currentLevel <= 0.0f)
        {
            currentLevel = 0.0f;
            currentState = Off;
        }
        break;
    default:
        break;
    }

    return currentLevel;
}

float Envelope::calculate (float rate, float targetRatio)
{
    return rate <= 0 ? 0.0f : expf (-logf ((1.0f + targetRatio) / targetRatio) / rate);
}
