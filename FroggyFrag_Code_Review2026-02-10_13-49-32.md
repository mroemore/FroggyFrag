# FroggyFrag C Code Review Report

**Project:** FroggyFrag - OpenGL Fragment Shader Preview Tool
**Review Date:** 2026-02-08
**Reviewer:** Claude Code / c-review
**Scope:** Complete codebase analysis against strict C coding standards

---

## Executive Summary

FroggyFrag is an OpenGL fragment shader preview tool built with C and raylib. The codebase demonstrates creative animation systems and GUI components but suffers from significant memory management issues, missing error handling, security vulnerabilities, and inconsistent coding standards adherence.

### Overall Assessment
- **Security:** HIGH RISK - Multiple vulnerabilities including unchecked system calls, unsafe string operations, and resource leaks
- **Memory Management:** POOR - Widespread memory leaks, missing NULL checks, no cleanup paths
- **Code Quality:** BELOW STANDARD - Inconsistent style, missing documentation, incomplete features
- **Maintainability:** LOW - Tight coupling, global state, dead code

---

## File-by-File Analysis

### 1. main.c

**Purpose:** Application entry point, main render loop, shader management, input handling

**Functions:**

#### `main()`
- **Usage:** Entry point, initializes all subsystems and runs main loop
- **Called by:** Operating system
- **Issues:**
  - ~~[CRITICAL] Line 56: Bug - `screenHeight` initialized with `getConfigValueInt("screenW")` instead of `"screenH"`~~ **FIXED**
  - [HIGH] No cleanup of resources on exit (shaders, textures, fonts, render textures)
  - ~~[HIGH] `originalWorkingDirectory` in `screenshot()` leaked (strdup without free)~~ **FIXED**
  - ~~[HIGH] Application fails when run from wrong directory - resource paths are relative~~ **FIXED**
  - [MEDIUM] Unused variables: `loadedShaderCount`, `selectedShader`, `drawConsole`, `now`, `autoReload`
  - [MEDIUM] `reloadTimer` check uses `autoReload` flag but it's not connected to the actual timer logic

#### `updateScreenDimensions()`
- **Usage:** Handles window resize events, updates rendering rectangles
- **Called by:** `main()` line 117
- **Issues:**
  - [LOW] Parameter `sourceRect` width used for ratio calculation but height is negated in OpenGL - potential confusion

#### `screenshot()`
- **Usage:** Captures screenshot and saves to configured folder
- **Called by:** `main()` line 146 when 'S' key pressed
- **Issues:**
  - ~~[CRITICAL] Memory leak: `strdup(GetWorkingDirectory())` result never freed~~ **FIXED**
  - [HIGH] Uses fixed 255-byte buffer for paths - potential buffer overflow
  - [MEDIUM] No validation that screenshot folder exists before changing directory
  - [MEDIUM] `TextSplit` returns internal raylib pointer - should not be treated as const char**

#### `addShaderPath()`
- **Usage:** Adds shader file path to ShaderManager array
- **Called by:** `initShaderManager()`, `rescanDirectory()`
- **Issues:**
  - ~~[HIGH] No NULL check on `strdup` return~~ **FIXED**
  - [MEDIUM] Uses `strdup` which is POSIX, not standard C

#### `initShaderManager()`
- **Usage:** Initializes shader manager, loads shader list from directory
- **Called by:** `main()` line 95
- **Issues:**
  - [HIGH] No cleanup if `createMessageBuffer()` fails
  - ~~[MEDIUM] Hardcoded `.glsl` extension in `rescanDirectory()` but uses config here~~ **FIXED**
  - ~~[MEDIUM] Uses wrong config key "systemFont" instead of "systemFontPath"~~ **FIXED**
  - [LOW] `printf` debug statements should be removed or converted to logging

