# libanimate Implementation Plan

## Overview

This document provides a detailed implementation plan for extracting FroggyFrag's animation subsystem into a standalone C library called `libanimate`. The plan follows strict C coding standards and addresses all critical issues identified in the code review.

---

## 1. Project Structure

```
libanimate/
├── include/
│   └── libanimate.h          # Public API header
├── src/
│   ├── animate.c             # Core implementation
│   ├── animate_easing.c      # Easing functions
│   ├── animate_easing.h      # Internal easing header
│   ├── animate_value.c       # Value type system
│   └── animate_value.h       # Internal value header
├── tests/
│   ├── unity/                # Unity test framework
│   ├── test_context.c        # Context tests
│   ├── test_chain.c          # Animation chain tests
│   ├── test_easing.c         # Easing function tests
│   ├── test_value.c          # Value type tests
│   └── main.c                # Test runner
├── examples/
│   ├── basic.c               # Basic usage example
│   └── raylib_integration.c  # FroggyFrag integration
├── meson.build               # Root build file
└── README.md
```

---

## 2. Public API Header (include/libanimate.h)

```c
#ifndef LIBANIMATE_H
#define LIBANIMATE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Version Information
 * ============================================================================ */
#define ANIMATE_VERSION_MAJOR 1
#define ANIMATE_VERSION_MINOR 0
#define ANIMATE_VERSION_PATCH 0

/* ============================================================================
 * Opaque Types
 * ============================================================================ */
typedef struct animate_context animate_context_t;
typedef uint32_t animate_chain_handle_t;
typedef uint32_t animate_group_handle_t;

#define ANIMATE_INVALID_HANDLE 0

/* ============================================================================
 * Result Codes
 * ============================================================================ */
typedef enum {
    ANIMATE_OK = 0,
    ANIMATE_ERROR_NULL_POINTER = -1,
    ANIMATE_ERROR_OUT_OF_MEMORY = -2,
    ANIMATE_ERROR_INVALID_HANDLE = -3,
    ANIMATE_ERROR_INVALID_PARAMETER = -4,
    ANIMATE_ERROR_CAPACITY_EXCEEDED = -5,
    ANIMATE_ERROR_NOT_FOUND = -6
} animate_result_t;

/* ============================================================================
 * Value Types
 * ============================================================================ */
typedef enum {
    ANIMATE_VALUE_FLOAT,
    ANIMATE_VALUE_INT,
    ANIMATE_VALUE_VEC2,
    ANIMATE_VALUE_VEC3,
    ANIMATE_VALUE_VEC4,
    ANIMATE_VALUE_COLOR_RGBA,
    ANIMATE_VALUE_CUSTOM
} animate_value_type_t;

typedef struct {
    animate_value_type_t type;
    union {
        float f;
        int i;
        struct { float x; float y; } vec2;
        struct { float x; float y; float z; } vec3;
        struct { float x; float y; float z; float w; } vec4;
        struct { float r; float g; float b; float a; } color;
        void *custom;
    } value;
} animate_value_t;

/* ============================================================================
 * Easing Types
 * ============================================================================ */
typedef enum {
    ANIMATE_EASE_LINEAR,
    ANIMATE_EASE_QUINT_IN,
    ANIMATE_EASE_QUINT_OUT,
    ANIMATE_EASE_QUINT_INOUT,
    ANIMATE_EASE_CUBIC_IN,
    ANIMATE_EASE_CUBIC_OUT,
    ANIMATE_EASE_CUBIC_INOUT,
    ANIMATE_EASE_SINE_IN,
    ANIMATE_EASE_SINE_OUT,
    ANIMATE_EASE_SINE_INOUT,
    ANIMATE_EASE_ELASTIC_IN,
    ANIMATE_EASE_ELASTIC_OUT,
    ANIMATE_EASE_ELASTIC_INOUT,
    ANIMATE_EASE_BOUNCE_IN,
    ANIMATE_EASE_BOUNCE_OUT,
    ANIMATE_EASE_BOUNCE_INOUT,
    ANIMATE_EASE_CIRC_IN,
    ANIMATE_EASE_CIRC_OUT,
    ANIMATE_EASE_CIRC_INOUT,
    ANIMATE_EASE_COUNT
} animate_ease_type_t;

typedef float (*animate_ease_func_t)(float t);

/* ============================================================================
 * Playback Types
 * ============================================================================ */
typedef enum {
    ANIMATE_PLAY_ONCE,
    ANIMATE_PLAY_LOOP,
    ANIMATE_PLAY_PINGPONG,
    ANIMATE_PLAY_LOOP_PINGPONG
} animate_playback_t;

typedef enum {
    ANIMATE_GROUP_SEQUENTIAL,
    ANIMATE_GROUP_PARALLEL
} animate_group_type_t;

typedef enum {
    ANIMATE_RETRIG_NONE,
    ANIMATE_RETRIG_RESTART,
    ANIMATE_RETRIG_RESTART_SEGMENT,
    ANIMATE_RETRIG_JUMP_TO_END
} animate_retrig_t;

/* ============================================================================
 * Event System
 * ============================================================================ */
typedef enum {
    ANIMATE_EVENT_START,
    ANIMATE_EVENT_SEGMENT_START,
    ANIMATE_EVENT_SEGMENT_END,
    ANIMATE_EVENT_COMPLETE,
    ANIMATE_EVENT_LOOP
} animate_event_t;

typedef void (*animate_callback_t)(
    animate_chain_handle_t chain,
    animate_event_t event,
    void *user_data
);

/* ============================================================================
 * Allocator Interface
 * ============================================================================ */
typedef struct {
    void *(*alloc)(size_t size, void *user_data);
    void (*free)(void *ptr, void *user_data);
    void *user_data;
} animate_allocator_t;

/* ============================================================================
 * Context Management
 * ============================================================================ */

/**
 * @brief Create a new animation context with default allocator.
 *
 * @param out_context Pointer to receive the new context.
 * @return ANIMATE_OK on success, error code otherwise.
 */
animate_result_t animate_context_create(animate_context_t **out_context);

/**
 * @brief Create a new animation context with custom allocator.
 *
 * @param allocator Custom allocator functions.
 * @param out_context Pointer to receive the new context.
 * @return ANIMATE_OK on success, error code otherwise.
 */
animate_result_t animate_context_create_ex(
    const animate_allocator_t *allocator,
    animate_context_t **out_context
);

/**
 * @brief Destroy an animation context and all associated resources.
 *
 * @param context The context to destroy.
 */
void animate_context_destroy(animate_context_t *context);

/**
 * @brief Update all animations in the context.
 *
 * @param context The animation context.
 * @param delta_time_seconds Time elapsed since last update in seconds.
 * @return ANIMATE_OK on success, error code otherwise.
 */
animate_result_t animate_tick(
    animate_context_t *context,
    double delta_time_seconds
);

/* ============================================================================
 * Chain Management
 * ============================================================================ */

/**
 * @brief Create a new animation chain.
 *
 * @param context The animation context.
 * @param playback Playback behavior for the chain.
 * @param retrig Retrigger behavior when started while playing.
 * @param out_handle Pointer to receive the chain handle.
 * @return ANIMATE_OK on success, error code otherwise.
 */
animate_result_t animate_chain_create(
    animate_context_t *context,
    animate_playback_t playback,
    animate_retrig_t retrig,
    animate_chain_handle_t *out_handle
);

/**
 * @brief Destroy an animation chain.
 *
 * @param context The animation context.
 * @param chain The chain handle to destroy.
 * @return ANIMATE_OK on success, error code otherwise.
 */
animate_result_t animate_chain_destroy(
    animate_context_t *context,
    animate_chain_handle_t chain
);

/**
 * @brief Add a float animation segment to a chain.
 *
 * @param context The animation context.
 * @param chain The target chain.
 * @param target Pointer to the float value to animate.
 * @param dest The destination value.
 * @param duration_seconds Duration of the animation.
 * @param easing The easing function to use.
 * @return ANIMATE_OK on success, error code otherwise.
 */
animate_result_t animate_chain_add_float(
    animate_context_t *context,
    animate_chain_handle_t chain,
    float *target,
    float dest,
    double duration_seconds,
    animate_ease_type_t easing
);

/**
 * @brief Add an int animation segment to a chain.
 *
 * @param context The animation context.
 * @param chain The target chain.
 * @param target Pointer to the int value to animate.
 * @param dest The destination value.
 * @param duration_seconds Duration of the animation.
 * @param easing The easing function to use.
 * @return ANIMATE_OK on success, error code otherwise.
 */
animate_result_t animate_chain_add_int(
    animate_context_t *context,
    animate_chain_handle_t chain,
    int *target,
    int dest,
    double duration_seconds,
    animate_ease_type_t easing
);

/**
 * @brief Add a vec2 animation segment to a chain.
 *
 * @param context The animation context.
 * @param chain The target chain.
 * @param target Pointer to the vec2 value to animate.
 * @param dest_x Destination X value.
 * @param dest_y Destination Y value.
 * @param duration_seconds Duration of the animation.
 * @param easing The easing function to use.
 * @return ANIMATE_OK on success, error code otherwise.
 */
animate_result_t animate_chain_add_vec2(
    animate_context_t *context,
    animate_chain_handle_t chain,
    float *target_x,
    float *target_y,
    float dest_x,
    float dest_y,
    double duration_seconds,
    animate_ease_type_t easing
);

/**
 * @brief Add a rest (delay) segment to a chain.
 *
 * @param context The animation context.
 * @param chain The target chain.
 * @param duration_seconds Duration of the rest.
 * @return ANIMATE_OK on success, error code otherwise.
 */
animate_result_t animate_chain_add_rest(
    animate_context_t *context,
    animate_chain_handle_t chain,
    double duration_seconds
);

/**
 * @brief Start playing an animation chain.
 *
 * @param context The animation context.
 * @param chain The chain to start.
 * @return ANIMATE_OK on success, error code otherwise.
 */
animate_result_t animate_chain_play(
    animate_context_t *context,
    animate_chain_handle_t chain
);

/**
 * @brief Stop an animation chain.
 *
 * @param context The animation context.
 * @param chain The chain to stop.
 * @return ANIMATE_OK on success, error code otherwise.
 */
animate_result_t animate_chain_stop(
    animate_context_t *context,
    animate_chain_handle_t chain
);

/**
 * @brief Reset an animation chain to initial state.
 *
 * @param context The animation context.
 * @param chain The chain to reset.
 * @return ANIMATE_OK on success, error code otherwise.
 */
animate_result_t animate_chain_reset(
    animate_context_t *context,
    animate_chain_handle_t chain
);

/**
 * @brief Check if a chain is currently playing.
 *
 * @param context The animation context.
 * @param chain The chain to check.
 * @param out_playing Pointer to receive playing status.
 * @return ANIMATE_OK on success, error code otherwise.
 */
animate_result_t animate_chain_is_playing(
    animate_context_t *context,
    animate_chain_handle_t chain,
    bool *out_playing
);

/**
 * @brief Set a callback for chain events.
 *
 * @param context The animation context.
 * @param chain The chain to set callback for.
 * @param callback The callback function.
 * @param user_data User data passed to callback.
 * @return ANIMATE_OK on success, error code otherwise.
 */
animate_result_t animate_chain_set_callback(
    animate_context_t *context,
    animate_chain_handle_t chain,
    animate_callback_t callback,
    void *user_data
);

/* ============================================================================
 * Group Management
 * ============================================================================ */

/**
 * @brief Create a new animation group.
 *
 * @param context The animation context.
 * @param type Group type (sequential or parallel).
 * @param out_handle Pointer to receive the group handle.
 * @return ANIMATE_OK on success, error code otherwise.
 */
animate_result_t animate_group_create(
    animate_context_t *context,
    animate_group_type_t type,
    animate_group_handle_t *out_handle
);

/**
 * @brief Destroy an animation group.
 *
 * @param context The animation context.
 * @param group The group handle to destroy.
 * @return ANIMATE_OK on success, error code otherwise.
 */
animate_result_t animate_group_destroy(
    animate_context_t *context,
    animate_group_handle_t group
);

/**
 * @brief Add a chain to a group.
 *
 * @param context The animation context.
 * @param group The target group.
 * @param chain The chain to add.
 * @return ANIMATE_OK on success, error code otherwise.
 */
animate_result_t animate_group_add_chain(
    animate_context_t *context,
    animate_group_handle_t group,
    animate_chain_handle_t chain
);

/**
 * @brief Start all chains in a group.
 *
 * @param context The animation context.
 * @param group The group to start.
 * @return ANIMATE_OK on success, error code otherwise.
 */
animate_result_t animate_group_play(
    animate_context_t *context,
    animate_group_handle_t group
);

/**
 * @brief Stop all chains in a group.
 *
 * @param context The animation context.
 * @param group The group to stop.
 * @return ANIMATE_OK on success, error code otherwise.
 */
animate_result_t animate_group_stop(
    animate_context_t *context,
    animate_group_handle_t group
);

/* ============================================================================
 * Easing Functions
 * ============================================================================ */

/**
 * @brief Get an easing function by type.
 *
 * @param type The easing type.
 * @return Pointer to the easing function, or NULL if invalid.
 */
animate_ease_func_t animate_ease_get_function(animate_ease_type_t type);

/**
 * @brief Register a custom easing function.
 *
 * @param context The animation context.
 * @param func The custom easing function.
 * @param out_type Pointer to receive the assigned type ID.
 * @return ANIMATE_OK on success, error code otherwise.
 */
animate_result_t animate_ease_register_custom(
    animate_context_t *context,
    animate_ease_func_t func,
    animate_ease_type_t *out_type
);

#ifdef __cplusplus
}
#endif

#endif /* LIBANIMATE_H */
```

