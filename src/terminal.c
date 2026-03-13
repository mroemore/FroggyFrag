#include "terminal.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <pty.h>
#include <sys/select.h>
#include <sys/wait.h>

#include <vterm.h>

#include "conf.h"

static int terminalDamageCallback(VTermRect rect, void *user) {
	TerminalOverlay *term = (TerminalOverlay *)user;
	term->dirty = true;
	return 1;
}

static int terminalMoveCursorCallback(VTermPos pos, VTermPos oldpos, int visible, void *user) {
	TerminalOverlay *term = (TerminalOverlay *)user;
	term->cursorRow = pos.row;
	term->cursorCol = pos.col;
	term->cursorVisible = visible;
	term->dirty = true;
	return 1;
}

static void terminalOutputCallback(const char *s, size_t len, void *user) {
	TerminalOverlay *term = (TerminalOverlay *)user;
	if(term->masterFd >= 0) {
		write(term->masterFd, s, len);
	}
}

static VTermScreenCallbacks screenCallbacks = {
	.damage = terminalDamageCallback,
	.movecursor = terminalMoveCursorCallback,
	.moverect = NULL,
	.settermprop = NULL,
	.bell = NULL,
	.resize = NULL,
	.sb_pushline = NULL,
	.sb_popline = NULL,
	.sb_clear = NULL
};

static void setNonBlocking(int fd) {
	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

static VTermKey raylibKeyToVTermKey(int raylibKey) {
	switch(raylibKey) {
		case KEY_ENTER: return VTERM_KEY_ENTER;
		case KEY_TAB: return VTERM_KEY_TAB;
		case KEY_BACKSPACE: return VTERM_KEY_BACKSPACE;
		case KEY_ESCAPE: return VTERM_KEY_ESCAPE;
		case KEY_UP: return VTERM_KEY_UP;
		case KEY_DOWN: return VTERM_KEY_DOWN;
		case KEY_LEFT: return VTERM_KEY_LEFT;
		case KEY_RIGHT: return VTERM_KEY_RIGHT;
		case KEY_INSERT: return VTERM_KEY_INS;
		case KEY_DELETE: return VTERM_KEY_DEL;
		case KEY_HOME: return VTERM_KEY_HOME;
		case KEY_END: return VTERM_KEY_END;
		case KEY_PAGE_UP: return VTERM_KEY_PAGEUP;
		case KEY_PAGE_DOWN: return VTERM_KEY_PAGEDOWN;
		case KEY_F1: return VTERM_KEY_FUNCTION(1);
		case KEY_F2: return VTERM_KEY_FUNCTION(2);
		case KEY_F3: return VTERM_KEY_FUNCTION(3);
		case KEY_F4: return VTERM_KEY_FUNCTION(4);
		case KEY_F5: return VTERM_KEY_FUNCTION(5);
		case KEY_F6: return VTERM_KEY_FUNCTION(6);
		case KEY_F7: return VTERM_KEY_FUNCTION(7);
		case KEY_F8: return VTERM_KEY_FUNCTION(8);
		case KEY_F9: return VTERM_KEY_FUNCTION(9);
		case KEY_F10: return VTERM_KEY_FUNCTION(10);
		case KEY_F11: return VTERM_KEY_FUNCTION(11);
		case KEY_F12: return VTERM_KEY_FUNCTION(12);
		default: return VTERM_KEY_NONE;
	}
}

static VTermModifier getModifiers(void) {
	VTermModifier mod = VTERM_MOD_NONE;
	if(IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) mod |= VTERM_MOD_SHIFT;
	if(IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) mod |= VTERM_MOD_CTRL;
	if(IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)) mod |= VTERM_MOD_ALT;
	return mod;
}

TerminalOverlay *createTerminalOverlay(Drawable *parent, int x, int y, int w, int h, const char *fontPath, int fontSize) {
	TerminalOverlay *term = (TerminalOverlay *)malloc(sizeof(TerminalOverlay));
	if(!term) {
		fprintf(stderr, "ERROR: Could not allocate TerminalOverlay\n");
		return NULL;
	}

	term->rows = TERMINAL_DEFAULT_ROWS;
	term->cols = TERMINAL_DEFAULT_COLS;
	term->masterFd = -1;
	term->childPid = -1;
	term->fontSize = fontSize;
	term->bgColor = (Color){ 20, 20, 30, 230 };
	term->fgColor = (Color){ 220, 220, 220, 255 };
	term->bgAlpha = 0.9f;
	term->dirty = true;
	term->cursorVisible = true;
	term->cursorRow = 0;
	term->cursorCol = 0;
	memset(term->outputBuffer, 0, TERMINAL_MAX_OUTPUT_BUFFER);

	Animateable *ani = (Animateable *)term;
	ani->animationCount = 0;

	initDrawableProperties((Drawable *)term, parent, x, y, w, h, term->bgColor, drawTerminalOverlay, updateTerminalOverlay);

	term->font = LoadFont(fontPath);

	term->vterm = vterm_new(term->rows, term->cols);
	if(!term->vterm) {
		fprintf(stderr, "ERROR: Could not create VTerm\n");
		free(term);
		return NULL;
	}
	vterm_set_utf8(term->vterm, 1);

	term->screen = vterm_obtain_screen(term->vterm);
	if(!term->screen) {
		fprintf(stderr, "ERROR: Could not obtain VTerm screen\n");
		vterm_free(term->vterm);
		free(term);
		return NULL;
	}
	vterm_screen_set_callbacks(term->screen, &screenCallbacks, term);
	vterm_screen_set_damage_merge(term->screen, VTERM_DAMAGE_CELL);
	vterm_screen_reset(term->screen, 1);

	vterm_output_set_callback(term->vterm, terminalOutputCallback, term);

	((Drawable *)term)->visible = false;

	return term;
}

