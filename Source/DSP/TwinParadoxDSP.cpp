#include "TwinParadoxDSP.h"

static constexpr float kPi = 3.14159265f;
static constexpr float kCompThreshold = 0.5f;
static constexpr float kCompRatio     = 4.0f;
static constexpr float kMaxDrive      = 10.0f;

void TwinParadoxDSP::prepare(double sampleRate, int /*samplesPerBlock*/)
{
    sr       = sampleRate;
    attCoeff = static_cast<float>(std::exp(-1.0 / (0.005 * sr))); // 5ms attack
    relCoeff = static_cast<float>(std::exp(-1.0 / (0.050 * sr))); // 50ms release
    reset();
}

void TwinParadoxDSP::reset()
{
    envA = envB = f1State = f2State = feedback = 0.0f;
}

float TwinParadoxDSP::updateEnv(float x, float& env) const noexcept
{
    const float level = std::abs(x);
    const float coeff = (level > env) ? attCoeff : relCoeff;
    env = coeff * env + (1.0f - coeff) * level;
    return env;
}

float TwinParadoxDSP::compGain(float env) noexcept
{
    if (env > kCompThreshold)
        return std::pow(kCompThreshold / env, 1.0f - 1.0f / kCompRatio);
    return 1.0f;
}

// tanh soft-clip normalized so that saturate(1, drive) == 1
float TwinParadoxDSP::saturate(float x, float drive) noexcept
{
    const float d = std::max(drive, 0.01f);
    return std::tanh(d * x) / std::tanh(d);
}

float TwinParadoxDSP::lpFilter(float x, float& state, float coeff) noexcept
{
    state = (1.0f - coeff) * x + coeff * state;
    return state;
}

float TwinParadoxDSP::lpCoeff(float fc, double sampleRate) noexcept
{
    const float clampedFc = std::clamp(fc, 20.0f, static_cast<float>(sampleRate * 0.49));
    return std::exp(-2.0f * kPi * clampedFc / static_cast<float>(sampleRate));
}

float TwinParadoxDSP::processSample(float input)
{
    const float dry = input;

    const float c1 = lpCoeff(filter1Fc, sr);
    const float c2 = lpCoeff(filter2Fc, sr);

    // --- Stage 1 (Outer) ---
    // Feedback from Filter 2 modulates Drive A
    const float modDriveA = std::clamp(drive1 * (1.0f + modBlend * std::abs(feedback)),
                                       0.0f, kMaxDrive);
    const float eA = updateEnv(input, envA);
    const float stage1Out = saturate(input * compGain(eA), modDriveA);

    // --- Filter 1: Forward (Stage 1 → Stage 2) ---
    const float f1Out = lpFilter(stage1Out, f1State, c1);

    // --- Stage 2 (Inner) ---
    // Comp A envelope modulates Drive B (the key cross-modulation)
    const float modDriveB = std::clamp(drive2 * (1.0f + modBlend * eA * 2.0f),
                                       0.0f, kMaxDrive);
    const float eB = updateEnv(f1Out, envB);
    const float stage2Out = saturate(f1Out * compGain(eB), modDriveB);

    // --- Filter 2: Feedback (Stage 2 → Stage 1) ---
    feedback = lpFilter(stage2Out, f2State, c2);

    return dry * (1.0f - mix) + stage2Out * mix;
}
