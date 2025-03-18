#include "animation.h"

#include "raylib.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include "conf.h"
#include "timer.h"

Drawable *drawableList[MAX_DRAWABLES];
Animation *animationList[MAX_ANIMATIONS_PER_DRAWABLE * MAX_DRAWABLES];
int drawableListCount = 0;
int animationListCount = 0;

void addDrawable(Drawable *d) {
	if(drawableListCount < MAX_DRAWABLES) {
		drawableList[drawableListCount] = d;
		drawableListCount++;
	}
}

void addAnimation(Animation *a) {
	if(animationListCount < MAX_ANIMATIONS_PER_DRAWABLE * MAX_DRAWABLES) {
		animationList[animationListCount] = a;
		animationListCount++;
	}
}

void drawItems() {
	for(int i = 0; drawableList[i]; i++) {
		drawableList[i]->draw(drawableList[i]);
	}
}

AnimationManager *createAnimationManager() {
	AnimationManager *am = (AnimationManager *)malloc(sizeof(AnimationManager));
	am->count = 0;
	return am;
}
void resetAnimation(Animation *a) {
	a->running = false;
	a->currentTime = 0.0;
}

Color vec4ToColor(Vector4 in) {
	return (Color){ (int)(in.x * 255.0), (int)(in.y * 255.0), (int)(in.z * 255.0), (int)(in.w * 255.0) };
}

Color vec4ColorInterpolate(Vector4 a, Vector4 b, float t) {
	Vector4 result;
	result.x = a.x + (b.x - a.x) * t;
	result.y = a.y + (b.y - a.y) * t;
	result.z = a.z + (b.z - a.z) * t;
	result.w = a.w + (b.w - a.w) * t;
	return vec4ToColor(result);
}

void colourFadeOutAnimation(void *self, float timeDelta) {
	ColAnimation *ca = (ColAnimation *)self;
	if(!ca->base.running) return;

	ca->base.currentTime += timeDelta;
	float t = ca->base.currentTime / ca->base.duration;
	if(t < 0.0f) t = 0.0f;
	if(t > 1.0f) t = 1.0f;

	*ca->toAnimate = vec4ColorInterpolate(ca->targetNormalised, ca->originalNormalised, t);

	// Check if the animation has finished
	if(ca->base.currentTime >= ca->base.duration) {
		ca->base.running = false;
		*ca->toAnimate = vec4ToColor(ca->targetNormalised); // Ensure the final value is exact
		//.f(ca->base.onFinish.data);
		if(ca->base.onFinish) {
			applyCallback(ca->base.onFinish);
		}
	}
}

void colourFadeInAnimation(void *self, float timeDelta) {
	ColAnimation *ca = (ColAnimation *)self;
	if(!ca->base.running) return;

	ca->base.currentTime += timeDelta;
	float t = ca->base.currentTime / ca->base.duration;
	if(t < 0.0f) t = 0.0f;
	if(t > 1.0f) t = 1.0f;

	*ca->toAnimate = vec4ColorInterpolate(ca->originalNormalised, ca->targetNormalised, t);

	// Check if the animation has finished
	if(ca->base.currentTime >= ca->base.duration) {
		ca->base.running = false;
		*ca->toAnimate = vec4ToColor(ca->targetNormalised); // Ensure the final value is exact
		//.f(ca->base.onFinish.data);
		if(ca->base.onFinish) {
			applyCallback(ca->base.onFinish);
		}
	}
}

ColAnimation *createColourFadeAnimation(bool *trigger, Color *toAnimate, Color target, bool match, float duration, bool fadeIn) {
	ColAnimation *ca = (ColAnimation *)malloc(sizeof(ColAnimation));

	ca->base.type = AT_COLOUR;
	ca->base.trigger = trigger;
	ca->base.match = match;
	ca->base.running = false;
	ca->base.duration = duration;
	ca->base.currentTime = 0;
	if(fadeIn) {
		ca->base.animate = colourFadeInAnimation;
	} else {
		ca->base.animate = colourFadeOutAnimation;
	}

	ca->toAnimate = toAnimate;
	ca->originalNormalised = ColorNormalize(*toAnimate);
	ca->targetNormalised = ColorNormalize(target);

	return ca;
}

void createColourAnimationInOut(bool *trigger, Color *toAnimate, Color target, float transitionTime, float duration) {
	ColAnimation *caIn = createColourFadeAnimation(trigger, toAnimate, target, false, transitionTime, true);
	ColAnimation *caOut = createColourFadeAnimation(trigger, toAnimate, *toAnimate, true, transitionTime, false);
	Timer *t = createTimer(1.0, toggleAnimationCB, caOut);
	Animation *a = (Animation *)caIn;
	a->onFinish->data = t;
	a->onFinish->f = startTimerCallback;
}

void toggleAnimationCB(void *data) {
	Animation *a = (Animation *)data;
	a->running = !a->running;
}

TextBox *createTextBoxEx(Rectangle r, char *text, int maxLines, Color bgColour, Font f, int fontSize, Color txtColour, bool visible, TextOverflowRule overflow, int padding) {
	TextBox *tb = (TextBox *)malloc(sizeof(TextBox));
	if(!tb) {
		fprintf(stderr, "ERROR: createTextBox could not allocate memory for text box\n");
		return NULL;
	}

	int count = 0;
	const char **textLines = TextSplit(text, '\n', &count);

	Drawable *d = (Drawable *)tb;
	Animateable *ani = (Animateable *)tb;
	ani->animationCount = 0;

	d->draw = drawTextBox;
	d->bounds = r;
	tb->cBackground = bgColour;
	tb->cText = txtColour;
	tb->visible = visible;

	tb->padding = padding;
	tb->font = f;
	tb->fontSize = fontSize;

	tb->lineCount = 0;
	tb->maxLines = maxLines > 0 ? maxLines : MAX_TEXTBOX_LINES;

	tb->text = textLines;

	addDrawable(d);
	return tb;
}

TextBox *createTextBox(int x, int y, int w, int h, char *text, Color bgCol, Color txtCol) {
	Font systemFont = LoadFont(getConfigValueString("systemFontPath"));
	TextBox *tb = createTextBoxEx((Rectangle){ x, y, w, h }, text, 1, bgCol, systemFont, 12, txtCol, true, TOR_EXPAND, 5);

	return tb;
}

void drawTextBox(void *self) {
	TextBox *tb = (TextBox *)self;
	Drawable *d = (Drawable *)tb;

	DrawRectangleRec(d->bounds, tb->cBackground);
	for(int i = 0; tb->text[i]; i++) {
		int textWidth = MeasureText(tb->text[i], tb->fontSize);
		DrawTextEx(tb->font, tb->text[i], (Vector2){ d->bounds.x + tb->padding * i, d->bounds.y + ((float)tb->padding + (float)tb->fontSize / 4) * i }, tb->fontSize, 2, tb->cText);
	}
}
