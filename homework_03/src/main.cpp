#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS
#define ENABLE_LOG 1
#define ENABLE_DEBUG 0

#if ENABLE_LOG
#define LOG(msg) std::cout << "[LOG] " << msg << std::endl
#else
#define LOG(msg)
#endif

#if ENABLE_DEBUG
#define DEBUG(msg) std::cout << "[DEBUG] " << msg << std::endl
#else
#define DEBUG(msg)
#endif

#include <iostream>
#include <fstream>
#include <cmath>
#include <nlohmann/json.hpp>

const float G = 9.81f;

using json = nlohmann::json;

struct Coord {
    float x;
    float y;

    Coord operator+(const Coord& other) const
    {
        Coord result;
        result.x = x + other.x;
        result.y = y + other.y;
        return result;
    }

    Coord operator-(const Coord& other) const
    {
        Coord result;
        result.x = x - other.x;
        result.y = y - other.y;
        return result;
    }

    Coord move(float distance, float direction) { return {x + distance * std::cos(direction), y + distance * std::sin(direction)}; }

    float distance(const Coord& other) const { return std::sqrt(std::pow((other.x - x), 2) + std::pow((other.y - y), 2)); }

    float direction(const Coord& other) const
    {
        float direction = std::atan2(other.y - y, other.x - x);

        if (direction < 0) {
            direction += 2.0f * M_PI;
        }

        return direction;
    }
};
#pragma pack(push, 1)
struct DroneConfig {
    Coord startPos;          // початкова позиція (x, y)
    float altitude;          // висота
    float initialDir;        // початковий напрямок (рад)
    float attackSpeed;       // швидкість атаки (м/с)
    float accelerationPath;  // шлях розгону (м)
    char ammoName[32];       // обрані боєприпаси
    float arrayTimeStep;     // крок часу масиву цілей
    float simTimeStep;       // крок симуляції
    float hitRadius;         // радіус влучення
    float angularSpeed;      // кутова швидкість (рад/с)
    float turnThreshold;     // поріг повороту (рад)
};
#pragma pack(pop)

struct AmmoParams {
    char name[32];
    float mass;  // маса (кг)
    float drag;  // коефіцієнт опору
    float lift;  // коефіцієнт підйому
};

struct SimStep {
    Coord pos;        // позиція дрона
    float direction;  // напрямок (рад)
    int state;        // стан автомата (0-4)
    int targetIdx;    // індекс поточної цілі
    Coord dropPoint;  // точка скиду (куди летить дрон)
    Coord aimPoint;   // куди впаде бомба (якщо скинути зараз)
    Coord predictedTarget;
    float speed;
};

enum DroneStatus { STOPPED = 0, ACCELERATING = 1, DECELERATING = 2, TURNING = 3, MOVING = 4 };

void interpolateTarget(int targetIndex, float time, float arrayTimeStep, float& outTargetX, float& outTargetY);
Coord interpolateTarget(const Coord* target, float time, float arrayTimeStep);
double calcDropTime(const AmmoParams& ammo, double v0, double z0);
double calcDropDistance(double t, const AmmoParams& ammo, double v0);

inline void doAcceleration(SimStep& simStep, float acceleration, float time, float attackSpeed);
inline void doDeceleration(SimStep& simStep, float acceleration, float time);
inline void doMoving(SimStep& simStep, float time);
inline void doTurning(SimStep& simStep, float turnAngle, float angleStep);

inline float calcReEntryPath(
    float distanceToDropPoint, bool isNeedTurnAngle, float speed, float attackSpeed, float acceleration, float accelerationPath);
inline float calcReEntryTime(float speed,
                             float angularSpeed,
                             float turnAngle,
                             float acceleration,
                             float accelerationPath,
                             float fullAccelerationTime,
                             float attackSpeed,
                             float reEntryPath);
inline float calcEntryTime(int state,
                           float distanceToDropPoint,
                           bool isNeedTurnAngle,
                           float speed,
                           float attackSpeed,
                           float acceleration,
                           float accelerationPath,
                           float fullAccelerationTime);
void from_json(const json& j, DroneConfig& droneConfig);
AmmoParams* loadAmmoParameters(std::ifstream& ammoFile, int& outCount);
Coord** loadTargets(std::ifstream& targetsFile, int& outCount, int& outTimeSteps);

const int MAX_STEPS{5000};
SimStep out[MAX_STEPS];