#### `swapOrReloadShader()`
- **Usage:** Hot-reloads shaders when file changes or user switches
- **Called by:** `main()`, `incShaderIndex()`, `decShaderIndex()`
- **Issues:**
  - ~~[CRITICAL] `freopen("/dev/tty", "w", stdout)` - non-portable (Windows incompatible)~~ **FIXED**
  - ~~[HIGH] `errFile` loaded with `LoadFileText` but never freed with `UnloadFileText`~~ **FIXED**
  - [HIGH] `freopen` error handling missing - if it fails, stdout is lost
  - [MEDIUM] Hardcoded "glerr" filename
  - [MEDIUM] No check if `TextSplit` returns NULL

#### `rescanDirectory()`
- **Usage:** Rescans shader directory for new files
- **Called by:** `main()` line 143 on F5 key
- **Issues:**
  - ~~[HIGH] Hardcoded `.glsl` extension instead of using config value~~ **FIXED**
  - [MEDIUM] No error handling if `DirectoryExists` fails after loop

#### `incShaderIndex()` / `decShaderIndex()`
- **Usage:** Navigate through shader list
- **Called by:** `main()` on +/- keys
- **Issues:**
  - [LOW] Silent failure at bounds - should provide user feedback

#### `pushMessage()`
- **Usage:** Adds message to MessageBuffer
- **Called by:** `swapOrReloadShader()`
- **Issues:**
  - ~~[CRITICAL] Memory leak: `strdup(msg)` assigned to `mb->messages[mb->index]` but old message at that index never freed~~ **FIXED**
  - [HIGH] No bounds checking on `strlen(msg)` - MAX_MESSAGE_LENGTH check is insufficient
  - [MEDIUM] Array index calculation is confusing and potentially wrong on overflow

---

### 2. gui.c

**Purpose:** GUI system with drawable hierarchy, animations, text rendering

#### Global State
- `rootElement` - Global root drawable
- **Issues:**
  - [HIGH] Global mutable state makes testing difficult
  - [MEDIUM] No thread safety considerations

#### `initRootDrawable()`
- **Usage:** Initialize root drawable with screen dimensions
- **Called by:** `main()` line 57
- **Issues:**
  - [LOW] `inheritProps` not initialized

#### `updateGraph()` / `updateDrawables()`
- **Usage:** Update drawable hierarchy recursively
- **Called by:** `main()` line 167
- **Issues:**
  - [LOW] No cycle detection in parent-child graph

#### `drawAll()` / `drawChildren()`
- **Usage:** Render all drawables
- **Called by:** `main()` line 169
- **Issues:**
  - [MEDIUM] No NULL check before calling draw callback

#### `addChildDrawable()` / `addDrawableToRoot()`
- **Usage:** Add child to parent drawable
- **Called by:** Various create functions
- **Issues:**
  - [MEDIUM] Silent failure when MAX_DRAWABLE_CHILDREN exceeded
  - [LOW] Duplicate code between functions

#### `createPopDownAnimation()` / `createDancingFroggyAnimation()`
- **Usage:** Animation creation (placeholder implementations)
- **Called by:** Unused
- **Issues:**
  - [HIGH] Empty implementations - dead code

#### `registerAnimationGroup()`
- **Usage:** Register animation group with animateable
- **Called by:** `createSettingsNotificationBox()`, `createMessageBuffer()`
- **Issues:**
  - [MEDIUM] No bounds check on `animationCount` before array access

#### `resetAnimateableAnimations()`
- **Usage:** Reset animations (placeholder)
- **Called by:** Unused
- **Issues:**
  - [HIGH] Empty implementation - dead code

#### `initDrawableProperties()`
- **Usage:** Initialize common drawable properties
- **Called by:** All create functions
- **Issues:**
  - [LOW] `inheritProps` not initialized

#### `populateTextBoxLines()`
- **Usage:** Parse text into lines for TextBox
- **Called by:** `createTextBoxEx()`, `newSettingsInfo()`
- **Issues:**
  - [HIGH] Memory leak: `textCopy` freed but `splitLines` from `TextSplit` may need different handling
  - [MEDIUM] `free(tb)` on error - caller may not expect this, potential double-free
  - [MEDIUM] No check if `TextSplit` returns NULL

