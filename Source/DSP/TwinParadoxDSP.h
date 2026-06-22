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
    float drive1    = 0.5f;    // Stage 1 drive (mapped 0.5–8.0)
    float drive2    = 0.5f;    // Stage 2 drive (mapped 0.5–8.0)
    float filter1Fc = 4000.0f; // Forward filter cutoff (Hz)
    float filter2Fc = 4000.0f; // Feedback filter cutoff (Hz)
    float modBlend  = 0.5f;    // Cross-modulation amount
    float mix       = 1.0f;    // Dry/Wet

private:
    double sr = 44100.0;

    // Stage 1 comp: fast (5ms att / 50ms rel / ratio 4:1)
    float envA     = 0.0f;
    float attCoeffA = 0.0f, relCoeffA = 0.0f;

    // Stage 2 comp: slow (50ms att / 500ms rel / ratio 8:1) — different engine
    float envB     = 0.0f;
    float attCoeffB = 0.0f, relCoeffB = 0.0f;

    // 1-pole IIR filter states
    float f1State = 0.0f, f2State = 0.0f;

    // Feedback sample from Filter 2 back to Stage 1
    float feedback = 0.0f;

    float updateEnvA(float x) noexcept;
    float updateEnvB(float x) noexcept;
    static float compGainA(float env) noexcept; // ratio 4:1
    static float compGainB(float env) noexcept; // ratio 8:1, harder
    static float saturate(float x, float drive) noexcept;  // Stage 1: asymmetric tanh
    static float wavefold(float x, float drive) noexcept;  // Stage 2: wavefolder
    static float lpFilter(float x, float& state, float coeff) noexcept;
    static float lpCoeff(float fc, double sampleRate) noexcept;
};
