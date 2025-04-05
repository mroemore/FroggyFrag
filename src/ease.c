#include "ease.h"
#include <math.h>

static float c4 = (2.0f * M_PI) / 3.0f;
static float c5 = (2.0f * M_PI) / 4.5f;
static float n1 = 7.5625f;
static float d1 = 2.75f;

float linear(float x) {
	return x;
}

float easeOutQuint(float x) {
	return 1.0f - powf(1.0f - x, 5.0f);
}

float easeInQuint(float x) {
	return x * x * x * x * x;
}

float easeInOutQuint(float x) {
	return x < 0.5f ? 16.0f * x * x * x * x * x : 1.0f - powf(-2.0f * x + 2.0f, 5.0f) / 2.0f;
}

float easeOutCubic(float x) {
	return 1.0f - powf(1.0f - x, 3.0f);
}

float easeInCubic(float x) {
	return x * x * x;
}

float easeInOutCubic(float x) {
	return x < 0.5f ? 4.0f * x * x * x : 1.0f - powf(-2.0f * x + 2.0f, 3.0f) / 2.0f;
}

float easeInSine(float x) {
	return 1.0f - cos((x * M_PI) / 2.0f);
}

float easeOutSine(float x) {
	return sin((x * M_PI) / 2.0f);
}

float easeInOutSine(float x) {
	return (cos(M_PI * x) - 1.0f) / 2.0f;
}

float easeInCirc(float x) {
	return 1.0f - sqrt(1.0f - powf(x, 2.0f));
}

float easeOutCirc(float x) {
	return sqrt(1 - powf(x - 1.0f, 2.0f));
}

float easeInOutCirc(float x) {
	return x < 0.5f
	         ? (1 - sqrt(1.0f - powf(2.0f * x, 2.0f))) / 2.0f
	         : (sqrt(1 - powf(-2.0f * x + 2.0f, 2.0f)) + 1.0f) / 2.0f;
}

float easeInElastic(float x) {
	return x == 0.0f
	         ? 0
	       : x == 1.0f
	         ? 1
	         : -powf(2.0f, 10.0 * x - 10.0) * sin((x * 10.0f - 10.75f) * c4);
}

float easeOutElastic(float x) {
	return x == 0.0f
	         ? 0.0f
	       : x == 1.0f
	         ? 1.0f
	         : powf(2.0f, -10.0f * x) * sin((x * 10.0f - 0.75f) * c4) + 1.0f;
}

float easeInOutElastic(float x) {
	return x == 0.0f
	         ? 0.0f
	       : x == 1.0f
	         ? 1.0f
	       : x < 0.5f
	         ? -(powf(2.0f, 20.0f * x - 10.0f) * sin((20.0f * x - 11.125f) * c5)) / 2.0f
	         : (powf(2.0f, -20.0f * x + 10.0f) * sin((20.0f * x - 11.125f) * c5)) / 2.0f + 1.0f;
}

float easeInBounce(float x) {
	return 1 - easeOutBounce(x);
}

float easeOutBounce(float x) {
	if(x < 1.0f / d1) {
		return n1 * x * x;
	} else if(x < 2.0f / d1) {
		return n1 * (x - 1.5f / d1) * x + 0.75f;
	} else if(x < 2.5 / d1) {
		return n1 * (x - 2.25f / d1) * x + 0.9375f;
	} else {
		return n1 * (x - 2.625f / d1) * x + 0.984375f;
	}
}

float easeInOutBounce(float x) {
	return x < 0.5f
	         ? (1.0f - easeOutBounce(1.0f - 2.0f * x)) / 2.0f
	         : (1.0f + easeOutBounce(2.0f * x - 1.0f)) / 2.0f;
}

InterpolationFunction assignEasingTransform(InterpolationType interpolationType) {
	InterpolationFunction interpFunc = linear;
	switch(interpolationType) {
		case IT_QUINT_INOUT:
			interpFunc = easeInOutQuint;
			break;
		case IT_QUINT_IN:
			interpFunc = easeInQuint;
			break;
		case IT_QUINT_OUT:
			interpFunc = easeOutQuint;
			break;
		case IT_CUBIC_IN:
			interpFunc = easeInCubic;
			break;
		case IT_CUBIC_OUT:
			interpFunc = easeOutCubic;
			break;
		case IT_CUBIC_INOUT:
			interpFunc = easeInOutCubic;
			break;
		case IT_CIRC_IN:
			interpFunc = easeInCirc;
			break;
		case IT_CIRC_OUT:
			interpFunc = easeOutCirc;
			break;
		case IT_CIRC_INOUT:
			interpFunc = easeInOutCirc;
			break;
		case IT_SINE_IN:
			interpFunc = easeInSine;
			break;
		case IT_SINE_OUT:
			interpFunc = easeOutSine;
			break;
		case IT_SINE_INOUT:
			interpFunc = easeInOutSine;
			break;
		case IT_ELASTIC_IN:
			interpFunc = easeInElastic;
			break;
		case IT_ELASTIC_OUT:
			interpFunc = easeOutElastic;
			break;
		case IT_ELASTIC_INOUT:
			interpFunc = easeInOutElastic;
			break;
		case IT_BOUNCE_IN:
			interpFunc = easeInBounce;
			break;
		case IT_BOUNCE_OUT:
			interpFunc = easeOutBounce;
			break;
		case IT_BOUNCE_INOUT:
			interpFunc = easeInOutBounce;
			break;
		case IT_LINEAR:
		default:
			break;
	}

	return interpFunc;
}
