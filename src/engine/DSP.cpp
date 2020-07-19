#include "engine/Globals.h"
#include "engine/DSP.h"

namespace dsp {

DelayLine::DelayLine(size_t size)
    : m_buffer (size, 0.0f)
    , m_writeIndex (0)
{
}

void DelayLine::resize(size_t size)
{
    m_buffer.resize(size);
    reset();
}

void DelayLine::reset()
{
    m_writeIndex = 0;
    ::memset(m_buffer.data(), 0, sizeof(float) * m_buffer.size());
}

void DelayLine::write (float x)
{
    if (m_writeIndex == 0)
        m_writeIndex = m_buffer.size() - 1;
    else
        --m_writeIndex;

    m_buffer[m_writeIndex] = x;
}

float DelayLine::read (float delay) const
{
    int index = (int)floor(delay);
    float frac = delay - (float)index;

    index = (index + m_writeIndex) % (int) m_buffer.size();
    const auto a = m_buffer[index];
    const auto b = index < (int) m_buffer.size() - 1 ? m_buffer[index + 1] : m_buffer[0];

    return math::lerp (a, b, frac);
}

//==============================================================================

void DCBlocker::updateSpec (DCBlocker::Spec&)
{    
}

void DCBlocker::resetState (const DCBlocker::Spec& spec, DCBlocker::State& state)
{
    state.x1 = 0.0f;
    state.y1 = 0.0f;
}

float DCBlocker::tick (const DCBlocker::Spec& spec, DCBlocker::State& state, float in)
{
    state.y1 = in - state.x1 + spec.alpha * state.y1;
    state.x1 = in;
    return state.y1;
}

void DCBlocker::process (const DCBlocker::Spec& spec, DCBlocker::State& state, const float* in, float* out, size_t size)
{
    float x = state.x1;
    float y = state.y1;

    for (size_t i = 0; i < size; ++i)
    {
        y = in[i] - x + spec.alpha * y;
        x = in[i];
    }

    state.x1 = x;
    state.y1 = y;
}

//==============================================================================

void BiquadFilter::updateSpec (BiquadFilter::Spec& spec)
{
    float A = 0.0f;

    if (spec.type == Type::PeakingEq || spec.type == Type::LowShelf || spec.type == Type::HighShelf)
        A = sqrt (powf (10.0f, spec.dbGain / 40.0f));
    else
        A = sqrtf (powf (10.0f, spec.dbGain / 20.0f));

    float w0 = 2.0f * math::Constants<float>::pi * spec.freq / spec.sampleRate;

    float cos_w0 = cos (w0);
    float sin_w0 = sin (w0);
    float alpha = 0.0f;

    switch (spec.type)
    {
    case Type::LowPass:
    case Type::HighPass:
    case Type::AllPass:
        alpha = sin_w0 / (2.0f * spec.q);
        break;
    case Type::BandPass:
    case Type::Notch:
    case Type::PeakingEq:
        alpha = sin_w0 * sinh (log (2.0f) / 2.0f * spec.q * w0 / sin_w0);
        break;
    case Type::LowShelf:
    case Type::HighShelf:
        alpha = sin_w0 / 2.0f * sqrt ((A + 1.0f / A) * (1.0f / spec.q - 1.0f) + 2.0f);
        break;
    default:
        // Unsupported filter type
        break;
    }

    switch (spec.type)
    {
    case Type::LowPass:
        spec.b[0] = (1.0f - cos_w0) / 2.0f;
        spec.b[1] = 1.0f - cos_w0;
        spec.b[2] = (1.0f - cos_w0) / 2.0f;
        spec.a[0] = 1.0f + alpha;
        spec.a[1] = -2.0f * cos_w0;
        spec.a[2] = 1.0f - alpha;
        break;
    case Type::HighPass:
        spec.b[0] = (1.0f + cos_w0) / 2.0f;
        spec.b[1] = -(1.0f + cos_w0);
        spec.b[2] = (1.0f + cos_w0) / 2.0f;
        spec.a[0] = 1.0f + alpha;
        spec.a[1] = -2.0f * cos_w0;
        spec.a[2] = 1.0f - alpha;
        break;
    case Type::BandPass:
        // Constant 0 dB peak gain
        spec.b[0] = alpha;
        spec.b[1] = 0.0f;
        spec.b[2] = -alpha;
        spec.a[0] = 1.0f + alpha;
        spec.a[1] = -2.0f * cos_w0;
        spec.a[2] = 1.0f - alpha;
        break;
    case Type::Notch:
        spec.b[0] = 1.0f;
        spec.b[1] = -2.0f * cos_w0;
        spec.b[2] = 1.0f;
        spec.a[0] = 1.0f + alpha;
        spec.a[1] = -2.0f * cos_w0;
        spec.a[2] = 1.0f - alpha;
        break;
    case Type::AllPass:
        spec.b[0] = 1.0f - alpha;
        spec.b[1] = -2.0f * cos_w0;
        spec.b[2] = 1.0f + alpha;
        spec.a[0] = 1.0f + alpha;
        spec.a[1] = -2.0f * cos_w0;
        spec.a[2] = 1.0f - alpha;
        break;
    case Type::PeakingEq:
        spec.b[0] = 1.0f + alpha * A;
        spec.b[1] = -2.0f * cos_w0;
        spec.b[2] = 1.0f - alpha * A;
        spec.a[0] = 1.0f + alpha / A;
        spec.a[1] = -2.0f * cos_w0;
        spec.a[2] = 1.0f - alpha / A;
        break;
    case Type::LowShelf:
        spec.b[0] = A * ((A + 1.0f) - (A - 1.0f) * cos_w0 + 2.0f * sqrt (A) * alpha);
        spec.b[1] = 2.0f * A * ((A - 1.0f) - (A + 1.0f) * cos_w0);
        spec.b[2] = A * ((A + 1.0f) - (A - 1.0f) * cos_w0 - 2.0f * sqrt (A) * alpha);
        spec.a[0] = (A + 1.0f) + (A - 1.0f) * cos_w0 + 2.0f * sqrt (A) * alpha;
        spec.a[1] = -2.0f * ((A - 1.0f) + (A + 1.0f) * cos_w0);
        spec.a[2] = (A + 1.0f) + (A - 1.0f) * cos_w0 - 2.0f * sqrt (A) * alpha;
        break;
    case Type::HighShelf:
        spec.b[0] = A * ((A + 1.0f) + (A - 1.0f) * cos_w0 + 2.0f * sqrt (A) * alpha);
        spec.b[1] = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * cos_w0);
        spec.b[2] = A * ((A + 1.0f) + (A - 1.0f) * cos_w0 - 2.0f * sqrt (A) * alpha);
        spec.a[0] = (A + 1.0f) - (A - 1.0f) * cos_w0 + 2.0f * sqrt (A) * alpha;
        spec.a[1] = 2.0f * ((A - 1.0f) - (A + 1.0f) * cos_w0);
        spec.a[2] = (A + 1.0f) - (A - 1.0f) * cos_w0 - 2.0f * sqrt(A) * alpha;
        break;
    default:
        // Should never get here
        break;
    }

