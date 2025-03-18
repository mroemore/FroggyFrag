#ifndef TIMER_H
#define TIMER_H

#include <complex.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX_CONCURRENT_TIMERS 128

typedef void (*TimerCallback)(void *data);

typedef struct {
	float duration;
	float elapsed;
	bool running;
	bool enabled;
	TimerCallback tc;
	void *tcData;
} Timer;

typedef struct {
	Timer timers[MAX_CONCURRENT_TIMERS];
	Timer *freeTimers[MAX_CONCURRENT_TIMERS];
	uint16_t timerCount;
	uint16_t freeTimerCount;
} TimerList;

void initTimerList();
Timer *createTimer(float duration, TimerCallback tc, void *data);
void startTimer(Timer *t);
void startTimerCallback(void *data);
void tickTimers(float interval);

void printTimerListStats(char *eventText);

#endif
