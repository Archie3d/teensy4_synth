#pragma once

#include <array>
#include <vector>
#include <cstring>

namespace dsp {

class DelayLine
{
public:

    DelayLine (size_t size = 1024);
    void resize (size_t size);
    void reset();
    void write (float x);
    float read (float delay) const;

    size_t size() const { return m_buffer.size(); }

private:
    std::vector<float> m_buffer;
    size_t m_writeIndex;
};

//==============================================================================

/**
 * @brief DC-component block filter.
 */
struct DCBlocker
{
    struct Spec
    {
        float alpha = 0.995f;
    };

    struct State
    {
        float x1;
        float y1;
    };

    static void updateSpec (Spec& spec);
    static void resetState (const Spec& spec, State& state);
    static float tick (const Spec& spec, State& state, float in);
    static void process (const Spec& spec, State& state, const float* in, float* out, size_t size);
};

//==============================================================================

/**
 * @brief BiQuad configurable IIR
 */
struct BiquadFilter
{
    enum Type
    {
        LowPass,
        HighPass,
        BandPass,
        Notch,
        AllPass,
        PeakingEq,
        LowShelf,
        HighShelf
    };

    struct Spec
    {
        Type type;
        float sampleRate;
        float freq;
        float q;
        float dbGain;

        float a[3];
        float b[4];
    };

    struct State
    {
        float x[2];
        float y[2];
    };

    static void updateSpec (Spec& spec);
    static void resetState (const Spec& spec, State& state);
    static float tick (const Spec& spec, State& state, float in);
    static void process (const Spec& spec, State& state, const float* in, float* out, size_t size);
};

//==============================================================================

/**
 * @brief All-pass filter of fixed delay size.
 */
template <int Size>
struct AllPassFilter
{
    using Buffer = std::array<float, Size>;

    struct Spec
    {
        float feedback;
    };

    struct State
    {
        Buffer buffer;
        int index;
    };

    static void resetState (const Spec&, State& state)
    {
        state.index = 0;
        ::memset (state.buffer.data(), 0, sizeof (float) * state.buffer.size());
    }

    static void process (const Spec& spec, State& state, const float* in, float* out, size_t size)
    {
        for (size_t i = 0; i < size; ++i)
            out[i] = tick (spec, state, in[i]);
    }

    inline static float tick (const Spec& spec, State& state, float x)
    {
        float output;
        float bufOut;

        bufOut = state.buffer[state.index];

        output = bufOut - x;
        state.buffer[state.index] = x + (bufOut * spec.feedback);

        state.index = (state.index + 1) % state.buffer.size();

        return output;
    }

};

//==============================================================================

/**
 * @brief Comb filter of fixed delay size.
 */
template <int Size>
struct CombFilter
{
    using Buffer = std::array<float, Size>;

    struct Spec
    {
        float feedback;
        float damp;
    };

    struct State
    {
        float filterStore;
        Buffer buffer;
        int index;
    };

    static void resetState (const Spec&, State& state)
    {
        state.filterStore = 0.0f;
        state.index = 0;
        ::memset (state.buffer.data(), 0, sizeof (float) * state.buffer.size());
    }

    static void process (const Spec& spec, State& state, const float* in, float* out, size_t size)
    {
        for (size_t i = 0; i < size; ++i)
            out[i] = tick (spec, state, in[i]);
    }

    inline static float tick (const Spec& spec, State& state, float x)
    {
        float output = state.buffer[state.index];

        state.filterStore = output * (1.0f - spec.damp) + state.filterStore * spec.damp;

        state.buffer[state.index] = x + state.filterStore * spec.feedback;

        if (++state.index >= (int) state.buffer.size())
            state.index = 0;

        return output;
    }

};

//==============================================================================

/**
 * @brief Single channel (mono) reverb filter.
 * 
 * This is essentially a combination of comb and all-pass
 * filter of various lengths.
 * 
 * This filter is used to implement the reverb effect.
 */
template <int TuningOffset = 0>
struct Reverb
{
    static constexpr int combTuning1 = 1116 + TuningOffset;
    static constexpr int combTuning2 = 1188 + TuningOffset;;
    static constexpr int combTuning3 = 1277 + TuningOffset;;
    static constexpr int combTuning4 = 1356 + TuningOffset;;
    static constexpr int combTuning5 = 1422 + TuningOffset;;
    static constexpr int combTuning6 = 1491 + TuningOffset;;
    static constexpr int combTuning7 = 1557 + TuningOffset;;
    static constexpr int combTuning8 = 1617 + TuningOffset;;
    static constexpr int allPassTuning1 = 556 + TuningOffset;;
    static constexpr int allPassTuning2 = 441 + TuningOffset;;
    static constexpr int allPassTuning3 = 341 + TuningOffset;;
    static constexpr int allPassTuning4 = 225 + TuningOffset;;

    struct Spec
    {
        float roomsize;
        float damp;

