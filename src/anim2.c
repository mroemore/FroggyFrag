#include "anim2.h"

#include <stdio.h>
AnimationManager am;
int frameCounter = 0;

int getFrameCount() {
	return frameCounter;
}

static void incrementFrameCount() {
	frameCounter++;
}

void initDefaultASProperties(AnimationSegment *as, int durationFrames, AnimationSegmentType ast) {
	as->startFrames = 0;
	as->running = false;
	as->durationFrames = durationFrames;
	as->segmentType = ast;
}

void initMoveAnimationSegment(AnimationSegment *mas, int durationFrames, AnimationTargetType att, InterpolationType it, void *target, void *dest, bool reverse) {
	initDefaultASProperties(mas, durationFrames, AST_MOVE);
	mas->process = processMove;
	mas->targetType = att;
	mas->interpolate = assignEasingTransform(it);
	switch(att) {
		case ATT_INT:
			mas->target.i = (int *)target;
			if(reverse) {
				mas->orig.i = *((int *)dest);
				mas->dest.i = *((int *)target);
			} else {
				mas->orig.i = *((int *)target);
				mas->dest.i = *((int *)dest);
			}
			printf("TARGET: %i, ORIGINAL: %i, DESTINATION: %i\n", *mas->target.i, mas->orig.i, mas->dest.i);
			mas->animate = animateInt;
			break;
		case ATT_FLOAT:
			mas->target.f = (float *)target;
			if(reverse) {
				mas->orig.f = *((float *)dest);
				mas->dest.f = *((float *)target);
			} else {
				mas->orig.f = *((float *)target);
				mas->dest.f = *((float *)dest);
			}
			mas->animate = animateFloat;
			break;
		case ATT_UCHAR:
			mas->target.c = (unsigned char *)target;
			if(reverse) {
				mas->orig.c = *((unsigned char *)dest);
				mas->dest.c = *((unsigned char *)target);
			} else {
				mas->orig.c = *((unsigned char *)target);
				mas->dest.c = *((unsigned char *)dest);
			}
			mas->animate = animateUChar;
			break;
	}
}
void initRestAnimationSegment(AnimationSegment *ras, int durationFrames) {
	initDefaultASProperties(ras, durationFrames, AST_REST);
	ras->process = processRest;
}
void startSegment(AnimationSegment *as, int frameCount) {
	as->startFrames = frameCount;
	as->running = true;
}

void animateInt(void *self, float t) {
	AnimationSegment *mas = (AnimationSegment *)self;
	*mas->target.i = mas->orig.i + (int)((float)(mas->dest.i - mas->orig.i) * t);
}

void animateFloat(void *self, float t) {
	AnimationSegment *mas = (AnimationSegment *)self;
	*mas->target.f = mas->orig.f + (mas->dest.f - mas->orig.f) * t;
}

void animateUChar(void *self, float t) {
	AnimationSegment *mas = (AnimationSegment *)self;
	*mas->target.c = mas->orig.c + (unsigned char)((float)(mas->dest.c - mas->orig.c) * t);
}

bool processBlank(void *self, int frameCount) {
	return false;
}

bool processMove(void *self, int frameCount) {
	AnimationSegment *as = (AnimationSegment *)self;
	int currentTime = frameCount - as->startFrames;

	if(currentTime >= as->durationFrames) {
		// animation ended.
		as->running = false;
	} else {
		float t = (float)currentTime / (float)as->durationFrames;
		if(t < 0.0f) t = 0.0f;
		if(t > 1.0f) t = 1.0f;
		t = as->interpolate(t);
		as->animate(as, t);
	}

	return as->running;
}

bool processRest(void *self, int frameCount) {
	AnimationSegment *as = (AnimationSegment *)self;
	int currentTime = frameCount - as->startFrames;
	if(currentTime >= as->durationFrames) {
		// animation ended.
		as->running = false;
	}
	return as->running;
}

void initAnimationChain(AnimationChain *ac, ACPlaybackType t, ChainRetrigBehaviour crb) {
	ac->segmentIndex = 0;
	ac->segmentCount = 0;
	ac->playback = t;
	ac->playing = false;
	ac->retrig = crb;
}

void addAnimation(AnimationChain *ac, int duration, AnimationTargetType t, InterpolationType it, void *target, void *dest, bool reverse) {
	if(ac->segmentCount < MAX_AC_LEN - 1) {
		initMoveAnimationSegment(&ac->segments[ac->segmentCount], duration, t, it, target, dest, reverse);
		ac->segmentCount++;
	}
}

void addRest(AnimationChain *ac, int duration) {
	if(ac->segmentCount < MAX_AC_LEN - 1) {
		initRestAnimationSegment(&ac->segments[ac->segmentCount], duration);
		ac->segmentCount++;
	}
}

void setCustomRetrigAnimation(AnimationChain *ac, int duration, AnimationTargetType t, InterpolationType it, void *target, void *dest, bool reverse) {
	initMoveAnimationSegment(&ac->segments[MAX_AC_LEN - 1], duration, t, it, target, dest, reverse);
}