#### `createTextBoxEx()`
- **Usage:** Create text box with extended options
- **Called by:** `createSettingsNotificationBox()`
- **Issues:**
  - [HIGH] Memory leak: `tb` allocated but not freed if `populateTextBoxLines` fails (actually it is freed inside, which is problematic)
  - [MEDIUM] No validation of font parameter

#### `createSettingsNotificationBox()`
- **Usage:** Create animated notification box with froggy mascot
- **Called by:** `main()` line 93
- **Issues:**
  - [CRITICAL] Memory leaks: Multiple `malloc` calls for AnimationChainGroup without any free mechanism
  - ~~[CRITICAL] `AnimationChain` structs (`opacityFade`, `positionSlide`, etc.) are stack-allocated but referenced by pointer in group - undefined behavior after function returns~~ **FIXED**
  - [HIGH] Font loaded every call but never unloaded
  - [HIGH] Hardcoded paths: "resources/FroggyOutlined64px.png"
  - [MEDIUM] Complex animation setup should be documented

#### `updateTextBox()`
- **Usage:** Update text box position based on parent
- **Called by:** Animation system via function pointer
- **Issues:**
  - [LOW] No NULL check on parent dereference

#### `drawTextBox()`
- **Usage:** Render text box
- **Called by:** Animation system via function pointer
- **Issues:**
  - [MEDIUM] `textWidth` calculated but unused
  - [LOW] Magic number `2` for spacing - should be constant

#### `newSettingsInfo()`
- **Usage:** Update notification text and trigger animation
- **Called by:** `screenshot()`, `swapOrReloadShader()`
- **Issues:**
  - [HIGH] Old text lines never freed - memory leak
  - [MEDIUM] No validation that animationList entries exist

#### `createAnimatedImage()`
- **Usage:** Create animated image component
- **Called by:** `createSettingsNotificationBox()`
- **Issues:**
  - [MEDIUM] `tmp` image loaded even if w/h parameters provided
  - [LOW] No validation of imagePath

#### `updateAnimatedImage()` / `drawAnimatedImage()`
- **Usage:** Update and render animated image
- **Called by:** Animation system
- **Issues:**
  - [LOW] No NULL checks

#### `createMessageBuffer()`
- **Usage:** Create message buffer for error display
- **Called by:** `initShaderManager()`
- **Issues:**
  - ~~[CRITICAL] Same stack allocation bug as `createSettingsNotificationBox` - AnimationChain structs are local~~ **FIXED**
  - ~~[CRITICAL] Uninitialized messages array causing segfault on first pushMessage~~ **FIXED**
  - [CRITICAL] Memory leaks: malloc'd AnimationChainGroup never freed
  - [HIGH] Font loaded but never unloaded
  - [MEDIUM] Hardcoded dimensions and animation parameters

#### `updateMessageBuffer()`
- **Usage:** Update message buffer
- **Called by:** Animation system
- **Issues:**
  - [LOW] No NULL checks

#### `drawMessageBuffer()`
- **Usage:** Render message buffer
- **Called by:** Animation system
- **Issues:**
  - [MEDIUM] Hardcoded width (600) and height (18)
  - [MEDIUM] `lineVisibilityMod` calculated but not used in rendering
  - [LOW] Magic numbers for spacing

#### `toggleMessageBufferVisibility()`
- **Usage:** Toggle message buffer visibility with animation
- **Called by:** `main()` line 136
- **Issues:**
  - [MEDIUM] No bounds checking on animationList access

---

### 3. gui.h

**Purpose:** Header file for GUI system

**Issues:**
- [MEDIUM] Include guard uses `ANIMATION2_H` in animation.h - potential conflict
- [LOW] `TextOverflowRule` enum defined but never used
- [LOW] `createShaderNotificationBox()` and `createPopupTextBox()` declared but not implemented

---

### 4. animation.c

**Purpose:** Animation system with keyframes, chains, and groups

#### Global State
- `am` - Global animation manager
- `frameCounter` - Global frame counter
- **Issues:**
  - [HIGH] Global mutable state
  - [MEDIUM] No thread safety

