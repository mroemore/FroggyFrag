#include "button.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <raylib.h>

void updateButton(void *self) {
	Button *btn = (Button *)self;
	Drawable *d = (Drawable *)btn;

	if(d->parent != NULL) {
		d->renderPos.x = d->parent->renderPos.x + d->offset.x;
		d->renderPos.y = d->parent->renderPos.y + d->offset.y;
	}

	btn->isHovered = isButtonHovered(btn);
	btn->cBackground = btn->isHovered ? btn->cHover : btn->cNormal;

	if(btn->isHovered && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
		printf("Button clicked! onClick=%p, userData=%p\n", (void*)btn->onClick, btn->userData);
		if(btn->onClick != NULL) {
			btn->onClick(btn->userData);
		}
	}
}

void drawButton(void *self) {
	Button *btn = (Button *)self;
	Drawable *d = (Drawable *)btn;

	if(!d->visible) return;

	DrawRectangle(d->renderPos.x, d->renderPos.y, d->width, d->height, btn->cBackground);

	if(btn->isHovered) {
		DrawRectangleLines(d->renderPos.x, d->renderPos.y, d->width, d->height, btn->cText);
	}

	if(btn->label != NULL) {
		Vector2 textSize = MeasureTextEx(btn->font, btn->label, btn->fontSize, 2);
		Vector2 textPos = {
			d->renderPos.x + (d->width - textSize.x) / 2,
			d->renderPos.y + (d->height - textSize.y) / 2
		};
		DrawTextEx(btn->font, btn->label, textPos, btn->fontSize, 2, btn->cText);
	}
}

bool isButtonHovered(Button *btn) {
	Drawable *d = (Drawable *)btn;
	Vector2 mousePos = GetMousePosition();
	Rectangle bounds = {d->renderPos.x, d->renderPos.y, d->width, d->height};
	return CheckCollisionPointRec(mousePos, bounds);
}

bool isButtonClicked(Button *btn) {
	return btn->isHovered && IsMouseButtonReleased(MOUSE_LEFT_BUTTON);
}

Button *createButton(Drawable *parent, int x, int y, int w, int h, const char *label, Color bgCol, Color txtCol, Font f, int fontSize, ButtonCallback onClick, void *userData) {
	Button *btn = (Button *)malloc(sizeof(Button));
	if(!btn) {
		fprintf(stderr, "ERROR: createButton could not allocate memory\n");
		return NULL;
	}

	initDrawableProperties((Drawable *)btn, parent, x, y, w, h, bgCol, drawButton, updateButton);
	Animateable *ani = (Animateable *)btn;
	ani->animationCount = 0;

	btn->label = strdup(label);
	btn->onClick = onClick;
	btn->userData = userData;
	btn->cBackground = bgCol;
	btn->cText = txtCol;
	btn->cNormal = bgCol;
	btn->cHover = (Color){
		.r = bgCol.r < 255 ? bgCol.r + 30 : 255,
		.g = bgCol.g < 255 ? bgCol.g + 30 : 255,
		.b = bgCol.b < 255 ? bgCol.b + 30 : 255,
		.a = bgCol.a
	};
	btn->font = f;
	btn->fontSize = fontSize;
	btn->isHovered = false;
	btn->wasPressed = false;

	return btn;
}
