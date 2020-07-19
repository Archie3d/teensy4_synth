#include "engine/MidiMessage.h"

MidiMessage::MidiMessage() noexcept
    : rawData(0)
{
}

MidiMessage::MidiMessage(unsigned int data) noexcept
    : rawData(data)
{
}

MidiMessage::MidiMessage(const MidiMessage& other) noexcept
    : rawData(other.rawData)
{
}

MidiMessage& MidiMessage::operator= (const MidiMessage& other) noexcept
{
    if (this != &other)
        rawData = other.rawData;
    
    return *this;
}

void MidiMessage::clear() noexcept
{
    rawData = 0;
}

MidiMessage::Type MidiMessage::type() const noexcept
{
    switch ((rawData & 0x00F00000) >> 16)
    {
        case 0x80: return Type::NoteOff;
        case 0x90: return Type::NoteOn;
        case 0xB0: return Type::ControlChange;
        case 0xE0: return Type::PitchBend;
    }

    return Type::Invalid;
}

int MidiMessage::channel() const noexcept
{
    return ((rawData & 0x000F0000) >> 16) + 1;
}

int MidiMessage::note() const noexcept
{
    return (rawData & 0x00007F00) >> 8;
}

int MidiMessage::velocity() const noexcept
{
    return (rawData & 0x0000007F);
}

int MidiMessage::cc() const noexcept
{
    return (rawData & 0x00007F00) >> 8;
}

int MidiMessage::value() const noexcept
{
    return rawData & 0x0000007F;
}

int MidiMessage::pitch() const noexcept
{
    const int lsb = (rawData & 0x00007F00) >> 8;
    const int msb = rawData & 0x0000007F;

    return (msb << 7) | lsb;
}

MidiMessage MidiMessage::noteOn(int note, int velocity)
{
    return MidiMessage(0x00900000 | ((note & 0x7F) << 8) | (velocity & 0x7F));
}

MidiMessage MidiMessage::noteOff(int note, int velocity)
{
    return MidiMessage(0x00800000 | ((note & 0x7F) << 8));
}

MidiMessage MidiMessage::controlChange(int control, int value)
{
    return MidiMessage(0x00B00000 | ((control & 0x7F) << 8) | (value & 0x7F));
}
