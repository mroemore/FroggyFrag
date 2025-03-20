#include "gui.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <raylib.h>

#include "conf.h"
#include "timer.h"
#include "animation.h"

Drawable *drawableList[MAX_DRAWABLES];
int drawableListCount = 0;

void addDrawable(Drawable *d) {
	if(drawableListCount < MAX_DRAWABLES) {
		drawableList[drawableListCount] = d;
		drawableListCount++;
	}
}

void drawItems() {
	for(int i = 0; drawableList[i]; i++) {
		drawableList[i]->draw(drawableList[i]);
	}
}

ColAnimation *createColourAnimationInOut(Animateable *a, bool *trigger, Color *toAnimate, Color target, float transitionTime, float duration) {
	ColAnimation *caIn = createColourFadeAnimation(trigger, toAnimate, target, true, transitionTime, true);
	registerAnimationInAnimateable(a, (Animation *)caIn);
	ColAnimation *caOut = createColourFadeAnimation(trigger, toAnimate, target, true, transitionTime, false);
	registerAnimationInAnimateable(a, (Animation *)caOut);
	Timer *t = createTimer(TT_REPEATING, duration, toggleAnimationCB, caOut);
	printf("duration: %f\n", t->duration);
	caIn->base.onFinish->data = t;
	caIn->base.onFinish->f = startTimerCallback;
	addAnimation((Animation *)caIn);
	addAnimation((Animation *)caOut);
	return caIn;
}

void registerAnimationInAnimateable(Animateable *a, Animation *animation) {
	if(a->animationCount < MAX_ANIMATIONS_PER_DRAWABLE) {
		a->animationList[a->animationCount] = animation;
		a->animationCount++;
	}
}

void resetAnimateableAnimations(Animateable *a) {
	for(int i = 0; i < a->animationCount; i++) {
		resetAnimation(a->animationList[i]);
	}
}

TextBox *createTextBoxEx(Rectangle r, char *text, int maxLines, Color bgColour, Font f, int fontSize, Color txtColour, bool visible, TextOverflowRule overflow, int padding) {
	TextBox *tb = (TextBox *)malloc(sizeof(TextBox));
	if(!tb) {
		fprintf(stderr, "ERROR: createTextBox could not allocate memory for text box\n");
		return NULL;
	}

	Drawable *d = (Drawable *)tb;
	Animateable *ani = (Animateable *)tb;
	ani->animationCount = 0;

	d->draw = drawTextBox;
	d->bounds = r;
	d->visible = visible;

	tb->cBackground = bgColour;
	tb->cText = txtColour;

	tb->padding = padding;
	tb->font = f;
	tb->fontSize = fontSize;

	tb->text = TextSplit(text, '\n', &tb->lineCount);
	tb->maxLines = maxLines > 0 ? maxLines : MAX_TEXTBOX_LINES;

	addDrawable(d);
	return tb;
}

TextBox *createNotificationTextBox(int x, int y, int w, int h, char *text, Color bgCol, Color txtCol) {
	Font systemFont = LoadFont(getConfigValueString("systemFontPath"));
	TextBox *tb = createTextBoxEx((Rectangle){ x, y, w, h }, text, 1, bgCol, systemFont, 12, txtCol, false, TOR_EXPAND, 5);
	createColourAnimationInOut((Animateable *)tb, &((Drawable *)tb)->visible, &tb->cBackground, (Color){ 0, 0, 0, 0 }, 0.2f, 3.0f);
	createColourAnimationInOut((Animateable *)tb, &((Drawable *)tb)->visible, &tb->cText, (Color){ 0, 0, 0, 0 }, 0.2f, 3.0f);
	tb->cBackground = (Color){ 0, 0, 0, 0 };
	tb->cText = (Color){ 0, 0, 0, 0 };
	return tb;
}

void drawTextBox(void *self) {
	TextBox *tb = (TextBox *)self;
	Drawable *d = (Drawable *)tb;

	DrawRectangleRec(d->bounds, tb->cBackground);
	for(int i = 0; tb->text[i]; i++) {
		int textWidth = MeasureText(tb->text[i], tb->fontSize);
		DrawTextEx(tb->font, tb->text[i], (Vector2){ d->bounds.x + tb->padding, d->bounds.y + ((float)tb->padding + (float)tb->fontSize / 3) * i }, tb->fontSize, 2, tb->cText);
	}
}

void triggerNotificationFade(TextBox *tb) {
	((Drawable *)tb)->visible = true;
	resetAnimateableAnimations((Animateable *)tb);
	toggleAnimationCB(tb->base.animationList[0]);
	toggleAnimationCB(tb->base.animationList[2]);
}

void newNotification(TextBox *tb, char *newText) {
	tb->text = TextSplit(newText, '\n', &tb->lineCount);
	triggerNotificationFade(tb);
}

void toggleTextBoxVisibilityCB(void *textBox) {
	TextBox *tb = (TextBox *)textBox;
	((Drawable *)tb)->visible = !((Drawable *)tb)->visible;
}