---

## 3. Implementation Details

### 3.1 Handle-Based Reference System

Replace raw pointers with handles to prevent use-after-free and enable internal memory management:

```c
/* Internal handle generation */
#define ANIMATE_MAX_CHAINS 256
#define ANIMATE_MAX_GROUPS 64

typedef struct {
    uint32_t index : 16;
    uint32_t generation : 16;
} animate_handle_internal_t;

/* Validate handle before dereferencing */
static bool validate_chain_handle(animate_context_t *ctx, animate_chain_handle_t handle) {
    animate_handle_internal_t h = { .index = handle & 0xFFFF, .generation = handle >> 16 };
    if (h.index >= ANIMATE_MAX_CHAINS) return false;
    if (ctx->chains[h.index].generation != h.generation) return false;
    if (!ctx->chains[h.index].active) return false;
    return true;
}
```

### 3.2 Time-Based Animation Core

```c
typedef struct {
    double start_time;
    double duration;
    double elapsed;
    bool running;
} animate_segment_base_t;

typedef struct {
    animate_segment_base_t base;
    float *target;
    float origin;
    float destination;
    animate_ease_type_t easing;
} animate_segment_float_t;

static void update_segment_float(animate_segment_float_t *seg, double current_time) {
    double elapsed = current_time - seg->base.start_time;
    if (elapsed >= seg->base.duration) {
        *seg->target = seg->destination;
        seg->base.running = false;
        return;
    }
    float t = (float)(elapsed / seg->base.duration);
    animate_ease_func_t ease = animate_ease_get_function(seg->easing);
    t = ease(t);
    *seg->target = seg->origin + (seg->destination - seg->origin) * t;
}
```