int main()
{
    // read config
    std::ifstream configFile{"homework_03/data/config.json", std::ios::out};

    if (!configFile.is_open()) {
        std::cerr << "Unable to open config file\n";

        return 1;
    }

    json jsonConfig;
    DroneConfig droneConfig;

    try {
        configFile >> jsonConfig;
        droneConfig = jsonConfig.get<DroneConfig>();
    }
    catch (const std::exception& ex) {
        std::cerr << "Unable to parse config file\n" << ex.what() << std::endl;

        return 1;
    }

    configFile.close();

    // read ammo parameters
    std::ifstream ammoFile{"homework_03/data/ammo.json", std::ios::out};

    if (!ammoFile.is_open()) {
        std::cerr << "Unable to open ammo file\n";

        return 1;
    }

    int ammoCount;
    AmmoParams* ammos = loadAmmoParameters(ammoFile, ammoCount);

    if (ammos == nullptr) {
        std::cerr << "Unable parse ammo config file\n";

        return 1;
    }

    ammoFile.close();

    // read targets
    std::ifstream targetsFile{"homework_03/data/targets.json", std::ios::out};

    if (!targetsFile.is_open()) {
        std::cerr << "Unable to open targets file\n";

        return 1;
    }

    int targetsCount, targetTimeSteps;
    Coord** targets = loadTargets(targetsFile, targetsCount, targetTimeSteps);

    if (targets == nullptr) {
        std::cerr << "Unable parse targets config file\n";

        return 1;
    }

    targetsFile.close();

    AmmoParams ammo;
    bool ammoDefined = false;

    for (int i = 0; i < targetsCount; i++) {
        if (strcmp(ammos[i].name, droneConfig.ammoName) == 0) {
            ammo = ammos[i];
            ammoDefined = true;

            break;
        }
    }

    if (!ammoDefined) {
        std::cerr << "Unable to define selecteg ammo\n";

        return 1;
    }
    //-------------
    SimStep simStep = {.pos = droneConfig.startPos,    // позиція дрона
                       .direction = 0.f,               // напрямок (рад)
                       .state = STOPPED,               // стан автомата(0 - 4)
                       .targetIdx = -1,                // індекс поточної цілі
                       .dropPoint = {0.f, 0.f},        // точка скиду (куди летить дрон)
                       .aimPoint = {0.f, 0.f},         // куди впаде бомба (якщо скинути зараз)
                       .predictedTarget = {0.f, 0.f},  // прогнозована позиція цілі
                       .speed = 0.f};

    float e{1e-5f};
    double h;

    try {
        double t = calcDropTime(ammo, droneConfig.attackSpeed, droneConfig.altitude);
        h = calcDropDistance(t, ammo, droneConfig.attackSpeed);
    }
    catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }

    float acceleration = droneConfig.attackSpeed * droneConfig.attackSpeed / (2 * droneConfig.accelerationPath);
    float fullAccelerationTime = droneConfig.attackSpeed / acceleration;
    float angleStep = droneConfig.angularSpeed * droneConfig.simTimeStep;

    int step = 0;
    float currentTime{0.f};

    while (MAX_STEPS > step) {
        float minTotalTime;
        int definedTargetIndex = -1;
        Coord definedTargetPos;

        for (int targetIndex = 0; targetIndex < 1; targetIndex++) {
            Coord currentTargetPos = interpolateTarget(targets[targetIndex], currentTime, droneConfig.arrayTimeStep);

            // define total time to reach target
            // totalTime time to stop + time to turn + time to accelerate + time to move
            float timeToTurn{0.f};

            // time to stop + time to turn + time to accelerate + time to move
            bool isNeedTurnAngle = false;
            float targetDirection = simStep.pos.direction(currentTargetPos);
            float turnAngle = targetDirection - simStep.direction;

            if (std::fabs(turnAngle) > droneConfig.turnThreshold) {
                timeToTurn = std::abs(turnAngle) / droneConfig.angularSpeed;
                isNeedTurnAngle = true;
            }

            // drop point calculation
            float distanceToTarget = currentTargetPos.distance(simStep.pos);
            float distanceToDropPoint = distanceToTarget - h;

            // define reentry path
            float reEntryPath = calcReEntryPath(
                distanceToDropPoint, isNeedTurnAngle, simStep.speed, droneConfig.attackSpeed, acceleration, droneConfig.accelerationPath);

            float totalTime = reEntryPath > 0 ? calcReEntryTime(simStep.speed,
                                                                droneConfig.angularSpeed,
                                                                turnAngle,
                                                                acceleration,
                                                                droneConfig.accelerationPath,
                                                                fullAccelerationTime,
                                                                droneConfig.attackSpeed,
                                                                reEntryPath)
                                              : calcEntryTime(simStep.state,
                                                              distanceToDropPoint,
                                                              isNeedTurnAngle,
                                                              simStep.speed,
                                                              droneConfig.attackSpeed,
                                                              acceleration,
                                                              droneConfig.accelerationPath,
                                                              fullAccelerationTime) +
                                                    timeToTurn;

            if (totalTime < 0) {
                continue;
            }

            if (definedTargetIndex == -1 || totalTime < minTotalTime) {
                minTotalTime = totalTime;
                definedTargetIndex = targetIndex;
                definedTargetPos = currentTargetPos;
            }
        }

        simStep.targetIdx = definedTargetIndex;

        // next coords
        Coord interpolatedPos =
            interpolateTarget(targets[definedTargetIndex], currentTime + droneConfig.simTimeStep, droneConfig.arrayTimeStep);
        Coord delta = interpolatedPos - definedTargetPos;

        float vx = delta.x / droneConfig.simTimeStep;
        float vy = delta.y / droneConfig.simTimeStep;

        simStep.predictedTarget = {
            definedTargetPos.x + vx * minTotalTime,
            definedTargetPos.y + vy * minTotalTime,
        };

        float targetDirection = simStep.pos.direction(simStep.predictedTarget);
        float turnAngle = targetDirection - simStep.direction;
        bool isNeedTurnAngle;
        if (simStep.state == TURNING) {
            isNeedTurnAngle = std::fabs(turnAngle) > angleStep;
        }
        else {
            isNeedTurnAngle = std::fabs(turnAngle) > droneConfig.turnThreshold;
        }

        // need reentry
        float distanceToTarget = simStep.pos.distance(simStep.predictedTarget);
        float distanceToDropPoint = distanceToTarget - h;
        float reEntryPath = distanceToDropPoint < 0 ? -distanceToDropPoint : 0.f;

        if (!isNeedTurnAngle) {
            if (simStep.speed < droneConfig.attackSpeed) {
                float accelerationPath =
                    (droneConfig.attackSpeed * droneConfig.attackSpeed - simStep.speed * simStep.speed) / (2 * acceleration);

                if (distanceToDropPoint - accelerationPath < 0) {
                    reEntryPath += accelerationPath;
                }
            }
        }
        else {
            float stopingPath = 0.f;

            if (simStep.speed > 0) {
                stopingPath = (simStep.speed * simStep.speed) / (2 * acceleration);
            }

            if (distanceToDropPoint < stopingPath + droneConfig.accelerationPath) {
                reEntryPath = reEntryPath + stopingPath + droneConfig.accelerationPath;
            }
        }

        if (reEntryPath > 0) {
            // perform reentry maneur
            float reversDirection = simStep.predictedTarget.direction(simStep.pos);
            float turnAngle = reversDirection - simStep.direction;
            bool isReverseDirection = std::fabs(turnAngle) < droneConfig.turnThreshold;

            if (!isReverseDirection) {
                if (simStep.state == MOVING || simStep.state == ACCELERATING || simStep.state == DECELERATING) {
                    // need stop
                    simStep.state = DECELERATING;
                    doDeceleration(simStep, acceleration, droneConfig.simTimeStep);
                }
                else if (simStep.state == STOPPED || simStep.state == TURNING) {
                    if (!isReverseDirection) {
                        simStep.direction = turnAngle > 0 ? simStep.direction + angleStep : simStep.direction - angleStep;
                        simStep.state = TURNING;
                    }
                    else {
                        simStep.state = ACCELERATING;
                    }
                }
            }
            else {
                // flying away
                if (simStep.state == TURNING || simStep.state == STOPPED || simStep.state == ACCELERATING) {
                    simStep.state = ACCELERATING;
                    doAcceleration(simStep, acceleration, droneConfig.simTimeStep, droneConfig.attackSpeed);
                }
                else if (simStep.state == MOVING) {
                    doMoving(simStep, droneConfig.simTimeStep);
                }
            }
        }
        else {
            // perform entry maneur
            if (isNeedTurnAngle) {
                // perform turn maneur
                if (simStep.state == STOPPED || simStep.state == TURNING) {
                    doTurning(simStep, turnAngle, angleStep);
                }
                else if (simStep.state == MOVING || simStep.state == ACCELERATING || simStep.state == DECELERATING) {
                    simStep.state = DECELERATING;
                    doDeceleration(simStep, acceleration, droneConfig.simTimeStep);
                }
            }
            else {
                if (simStep.state == STOPPED || simStep.state == TURNING || simStep.state == DECELERATING ||
                    simStep.state == ACCELERATING) {
                    simStep.state = ACCELERATING;
                    doAcceleration(simStep, acceleration, droneConfig.simTimeStep, droneConfig.attackSpeed);
                }
                else if (simStep.state == MOVING) {
                    doMoving(simStep, droneConfig.simTimeStep);
                }
            }
        }

        // check hitRadius
        float targetHit = false;
        float D = simStep.pos.distance(interpolatedPos);
        simStep.dropPoint = simStep.pos.move(D - h, simStep.direction);

        LOG("|step: " << step << "|st: " << simStep.state << "|sp: " << simStep.speed << "|x,y,d: " << simStep.pos.x << "," << simStep.pos.x
                      << "," << simStep.direction << "|a: " << turnAngle);
        LOG("|step: " << step << "|ix:iy " << interpolatedPos.x << interpolatedPos.y << "|px,py: " << simStep.predictedTarget.x << ","
                      << simStep.predictedTarget.x << "|D: " << D);

        if (simStep.speed >= droneConfig.attackSpeed) {
            simStep.aimPoint = simStep.pos.move(h, simStep.direction);
            float DF = simStep.aimPoint.distance(interpolatedPos);

            LOG("|DF: " << DF);

            if (DF <= droneConfig.hitRadius) {
                std::cout << std::endl << "Hit!" << std::endl;

                targetHit = true;
            }
        }

        out[step] = simStep;

        if (targetHit) {
            break;
        }

        step++;
        currentTime += droneConfig.simTimeStep;
    }

    // write output
    json outData;
    outData["totalSteps"] = step;
    outData["steps"] = json::array();
    for (int i = 0; i < step; i++) {
        json step;
        step["position"] = {{"x", out[i].pos.x}, {"y", out[i].pos.y}};
        step["direction"] = out[i].direction;
        step["state"] = out[i].state;
        step["targetIndex"] = out[i].targetIdx;
        step["dropPoint"] = {{"x", out[i].dropPoint.x}, {"y", out[i].dropPoint.y}};
        step["aimPoint"] = {{"x", out[i].aimPoint.x}, {"y", out[i].aimPoint.y}};
        step["predictedTarget"] = {{"x", out[i].predictedTarget.x}, {"y", out[i].predictedTarget.y}};
        outData["steps"].push_back(step);
    }
    std::ofstream simulationLile("homework_03/data/simulation.json");

    if (!simulationLile.is_open()) {
        std::cerr << "Unable to open ammo file\n";

        return 1;
    }

    simulationLile << outData.dump(2);
    simulationLile.close();

    // clean resources
    delete[] ammos;

    for (int i = 0; i < targetsCount; i++)
        delete[] targets[i];
    delete[] targets;
}

