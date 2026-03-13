#include "conf.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#define SYSTEM_DATA_DIR "/usr/share/froggy-frag"
#define APP_NAME "froggy-frag"

static Config gc;
static char xdgConfigDir[MAX_PATH_LENGTH] = {0};
static char xdgDataDir[MAX_PATH_LENGTH] = {0};
static char systemDataDir[MAX_PATH_LENGTH] = SYSTEM_DATA_DIR;

static ConfigMap configMap[CONFIG_PARAMETER_COUNT] = {
	{ "screenW", offsetof(Config, screenW), CVT_INT },
	{ "screenH", offsetof(Config, screenH), CVT_INT },
	{ "shaderFolder", offsetof(Config, shaderFolder), CVT_STRING },
	{ "screenshotsFolder", offsetof(Config, screenshotsFolder), CVT_STRING },
	{ "imagesFolder", offsetof(Config, imagesFolder), CVT_STRING },
	{ "backgroundImagePath", offsetof(Config, backgroundImagePath), CVT_STRING },
	{ "autoReload", offsetof(Config, autoReload), CVT_BOOLEAN },
	{ "reloadCheckInterval", offsetof(Config, reloadCheckInterval), CVT_FLOAT },
	{ "maintainContentAspectRatio", offsetof(Config, maintainContentAspectRatio), CVT_BOOLEAN },
	{ "systemFontPath", offsetof(Config, systemFontPath), CVT_STRING },
	{ "copyOnDrag", offsetof(Config, copyOnDrag), CVT_BOOLEAN },
	{ "shaderFileExtension", offsetof(Config, shaderFileExtension), CVT_STRING }

};

void setConfigValue(Config *c, ConfigMap cm[], char *key, cJSON *jsonRoot) {
	bool keyFound = false;
	cJSON *value = cJSON_GetObjectItemCaseSensitive(jsonRoot, key);
	if(value != NULL) {
		for(int i = 0; i < CONFIG_PARAMETER_COUNT; i++) {
			if(strcmp(cm[i].key, key) == 0) {
				void *param = (void *)((char *)c + cm[i].offset);
				switch(cm[i].type) {
					case CVT_INT:
						*(int *)param = (int)cJSON_GetNumberValue(value);
						printf("\n\tKey: %s Value: %i\n", key, *(int *)param);
						break;
					case CVT_FLOAT:
						*(float *)param = (float)cJSON_GetNumberValue(value);
						printf("\n\tKey: %s Value: %f\n", key, *(float *)param);
						break;
					case CVT_BOOLEAN:
						*(bool *)param = (bool)cJSON_IsTrue(value);
						printf("\n\tKey: %s Value: %i\n", key, *(bool *)param);
						break;
					case CVT_STRING:
						*(char **)param = malloc(MAX_PATH_LENGTH);
						strncpy(*(char **)param, cJSON_GetStringValue(value), MAX_PATH_LENGTH);
						printf("\n\tKey: %s Value: %s\n", key, *(char **)param);
						break;
					default:
						fprintf(stderr, "ERROR: conf mapping type invalid:\n");
						break;
				}
				keyFound = true;
				break;
			}
		}
		if(!keyFound) {
			printf("Could not find match for %s.\n", key);
		}
	} else {
		fprintf(stderr, "ERROR: Config value %s not found in JSON.\n", key);
	}

	if(!keyFound) {
		printf("key not found in cofig map.\n");
	}
}

