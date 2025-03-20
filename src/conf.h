#ifndef CONF_H
#define CONF_H

#include <stdbool.h>

#include "raylib.h"
#include "cJSON.h"

#define MAX_PATH_LENGTH 256
#define CONFIG_PARAMETER_COUNT 12

typedef enum {
	CVT_INT,
	CVT_FLOAT,
	CVT_BOOLEAN,
	CVT_STRING,
} ConfVarType;

typedef struct {
	char *key;
	size_t offset;
	ConfVarType type;
} ConfigMap;

typedef struct {
	int screenW;
	int screenH;
	char *shaderFolder;
	char *screenshotsFolder;
	char *imagesFolder;
	char *backgroundImagePath;
	bool autoReload;
	float reloadCheckInterval;
	bool maintainContentAspectRatio;
	char *systemFontPath;
	char *shaderFileExtension;
	Font systemFont;
	bool copyOnDrag;
	bool initialized;
} Config;

// internal functions

static void setConfigValue(Config *c, ConfigMap cm[], char *key, cJSON *jsonRoot);
static void parseJSONConfig(Config *conf, const char *filePath);
static void freeConfig(Config *conf);
static void initDefaultConf(Config *conf);

// global functions and getters
void initGlobalConf();
bool globalConfIsInitialised();

int getConfigValueInt(char *key);
float getConfigValueFloat(char *key);
char *getConfigValueString(char *key);
bool getConfigValueBool(char *key);

#endif
