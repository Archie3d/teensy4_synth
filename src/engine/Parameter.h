#pragma once

#include <vector>

/**
 * Parameter with smoothed float value.
 */
class Parameter
{
public:

    Parameter (float value = 0.0f,
               float min = 0.0f,
               float max = 1.0f,
               float smooth = 0.5f);
    
    void setValue (float v, float s, bool force = false);
    void setValue (float v, bool force = false);
    void setSmoothing (float s) noexcept;
    void setRange (float min, float max);

    Parameter& operator = (float v);

    float value() const noexcept      { return m_currentValue; }
    float target() const noexcept     { return m_targetValue; }
    float min() const noexcept        { return m_minValue; }
    float max() const noexcept        { return m_maxValue; }
    bool isSmoothing() const noexcept { return m_smoothing || m_currentValue != m_targetValue; }

    float nextValue();

    float& targetRef() noexcept { return m_targetValue; }

private:

    void updateSmoothing();

    float m_currentValue;
    float m_minValue;
    float m_maxValue;
    float m_targetValue;
    float m_frac;
    bool  m_smoothing;
};

//----------------------------------------------------------

class ParameterPool
{
public:
    ParameterPool (size_t size);
    size_t size() const { return m_params.size(); }
    Parameter& operator[] (int index);

private:
    std::vector<Parameter> m_params;
    Parameter m_dummyParameter;
};