### 3.3 Context Structure

```c
struct animate_context {
    animate_allocator_t allocator;
    double current_time;

    /* Chain pool */
    animate_chain_t chains[ANIMATE_MAX_CHAINS];
    uint16_t chain_generations[ANIMATE_MAX_CHAINS];
    bool chain_active[ANIMATE_MAX_CHAINS];
    uint16_t free_chain_indices[ANIMATE_MAX_CHAINS];
    int free_chain_count;

    /* Group pool */
    animate_group_t groups[ANIMATE_MAX_GROUPS];
    uint16_t group_generations[ANIMATE_MAX_GROUPS];
    bool group_active[ANIMATE_MAX_GROUPS];
    uint16_t free_group_indices[ANIMATE_MAX_GROUPS];
    int free_group_count;

    /* Custom easing slots */
    animate_ease_func_t custom_easings[8];
    int custom_easing_count;
};
```

---

## 4. Migration Strategy for FroggyFrag

### Step 1: Add libanimate as Dependency

```meson
# FroggyFrag meson.build
libanimate_proj = subproject('libanimate')
libanimate_dep = libanimate_proj.get_variable('libanimate_dep')

dependencies: [raylib_dep, libanimate_dep]
```

### Step 2: Create Compatibility Layer (gui_animation.c)

