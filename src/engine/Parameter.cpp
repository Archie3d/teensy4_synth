#include <cassert>
#include <cmath>
#include <algorithm>
#include "Globals.h"
#include "Parameter.h"

Parameter::Parameter (float value,
                      float min,
                      float max,
                      float smooth)
    : m_currentValue (value)
    , m_minValue (min)
    , m_maxValue (max)
    , m_targetValue (value)
    , m_frac (smooth)
    , m_smoothing (false)
{
}

void Parameter::setValue (float v, float s, bool force)
{
    m_targetValue = math::clamp (m_minValue, m_maxValue, v);
    m_frac = math::clamp (0.0f, 1.0f, s);

    if (force) {
        m_currentValue = m_targetValue;
        m_smoothing = false;
    } else {
        updateSmoothing();
    }  
}

void Parameter::setValue (float v, bool force)
{
    m_targetValue = math::clamp (m_minValue, m_maxValue, v);

    if (force) {
        m_currentValue = m_targetValue;
        m_smoothing = false;
    } else {
        updateSmoothing();
    }   
}

void Parameter::setSmoothing (float s) noexcept
{
    m_frac = math::clamp (0.0f, 1.0f, s);
}

void Parameter::setRange (float min, float max)
{
    m_minValue = std::min (min, max);
    m_maxValue = std::max (min, max);
}

Parameter& Parameter::operator = (float v)
{
    setValue(v);

    return *this;
}

float Parameter::nextValue()
{
    if (m_smoothing)
        m_currentValue = m_targetValue * m_frac + m_currentValue * (1.0f - m_frac);

    updateSmoothing();

    return m_currentValue;
}

void Parameter::updateSmoothing()
{
    constexpr float epsilon = 1e-6f;

    m_smoothing = fabsf (m_currentValue - m_targetValue) > epsilon;

    if (! m_smoothing)
        m_currentValue = m_targetValue;
}

//==============================================================================

ParameterPool::ParameterPool (size_t size)
    : m_params (size)
{
}

Parameter& ParameterPool::operator[] (int index)
{
    if (index >= 0 && index < (int) m_params.size())
        return m_params.at (index);

    return m_dummyParameter;
}
