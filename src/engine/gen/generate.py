"""
Generate Look-Up Tabels
"""

import math

SAMPLE_RATE = 44100.0

def gen_midi_note_to_phase_inc():
    """
    Generate MIDI note to phase increment table
    """

    def note_freq(n):
        #return (440.0 / 32.0) * math.pow(2.0, n * (1.0/12.0))
        return 440.0 * math.pow(2.0, (n - 69) / 12.0)

    res = "const static float DPHASE[128] = {\n"

    for n in range(128):
        f = note_freq(n)
        dp = f / SAMPLE_RATE
        res += "    {:.12f}".format(dp) + "f,\n"
    
    res += "};\n"

    return res


LUT_SINE_SIZE = 4096

def gen_sine():
    """
    Generate sine wave table
    """
    res = "constexpr size_t SINE_SIZE = " + str(LUT_SINE_SIZE) + ";\n"
    res += "const static float SINE[" + str(LUT_SINE_SIZE + 1) + "] = {\n"

    for i in range(LUT_SINE_SIZE):
        s = math.sin(2.0 * i * math.pi / LUT_SINE_SIZE)
        res += "    {:.12f}".format(s) + "f,\n"
    
    res += "    0.0f\n};\n"

    return res

def gen_velocity():
    """
    Generate velocity curve
    """

    res = "constexpr size_t VELOCITY_CURVE_SIZE = 128;\n"
    res += "const static float VELOCITY_CURVE[VELOCITY_CURVE_SIZE] = {\n"

    for i in range(128):
        v = float(i) / 127.0
        v = math.pow(v, 1.5);
        res += "    {:.12f}".format(v) + "f,\n"
    
    res += "};\n"

    return res;

#print(gen_midi_note_to_phase_inc());
#print(gen_sine());
print(gen_velocity());
