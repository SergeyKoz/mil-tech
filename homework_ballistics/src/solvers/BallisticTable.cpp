#include "solvers/BallisticTable.hpp"
#include <fstream>
#include <array>

auto lerp(const Result& a, const Result& b, float t) -> Result
{
    return {a.t + (b.t - a.t) * t, a.hDist + (b.hDist - a.hDist) * t};
}

auto findInterp(float val, const std::vector<float>& axis) -> Interp
{
    if (val <= axis.front()) {
        return {0, 0.0F};
    }

    if (val >= axis.back()) {
        return {(int)axis.size() - 2, 1.0F};
    }

    auto it = std::lower_bound(axis.begin(), axis.end(), val);

    int i = (int)(it - axis.begin()) - 1;

    if (i < 0) {
        i = 0;
    }

    float frac = (val - axis[i]) / (axis[i + 1] - axis[i]);

    return {i, frac};
}

auto BallisticTable::lookup(float Z0, float V0, float m, float d, float l) const -> Result
{
    Interp iz = findInterp(Z0, axisZ0);
    Interp iv = findInterp(V0, axisV0);
    Interp im = findInterp(m, axisM);
    Interp id = findInterp(d, axisD);
    Interp il = findInterp(l, axisL);

    // 2^5 = 32 вершини гіперкуба
    // Згортаємо: 32 → 16 → 8 → 4 → 2 → 1

    // l: 32 → 16
    std::array<Result, 16> v{};

    for (int a = 0; a < 2; a++) {
        for (int b = 0; b < 2; b++) {
            for (int c = 0; c < 2; c++) {
                for (int e = 0; e < 2; e++) {
                    const auto& lo = at(iz.lo + a, iv.lo + b, im.lo + c, id.lo + e, il.lo);
                    const auto& hi = at(iz.lo + a, iv.lo + b, im.lo + c, id.lo + e, il.lo + 1);
                    v.at(a * 8 + b * 4 + c * 2 + e) = lerp(lo, hi, il.frac);
                }
            }
        }
    }

    // d: 16 → 8
    std::array<Result, 8> w{};

    for (int a = 0; a < 2; a++) {
        for (int b = 0; b < 2; b++) {
            for (int c = 0; c < 2; c++) {
                w.at(a * 4 + b * 2 + c) = lerp(v.at(a * 8 + b * 4 + c * 2), v.at(a * 8 + b * 4 + c * 2 + 1), id.frac);
            }
        }
    }

    // m: 8 → 4
    std::array<Result, 4> u{};

    for (int a = 0; a < 2; a++) {
        for (int b = 0; b < 2; b++) {
            u.at(a * 2 + b) = lerp(w.at(a * 4 + b * 2), w.at(a * 4 + b * 2 + 1), im.frac);
        }
    }

    // V0: 4 → 2
    std::array<Result, 2> s{};

    for (size_t a = 0; a < 2; a++) {
        s.at(a) = lerp(u.at(a * 2), u.at(a * 2 + 1), iv.frac);
    }

    // Z0: 2 → 1
    return lerp(s.at(0), s.at(1), iz.frac);
}

// Завантаження з текстового файлу
auto BallisticTable::load(std::string& path) -> bool
{
    std::ifstream f(path);

    if (!f.is_open()) {
        return false;
    }

    int nZ = 0;
    int nV = 0;
    int nM = 0;
    int nD = 0;
    int nL = 0;

    f >> nZ >> nV >> nM >> nD >> nL;

    axisZ0.resize(nZ);
    for (auto& v : axisZ0) {
        f >> v;
    }

    axisV0.resize(nV);

    for (auto& v : axisV0) {
        f >> v;
    }

    axisM.resize(nM);

    for (auto& v : axisM) {
        f >> v;
    }

    axisD.resize(nD);

    for (auto& v : axisD) {
        f >> v;
    }

    axisL.resize(nL);

    for (auto& v : axisL) {
        f >> v;
    }

    size_t total = (size_t)nZ * nV * nM * nD * nL;
    data.resize(total);

    // Порядок: Z0 → V0 → m → d → l (зовнішній → внутрішній)
    for (size_t i = 0; i < total; i++) {
        f >> data[i].t >> data[i].hDist;
    }

    return f.good();
}
