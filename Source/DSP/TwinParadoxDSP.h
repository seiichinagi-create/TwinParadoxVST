#pragma once
#include <cmath>
#include <algorithm>

class TwinParadoxDSP
{
public:
    void prepare(double sampleRate, int samplesPerBlock);
    void reset();
    float processSample(float input);

    // Parameters (set before each processBlock)
    float drive1    = 0.5f;   // Stage 1 drive (mapped 0.5–8.0)
    float drive2    = 0.5f;   // Stage 2 drive (mapped 0.5–8.0)
    float filter1Fc = 1000.0f; // Forward filter cutoff (Hz)
    float filter2Fc = 1000.0f; // Feedback filter cutoff (Hz)
    float modBlend  = 0.5f;   // Cross-modulation amount
    float mix       = 1.0f;   // Dry/Wet

private:
    double sr = 44100.0;

    // Envelope state for Comp A and Comp B
    float envA = 0.0f, envB = 0.0f;
    float attCoeff = 0.0f, relCoeff = 0.0f;

    // 1-pole IIR filter states
    float f1State = 0.0f, f2State = 0.0f;

    // Feedback sample from Filter 2 back to Stage 1
    float feedback = 0.0f;

    float updateEnv(float x, float& env) const noexcept;
    static float compGain(float env) noexcept;
    static float saturate(float x, float drive) noexcept;
    static float lpFilter(float x, float& state, float coeff) noexcept;
    static float lpCoeff(float fc, double sampleRate) noexcept;
};
