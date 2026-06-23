#pragma once
#include <cmath>
#include <algorithm>
#include "WaveshapersADAA.h"

// ─────────────────────────────────────────────────────────────────────────────
// OutputComp — ブリックウォールリミッター（Drive2 特異点の爆発を封じる）
//
//   wavefold の sin(π) 特異点で発生する最大 1000x ゲインを制御するため
//   ratio ∞:1 の hard limiting を採用。
//
//   Threshold : -3 dBFS (0.7)  ← 通常信号はそのまま通過
//   Ratio     : ∞ : 1          ← gain = kThresh / env（出力は絶対に超えない）
//   Attack    : 0.2 ms         ← 約 9 サンプル。特異点の瞬発ピークを捕捉
//   Release   : 250 ms         ← 自然な戻り
// ─────────────────────────────────────────────────────────────────────────────
struct OutputComp {
    float env     = 0.f;
    float attCoef = 0.f;
    float relCoef = 0.f;

    static constexpr float kThresh = 0.7f;   // -3 dBFS

    void prepare(double sr) noexcept {
        attCoef = static_cast<float>(std::exp(-1.0 / (0.0002 * sr)));  // 0.2ms
        relCoef = static_cast<float>(std::exp(-1.0 / (0.250  * sr)));  // 250ms
    }

    void reset() noexcept { env = 0.f; }

    float process(float x) noexcept {
        const float absX = std::abs(x);
        const float coef = (absX > env) ? attCoef : relCoef;
        env = coef * env + (1.f - coef) * absX;
        if (env <= kThresh) return x;
        return x * (kThresh / env);   // ∞:1 hard limit — output ≤ kThresh
    }
};

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

    // ADAA state (1 instance = 1 channel)
    ADAA::AsymTanh1 adaaSat;
    ADAA::SineFold1 adaaFold;

    // 出力コンプレッサー
    OutputComp outComp;

    float updateEnvA(float x) noexcept;
    float updateEnvB(float x) noexcept;
    static float compGainA(float env) noexcept; // ratio 4:1
    static float compGainB(float env) noexcept; // ratio 8:1, harder
    float saturate(float x, float drive) noexcept;  // Stage 1: asymmetric tanh (ADAA)
    float wavefold(float x, float drive) noexcept;  // Stage 2: sine wavefolder (ADAA)
    static float lpFilter(float x, float& state, float coeff) noexcept;
    static float lpCoeff(float fc, double sampleRate) noexcept;
};
