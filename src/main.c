#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#include "raylib.h"
#include "conf.h"
#include "timer.h"
#include "animation.h"
#include "gui.h"

#define MAX_SHADERPATHS 256
#define MAX_IMAGEPATHS 256
#define MAX_MESSAGE_BUFFER_LINES 512
#define MAX_MESSAGE_LENGTH 1024

typedef struct {
	Font *font;
	uint8_t glyph_widths[255];
} FontInfo;

typedef struct {
	char *messages[MAX_MESSAGE_BUFFER_LINES];
	uint16_t count;
	uint16_t index;
} MessageBuffer;

typedef struct {
	Shader current;
	time_t currentModTime;
	char *shaderFolder;
	char *shaderPaths[MAX_SHADERPATHS];
	uint16_t loadedShaderCount;
	uint16_t currentShaderIndex;
	bool loadSuccessful;
	MessageBuffer errors;
	TextBox *errorNotification;
} ShaderManager;

typedef struct {
	bool running;
	float duration;
	float currentTime;
	Color *c;
	Color initial;
	Color final;
	Color increment;
	InterpType it;
	float doneThreshold;
} ColourAnimation;

// window resize functions
int updateScreenDimensions(Rectangle *sourceRect, Rectangle *destRect, int *screenWidth, int *screenHeight, float *virtualRatio);

// ShaderManager functions
void addShaderPath(ShaderManager *sm, char *newPath);
void initShaderManager(ShaderManager *sm, TextBox *notificationTB, char *folder);
void swapOrReloadShader(ShaderManager *sm, int index);
void incShaderIndex(ShaderManager *sm);
void decShaderIndex(ShaderManager *sm);
void rescanDirectory(ShaderManager *sm, char *folderPath);

// error capture
void redirect_stdout_to_file(const char *filename);

// message buffer functions
void initMessageBuffer(MessageBuffer *mb);
void pushMessage(MessageBuffer *mb, const char *msg);
void drawMessageBuffer(MessageBuffer *mb, Font f, Vector2 pos, int fontSize);

