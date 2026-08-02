#include <gtest/gtest.h>
#include "ballistics.hpp"

struct BallisticTestCase {
    char ammoName[32];
    float expectedTime;
    float expectedDistance;
};

struct FireTestCase {
    Coord drone;
    Coord target;
    float expectedDistance;
    Coord expectedFire;
    Coord expectedIntermediate;
};

TEST(Ballistics, ComputeBallisticParametersPositive)
{
    float v0 = 10.0F;
    float z0 = 100.0F;
    constexpr float eps = 1.0e-4F;

    BallisticTestCase testCases[] = {{"VOG-17", 5.74975824F, 37.1102219F},
                                     {"M67", 5.39585114F, 36.9690475F},
                                     {"RKG-3", 4.85437393F, 40.0490074F},
                                     {"GLIDING-VOG", 6.26317024F, 59.8834152F},
                                     {"GLIDING-RKG", 4.9677F, 52.0328293F}};

    for (const auto& testCase : testCases) {
        BallisticParameters dropParams = calc_ballistic_parameters(testCase.ammoName, v0, z0);

        EXPECT_NEAR(dropParams.distance, testCase.expectedDistance, eps);
        EXPECT_NEAR(dropParams.time, testCase.expectedTime, eps);
    }
}

TEST(Ballistics, ComputeFireParametersPositive)
{
    float accelerationPath = 10.0F;
    // float z0 = 100.0F;
    constexpr float eps = 1.0e-4F;
    BallisticParameters dropParams = {10.0F, 50.0F};

    FireTestCase testCases[] = {{{0.F, 0.F}, {100.F, 100.F}, 141.421356F, {64.6446609F, 64.6446609F}, {0.F, 0.F}},
                                {{70.F, 70.F}, {100.F, 100.F}, 42.426406F, {64.6446609F, 64.6446609F}, {57.57359313F, 57.57359313F}}};

    for (const auto& testCase : testCases) {
        FireParameters fireParams = calc_fire_parameters(dropParams, testCase.drone, testCase.target, accelerationPath);

        EXPECT_NEAR(fireParams.distance, testCase.expectedDistance, eps);
        EXPECT_NEAR(fireParams.fire.x, testCase.expectedFire.x, eps);
        EXPECT_NEAR(fireParams.fire.y, testCase.expectedFire.y, eps);
        EXPECT_NEAR(fireParams.intermediate.x, testCase.expectedIntermediate.x, eps);
        EXPECT_NEAR(fireParams.intermediate.y, testCase.expectedIntermediate.y, eps);
    }
}

TEST(Ballistics, BallisticParametersWrongAmmoNameNegative)
{
    float v0 = 10.0F;
    float z0 = 100.0F;

    EXPECT_THROW(calc_ballistic_parameters("wrong_ammo", v0, z0), std::runtime_error);
}

TEST(Ballistics, BallisticParametersDropParametersCalcNegative)
{
    float v0 = 10.0F;
    float z0 = 2000.0F;

    EXPECT_THROW(calc_ballistic_parameters("GLIDING-RKG", v0, z0), std::runtime_error);
}

TEST(Ballistics, BallisticParametersDropInputParametersNegative)
{
    float v0 = -10.0F;
    float z0 = 0.0F;

    EXPECT_THROW(calc_ballistic_parameters("GLIDING-RKG", v0, z0), std::runtime_error);
}
