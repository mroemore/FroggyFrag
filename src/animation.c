#include "animation.h"

#include "callback.h"
#include "raylib.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

// #include <ctype.h>

Animation *animationList[MAX_ANIMATIONS_PER_DRAWABLE * MAX_DRAWABLES];
int animationListCount = 0;

void addAnimation(Animation *a) {
	if(animationListCount < MAX_ANIMATIONS_PER_DRAWABLE * MAX_DRAWABLES) {
		animationList[animationListCount] = a;
		animationListCount++;
	}
}

AnimationGroup *createAnimationGroup() {
	AnimationGroup *ag = (AnimationGroup *)malloc(sizeof(AnimationGroup));
	ag->animationCount = 0;
	ag->timerCount = 0;
	return ag;
}

void tickAnimations(float timeDelta) {
	for(int i = 0; i < animationListCount; i++) {
		if(*animationList[i]->trigger == animationList[i]->match && animationList[i]->running) {
			animationList[i]->animate(animationList[i], timeDelta);
		}
	}
}

AnimationManager *createAnimationManager() {
	AnimationManager *am = (AnimationManager *)malloc(sizeof(AnimationManager));
	am->count = 0;
	return am;
}

void resetAnimation(Animation *a) {
	a->running = false;
	a->currentTime = 0.0;
}

Color vec4ToColor(Vector4 in) {
	return (Color){ (int)(in.x * 255.0), (int)(in.y * 255.0), (int)(in.z * 255.0), (int)(in.w * 255.0) };
}

Color vec4ColorInterpolate(Vector4 a, Vector4 b, float t) {
	Vector4 result = {
		result.x = a.x + (b.x - a.x) * t,
		result.y = a.y + (b.y - a.y) * t,
		result.z = a.z + (b.z - a.z) * t,
		result.w = a.w + (b.w - a.w) * t
	};
	return vec4ToColor(result);
}

void colourFadeOutAnimation(void *self, float timeDelta) {
	ColAnimation *ca = (ColAnimation *)self;
	if(!ca->base.running) return;

	ca->base.currentTime += timeDelta;
	float t = ca->base.currentTime / ca->base.duration;
	if(t < 0.0f) t = 0.0f;
	if(t > 1.0f) t = 1.0f;

	*ca->toAnimate = vec4ColorInterpolate(ca->originalNormalised, ca->targetNormalised, t);

	// Check if the animation has finished
	if(ca->base.currentTime >= ca->base.duration) {
		ca->base.running = false;
		*ca->toAnimate = vec4ToColor(ca->targetNormalised); // Ensure the final value is exact
		if(ca->base.onFinish->f != NULL) {
			applyCallback(ca->base.onFinish);
		}
	}
}

void colourFadeInAnimation(void *self, float timeDelta) {
	ColAnimation *ca = (ColAnimation *)self;
	if(!ca->base.running) return;

	ca->base.currentTime += timeDelta;
	float t = ca->base.currentTime / ca->base.duration;
	if(t < 0.0f) t = 0.0f;
	if(t > 1.0f) t = 1.0f;

	*ca->toAnimate = vec4ColorInterpolate(ca->targetNormalised, ca->originalNormalised, t);

	// Check if the animation has finished
	if(ca->base.currentTime >= ca->base.duration) {
		ca->base.running = false;
		*ca->toAnimate = vec4ToColor(ca->originalNormalised); // Ensure the final value is exact
		if(ca->base.onFinish->f != NULL) {
			applyCallback(ca->base.onFinish);
		}
	}
}

ColAnimation *createColourFadeAnimation(bool *trigger, Color *toAnimate, Color target, bool match, float duration, bool fadeIn) {
	ColAnimation *ca = (ColAnimation *)malloc(sizeof(ColAnimation));

	ca->base.type = AT_COLOUR;
	ca->base.trigger = trigger;
	ca->base.match = match;
	ca->base.running = false;
	ca->base.duration = duration;
	ca->base.currentTime = 0.0f;
	ca->base.onFinish = (Callback *)malloc(sizeof(Callback));
	ca->base.onFinish->f = NULL;
	ca->base.onFinish->data = NULL;

	if(fadeIn) {
		ca->base.animate = colourFadeInAnimation;
	} else {
		ca->base.animate = colourFadeOutAnimation;
	}

	ca->toAnimate = toAnimate;
	ca->originalNormalised = ColorNormalize(*toAnimate);
	ca->targetNormalised = ColorNormalize(target);

	return ca;
}

void toggleAnimationCB(void *data) {
	Animation *a = (Animation *)data;
	a->running = !a->running;
}
