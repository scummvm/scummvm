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
 */

#ifndef ILLUSIONS_ACTOR_H
#define ILLUSIONS_ACTOR_H

#include "illusions/resources/actorresource.h"
#include "illusions/resources/backgroundresource.h"
#include "illusions/graphics.h"
#include "illusions/pathfinder.h"
#include "common/algorithm.h"
#include "common/func.h"
#include "common/list.h"
#include "graphics/surface.h"

namespace Illusions {

class Control;
class IllusionsEngine;
class SequenceOpcodes;
struct OpCall;

enum ActorFlags {
	ACTOR_FLAG_IS_VISIBLE = 1,
	ACTOR_FLAG_HAS_WALK_POINTS = 2,
	ACTOR_FLAG_SCALED = 4,
	ACTOR_FLAG_PRIORITY = 8,
	ACTOR_FLAG_HAS_WALK_RECTS = 0x10,
	ACTOR_FLAG_REGION = 0x20,
	ACTOR_FLAG_40 = 0x40,
	ACTOR_FLAG_80 = 0x80,
	ACTOR_FLAG_100 = 0x100,
	ACTOR_FLAG_200 = 0x200,
	ACTOR_FLAG_400 = 0x400,
	ACTOR_FLAG_800 = 0x800,
	ACTOR_FLAG_1000 = 0x1000,
	ACTOR_FLAG_2000 = 0x2000,
	ACTOR_FLAG_4000 = 0x4000,
	ACTOR_FLAG_8000 = 0x8000
};

enum ControlObjectID {
	CURSOR_OBJECT_ID = 0x40004
};

const uint kSubObjectsCount = 15;

struct DefaultSequence {
	uint32 _sequenceId;
	uint32 _newSequenceId;
	DefaultSequence()
		: _sequenceId(0), _newSequenceId(0) {}
	DefaultSequence(uint32 sequenceId, uint32 newSequenceId)
		: _sequenceId(sequenceId), _newSequenceId(newSequenceId) {}
};

class DefaultSequences {
public:
	uint32 use(uint32 sequenceId);
	void set(uint32 sequenceId, uint32 newSequenceId);
protected:
	typedef Common::Array<DefaultSequence> Items;
	typedef Items::iterator ItemsIterator;
	struct DefaultSequenceEqual : public Common::UnaryFunction<const DefaultSequence&, bool> {
		uint32 _sequenceId;
		DefaultSequenceEqual(uint32 sequenceId) : _sequenceId(sequenceId) {}
		bool operator()(const DefaultSequence &defaultSequence) const {
			return defaultSequence._sequenceId == _sequenceId;
		}
	};
	Common::Array<DefaultSequence> _items;
};

typedef Common::Functor2<Control*, uint32, void> ActorControlRoutine;

class Actor {
public:
	Actor(IllusionsEngine *vm);
	~Actor();
	void pause();
	void unpause();
	void createSurface(SurfInfo &surfInfo);
	void destroySurface();
	void initSequenceStack();
	void pushSequenceStack(int16 value);
	int16 popSequenceStack();
	void setControlRoutine(ActorControlRoutine *controlRoutine);
	void runControlRoutine(Control *control, uint32 deltaTime);
	bool findNamedPoint(uint32 namedPointId, Common::Point &pt);
public:
	IllusionsEngine *_vm;
	byte _drawFlags;
	uint _spriteFlags;

	int _pauseCtr;
	uint _flags;

	int _scale;
	int16 _frameIndex;
	int16 _newFrameIndex;
	SurfInfo _surfInfo;
	Graphics::Surface *_surface;

	FramesList *_frames;
	NamedPoints *_namedPoints;

	ScaleLayer *_scaleLayer;
	PriorityLayer *_priorityLayer;
	RegionLayer *_regionLayer;
	PathWalkPoints *_pathWalkPoints;
	PathWalkRects *_pathWalkRects;

	uint _seqStackCount;
	int16 _seqStack[5];

	Common::Point _position;
	Common::Point _position2;
	uint _facing;
	int _regionIndex;

	uint32 _fontId;
	int16 _actorIndex;

	DefaultSequences _defaultSequences;

	uint32 _parentObjectId;
	int _linkIndex;
	int _linkIndex2;
	uint32 _subobjects[kSubObjectsCount];

	uint32 _notifyThreadId1;
	uint32 _notifyId3C;

	uint32 _notifyThreadId2;
	byte *_entryTblPtr;

	ActorControlRoutine *_controlRoutine;

	uint32 _sequenceId;
	int _seqCodeValue2;
	byte *_seqCodeIp;
	int _seqCodeValue1;
	int _seqCodeValue3;

	int _pathCtrX, _pathCtrY;
	int _pathAngle;
	int32 _posXShl, _posYShl;
	uint _pathPointIndex;
	uint _pathPointsCount;
	Common::Point _pathInitialPos;
	bool _pathInitialPosFlag;
	bool _pathFlag50;
	PointArray *_pathNode;
	uint _pathPoints;
	uint32 _walkCallerThreadId1;