Coord interpolateTarget(const Coord* target, float time, float arrayTimeStep)
{
    int _current = (int)floor(time / arrayTimeStep) % 60;
    int _next = (_current + 1) % 60;
    float frac = (time - _current * arrayTimeStep) / arrayTimeStep;
    Coord current = target[_current];
    Coord next = target[_next];

    return {current.x + (next.x - current.x) * frac, current.y + (next.y - current.y) * frac};
}

double calcDropTime(const AmmoParams& ammo, double v0, double z0)
{
    double a = ammo.drag * G * ammo.mass - 2.0 * ammo.drag * ammo.drag * ammo.lift * v0;
    double b = -(3.0 * G * ammo.mass * ammo.mass) + 3.0 * (ammo.drag * ammo.lift * ammo.mass * v0);
    double c = 6.0 * ammo.mass * ammo.mass * z0;

    // cardano

    double p = -(b * b) / (3.0 * a * a);
    double q = ((2.0 * b * b * b) / (27.0 * a * a * a)) + c / a;

    if (p >= 0) {
        throw std::runtime_error("Wrong p calc");
    }

    double acosArg = 3.0 * q / (2.0 * p) * std::sqrt(-3.0 / p);

    if (acosArg < -1 && acosArg > 1) {
        throw std::runtime_error("Wrong acos calc");
    }

    double fi = std::acos(acosArg);
    double t = (2.0 * std::sqrt(-p / 3.0) * std::cos((fi + 4.0 * M_PI) / 3)) - (b / (3.0 * a));

    return t;
}

