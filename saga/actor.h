/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// Actor management module header file

#ifndef SAGA_ACTOR_H__
#define SAGA_ACTOR_H__

#include "saga/sprite.h"
#include "saga/actordata.h"
#include "saga/list.h"

namespace Saga {

#define ACTOR_BASE_SPEED 0.25
#define ACTOR_BASE_ZMOD 0.5

#define ACTOR_STEPS_COUNT 32
#define ACTOR_STEPS_MAX (ACTOR_STEPS_COUNT*2)

#define ACTOR_ACTIONTIME 80

#define ACTOR_DIALOGUE_HEIGHT 100

#define ACTOR_LMULT 4

#define ACTOR_DIRECTIONS_COUNT	4	// for ActorFrameSequence
#define ACTOR_DIRECTION_RIGHT	0
#define ACTOR_DIRECTION_LEFT	1
#define ACTOR_DIRECTION_BACK	2
#define ACTOR_DIRECTION_FORWARD	3

#define ACTOR_SPEECH_STRING_MAX 16	// speech const
#define ACTOR_SPEECH_ACTORS_MAX 8

#define ID_NOTHING 0
#define ID_PROTAG 1

#define IS_VALID_ACTOR_INDEX(index) ((index >= 0) && (index < ACTORCOUNT))
#define IS_VALID_ACTOR_ID(id) ((id == 1) || (id >= 0x2000) && (id < (0x2000 | ACTORCOUNT)))
#define ACTOR_ID_TO_INDEX(id) ((((uint16)id) == 1 ) ? 0 : (int)(((uint16)id) & ~0x2000))
#define ACTOR_INDEX_TO_ID(index) ((((int)index) == 0 ) ? 1 : (uint16)(((int)index) | 0x2000))


enum ActorActions {
	kActionWait = 0,
	kActionWalkToPoint = 1,
	kActionWalkToLink = 2,
	kActionWalkDir = 3,
	kActionSpeak = 4,
	kActionAccept = 5,
	kActionStoop = 6,
	kActionLook = 7,
	kActionCycleFrames = 8,
	kActionPongFrames = 9,
	kActionFreeze = 10,
	kActionFall = 11,
	kActionClimb = 12
};

enum SpeechFlags {
	kSpeakNoAnimate = 1,
	kSpeakAsync = 2,
	kSpeakSlow = 4
};

enum ActorFrameTypes {
	kFrameStand = 0,
	kFrameWalk = 1,
	kFrameSpeak = 2,
	kFrameGive = 3,
	kFrameGesture = 4,
	kFrameWait = 5,
	kFramePickUp = 6,
	kFrameLook = 7
//...some special
};

enum ActorFlagsEx {
	kActorNoCollide = (1 << 0),
	kActorNoFollow = (1 << 1),
	kActorCollided = (1 << 2),
	kActorBackwards = (1 << 3),
	kActorContinuous = (1 << 4),
	kActorFinalFace = (1 << 5),
	kActorFinishLeft = ((1 << 5) | (kDirLeft << 6)),
	kActorFinishRight = ((1 << 5) | (kDirRight << 6)),
	kActorFinishUp = ((1 << 5) | (kDirUp << 6)),
	kActorFinishDown = ((1 << 5) | (kDirDown << 6)),
	kActorFacingMask = (0xf << 5),
	kActorRandom = (1 << 10)
};


struct ActorFrameRange {
	int frameIndex;
	int frameCount;
};

struct ActorFrameSequence {
	ActorFrameRange directions[ACTOR_DIRECTIONS_COUNT];
};

struct ActorLocation {
	int x;					// Actor's logical coordinates
	int y;					// 
	int z;					// 
	int distance(const ActorLocation &location) {
		return max(abs(x - location.x), abs(y - location.y));
	}
};
struct ActorData {
	bool disabled;				// Actor disabled in init section
	int index;					// Actor index
	uint16 actorId;				// Actor id
	int nameIndex;				// Actor's index in actor name string list
	byte speechColor;			// Actor dialogue color
	uint16 flags;				// Actor initial flags
	