    // Normalize the coefficients.
    spec.a[1] /= spec.a[0];
    spec.a[2] /= spec.a[0];
    spec.b[0] /= spec.a[0];
    spec.b[1] /= spec.a[0];
    spec.b[2] /= spec.a[0];
}

void BiquadFilter::resetState (const BiquadFilter::Spec&, BiquadFilter::State& state)
{
    ::memset (&state, 0, sizeof (state));
}

float BiquadFilter::tick (const Spec& spec, State& state, float in)
{
    const float x = in;
    const float y = spec.b[0] * x + spec.b[1] * state.x[0] + spec.b[2] * state.x[1]
                    - spec.a[1] * state.y[0] - spec.a[2] * state.y[1];

    state.x[1] = state.x[0];
    state.x[0] = x;
    state.y[1] = state.y[0];
    state.y[0] = y;

    return y;
}


void BiquadFilter::process (const Spec& spec, State& state, const float* in, float* out, size_t size)
{
    for (size_t i = 0; i < size; ++i)
    {
        const float x = in[i];
        const float y = spec.b[0] * x + spec.b[1] * state.x[0] + spec.b[2] * state.x[1]
                        - spec.a[1] * state.y[0] - spec.a[2] * state.y[1];

        state.x[1] = state.x[0];
        state.x[0] = x;
        state.y[1] = state.y[0];
        state.y[0] = y;

        out[i] = y;
    }
}


} // namespace dsp