double calcDropDistance(double t, const AmmoParams& ammo, double v0)
{
    double l = ammo.lift;
    double l2 = l * l;
    double l3 = l2 * l;
    double l4 = l3 * l;
    double ll2 = l2 + 1;

    double t2 = t * t;
    double t3 = t2 * t;
    double t4 = t3 * t;
    double t5 = t4 * t;

    double d = ammo.drag;
    double d2 = d * d;
    double d3 = d2 * d;
    double d4 = d3 * d;

    double m = ammo.mass;
    double m2 = m * m;
    double m3 = m2 * m;
    double m4 = m3 * m;

    double h1 = v0 * t;
    double h2 = (t2 * d * v0) / (2.0 * m);
    double h3 =
        t4 * ((3.0 * d3 * ll2 * l2 * v0) + (6.0 * d3 * l4 * ll2 * v0) - (6.0 * d2 * G * (l4 + ll2) * l * m)) / (36.0 * ll2 * ll2 * m3);
    double h4 = t5 * ((3.0 * d3 * G * l3 * m) - (3.0 * d4 * l2 * ll2 * v0)) / (36.0 * ll2 * m4);
    double h5 = t3 * ((6.0 * d * G * l * m) - (6.0 * d2 * (l2 - 1) * v0)) / (36.0 * m2);

    return h1 - h2 + h3 + h4 + h5;
}