```c
/* Wrapper functions to ease migration */
typedef animate_chain_handle_t animation_chain_t;
typedef animate_group_handle_t animation_group_t;

static animate_context_t *g_gui_animation_ctx = NULL;

void gui_animation_init(void) {
    animate_context_create(&g_gui_animation_ctx);
}

void gui_animation_shutdown(void) {
    animate_context_destroy(g_gui_animation_ctx);
}

void gui_animation_update(double delta_time) {
    animate_tick(g_gui_animation_ctx, delta_time);
}
```

### Step 3: Update createSettingsNotificationBox

```c
TextBox *createSettingsNotificationBox(int x, int y, int w, int h,
                                       char *text, Color bgCol, Color txtCol) {
    /* ... setup code ... */

    animate_group_handle_t pop_up;
    animate_group_create(g_gui_animation_ctx, ANIMATE_GROUP_PARALLEL, &pop_up);

    animate_chain_handle_t opacity_fade;
    animate_chain_create(g_gui_animation_ctx, ANIMATE_PLAY_ONCE,
                         ANIMATE_RETRIG_RESTART, &opacity_fade);

    unsigned char opacity_dest = 0;
    animate_chain_add_uchar(g_gui_animation_ctx, opacity_fade,
                            &d->cDefault.a, opacity_dest, 0.5,
                            ANIMATE_EASE_CUBIC_OUT);
    animate_chain_add_rest(g_gui_animation_ctx, opacity_fade, 1.0);
    animate_chain_add_uchar(g_gui_animation_ctx, opacity_fade,
                            &d->cDefault.a, 255, 0.5,
                            ANIMATE_EASE_CUBIC_OUT);

    animate_group_add_chain(g_gui_animation_ctx, pop_up, opacity_fade);

    /* ... rest of setup ... */
}
```

