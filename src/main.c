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
#include "animation.h"
#include "gui.h"

#define MAX_SHADERPATHS 256
#define MAX_IMAGEPATHS 256

typedef struct {
	Shader current;
	time_t currentModTime;
	char *shaderFolder;
	char *shaderPaths[MAX_SHADERPATHS];
	uint16_t loadedShaderCount;
	uint16_t currentShaderIndex;
	bool loadSuccessful;
	MessageBuffer *errors;
	TextBox *notification;
} ShaderManager;

// window resize functions
bool updateScreenDimensions(Rectangle *sourceRect, Rectangle *destRect, int *screenWidth, int *screenHeight, float *virtualRatio);

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

// utility & QOL functions
void screenshot(ShaderManager *sm);

int main(void) {
	// Change to executable's directory so resources load correctly regardless of where app is run from
	const char *appDir = GetApplicationDirectory();
	if(appDir) {
		ChangeDirectory(appDir);
	}

	initGlobalConf();

	int screenWidth = getConfigValueInt("screenW");
	int screenHeight = getConfigValueInt("screenH");
	initRootDrawable(screenWidth, screenHeight);
	initAnimationManager();
	const int virtualScreenWidth = 320;
	const int virtualScreenHeight = 240;
	float virtualRatio = (float)screenWidth / (float)virtualScreenWidth;

	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
	InitWindow(screenWidth, screenHeight, "Shader Preview Tool");

	char *fontPath = getConfigValueString("systemFontPath");
	Font fontSystem = LoadFont(fontPath);
	free(fontPath);
	SetTextLineSpacing(8);

	char *bgPath = getConfigValueString("backgroundImagePath");
	Image bg = LoadImage(bgPath);
	free(bgPath);
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
	TextBox *notificationTB = createSettingsNotificationBox(10, -50, 400, 50, "default", (Color){ 36, 36, 35, 200 }, (Color){ 245, 203, 92, 200 });
	addDrawableToRoot((Drawable *)notificationTB);
	char *shaderFolder = getConfigValueString("shaderFolder");
	initShaderManager(&sm, notificationTB, shaderFolder);
	free(shaderFolder);

	// If no shaders loaded, use default shader to prevent crash
	if(sm.loadedShaderCount == 0) {
		sm.current = LoadShader(0, 0); // Default raylib shader
		sm.loadSuccessful = true;
	}

	float windowResolution[2] = { (float)screenWidth, (float)screenHeight };
	float mousePosition[2] = { (float)screenWidth / 2, (float)screenHeight / 2 };
	// TextBox *ptb = createPopupTextBox(100, 100, 100, 100, "Hello,\n there.", (Color){ 0, 0, 0, 170 }, (Color){ 200, 200, 200, 200 });

	float reloadTimer = 0.0;
	bool drawConsole = false;
	float frameTime = 0.0;
	float reloadCheckInterval = getConfigValueFloat("reloadCheckInterval");
	bool autoReload = getConfigValueBool("autoReload");
	SetTargetFPS(60);
	double now = GetTime();
	while(!WindowShouldClose()) // Detect window close button or ESC key
	{
		float time = GetTime();
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

		mousePosition[0] = GetMouseX();
		mousePosition[1] = windowResolution[1] - GetMouseY();

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
			toggleMessageBufferVisibility(sm.errors);
		}
		if(IsKeyReleased(KEY_A)) {
			autoReload = !autoReload;
			autoReload ? newSettingsInfo(sm.notification, "auto reload enabled.") : newSettingsInfo(sm.notification, "auto reload disabled.");
		}
		if(IsKeyReleased(KEY_F5)) {
			rescanDirectory(&sm, getConfigValueString("shaderFolder"));
		}
		if(IsKeyReleased(KEY_S)) {
			screenshot(&sm);
		}

		BeginTextureMode(target);
		BeginMode2D(worldSpaceCamera);
		DrawTexture(bgTex, 0, 0, GRAY);
		EndMode2D();
		EndTextureMode();

		BeginDrawing();
		ClearBackground(WHITE);
		if(sm.loadSuccessful && IsShaderValid(sm.current)) {
			BeginShaderMode(sm.current);
			SetShaderValue(sm.current, GetShaderLocation(sm.current, "iTime"), &time, SHADER_UNIFORM_FLOAT);
			SetShaderValue(sm.current, GetShaderLocation(sm.current, "mousePosition"), &mousePosition, SHADER_UNIFORM_VEC2);

			SetShaderValue(sm.current, GetShaderLocation(sm.current, "windowResolution"), &windowResolution, SHADER_UNIFORM_VEC2);
			DrawTexturePro(target.texture, sourceRec, destRec, origin, 0.0f, WHITE);
			EndShaderMode();
		} else {
			// Draw without shader if shader is invalid
			DrawTexturePro(target.texture, sourceRec, destRec, origin, 0.0f, WHITE);
		}

		if(drawConsole) {
		}
		updateDrawables();
		tickManagedAnimations();
		drawAll();
		DrawFPS(GetScreenWidth() - 95, 10);
		EndDrawing();
	}
}