inline void doAcceleration(SimStep& simStep, float acceleration, float time, float attackSpeed)
{
    float path = simStep.speed * time + 0.5f * acceleration * time * time;
    simStep.pos = simStep.pos.move(path, simStep.direction);

    simStep.speed += acceleration * time;

    if (simStep.speed >= attackSpeed) {
        simStep.speed = attackSpeed;
        simStep.state = MOVING;
    }
}

inline void doDeceleration(SimStep& simStep, float acceleration, float time)
{
    float path = simStep.speed * time - 0.5f * acceleration * time * time;
    simStep.pos = simStep.pos.move(path, simStep.direction);
    simStep.speed -= acceleration * time;

    if (simStep.speed <= 0.f) {
        simStep.state = STOPPED;
        simStep.speed = 0.f;
    }
}

inline void doMoving(SimStep& simStep, float time)
{
    float path = simStep.speed * time;
    simStep.pos = simStep.pos.move(path, simStep.direction);
}

inline void doTurning(SimStep& simStep, float turnAngle, float angleStep)
{
    simStep.direction = turnAngle > 0 ? simStep.direction + angleStep : simStep.direction - angleStep;
    simStep.state = std::fabs(turnAngle) >= angleStep ? TURNING : ACCELERATING;
}

inline float calcReEntryPath(
    float distanceToDropPoint, bool isNeedTurnAngle, float speed, float attackSpeed, float acceleration, float accelerationPath)
{
    // define reentry path
    float reEntryPath = distanceToDropPoint < 0 ? -distanceToDropPoint : 0.f;

    if (!isNeedTurnAngle) {
        if (speed < attackSpeed) {
            float accelerationPath = (attackSpeed * attackSpeed - speed * speed) / (2 * acceleration);

            if (distanceToDropPoint - accelerationPath < 0) {
                reEntryPath += accelerationPath;
            }
        }
    }
    else {
        float stopingPath = 0.f;

        if (speed > 0) {
            stopingPath = (speed * speed) / (2 * acceleration);
        }

        if (distanceToDropPoint < stopingPath + accelerationPath) {
            reEntryPath = reEntryPath + stopingPath + accelerationPath;
        }
    }

    return reEntryPath;
}

inline float calcReEntryTime(float speed,
                             float angularSpeed,
                             float turnAngle,
                             float acceleration,
                             float accelerationPath,
                             float fullAccelerationTime,
                             float attackSpeed,
                             float reEntryPath)
{
    float reEntryTimeToTurn = (2 * M_PI) / angularSpeed + turnAngle;  // 360
    float timeToStop = 0.f;
    float stopingPath = 0.f;

    if (speed > 0) {
        timeToStop = speed / acceleration;
        stopingPath = (speed * speed) / (2 * acceleration);
    }

    float maneuverTime;

    if (reEntryPath > 2 * accelerationPath) {
        maneuverTime = fullAccelerationTime + fullAccelerationTime + (reEntryPath - 2 * accelerationPath) / attackSpeed;
    }
    else {
        maneuverTime = std::sqrt(reEntryPath / acceleration);
    }

    return timeToStop + reEntryTimeToTurn + maneuverTime + fullAccelerationTime;
}

