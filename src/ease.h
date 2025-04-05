#ifndef EASE_H
#define EASE_H

typedef enum {
	IT_LINEAR,
	IT_QUINT_IN,
	IT_QUINT_OUT,
	IT_QUINT_INOUT,
	IT_CUBIC_IN,
	IT_CUBIC_OUT,
	IT_CUBIC_INOUT,
	IT_SINE_IN,
	IT_SINE_OUT,
	IT_SINE_INOUT,
	IT_ELASTIC_IN,
	IT_ELASTIC_OUT,
	IT_ELASTIC_INOUT,
	IT_BOUNCE_IN,
	IT_BOUNCE_OUT,
	IT_BOUNCE_INOUT,
	IT_CIRC_IN,
	IT_CIRC_OUT,
	IT_CIRC_INOUT,
	IT_COUNT
} InterpolationType;

typedef float (*InterpolationFunction)(float);

float linear(float x);
float easeOutQuint(float x);
float easeInQuint(float x);
float easeInOutQuint(float x);
float easeOutCubic(float x);
float easeInCubic(float x);
float easeInOutCubic(float x);
float easeInSine(float x);
float easeOutSine(float x);
float easeInOutSine(float x);
float easeInCirc(float x);
float easeOutCirc(float x);
float easeInOutCirc(float x);
float easeInElastic(float x);
float easeOutElastic(float x);
float easeInOutElastic(float x);
float easeInBounce(float x);
float easeOutBounce(float x);
float easeInOutBounce(float x);

InterpolationFunction assignEasingTransform(InterpolationType interpolationType);

#endif