void destroyTerminalOverlay(TerminalOverlay *term) {
	if(!term) return;

	terminateTerminalProcess(term);

	if(term->screen) {
	}
	if(term->vterm) {
		vterm_free(term->vterm);
	}

	UnloadFont(term->font);
	free(term);
}

bool spawnTerminalProcess(TerminalOverlay *term, const char *workingDir) {
	if(term->childPid > 0) {
		return true;
	}

	int masterFd, slaveFd;
	char slaveName[256];

	if(openpty(&masterFd, &slaveFd, slaveName, NULL, NULL) < 0) {
		perror("openpty");
		return false;
	}

	term->masterFd = masterFd;
	setNonBlocking(masterFd);

	pid_t pid = fork();
	if(pid < 0) {
		perror("fork");
		close(masterFd);
		close(slaveFd);
		term->masterFd = -1;
		return false;
	}

	if(pid == 0) {
		close(masterFd);

		setsid();

		dup2(slaveFd, STDIN_FILENO);
		dup2(slaveFd, STDOUT_FILENO);
		dup2(slaveFd, STDERR_FILENO);

		if(slaveFd > STDERR_FILENO) {
			close(slaveFd);
		}

		if(workingDir) {
			chdir(workingDir);
		}

		setenv("TERM", "xterm-256color", 1);
		setenv("COLORTERM", "truecolor", 1);

		execlp("nvim", "nvim", NULL);

		execlp("vim", "vim", NULL);

		fprintf(stderr, "Could not find nvim or vim\n");
		_exit(1);
	}

	term->childPid = pid;
	close(slaveFd);

	return true;
}

void terminateTerminalProcess(TerminalOverlay *term) {
	if(term->childPid > 0) {
		kill(term->childPid, SIGTERM);
		int status;
		waitpid(term->childPid, &status, 0);
		term->childPid = -1;
	}

	if(term->masterFd >= 0) {
		close(term->masterFd);
		term->masterFd = -1;
	}
}

void updateTerminalOverlay(void *self) {
	TerminalOverlay *term = (TerminalOverlay *)self;
	Drawable *d = (Drawable *)self;

	if(d->parent != NULL) {
		d->renderPos.x = d->parent->renderPos.x + d->offset.x;
		d->renderPos.y = d->parent->renderPos.y + d->offset.y;
	}

	if(!d->visible || term->masterFd < 0) {
		return;
	}

	fd_set readFds;
	struct timeval timeout;
	FD_ZERO(&readFds);
	FD_SET(term->masterFd, &readFds);
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	if(select(term->masterFd + 1, &readFds, NULL, NULL, &timeout) > 0) {
		char buffer[4096];
		ssize_t bytesRead = read(term->masterFd, buffer, sizeof(buffer) - 1);
		if(bytesRead > 0) {
			buffer[bytesRead] = '\0';
			vterm_input_write(term->vterm, buffer, bytesRead);
			term->dirty = true;
		} else if(bytesRead < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
			terminateTerminalProcess(term);
		}
	}

	int status;
	if(term->childPid > 0 && waitpid(term->childPid, &status, WNOHANG) > 0) {
		term->childPid = -1;
		if(term->masterFd >= 0) {
			close(term->masterFd);
			term->masterFd = -1;
		}
	}
}