int main(void) {
	initGlobalConf();

	int screenWidth = getConfigValueInt("screenW");
	int screenHeight = getConfigValueInt("screenW");

	const int virtualScreenWidth = 320;
	const int virtualScreenHeight = 240;
	float virtualRatio = (float)screenWidth / (float)virtualScreenWidth;

	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
	InitWindow(screenWidth, screenHeight, "Shader Preview Tool");

	Font fontSystem = LoadFont(getConfigValueString("systemFontPath"));
	SetTextLineSpacing(8);

	Image bg = LoadImage(getConfigValueString("backgroundImagePath"));
	Texture2D bgTex = LoadTextureFromImage(bg);
	UnloadImage(bg);

	Camera2D worldSpaceCamera = { 0 }; // Game world camera
	worldSpaceCamera.zoom = 1.0f;

	Camera2D screenSpaceCamera = { 0 }; // Smoothing camera
	screenSpaceCamera.zoom = 1.0f;

	RenderTexture2D target = LoadRenderTexture(virtualScreenWidth, virtualScreenHeight); // This is where we'll draw all our objects.

	// The target's height is flipped (in the source Rectangle), due to OpenGL reasons
	Rectangle sourceRec = { 0.0f, 0.0f, (float)target.texture.width, -(float)target.texture.height };
	Rectangle destRec = { -virtualRatio, -virtualRatio, screenWidth + (virtualRatio * 2), screenHeight + (virtualRatio * 2) };

	Vector2 origin = { 0.0f, 0.0f };

	float cameraX = 0.0f;
	float cameraY = 0.0f;

	int loadedShaderCount = 0;
	int selectedShader = 0;
	ShaderManager sm;
	TextBox *notificationTB = createNotificationTextBox(10, 10, 750, 75, "default", (Color){ 0, 0, 0, 170 }, (Color){ 200, 200, 200, 200 });
	initShaderManager(&sm, notificationTB, getConfigValueString("shaderFolder"));

	float windowResolution[2] = { (float)screenWidth, (float)screenHeight };

	// TextBox *ttb = createNotificationTextBox(10, 10, 100, 100, "Hello,\n there.", (Color){ 0, 0, 0, 170 }, (Color){ 200, 200, 200, 200 });

	float reloadTimer = 0.0;
	bool drawConsole = false;
	float frameTime = 0.0;
	float reloadCheckInterval = getConfigValueFloat("reloadCheckInterval");
	bool autoReload = getConfigValueBool("autoReload");
	SetTargetFPS(60);

	while(!WindowShouldClose()) // Detect window close button or ESC key
	{
		frameTime = GetFrameTime();
		reloadTimer += frameTime;
		if(reloadTimer >= reloadCheckInterval && sm.loadSuccessful) {
			swapOrReloadShader(&sm, sm.currentShaderIndex);
			reloadTimer = 0.0;
		}
		if(updateScreenDimensions(&sourceRec, &destRec, &screenWidth, &screenHeight, &virtualRatio)) {
			// code here executes on resize.
			windowResolution[0] = (float)screenWidth;
			windowResolution[1] = (float)screenHeight;
		}

		if(IsKeyReleased(KEY_R)) {
			swapOrReloadShader(&sm, sm.currentShaderIndex);
		}
		if(IsKeyReleased(KEY_EQUAL) || IsKeyReleased(KEY_RIGHT)) {
			incShaderIndex(&sm);
		}
		if(IsKeyReleased(KEY_MINUS) || IsKeyReleased(KEY_LEFT)) {
			decShaderIndex(&sm);
		}
		if(IsKeyReleased(KEY_Q)) {
			drawConsole = !drawConsole;
		}
		if(IsKeyReleased(KEY_A)) {
			autoReload = !autoReload;
		}
		if(IsKeyReleased(KEY_F5)) {
			rescanDirectory(&sm, getConfigValueString("shaderFolder"));
		}

		float timeDelta = GetFrameTime();

		float time = GetTime();
		BeginTextureMode(target);
		BeginMode2D(worldSpaceCamera);
		DrawTexture(bgTex, 0, 0, GRAY);
		EndMode2D();
		EndTextureMode();

		BeginDrawing();
		ClearBackground(WHITE);
		BeginShaderMode(sm.current);
		SetShaderValue(sm.current, GetShaderLocation(sm.current, "iTime"), &time, SHADER_UNIFORM_FLOAT);

		SetShaderValue(sm.current, GetShaderLocation(sm.current, "windowResolution"), &windowResolution, SHADER_UNIFORM_VEC2);
		DrawTexturePro(target.texture, sourceRec, destRec, origin, 0.0f, WHITE);
		EndShaderMode();

		// DrawRectangle(2, 2, 600, 100, infoBgColour);
		// DrawTextEx(fontSystem, TextFormat("Screen resolution: %ix%i", screenWidth, screenHeight), (Vector2){ 10, 10 }, 14, 2, infoTxtColour);
		// DrawTextEx(fontSystem, TextFormat("World resolution: %ix%i", virtualScreenWidth, virtualScreenHeight), (Vector2){ 10, 34 }, 14, 2, infoTxtColour);
		// DrawTextEx(fontSystem, TextFormat("Current Shader: [%i/%i]%s", sm.currentShaderIndex, sm.loadedShaderCount, sm.shaderPaths[sm.currentShaderIndex]), (Vector2){ 10, 50 }, 14, 2, infoTxtColour);
		// DrawTextEx(fontSystem, "Keys: H: help. R: reload shader, A:auto-reload =: next shader, -:prev shader, Esc: quit", (Vector2){ 10, 68 }, 14, 2, infoTxtColour);

		if(drawConsole) {
			drawMessageBuffer(&sm.errors, fontSystem, (Vector2){ 10, 10 }, 20);
		}
		tickAnimations(frameTime);
		tickTimers(frameTime);
		drawItems();
		DrawFPS(GetScreenWidth() - 95, 10);
		EndDrawing();
	}
}

int updateScreenDimensions(Rectangle *sourceRect, Rectangle *destRect, int *screenWidth, int *screenHeight, float *virtualRatio) {
	int tmpW = GetScreenWidth();
	int tmpH = GetScreenHeight();
	int updated = 0;
	if(*screenWidth != tmpW || *screenHeight != tmpH) {
		updated = 1;
		*screenWidth = tmpW;
		*screenHeight = tmpH;
		*virtualRatio = (float)*screenWidth / (float)sourceRect->width;
		destRect->x = -*virtualRatio;
		destRect->y = -*virtualRatio;
		destRect->width = *screenWidth + (*virtualRatio * 2);
		destRect->height = *screenHeight + (*virtualRatio * 2);
	}
	return updated;
}

void addShaderPath(ShaderManager *sm, char *newPath) {
	if(sm->loadedShaderCount >= MAX_SHADERPATHS) {
		printf("Error: Shader paths array is full.\n");
		return;
	}
	sm->shaderPaths[sm->loadedShaderCount] = strdup(newPath);
	sm->loadedShaderCount++;
}

