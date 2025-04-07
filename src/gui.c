#include "gui.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <raylib.h>

#include "conf.h"
#include "animation.h"
#include "ease.h"

Drawable rootElement;

void initRootDrawable(int screenW, int screenH) {
	rootElement.childCount = 0;
	rootElement.renderPos = (Vector2){ 0, 0 };
	rootElement.offset = (Vector2){ 0, 0 };
	rootElement.width = screenW;
	rootElement.height = screenH;
	rootElement.parent = NULL;
	rootElement.update = NULL;
	rootElement.visible = true;
}

void updateGraph(Drawable *d) {
	if(d->update != NULL) {
		d->update(d);
	}

	if(d->childCount > 0) {
		for(int i = 0; i < d->childCount; i++) {
			updateGraph(d->children[i]);
		}
	}
}

void updateDrawables() {
	updateGraph(&rootElement);
}

void drawAll() {
	drawChildren(&rootElement);
}

void drawChildren(Drawable *d) {
	for(int i = 0; i < d->childCount; i++) {
		d->children[i]->draw(d->children[i]);
		drawChildren(d->children[i]);
	}
}

void addChildDrawable(Drawable *p, Drawable *c) {
	if(p == NULL || c == NULL) return;
	if(p->childCount < MAX_DRAWABLE_CHILDREN) {
		c->parent = p;
		p->children[p->childCount] = c;
		p->childCount++;
	}
}

void addDrawableToRoot(Drawable *c) {
	if(c == NULL) return;
	if(rootElement.childCount < MAX_DRAWABLE_CHILDREN) {
		c->parent = &rootElement;
		rootElement.children[rootElement.childCount] = c;
		rootElement.childCount++;
	}
}

void createPopDownAnimation(Animateable *a, bool *trigger, Rectangle *toAnimate, Rectangle offset, int transitionFrames) {
}

void createDancingFroggyAnimation(Animateable *a, bool *trigger, AnimatedImage *toAnimate, int transitionFrames) {
}

void registerAnimationGroup(Animateable *a, AnimationChainGroup *ac) {
	a->animationList[a->animationCount] = ac;
	a->animationCount++;
}

void resetAnimateableAnimations(Animateable *a) {
}

void initDrawableProperties(Drawable *d, Drawable *parent, int x, int y, int w, int h, Color cDefault, DrawCallback drawFunc, UpdateCallback updateFunc) {
	if(d == NULL) return;
	d->parent = parent;
	d->childCount = 0;
	d->draw = drawFunc;
	d->update = updateFunc;
	d->cDefault = cDefault;
	d->visible = true;

	if(parent != NULL) {
		addChildDrawable(parent, d);
		d->offset = (Vector2){ x, y };
		d->renderPos = (Vector2){ parent->renderPos.x + x, parent->renderPos.y + y };
	} else {
		d->offset = (Vector2){ x, y };
		d->renderPos = (Vector2){ x, y };
	}
	d->width = w;
	d->height = h;
}

bool populateTextBoxLines(TextBox *tb, const char *text) {
	char *textCopy = strdup(text);
	if(!textCopy) {
		fprintf(stderr, "ERROR: Could not duplicate text\n");
		free(tb);
		return false;
	}

	int lineCount = 0;
	int longestLineWidth = 0;
	const char **splitLines = TextSplit(textCopy, '\n', &lineCount);
	tb->lineCount = lineCount;

	if(lineCount > 0) {
		tb->text = (const char **)malloc(lineCount * sizeof(char *));
		if(!tb->text) {
			fprintf(stderr, "ERROR: Could not allocate memory for text lines\n");
			free(textCopy);
			return false;
		}

		// Copy each line
		for(int i = 0; i < lineCount; i++) {
			tb->text[i] = strdup(splitLines[i]);
			Vector2 size = MeasureTextEx(tb->font, tb->text[i], tb->fontSize, 2);
			longestLineWidth = size.x > longestLineWidth ? size.x : longestLineWidth;
		}
	} else {
		tb->text = (const char **)malloc(sizeof(char *));
		tb->text[0] = strdup("");
	}

	tb->base.base.width = longestLineWidth + tb->padding.x;

	free(textCopy);
	return true;
}

