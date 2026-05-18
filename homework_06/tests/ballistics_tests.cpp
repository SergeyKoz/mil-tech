#include <gtest/gtest.h>
#include "ballistics.hpp"

struct TestCase {
    char ammoName[32];
    float expectedTime;
    float expectedDistance;
};

TEST(Ballistics, ComputePositive)
{
    float v0 = 10.0F;
    float z0 = 100.0F;
    constexpr float eps = 1.0e-4F;

    TestCase testCases[] = {{"VOG-17", 5.74975824F, 37.1102219F},
                            {"M67", 5.39585114F, 36.9690475F},
                            {"RKG-3", 4.85437393F, 40.0490074F},
                            {"GLIDING-VOG", 6.26317024F, 59.8834152F},
                            {"GLIDING-RKG", 4.9677F, 52.0328293F}};

    for (const auto& testCase : testCases) {
        DropParameters dropParams = calc_drop_parameters(testCase.ammoName, v0, z0);

        EXPECT_NEAR(dropParams.distance, testCase.expectedDistance, eps);
        EXPECT_NEAR(dropParams.time, testCase.expectedTime, eps);
    }
}

TEST(Ballistics, WrongAmmoNameNegative)
{
    float v0 = 10.0F;
    float z0 = 100.0F;

    EXPECT_THROW(calc_drop_parameters("wrong_ammo", v0, z0), std::runtime_error);
}

TEST(Ballistics, DropParametersCalcNegative)
{
    float v0 = 10.0F;
    float z0 = 2000.0F;

    EXPECT_THROW(calc_drop_parameters("GLIDING-RKG", v0, z0), std::runtime_error);
}

TEST(Ballistics, DropInputParametersNegative)
{
    float v0 = -10.0F;
    float z0 = 0.0F;

    EXPECT_THROW(calc_drop_parameters("GLIDING-RKG", v0, z0), std::runtime_error);
}
