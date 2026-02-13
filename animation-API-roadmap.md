# Animation Subsystem Code Review & API Roadmap

## Executive Summary

This document provides a comprehensive analysis of the animation subsystem in FroggyFrag, identifying bugs, bad practices, architectural issues, and outlining a roadmap for extracting the subsystem into a standalone reusable library.

---

## Part 1: Code Review Findings

### CRITICAL ISSUES

#### 1. **Broken freeList Logic in AnimationManager** [CRITICAL]
**File:** `src/animation.c:262-272`
**Code:**
```c
void addAnimationGroupToManager(AnimationChainGroup *acg) {
    if(am.freeCount > 0) {
        *am.freeList[am.freeCount - 1] = acg;  // BUG: Wrong pointer type
        am.freeCount--;
    } else {
        if(am.count < MAX_ACG_COUNT) {
            am.list[am.count] = acg;
            am.count++;
        }
    }
}
```
**Issue:** The freeList is declared as `AnimationChainGroup **freeList[MAX_ACG_COUNT]` (array of pointer-to-pointer), but the logic treats it as a slot recycling mechanism. The assignment `*am.freeList[am.freeCount - 1] = acg` dereferences and writes to an arbitrary memory location. Additionally, there is no function to remove groups from the manager, making the freeList useless.

**Fix:** Remove freeList entirely or implement proper slot management with remove functionality:
```c
void addAnimationGroupToManager(AnimationChainGroup *acg) {
    if(am.count < MAX_ACG_COUNT) {
        am.list[am.count] = acg;
        am.count++;
    }
}
```

---

#### 2. **Stack Use-After-Return Risk in addChainToGroup** [CRITICAL]
**File:** `src/animation.c:226-231`
**Code:**
```c
void addChainToGroup(AnimationChainGroup *acg, AnimationChain ac) {
    if(acg->chainCount < MAX_ACG_LEN) {
        acg->chains[acg->chainCount] = ac;  // Copies by value
        acg->chainCount++;
    }
}
```
**Issue:** The function takes `AnimationChain` by value and copies it into the array. When called with a local stack variable (as seen in gui.c), the copy is valid, but this pattern is fragile. More critically, if any pointer inside AnimationChain pointed to stack data, it would dangle.

**Evidence from gui.c:187:**
```c
AnimationChain *opacityFade = (AnimationChain *)malloc(sizeof(AnimationChain));
initAnimationChain(opacityFade, ACPT_ONCE, restartBehaviour);
// ...
addChainToGroup(popUp, *opacityFade);  // Dereferences then copies
```

While this specific case is safe (the chain is heap-allocated), the API encourages dangerous patterns.

**Fix:** Change API to accept pointer and transfer ownership:
```c
void addChainToGroup(AnimationChainGroup *acg, AnimationChain *ac) {
    if(acg->chainCount < MAX_ACG_LEN) {
        memcpy(&acg->chains[acg->chainCount], ac, sizeof(AnimationChain));
        acg->chainCount++;
    }
}
```

---

#### 3. **No NULL Checks on malloc Results** [HIGH]
**File:** `src/gui.c` (multiple locations)
**Code:**
```c
// gui.c:177
AnimationChainGroup *popUp = (AnimationChainGroup *)malloc(sizeof(AnimationChainGroup));
initAnimationChainGroup(popUp, ACGP_SEQUENTIAL);  // No NULL check
```

**Issue:** Multiple malloc calls in gui.c lack NULL checks, leading to potential NULL pointer dereferences under memory pressure.

**Fix:** Add NULL checks after every malloc:
```c
AnimationChainGroup *popUp = malloc(sizeof(AnimationChainGroup));
if(!popUp) {
    fprintf(stderr, "ERROR: Failed to allocate AnimationChainGroup\n");
    return NULL;
}
```

---

#### 4. **Memory Leaks in Animation Creation** [HIGH]
**File:** `src/gui.c:167-238`
**Issue:** `createSettingsNotificationBox` allocates multiple AnimationChain and AnimationChainGroup objects but stores only the groups in the manager. The individual AnimationChain objects (opacityFade, positionSlide, frogWiggle, etc.) are copied by value into groups but their heap allocations are never freed. The pointers to these chains are lost after function return.

