#include "timer.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

TimerList timerList;

void initTimerList() {
	timerList.timerCount = 0;
	timerList.freeTimerCount = 0;
	for(int i = 0; i < MAX_CONCURRENT_TIMERS; i++) {
		timerList.freeTimers[i] = NULL;
	}
	printf("Timer struct size: %lu bytes\n", sizeof(Timer));
	printf("Free ptr size: %lu bytes\n", sizeof(timerList.timers[0]));
	printf("Free ptr ptr size: %lu bytes\n", sizeof(timerList.freeTimers[0]));
}

Timer *createTimer(float duration, TimerCallback tc, void *data) {
	Timer t;
	t.elapsed = 0.0;
	t.duration = duration;
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
			tPtr = timerList.freeTimers[timerList.timerCount];
			timerList.timerCount++;
		} else {
			fprintf(stderr, "ERROR: Too many timers.\n");
		}
	}
	printTimerListStats("ADD");

	return tPtr;
}

void startTimer(Timer *t) {
	if(t->enabled) {
		t->running = true;
	} else {
		fprintf(stderr, "ERROR: typing to start disabled timer.");
	}
	printTimerListStats("START");
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

	printTimerListStats("REMOVE");
}

void printTimerListStats(char *eventText) {
	printf("[ %s ]: timercount:  %i, timerFreeCount: %i\n", eventText, timerList.timerCount, timerList.freeTimerCount);
}

void tickTimers(float interval) {
	for(int i = 0; i < timerList.timerCount; i++) {
		if(timerList.timers[i].enabled) {
			if(timerList.timers[i].running) {
				timerList.timers[i].elapsed += interval;
				if(timerList.timers[i].elapsed > timerList.timers[i].duration) {
					timerList.timers[i].tc(timerList.timers[i].tcData);
					removeTimer(&timerList.timers[i]);
				}
			}
		}
	}
}