        typename CombFilter<combTuning1>::Spec comb1;
        typename CombFilter<combTuning2>::Spec comb2;
        typename CombFilter<combTuning3>::Spec comb3;
        typename CombFilter<combTuning4>::Spec comb4;
        typename CombFilter<combTuning5>::Spec comb5;
        typename CombFilter<combTuning6>::Spec comb6;
        typename CombFilter<combTuning7>::Spec comb7;
        typename CombFilter<combTuning8>::Spec comb8;

        typename AllPassFilter<allPassTuning1>::Spec allPass1;
        typename AllPassFilter<allPassTuning2>::Spec allPass2;
        typename AllPassFilter<allPassTuning3>::Spec allPass3;
        typename AllPassFilter<allPassTuning4>::Spec allPass4;
    };

    struct State
    {
        typename CombFilter<combTuning1>::State comb1;
        typename CombFilter<combTuning2>::State comb2;
        typename CombFilter<combTuning3>::State comb3;
        typename CombFilter<combTuning4>::State comb4;
        typename CombFilter<combTuning5>::State comb5;
        typename CombFilter<combTuning6>::State comb6;
        typename CombFilter<combTuning7>::State comb7;
        typename CombFilter<combTuning8>::State comb8;

        typename AllPassFilter<allPassTuning1>::State allPass1;
        typename AllPassFilter<allPassTuning2>::State allPass2;
        typename AllPassFilter<allPassTuning3>::State allPass3;
        typename AllPassFilter<allPassTuning4>::State allPass4;
    };

    static void updateSpec (Spec& spec)
    {
        spec.comb1.feedback = spec.roomsize;
        spec.comb2.feedback = spec.roomsize;
        spec.comb3.feedback = spec.roomsize;
        spec.comb4.feedback = spec.roomsize;
        spec.comb5.feedback = spec.roomsize;
        spec.comb6.feedback = spec.roomsize;
        spec.comb7.feedback = spec.roomsize;
        spec.comb8.feedback = spec.roomsize;

        spec.comb1.damp = spec.damp;
        spec.comb2.damp = spec.damp;
        spec.comb3.damp = spec.damp;
        spec.comb4.damp = spec.damp;
        spec.comb5.damp = spec.damp;
        spec.comb6.damp = spec.damp;
        spec.comb7.damp = spec.damp;
        spec.comb8.damp = spec.damp;

        spec.allPass1.feedback = 0.5f;
        spec.allPass2.feedback = 0.5f;
        spec.allPass3.feedback = 0.5f;
        spec.allPass4.feedback = 0.5f;
    }

    static void resetState (const Spec& spec, State& state)
    {
        CombFilter<combTuning1>::resetState (spec.comb1, state.comb1);
        CombFilter<combTuning2>::resetState (spec.comb2, state.comb2);
        CombFilter<combTuning3>::resetState (spec.comb3, state.comb3);
        CombFilter<combTuning4>::resetState (spec.comb4, state.comb4);
        CombFilter<combTuning5>::resetState (spec.comb5, state.comb5);
        CombFilter<combTuning6>::resetState (spec.comb6, state.comb6);
        CombFilter<combTuning7>::resetState (spec.comb7, state.comb7);
        CombFilter<combTuning8>::resetState (spec.comb8, state.comb8);

        AllPassFilter<allPassTuning1>::resetState (spec.allPass1, state.allPass1);
        AllPassFilter<allPassTuning2>::resetState (spec.allPass2, state.allPass2);
        AllPassFilter<allPassTuning3>::resetState (spec.allPass3, state.allPass3);
        AllPassFilter<allPassTuning4>::resetState (spec.allPass4, state.allPass4);
    }

    static void process (const Spec& spec, State& state, const float* in, float* out, size_t size)
    {
        for (size_t i = 0; i < size; ++i) {
            const auto x = in[i];

            float y = CombFilter<combTuning1>::tick (spec.comb1, state.comb1, x);
            y += CombFilter<combTuning2>::tick (spec.comb2, state.comb2, x);
            y += CombFilter<combTuning3>::tick (spec.comb3, state.comb3, x);
            y += CombFilter<combTuning4>::tick (spec.comb4, state.comb4, x);
            y += CombFilter<combTuning5>::tick (spec.comb5, state.comb5, x);
            y += CombFilter<combTuning6>::tick (spec.comb6, state.comb6, x);
            y += CombFilter<combTuning7>::tick (spec.comb7, state.comb7, x);
            y += CombFilter<combTuning8>::tick (spec.comb8, state.comb8, x);

            y *= 0.125f; // normalize due to x8 combs added together 1/8

            y = AllPassFilter<allPassTuning1>::tick (spec.allPass1, state.allPass1, y);
            y = AllPassFilter<allPassTuning2>::tick (spec.allPass2, state.allPass2, y);
            y = AllPassFilter<allPassTuning3>::tick (spec.allPass3, state.allPass3, y);
            y = AllPassFilter<allPassTuning4>::tick (spec.allPass4, state.allPass4, y);

            out[i] = y;
        }
    }

};

} // namespace dsp