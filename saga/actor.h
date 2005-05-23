/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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

#include "common/file.h"

#include "saga/sprite.h"
#include "saga/itedata.h"
#include "saga/list.h"
#include "saga/saga.h"

namespace Saga {

class HitZone;


#define ACTOR_DEBUG

#define ACTOR_BARRIERS_MAX 16

#define ACTOR_MAX_STEPS_COUNT 32

#define ACTOR_DIALOGUE_HEIGHT 100

#define ACTOR_LMULT 4

#define ACTOR_COLLISION_WIDTH       32
#define ACTOR_COLLISION_HEIGHT       8

#define ACTOR_DIRECTIONS_COUNT	4	// for ActorFrameSequence
#define ACTOR_DIRECTION_RIGHT	0
#define ACTOR_DIRECTION_LEFT	1
#define ACTOR_DIRECTION_BACK	2
#define ACTOR_DIRECTION_FORWARD	3

#define ACTOR_SPEECH_STRING_MAX 16	// speech const
#define ACTOR_SPEECH_ACTORS_MAX 8

#define ACTOR_NO_ENTRANCE -1

#define PATH_NODE_EMPTY -1

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

enum PathCellType {
	kPathCellEmpty = -1,
	//kDirUp = 0 .... kDirUpLeft = 7 
	kPathCellBarrier = 0x57
};

struct PathDirectionData {
	int8 direction;
	int16	x;
	int16 y;
};

struct ActorFrameRange {
	int frameIndex;
	int frameCount;
};

struct ActorFrameSequence {
	ActorFrameRange directions[ACTOR_DIRECTIONS_COUNT];
};

struct Location {
	int32 x;					// logical coordinates
	int32 y;					// 
	int32 z;					// 
	Location() {
		x = y = z = 0;
	}
	void saveState(Common::File& out) {
		out.writeSint32LE(x);
		out.writeSint32LE(y);
		out.writeSint32LE(z);
	}
	void loadState(Common::File& in) {
		x = in.readSint32LE();
		y = in.readSint32LE();
		z = in.readSint32LE();
	}

	int distance(const Location &location) const {
		return MAX(ABS(x - location.x), ABS(y - location.y));
	}
	int32 &u() {
		return x;
	}
	int32 &v() {
		return y;
	}
	int32 u() const {
		return x;
	}
	int32 v() const {
		return y;
	}
	int32 uv() const {
		return u() + v();
	}
	void delta(const Location &location, Location &result) const {
		result.x = x - location.x;
		result.y = y - location.y;
		result.z = z - location.z;
	}
	void addXY(const Location &location) {
		x += location.x;
		y += location.y;
	}
	void add(const Location &location) {
		x += location.x;
		y += location.y;
		z += location.z;
	}
	void fromScreenPoint(const Point &screenPoint) {
		x = (screenPoint.x * ACTOR_LMULT);
		y = (screenPoint.y * ACTOR_LMULT);
		z = 0;
	}
	void toScreenPointXY(Point &screenPoint) const {
		screenPoint.x = x / ACTOR_LMULT;
		screenPoint.y = y / ACTOR_LMULT;
	}
	void toScreenPointUV(Point &screenPoint) const {
		screenPoint.x = u();
		screenPoint.y = v();
	}
	void toScreenPointXYZ(Point &screenPoint) const {
		screenPoint.x = x / ACTOR_LMULT;
		screenPoint.y = y / ACTOR_LMULT - z;
	}
};

class CommonObjectData {
public:
//constant
	bool disabled;					// disabled in init section 
	int32 index;					// index in local array
	uint16 id;						// object id
	int32 scriptEntrypointNumber;	// script entrypoint number

//variables
	uint16 flags;				// initial flags
	int32 nameIndex;			// index in name string list
	int32 sceneNumber;			// scene
	int32 spriteListResourceId;	// sprite list resource id

	Location location;			// logical coordinates
	Point screenPosition;		// screen coordinates
	int32 screenDepth;			//
	int32 screenScale;			//