### Step 4: Remove Old Animation System

1. Delete `src/animation.c` and `src/animation.h`
2. Delete `src/ease.c` and `src/ease.h`
3. Update all includes from `"animation.h"` to `<libanimate.h>`
4. Update build system

---

## 5. Testing Strategy

### Unit Tests (Unity Framework)

```c
/* tests/test_chain.c */
#include "unity.h"
#include "libanimate.h"

static animate_context_t *ctx = NULL;

void setUp(void) {
    animate_context_create(&ctx);
}

void tearDown(void) {
    animate_context_destroy(ctx);
    ctx = NULL;
}

void test_chain_create_destroy(void) {
    animate_chain_handle_t chain;
    animate_result_t result = animate_chain_create(ctx, ANIMATE_PLAY_ONCE,
                                                   ANIMATE_RETRIG_NONE, &chain);
    TEST_ASSERT_EQUAL(ANIMATE_OK, result);
    TEST_ASSERT_NOT_EQUAL(ANIMATE_INVALID_HANDLE, chain);

    result = animate_chain_destroy(ctx, chain);
    TEST_ASSERT_EQUAL(ANIMATE_OK, result);
}

void test_chain_float_animation(void) {
    animate_chain_handle_t chain;
    animate_chain_create(ctx, ANIMATE_PLAY_ONCE, ANIMATE_RETRIG_NONE, &chain);

    float value = 0.0f;
    animate_chain_add_float(ctx, chain, &value, 100.0f, 1.0, ANIMATE_EASE_LINEAR);
    animate_chain_play(ctx, chain);

    /* Before tick - should be at origin */
    TEST_ASSERT_EQUAL_FLOAT(0.0f, value);

    /* Halfway through */
    animate_tick(ctx, 0.5);
    TEST_ASSERT_EQUAL_FLOAT(50.0f, value);

    /* Complete */
    animate_tick(ctx, 0.5);
    TEST_ASSERT_EQUAL_FLOAT(100.0f, value);

    animate_chain_destroy(ctx, chain);
}

void test_chain_loop(void) {
    animate_chain_handle_t chain;
    animate_chain_create(ctx, ANIMATE_PLAY_LOOP, ANIMATE_RETRIG_NONE, &chain);

    float value = 0.0f;
    animate_chain_add_float(ctx, chain, &value, 100.0f, 1.0, ANIMATE_EASE_LINEAR);
    animate_chain_play(ctx, chain);

    /* Complete first iteration */
    animate_tick(ctx, 1.0);
    TEST_ASSERT_EQUAL_FLOAT(100.0f, value);

    /* Should have looped back */
    bool playing;
    animate_chain_is_playing(ctx, chain, &playing);
    TEST_ASSERT_TRUE(playing);

    animate_chain_destroy(ctx, chain);
}

void test_null_pointer_checks(void) {
    animate_result_t result;

    result = animate_context_create(NULL);
    TEST_ASSERT_EQUAL(ANIMATE_ERROR_NULL_POINTER, result);

    result = animate_tick(NULL, 0.1);
    TEST_ASSERT_EQUAL(ANIMATE_ERROR_NULL_POINTER, result);
}

void test_invalid_handle(void) {
    animate_result_t result;
    bool playing;

    result = animate_chain_is_playing(ctx, ANIMATE_INVALID_HANDLE, &playing);
    TEST_ASSERT_EQUAL(ANIMATE_ERROR_INVALID_HANDLE, result);

    result = animate_chain_destroy(ctx, 0xDEADBEEF);
    TEST_ASSERT_EQUAL(ANIMATE_ERROR_INVALID_HANDLE, result);
}
```