bool updateScreenDimensions(Rectangle *sourceRect, Rectangle *destRect, int *screenWidth, int *screenHeight, float *virtualRatio) {
	int tmpW = GetScreenWidth();
	int tmpH = GetScreenHeight();
	bool updated = false;
	if(*screenWidth != tmpW || *screenHeight != tmpH) {
		updated = true;
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

void screenshot(ShaderManager *sm) {
	if(sm->loadedShaderCount <= 0) {
		return;
	}
	char *originalWorkingDirectory = strdup(GetWorkingDirectory());
	if(originalWorkingDirectory == NULL) {
		fprintf(stderr, "ERROR: Failed to allocate memory for working directory\n");
		return;
	}

	char *screenshotsFolder = getConfigValueString("screenshotsFolder");
	bool chDirSuccess = ChangeDirectory(screenshotsFolder);
	free(screenshotsFolder);

	if(chDirSuccess) {
		int count = 0;
		const char **currentShaderName = TextSplit(sm->shaderPaths[sm->currentShaderIndex], '/', &count);

		int screenshotIndex = 0;
		char screenshotFileName[255];
		snprintf(screenshotFileName, 255, "%s_%i.png", currentShaderName[count - 1], screenshotIndex);

		while(FileExists(screenshotFileName)) {
			screenshotIndex++;
			snprintf(screenshotFileName, 255, "%s_%i.png", currentShaderName[count - 1], screenshotIndex);
		}
		TakeScreenshot(screenshotFileName);
		char settingsMessage[255];
		snprintf(settingsMessage, 255, "Screenshot \"%s\" saved!", screenshotFileName);
		newSettingsInfo(sm->notification, settingsMessage);

		ChangeDirectory(originalWorkingDirectory);
	}
	free(originalWorkingDirectory);
}

void addShaderPath(ShaderManager *sm, char *newPath) {
	if(sm->loadedShaderCount >= MAX_SHADERPATHS) {
		printf("Error: Shader paths array is full.\n");
		return;
	}
	char *pathCopy = strdup(newPath);
	if(pathCopy == NULL) {
		fprintf(stderr, "ERROR: Failed to allocate memory for shader path\n");
		return;
	}
	sm->shaderPaths[sm->loadedShaderCount] = pathCopy;
	sm->loadedShaderCount++;
}

void initShaderManager(ShaderManager *sm, TextBox *notificationTB, char *folderPath) {
	sm->loadedShaderCount = 0;
	sm->currentShaderIndex = 0;
	sm->loadSuccessful = true;
	char *fontPath = getConfigValueString("systemFontPath");
	sm->errors = createMessageBuffer(NULL, 10, 10, 800, 800, BLACK, WHITE, fontPath, 14);
	free(fontPath);
	addDrawableToRoot((Drawable *)sm->errors);
	sm->notification = notificationTB;

	if(DirectoryExists(folderPath)) {
		char *shaderExt = getConfigValueString("shaderFileExtension");
		FilePathList pl = LoadDirectoryFilesEx(folderPath, shaderExt, false);
		free(shaderExt);
		for(int i = 0; i < pl.count; i++) {
			printf("Grabbing file path: %s\n", pl.paths[i]);
			addShaderPath(sm, pl.paths[i]);
		}
		UnloadDirectoryFiles(pl);
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
			char settingsMessage[255];
			snprintf(settingsMessage, 255, "Shader \"%s\" loaded.", sm->shaderPaths[sm->currentShaderIndex]);
			newSettingsInfo(sm->notification, settingsMessage);

			sm->current = newShader;
			sm->currentModTime = newModTime;
			sm->loadSuccessful = true;
		} else {
			printf("shader is invalid, aborting load\n");
			sm->loadSuccessful = false;
		}

		// closing GLSL error file and redirecting stdout to console.
		fflush(stdout);
		fclose(err);
		FILE *tty = NULL;
#if defined(__linux__) || defined(__APPLE__)
		tty = freopen("/dev/tty", "w", stdout);
#elif defined(_WIN32)
		tty = freopen("CONOUT$", "w", stdout);
#endif
		if(!tty) {
			// If we can't reopen tty, stderr still works
			fprintf(stderr, "WARNING: Could not reopen stdout, using stderr\n");
		}

		char *errFile = LoadFileText("glerr");
		if(errFile) {
			int lineCount = 0;
			const char **errLines = TextSplit(errFile, '\n', &lineCount);

			for(int i = 0; i < lineCount && errLines[i]; i++) {
				if(strlen(errLines[i]) == 0) continue;
				int shaderIndex = TextFindIndex(errLines[i], "SHADER");
				if(shaderIndex > -1) {
					const char *msg = &errLines[i][shaderIndex];
					fprintf(tty ? stdout : stderr, "!ERRLINE: %s\n", msg);
					pushMessage(sm->errors, msg);
				} else {
					fprintf(tty ? stdout : stderr, "?ERRLINE: %s\n", errLines[i]);
				}
			}
			UnloadFileText(errFile);
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
		char *extension = getConfigValueString("shaderFileExtension");
		FilePathList pl = LoadDirectoryFilesEx(folderPath, extension, false);
		free(extension);
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

void pushMessage(MessageBuffer *mb, const char *msg) {
	if(strlen(msg) < MAX_MESSAGE_LENGTH) {
		mb->index++;

		if(mb->index < MAX_MESSAGE_BUFFER_LINES) {
			mb->count++;
		} else {
			mb->index %= MAX_MESSAGE_BUFFER_LINES;
		}
		// Free existing message before overwriting
		if(mb->messages[mb->index] != NULL) {
			free(mb->messages[mb->index]);
		}
		mb->messages[mb->index] = strdup(msg);
		printf("Adding: '%s'\n", mb->messages[mb->index]);
	}
}