void parseJSONConfig(Config *conf, const char *filePath) {
	char *fileData = LoadFileText(filePath);
	if(fileData) {
		cJSON *json = cJSON_Parse(fileData);
		if(json) {
			printf("\n\nBEGIN CONFIG PARSING\n\n");
			setConfigValue(conf, configMap, "screenW", json);
			setConfigValue(conf, configMap, "screenH", json);
			setConfigValue(conf, configMap, "systemFontPath", json);
			setConfigValue(conf, configMap, "shaderFolder", json);
			setConfigValue(conf, configMap, "backgroundImagePath", json);
			setConfigValue(conf, configMap, "screenshotsFolder", json);
			setConfigValue(conf, configMap, "imagesFolder", json);
			setConfigValue(conf, configMap, "reloadCheckInterval", json);
			setConfigValue(conf, configMap, "autoReload", json);
			setConfigValue(conf, configMap, "maintainContentAspectRatio", json);
			setConfigValue(conf, configMap, "shaderFileExtension", json);
			setConfigValue(conf, configMap, "copyOnDrag", json);
			conf->initialized = true;
			printf("\n\nEND CONFIG PARSING\n\n");
			cJSON_Delete(json);
		} else {
			fprintf(stderr, "ERROR: json not parsed\n");
		}
		UnloadFileText(fileData);
	} else {
		fprintf(stderr, "ERROR: file not opened\n");
	}
}

static void initXDGPaths(void) {
	if(xdgConfigDir[0] != '\0') return;

	const char *configHome = getenv("XDG_CONFIG_HOME");
	const char *dataHome = getenv("XDG_DATA_HOME");
	const char *home = getenv("HOME");

	if(configHome && configHome[0] != '\0') {
		snprintf(xdgConfigDir, MAX_PATH_LENGTH, "%s/%s", configHome, APP_NAME);
	} else if(home) {
		snprintf(xdgConfigDir, MAX_PATH_LENGTH, "%s/.config/%s", home, APP_NAME);
	}

	if(dataHome && dataHome[0] != '\0') {
		snprintf(xdgDataDir, MAX_PATH_LENGTH, "%s/%s", dataHome, APP_NAME);
	} else if(home) {
		snprintf(xdgDataDir, MAX_PATH_LENGTH, "%s/.local/share/%s", home, APP_NAME);
	}
}

static void ensureDirExists(const char *path) {
	struct stat st = {0};
	if(stat(path, &st) == -1) {
		if(mkdir(path, 0755) == 0) {
			printf("Created directory: %s\n", path);
		} else {
			fprintf(stderr, "Warning: Could not create directory %s: %s\n", path, strerror(errno));
		}
	}
}

static bool fileExists(const char *path) {
	struct stat st;
	return stat(path, &st) == 0;
}

static void resolvePath(char *dest, const char *userSubdir, const char *systemSubdir, const char *fallback) {
	initXDGPaths();

	char userPath[MAX_PATH_LENGTH];
	char systemPath[MAX_PATH_LENGTH];

	if(fallback && (fileExists(fallback) || DirectoryExists(fallback))) {
		strncpy(dest, fallback, MAX_PATH_LENGTH);
		printf("Path resolved (fallback exists): %s\n", dest);
		return;
	}

	if(xdgDataDir[0] != '\0' && userSubdir) {
		snprintf(userPath, MAX_PATH_LENGTH, "%s/%s", xdgDataDir, userSubdir);
		if(fileExists(userPath) || DirectoryExists(userPath)) {
			strncpy(dest, userPath, MAX_PATH_LENGTH);
			printf("Path resolved (user): %s\n", dest);
			return;
		}
	}

	if(systemSubdir) {
		snprintf(systemPath, MAX_PATH_LENGTH, "%s/%s", systemDataDir, systemSubdir);
		if(fileExists(systemPath) || DirectoryExists(systemPath)) {
			strncpy(dest, systemPath, MAX_PATH_LENGTH);
			printf("Path resolved (system): %s\n", dest);
			return;
		}
	}

	strncpy(dest, fallback, MAX_PATH_LENGTH);
	printf("Path resolved (fallback): %s\n", dest);
}

