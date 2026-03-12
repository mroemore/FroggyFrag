# FroggyFrag Codebase Analysis Report

**Analysis Date**: 2026-03-02
**Project**: OpenGL Fragment Shader Preview Tool (Raylib-based C Application)

---

## 1. PROJECT LEVEL UNDERSTANDING

### 1a) Robustness of Current Feature Set

**Implemented Features:**
| Feature | Status | Quality |
|---------|--------|---------|
| Shader loading/compilation | ✅ Working | ⚠️ Fragile |
| Shader hot-reload (file watching) | ✅ Working | ⚠️ Fragile |
| Shader cycling (next/prev) | ✅ Working | ✅ Good |
| Directory rescanning | ✅ Working | ✅ Good |
| Screenshot capture | ✅ Working | ⚠️ Buffer risk |
| Config file parsing | ✅ Working | ⚠️ Memory leaks |
| UI notifications | ✅ Working | ✅ Good |
| Error message display | ✅ Working | ⚠️ Limited |
| FPS counter | ✅ Working | ✅ Good |

**Critical Robustness Issues:**
- **BUG**: `main.c:59` - Uses `screenW` for both width AND height (copy-paste error)
- No resource cleanup on exit (shaders, textures, fonts leaked)
- Config keys `contentW/contentH` defined in README but NOT IMPLEMENTED
- `copyOnDrag` and `maintainContentAspectRatio` in config but NOT IMPLEMENTED

### 1b) Codebase Structure and Organization

```
FroggyFrag/
├── inc/                    # External libraries
│   ├── raylib.h           # Graphics/windowing (vendored)
│   └── raymath.h          # Math utilities (vendored)
├── src/                    # Source code
│   ├── main.c             # Entry point + ShaderManager (390 lines)
│   ├── conf.c/h           # Configuration system (183 lines)
│   ├── gui.c/h            # UI rendering (406 lines)
│   ├── animation.c/h      # Animation engine (286 lines)
│   ├── timer.c/h          # Timer system (118 lines)
│   ├── ease.c/h           # Easing functions (175 lines)
│   ├── callback.c/h       # Callback wrapper (6 lines)
│   └── cJSON.c/h          # JSON parser (vendored)
├── tests/                  # Test files
│   ├── timer_tests.c      # Only test file (65 lines)
│   └── unity.c/h          # Unity test framework
└── bin/                    # Build output + resources
```

**Structure Assessment:**
- ✅ Clean separation of concerns (conf, gui, animation, timer)
- ✅ No circular dependencies
- ⚠️ Main.c is monolithic (entry point + ShaderManager + helpers)
- ⚠️ Timer system exists but is NOT USED in main application

### 1c) Modularity of Codebase

**Module Dependency Graph:**
```
main.c ──► conf.h ──► cJSON.h
     ──► gui.h ──► animation.h ──► ease.h
     ──► raylib.h

Independent: timer.h/c, callback.h/c
```

**Modularity Score: 7/10**
- ✅ Clear module boundaries
- ✅ Header files define clean public APIs
- ⚠️ Global singletons (gc, rootElement, am, timerList)
- ⚠️ Timer system completely disconnected from main app
- ⚠️ Some modules overly coupled (gui → animation → ease chain)

### 1d) Clarity of Trajectory

**Documented Roadmap (README):**
1. Drag and drop loading of images/shaders - **NOT IMPLEMENTED**
2. Keybindings for cycling background images - **NOT IMPLEMENTED**
3. Aspect ratio settings - **NOT IMPLEMENTED**
4. Config files for shader variables - **NOT IMPLEMENTED**

**Trajectory Assessment:**
- ⚠️ 4 features documented as "upcoming" with NO code scaffolding
- ⚠️ Config keys exist for some features but no implementation
- ⚠️ No ADRs (Architecture Decision Records) or design docs
- ❌ Architecture NOT ready for drag-and-drop (no event handling hooks)

