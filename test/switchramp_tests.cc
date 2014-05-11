#include "gtest/gtest.h"

extern "C" {
    #include "../audio.h"
}

typedef struct {
    mod_handle_t in;
    int triggered;
    unsigned int time;
    float leftSample;
    float rightSample;
} switchramp_data_t;

extern "C" void switchramp_fillblock(int dummy, float* block, void* d);

#define DUMMY_MODULE 0

TEST(switchramp, PreservesInputWhenNotTriggered) {
    switchramp_data_t data;
    data.triggered = 0;

    float* block = mod_rdblock(DUMMY_MODULE);
    int i;
    for (i = 0; i < BLOCK_SIZE; i++) {
        block[i] = i;
    }

    float out[BLOCK_SIZE];
    switchramp_fillblock(DUMMY_MODULE, out, &data);

    for (i = 0; i < BLOCK_SIZE; i++) {
        ASSERT_EQ(i, out[i]);
    }

    ASSERT_EQ(i - 2, data.leftSample);
    ASSERT_EQ(i - 1, data.rightSample);
}

TEST(switchramp, RampsWhenTriggered) {
    switchramp_data_t data;
    data.triggered = 1;
    data.leftSample = 0;
    data.rightSample = 0;
    data.time = 4;

    float* block = mod_rdblock(DUMMY_MODULE);
    int i;
    for (i = 0; i < BLOCK_FRAMES; i++) {
        block[i * 2] = 1;
        block[i * 2 + 1] = -1;
    }

    float out[BLOCK_SIZE];
    switchramp_fillblock(DUMMY_MODULE, out, &data);

    ASSERT_EQ(0, out[0]);
    ASSERT_EQ(0.25, out[2]);
    ASSERT_EQ(0.5, out[4]);
    ASSERT_EQ(0.75, out[6]);
    ASSERT_EQ(1, out[8]);
    ASSERT_EQ(1, out[10]);

    ASSERT_EQ(0, out[1]);
    ASSERT_EQ(-0.25, out[3]);
    ASSERT_EQ(-0.5, out[5]);
    ASSERT_EQ(-0.75, out[7]);
    ASSERT_EQ(-1, out[9]);
    ASSERT_EQ(-1, out[11]);
}


GTEST_API_ int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
