#include "timer.h"
#include "raylib.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

TimerList timerList;

uint32_t frameCounter = 0;

uint32_t getFrameCount() {
	return frameCounter;
}

void incrementFrameCount() {
	frameCounter++;
}

void initTimerList() {
	timerList.timerCount = 0;
	timerList.freeTimerCount = 0;
	for(int i = 0; i < MAX_CONCURRENT_TIMERS; i++) {
		timerList.freeTimers[i] = NULL;
	}
}

Timer *createTimer(TimerType type, int durationFrames, TimerCallback tc, void *data) {
	Timer t;
	t.type = type;
	t.elapsed = 0.0;
	t.durationFrames = durationFrames;
	t.tc = tc;
	t.running = false;
	t.enabled = true;
	t.tcData = data;

	Timer *tPtr = NULL;
	if(timerList.freeTimerCount > 0) {
		*timerList.freeTimers[timerList.freeTimerCount - 1] = t;
		tPtr = timerList.freeTimers[timerList.freeTimerCount - 1];
		timerList.freeTimerCount--;
	} else {
		if(timerList.timerCount < MAX_CONCURRENT_TIMERS) {
			timerList.timers[timerList.timerCount] = t;
			tPtr = &timerList.timers[timerList.timerCount];
			timerList.timerCount++;
		} else {
			fprintf(stderr, "ERROR: Too many timers.\n");
		}
	}

	return tPtr;
}

void startTimer(Timer *t) {
	if(t->enabled) {
		t->running = true;
		t->startFrameCount = getFrameCount();
		printf("timer start time: %i\n", t->startFrameCount);
	} else {
		fprintf(stderr, "ERROR: typing to start disabled timer.");
	}
}

void startTimerCallback(void *data) {
	Timer *t = (Timer *)data;
	startTimer(t);
}

void removeTimer(Timer *t) {
	bool found = false;
	for(int i = 0; i < timerList.timerCount; i++) {
		if(&timerList.timers[i] == t) {
			found = true;

			timerList.timers[i].running = false;
			timerList.timers[i].enabled = false;

			timerList.freeTimers[timerList.freeTimerCount] = &timerList.timers[i];
			timerList.freeTimerCount++;
			break;
		}
	}
	if(!found) {
		fprintf(stderr, "Error: Count not remove timer, not in list.\n");
	}
}

void printTimerListStats(char *eventText) {
	printf("[ %s ]: timercount:  %i, timerFreeCount: %i\n", eventText, timerList.timerCount, timerList.freeTimerCount);
}

void tickTimers(int frameCount) {
	for(int i = 0; i < timerList.timerCount; i++) {
		if(timerList.timers[i].enabled) {
			if(timerList.timers[i].running) {
				int elapsed = frameCount - timerList.timers[i].startFrameCount;
				// timerList.timers[i].elapsed += interval;
				if(elapsed > timerList.timers[i].durationFrames) {
					if(timerList.timers[i].tc != NULL) {
						timerList.timers[i].tc(timerList.timers[i].tcData);
					}
					if(timerList.timers[i].type == TT_ONCE) {
						removeTimer(&timerList.timers[i]);
					} else if(timerList.timers[i].type == TT_LOOPING) {
						timerList.timers[i].startFrameCount = frameCount;
					}
					if(timerList.timers[i].type == TT_REPEATING) {
						printf("timer end. now: %i, currentTime: %i, startTime: %i\n", frameCount, elapsed, timerList.timers[i].startFrameCount);
						timerList.timers[i].running = false;
					}
				}
			}
		}
	}
}
