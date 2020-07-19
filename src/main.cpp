#include <Arduino.h>
#include "output_i2s.h"
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
}

// =========================================================

extern "C" int main(void) {
	Serial.begin(115200);
	Serial.println("Initialized");

    {
        Engine::AudioLock lock;

        usbMIDI.setHandleNoteOn        (midi::noteOn);
        usbMIDI.setHandleNoteOff       (midi::noteOff);
        usbMIDI.setHandleControlChange (midi::controlChange);
    }

    pinMode(13, OUTPUT);

    auto ts = millis();
    bool toggle = false;

	while (1) {
        usbMIDI.read();

        const auto t = millis() - ts;

        if (t >= 500) {
            digitalWriteFast(13, toggle ? HIGH : LOW);

            if (toggle)
                Serial.printf("DSP Load: %f%%  Voices: %d, L: %f R: %f\r\n",
                    audioProcess.dspLoadPercent(),
                    audioProcess.numActiveVoices(),
                    audioProcess.amplitudeL(),
                    audioProcess.amplitudeR());

            toggle = !toggle;
            ts += t;
        }
    }
}