TextBox *createTextBoxEx(Drawable *parent, int x, int y, int w, int h, Vector4 padding, const char *text, int maxLines, Color bgColour, Font f, int fontSize, Color txtColour) {
	TextBox *tb = (TextBox *)malloc(sizeof(TextBox));
	if(!tb) {
		fprintf(stderr, "ERROR: createTextBox could not allocate memory for text box\n");
		return NULL;
	}
	initDrawableProperties((Drawable *)tb, parent, x, y, w, h, bgColour, drawTextBox, updateTextBox);
	Animateable *ani = (Animateable *)tb;
	ani->animationCount = 0;
	tb->cBackground = bgColour;
	tb->cText = txtColour;

	tb->padding = padding;
	tb->font = f;
	tb->fontSize = fontSize;
	if(!populateTextBoxLines(tb, text)) {
		free(tb);
		return NULL;
	}
	tb->maxLines = maxLines > 0 ? maxLines : MAX_TEXTBOX_LINES;
	return tb;
}

TextBox *createSettingsNotificationBox(int x, int y, int w, int h, char *text, Color bgCol, Color txtCol) {
	Font systemFont = LoadFont(getConfigValueString("systemFontPath"));

	TextBox *snb = createTextBoxEx(NULL, x, y, w, h, (Vector4){ 55, 5, 5, 5 }, text, 5, bgCol, systemFont, 20, txtCol);
	AnimatedImage *frog = createAnimatedImage((Drawable *)snb, 5, 5, 0, 0, WHITE, "resources/FroggyOutlined64px.png", 0.5, 0.0);
	addChildDrawable((Drawable *)snb, (Drawable *)frog);
	Animateable *anim = (Animateable *)snb;
	Drawable *d = (Drawable *)snb;

	int restartBehaviour = CRB_CUSTOM_THEN_RESTART;
	AnimationChainGroup *popUp = (AnimationChainGroup *)malloc(sizeof(AnimationChainGroup));
	initAnimationChainGroup(popUp, ACGP_SEQUENTIAL);

	AnimationChain opacityFade;
	initAnimationChain(&opacityFade, ACPT_ONCE, restartBehaviour);
	unsigned char opacityDest = 0;
	addAnimation(&opacityFade, 30, ATT_UCHAR, IT_CUBIC_OUT, &d->cDefault.a, &opacityDest, true);
	addRest(&opacityFade, 60);
	addAnimation(&opacityFade, 30, ATT_UCHAR, IT_CUBIC_OUT, &d->cDefault.a, &opacityDest, false);
	setCustomRetrigAnimation(&opacityFade, 3, ATT_UCHAR, IT_LINEAR, &d->cDefault.a, &opacityDest, false);
	addChainToGroup(popUp, opacityFade);
	d->cDefault.a = 0;

	AnimationChain positionSlide;
	initAnimationChain(&positionSlide, ACPT_ONCE, restartBehaviour);
	float yPosDest = 10.0;
	addAnimation(&positionSlide, 30, ATT_FLOAT, IT_QUINT_OUT, &d->offset.y, &yPosDest, false);
	addRest(&positionSlide, 60);
	addAnimation(&positionSlide, 30, ATT_FLOAT, IT_QUINT_OUT, &d->offset.y, &yPosDest, true);
	setCustomRetrigAnimation(&positionSlide, 3, ATT_FLOAT, IT_LINEAR, &d->offset.y, &yPosDest, true);
	addChainToGroup(popUp, positionSlide);

	registerAnimationGroup(anim, popUp);
	addAnimationGroupToManager(popUp);

	// dancing froggy animation stuff
	Drawable *fd = (Drawable *)frog;

	restartBehaviour = CRB_DO_NOTHING;
	AnimationChainGroup *dancingFrog = (AnimationChainGroup *)malloc(sizeof(AnimationChainGroup));
	initAnimationChainGroup(dancingFrog, ACGP_SEQUENTIAL);

	AnimationChain frogWiggle;
	initAnimationChain(&frogWiggle, ACPT_LOOP, CRB_DO_NOTHING);
	float xPosDest = 30.0;
	addAnimation(&frogWiggle, 16, ATT_FLOAT, IT_QUINT_OUT, &fd->offset.x, &xPosDest, false);
	addRest(&frogWiggle, 8);
	addAnimation(&frogWiggle, 16, ATT_FLOAT, IT_QUINT_OUT, &fd->offset.x, &xPosDest, true);
	addRest(&frogWiggle, 8);
	addChainToGroup(dancingFrog, frogWiggle);

	AnimationChain frogSquish;
	initAnimationChain(&frogSquish, ACPT_LOOP, CRB_DO_NOTHING);
	int heightDest = 50;
	addAnimation(&frogSquish, 8, ATT_INT, IT_ELASTIC_OUT, &fd->height, &heightDest, false);
	addAnimation(&frogSquish, 8, ATT_INT, IT_ELASTIC_OUT, &fd->height, &heightDest, true);
	addRest(&frogSquish, 8);
	addChainToGroup(dancingFrog, frogSquish);

	AnimationChain frogLean;
	initAnimationChain(&frogLean, ACPT_LOOP, CRB_DO_NOTHING);
	float rotationDest = 15.0;
	addAnimation(&frogLean, 16, ATT_FLOAT, IT_CUBIC_OUT, &frog->rotation, &rotationDest, false);
	addRest(&frogLean, 8);
	addAnimation(&frogLean, 16, ATT_FLOAT, IT_CUBIC_OUT, &frog->rotation, &rotationDest, true);
	addRest(&frogLean, 8);
	addChainToGroup(dancingFrog, frogLean);

	registerAnimationGroup(anim, dancingFrog);
	addAnimationGroupToManager(dancingFrog);
	return snb;
}

