#include "gtest/gtest.h"

#define DELTA 0.00001

extern "C" void create_exp_table();
extern "C" float pow2(float);

TEST(pow2, Zeroth) {
    float result = pow2(0);

    ASSERT_EQ(result, 1);
}

TEST(pow2, First) {
    float result = pow2(1);

    ASSERT_EQ(result, 2);
}

TEST(pow2, Inverse) {
    float result = pow2(-1);

    ASSERT_EQ(result, 0.5);
}

TEST(pow2, LargeExponent) {
    float result = pow2(10);

    ASSERT_EQ(result, 1024);
}

TEST(pow2, LargeNegativeExponent) {
    float result = pow2(-10);

    ASSERT_EQ(result, 1.0/1024);
}

TEST(pow2, Fractional) {
    float result = pow2(0.5);

    ASSERT_NEAR(result, 1.41421, DELTA);
}

TEST(pow2, NegFractional) {
    float result = pow2(-0.5);

    ASSERT_NEAR(result, 0.70711, DELTA);
}

TEST(pow2, SemitoneFraction) {
    float result = pow2(5 + 1.0/12);

    ASSERT_NEAR(result,33.90282, DELTA);
}

GTEST_API_ int main(int argc, char **argv) {
    // Set up the lookup table
    create_exp_table();

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
