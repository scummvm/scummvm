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

#include "illusions/actorresource.h"
#include "illusions/backgroundresource.h"
#include "illusions/graphics.h"
#include "common/algorithm.h"
#include "common/list.h"
#include "graphics/surface.h"

namespace Illusions {

class IllusionsEngine;

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

class Actor {
public:
	Actor(IllusionsEngine *vm);
	void pause();
	void unpause();
	void createSurface(SurfInfo &surfInfo);
	void destroySurface();
	void initSequenceStack();
	void pushSequenceStack(int16 value);
	int16 popSequenceStack();
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
	
	ScaleLayer *_scaleLayer;
	PriorityLayer *_priorityLayer;
	
	uint _seqStackCount;
	int16 _seqStack[5];
	
	Common::Point _position;
	Common::Point _position2;
	uint _facing;
	
	uint32 _fontId;
	
	DefaultSequences _defaultSequences;

	uint32 _parentObjectId;
	int _linkIndex;
	int _linkIndex2;
	uint32 _subobjects[kSubObjectsCount];
	
	uint32 _notifyThreadId1;
	uint32 _notifyId3C;

	uint32 _notifyThreadId2;
	int _field30;
	
	int _surfaceTextFlag;
	
	byte *_seqCodeIp;
	uint32 _sequenceId;
	int _seqCodeValue1;
	int _seqCodeValue2;
	int _seqCodeValue3;
	
	int _pathCtrY;
	int _path40;
	

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
	int getPriority();
	Common::Point calcPosition(Common::Point posDelta);
	uint32 getSubActorParent();
	void getCollisionRectAccurate(Common::Rect &collisionRect);
	void setActorUsePan(int usePan);
	void setActorFrameIndex(int16 frameIndex);
	void stopActor();
	void startSequenceActor(uint32 sequenceId, int value, uint32 notifyThreadId);
	void stopSequenceActor();
	void sequenceActor();
public:
	IllusionsEngine *_vm;
	uint _flags;
	int _pauseCtr;
	int16 _priority;
	Actor *_actor;
	//field_6 dw
	uint32 _tag;
	uint32 _objectId;
	uint32 _actorTypeId;
	// TODO Move points into own struct
	Common::Point _unkPt;
	Common::Point _pt;
	Common::Point _feetPt;
	Common::Point _position;
	Common::Point _subobjectsPos[kSubObjectsCount];
	// TODO 0000001C - 00000054 unknown
	void startSequenceActorIntern(uint32 sequenceId, int value, int value2, uint32 notifyThreadId);
};

class Controls {
public:
	Controls(IllusionsEngine *vm);
	void placeActor(uint32 actorTypeId, Common::Point placePt, uint32 sequenceId, uint32 objectId, uint32 notifyThreadId);
	void placeSequenceLessActor(uint32 objectId, Common::Point placePt, WidthHeight dimensions, int16 priority);
	void placeActorLessObject(uint32 objectId, Common::Point feetPt, Common::Point pt, int16 priority, uint flags);	
public:
	typedef Common::List<Control*> Items;
	typedef Items::iterator ItemsIterator;
	IllusionsEngine *_vm;
	Items _controls;
	Actor *newActor();
	Control *newControl();
	void destroyControl(Control *control);
};

} // End of namespace Illusions

#endif // ILLUSIONS_ACTOR_H