void initShaderManager(ShaderManager *sm, TextBox *notificationTB, char *folderPath) {
	sm->loadedShaderCount = 0;
	sm->currentShaderIndex = 0;
	sm->loadSuccessful = true;
	initMessageBuffer(&sm->errors);
	sm->errorNotification = notificationTB;

	if(DirectoryExists(folderPath)) {
		FilePathList pl = LoadDirectoryFilesEx(folderPath, getConfigValueString("shaderFileExtension"), false);
		for(int i = 0; i < pl.count; i++) {
			printf("Grabbing file path: %s\n", pl.paths[i]);
			addShaderPath(sm, pl.paths[i]);
		}
	}
	if(sm->loadedShaderCount > 0) {
		sm->current = LoadShader(0, sm->shaderPaths[0]);
		sm->currentModTime = GetFileModTime(sm->shaderPaths[0]);
	} else {
		printf("no shaders at: %s\n", folderPath);
	}
}

void swapOrReloadShader(ShaderManager *sm, int index) {
	if(index < 0 || index >= sm->loadedShaderCount) {
		printf("invalid index.\n");
		return;
	}

	bool loadRequired = false;

	if(index != sm->currentShaderIndex) {
		sm->currentShaderIndex = index;
		loadRequired = true;
	}

	long newModTime = GetFileModTime(sm->shaderPaths[sm->currentShaderIndex]);

	if(sm->currentModTime != newModTime) {
		loadRequired = true;
	}

	if(loadRequired) {
		// open and overwrite file for dumping potential GLSL errors, redirecting stdout to it.
		FILE *err = freopen("glerr", "w", stdout);

		Shader newShader = LoadShader(0, sm->shaderPaths[sm->currentShaderIndex]);
		if(IsShaderValid(newShader)) {
			UnloadShader(sm->current);
			sm->current = newShader;
			sm->currentModTime = newModTime;
			sm->loadSuccessful = true;
		} else {
			printf("shader is invalid, aborting load\n");
			sm->loadSuccessful = false;
		}

		// closing GLSL error file and redirecting stdout to console.
		fclose(err);
		freopen("/dev/tty", "w", stdout);

		char *errFile = LoadFileText("glerr");
		int lineCount = 0;
		const char **errLines = TextSplit(errFile, '\n', &lineCount);

		for(int i = 0; errLines[i]; i++) {
			int shaderIndex = TextFindIndex(errLines[i], "SHADER");
			if(shaderIndex > -1) {
				const char *msg = &errLines[i][shaderIndex];
				printf("!ERRLINE: %s\n", msg);
				pushMessage(&sm->errors, msg);
			} else {
				printf("?ERRLINE: %s\n", errLines[i]);
			}
			newNotification(sm->errorNotification, errFile);
		}
	}
}

void rescanDirectory(ShaderManager *sm, char *folderPath) {
	for(int i = 0; i < sm->loadedShaderCount; i++) {
		free(sm->shaderPaths[i]);
		sm->shaderPaths[i] = NULL;
	}

	sm->loadedShaderCount = 0;
	sm->currentShaderIndex = 0;

	if(DirectoryExists(folderPath)) {
		FilePathList pl = LoadDirectoryFilesEx(folderPath, ".glsl", false);
		for(int i = 0; i < pl.count; i++) {
			addShaderPath(sm, pl.paths[i]);
		}
	}
}

void incShaderIndex(ShaderManager *sm) {
	int newIndex = sm->currentShaderIndex + 1;
	if(newIndex >= sm->loadedShaderCount) {
		return;
	}
	swapOrReloadShader(sm, newIndex);
}

void decShaderIndex(ShaderManager *sm) {
	int newIndex = sm->currentShaderIndex - 1;
	if(newIndex < 0) {
		return;
	}
	swapOrReloadShader(sm, newIndex);
}

void initMessageBuffer(MessageBuffer *mb) {
	mb->count = 0;
	mb->index = 0;
}

void pushMessage(MessageBuffer *mb, const char *msg) {
	if(strlen(msg) < MAX_MESSAGE_LENGTH) {
		mb->index++;

		if(mb->index < MAX_MESSAGE_BUFFER_LINES) {
			mb->count++;
		} else {
			mb->index %= MAX_MESSAGE_BUFFER_LINES;
		}
		mb->messages[mb->index] = strdup(msg);
		printf("Adding: '%s'\n", mb->messages[mb->index]);
	}
}

void drawMessageBuffer(MessageBuffer *mb, Font f, Vector2 pos, int fontSize) {
	for(int i = mb->count - 1; i > 0; i--) {
		int index = (mb->index - i) % MAX_MESSAGE_BUFFER_LINES;
		DrawRectangle(pos.x, pos.y + i * (fontSize + 2), 600, 18, (Color){ 0, 0, 0, 175 });
		DrawTextEx(f, mb->messages[index], (Vector2){ pos.x, pos.y + i * (fontSize + 2) }, fontSize, 2, (Color){ 255, 255, 255, 255 });
	}
}
