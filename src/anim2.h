#ifndef ANIMATION2_H
#define ANIMATION2_H

#include <stdbool.h>
#include "ease.h"

#define MAX_AC_LEN 64 // effectively 1 less than stated here, since the last index is reserved for a custom interrupt effect.
#define MAX_ACG_LEN 64
#define MAX_ACG_COUNT 64

typedef void (*ProcessAnimation)(void *self, float t);
typedef bool (*ProcessSegment)(void *self, int currentFrames);

typedef enum {
	ACPT_ONCE,
	ACPT_LOOP,
	ACPT_PINGPONG,
	ACPT_LOOP_PINGPONG
} ACPlaybackType;

typedef enum {
	AST_MOVE,
	AST_REST
} AnimationSegmentType;

typedef enum {
	ATT_INT,
	ATT_FLOAT,
	ATT_UCHAR
} AnimationTargetType;

typedef enum {
	ACGP_SEQUENTIAL,
	ACGP_PARALLEL
} ACGroupPlaybackType;

typedef enum {
	CRB_DO_NOTHING,
	CRB_FULL_RESTART,
	CRB_RESTART_SEGMENT,
	CRB_JUMP_TO_LAST,
	CRB_CUSTOM_THEN_RESTART,
	CRB_CUSTOM_THEN_LAST
} ChainRetrigBehaviour;

typedef struct {
	int startFrames;
	int durationFrames;
	bool running;
	ProcessSegment process;
	AnimationSegmentType segmentType;
	AnimationTargetType targetType;
	union {
		float *f;
		int *i;
		unsigned char *c;
	} target;
	union {
		float f;
		int i;
		unsigned char c;
	} orig;
	union {
		float f;
		int i;
		unsigned char c;
	} dest;
	InterpolationFunction interpolate;
	ProcessAnimation animate;
} AnimationSegment;

int getFrameCount();
static void incrementFrameCount();

void initDefaultASProperties(AnimationSegment *as, int durationFrames, AnimationSegmentType ast);

void initMoveAnimationSegment(AnimationSegment *mas, int durationFrames, AnimationTargetType att, InterpolationType it, void *target, void *dest, bool reverse);
void initRestAnimationSegment(AnimationSegment *ras, int durationFrames);
void startSegment(AnimationSegment *as, int frameCount);

void animateInt(void *moveAnimationSegment, float t);
void animateFloat(void *moveAnimationSegment, float t);
void animateUChar(void *self, float t);

bool processMove(void *as, int frameCount);
bool processRest(void *as, int frameCount);

typedef struct {
	AnimationSegment segments[MAX_AC_LEN];
	int segmentCount;
	int segmentIndex;
	ACPlaybackType playback;
	bool playing;
	ChainRetrigBehaviour retrig;
} AnimationChain;

void initAnimationChain(AnimationChain *ac, ACPlaybackType t, ChainRetrigBehaviour crb);

void addAnimation(AnimationChain *ac, int duration, AnimationTargetType t, InterpolationType it, void *target, void *dest, bool reverse);
void addRest(AnimationChain *ac, int duration);
void setCustomRetrigAnimation(AnimationChain *ac, int duration, AnimationTargetType t, InterpolationType it, void *target, void *dest, bool reverse);

void startAnimationChain(AnimationChain *ac, int currentFrameCount);
void resetAnimationChain(AnimationChain *ac);
void tickAnimationChain(AnimationChain *ac, int currentFrameCount);

typedef struct {
	AnimationChain chains[MAX_ACG_LEN];
	int chainCount;
	int chainIndex;
	ACGroupPlaybackType type;
	int activeChains;
	bool running;
} AnimationChainGroup;

void initAnimationChainGroup(AnimationChainGroup *acg, ACGroupPlaybackType t);
void addChainToGroup(AnimationChainGroup *acg, AnimationChain ac);
void startGroup(AnimationChainGroup *acg, int currentFrameCount);
void resetGroup(AnimationChainGroup *acg);
void togglePauseGroup(AnimationChainGroup *acg);
void tickAnimationChainGroup(AnimationChainGroup *acg, int currentFrameCount);

typedef struct {
	AnimationChainGroup *list[MAX_ACG_COUNT];
	int count;
	AnimationChainGroup **freeList[MAX_ACG_COUNT];
	int freeCount;
} AnimationManager;

void initAnimationManager();
void addAnimationGroupToManager(AnimationChainGroup *acg);
void tickManagedAnimations();
void startManagedGroup(AnimationChainGroup *acg);
#endif
