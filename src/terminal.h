#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdbool.h>
#include <sys/types.h>

#include "gui.h"
#include "raylib.h"

#define TERMINAL_DEFAULT_ROWS 24
#define TERMINAL_DEFAULT_COLS 80
#define TERMINAL_MAX_OUTPUT_BUFFER 4096

typedef struct VTerm VTerm;
typedef struct VTermScreen VTermScreen;

typedef struct {
	Animateable base;
	VTerm *vterm;
	VTermScreen *screen;
	int masterFd;
	pid_t childPid;
	int rows;
	int cols;
	Font font;
	int fontSize;
	Color bgColor;
	Color fgColor;
	float bgAlpha;
	bool dirty;
	bool cursorVisible;
	int cursorRow;
	int cursorCol;
	char outputBuffer[TERMINAL_MAX_OUTPUT_BUFFER];
} TerminalOverlay;

TerminalOverlay *createTerminalOverlay(Drawable *parent, int x, int y, int w, int h, const char *fontPath, int fontSize);
void destroyTerminalOverlay(TerminalOverlay *term);

void updateTerminalOverlay(void *self);
void drawTerminalOverlay(void *self);

void toggleTerminalOverlay(TerminalOverlay *term);
bool isTerminalOverlayVisible(TerminalOverlay *term);
void showTerminalOverlay(TerminalOverlay *term);
void hideTerminalOverlay(TerminalOverlay *term);

void handleTerminalInput(TerminalOverlay *term);

bool spawnTerminalProcess(TerminalOverlay *term, const char *workingDir);
void terminateTerminalProcess(TerminalOverlay *term);

#endif
