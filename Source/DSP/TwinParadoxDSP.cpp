#include "TwinParadoxDSP.h"

static constexpr float kPi            = 3.14159265f;
static constexpr float kCompThreshold = 0.5f;
static constexpr float kMaxDrive      = 10.0f;

void TwinParadoxDSP::prepare(double sampleRate, int /*samplesPerBlock*/)
{
    sr = sampleRate;

    attCoeffA = static_cast<float>(std::exp(-1.0 / (0.005 * sr)));
    relCoeffA = static_cast<float>(std::exp(-1.0 / (0.050 * sr)));
    attCoeffB = static_cast<float>(std::exp(-1.0 / (0.050 * sr)));
    relCoeffB = static_cast<float>(std::exp(-1.0 / (0.500 * sr)));

    outComp.prepare(sampleRate);
    reset();
}

void TwinParadoxDSP::reset()
{
    envA = envB = f1State = f2State = feedback = 0.0f;
    adaaSat.reset();
    adaaFold.reset();
    outComp.reset();
}

float TwinParadoxDSP::updateEnvA(float x) noexcept
{
    const float level = std::abs(x);
    const float coeff = (level > envA) ? attCoeffA : relCoeffA;
    envA = coeff * envA + (1.0f - coeff) * level;
    return envA;
}

float TwinParadoxDSP::updateEnvB(float x) noexcept
{
    const float level = std::abs(x);
    const float coeff = (level > envB) ? attCoeffB : relCoeffB;
    envB = coeff * envB + (1.0f - coeff) * level;
    return envB;
}

// Stage 1 comp: gentle ratio 4:1
float TwinParadoxDSP::compGainA(float env) noexcept
{
    if (env > kCompThreshold)
        return std::pow(kCompThreshold / env, 1.0f - 1.0f / 4.0f);
    return 1.0f;
}

// Stage 2 comp: hard ratio 8:1 — clamps aggressively, amplifies the time-offset effect
float TwinParadoxDSP::compGainB(float env) noexcept
{
    if (env > kCompThreshold)
        return std::pow(kCompThreshold / env, 1.0f - 1.0f / 8.0f);
    return 1.0f;
}

// Stage 1: asymmetric tanh — ADAA 1st-order piecewise
// x≥0: tanh(ap·x)/tanh(ap)   x<0: tanh(an·x)/tanh(an)
// 非対称係数 ap=d·1.3 / an=d·0.8 で奇数倍音 + 2次倍音を生成
float TwinParadoxDSP::saturate(float x, float drive) noexcept
{
    const float d  = std::max(drive, 0.01f);
    const float ap = d * 1.3f;
    const float an = d * 0.8f;
    return adaaSat.process(x, ap, an);
}

// Stage 2: sine wavefolder — ADAA 1st-order
// f(x) = sin(a·x) / norm,  a = d·π/2,  norm = sin(d·π/2)
// F(x) = -cos(a·x) / (a·norm)
float TwinParadoxDSP::wavefold(float x, float drive) noexcept
{
    const float d    = std::max(drive, 0.01f);
    const float a    = d * kPi * 0.5f;
    const float norm = std::max(std::sin(d * kPi * 0.5f), 0.001f);
    return adaaFold.process(x, a, norm);
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

    // --- Stage 1 (Outer): fast tanh saturator ---
    // Stage 2's slow-moving feedback modulates Drive A with time lag
    const float modDriveA = std::clamp(drive1 * (1.0f + modBlend * 5.0f * std::abs(feedback)),
                                       0.0f, kMaxDrive);
    const float eA = updateEnvA(input);
    const float stage1Out = saturate(input * compGainA(eA), modDriveA);

    // --- Filter 1: Forward (Stage 1 → Stage 2) ---
    const float f1Out = lpFilter(stage1Out, f1State, c1);

    // --- Stage 2 (Inner): slow wavefolder — different engine entirely ---
    // Stage 1's fast envelope modulates Stage 2's wavefold drive.
    // Because Stage 2 comp is slow, it hasn't caught up to transients yet,
    // so the two stages are always slightly out of phase with each other.
    const float modDriveB = std::clamp(drive2 * (1.0f + modBlend * eA * 6.0f),
                                       0.0f, kMaxDrive);
    const float eB = updateEnvB(f1Out);
    const float stage2Out = wavefold(f1Out * compGainB(eB), modDriveB);

    // --- Filter 2: Feedback (Stage 2 wavefold output → Stage 1 drive mod) ---
    feedback = lpFilter(stage2Out, f2State, c2);

    // Parallel sum: Stage 1 (tanh body) + Stage 2 (wavefold chaos)
    const float wet = (stage1Out + stage2Out) * 0.5f;
    const float out = dry * (1.0f - mix) + wet * mix;

    // 出力コンプレッサー: Drive 2 特異点付近の急激な音量変化を平準化
    return outComp.process(out);
}
