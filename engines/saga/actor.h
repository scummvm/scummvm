/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

// Actor management module header file

#ifndef SAGA_ACTOR_H
#define SAGA_ACTOR_H

#include "common/savefile.h"

#include "saga/sprite.h"
#include "saga/itedata.h"
#include "saga/list.h"
#include "saga/saga.h"
#include "saga/font.h"

namespace Saga {

class HitZone;


//#define ACTOR_DEBUG 1 //only for actor pathfinding debug!

#define ACTOR_BARRIERS_MAX 16

#define ACTOR_MAX_STEPS_COUNT 32

#define ACTOR_DIALOGUE_HEIGHT 100

#define ACTOR_LMULT 4

#define ACTOR_SPEED 72

#define ACTOR_CLIMB_SPEED 8

#define ACTOR_COLLISION_WIDTH       32
#define ACTOR_COLLISION_HEIGHT       8

#define ACTOR_DIRECTIONS_COUNT	4	// for ActorFrameSequence
#define ACTOR_DIRECTION_RIGHT	0
#define ACTOR_DIRECTION_LEFT	1
#define ACTOR_DIRECTION_BACK	2
#define ACTOR_DIRECTION_FORWARD	3

#define ACTOR_SPEECH_STRING_MAX 16	// speech const
#define ACTOR_SPEECH_ACTORS_MAX 8

#define ACTOR_DRAGON_TURN_MOVES 4
#define ACTOR_DRAGON_INDEX 133

#define ACTOR_NO_ENTRANCE -1

#define ACTOR_EXP_KNOCK_RIF 24

#define PATH_NODE_EMPTY -1

#define ACTOR_INHM_SIZE 228

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

enum ActorFrameIds {
//ITE
	kFrameITEStand = 0,
	kFrameITEWalk = 1,
	kFrameITESpeak = 2,
	kFrameITEGive = 3,
	kFrameITEGesture = 4,
	kFrameITEWait = 5,
	kFrameITEPickUp = 6,
	kFrameITELook = 7,
//IHNM
	kFrameIHNMStand = 0,
	kFrameIHNMSpeak = 1,
	kFrameIHNMWait = 2,
	kFrameIHNMGesture = 3,
	kFrameIHNMWalk = 4
};

enum SpeechFlags {
	kSpeakNoAnimate = 1,
	kSpeakAsync = 2,
	kSpeakSlow = 4,
	kSpeakForceText = 8
};

enum ActorFrameTypes {
	kFrameStand,
	kFrameWalk,
	kFrameSpeak,
	kFrameGive,
	kFrameGesture,
	kFrameWait,
	kFramePickUp,
	kFrameLook
};

// Lookup table to convert 8 cardinal directions to 4
static const int actorDirectectionsLUT[8] = {
	ACTOR_DIRECTION_BACK,	// kDirUp
	ACTOR_DIRECTION_RIGHT,	// kDirUpRight
	ACTOR_DIRECTION_RIGHT,	// kDirRight
	ACTOR_DIRECTION_RIGHT,	// kDirDownRight
	ACTOR_DIRECTION_FORWARD,// kDirDown
	ACTOR_DIRECTION_LEFT,	// kDirDownLeft
	ACTOR_DIRECTION_LEFT,	// kDirLeft
	ACTOR_DIRECTION_LEFT,	// kDirUpLeft
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

enum DragonMoveTypes {
	kDragonMoveUpLeft			=	0,
	kDragonMoveUpRight			=	1,
	kDragonMoveDownLeft			=	2,
	kDragonMoveDownRight		=	3,
	kDragonMoveUpLeft_Left		=	4,
	kDragonMoveUpLeft_Right		=	5,
	kDragonMoveUpRight_Left		=	6,
	kDragonMoveUpRight_Right	=	7,
	kDragonMoveDownLeft_Left	=	8,
	kDragonMoveDownLeft_Right	=	9,
	kDragonMoveDownRight_Left	=	10,
	kDragonMoveDownRight_Right	=	11,
	kDragonMoveInvalid			=	12
};

struct PathDirectionData {
	int8 direction;
	int x;
	int y;
};

struct ActorFrameRange {
	int frameIndex;
	int frameCount;
};

struct ActorFrameSequence {
	ActorFrameRange directions[ACTOR_DIRECTIONS_COUNT];
};

int pathLine(Point *pointList, const Point &point1, const Point &point2);

struct Location {
	int32 x;					// logical coordinates
	int32 y;					//
	int32 z;					//
	Location() {
		x = y = z = 0;
	}
	void saveState(Common::OutSaveFile *out) {
		out->writeSint32LE(x);
		out->writeSint32LE(y);
		out->writeSint32LE(z);
	}
	void loadState(Common::InSaveFile *in) {
		x = in->readSint32LE();
		y = in->readSint32LE();
		z = in->readSint32LE();
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
	void fromStream(MemoryReadStream &stream) {
		x = stream.readUint16LE();
		y = stream.readUint16LE();
		z = stream.readUint16LE();
	}

#if 0
	// Obsolete function, throws warnings in older versions of GCC
	// (warning: int format, int32 arg)
	// Keeping it around for debug purposes
	void debugPrint(int debuglevel = 0, const char *loc = "Loc:") const {
		debug(debuglevel, "%s %d, %d, %d", loc, x, y, z);
	}
#endif

};

class CommonObjectData {
public:
//constant
	bool _disabled;					// disabled in init section
	int32 _index;					// index in local array
	uint16 _id;						// object id
	int32 _scriptEntrypointNumber;	// script entrypoint number

//variables
	uint16 _flags;				// initial flags
	int32 _nameIndex;			// index in name string list
	int32 _sceneNumber;			// scene
	int32 _spriteListResourceId;	// sprite list resource id

	Location _location;			// logical coordinates
	Point _screenPosition;		// screen coordinates
	int32 _screenDepth;			//
	int32 _screenScale;			//

	void saveState(Common::OutSaveFile *out) {
		out->writeUint16LE(_flags);
		out->writeSint32LE(_nameIndex);
		out->writeSint32LE(_sceneNumber);
		out->writeSint32LE(_spriteListResourceId);
		_location.saveState(out);
		out->writeSint16LE(_screenPosition.x);
		out->writeSint16LE(_screenPosition.y);
		out->writeSint32LE(_screenDepth);
		out->writeSint32LE(_screenScale);
	}
	void loadState(Common::InSaveFile *in) {
		_flags = in->readUint16LE();
		_nameIndex = in->readSint32LE();
		_sceneNumber = in->readSint32LE();
		_spriteListResourceId = in->readSint32LE();
		_location.loadState(in);
		_screenPosition.x = in->readSint16LE();
		_screenPosition.y = in->readSint16LE();
		_screenDepth = in->readSint32LE();
		_screenScale = in->readSint32LE();
	}
};

typedef CommonObjectData *CommonObjectDataPointer;

typedef SortedList<CommonObjectDataPointer> CommonObjectOrderList;

class ObjectData: public CommonObjectData {
public:
	//constant
	uint16 _interactBits;
	ObjectData() {
		memset(this, 0, sizeof(*this));
	}
};

class ActorData: public CommonObjectData {
public:
	//constant
	SpriteList _spriteList;		// sprite list data

	bool _shareFrames;
	ActorFrameSequence *_frames;	// Actor's frames
	int _framesCount;			// Actor's frames count
	int _frameListResourceId;	// Actor's frame list resource id

	byte _speechColor;			// Actor dialogue color
	//
	bool _inScene;

	//variables
	uint16 _actorFlags;			// dynamic flags
	int32 _currentAction;			// ActorActions type
	int32 _facingDirection;		// orientation
	int32 _actionDirection;
	int32 _actionCycle;
	uint16 _targetObject;
	const HitZone *_lastZone;

	int32 _cycleFrameSequence;
	uint8 _cycleDelay;
	uint8 _cycleTimeCount;
	uint8 _cycleFlags;

	int16 _fallVelocity;
	int16 _fallAcceleration;
	int16 _fallPosition;

	uint8 _dragonBaseFrame;
	uint8 _dragonStepCycle;
	uint8 _dragonMoveType;

	int32 _frameNumber;			// current frame number

	int32 _tileDirectionsAlloced;
	byte *_tileDirections;

	int32 _walkStepsAlloced;
	Point *_walkStepsPoints;

	int32 _walkStepsCount;
	int32 _walkStepIndex;

	Location _finalTarget;
	Location _partialTarget;
	int32 _walkFrameSequence;

public:
	void saveState(Common::OutSaveFile *out) {
		int i = 0;
		CommonObjectData::saveState(out);
		out->writeUint16LE(_actorFlags);
		out->writeSint32LE(_currentAction);
		out->writeSint32LE(_facingDirection);
		out->writeSint32LE(_actionDirection);
		out->writeSint32LE(_actionCycle);
		out->writeUint16LE(_targetObject);

		out->writeSint32LE(_cycleFrameSequence);
		out->writeByte(_cycleDelay);
		out->writeByte(_cycleTimeCount);
		out->writeByte(_cycleFlags);
		out->writeSint16LE(_fallVelocity);
		out->writeSint16LE(_fallAcceleration);
		out->writeSint16LE(_fallPosition);
		out->writeByte(_dragonBaseFrame);
		out->writeByte(_dragonStepCycle);
		out->writeByte(_dragonMoveType);
		out->writeSint32LE(_frameNumber);

		out->writeSint32LE(_tileDirectionsAlloced);
		for (i = 0; i < _tileDirectionsAlloced; i++) {
			out->writeByte(_tileDirections[i]);
		}

		out->writeSint32LE(_walkStepsAlloced);
		for (i = 0; i < _walkStepsAlloced; i++) {
			out->writeSint16LE(_walkStepsPoints[i].x);
			out->writeSint16LE(_walkStepsPoints[i].y);
		}

		out->writeSint32LE(_walkStepsCount);
		out->writeSint32LE(_walkStepIndex);
		_finalTarget.saveState(out);
		_partialTarget.saveState(out);
		out->writeSint32LE(_walkFrameSequence);
	}

	void loadState(uint32 version, Common::InSaveFile *in) {
		int i = 0;
		CommonObjectData::loadState(in);
		_actorFlags = in->readUint16LE();
		_currentAction = in->readSint32LE();
		_facingDirection = in->readSint32LE();
		_actionDirection = in->readSint32LE();
		_actionCycle = in->readSint32LE();
		_targetObject = in->readUint16LE();

		_lastZone = NULL;
		_cycleFrameSequence = in->readSint32LE();
		_cycleDelay = in->readByte();
		_cycleTimeCount = in->readByte();
		_cycleFlags = in->readByte();
		if (version > 1) {
			_fallVelocity = in->readSint16LE();
			_fallAcceleration = in->readSint16LE();
			_fallPosition = in->readSint16LE();
		} else {
			_fallVelocity = _fallAcceleration = _fallPosition = 0;
		}
		if (version > 2) {
			_dragonBaseFrame = in->readByte();
			_dragonStepCycle = in->readByte();
			_dragonMoveType = in->readByte();
		} else {
			_dragonBaseFrame = _dragonStepCycle = _dragonMoveType = 0;
		}

		_frameNumber = in->readSint32LE();


		setTileDirectionsSize(in->readSint32LE(), true);
		for (i = 0; i < _tileDirectionsAlloced; i++) {
			_tileDirections[i] = in->readByte();
		}

		setWalkStepsPointsSize(in->readSint32LE(), true);
		for (i = 0; i < _walkStepsAlloced; i++) {
			_walkStepsPoints[i].x = in->readSint16LE();
			_walkStepsPoints[i].y = in->readSint16LE();
		}

		_walkStepsCount = in->readSint32LE();
		_walkStepIndex = in->readSint32LE();
		_finalTarget.loadState(in);
		_partialTarget.loadState(in);
		_walkFrameSequence = in->readSint32LE();
	}

	void setTileDirectionsSize(int size, bool forceRealloc) {
		if ((size <= _tileDirectionsAlloced) && !forceRealloc) {
			return;
		}
		_tileDirectionsAlloced = size;
		_tileDirections = (byte*)realloc(_tileDirections, _tileDirectionsAlloced * sizeof(*_tileDirections));
	}

	void cycleWrap(int cycleLimit) {
		if (_actionCycle >= cycleLimit)
			_actionCycle = 0;
	}

	void setWalkStepsPointsSize(int size, bool forceRealloc) {
		if ((size <= _walkStepsAlloced) && !forceRealloc) {
			return;
		}
		_walkStepsAlloced = size;
		_walkStepsPoints = (Point*)realloc(_walkStepsPoints, _walkStepsAlloced * sizeof(*_walkStepsPoints));
	}

	void addWalkStepPoint(const Point &point) {
		setWalkStepsPointsSize(_walkStepsCount + 1, false);
		_walkStepsPoints[_walkStepsCount++] = point;
	}

	void freeSpriteList() {
		_spriteList.freeMem();
	}

	ActorData() {
		memset(this, 0, sizeof(*this));
	}
	~ActorData() {
		if (!_shareFrames)
			free(_frames);
		free(_tileDirections);
		free(_walkStepsPoints);
		freeSpriteList();
	}
};

struct ProtagStateData {
	ActorFrameSequence *_frames;	// Actor's frames
	int	_framesCount;			// Actor's frames count
};


struct SpeechData {
	int speechColor[ACTOR_SPEECH_ACTORS_MAX];
	int outlineColor[ACTOR_SPEECH_ACTORS_MAX];
	int speechFlags;
	const char *strings[ACTOR_SPEECH_STRING_MAX];
	Rect speechBox;
	Rect drawRect;
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

	FontEffectFlags getFontFlags(int i) {
		if (outlineColor[i] != 0) {
			return kFontOutline;
		} else {
			return kFontNormal;
		}
	}
};



class Actor {
	friend class IsoMap;
	friend class SagaEngine;
	friend class Puzzle;
public:

	Actor(SagaEngine *vm);
	~Actor();

	void cmdActorWalkTo(int argc, const char **argv);

	bool validActorId(uint16 id) { return (id == ID_PROTAG) || ((id >= objectIndexToId(kGameObjectActor, 0)) && (id < objectIndexToId(kGameObjectActor, _actorsCount))); }
	int actorIdToIndex(uint16 id) { return (id == ID_PROTAG ) ? 0 : objectIdToIndex(id); }
	uint16 actorIndexToId(int index) { return (index == 0 ) ? ID_PROTAG : objectIndexToId(kGameObjectActor, index); }
	ActorData *getActor(uint16 actorId);
	ActorData *getFirstActor() { return _actors[0]; }

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
		return (objectType == kGameObjectObject) ? getObj(id)->_scriptEntrypointNumber : getActor(id)->_scriptEntrypointNumber;
	}
	int getObjectFlags(uint16 id) {
		int objectType;
		objectType = objectTypeId(id);
		if (!(objectType & (kGameObjectObject | kGameObjectActor))) {
			error("Actor::getObjectFlags wrong id 0x%X", id);
		}
		return (objectType == kGameObjectObject) ? getObj(id)->_flags : getActor(id)->_flags;
	}

	void direct(int msec);
	void drawActors();
	void updateActorsScene(int actorsEntrance);			// calls from scene loading to update Actors info

	void drawSpeech();

#ifdef ACTOR_DEBUG
	void drawPathTest();
#endif

	uint16 hitTest(const Point &testPoint, bool skipProtagonist);
	void takeExit(uint16 actorId, const HitZone *hitZone);
	bool actorEndWalk(uint16 actorId, bool recurse);
	bool actorWalkTo(uint16 actorId, const Location &toLocation);
	int getFrameType(ActorFrameTypes frameType);
	ActorFrameRange *getActorFrameRange(uint16 actorId, int frameType);
	void actorFaceTowardsPoint(uint16 actorId, const Location &toLocation);
	void actorFaceTowardsObject(uint16 actorId, uint16 objectId);

	void realLocation(Location &location, uint16 objectId, uint16 walkFlags);

//	speech
	void actorSpeech(uint16 actorId, const char **strings, int stringsCount, int sampleResourceId, int speechFlags);
	void nonActorSpeech(const Common::Rect &box, const char **strings, int stringsCount, int sampleResourceId, int speechFlags);
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

	int isForcedTextShown() {
		return _activeSpeech.speechFlags & kSpeakForceText;
	}

	void saveState(Common::OutSaveFile *out);
	void loadState(Common::InSaveFile *in);

	void setProtagState(int state);
	int getProtagState() { return _protagState; }

	void freeProtagStates();

	void freeActorList();
	void loadActorList(int protagonistIdx, int actorCount, int actorsResourceID,
				  int protagStatesCount, int protagStatesResourceID);
	void freeObjList();
	void loadObjList(int objectCount, int objectsResourceID);

protected:
	friend class Script;
	bool loadActorResources(ActorData *actor);
	void loadFrameList(int frameListResourceId, ActorFrameSequence *&framesPointer, int &framesCount);
private:
	void stepZoneAction(ActorData *actor, const HitZone *hitZone, bool exit, bool stopped);
	void loadActorSpriteList(ActorData *actor);

	void createDrawOrderList();
	bool calcScreenPosition(CommonObjectData *commonObjectData);
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
#ifdef ACTOR_DEBUG
		if (!validPathCellPoint(testPoint)) {
			error("Actor::setPathCell wrong point");
		}
#endif
		_pathCell[testPoint.x + testPoint.y * _xCellCount] = value;
	}
	int8 getPathCell(const Point &testPoint) {
#ifdef ACTOR_DEBUG
		if (!validPathCellPoint(testPoint)) {
			error("Actor::getPathCell wrong point");
		}
#endif
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
	void moveDragon(ActorData *actor);


protected:
//constants
	int _actorsCount;
	ActorData **_actors;

	int _objsCount;
	ObjectData **_objs;

	SagaEngine *_vm;
	ResourceContext *_actorContext;

	int _lastTickMsec;
	CommonObjectOrderList _drawOrderList;

//variables
public:
	ActorData *_centerActor;
	ActorData *_protagonist;
	int _handleActionDiv;

	Rect _speechBoxScript;

	StringsTable _objectsStrings;
	StringsTable _actorsStrings;

protected:
	SpeechData _activeSpeech;
	int _protagState;
	bool _dragonHunt;

private:
	ProtagStateData *_protagStates;
	int _protagStatesCount;

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

} // End of namespace Saga

#endif