#### `getFrameCount()` / `incrementFrameCount()`
- **Usage:** Frame counter access
- **Called by:** Various animation functions
- **Issues:**
  - [LOW] `incrementFrameCount` is static but declared in header

#### `initDefaultASProperties()`
- **Usage:** Initialize animation segment defaults
- **Called by:** `initMoveAnimationSegment()`, `initRestAnimationSegment()`
- **Issues:** None significant

#### `initMoveAnimationSegment()`
- **Usage:** Initialize movement animation segment
- **Called by:** `addAnimation()`, `setCustomRetrigAnimation()`
- **Issues:**
  - [MEDIUM] Debug printf left in code
  - [LOW] No validation of durationFrames

#### `initRestAnimationSegment()`
- **Usage:** Initialize rest/delay segment
- **Called by:** `addRest()`
- **Issues:** None significant

#### `startSegment()`
- **Usage:** Start animation segment
- **Called by:** `startAnimationChain()`
- **Issues:** None significant

#### `animateInt()` / `animateFloat()` / `animateUChar()`
- **Usage:** Type-specific animation interpolation
- **Called by:** `processMove()` via function pointer
- **Issues:**
  - [LOW] Integer animation may have precision issues with large deltas

#### `processBlank()`
- **Usage:** Placeholder process function
- **Called by:** Unused
- **Issues:**
  - [LOW] Dead code

#### `processMove()`
- **Usage:** Process movement animation frame
- **Called by:** `tickAnimationChain()`
- **Issues:**
  - [LOW] t clamping (0.0-1.0) is redundant given duration check

#### `processRest()`
- **Usage:** Process rest segment
- **Called by:** `tickAnimationChain()`
- **Issues:** None significant

#### `initAnimationChain()`
- **Usage:** Initialize animation chain
- **Called by:** Various create functions in gui.c
- **Issues:** None significant

#### `addAnimation()`
- **Usage:** Add animation segment to chain
- **Called by:** GUI create functions
- **Issues:**
  - [MEDIUM] Silent failure if MAX_AC_LEN exceeded

#### `addRest()`
- **Usage:** Add rest segment to chain
- **Called by:** GUI create functions
- **Issues:**
  - [MEDIUM] Silent failure if MAX_AC_LEN exceeded

#### `setCustomRetrigAnimation()`
- **Usage:** Set custom animation for retrigger behavior
- **Called by:** GUI create functions
- **Issues:**
  - [MEDIUM] Always writes to MAX_AC_LEN-1, no validation if already set

#### `startAnimationChain()`
- **Usage:** Start or restart animation chain
- **Called by:** `startGroup()`
- **Issues:**
  - [MEDIUM] Complex switch statement could use comments

#### `resetAnimationChain()`
- **Usage:** Reset chain to initial state
- **Called by:** `resetGroup()`
- **Issues:**
  - ~~[HIGH] `segments[0].animate` called without checking if segment exists~~ **FIXED**

#### `tickAnimationChain()`
- **Usage:** Update animation chain for current frame
- **Called by:** `tickAnimationChainGroup()`
- **Issues:**
  - [MEDIUM] Complex state machine, needs documentation

#### `initAnimationChainGroup()`
- **Usage:** Initialize animation group
- **Called by:** GUI create functions
- **Issues:** None significant

#### `addChainToGroup()`
- **Usage:** Add chain to group
- **Called by:** GUI create functions
- **Issues:**
  - [MEDIUM] Silent failure if MAX_ACG_LEN exceeded
  - [CRITICAL] Takes AnimationChain by value - pointer stored in gui.c becomes invalid

#### `startGroup()` / `resetGroup()` / `togglePauseGroup()`
- **Usage:** Group control functions
- **Called by:** Animation manager
- **Issues:**
  - [LOW] `togglePauseGroup` comment says "do not use, needs to be fixed"

#### `tickAnimationChainGroup()`
- **Usage:** Update all chains in group
- **Called by:** `tickManagedAnimations()`
- **Issues:** None significant