	int sceneNumber;			// scene of actor
	ActorLocation location;		// Actor's logical coordinates

	Point screenPosition;		// Actor's screen coordinates
	int screenDepth;			//
	int screenScale;			//

	uint16 actorFlags;			// dynamic flags
	int currentAction;			// ActorActions type
	int facingDirection;		// orientation
	int actionDirection;
	int actionCycle;
	int frameNumber;			// current actor frame number
	uint16 targetObject;		
	
	int cycleFrameSequence;
	uint8 cycleDelay;
	uint8 cycleTimeCount;
	uint8 cycleFlags;

	SPRITELIST *spriteList;		// Actor's sprite list data
	int spriteListResourceId;	// Actor's sprite list resource id

	ActorFrameSequence *frames;	// Actor's frames
	int framesCount;			// Actor's frames count
	int frameListResourceId;	// Actor's frame list resource id
	
	int walkPath[ACTOR_STEPS_MAX];
	int walkStepsCount;
	int walkStepIndex;
	ActorLocation finalTarget;
	ActorLocation partialTarget;
	int walkFrameSequence;
	
	void cycleWrap(int cycleLimit) {
		if (actionCycle >= cycleLimit)
			actionCycle = 0;
	}

	ActorData() {
		memset(this, 0xFE, sizeof(*this)); 
	}
};

typedef ActorData* ActorDataPointer;
typedef SortedList<ActorDataPointer> ActorOrderList;

struct SpeechData {
	int speechColor;
	int outlineColor;
	int speechFlags;
	const char *strings[ACTOR_SPEECH_STRING_MAX];
	int stringsCount;
	int slowModeCharIndex;
	uint16 actorIds[ACTOR_SPEECH_ACTORS_MAX];
	int actorsCount;
	int sampleResourceId;
	bool playing;
	int playingTime;

	SpeechData() { 
		memset(this, 0, sizeof(*this)); 
	}
};

class Actor {
public:
	ActorData *_centerActor;
	ActorData *_protagonist;

	Actor(SagaEngine *vm);
	~Actor();
/*
	void CF_actor_move(int argc, const char **argv);
	void CF_actor_moverel(int argc, const char **argv);
	void CF_actor_seto(int argc, const char **argv);
	void CF_actor_setact(int argc, const char **argv);
*/
	int direct(int msec);
	int drawActors();
	void updateActorsScene();			// calls from scene loading to update Actors info

	void StoA(Point &actorPoint, const Point &screenPoint);

	
	bool actorEndWalk(uint16 actorId, bool recurse);
	bool actorWalkTo(uint16 actorId, const ActorLocation &toLocation);
	ActorData *getActor(uint16 actorId);
	ActorFrameRange *getActorFrameRange(uint16 actorId, int frameType);

//	speech 
	void actorSpeech(uint16 actorId, const char **strings, int stringsCount, uint16 sampleResourceId, int speechFlags);
	void nonActorSpeech(const char **strings, int stringsCount, int speechFlags);
	void simulSpeech(const char *string, uint16 *actorIds, int actorIdsCount, int speechFlags);
	void setSpeechColor(int speechColor, int outlineColor) {
		_activeSpeech.speechColor = speechColor;
		_activeSpeech.outlineColor = outlineColor;
	}
	void abortAllSpeeches();
	void abortSpeech();
	bool isSpeaking() {
		return _activeSpeech.stringsCount > 0;
	}
	
private:
	bool loadActorResources(ActorData * actor);
	
	void createDrawOrderList();
	void calcActorScreenPosition(ActorData * actor);
	bool followProtagonist(ActorData * actor);
	void handleSpeech(int msec);
	void handleActions(int msec, bool setup);
	
	int _lastTickMsec;
	SagaEngine *_vm;
	RSCFILE_CONTEXT *_actorContext;
	ActorOrderList _drawOrderList;
	ActorData _actors[ACTORCOUNT];
	SpeechData _activeSpeech;
};

} // End of namespace Saga

#endif