	void saveState(Common::File& out) {
		out.writeUint16LE(flags);
		out.writeSint32LE(nameIndex);
		out.writeSint32LE(sceneNumber);
		out.writeSint32LE(spriteListResourceId);
		location.saveState(out);
		out.writeSint16LE(screenPosition.x);
		out.writeSint16LE(screenPosition.y);
		out.writeSint32LE(screenDepth);
		out.writeSint32LE(screenScale);
	}
	void loadState(Common::File& in) {
		flags = in.readUint16LE();
		nameIndex = in.readSint32LE();
		sceneNumber = in.readSint32LE();
		spriteListResourceId = in.readSint32LE();
		location.loadState(in);
		screenPosition.x = in.readSint16LE();
		screenPosition.y = in.readSint16LE();
		screenDepth = in.readSint32LE();
		screenScale = in.readSint32LE();
	}
};

typedef CommonObjectData *CommonObjectDataPointer;

typedef SortedList<CommonObjectDataPointer> CommonObjectOrderList;

class ObjectData: public CommonObjectData {	
public:
	//constant
	uint16 interactBits;
	ObjectData() {
		memset(this, 0, sizeof(*this)); 
	}
};

class ActorData: public CommonObjectData {
public:
	//constant
	SpriteList spriteList;		// sprite list data

	ActorFrameSequence *frames;	// Actor's frames
	int framesCount;			// Actor's frames count
	int frameListResourceId;	// Actor's frame list resource id

	byte speechColor;			// Actor dialogue color
	
	//variables
	uint16 actorFlags;			// dynamic flags
	int32 currentAction;			// ActorActions type
	int32 facingDirection;		// orientation
	int32 actionDirection;
	int32 actionCycle;
	uint16 targetObject;
	const HitZone *lastZone;
	
	int32 cycleFrameSequence;
	uint8 cycleDelay;
	uint8 cycleTimeCount;
	uint8 cycleFlags;

	int32 frameNumber;			// current frame number
	
	int32 tileDirectionsAlloced;
	byte *tileDirections;

	int32 walkStepsAlloced;
	Point *walkStepsPoints;

	int32 walkStepsCount;
	int32 walkStepIndex;

	Location finalTarget;
	Location partialTarget;
	int32 walkFrameSequence;

	void saveState(Common::File& out) {
		CommonObjectData::saveState(out);
		out.writeUint16LE(actorFlags);
		out.writeSint32LE(currentAction);
		out.writeSint32LE(facingDirection);
		out.writeSint32LE(actionDirection);
		out.writeSint32LE(actionCycle);
		out.writeUint16LE(targetObject);

		//TODO: write lastZone 
		out.writeSint32LE(cycleFrameSequence);
		out.writeByte(cycleDelay);
		out.writeByte(cycleTimeCount);
		out.writeByte(cycleFlags);
		out.writeSint32LE(frameNumber);

		out.writeSint32LE(tileDirectionsAlloced);
		for (int i = 0; i < tileDirectionsAlloced; i++) {
			out.writeByte(tileDirections[i]);
		}

		out.writeSint32LE(walkStepsAlloced);
		for (int i = 0; i < walkStepsAlloced; i++) {
			out.writeSint16LE(walkStepsPoints[i].x);
			out.writeSint16LE(walkStepsPoints[i].y);
		}

		out.writeSint32LE(walkStepsCount);
		out.writeSint32LE(walkStepIndex);
		finalTarget.saveState(out);
		partialTarget.saveState(out);
		out.writeSint32LE(walkFrameSequence);
	}
	void loadState(Common::File& in) {
		CommonObjectData::loadState(in);
		actorFlags = in.readUint16LE();
		currentAction = in.readSint32LE();
		facingDirection = in.readSint32LE();
		actionDirection = in.readSint32LE();
		actionCycle = in.readSint32LE();
		targetObject = in.readUint16LE();

		//TODO: read lastZone 
		lastZone = NULL;
		cycleFrameSequence = in.readSint32LE();
		cycleDelay = in.readByte();
		cycleTimeCount = in.readByte();
		cycleFlags = in.readByte();
		frameNumber = in.readSint32LE();

		
		setTileDirectionsSize(in.readSint32LE(), true);
		for (int i = 0; i < tileDirectionsAlloced; i++) {
			tileDirections[i] = in.readByte();
		}

		setWalkStepsPointsSize(in.readSint32LE(), true);
		for (int i = 0; i < walkStepsAlloced; i++) {
			walkStepsPoints[i].x = in.readSint16LE();
			walkStepsPoints[i].y = in.readSint16LE();
		}

		walkStepsCount = in.readSint32LE();
		walkStepIndex = in.readSint32LE();
		finalTarget.loadState(in);
		partialTarget.loadState(in);
		walkFrameSequence = in.readSint32LE();
	}