void drawTerminalOverlay(void *self) {
	TerminalOverlay *term = (TerminalOverlay *)self;
	Drawable *d = (Drawable *)self;

	if(!d->visible) return;

	Color bgWithAlpha = term->bgColor;
	bgWithAlpha.a = (unsigned char)(term->bgAlpha * 255);

	DrawRectangle(d->renderPos.x, d->renderPos.y, d->width, d->height, bgWithAlpha);

	float cellWidth = (float)d->width / term->cols;
	float cellHeight = (float)d->height / term->rows;

	VTermPos pos;
	for(pos.row = 0; pos.row < term->rows; pos.row++) {
		for(pos.col = 0; pos.col < term->cols; pos.col++) {
			VTermScreenCell cell;
			if(vterm_screen_get_cell(term->screen, pos, &cell) == 0) {
				continue;
			}

			bool isDefaultBg = VTERM_COLOR_IS_DEFAULT_BG(&cell.bg);
			bool isDefaultFg = VTERM_COLOR_IS_DEFAULT_FG(&cell.fg);

			VTermColor bg = cell.bg;
			VTermColor fg = cell.fg;

			if(!isDefaultBg) {
				vterm_screen_convert_color_to_rgb(term->screen, &bg);
			}
			if(!isDefaultFg) {
				vterm_screen_convert_color_to_rgb(term->screen, &fg);
			}

			float cellX = d->renderPos.x + pos.col * cellWidth;
			float cellY = d->renderPos.y + pos.row * cellHeight;

			if(!isDefaultBg) {
				DrawRectangle(cellX, cellY, cellWidth + 1, cellHeight + 1, 
					(Color){ bg.rgb.red, bg.rgb.green, bg.rgb.blue, 255 });
			}

			if(cell.chars[0] != 0 && cell.chars[0] != ' ') {
				char utf8[16];
				int len = 0;
				uint32_t ch = cell.chars[0];
				
				if(ch < 0x80) {
					utf8[len++] = (char)ch;
				} else if(ch < 0x800) {
					utf8[len++] = (char)(0xC0 | (ch >> 6));
					utf8[len++] = (char)(0x80 | (ch & 0x3F));
				} else if(ch < 0x10000) {
					utf8[len++] = (char)(0xE0 | (ch >> 12));
					utf8[len++] = (char)(0x80 | ((ch >> 6) & 0x3F));
					utf8[len++] = (char)(0x80 | (ch & 0x3F));
				} else {
					utf8[len++] = (char)(0xF0 | (ch >> 18));
					utf8[len++] = (char)(0x80 | ((ch >> 12) & 0x3F));
					utf8[len++] = (char)(0x80 | ((ch >> 6) & 0x3F));
					utf8[len++] = (char)(0x80 | (ch & 0x3F));
				}
				utf8[len] = '\0';

				Color textColor;
				if(isDefaultFg) {
					textColor = term->fgColor;
				} else {
					textColor = (Color){ fg.rgb.red, fg.rgb.green, fg.rgb.blue, 255 };
				}
				DrawTextEx(term->font, utf8, 
					(Vector2){ cellX, cellY }, 
					term->fontSize, 1, textColor);
			}
		}
	}

	if(term->cursorVisible) {
		float cursorX = d->renderPos.x + term->cursorCol * cellWidth;
		float cursorY = d->renderPos.y + term->cursorRow * cellHeight;
		DrawRectangle(cursorX, cursorY, cellWidth, 2, term->fgColor);
	}

	DrawRectangleLines(d->renderPos.x, d->renderPos.y, d->width, d->height, 
		(Color){ 100, 100, 120, 255 });
}

void handleTerminalInput(TerminalOverlay *term) {
	if(!term || !((Drawable *)term)->visible || term->masterFd < 0) return;

	int keyPressed = GetCharPressed();
	while(keyPressed > 0) {
		VTermModifier mod = getModifiers();
		vterm_keyboard_unichar(term->vterm, (uint32_t)keyPressed, mod);
		keyPressed = GetCharPressed();
	}

	int key = GetKeyPressed();
	while(key > 0) {
		VTermKey vkey = raylibKeyToVTermKey(key);
		if(vkey != VTERM_KEY_NONE) {
			VTermModifier mod = getModifiers();
			vterm_keyboard_key(term->vterm, vkey, mod);
		}
		key = GetKeyPressed();
	}

	vterm_screen_flush_damage(term->screen);
}

void toggleTerminalOverlay(TerminalOverlay *term) {
	if(!term) {
		printf("toggleTerminalOverlay: term is NULL!\n");
		return;
	}
	Drawable *d = (Drawable *)term;
	d->visible = !d->visible;
	printf("Terminal toggled: visible=%d\n", d->visible);

	if(d->visible && term->childPid < 0) {
		const char *shaderFolder = getConfigValueString("shaderFolder");
		printf("Spawning terminal in: %s\n", shaderFolder);
		spawnTerminalProcess(term, shaderFolder);
	}
}

bool isTerminalOverlayVisible(TerminalOverlay *term) {
	if(!term) return false;
	return ((Drawable *)term)->visible;
}

void showTerminalOverlay(TerminalOverlay *term) {
	if(!term) return;
	Drawable *d = (Drawable *)term;
	d->visible = true;

	if(term->childPid < 0) {
		const char *shaderFolder = getConfigValueString("shaderFolder");
		spawnTerminalProcess(term, shaderFolder);
	}
}

void hideTerminalOverlay(TerminalOverlay *term) {
	if(!term) return;
	Drawable *d = (Drawable *)term;
	d->visible = false;
}
