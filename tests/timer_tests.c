#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/timer.h"
#include "unity.h"

typedef struct {
	int ia;
	float fb;
	bool bc;
	char *name;
} DemoData;

DemoData *create_dd(int a, float b, bool c, const char *name);
void dd_cb(void *data);

DemoData *create_dd(int a, float b, bool c, const char *name) {
	DemoData *dd = (DemoData *)malloc(sizeof(DemoData));
	dd->ia = a;
	dd->fb = b;
	dd->bc = c;
	strcpy(dd->name, name);
	return dd;
}

void dd_cb(void *data) {
	DemoData *dd = (DemoData *)data;
	printf("dd callback: %i, %f, %b, %s", dd->ia, dd->fb, dd->bc, dd->name);
}

void setUp() {
	initTimerList();
}

void tearDown() {
}

void test_createTimer() {
	DemoData *dd = create_dd(4, 2.2, false, "demo1");
	Timer *t = createTimer(TT_ONCE, 1.0, dd_cb, dd);
}

void testTimerSys_1() {
	initTimerList();
	DemoData *d1 = create_dd(5, 2.7f, true, "d1");
	DemoData *d2 = create_dd(6, 11.7f, false, "d2");
	Timer *t1 = createTimer(TT_ONCE, 2.0f, dd_cb, d1);
	Timer *t2 = createTimer(TT_ONCE, .5f, dd_cb, d2);
	Timer *t3 = createTimer(TT_ONCE, 4.5f, startTimerCallback, t2);
	startTimer(t1);
	startTimer(t3);

	float maxRunTime = 10.0f;
	float currentTime = 0.0f;
	float inc = 0.013;
	int iterCount = 0;
	while(currentTime < maxRunTime) {
		tickTimers(inc);
		currentTime += inc;
		iterCount++;
	}
	printf("Test complete, %i iterations.\n", iterCount);
	printTimerListStats("COMPLETE");
}