	void setTileDirectionsSize(int size, bool forceRealloc) {
		if ((size <= tileDirectionsAlloced) && !forceRealloc) {
			return;
		}
		tileDirectionsAlloced = size;
		tileDirections = (byte*)realloc(tileDirections, tileDirectionsAlloced * sizeof(*tileDirections));
	}

	void cycleWrap(int cycleLimit) {
		if (actionCycle >= cycleLimit)
			actionCycle = 0;
	}

	void setWalkStepsPointsSize(int size, bool forceRealloc) {
		if ((size <= walkStepsAlloced) && !forceRealloc) {
			return;
		}
		walkStepsAlloced = size;
		walkStepsPoints = (Point*)realloc(walkStepsPoints, walkStepsAlloced * sizeof(*walkStepsPoints));		
	}

	void addWalkStepPoint(const Point &point) {
		setWalkStepsPointsSize(walkStepsCount + 1, false);
		walkStepsPoints[walkStepsCount++] = point;
	}

	ActorData() {
		memset(this, 0, sizeof(*this)); 
	}
	~ActorData() {
		free(frames);
		free(tileDirections);
		free(walkStepsPoints);
		spriteList.freeMem();
	}
};



struct SpeechData {
	int speechColor[ACTOR_SPEECH_ACTORS_MAX];
	int outlineColor[ACTOR_SPEECH_ACTORS_MAX];
	int speechFlags;
	const char *strings[ACTOR_SPEECH_STRING_MAX];
	Point speechCoords[ACTOR_SPEECH_ACTORS_MAX];
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
	friend class IsoMap;
	friend class SagaEngine;
public:

	Actor(SagaEngine *vm);
	~Actor();

	void cmdActorWalkTo(int argc, const char **argv);

	bool validActorId(uint16 id) { return (id == ID_PROTAG) || ((id >= objectIndexToId(kGameObjectActor, 0)) && (id < objectIndexToId(kGameObjectActor, _actorsCount))); }
	int actorIdToIndex(uint16 id) { return (id == ID_PROTAG ) ? 0 : objectIdToIndex(id); }
	uint16 actorIndexToId(int index) { return (index == 0 ) ? ID_PROTAG : objectIndexToId(kGameObjectActor, index); }
	ActorData *getActor(uint16 actorId);
	
// clarification: Obj - means game object, such Hat, Spoon etc,  Object - means Actor,Obj,HitZone,StepZone

	bool validObjId(uint16 id) { return (id >= objectIndexToId(kGameObjectObject, 0)) && (id < objectIndexToId(kGameObjectObject, _objsCount)); }
	int objIdToIndex(uint16 id) { return objectIdToIndex(id); }
	uint16 objIndexToId(int index) { return objectIndexToId(kGameObjectObject, index); }
	ObjectData *getObj(uint16 objId);
	
