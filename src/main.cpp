#include <Arduino.h>
#include "output_i2s.h"
#include "MIDI.h"
#include "usb_midi.h"
#include "engine/Engine.h"
#include "engine/MidiMessage.h"
#include "engine/AudioProcess.h"

extern "C" {
    // These are to avoid linker undefined references error
    // compiling unwind-arm.c, since exceptions are
    // disabled anyway.
    int __exidx_start() { return -1;}
    int __exidx_end() { return -1; }
}

//==========================================================

AudioProcess audioProcess;
AudioOutputI2S i2s;
AudioConnection wireL(audioProcess, 0, i2s, 0);
AudioConnection wireR(audioProcess, 1, i2s, 1);

// =========================================================

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

namespace midi
{
    static void noteOn(byte channel, byte note, byte velocity)
    {
        audioProcess.noteOn(channel, note, velocity);
	}

    static void noteOff(byte channel, byte note, byte velocity)
    {
        audioProcess.noteOff(channel, note, velocity);
    }

    static void controlChange(byte channel, byte control, byte value)
    {
        audioProcess.controlChange(channel, control, value);
    }

    static void processHardwareMIDI()
    {
        if (MIDI.read()) {
            const auto type = MIDI.getType();

            switch (type) {
                case midi::NoteOn:
                    audioProcess.noteOn(MIDI.getChannel(), MIDI.getData1(), MIDI.getData2());
                    break;
                case midi::NoteOff:
                    audioProcess.noteOff(MIDI.getChannel(), MIDI.getData1(), MIDI.getData2());
                    break;
                case midi::ControlChange:
                    audioProcess.controlChange(MIDI.getChannel(), MIDI.getData1(), MIDI.getData2());
                    break;
                default:
                    break;
            }
        }
    }
}

// =========================================================

extern "C" int main(void) {
	Serial.begin(115200);
	Serial.println("Initialized");

    {
        Engine::AudioLock lock;

        // Initialize USB (slave) midi
        usbMIDI.setHandleNoteOn        (midi::noteOn);
        usbMIDI.setHandleNoteOff       (midi::noteOff);
        usbMIDI.setHandleControlChange (midi::controlChange);

        // Initialize hardware MIDI
        MIDI.begin(MIDI_CHANNEL_OMNI);
    }

    pinMode(13, OUTPUT);

    auto ts = millis();

	while (1) {
        usbMIDI.read();
        midi::processHardwareMIDI();

        const auto t = millis() - ts;

        const bool sense = (audioProcess.amplitudeL() + audioProcess.amplitudeR()) > 0.5f;
        digitalWriteFast(13, sense);

        if (t >= 1000) {
            Serial.printf("DSP Load: %f%%  Voices: %d, L: %f R: %f\r\n",
                audioProcess.dspLoadPercent(),
                audioProcess.numActiveVoices(),
                audioProcess.amplitudeL(),
                audioProcess.amplitudeR());

            ts += t;
        }
    }
}