---

## 2. MICRO LEVEL CODE REVIEW

### 2a) Bug Mitigation Score: 3/10

**Critical Bugs Found:**

| File:Line | Issue | Severity |
|-----------|-------|----------|
| `main.c:59` | `screenH = getConfigValueInt("screenW")` - Wrong key | 🔴 CRITICAL |
| `conf.c:171` | `bool result = NULL` - Invalid bool value | 🔴 CRITICAL |
| `main.c:59` | screenH never gets correct value | 🔴 CRITICAL |
| `main.c:326` | `LoadFileText()` result not freed | 🟡 WARNING |
| `main.c:216` | `strdup()` result never freed | 🟡 WARNING |

**Error Handling Gaps (23 issues):**
- No NULL checks after: LoadFont, LoadImage, LoadShader, LoadRenderTexture, strdup, fopen, freopen
- No validation of file paths before use
- No error recovery for shader compilation failures (just logs)

### 2b) Quality of Implementation: 5/10

**Strengths:**
- Animation system is well-designed (segment → chain → group hierarchy)
- Configuration system uses offset-based mapping (clever pattern)
- GUI scene graph pattern is solid
- Easing functions are mathematically correct

**Weaknesses:**
- Memory leaks throughout (see Section 2a)
- No cleanup functions for ShaderManager, Drawable tree
- Hardcoded paths and magic numbers scattered
- Mixed responsibility in main.c (entry + shader + event handling)

### 2c) Adherence to Best Practices: 4/10

**C Best Practices Violations:**
```c
// Missing NULL checks pattern (main.c:69)
Font fontSystem = LoadFont(getConfigValueString("systemFontPath"));
// Should be: if (fontSystem.texture.id == 0) { error handling }

// Memory leak pattern (conf.c:46)
*(char **)param = malloc(MAX_PATH_LENGTH);
// If called twice for same key, previous allocation leaked

// Invalid initialization (conf.c:171)
bool result = NULL;  // NULL is pointer, not bool
// Should be: bool result = false;

// Unsafe buffer operations (main.c:226)
char screenshotFileName[255];
snprintf(screenshotFileName, 255, ...);  // Should use sizeof()-1
```

**Positive Patterns:**
- ✅ Static functions properly marked in headers
- ✅ Consistent use of size_t for offsets
- ✅ Union types for animation targets (type-safe)
- ✅ Enum types for configuration

### 2d) Cross-Codebase Consistency of Style: 6/10

**Naming Conventions (Mixed):**
- camelCase: `initShaderManager`, `createTextBox`, `getConfigValueInt`
- snake_case: `setConfigValue`, `tick_timers` (inconsistently)
- Mixed: `initDefaultASProperties` vs `init_default_conf`

**Code Style Issues:**
- Inconsistent brace placement
- Mixed tab/space indentation
- No consistent return value conventions (void vs int vs pointer)

### 2e) Redundantly Implemented Features: 2 instances

1. **Duplicate Frame Counters:**
   - `animation.c:6` has `frameCounter`
   - `timer.c:10` has `frameCounter`
   - Both have `getFrameCount()` - completely redundant

2. **Timer System Not Used:**
   - Full timer implementation (118 lines)
   - Zero usage in main application
   - Dead code that should be integrated or removed

---

## 3. TESTING

### 3a) Test Coverage: ~1%

**Modules Tested:**
- ✅ timer.c (partially - 2 test functions, 0 assertions)

**Modules NOT Tested (93% of codebase):**
- ❌ main.c (390 lines) - shader loading, window management
- ❌ conf.c (183 lines) - config parsing
- ❌ gui.c (406 lines) - UI rendering
- ❌ animation.c (286 lines) - animation system
- ❌ ease.c (175 lines) - easing functions

### 3b) Test Case Maturity: CRITICAL

