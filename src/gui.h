#ifndef GUI_H
#define GUI_H

#include "animation.h"
#include "raylib.h"

typedef void (*DrawCallback)(void *self);

typedef struct {
	Rectangle bounds;
	DrawCallback draw;
	bool visible;
} Drawable;

typedef struct {
	Drawable base;
	Animation *animationList[MAX_ANIMATIONS_PER_DRAWABLE];
	uint8_t animationCount;
} Animateable;

typedef struct {
	Animateable base;
	const char **text;
	int maxLines;
	int lineCount;
	Color cBackground;
	Color cText;
	int padding;
	Font font;
	int fontSize;
} TextBox;

typedef struct {
	Animateable base;
	Rectangle bounds;
	Texture2D t;
	float aspectRatio;
	float scale;
	float rotation;
} AnimatedImage;

typedef enum {
	TOR_TRUNCATE,
	TOR_WRAP,
	TOR_ELLIPSES,
	TOR_EXPAND
} TextOverflowRule;

// general drawing functions
void addDrawable(Drawable *d);
void drawItems();

// animation extensions
ColAnimation *createColourAnimationInOut(Animateable *a, bool *trigger, Color *toAnimate, Color target, float transitionTime, float duration);
void registerAnimationInAnimateable(Animateable *a, Animation *animation);
void resetAnimateableAnimations(Animateable *a);

// UI element creation and updating

// text box and notifications
TextBox *createTextBoxEx(Rectangle r, char *text, int maxLines, Color bgColour, Font f, int fontSize, Color txtColour, bool visible, TextOverflowRule overflow, int padding);
TextBox *createNotificationTextBox(int x, int y, int w, int h, char *text, Color bgCol, Color txtCol);
void drawTextBox(void *self);
void triggerNotificationFade(TextBox *tb);
void newNotification(TextBox *tb, char *newText);

// images
AnimatedImage *createAnimatedImageRect(const char *imagePath, Rectangle bounds);
AnimatedImage *createAnimatedImageAspectRatio(const char *imagePath, Vector2 pos, float scale);

void drawImage(void *self);

// callback functions
void toggleTextBoxVisibilityCB(void *textBox);

#endif
