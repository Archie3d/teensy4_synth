#include "engine/Engine.h"

Engine::Engine()
{    
}

Engine::~Engine() = default;

int Engine::numActiveVoices() const noexcept
{
    return m_instrument.numActiveVoices();
}

void Engine::noteOn(int channel, int note, int velocity)
{
    AudioLock lock;
    auto* msg = m_midiQueue.allocate();
    *msg = MidiMessage::noteOn(note, velocity);
    m_midiQueue.push(msg);
}

void Engine::noteOff(int channel, int note, int velocity)
{
    AudioLock lock;
    auto* msg = m_midiQueue.allocate();
    *msg = MidiMessage::noteOff(note, velocity);
    m_midiQueue.push(msg);
}

void Engine::controlChange(int channel, int control, int value)
{
    AudioLock lock;
    auto* msg = m_midiQueue.allocate();
    *msg = MidiMessage::controlChange(control, value);
    m_midiQueue.push(msg);
}

void Engine::process(float* outL, float* outR, size_t numFrames)
{
    processMidi();

    ::memset (outL, 0, sizeof(float) * numFrames);
    ::memset (outR, 0, sizeof(float) * numFrames);

    m_instrument.process(outL, outR, numFrames);
}

void Engine::processMidi()
{
    while (auto* midiMessage = m_midiQueue.next())
    {
        processMidiMessage(*midiMessage);
        m_midiQueue.recycle(midiMessage);
    }
}

void Engine::processMidiMessage(const MidiMessage& msg)
{    
    m_instrument.processMidiMessage(msg);
}