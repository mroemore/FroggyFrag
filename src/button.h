#ifndef BUTTON_H
#define BUTTON_H

#include "gui.h"

typedef void (*ButtonCallback)(void *userData);

typedef struct {
	Animateable base;
	const char *label;
	ButtonCallback onClick;
	void *userData;
	Color cBackground;
	Color cText;
	Color cHover;
	Color cNormal;
	Font font;
	int fontSize;
	bool isHovered;
	bool wasPressed;
} Button;

Button *createButton(Drawable *parent, int x, int y, int w, int h, const char *label, Color bgCol, Color txtCol, Font f, int fontSize, ButtonCallback onClick, void *userData);
void updateButton(void *self);
void drawButton(void *self);
bool isButtonHovered(Button *btn);
bool isButtonClicked(Button *btn);

#endif
