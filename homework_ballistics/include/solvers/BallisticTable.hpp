#pragma once

#include <vector>
#include <string>

struct Result {
    float t;      // час польоту
    float hDist;  // горизонтальна дистанція
};

// Індекс і коефіцієнт для одного виміру
struct Interp {
    int lo;      // нижній індекс в осі
    float frac;  // коефіцієнт [0..1]
};

struct BallisticTable {
    // 5 осей — кожна зі своїм набором вузлів (нерівномірний крок)
    std::vector<float> axisZ0;  // висота
    std::vector<float> axisV0;  // швидкість
    std::vector<float> axisM;   // маса
    std::vector<float> axisD;   // опір
    std::vector<float> axisL;   // підйомна сила

    // Результат в кожному вузлі сітки

    // Плоский масив розміром |Z0| * |V0| * |M| * |D| * |L|
    std::vector<Result> data;

    // Індекс у плоскому масиві: [iZ0][iV0][iM][iD][iL]
    size_t index(int iz, int iv, int im, int id, int il) const
    {
        return ((((size_t)iz * axisV0.size() + iv) * axisM.size() + im) * axisD.size() + id) * axisL.size() + il;
    }

    const Result& at(int iz, int iv, int im, int id, int il) const { return data[index(iz, iv, im, id, il)]; }

    bool load(std::string& path);

    Result lookup(float Z0, float V0, float m, float d, float l) const;
};