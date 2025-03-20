#ifndef ANIMATION_H
#define ANIMATION_H

#include <stdint.h>

#include "raylib.h"
#include "callback.h"
#include "timer.h"

#define MAX_DRAWABLES 1024
#define MAX_ANIMATIONS_PER_GROUP 64
#define MAX_TIMERS_PER_GROUP 64
#define MAX_ANIMATIONS_PER_DRAWABLE 16

#define MAX_TEXTBOX_LINES 128
#define MAX_TEXTBOX_LINE_LENGTH 512

typedef void (*DrawCallback)(void *self);
typedef void (*AnimationEventCallback)(void *self);
typedef void (*ProcessAnimation)(void *self, float timeDelta);

typedef enum {
	IT_LINEAR,
	IT_COUNT
} InterpType;

typedef enum {
	AT_COLOUR,
	AT_VECTOR2,
	AT_RECTANGLE
} AnimationType;

typedef struct {
	AnimationType type;
	bool *trigger;
	bool match;
	bool running;
	float duration;
	float currentTime;
	ProcessAnimation animate;
	Callback *onFinish;
} Animation;

typedef enum {
	AET_ANIMATION,
	AET_TIMER
} AnimationElementType;

typedef struct {
	union {
		Animation a;
		Timer t;
	} data;
	AnimationElementType type;
} AnimationElement;

typedef struct {
	Animation *animations[MAX_ANIMATIONS_PER_GROUP];
	int animationCount;
	Timer *timers[MAX_TIMERS_PER_GROUP];
	int timerCount;
} AnimationSequence;

typedef struct {
	Animation base;
	Color *toAnimate;
	Vector4 originalNormalised;
	Vector4 targetNormalised;
} ColAnimation;

typedef struct {
	Animation *animationList[MAX_DRAWABLES];
	uint16_t count;
} AnimationManager;

void tickAnimations(float timeDelta);

// setup & logic
AnimationManager *createAnimationManager();
void resetAnimation(Animation *a);
void addAnimation(Animation *a);
AnimationSequence *createAnimationGroup();

// utility functions
Color vec4ToColor(Vector4 in);
Color vec4ColorInterpolate(Vector4 a, Vector4 b, float t);

// specific animations
void colourFadeInAnimation(void *self, float timeDelta);
void colourFadeOutAnimation(void *self, float timeDelta);
ColAnimation *createColourFadeAnimation(bool *trigger, Color *toAnimate, Color target, bool match, float duration, bool fadeIn);
void toggleAnimationCB(void *data);

#endif
