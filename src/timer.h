#ifndef TIMER_H
#define TIMER_H

#include <complex.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX_CONCURRENT_TIMERS 128

typedef void (*TimerCallback)(void *data);

typedef enum {
	TT_ONCE,
	TT_REPEATING,
	TT_LOOPING
} TimerType;

typedef struct {
	TimerType type;
	int startFrameCount;
	int durationFrames;
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
uint32_t getFrameCount();
void incrementFrameCount();
Timer *createTimer(TimerType type, int durationFrames, TimerCallback tc, void *data);
void startTimer(Timer *t);
void startTimerCallback(void *data);
void tickTimers(int frameCount);

void printTimerListStats(char *eventText);

#endif