### Integration Test

```c
/* tests/test_integration.c */
void test_complex_animation_sequence(void) {
    animate_context_t *ctx;
    animate_context_create(&ctx);

    /* Create a complex UI animation like FroggyFrag's notification box */
    float x_pos = 0.0f, y_pos = 0.0f;
    float opacity = 0.0f;

    animate_group_handle_t group;
    animate_group_create(ctx, ANIMATE_GROUP_PARALLEL, &group);

    /* Fade in + slide in */
    animate_chain_handle_t fade_in;
    animate_chain_create(ctx, ANIMATE_PLAY_ONCE, ANIMATE_RETRIG_NONE, &fade_in);
    animate_chain_add_float(ctx, fade_in, &opacity, 1.0f, 0.3, ANIMATE_EASE_CUBIC_OUT);

    animate_chain_handle_t slide_in;
    animate_chain_create(ctx, ANIMATE_PLAY_ONCE, ANIMATE_RETRIG_NONE, &slide_in);
    animate_chain_add_float(ctx, slide_in, &y_pos, 100.0f, 0.5, ANIMATE_EASE_QUINT_OUT);

    animate_group_add_chain(ctx, group, fade_in);
    animate_group_add_chain(ctx, group, slide_in);

    /* Start and verify */
    animate_group_play(ctx, group);
    animate_tick(ctx, 0.3);

    TEST_ASSERT_FLOAT_WITHIN(0.01f, 1.0f, opacity);
    TEST_ASSERT_TRUE(y_pos > 0.0f);

    animate_context_destroy(ctx);
}
```

---

## 6. Build Configuration

### meson.build (Root)

```meson
project('libanimate', 'c',
    version: '1.0.0',
    default_options: [
        'c_std=c99',
        'warning_level=3',
        'werror=true'
    ]
)

# Compiler flags
cc = meson.get_compiler('c')
add_project_arguments(
    cc.get_supported_arguments([
        '-Wall',
        '-Wextra',
        '-Wpedantic',
        '-Wstrict-prototypes',
        '-Wmissing-prototypes',
        '-Wshadow',
        '-Wconversion',
        '-Wsign-conversion',
        '-Wfloat-equal',
        '-Wundef',
        '-Wcast-align',
        '-Wcast-qual'
    ]),
    language: 'c'
)

# Source files
libanimate_sources = files(
    'src/animate.c',
    'src/animate_easing.c',
    'src/animate_value.c'
)

# Include directories
libanimate_inc = include_directories('include', 'src')

# Library target
libanimate = library('animate',
    libanimate_sources,
    include_directories: libanimate_inc,
    install: true
)

# Dependency declaration for subproject use
libanimate_dep = declare_dependency(
    link_with: libanimate,
    include_directories: include_directories('include')
)

# Install headers
install_headers('include/libanimate.h')

# Tests
if get_option('tests')
    subdir('tests')
endif

# Examples
if get_option('examples')
    subdir('examples')
endif
```

