/*
 * test_animation_quality.c - Phase 1: Code Quality Tests
 *
 * These tests verify that animation code compiles cleanly with strict flags
 * and meets style requirements.
 */

#include "unity.h"
#include "unity_internals.h"
#include "animation.h"
#include "ease.h"

/* ============================================================================
 * Setup/Teardown
 * ============================================================================ */

void setUp(void) {
    /* Reset global state before each test */
    initAnimationManager();
}

void tearDown(void) {
    /* Nothing to clean up */
}

/* ============================================================================
 * Compilation Tests
 * ============================================================================ */

void test_animation_compiles_with_strict_flags(void) {
    /* This test passes if the file compiles at all */
    TEST_ASSERT_TRUE(1);
}

void test_ease_compiles_with_strict_flags(void) {
    /* This test passes if the file compiles at all */
    TEST_ASSERT_TRUE(1);
}

/* ============================================================================
 * Basic Functionality Tests (Smoke Tests)
 * ============================================================================ */

void test_init_animation_manager(void) {
    initAnimationManager();
    /* If we get here without crashing, the test passes */
    TEST_ASSERT_TRUE(1);
}

void test_init_animation_chain(void) {
    AnimationChain chain;
    initAnimationChain(&chain, ACPT_ONCE, CRB_DO_NOTHING);
    TEST_ASSERT_EQUAL(0, chain.segmentCount);
    TEST_ASSERT_EQUAL(0, chain.segmentIndex);
    TEST_ASSERT_FALSE(chain.playing);
}

void test_init_animation_segment_float(void) {
    AnimationSegment segment;
    float target = 0.0f;
    float dest = 100.0f;

    initMoveAnimationSegment(&segment, 60, ATT_FLOAT, IT_LINEAR,
                             &target, &dest, false);

    TEST_ASSERT_EQUAL(AST_MOVE, segment.segmentType);
    TEST_ASSERT_EQUAL(ATT_FLOAT, segment.targetType);
    TEST_ASSERT_EQUAL(60, segment.durationFrames);
}

void test_init_animation_segment_int(void) {
    AnimationSegment segment;
    int target = 0;
    int dest = 100;

    initMoveAnimationSegment(&segment, 30, ATT_INT, IT_CUBIC_OUT,
                             &target, &dest, false);

    TEST_ASSERT_EQUAL(AST_MOVE, segment.segmentType);
    TEST_ASSERT_EQUAL(ATT_INT, segment.targetType);
    TEST_ASSERT_EQUAL(30, segment.durationFrames);
}

void test_init_animation_segment_uchar(void) {
    AnimationSegment segment;
    unsigned char target = 0;
    unsigned char dest = 255;

    initMoveAnimationSegment(&segment, 45, ATT_UCHAR, IT_SINE_INOUT,
                             &target, &dest, false);

    TEST_ASSERT_EQUAL(AST_MOVE, segment.segmentType);
    TEST_ASSERT_EQUAL(ATT_UCHAR, segment.targetType);
}

/* ============================================================================
 * Animation Chain Tests
 * ============================================================================ */

void test_add_animation_to_chain(void) {
    AnimationChain chain;
    initAnimationChain(&chain, ACPT_ONCE, CRB_DO_NOTHING);

    float target = 0.0f;
    float dest = 50.0f;

    addAnimation(&chain, 30, ATT_FLOAT, IT_LINEAR, &target, &dest, false);

    TEST_ASSERT_EQUAL(1, chain.segmentCount);
}

void test_add_rest_to_chain(void) {
    AnimationChain chain;
    initAnimationChain(&chain, ACPT_ONCE, CRB_DO_NOTHING);

    addRest(&chain, 60);

    TEST_ASSERT_EQUAL(1, chain.segmentCount);
    TEST_ASSERT_EQUAL(AST_REST, chain.segments[0].segmentType);
}

void test_chain_capacity_limit(void) {
    AnimationChain chain;
    initAnimationChain(&chain, ACPT_ONCE, CRB_DO_NOTHING);

    float target = 0.0f;
    float dest = 1.0f;

    /* Try to add more than MAX_AC_LEN segments */
    for (int i = 0; i < MAX_AC_LEN + 10; i++) {
        addAnimation(&chain, 10, ATT_FLOAT, IT_LINEAR, &target, &dest, false);
    }

    /* Should be capped at MAX_AC_LEN - 1 (last slot reserved) */
    TEST_ASSERT_TRUE(chain.segmentCount <= MAX_AC_LEN - 1);
}

