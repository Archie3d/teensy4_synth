#pragma once

#include <array>
#include "engine/FastList.h"

class MidiMessage : public ListItem<MidiMessage>
{
public:
    enum class Type
    {
        Invalid,
        NoteOff,
        NoteOn,
        ControlChange,
        PitchBend
    };

    enum Controller {
        CC_Modulation   = 1,
        CC_SustainPedal = 64
    };

    MidiMessage() noexcept;
    MidiMessage(unsigned int data) noexcept;
    MidiMessage(const MidiMessage& other) noexcept;
    MidiMessage& operator = (const MidiMessage& other) noexcept;

    void clear() noexcept;

    Type type()    const noexcept;
    int channel()  const noexcept;
    int note()     const noexcept;
    int velocity() const noexcept;
    int cc()       const noexcept;
    int value()    const noexcept;
    int pitch()    const noexcept;

    static MidiMessage noteOn(int note, int velocity) noexcept;
    static MidiMessage noteOff(int note, int velocity) noexcept;
    static MidiMessage controlChange(int control, int value) noexcept;

    // Message raw data (4 bytes only).
    unsigned int rawData;
};

//==============================================================================

template<size_t Size>
class MidiQueue
{
public:
    MidiQueue()
    {
        for (auto& msg : m_allocatedMessages)
            m_idleMessages.append(&msg);
    }

    MidiMessage* allocate()
    {
        if (auto* msg = m_idleMessages.first()) {
            m_idleMessages.remove(msg);
            return msg;
        }

        return nullptr;
    }

    void push(MidiMessage* msg)
    {
        m_pendingMessages.append(msg);
    }

    MidiMessage* next()
    {
        if (auto* msg = m_pendingMessages.first()) {
            m_pendingMessages.remove(msg);
            return msg;
        }

        return nullptr;
    }

    void recycle(MidiMessage* msg)
    {
        msg->clear();
        m_idleMessages.append(msg);
    }

private:
    std::array<MidiMessage, Size> m_allocatedMessages;
    List<MidiMessage> m_idleMessages;
    List<MidiMessage> m_pendingMessages;
};