void startAnimationChain(AnimationChain *ac, int currentFrameCount) {
	if(!ac->playing) {
		ac->playing = true;
		startSegment(&ac->segments[ac->segmentIndex], currentFrameCount);
	} else {
		switch(ac->retrig) {
			case CRB_FULL_RESTART:
				ac->segmentIndex = 0;
				startSegment(&ac->segments[ac->segmentIndex], currentFrameCount);
				break;
			case CRB_RESTART_SEGMENT:
				startSegment(&ac->segments[ac->segmentIndex], currentFrameCount);
				break;
			case CRB_JUMP_TO_LAST:
				ac->segmentIndex = ac->segmentCount - 1;
				startSegment(&ac->segments[ac->segmentIndex], currentFrameCount);
				break;
			case CRB_CUSTOM_THEN_RESTART:
			case CRB_CUSTOM_THEN_LAST:
				ac->segmentIndex = MAX_AC_LEN - 1;
				startSegment(&ac->segments[ac->segmentIndex], currentFrameCount);
				break;
			default:
			case CRB_DO_NOTHING:
				break;
		}
	}
}

void resetAnimationChain(AnimationChain *ac) {
	ac->playing = false;
	ac->segmentIndex = 0;
	ac->segments[0].animate(&ac->segments[0], 0.0);
}

void tickAnimationChain(AnimationChain *ac, int currentFrameCount) {
	if(ac->playing) {
		if(!ac->segments[ac->segmentIndex].process(&ac->segments[ac->segmentIndex], currentFrameCount)) {
			// logic for current segment ending
			if(ac->segmentIndex + 1 < ac->segmentCount) {
				ac->segmentIndex++;
				startSegment(&ac->segments[ac->segmentIndex], currentFrameCount);
			} else if(ac->segmentIndex == MAX_AC_LEN - 1) { // case for custom interrupt
				if(ac->retrig == CRB_CUSTOM_THEN_LAST) {
					ac->segmentIndex = ac->segmentCount - 1;
					startSegment(&ac->segments[ac->segmentIndex], currentFrameCount);
				} else if(ac->retrig == CRB_CUSTOM_THEN_RESTART) {
					ac->segmentIndex = 0;
					startSegment(&ac->segments[ac->segmentIndex], currentFrameCount);
				}
			} else {
				switch(ac->playback) {
					case ACPT_LOOP_PINGPONG:
					case ACPT_LOOP:
						ac->segmentIndex = 0;
						startSegment(&ac->segments[ac->segmentIndex], currentFrameCount);

						break;
					default:
					case ACPT_PINGPONG:
					case ACPT_ONCE:
						ac->segmentIndex = 0;
						ac->playing = false;
						break;
				}
			}
		}
	}
}

void initAnimationChainGroup(AnimationChainGroup *acg, ACGroupPlaybackType t) {
	acg->chainCount = 0;
	acg->chainIndex = 0;
	acg->activeChains = 0;
	acg->type = t;
}

void addChainToGroup(AnimationChainGroup *acg, AnimationChain ac) {
	if(acg->chainCount < MAX_ACG_LEN) {
		acg->chains[acg->chainCount] = ac;
		acg->chainCount++;
	}
}

void startGroup(AnimationChainGroup *acg, int currentFrameCount) {
	for(int i = 0; i < acg->chainCount; i++) {
		startAnimationChain(&acg->chains[i], currentFrameCount);
	}
}

void resetGroup(AnimationChainGroup *acg) {
	for(int i = 0; i < acg->chainCount; i++) {
		resetAnimationChain(&acg->chains[i]);
	}
}

void togglePauseGroup(AnimationChainGroup *acg) { // do not use, needs to be fixed.
	for(int i = 0; i < acg->chainCount; i++) {
		acg->chains[i].playing = !acg->chains[i].playing;
	}
}

void tickAnimationChainGroup(AnimationChainGroup *acg, int currentFrameCount) {
	for(int i = 0; i < acg->chainCount; i++) {
		tickAnimationChain(&acg->chains[i], currentFrameCount);
	}
}

void initAnimationManager() {
	am.count = 0;
	am.freeCount = 0;
}

void addAnimationGroupToManager(AnimationChainGroup *acg) {
	if(am.freeCount > 0) {
		*am.freeList[am.freeCount - 1] = acg;
		am.freeCount--;
	} else {
		if(am.count < MAX_ACG_COUNT) {
			am.list[am.count] = acg;
			am.count++;
		}
	}
}

void tickManagedAnimations() {
	incrementFrameCount();
	for(int i = 0; i < am.count; i++) {
		tickAnimationChainGroup(am.list[i], getFrameCount());
	}
}

void startManagedGroup(AnimationChainGroup *acg) {
	for(int i = 0; i < am.count; i++) {
		if(am.list[i] == acg) {
			startGroup(acg, getFrameCount());
		}
	}
}
