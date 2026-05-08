#define _USE_MATH_DEFINES

#include <iostream>
#include <fstream>
#include <cmath>

struct DeltaTelemetry {
    const int ticksPerRevolution = 1024;
    const float wheelRadius = 0.3f;
    const float wheelbase = 1.0f;
    float time;
    float distance;
    float theta;
};

struct InputTelemetry {
    unsigned long timestamp;
    long flTicks;
    long frTicks;
    long blTicks;
    long brTicks;

    DeltaTelemetry calculateDelta(const InputTelemetry& prev) const {
        DeltaTelemetry delta;

        long deltaFlTicks = flTicks - prev.flTicks;
        long deltaFrTicks = frTicks - prev.frTicks;
        long deltaBlTicks = blTicks - prev.blTicks;
        long deltaBrTicks = brTicks - prev.brTicks;

        float deltaleft = (deltaFlTicks + deltaBlTicks) / 2.0f;
        float deltaRight = (deltaFrTicks + deltaBrTicks) / 2.0f;
        float distancePerTick = (2.0f * M_PI * delta.wheelRadius) / delta.ticksPerRevolution;

        float distanceLeft = deltaleft * distancePerTick;
        float distanceRight = deltaRight * distancePerTick;

        delta.distance = (distanceLeft + distanceRight) / 2.0f;
        delta.theta = (distanceRight - distanceLeft) / delta.wheelbase;
        delta.time = timestamp - prev.timestamp;

        return delta;
    }
};

struct OutputTelemetry {
    unsigned long timestamp;
    float x;
    float y;
    float theta;

    void applyDelta(const DeltaTelemetry& delta) {
        x += delta.distance * std::cos(theta + delta.theta / 2.0f);
        y += delta.distance * std::sin(theta + delta.theta / 2.0f);
        theta += delta.theta;
        timestamp += delta.time;
    }
};



int main(int argc, char** argv) {    
    if (argc != 2) {
        std::cerr << "usage: ugv_odometry <input_path>\n";

        return 1;
    }
    
    const char* inputFile = argv[1];
    
    std::ifstream input;
    input.open(inputFile, std::ios::out);

    if (!input.is_open()) {
        std::cerr << "Unable to open input file " << inputFile << "\n";

        return 1;
    }

    InputTelemetry currTelemetry;
    InputTelemetry prevTelemetry;
    OutputTelemetry currentPosition = {0, 0.0f, 0.0f, 0.0f};

    int i = 0;
    while (input >> currTelemetry.timestamp >> currTelemetry.flTicks >> currTelemetry.frTicks >> currTelemetry.blTicks >> currTelemetry.brTicks) {
        if (i == 0) {            
            prevTelemetry = currTelemetry;
            i++;

            continue;
        }
        
        DeltaTelemetry delta = currTelemetry.calculateDelta(prevTelemetry);
        currentPosition.applyDelta(delta);

        std::cout << currentPosition.timestamp << " " << currentPosition.x << " " << currentPosition.y << " " << currentPosition.theta << "\n";

        prevTelemetry = currTelemetry;
    }

    input.close();

    return 0;
}