void updateTextBox(void *self) {
	Drawable *d = (Drawable *)self;
	if(d->parent != NULL) {
		d->renderPos.x = d->parent->renderPos.x + d->offset.x;
		d->renderPos.y = d->parent->renderPos.y + d->offset.y;

		TextBox *tb = (TextBox *)self;
		tb->cBackground.a = d->cDefault.a;
		tb->cText.a = d->cDefault.a;
	}
}

void drawTextBox(void *self) {
	TextBox *tb = (TextBox *)self;
	Drawable *d = (Drawable *)tb;

	DrawRectangle(d->renderPos.x, d->renderPos.y, d->width, d->height, tb->cBackground);
	for(int i = 0; i < tb->lineCount; i++) {
		if(!tb->text[i]) continue;
		int textWidth = MeasureText(tb->text[i], tb->fontSize);
		DrawTextEx(tb->font, tb->text[i], (Vector2){ d->renderPos.x + tb->padding.x, d->renderPos.y + ((float)tb->padding.y + (float)tb->fontSize / 3) * (i + 1) }, tb->fontSize, 2, tb->cText);
	}
}

void newSettingsInfo(TextBox *settingsInfoBox, const char *newText) {
	populateTextBoxLines(settingsInfoBox, newText);
	Animateable *a = (Animateable *)settingsInfoBox;
	startManagedGroup(a->animationList[0]);
	startManagedGroup(a->animationList[1]);
}

AnimatedImage *createAnimatedImage(Drawable *parent, int x, int y, int w, int h, Color cTint, const char *imagePath, float scale, float rotation) {
	AnimatedImage *img = (AnimatedImage *)malloc(sizeof(AnimatedImage));
	Image tmp = LoadImage(imagePath);
	int tw = w > 0 ? w : tmp.width;
	int th = h > 0 ? h : tmp.height;
	initDrawableProperties((Drawable *)img, parent, x + tw * scale / 2, y + th * scale / 2, tw, th, cTint, drawAnimatedImage, updateAnimatedImage);
	img->t = LoadTextureFromImage(tmp);
	img->scale = scale;
	img->rotation = rotation;
	img->srcBounds = (Rectangle){ 0.0, 0.0, (float)tw, (float)th };
	img->aspectRatio = (float)tw / (float)th;
	img->targetBounds = (Rectangle){ x, y, (float)tw * scale, (float)th * scale };
	UnloadImage(tmp);

	return img;
}

void updateAnimatedImage(void *self) {
	Drawable *d = (Drawable *)self;
	if(d->parent != NULL) {
		d->renderPos.x = d->parent->renderPos.x + d->offset.x;
		d->renderPos.y = d->parent->renderPos.y + d->offset.y;
		AnimatedImage *ani = (AnimatedImage *)self;
		ani->targetBounds.x = d->renderPos.x;
		ani->targetBounds.y = d->renderPos.y;
		ani->targetBounds.width = d->width * ani->scale;
		ani->targetBounds.height = d->height * ani->scale;
		d->cDefault.a = d->parent->cDefault.a;
	}
}

void drawAnimatedImage(void *self) {
	AnimatedImage *img = (AnimatedImage *)self;
	DrawTexturePro(img->t, img->srcBounds, img->targetBounds, (Vector2){ img->targetBounds.width / 2, img->targetBounds.height / 2 }, img->rotation, (Color){ 255, 255, 255, img->base.base.cDefault.a });
}