**Fix:** Either:
1. Use stack-allocated chains (they're copied anyway)
2. Document ownership transfer and free source after copy
3. Change design to use pointers throughout (preferred for library)

---

### ARCHITECTURAL ISSUES

#### 5. **Frame-Based Timing Instead of Delta Time** [HIGH]
**File:** `src/animation.c:93-119`
**Code:**
```c
bool processMove(void *self, int frameCount) {
    AnimationSegment *as = (AnimationSegment *)self;
    int currentTime = frameCount - as->startFrames;
    // ...
    float t = (float)currentTime / (float)as->durationFrames;
```

**Issue:** Animations use frame counts for timing, making them frame-rate dependent. A 60-frame animation takes 1 second at 60 FPS but 2 seconds at 30 FPS. This is unacceptable for a general-purpose animation library.

**Fix:** Use time-based animation with delta time:
```c
typedef struct {
    double startTime;
    double durationSeconds;
    // ...
} AnimationSegment;

float t = (float)((currentTime - as->startTime) / as->durationSeconds);
```

---

#### 6. **Global Singleton State** [MEDIUM]
**File:** `src/animation.c:5-6`
**Code:**
```c
AnimationManager am;
int frameCounter = 0;
```

**Issue:** Global state prevents multiple independent animation contexts, complicates testing, and creates hidden dependencies. No cleanup function exists.

**Fix:** Pass context explicitly:
```c
typedef struct AnimationContext AnimationContext;
AnimationContext *animation_context_create(void);
void animation_context_destroy(AnimationContext *ctx);
void animation_context_tick(AnimationContext *ctx, double deltaTime);
```

---

#### 7. **Type Safety Violations via void Pointers** [MEDIUM]
**File:** `src/animation.c:23-64`
**Code:**
```c
void initMoveAnimationSegment(AnimationSegment *mas, int durationFrames,
    AnimationTargetType att, InterpolationType it,
    void *target, void *dest, bool reverse) {
    // ...
    mas->target.i = (int *)target;
    // ...
}
```

**Issue:** The API uses `void *` for target and destination values, discarding type safety. Mismatches between `AnimationTargetType` and actual pointer types cause undefined behavior.

**Fix:** Use type-specific functions or macros:
```c
void initMoveAnimationSegmentInt(AnimationSegment *mas, int duration,
    InterpolationType it, int *target, int dest, bool reverse);
void initMoveAnimationSegmentFloat(AnimationSegment *mas, int duration,
    InterpolationType it, float *target, float dest, bool reverse);
```

---

#### 8. **Unimplemented Functions** [MEDIUM]
**File:** `src/gui.c:71-75`
**Code:**
```c
void createPopDownAnimation(Animateable *a, bool *trigger, Rectangle *toAnimate, Rectangle offset, int transitionFrames) {
}

void createDancingFroggyAnimation(Animateable *a, bool *trigger, AnimatedImage *toAnimate, int transitionFrames) {
}
```

**Issue:** Empty stub functions suggest incomplete API design.

---

#### 9. **Broken togglePauseGroup Function** [MEDIUM]
**File:** `src/animation.c:245-249`
**Code:**
```c
void togglePauseGroup(AnimationChainGroup *acg) { // do not use, needs to be fixed.
    for(int i = 0; i < acg->chainCount; i++) {
        acg->chains[i].playing = !acg->chains[i].playing;
    }
}
```

**Issue:** Comment admits this is broken. Toggling playing state without adjusting startFrames causes time jumps when resumed.

**Fix:** Implement proper pause with time offset tracking or remove the function.

---

### CODE QUALITY ISSUES

#### 10. **Magic Numbers Without Documentation** [LOW]
**File:** `src/animation.c:143, 166, 191`
**Code:**
```c
void setCustomRetrigAnimation(AnimationChainGroup *ac, int duration, ...) {
    initMoveAnimationSegment(&ac->segments[MAX_AC_LEN - 1], ...);
    // MAX_AC_LEN - 1 is reserved for custom interrupt
}
```

**Issue:** The special index `MAX_AC_LEN - 1` is used for custom interrupt animations but this is only documented in the comment on the define, not at usage sites.

---

#### 11. **Unused Code** [LOW]
**File:** `src/animation.c:89-91`
**Code:**
```c
bool processBlank(void *self, int frameCount) {
    return false;
}
```

**Issue:** Function defined but never used.

---

#### 12. **Inconsistent Error Handling** [MEDIUM]
**File:** `src/animation.c:129-141`
**Code:**
```c
void addAnimation(AnimationChain *ac, int duration, AnimationTargetType t, ...) {
    if(ac->segmentCount < MAX_AC_LEN - 1) {
        initMoveAnimationSegment(&ac->segments[ac->segmentCount], ...);
        ac->segmentCount++;
    }
    // Silent failure if at capacity
}
```

**Issue:** Functions silently fail when capacity is reached. No way for caller to detect failure.

**Fix:** Return success/failure indicator:
```c
bool addAnimation(AnimationChain *ac, int duration, ...);
```

---

#### 13. **Debug Printf Left in Production Code** [LOW]
**File:** `src/animation.c:38`
**Code:**
```c
printf("TARGET: %i, ORIGINAL: %i, DESTINATION: %i\n", *mas->target.i, mas->orig.i, mas->dest.i);
```

**Issue:** Debug output should be conditional or removed.

---

### STYLE & FORMATTING ISSUES

#### 14. **Inconsistent Brace Style** [LOW]
**File:** `src/animation.c`
**Issue:** Mix of K&R and Allman styles. Function definitions use K&R but some control structures are inconsistent.

#### 15. **Tab-Based Indentation** [LOW]
**File:** All files
**Issue:** Code uses tabs instead of 4 spaces, causing inconsistent display across editors.

#### 16. **Include Guard Naming** [LOW]
**File:** `src/animation.h:1-2`
**Code:**
```c
#ifndef ANIMATION2_H
#define ANIMATION2_H
```

**Issue:** Inconsistent naming (ANIMATION2_H vs animation.h filename).

---

## Part 2: Library Extraction Roadmap

### Phase 1: Core Architecture Redesign

#### 1.1 Context-Based API
Replace global singleton with explicit context:

```c
// libanimate.h
typedef struct AnimateContext AnimateContext;

AnimateContext *animate_create_context(void);
void animate_destroy_context(AnimateContext *ctx);
void animate_tick(AnimateContext *ctx, double delta_time_seconds);
```

#### 1.2 Time-Based Animation System
Replace frame counts with time:

```c
typedef struct {
    double start_time;
    double duration;
    double elapsed;
    // ...
} AnimateSegment;
```

#### 1.3 Type-Safe Value Animation
Use tagged unions with type-safe accessors:

```c
typedef enum {
    ANIMATE_VALUE_FLOAT,
    ANIMATE_VALUE_INT,
    ANIMATE_VALUE_VEC2,
    ANIMATE_VALUE_VEC3,
    ANIMATE_VALUE_COLOR,
    ANIMATE_VALUE_CUSTOM
} AnimateValueType;

typedef struct {
    AnimateValueType type;
    union {
        float f;
        int i;
        struct { float x, y; } vec2;
        struct { float x, y, z; } vec3;
        struct { float r, g, b, a; } color;
        void *custom;
    } value;
} AnimateValue;
```

### Phase 2: Memory Management

#### 2.1 Arena Allocator Support
Allow users to provide memory:

```c
typedef struct {
    void *(*alloc)(size_t size, void *user_data);
    void (*free)(void *ptr, void *user_data);
    void *user_data;
} AnimateAllocator;

AnimateContext *animate_create_context_ex(const AnimateAllocator *allocator);
```

#### 2.2 Handle-Based References
Replace raw pointers with handles for safety:

```c
typedef uint32_t AnimateChainHandle;
typedef uint32_t AnimateGroupHandle;

AnimateChainHandle animate_chain_create(AnimateContext *ctx);
void animate_chain_destroy(AnimateContext *ctx, AnimateChainHandle chain);
```

### Phase 3: Extensibility

#### 3.1 Custom Interpolation Functions
```c
typedef float (*AnimateEasingFunc)(float t, void *user_data);
void animate_set_custom_easing(AnimateContext *ctx, AnimateEasingId id,
    AnimateEasingFunc func, void *user_data);
```

#### 3.2 Event/Callback System
```c
typedef void (*AnimateCallback)(AnimateChainHandle chain, AnimateEvent event, void *user_data);
void animate_chain_on_complete(AnimateContext *ctx, AnimateChainHandle chain,
    AnimateCallback callback, void *user_data);
```

#### 3.3 Custom Value Types
```c
typedef void (*AnimateLerpFunc)(void *result, const void *a, const void *b, float t);
AnimateValueType animate_register_type(AnimateContext *ctx, size_t size, AnimateLerpFunc lerp);
```

### Phase 4: API Design

#### 4.1 Fluent/Builder API (Optional)
```c
AnimateChainBuilder animate_chain(AnimateContext *ctx)
    .to(&target_value, ANIMATE_VALUE_FLOAT)
    .duration(1.5)
    .easing(ANIMATE_EASE_OUT_CUBIC)
    .then()
    .rest(0.5)
    .then()
    .to(&other_value, ANIMATE_VALUE_FLOAT)
    .duration(1.0)
    .easing(ANIMATE_EASE_IN_QUINT);
```

#### 4.2 Simple C API
```c
// Create and configure chain
AnimateChainHandle chain = animate_chain_create(ctx);
animate_chain_add_float(chain, &opacity, 0.0f, 1.0f, 0.5f, ANIMATE_EASE_OUT_CUBIC);
animate_chain_add_rest(chain, 1.0f);
animate_chain_add_float(chain, &opacity, 1.0f, 0.0f, 0.5f, ANIMATE_EASE_IN_CUBIC);

// Play
animate_chain_play(chain, ANIMATE_PLAY_ONCE);
```

### Phase 5: Testing & Documentation

#### 5.1 Unit Tests Required
- Memory allocation failure paths
- Edge cases (zero duration, negative values)
- Time precision at high frame rates
- Concurrent chain execution
- Custom type interpolation

#### 5.2 Documentation
- API reference with examples
- Performance characteristics
- Thread safety guarantees (or lack thereof)
- Migration guide from internal FroggyFrag API

### Phase 6: Build System

#### 6.1 Library Structure
```
libanimate/
├── include/
│   └── libanimate.h          # Public API
├── src/
│   ├── animate.c             # Core implementation
│   ├── animate_easing.c      # Easing functions
│   └── animate_value.c       # Value type system
├── tests/
│   └── test_*.c
├── examples/
│   ├── basic.c
│   ├── custom_types.c
│   └── raylib_integration.c
├── CMakeLists.txt
└── README.md
```

#### 6.2 Dependencies
- **Required:** None (standard C library only)
- **Optional:** math library (-lm) for easing functions
- **Test:** Custom test framework or simple assert-based tests

### Migration Checklist for FroggyFrag

1. [ ] Create libanimate repository
2. [ ] Implement core context and time-based system
3. [ ] Port all easing functions
4. [ ] Implement handle-based API
5. [ ] Write comprehensive tests
6. [ ] Update FroggyFrag to use libanimate
7. [ ] Remove old animation.c/animation.h
8. [ ] Update build system

---

## Summary

The current animation subsystem has **3 critical bugs**, **4 high-priority architectural issues**, and numerous code quality concerns. The most severe issues are:

1. **Broken freeList logic** that writes to arbitrary memory
2. **Memory leaks** in chain creation
3. **Frame-based timing** unsuitable for general use
4. **Global singleton** preventing reusability

Extracting this into a standalone library requires significant redesign, particularly around memory management, timing, and API safety. The proposed roadmap provides a path to a robust, reusable animation library.

---

*Report generated: 2026-02-11*
*Files reviewed: animation.h, animation.c, ease.h, ease.c, gui.h, gui.c*