### tests/meson.build

```meson
# Unity test framework
unity_sources = files('unity/src/unity.c')
unity_inc = include_directories('unity/src')

test_sources = files(
    'test_context.c',
    'test_chain.c',
    'test_easing.c',
    'test_value.c',
    'main.c'
)

test_exe = executable('libanimate_tests',
    test_sources + unity_sources,
    include_directories: [unity_inc, libanimate_inc],
    link_with: libanimate
)

test('libanimate_unit_tests', test_exe)
```

---

## 7. Static Analysis Configuration

### .clang-tidy

```yaml
---
Checks: >
    bugprone-*,
    clang-analyzer-*,
    cppcoreguidelines-*,
    misc-*,
    performance-*,
    portability-*,
    readability-*,
    -cppcoreguidelines-avoid-magic-numbers,
    -readability-named-parameter

WarningsAsErrors: '*'
HeaderFilterRegex: '.*'
AnalyzeTemporaryDtors: false
...
```

### CI/CD Pipeline (GitHub Actions)

```yaml
name: CI

on: [push, pull_request]

jobs:
  build-and-test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3

      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y meson ninja-build clang-tidy cppcheck valgrind

      - name: Configure
        run: meson setup builddir -Dtests=true

      - name: Build
        run: ninja -C builddir

      - name: Run tests
        run: ninja -C builddir test

      - name: Static analysis (clang-tidy)
        run: |
          find src -name '*.c' -exec clang-tidy {} \;

      - name: Static analysis (cppcheck)
        run: |
          cppcheck --enable=all --error-exitcode=1 src/

      - name: Memory check (valgrind)
        run: |
          valgrind --leak-check=full --error-exitcode=1 ./builddir/tests/libanimate_tests
```

---

## 8. Documentation

### README.md Template

```markdown
# libanimate

A lightweight, time-based animation library for C.

## Features

- Time-based animation (frame-rate independent)
- Type-safe API (no void* pointers)
- Handle-based memory management
- Custom allocator support
- 19 built-in easing functions
- Event/callback system
- Group-based animation coordination
- Zero external dependencies

## Quick Start

```c
#include <libanimate.h>

int main(void) {
    animate_context_t *ctx;
    animate_context_create(&ctx);

    float opacity = 0.0f;
    animate_chain_handle_t fade;
    animate_chain_create(ctx, ANIMATE_PLAY_ONCE, ANIMATE_RETRIG_NONE, &fade);
    animate_chain_add_float(ctx, fade, &opacity, 1.0f, 0.5, ANIMATE_EASE_OUT_CUBIC);
    animate_chain_play(ctx, fade);

    while (opacity < 1.0f) {
        animate_tick(ctx, 0.016); /* 60 FPS */
    }

    animate_context_destroy(ctx);
    return 0;
}
```

## Building

```bash
meson setup builddir
ninja -C builddir
ninja -C builddir test
```

## Integration

### As a Meson Subproject

```meson
libanimate = subproject('libanimate')
libanimate_dep = libanimate.get_variable('libanimate_dep')
```

### As a Static Library

Link against `libanimate.a` and include `libanimate.h`.

## API Documentation

See [API.md](docs/API.md) for complete documentation.

## License

MIT License - See LICENSE file for details.
```

---

## Summary

This implementation plan provides:

1. **Clean separation** from FroggyFrag with no dependencies
2. **Type-safe API** eliminating void* pointer issues
3. **Time-based animation** for frame-rate independence
4. **Handle-based references** for memory safety
5. **Comprehensive testing** with Unity framework
6. **Static analysis** integration (clang-tidy, cppcheck, valgrind)
7. **Clear migration path** for existing FroggyFrag code

The library will be production-ready with proper error handling, documentation, and test coverage.
