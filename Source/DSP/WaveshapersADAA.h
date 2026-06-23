#pragma once
#include <cmath>
#include <algorithm>

// ─────────────────────────────────────────────────────────────────────────────
// Antiderivative Anti-Aliasing (ADAA) Waveshapers
//
// References:
//   Parker, Zavalishin, Le Bivic — DAFx 2016
//     "Reducing the Aliasing of Nonlinear Waveshaping Using
//      Continuous-Time Convolution"
//
//   Chowdhury — Medium 2020
//     "Practical Considerations for Antiderivative Anti-Aliasing"
//     github.com/jatinchowdhury18/ADAA
//
//   Numerically stable ln(cosh(x)) derivation:
//     apulsoft.ch/blog/tanh-antiderivatives/
//
// ─────────────────────────────────────────────────────────────────────────────
// 1st-order ADAA 公式:
//   y[n] = (F(x[n]) − F(x[n−1])) / (x[n] − x[n−1])   |Δx| > eps の場合
//        = f((x[n]+x[n−1]) / 2)                        |Δx| ≈ 0 の場合 (fallback)
//
// ドライブがオーディオレートで変化する場合: a が変わったら xn1 の地点で
// 新しい不動点基底で Fn1 を再計算してグリッチを防ぐ。
// ─────────────────────────────────────────────────────────────────────────────

namespace ADAA {

static constexpr float kEps = 1e-5f;
static constexpr float kLn2 = 0.6931471805599453f;

// ln(cosh(x)) の数値安定版
// |x| > 20: exp(-2|x|) → 0 なので ln(cosh(x)) ≈ |x| − ln2
inline float logCosh(float x) noexcept
{
    const float ax = std::abs(x);
    if (ax > 20.f) return ax - kLn2;
    return ax - kLn2 + std::log1p(std::exp(-2.f * ax));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tanh1 — f(x) = tanh(a·x) の 1st-order ADAA
//   F(x) = logCosh(a·x) / a
// ─────────────────────────────────────────────────────────────────────────────
struct Tanh1
{
    float xn1   = 0.f;
    float Fn1   = 0.f;
    float prevA = 1.f;

    void reset() noexcept { xn1 = Fn1 = 0.f; prevA = 1.f; }

    float process(float xn, float a = 1.f) noexcept
    {
        if (a != prevA) {
            Fn1   = logCosh(a * xn1) / a;
            prevA = a;
        }
        const float Fn  = logCosh(a * xn) / a;
        const float dx  = xn - xn1;
        const float out = (std::abs(dx) < kEps)
            ? std::tanh(a * 0.5f * (xn + xn1))
            : (Fn - Fn1) / dx;
        xn1 = xn;
        Fn1 = Fn;
        return out;
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// AsymTanh1 — piecewise asymmetric tanh の 1st-order ADAA
//   x ≥ 0: f(x) = tanh(ap·x) / tanh(ap)
//   x < 0: f(x) = tanh(an·x) / tanh(an)
//
// 両ピースで F(0) = 0 なので、sign をまたぐ遷移も同一公式で扱える。
//   F(x) = logCosh(a·x) / (a · tanh(a))   (a = ap or an)
// ─────────────────────────────────────────────────────────────────────────────
struct AsymTanh1
{
    float xn1    = 0.f;
    float Fn1    = 0.f;
    float prevAp = 1.f;
    float prevAn = 1.f;

    void reset() noexcept { xn1 = Fn1 = 0.f; prevAp = prevAn = 1.f; }

    static float eval(float x, float ap, float an) noexcept
    {
        const float a = (x >= 0.f) ? ap : an;
        return std::tanh(a * x) / std::tanh(a);
    }

    static float antideriv(float x, float ap, float an) noexcept
    {
        const float a  = (x >= 0.f) ? ap : an;
        const float ta = std::tanh(a);
        return logCosh(a * x) / (a * ta);
    }

    float process(float xn, float ap, float an) noexcept
    {
        if (ap != prevAp || an != prevAn) {
            Fn1    = antideriv(xn1, ap, an);
            prevAp = ap;
            prevAn = an;
        }
        const float Fn  = antideriv(xn, ap, an);
        const float dx  = xn - xn1;
        const float out = (std::abs(dx) < kEps)
            ? eval(0.5f * (xn + xn1), ap, an)
            : (Fn - Fn1) / dx;
        xn1 = xn;
        Fn1 = Fn;
        return out;
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// SineFold1 — f(x) = sin(a·x) / norm の 1st-order ADAA (sine wavefolder)
//   F(x) = −cos(a·x) / (a · norm)
// ─────────────────────────────────────────────────────────────────────────────
struct SineFold1
{
    float xn1   = 0.f;
    float Fn1   = 0.f;
    float prevA = 1.f;
    float prevN = 1.f;

    void reset() noexcept { xn1 = Fn1 = 0.f; prevA = prevN = 1.f; }

    static float antideriv(float x, float a, float norm) noexcept
    {
        return -std::cos(a * x) / (a * norm);
    }

    float process(float xn, float a, float norm) noexcept
    {
        if (a != prevA || norm != prevN) {
            Fn1   = antideriv(xn1, a, norm);
            prevA = a;
            prevN = norm;
        }
        const float Fn  = antideriv(xn, a, norm);
        const float dx  = xn - xn1;
        const float out = (std::abs(dx) < kEps)
            ? std::sin(a * 0.5f * (xn + xn1)) / norm
            : (Fn - Fn1) / dx;
        xn1 = xn;
        Fn1 = Fn;
        return out;
    }
};

} // namespace ADAA