#### `initAnimationManager()`
- **Usage:** Initialize global animation manager
- **Called by:** `main()` line 58
- **Issues:** None significant

#### `addAnimationGroupToManager()`
- **Usage:** Add group to manager
- **Called by:** GUI create functions
- **Issues:**
  - [MEDIUM] Complex free list logic, could be simplified

#### `tickManagedAnimations()`
- **Usage:** Update all managed animations
- **Called by:** `main()` line 168
- **Issues:** None significant

#### `startManagedGroup()`
- **Usage:** Start managed group by pointer
- **Called by:** `newSettingsInfo()`, `toggleMessageBufferVisibility()`
- **Issues:**
  - [LOW] Linear search could be slow with many groups

---

### 5. animation.h

**Purpose:** Animation system header

**Issues:**
- [MEDIUM] Include guard `ANIMATION2_H` - why "2"?
- [LOW] `static void incrementFrameCount()` declared but should not be in public header
- [LOW] `running` field in AnimationChainGroup never used

---

### 6. conf.c

**Purpose:** Configuration file parsing and management

#### Global State
- `gc` - Global config
- `configMap` - Static config mapping table

#### `setConfigValue()`
- **Usage:** Set single config value from JSON
- **Called by:** `parseJSONConfig()`
- **Issues:**
  - [CRITICAL] Memory leak: CVT_STRING case malloc's but no cleanup of previous value
  - ~~[HIGH] `strncpy` doesn't null-terminate if source >= MAX_PATH_LENGTH~~ **FIXED**
  - [MEDIUM] Duplicate key search (once in cJSON, once in configMap)

#### `parseJSONConfig()`
- **Usage:** Parse configuration file
- **Called by:** `initGlobalConf()`
- **Issues:**
  - [MEDIUM] Hardcoded list of keys - could iterate configMap
  - [LOW] Debug printf statements

#### `initDefaultConf()`
- **Usage:** Set default configuration values
- **Called by:** `initGlobalConf()`
- **Issues:**
  - [MEDIUM] `screenshotsFolder`, `imagesFolder` not initialized but freed in `freeConfig()`

#### `freeConfig()`
- **Usage:** Free configuration memory
- **Called by:** Unused (should be called on exit)
- **Issues:**
  - [MEDIUM] Not called anywhere - memory leaks on exit
  - ~~[MEDIUM] Doesn't free `shaderFileExtension`~~ **FIXED**

#### `initGlobalConf()`
- **Usage:** Initialize global configuration
- **Called by:** `main()` line 53
- **Issues:** None significant

#### `globalConfIsInitialised()`
- **Usage:** Check if config is initialized
- **Called by:** Unused
- **Issues:**
  - [LOW] Dead code (not called)

#### `getConfigValueInt()` / `getConfigValueFloat()` / `getConfigValueBool()`
- **Usage:** Typed config getters
- **Called by:** Various
- **Issues:**
  - ~~[MEDIUM] Return -1/-1.0f/NULL on error - indistinguishable from valid values~~ **FIXED (for bool)**
  - [MEDIUM] Linear search for every access - inefficient

#### `getConfigValueString()`
- **Usage:** Get string config value
- **Called by:** Various
- **Issues:**
  - [CRITICAL] Memory leak: malloc's result every call, caller must free but many don't
  - ~~[HIGH] `strncpy` may not null-terminate~~ **FIXED**
  - ~~[HIGH] Returns uninitialized memory when key not found~~ **FIXED**
  - [MEDIUM] Returns malloc'd empty string even if key not found

---

### 7. conf.h

**Purpose:** Configuration header

**Issues:**
- [LOW] `freeConfig()` declared static but should be public for cleanup

---

### 8. ease.c / ease.h

**Purpose:** Easing/interpolation functions

**Functions:** 17 easing functions + assigner

**Issues:**
- [LOW] Global constants `c4`, `c5`, `n1`, `d1` should be const
- [LOW] `easeInOutSine` formula appears incorrect: `(cos(M_PI * x) - 1.0f) / 2.0f` should be `(1 - cos(M_PI * x)) / 2`
- [LOW] Missing documentation on what each easing function does