static void initDefaultConf(Config *conf) {
	conf->screenW = 1280;
	conf->screenH = 960;
	conf->shaderFileExtension = ".glsl";
	conf->autoReload = true;
	conf->reloadCheckInterval = 1.5;
	conf->maintainContentAspectRatio = false;
	conf->initialized = true;
	conf->copyOnDrag = false;

	conf->shaderFolder = malloc(MAX_PATH_LENGTH);
	conf->screenshotsFolder = malloc(MAX_PATH_LENGTH);
	conf->imagesFolder = malloc(MAX_PATH_LENGTH);
	conf->backgroundImagePath = malloc(MAX_PATH_LENGTH);
	conf->systemFontPath = malloc(MAX_PATH_LENGTH);

	resolvePath(conf->shaderFolder, "shaders", "shaders", "resources/shaders");
	resolvePath(conf->imagesFolder, "images", "images", "resources/images");
	resolvePath(conf->backgroundImagePath, "images/train.png", "images/train.png", "resources/images/train.png");
	resolvePath(conf->systemFontPath, NULL, "fonts/Targa.ttf", "resources/fonts/Targa.ttf");

	initXDGPaths();
	if(xdgDataDir[0] != '\0') {
		ensureDirExists(xdgDataDir);
		snprintf(conf->screenshotsFolder, MAX_PATH_LENGTH, "%s/screenshots", xdgDataDir);
		ensureDirExists(conf->screenshotsFolder);
	} else {
		strncpy(conf->screenshotsFolder, "resources/screenshots", MAX_PATH_LENGTH);
	}
}

void freeConfig(Config *conf) {
	if(conf->shaderFolder) free(conf->shaderFolder);
	if(conf->screenshotsFolder) free(conf->screenshotsFolder);
	if(conf->imagesFolder) free(conf->imagesFolder);
	if(conf->backgroundImagePath) free(conf->backgroundImagePath);
	if(conf->systemFontPath) free(conf->systemFontPath);
	conf->initialized = false;
}

void initGlobalConf() {
	initXDGPaths();
	initDefaultConf(&gc);

	char configPath[MAX_PATH_LENGTH];
	bool configFound = false;

	if(xdgConfigDir[0] != '\0') {
		snprintf(configPath, MAX_PATH_LENGTH, "%s/config.json", xdgConfigDir);
		if(fileExists(configPath)) {
			parseJSONConfig(&gc, configPath);
			configFound = true;
		}
	}

	if(!configFound && fileExists("conf.json")) {
		parseJSONConfig(&gc, "conf.json");
		configFound = true;
	}

	if(!configFound && xdgConfigDir[0] != '\0') {
		ensureDirExists(xdgConfigDir);
		printf("No config found. Using defaults. Config location: %s/config.json\n", xdgConfigDir);
	}
}

bool globalConfIsInitialised() {
	return gc.initialized;
}

int getConfigValueInt(char *key) {
	int result = -1;
	for(int i = 0; i < CONFIG_PARAMETER_COUNT; i++) {
		if(strcmp(configMap[i].key, key) == 0) {
			if(configMap[i].type == CVT_INT) {
				void *param = (void *)((char *)&gc + configMap[i].offset);
				result = *(int *)param;
			}
			break;
		}
	}
	return result;
}
float getConfigValueFloat(char *key) {
	float result = -1.0f;
	for(int i = 0; i < CONFIG_PARAMETER_COUNT; i++) {
		if(strcmp(configMap[i].key, key) == 0) {
			if(configMap[i].type == CVT_FLOAT) {
				void *param = (void *)((char *)&gc + configMap[i].offset);
				result = *(float *)param;
			}
			break;
		}
	}
	return result;
}
char *getConfigValueString(char *key) {
	char *result = malloc(MAX_PATH_LENGTH);
	for(int i = 0; i < CONFIG_PARAMETER_COUNT; i++) {
		if(strcmp(configMap[i].key, key) == 0) {
			if(configMap[i].type == CVT_STRING) {
				void *param = (void *)((char *)&gc + configMap[i].offset);
				strncpy(result, *(char **)param, MAX_PATH_LENGTH);
			}
			break;
		}
	}
	return result;
}

bool getConfigValueBool(char *key) {
	bool result = false;
	for(int i = 0; i < CONFIG_PARAMETER_COUNT; i++) {
		if(strcmp(configMap[i].key, key) == 0) {
			if(configMap[i].type == CVT_BOOLEAN) {
				void *param = (void *)((char *)&gc + configMap[i].offset);
				result = *(bool *)param;
			}
			break;
		}
	}
	return result;
}