MessageBuffer *createMessageBuffer(Drawable *parent, int x, int y, int w, int h, Color cBackground, Color cText, const char *fontPath, int fontSize) {
	MessageBuffer *mb = (MessageBuffer *)malloc(sizeof(MessageBuffer));
	Drawable *d = (Drawable *)mb;
	Animateable *a = (Animateable *)mb;
	a->animationCount = 0;
	initDrawableProperties(d, parent, x, y, w, h, cBackground, drawMessageBuffer, updateMessageBuffer);
	mb->count = 0;
	mb->index = 0;
	mb->fontSize = fontSize;
	mb->lineVisibilityMod = 1.0;
	mb->cText = cText;
	mb->cBackground = cBackground;

	Font msgFont = LoadFont(fontPath);
	mb->font = msgFont;

	AnimationChainGroup *fadeIn = (AnimationChainGroup *)malloc(sizeof(AnimationChainGroup));
	initAnimationChainGroup(fadeIn, ACGP_SEQUENTIAL);

	AnimationChain opacityIn;
	initAnimationChain(&opacityIn, ACPT_ONCE, CRB_DO_NOTHING);
	unsigned char opacityDest = 0;
	addAnimation(&opacityIn, 60, ATT_UCHAR, IT_CUBIC_OUT, &d->cDefault.a, &opacityDest, false);

	AnimationChain visModIn;
	initAnimationChain(&visModIn, ACPT_ONCE, CRB_DO_NOTHING);
	float visModDest = 1.0;
	addAnimation(&visModIn, 60, ATT_UCHAR, IT_CUBIC_OUT, &mb->lineVisibilityMod, &visModDest, false);

	AnimationChain slideIn;
	initAnimationChain(&slideIn, ACPT_ONCE, CRB_DO_NOTHING);
	float xModIn = 30.0;
	addAnimation(&slideIn, 60, ATT_UCHAR, IT_CUBIC_OUT, &d->offset.x, &xModIn, false);

	addChainToGroup(fadeIn, opacityIn);
	addChainToGroup(fadeIn, visModIn);
	addChainToGroup(fadeIn, slideIn);

	AnimationChainGroup *fadeOut = (AnimationChainGroup *)malloc(sizeof(AnimationChainGroup));
	initAnimationChainGroup(fadeOut, ACGP_SEQUENTIAL);

	AnimationChain opacityOut;
	initAnimationChain(&opacityOut, ACPT_ONCE, CRB_DO_NOTHING);
	unsigned char opacityOutDest = 0;
	addAnimation(&opacityOut, 60, ATT_UCHAR, IT_CUBIC_OUT, &d->cDefault.a, &opacityOutDest, true);

	AnimationChain visModOut;
	initAnimationChain(&visModOut, ACPT_ONCE, CRB_DO_NOTHING);
	float visModOutDest = 1.0;
	addAnimation(&visModOut, 60, ATT_FLOAT, IT_CUBIC_OUT, &mb->lineVisibilityMod, &visModOutDest, true);

	AnimationChain slideOut;
	initAnimationChain(&slideOut, ACPT_ONCE, CRB_DO_NOTHING);
	float xModOut = 30.0;
	addAnimation(&slideOut, 60, ATT_FLOAT, IT_CUBIC_OUT, &d->offset.x, &xModOut, true);

	d->cDefault.a = 0;
	addChainToGroup(fadeOut, opacityOut);
	addChainToGroup(fadeOut, visModOut);
	addChainToGroup(fadeOut, slideOut);

	registerAnimationGroup(a, fadeIn);
	addAnimationGroupToManager(fadeIn);
	registerAnimationGroup(a, fadeOut);
	addAnimationGroupToManager(fadeOut);

	return mb;
}

void updateMessageBuffer(void *self) {
	Drawable *d = (Drawable *)self;
	MessageBuffer *mb = (MessageBuffer *)self;
	mb->cBackground.a = d->cDefault.a;
	mb->cText.a = d->cDefault.a;
	if(d->parent != NULL) {
		d->renderPos.x = d->parent->renderPos.x + d->offset.x;
		d->renderPos.y = d->parent->renderPos.y + d->offset.y;
	}
}

void drawMessageBuffer(void *self) {
	MessageBuffer *mb = (MessageBuffer *)self;
	Drawable *d = (Drawable *)self;
	for(int i = mb->count - 1; i >= 0; i--) {
		int index = (mb->index - i) % MAX_MESSAGE_BUFFER_LINES;
		DrawRectangle(d->renderPos.x, d->renderPos.y + i * (mb->fontSize + 2), 600, 18, mb->cBackground);
		DrawTextEx(mb->font, mb->messages[index], (Vector2){ d->renderPos.x, d->renderPos.y + i * (mb->fontSize + 2) }, mb->fontSize, 2, mb->cText);
	}
}

void toggleMessageBufferVisibility(MessageBuffer *mb) {
	Animateable *a = (Animateable *)mb;
	a->base.visible = !a->base.visible;
	if(a->base.visible) {
		startManagedGroup(a->animationList[0]);
	} else {
		startManagedGroup(a->animationList[1]);
	}
}