---

### 9. timer.c / timer.h

**Purpose:** Frame-based timer system

**Issues:**
- [MEDIUM] `timer.h` includes `<complex.h>` - unused
- [HIGH] `getFrameCount()` in timer.h conflicts with `getFrameCount()` in animation.h
- [MEDIUM] `createTimer()` - `t.elapsed` set but never used
- [MEDIUM] `tickTimers()` - modifies list while iterating (removeTimer during loop)

---

### 10. callback.c / callback.h

**Purpose:** Simple callback system

**Issues:**
- [LOW] `applyCallback()` - no NULL check on `c` or `c->f`

---

### 11. cJSON.c / cJSON.h

**Purpose:** Third-party JSON parser

**Note:** Standard third-party library, not reviewed in detail

---

## System-Level Analysis

### Architecture Issues

1. **Global State Pollution**
   - Multiple global variables: `rootElement`, `am`, `frameCounter`, `gc`, `timerList`
   - Makes testing impossible, creates hidden dependencies
   - [HIGH] Recommendation: Encapsulate in context structures

2. **Memory Management Strategy**
   - No consistent ownership semantics
   - malloc without free throughout codebase
   - No cleanup on exit
   - [CRITICAL] Recommendation: Implement proper RAII-style resource management

3. **Error Handling Strategy**
   - Inconsistent error handling (printf, fprintf, silent failure)
   - No centralized error reporting
   - Many functions return void with no error indication
   - [HIGH] Recommendation: Standardize error handling with return codes

4. **Type Safety Issues**
   - Heavy use of void* and casting
   - Union types in AnimationSegment lack discriminant validation
   - [MEDIUM] Recommendation: Add type validation where possible

### Security Vulnerabilities

1. **Buffer Overflows**
   - Fixed-size buffers in `screenshot()` (255 bytes)
   - `strncpy` without null termination checks
   - [CRITICAL] Recommendation: Use snprintf with sizeof, verify null termination

2. **Resource Exhaustion**
   - No limits on shader file count
   - No limits on message buffer growth
   - [MEDIUM] Recommendation: Add resource limits

3. **Path Traversal**
   - User-controlled paths from config used directly
   - [MEDIUM] Recommendation: Validate and sanitize paths

### Performance Issues

1. **Linear Searches**
   - Config lookups linear search for every access
   - Animation group lookup linear on start
   - [MEDIUM] Recommendation: Use hash maps for frequent lookups

2. **Memory Fragmentation**
   - Many small allocations without pooling
   - [LOW] Recommendation: Consider arena allocators

3. **Redundant Operations**
   - `getConfigValueString()` malloc's on every call
   - Font loaded multiple times
   - [MEDIUM] Recommendation: Cache frequently accessed values

### Code Quality Issues

1. **Dead Code**
   - `createPopDownAnimation()` - empty
   - `createDancingFroggyAnimation()` - empty
   - `resetAnimateableAnimations()` - empty
   - `processBlank()` - returns false, never used
   - `globalConfIsInitialised()` - never called
   - [LOW] Recommendation: Remove or implement

2. **Magic Numbers**
   - Animation durations hardcoded throughout
   - Color values hardcoded
   - [LOW] Recommendation: Define named constants

3. **Inconsistent Naming**
   - `cDefault`, `cBackground`, `cText` - inconsistent prefixing
   - Some functions use camelCase, others snake_case
   - [LOW] Recommendation: Establish naming convention

4. **Missing Documentation**
   - No function documentation
   - Complex animation system undocumented
   - [MEDIUM] Recommendation: Add comprehensive documentation

---

## Recommendations by Priority

### CRITICAL (Fix Immediately)

1. ~~**Fix screen height bug** - `main.c:56`~~ **FIXED**
   ```c
   // WRONG:
   int screenHeight = getConfigValueInt("screenW");
   // CORRECT:
   int screenHeight = getConfigValueInt("screenH");
   ```

