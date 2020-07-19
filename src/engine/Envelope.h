#pragma once

class Envelope
{
public:

    enum State
    {
        Off = 0,
        Attack,
        Decay,
        Sustain,
        Release,
        NumStates
    };

    constexpr static float AttackTargetRatio = 0.3f;
    constexpr static float DecayReleaseTargetRatio = 0.0001f;

    struct Trigger
    {
        float attack      = 0.0f;
        float decay       = 0.0f;
        float sustain     = 1.0f; 
        float release     = 1.0f;
    };

    Envelope();

    State state() const noexcept { return currentState; }

    void trigger (const Trigger& trigger, float sampleRate);
    void release();
    void release (float t, float sampleRate);

    float next();

    float level() const noexcept { return currentLevel; }

private:

    static float calculate (float rate, float targetRatio);

    State currentState;
    float currentLevel;

    float attackRate;
    float attackCoef;
    float attackBase;

    float decayRate;
    float decayCoef;
    float decayBase;

    float releaseRate;
    float releaseCoef;
    float releaseBase;

    float sustainLevel;
};