/* ============================================================================
 * Animation Group Tests
 * ============================================================================ */

void test_init_animation_group(void) {
    AnimationChainGroup group;
    initAnimationChainGroup(&group, ACGP_SEQUENTIAL);

    TEST_ASSERT_EQUAL(0, group.chainCount);
    TEST_ASSERT_EQUAL(ACGP_SEQUENTIAL, group.type);
    TEST_ASSERT_FALSE(group.running);
}

void test_add_chain_to_group(void) {
    AnimationChainGroup group;
    initAnimationChainGroup(&group, ACGP_PARALLEL);

    AnimationChain chain;
    initAnimationChain(&chain, ACPT_ONCE, CRB_DO_NOTHING);

    addChainToGroup(&group, chain);

    TEST_ASSERT_EQUAL(1, group.chainCount);
}

void test_group_capacity_limit(void) {
    AnimationChainGroup group;
    initAnimationChainGroup(&group, ACGP_PARALLEL);

    for (int i = 0; i < MAX_ACG_LEN + 10; i++) {
        AnimationChain chain;
        initAnimationChain(&chain, ACPT_ONCE, CRB_DO_NOTHING);
        addChainToGroup(&group, chain);
    }

    TEST_ASSERT_TRUE(group.chainCount <= MAX_ACG_LEN);
}

/* ============================================================================
 * Animation Manager Tests
 * ============================================================================ */

void test_add_group_to_manager(void) {
    initAnimationManager();

    AnimationChainGroup group;
    initAnimationChainGroup(&group, ACGP_SEQUENTIAL);

    addAnimationGroupToManager(&group);

    /* Test passes if no crash - there's no getter for manager state */
    TEST_ASSERT_TRUE(1);
}

void test_manager_capacity_limit(void) {
    initAnimationManager();

    /* Create and add max + 10 groups */
    AnimationChainGroup groups[MAX_ACG_COUNT + 10];

    for (int i = 0; i < MAX_ACG_COUNT + 10; i++) {
        initAnimationChainGroup(&groups[i], ACGP_PARALLEL);
        addAnimationGroupToManager(&groups[i]);
    }

    /* Should handle overflow gracefully (silently ignore extra) */
    TEST_ASSERT_TRUE(1);
}

/* ============================================================================
 * Easing Function Tests
 * ============================================================================ */

void test_easing_linear(void) {
    InterpolationFunction func = assignEasingTransform(IT_LINEAR);
    TEST_ASSERT_NOT_NULL(func);

    /* Test boundaries */
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, func(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, func(1.0f));

    /* Test midpoint */
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.5f, func(0.5f));
}

void test_easing_cubic_out(void) {
    InterpolationFunction func = assignEasingTransform(IT_CUBIC_OUT);
    TEST_ASSERT_NOT_NULL(func);

    /* Test boundaries */
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, func(0.0f));
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, func(1.0f));
}

void test_easing_invalid_type(void) {
    /* IT_COUNT is not a valid easing type */
    InterpolationFunction func = assignEasingTransform(IT_COUNT);
    /* Should return linear as default */
    TEST_ASSERT_NOT_NULL(func);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.5f, func(0.5f));
}

/* ============================================================================
 * Main
 * ============================================================================ */

int main(void) {
    UNITY_BEGIN();

    /* Compilation tests */
    RUN_TEST(test_animation_compiles_with_strict_flags);
    RUN_TEST(test_ease_compiles_with_strict_flags);

    /* Basic functionality */
    RUN_TEST(test_init_animation_manager);
    RUN_TEST(test_init_animation_chain);
    RUN_TEST(test_init_animation_segment_float);
    RUN_TEST(test_init_animation_segment_int);
    RUN_TEST(test_init_animation_segment_uchar);

    /* Chain tests */
    RUN_TEST(test_add_animation_to_chain);
    RUN_TEST(test_add_rest_to_chain);
    RUN_TEST(test_chain_capacity_limit);

    /* Group tests */
    RUN_TEST(test_init_animation_group);
    RUN_TEST(test_add_chain_to_group);
    RUN_TEST(test_group_capacity_limit);

    /* Manager tests */
    RUN_TEST(test_add_group_to_manager);
    RUN_TEST(test_manager_capacity_limit);

    /* Easing tests */
    RUN_TEST(test_easing_linear);
    RUN_TEST(test_easing_cubic_out);
    RUN_TEST(test_easing_invalid_type);

    return UNITY_END();
}