2. ~~**Fix stack-use-after-return** - AnimationChain in gui.c~~ **FIXED**
   - AnimationChain structs must be heap-allocated, not stack

3. **Fix memory leaks in getConfigValueString() callers**
   - Audit all callers to ensure they free the returned string

4. **Add missing resource cleanup**
   - Unload shaders, textures, fonts on exit

### HIGH (Fix Soon)

1. Implement proper error handling with return codes
2. ~~Add NULL checks after all allocations~~ **FIXED (for addShaderPath)**
3. Fix buffer overflow risks with bounded operations
4. ~~Make `freopen` portable or add platform abstraction~~ **FIXED**
5. Add cleanup for MessageBuffer messages

### MEDIUM (Fix When Convenient)

1. Standardize naming conventions
2. Add comprehensive documentation
3. Remove or implement dead code
4. Optimize config lookups
5. Add input validation

### LOW (Nice to Have)

1. Remove debug printf statements
2. Define magic numbers as constants
3. Add thread safety
4. Implement proper logging system

---

## Compliance Summary

| Category | Status | Notes |
|----------|--------|-------|
| Style & Formatting | FAIL | Mixed indentation, long lines |
| Naming Conventions | FAIL | Inconsistent casing |
| Documentation | FAIL | No function documentation |
| Memory Management | FAIL | Widespread leaks, no cleanup |
| Error Handling | FAIL | Inconsistent, many unchecked calls |
| Security | FAIL | Buffer overflows, unsafe functions |
| Compiler Compliance | PARTIAL | Missing const, some casting issues |

---

## Appendix: File Statistics

| File | Lines | Functions | Issues Found |
|------|-------|-----------|--------------|
| main.c | 360 | 11 | 23 |
| gui.c | 406 | 22 | 31 |
| gui.h | 121 | - | 3 |
| animation.c | 286 | 26 | 12 |
| animation.h | 135 | - | 3 |
| conf.c | 183 | 9 | 14 |
| conf.h | 59 | - | 1 |
| ease.c | 175 | 20 | 3 |
| ease.h | 52 | - | 0 |
| timer.c | 118 | 7 | 4 |
| timer.h | 47 | - | 2 |
| callback.c | 6 | 1 | 1 |
| callback.h | 14 | - | 0 |

**Total:** ~2,000 lines of code, ~100 functions, ~97 issues identified

---

*Report generated by Claude Code c-review tool*

---

## Fix Log

**Date:** 2026-02-10

### Fixes Applied:

1. **main.c:56** - Fixed screenHeight initialization bug (screenW → screenH)
2. **main.c:pushMessage()** - Added free for existing message before strdup
3. **main.c:screenshot()** - Added free for originalWorkingDirectory and screenshotsFolder
4. **main.c:swapOrReloadShader()** - Added platform checks for freopen, added UnloadFileText for errFile
5. **main.c:addShaderPath()** - Added NULL check on strdup return
6. **main.c:rescanDirectory()** - Changed hardcoded ".glsl" to use config value
7. **gui.c** - Fixed stack-use-after-return by heap-allocating AnimationChain structs in createSettingsNotificationBox() and createMessageBuffer()
8. **animation.c:resetAnimationChain()** - Added bounds check before calling segments[0].animate
9. **conf.c:freeConfig()** - Added free for shaderFileExtension
10. **conf.c:getConfigValueBool()** - Fixed to return false instead of NULL on error
11. **conf.c:setConfigValue()** - Fixed strncpy to use MAX_PATH_LENGTH-1 and ensure null termination
12. **gui.c:createMessageBuffer()** - Changed to use calloc instead of malloc to initialize messages array to NULL
13. **main.c:initShaderManager()** - Fixed "systemFont" key to "systemFontPath", added free for returned string
14. **conf.c:getConfigValueString()** - Fixed to return empty string instead of uninitialized memory for missing keys
15. **main.c** - Added GetApplicationDirectory() call to ensure resources load correctly regardless of where app is run from
