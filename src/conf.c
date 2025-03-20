#include "conf.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static Config gc;

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

static void initDefaultConf(Config *conf) {
	conf->screenW = 1280;
	conf->screenH = 960;
	conf->shaderFolder = "resources/shaders";
	conf->shaderFileExtension = ".glsl";
	conf->backgroundImagePath = "resources/train.png";
	conf->autoReload = true;
	conf->reloadCheckInterval = 1.5;
	conf->maintainContentAspectRatio = false;
	conf->systemFontPath = "resources/fonts/04B_03__.TTF";
	conf->initialized = true;
	conf->copyOnDrag = false;
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
	initDefaultConf(&gc);
	parseJSONConfig(&gc, "conf.json");
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
	bool result = NULL;
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