	RGB _color;
	int16 _choiceJumpOffs;

};

class Control {
public:
	Control(IllusionsEngine *vm);
	~Control();
	void pause();
	void unpause();
	void appearActor();
	void disappearActor();
	bool isActorVisible();
	void activateObject();
	void deactivateObject();
	void readPointsConfig(byte *pointsConfig);
	void setActorPosition(Common::Point position);
	Common::Point getActorPosition();
	void setActorScale(int scale);
	void faceActor(uint facing);
	void linkToObject(uint32 parentObjectId, uint32 linkedObjectValue);
	void unlinkObject();
	void clearNotifyThreadId1();
	void clearNotifyThreadId2();
	void setPriority(int16 priority);
	uint32 getOverlapPriority();
	uint32 getDrawPriority();
	uint32 getPriority();
	Common::Point calcPosition(Common::Point posDelta);
	uint32 getSubActorParent();
	void getCollisionRectAccurate(Common::Rect &collisionRect);
	void getCollisionRect(Common::Rect &collisionRect);
	void setActorUsePan(int usePan);
	void setActorFrameIndex(int16 frameIndex);
	void stopActor();
	void startSequenceActor(uint32 sequenceId, int value, uint32 notifyThreadId);
	void stopSequenceActor();
	void startTalkActor(uint32 sequenceId, byte *entryTblPtr, uint32 threadId);
	void sequenceActor();
	void setActorIndex(int actorIndex);
	void setActorIndexTo1();
	void setActorIndexTo2();
	void startSubSequence(int linkIndex, uint32 sequenceId);
	void stopSubSequence(int linkIndex);
	void startMoveActor(uint32 sequenceId, Common::Point destPt, uint32 callerThreadId1, uint32 callerThreadId2);
	PointArray *createPath(Common::Point destPt);
	void updateActorMovement(uint32 deltaTime);
	void refreshSequenceCode();
	void getActorFrameDimensions(WidthHeight &dimensions);
	void drawActorRect(const Common::Rect r, byte color);
	void fillActor(byte color);
	bool isPixelCollision(Common::Point &pt);
public:
	IllusionsEngine *_vm;
	uint _flags;
	int _pauseCtr;
	int16 _priority;
	Actor *_actor;
	uint32 _sceneId;
	uint32 _objectId;
	uint32 _actorTypeId;
	WRect _bounds;
	Common::Point _feetPt;
	Common::Point _position;
	Common::Point _subobjectsPos[kSubObjectsCount];
	void startSequenceActorIntern(uint32 sequenceId, int value, byte *entryTblPtr, uint32 notifyThreadId);
	void execSequenceOpcode(OpCall &opCall);
};

class Controls {
public:
	Controls(IllusionsEngine *vm);
	~Controls();
	void placeBackgroundObject(BackgroundObject *backgroundObject);
	void placeActor(uint32 actorTypeId, Common::Point placePt, uint32 sequenceId, uint32 objectId, uint32 notifyThreadId);
	void placeSequenceLessActor(uint32 objectId, Common::Point placePt, WidthHeight dimensions, int16 priority);
	void placeActorLessObject(uint32 objectId, Common::Point feetPt, Common::Point pt, int16 priority, uint flags);
	void placeSubActor(uint32 objectId, int linkIndex, uint32 actorTypeId, uint32 sequenceId);
	void placeDialogItem(uint16 objectNum, uint32 actorTypeId, uint32 sequenceId, Common::Point placePt, int16 choiceJumpOffs);
	void destroyControls();
	void destroyActiveControls();
	void destroyControlsBySceneId(uint32 sceneId);
	void destroyDialogItems();
	void threadIsDead(uint32 threadId);
	void pauseControls();
	void unpauseControls();
	void pauseControlsBySceneId(uint32 sceneId);
	void unpauseControlsBySceneId(uint32 sceneId);
	bool getOverlappedObject(Control *control, Common::Point pt, Control **outOverlappedControl, int minPriority);
	bool getOverlappedObjectAccurate(Control *control, Common::Point pt, Control **outOverlappedControl, int minPriority);
	bool getDialogItemAtPos(Control *control, Common::Point pt, Control **outOverlappedControl);
	bool getOverlappedWalkObject(Control *control, Common::Point pt, Control **outOverlappedControl);
	void destroyControl(Control *control);
	bool findNamedPoint(uint32 namedPointId, Common::Point &pt);
	void actorControlRoutine(Control *control, uint32 deltaTime);
	void dialogItemControlRoutine(Control *control, uint32 deltaTime);
	void disappearActors();
	void appearActors();
	void pauseActors(uint32 objectId);
	void unpauseActors(uint32 objectId);
public:
	typedef Common::List<Control*> Items;
	typedef Items::iterator ItemsIterator;
	IllusionsEngine *_vm;
	Items _controls;
	SequenceOpcodes *_sequenceOpcodes;
	uint32 _nextTempObjectId;
	Actor *newActor();
	Control *newControl();
	uint32 newTempObjectId();
	void destroyControlInternal(Control *control);
};

} // End of namespace Illusions

#endif // ILLUSIONS_ACTOR_H