	int getObjectScriptEntrypointNumber(uint16 id) {
		int objectType;
		objectType = objectTypeId(id);
		if (!(objectType & (kGameObjectObject | kGameObjectActor))) {
			error("Actor::getObjectScriptEntrypointNumber wrong id 0x%X", id);
		}
		return (objectType == kGameObjectObject) ? getObj(id)->scriptEntrypointNumber : getActor(id)->scriptEntrypointNumber;
	}
	int getObjectFlags(uint16 id) {
		int objectType;
		objectType = objectTypeId(id);
		if (!(objectType & (kGameObjectObject | kGameObjectActor))) {
			error("Actor::getObjectFlags wrong id 0x%X", id);
		}
		return (objectType == kGameObjectObject) ? getObj(id)->flags : getActor(id)->flags;
	}

	int direct(int msec);
	void drawActors();
	void updateActorsScene(int actorsEntrance);			// calls from scene loading to update Actors info

	void drawSpeech();

	void drawPathTest();

	uint16 hitTest(const Point &testPoint, bool skipProtagonist);
	void takeExit(uint16 actorId, const HitZone *hitZone);
	bool actorEndWalk(uint16 actorId, bool recurse);
	bool actorWalkTo(uint16 actorId, const Location &toLocation);		
	ActorFrameRange *getActorFrameRange(uint16 actorId, int frameType);
	void actorFaceTowardsPoint(uint16 actorId, const Location &toLocation);
	void actorFaceTowardsObject(uint16 actorId, uint16 objectId);

	void realLocation(Location &location, uint16 objectId, uint16 walkFlags);

//	speech 
	void actorSpeech(uint16 actorId, const char **strings, int stringsCount, uint16 sampleResourceId, int speechFlags);
	void nonActorSpeech(const char **strings, int stringsCount, int speechFlags);
	void simulSpeech(const char *string, uint16 *actorIds, int actorIdsCount, int speechFlags, int sampleResourceId);
	void setSpeechColor(int speechColor, int outlineColor) {
		_activeSpeech.speechColor[0] = speechColor;
		_activeSpeech.outlineColor[0] = outlineColor;
	}
	void abortAllSpeeches();
	void abortSpeech();
	bool isSpeaking() {
		return _activeSpeech.stringsCount > 0;
	}

	void saveState(Common::File& out);
	void loadState(Common::File& in);

	void setProtagState(int state);
	int getProtagState() { return _protagState; }
	
private:
	bool loadActorResources(ActorData *actor);
	void stepZoneAction(ActorData *actor, const HitZone *hitZone, bool exit, bool stopped);

	void createDrawOrderList();
	void calcScreenPosition(CommonObjectData *commonObjectData);
	bool getSpriteParams(CommonObjectData *commonObjectData, int &frameNumber, SpriteList *&spriteList);

	bool followProtagonist(ActorData *actor);
	void findActorPath(ActorData *actor, const Point &fromPoint, const Point &toPoint);
	void handleSpeech(int msec);
	void handleActions(int msec, bool setup);
	bool validPathCellPoint(const Point &testPoint) {
		return !((testPoint.x < 0) || (testPoint.x >= _xCellCount) ||
			(testPoint.y < 0) || (testPoint.y >= _yCellCount));
	}
	void setPathCell(const Point &testPoint, int8 value) {		
		if (!validPathCellPoint(testPoint)) {
			error("Actor::setPathCell wrong point");
		}
		_pathCell[testPoint.x + testPoint.y * _xCellCount] = value;
	}
	int8 getPathCell(const Point &testPoint) {
		if (!validPathCellPoint(testPoint)) {
			error("Actor::getPathCell wrong point");
		}
		return _pathCell[testPoint.x + testPoint.y * _xCellCount];
	}
	bool scanPathLine(const Point &point1, const Point &point2);
	int fillPathArray(const Point &fromPoint, const Point &toPoint, Point &bestPoint);
	void setActorPath(ActorData *actor, const Point &fromPoint, const Point &toPoint);
	void pathToNode();
	void condenseNodeList();
	void removeNodes();
	void nodeToPath();
	void removePathPoints();
	bool validFollowerLocation(const Location &location);
	
	
protected:
//constants
	int _actorsCount;
	ActorData **_actors;