**Current Test State:**
```c
// tests/timer_tests.c - "tests" without assertions
void test_createTimer() {
    DemoData *dd = create_dd(4, 2.2, false, "demo1");
    Timer *t = createTimer(TT_ONCE, 1.0, dd_cb, dd);
    // NO ASSERTION - test passes even if t is NULL!
}

void testTimerSys_1() {
    // Runs for 10 seconds, prints output
    // NO ASSERTIONS - no pass/fail detection
}
```

**Maturity Issues:**
- Zero assertions across entire test suite
- No test runner (tests don't actually run)
- Memory leaks in tests (create_dd allocates, never freed)
- No edge cases tested (NULL inputs, overflow, etc.)

### 3c) Breadth of Testing Approaches

| Test Type | Status |
|-----------|--------|
| Unit Tests | ❌ None functional |
| Integration Tests | ❌ None |
| E2E Tests | ❌ None |
| Visual/Regression | ❌ None |
| Performance Tests | ❌ None |
| Memory Tests | ❌ None |

**Critical Path Test Gaps:**
- Shader compilation failure handling - UNTESTED
- Config file parsing errors - UNTESTED
- Window resize behavior - UNTESTED
- Memory leak detection - UNTESTED

---

## 4. ARCHITECTURAL DEBT ANALYSIS

### Can Current Architecture Support Planned Features?

| Planned Feature | Architectural Readiness | Blockers |
|-----------------|------------------------|----------|
| Drag & drop | ❌ NOT READY | No event system, no file handling hooks |
| Background cycling | ⚠️ PARTIAL | Background hardcoded, need array system |
| Aspect ratio | ⚠️ PARTIAL | Config key exists, no implementation |
| Shader variables | ✅ READY | Could extend config system |

### Architectural Debt Items:

1. **No Event System**
   - Drag-and-drop requires event callbacks
   - Current architecture has no event queue/callback pattern
   - callback.h exists but unused

2. **Global State Proliferation**
   - 4+ global singletons (gc, rootElement, am, timerList)
   - Makes testing nearly impossible
   - Prevents multiple instances

3. **No Resource Lifecycle Management**
   - No cleanup on exit
   - No reference counting
   - Leak detection impossible

4. **Timer System Orphaned**
   - 118 lines of working code
   - Zero integration with main app
   - Separate frame counter from animation system

---

## 5. SUMMARY SCORES

| Category | Score | Notes |
|----------|-------|-------|
| Feature Robustness | 5/10 | Core works, fragile edges |
| Code Organization | 7/10 | Clean modules, monolithic main |
| Modularity | 7/10 | Good boundaries, global state issue |
| Trajectory Clarity | 4/10 | Features planned, no scaffolding |
| Bug Mitigation | 3/10 | Critical bugs, poor error handling |
| Implementation Quality | 5/10 | Good patterns, memory issues |
| Best Practices | 4/10 | Missing NULL checks, leaks |
| Style Consistency | 6/10 | Mostly consistent, some drift |
| Redundancy | 7/10 | Minor duplication (frame counters) |
| Test Coverage | 1/10 | Essentially untested |
| Test Maturity | 1/10 | No assertions, no runner |
| **OVERALL** | **4.6/10** | **Needs significant work** |

---

## 6. RECOMMENDED NEXT STEPS

### Immediate (Critical Bugs):
1. Fix `main.c:59` screenH bug
2. Fix `conf.c:171` NULL assignment
3. Add NULL checks for all resource loads

### Short-term (Memory Safety):
4. Implement cleanup functions for ShaderManager
5. Fix memory leaks in conf.c setConfigValue
6. Add free() for strdup results

### Medium-term (Testing):
7. Add assertions to timer_tests.c
8. Add tests for conf.c parsing
9. Add tests for shader loading

### Long-term (Architecture):
10. Extract ShaderManager to separate module
11. Implement event system for drag-and-drop
12. Consolidate frame counter (animation vs timer)
