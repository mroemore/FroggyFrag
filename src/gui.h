#ifndef GUI_H
#define GUI_H

#include <stdint.h>
#include "anim2.h"
#include "raylib.h"

typedef struct Drawable Drawable;
typedef void (*DrawCallback)(void *self);
typedef void (*UpdateCallback)(void *self);
#define MAX_ANIMATIONS_PER_DRAWABLE 16
#define MAX_DRAWABLE_CHILDREN 32
#define MAX_TEXTBOX_LINES 128
#define MAX_TEXTBOX_LINE_LENGTH 512

struct Drawable {
	Vector2 offset;
	Vector2 renderPos;
	Color cDefault;
	int width;
	int height;
	DrawCallback draw;
	UpdateCallback update;
	Drawable *children[MAX_DRAWABLE_CHILDREN];
	int childCount;
	Drawable *parent;
	bool inheritProps;
	bool visible;
};

typedef struct {
	Drawable base;
	const char **text;
	AnimationChainGroup *animationList[MAX_ANIMATIONS_PER_DRAWABLE];
	int animationCount;
} Animateable;

typedef struct {
	Animateable base;
	Rectangle srcBounds;
	Rectangle targetBounds;
	Texture2D t;
	float aspectRatio;
	float scale;
	float rotation;
	Color tint;
} AnimatedImage;

typedef struct {
	Animateable base;
	const char **text;
	int maxLines;
	int lineCount;
	Color cBackground;
	Color cText;
	Vector4 padding;
	Font font;
	int fontSize;
	AnimatedImage *img;
} TextBox;

#define MAX_MESSAGE_BUFFER_LINES 512
#define MAX_MESSAGE_LENGTH 1024

typedef struct {
	Animateable base;
	Font font;
	int fontSize;
	char *messages[MAX_MESSAGE_BUFFER_LINES];
	uint16_t count;
	uint16_t index;
	float lineVisibilityMod;
	Color cBackground;
	Color cText;
} MessageBuffer;

typedef enum {
	TOR_TRUNCATE,
	TOR_WRAP,
	TOR_ELLIPSES,
	TOR_EXPAND
} TextOverflowRule;

// general drawing functions
void initRootDrawable(int screenW, int screenH);
void drawAll();
void updateGraph(Drawable *d);
void updateDrawables();
void drawChildren(Drawable *d);
void addChildDrawable(Drawable *p, Drawable *c);
void addDrawableToRoot(Drawable *c);
void initDrawableProperties(Drawable *d, Drawable *parent, int x, int y, int w, int h, Color cDefault, DrawCallback drawFunction, UpdateCallback updateFunc);
// animation extensions
void createTextboxFadeAnimation(Animateable *tb, bool *trigger, Color *toAnimate, int transitionFrames, int pauseFrames);
void createPopDownAnimation(Animateable *a, bool *trigger, Rectangle *toAnimate, Rectangle offset, int transitionFrames);
void createDancingFroggyAnimation(Animateable *a, bool *trigger, AnimatedImage *toAnimate, int transitionFrames);
void registerAnimationGroup(Animateable *a, AnimationChainGroup *ac);
void resetAnimateableAnimations(Animateable *a);

// UI element creation and updating //

// text box and notifications
TextBox *createTextBoxEx(Drawable *parent, int x, int y, int w, int h, Vector4 padding, const char *text, int maxLines, Color bgColour, Font f, int fontSize, Color txtColour);
TextBox *createShaderNotificationBox(int x, int y, int w, int h, const char *text, Color bgCol, Color txtCol);
TextBox *createPopupTextBox(int x, int y, int w, int h, const char *text, Color bgCol, Color txtCol);
TextBox *createSettingsNotificationBox(int x, int y, int w, int h, char *text, Color bgCol, Color txtCol);

void updateTextBox(void *self);
void drawTextBox(void *self);
void newSettingsInfo(TextBox *settingsInfoBox, const char *newText);
// Animated Images
AnimatedImage *createAnimatedImage(Drawable *parent, int x, int y, int w, int h, Color cTint, const char *imagePath, float scale, float rotation);
void updateAnimatedImage(void *self);
void drawAnimatedImage(void *self);

MessageBuffer *createMessageBuffer(Drawable *parent, int x, int y, int w, int h, Color cBackground, Color cText, const char *fontPath, int fontSize);
void updateMessageBuffer(void *self);
void drawMessageBuffer(void *self);
void toggleMessageBufferVisibility(MessageBuffer *mb);
#endif