inline float calcEntryTime(int state,
                           float distanceToDropPoint,
                           bool isNeedTurnAngle,
                           float speed,
                           float attackSpeed,
                           float acceleration,
                           float accelerationPath,
                           float fullAccelerationTime)
{
    float timeToStop{0.f};
    float timeToAccelerate{0.f};
    float timeToMove{0.f};

    // target Entry calculation
    if (state == STOPPED || state == TURNING) {
        timeToAccelerate = fullAccelerationTime;
        timeToMove = (distanceToDropPoint - accelerationPath) / attackSpeed;
    }

    if (!isNeedTurnAngle) {
        if (state == ACCELERATING || state == DECELERATING) {
            // calculate path to accelerate from current speed
            timeToAccelerate = (attackSpeed - speed) / acceleration;
            float accelerationPath = (attackSpeed * attackSpeed - speed * speed) / (2 * acceleration);
            timeToMove = (distanceToDropPoint - accelerationPath) / attackSpeed;
        }
    }
    else {
        if (state == MOVING) {
            timeToStop = fullAccelerationTime;
            timeToAccelerate = fullAccelerationTime;
            timeToMove = (distanceToDropPoint - accelerationPath - accelerationPath) / attackSpeed;
        }

        if (state == DECELERATING || state == ACCELERATING) {
            timeToStop = speed / acceleration;
            timeToAccelerate = fullAccelerationTime;
            float stopingPath = (speed * speed) / (2 * acceleration);
            timeToMove = (distanceToDropPoint - stopingPath - accelerationPath) / attackSpeed;
        }
    }

    return timeToStop + timeToAccelerate + timeToMove;
}

void from_json(const json& j, DroneConfig& droneConfig)
{
    droneConfig.startPos = {j["drone"]["position"]["x"], j["drone"]["position"]["y"]};
    const char* ammoText = j["ammo"].get_ref<const std::string&>().c_str();
    std::strncpy(droneConfig.ammoName, ammoText, 31);
    droneConfig.altitude = j["drone"]["altitude"];
    droneConfig.initialDir = j["drone"]["initialDirection"];
    droneConfig.attackSpeed = j["drone"]["attackSpeed"];
    droneConfig.accelerationPath = j["drone"]["accelerationPath"];
    droneConfig.arrayTimeStep = j["targetArrayTimeStep"];
    droneConfig.simTimeStep = j["simulation"]["timeStep"];
    droneConfig.hitRadius = j["simulation"]["hitRadius"];
    droneConfig.turnThreshold = j["drone"]["turnThreshold"];
    droneConfig.angularSpeed = j["drone"]["angularSpeed"];
}

AmmoParams* loadAmmoParameters(std::ifstream& ammoFile, int& outCount)
{
    json ammoConfig = json::parse(ammoFile);

    if (!ammoConfig.is_array()) {
        outCount = 0;

        return nullptr;
    }

    outCount = ammoConfig.size();
    AmmoParams* ammoList = new AmmoParams[outCount];

    for (int i = 0; i < outCount; i++) {
        const char* ammoName = ammoConfig[i]["name"].get_ref<const std::string&>().c_str();
        ammoList[i] = {.mass = ammoConfig[i]["mass"], .drag = ammoConfig[i]["drag"], .lift = ammoConfig[i]["lift"]};
        std::strncpy(ammoList[i].name, ammoName, 31);
    }

    return ammoList;
}

Coord** loadTargets(std::ifstream& targetsFile, int& outCount, int& outTimeSteps)
{
    json targetsJson = json::parse(targetsFile);

    outCount = targetsJson["targetCount"];
    outTimeSteps = targetsJson["timeSteps"];
    AmmoParams* ammoList = new AmmoParams[outCount];

    Coord** targets = new Coord*[outCount];
    for (int i = 0; i < outCount; i++) {
        targets[i] = new Coord[outTimeSteps];
        for (int j = 0; j < outTimeSteps; j++) {
            targets[i][j].x = targetsJson["targets"][i]["positions"][j]["x"];
            targets[i][j].y = targetsJson["targets"][i]["positions"][j]["y"];
        }
    }

    return targets;
}