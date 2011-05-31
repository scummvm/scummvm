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

#ifndef TOON_CHARACTER_H
#define TOON_CHARACTER_H

#include "toon/toon.h"

namespace Toon {

class ToonEngine;

struct SpecialCharacterAnimation {
	char _filename[9]; // 0
	byte _flag1;       // 9
	short _offsetX;    // 10
	short _offsetY;    // 12
	short _unused;     // 14
	short _unused2;    // 16
	byte _flags2;      // 18
	byte _flags3;      // 19
	byte _flags4;      // 20
	byte _flags5;      // 21
	byte _flags6;      // 22
	byte _flags7;      // 23
	byte _flags8;      // 24
	byte _flags9;      // 25
};

class Character {
public:
	Character(ToonEngine *vm);
	virtual ~Character();
	virtual void init();
	virtual int32 getId();
	virtual void setId(int32 id);
	virtual void setFacing(int32 facing);
	virtual void forceFacing(int32 facing);
	virtual int32 getFacing();
	virtual void setAnimScript(int32 animScriptId);
	virtual void setSceneAnimationId(int32 sceneAnimationId);
	virtual void setDefaultSpecialAnimationId(int32 defaultAnimationId);
	virtual int32 getAnimScript();
	virtual int32 getSceneAnimationId();
	virtual void setFlag(int flag);
	virtual int32 getFlag();
	virtual int32 getAnimFlag();
	virtual void setAnimFlag(int32 flag);
	virtual void setPosition(int32 x, int32 y);
	virtual void forcePosition(int32 x, int32 y);
	virtual int32 getX();
	virtual int32 getY();
	virtual int32 getFinalX();
	virtual int32 getFinalY();
	virtual bool walkTo(int32 newPosX, int32 newPosY);
	virtual bool getVisible();
	virtual void setVisible(bool visible);
	virtual bool loadWalkAnimation(Common::String animName);
	virtual bool loadIdleAnimation(Common::String animName);
	virtual bool loadTalkAnimation(Common::String animName);
	virtual bool loadShadowAnimation(Common::String animName);
	virtual bool setupPalette();
	virtual void playStandingAnim();
	virtual void playWalkAnim(int32 start, int32 end);
	virtual void playTalkAnim();
	virtual void playAnim(int32 animId, int32 unused, int32 flags);
	virtual void update(int32 timeIncrement);
	virtual int32 getScale();
	virtual AnimationInstance *getAnimationInstance();
	virtual void setAnimationInstance(AnimationInstance *instance);
	virtual void save(Common::WriteStream *stream);
	virtual void load(Common::ReadStream *stream);
	virtual void stopWalk();
	virtual void stopSpecialAnim();
	virtual void updateIdle();
	virtual int32 getRandomIdleAnim() { return 0; }
	virtual void updateTimers(int32 relativeAdd);
	virtual void setTalking(bool talking) { _isTalking = talking; }
	virtual bool isTalking() { return _isTalking; }
	virtual void resetScale() { };
	virtual void plotPath(Graphics::Surface& surface);

	int32 getFacingFromDirection(int32 dx, int32 dy);
	static const SpecialCharacterAnimation *getSpecialAnimation(int32 characterId, int32 animationId);

protected:
	ToonEngine *_vm;

	int32 _id;
	int32 _animScriptId;
	int32 _animSpecialId;
	int32 _animSpecialDefaultId;
	int32 _sceneAnimationId;
	int32 _lineToSayId;
	int32 _time;
	int32 _x;
	int32 _y;
	int32 _z;
	int32 _finalX;
	int32 _finalY;
	int32 _facing;
	int32 _flags;
	int32 _animFlags;
	int32 _scale;
	int32 _nextIdleTime;
	bool _visible;
	bool _blockingWalk;
	int32 _speed;
	int32 _lastWalkTime;
	int32 _numPixelToWalk;
	bool _isTalking;

	AnimationInstance *_animationInstance;
	AnimationInstance *_shadowAnimationInstance;
	Animation *_walkAnim;
	Animation *_idleAnim;
	Animation *_talkAnim;
	Animation *_shadowAnim;
	Animation *_specialAnim;

	int32 _currentPathX[4096];
	int32 _currentPathY[4096];
	int32 _currentPathNodeCount;
	int32 _currentPathNode;
	int32 _currentWalkStamp;
};

} // End of namespace Toon
#endif