	int _objsCount;
	ObjectData **_objs;

	SagaEngine *_vm;
	RSCFILE_CONTEXT *_actorContext;

	StringsTable _actorsStrings;
	int _lastTickMsec;
	CommonObjectOrderList _drawOrderList;

//variables
public:
	ActorData *_centerActor;
	ActorData *_protagonist;
	int _handleActionDiv;
protected:
	SpeechData _activeSpeech;
	int _protagState;

private:
//path stuff
	struct PathNode {
		Point point;
		int link;
	};

	Rect _barrierList[ACTOR_BARRIERS_MAX];
	int _barrierCount;
	int8 *_pathCell;

	int _xCellCount;
	int _yCellCount;
	Rect _pathRect;

	PathDirectionData *_pathDirectionList;
	int _pathDirectionListCount;
	int _pathDirectionListAlloced;
	PathDirectionData * addPathDirectionListData() {
		if (_pathDirectionListCount + 1 >= _pathDirectionListAlloced) {
			_pathDirectionListAlloced += 100;
			_pathDirectionList = (PathDirectionData*) realloc(_pathDirectionList, _pathDirectionListAlloced * sizeof(*_pathDirectionList));
		}
		return &_pathDirectionList[_pathDirectionListCount++];
	}

	Point *_pathList;
	int _pathListIndex;
	int _pathListAlloced;
	void addPathListPoint(const Point &point) {
		++_pathListIndex;
		if (_pathListIndex >= _pathListAlloced) {
			_pathListAlloced += 100;
			_pathList = (Point*) realloc(_pathList, _pathListAlloced * sizeof(*_pathList));
			
		}
		_pathList[_pathListIndex] = point;
	}

	int _pathNodeListIndex;
	int _pathNodeListAlloced;
	PathNode *_pathNodeList;
	void addPathNodeListPoint(const Point &point) {
		++_pathNodeListIndex;
		if (_pathNodeListIndex >= _pathNodeListAlloced) {
			_pathNodeListAlloced += 100;
			_pathNodeList = (PathNode*) realloc(_pathNodeList, _pathNodeListAlloced * sizeof(*_pathNodeList));

		}
		_pathNodeList[_pathNodeListIndex].point = point;
	}

	int _newPathNodeListIndex;
	int _newPathNodeListAlloced;
	PathNode *_newPathNodeList;
	void incrementNewPathNodeListIndex() {
		++_newPathNodeListIndex;
		if (_newPathNodeListIndex >= _newPathNodeListAlloced) {
			_newPathNodeListAlloced += 100;
			_newPathNodeList = (PathNode*) realloc(_newPathNodeList, _newPathNodeListAlloced * sizeof(*_newPathNodeList));

		}
	}
	void addNewPathNodeListPoint(const PathNode &pathNode) {
		incrementNewPathNodeListIndex();
		_newPathNodeList[_newPathNodeListIndex] = pathNode;
	}

public:
#ifdef ACTOR_DEBUG
//path debug - use with care
	struct DebugPoint {
		Point point;
		byte color;
	};
	DebugPoint *_debugPoints;
	int _debugPointsCount;
	int _debugPointsAlloced;
	void addDebugPoint(const Point &point, byte color) {
		if (_debugPointsCount + 1 > _debugPointsAlloced) {
			_debugPointsAlloced += 1000;
			_debugPoints = (DebugPoint*) realloc(_debugPoints, _debugPointsAlloced * sizeof(*_debugPoints));
		}
		_debugPoints[_debugPointsCount].color = color;
		_debugPoints[_debugPointsCount++].point = point;
	}
#endif
};

inline int16 quickDistance(const Point &point1, const Point &point2) {
	Point delta;
	delta.x = ABS(point1.x - point2.x);
	delta.y = ABS(point1.y - point2.y);
	return ((delta.x < delta.y) ? (delta.y + delta.x / 2) : (delta.x + delta.y / 2));
}
} // End of namespace Saga

#endif
