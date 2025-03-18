#ifndef ANIMATION_H
#define ANIMATION_H

#include <stdint.h>

#include "raylib.h"
#include "callback.h"
#include "timer.h"

#define MAX_DRAWABLES 1024
#define MAX_ANIMATIONS_PER_DRAWABLE 8

#define MAX_TEXTBOX_LINES 128
#define MAX_TEXTBOX_LINE_LENGTH 512

typedef void (*DrawCallback)(void *self);
typedef void (*AnimationEventCallback)(void *self);
typedef void (*ProcessAnimation)(void *self, float timeDelta);

typedef enum {
	IT_LINEAR,
	IT_COUNT
} InterpType;

typedef struct {
	Rectangle bounds;
	DrawCallback draw;
} Drawable;

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

typedef struct {
	Drawable base;
	Animation animationList[MAX_ANIMATIONS_PER_DRAWABLE];
	uint8_t animationCount;
} Animateable;

typedef struct {
	Animateable base;
	const char **text;
	int maxLines;
	uint16_t lineCount;
	Color cBackground;
	Color cText;
	bool visible;
	int padding;
	Font font;
	int fontSize;
} TextBox;

typedef enum {
	TOR_TRUNCATE,
	TOR_WRAP,
	TOR_ELLIPSES,
	TOR_EXPAND
} TextOverflowRule;

// general drawing functions
void addDrawable(Drawable *d);
void drawItems();

// setup & logic
AnimationManager *createAnimationManager();
void resetAnimation(Animation *a);

// utility functions
Color vec4ToColor(Vector4 in);
Color vec4ColorInterpolate(Vector4 a, Vector4 b, float t);

// specific animations
void colourFadeInAnimation(void *self, float timeDelta);
void colourFadeOutAnimation(void *self, float timeDelta);
ColAnimation *createColourFadeAnimation(bool *trigger, Color *toAnimate, Color target, bool match, float duration, bool fadeIn);
void toggleAnimationCB(void *data);

void createColourAnimationInOut(bool *trigger, Color *toAnimate, Color target, float transitionTime, float duration);

TextBox *createTextBoxEx(Rectangle r, char *text, int maxLines, Color bgColour, Font f, int fontSize, Color txtColour, bool visible, TextOverflowRule overflow, int padding);
TextBox *createTextBox(int x, int y, int w, int h, char *text, Color bgCol, Color txtCol);

void drawTextBox(void *self);

#endif
