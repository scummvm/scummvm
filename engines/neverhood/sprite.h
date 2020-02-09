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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef NEVERHOOD_SPRITE_H
#define NEVERHOOD_SPRITE_H

#include "neverhood/neverhood.h"
#include "neverhood/entity.h"
#include "neverhood/graphics.h"
#include "neverhood/resource.h"

namespace Neverhood {

#define SetSpriteUpdate(callback)											\
	do {																	\
		_spriteUpdateCb = static_cast <void (Sprite::*)(void)> (callback);	\
		debug(2, "SetSpriteUpdate(" #callback ")");							\
		_spriteUpdateCbName = #callback;									\
	} while (0)

#define SetFilterX(callback)												\
	do {																	\
		_filterXCb = static_cast <int16 (Sprite::*)(int16)> (callback);		\
		debug(2, "SetFilterX(" #callback ")");								\
	} while (0)

#define SetFilterY(callback)												\
	do {																	\
		_filterYCb = static_cast <int16 (Sprite::*)(int16)> (callback);		\
		debug(2, "SetFilterY(" #callback ")");								\
	} while (0)

const int16 kDefPosition = -32768;

class Sprite : public Entity {
public:
	Sprite(NeverhoodEngine *vm, int objectPriority);
	~Sprite() override;
	void init() {}
	BaseSurface *getSurface() { return _surface; }
	void updateBounds();
	void setDoDeltaX(int type);
	void setDoDeltaY(int type);
	bool isPointInside(int16 x, int16 y);
	bool checkCollision(NRect &rect);
	int16 getX() const { return _x; }
	int16 getY() const { return _y; }
	void setX(int16 value) { _x = value; }
	void setY(int16 value) { _y = value; }
	uint16 getFlags() const { return _flags; }
	bool isDoDeltaX() const { return _doDeltaX; }
	bool isDoDeltaY() const { return _doDeltaY; }
	NRect& getCollisionBounds() { return _collisionBounds; }
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void loadDataResource(uint32 fileHash);
	int16 defFilterY(int16 y);
	bool getVisible() const { return _surface->getVisible(); }
	void setVisible(bool value) { _surface->setVisible(value); }
	NDrawRect& getDrawRect() { return _surface->getDrawRect(); }
	// Some shortcuts to set the clipRect
	NRect& getClipRect() { return _surface->getClipRect(); }
	void setClipRect(int16 x1, int16 y1, int16 x2, int16 y2);
	void setClipRect(NRect& clipRect);
	void setClipRect(NDrawRect& drawRect);
protected:
	void (Sprite::*_spriteUpdateCb)();
	Common::String _spriteUpdateCbName; // For debugging purposes
	int16 (Sprite::*_filterXCb)(int16);
	int16 (Sprite::*_filterYCb)(int16);
	BaseSurface *_surface;
	int16 _x, _y;
	bool _doDeltaX, _doDeltaY;
	bool _needRefresh;
	NDrawRect _drawOffset;
	NRect _collisionBounds;
	NDrawRect _collisionBoundsOffset;
	uint16 _flags;
	DataResource _dataResource;
	void createSurface(int surfacePriority, int16 width, int16 height);
	void handleSpriteUpdate() {
		if (_spriteUpdateCb)
			(this->*_spriteUpdateCb)();
	}
	int16 filterX(int16 x) {
		return _filterXCb ? (this->*_filterXCb)(x) : x;
	}
	int16 filterY(int16 y) {
		return _filterYCb ? (this->*_filterYCb)(y) : y;
	}
};

enum {
	kSLFDefDrawOffset				= 1 << 0,
	kSLFCenteredDrawOffset			= 1 << 1,
	kSLFDefPosition					= 1 << 2,
	kSLFSetPosition					= 1 << 3,
	kSLFDefCollisionBoundsOffset	= 1 << 4
};

class StaticSprite : public Sprite {
public:
	StaticSprite(NeverhoodEngine *vm, int objectPriority);
	StaticSprite(NeverhoodEngine *vm, uint32 fileHash, int surfacePriority, int16 x = kDefPosition, int16 y = kDefPosition);
	void loadSprite(uint32 fileHash, uint flags = 0, int surfacePriority = 0, int16 x = kDefPosition, int16 y = kDefPosition);
	void updatePosition();
protected:
	SpriteResource _spriteResource;
};

#define AnimationCallback(callback) static_cast <void (AnimatedSprite::*)()> (callback)
#define GotoState(callback) gotoState(static_cast <void (AnimatedSprite::*)()> (callback))
#define NextState(callback)															\
	do {																			\
		_nextStateCb = static_cast <void (AnimatedSprite::*)(void)> (callback);		\
		debug(2, "NextState(" #callback ")"); _nextStateCbName = #callback;			\
	} while (0)
#define FinalizeState(callback) setFinalizeState(static_cast <void (AnimatedSprite::*)()> (callback));

const int STICK_LAST_FRAME = -2;

class AnimatedSprite : public Sprite {
public:
	AnimatedSprite(NeverhoodEngine *vm, int objectPriority);
	AnimatedSprite(NeverhoodEngine *vm, uint32 fileHash, int surfacePriority, int16 x, int16 y);
	void update();
	void updateDeltaXY();
	void setRepl(byte oldColor, byte newColor);
	void clearRepl();
	uint32 getCurrAnimFileHash() const { return _currAnimFileHash; }
	int16 getFrameIndex() const { return _currFrameIndex; }
	int16 getFrameIndex(uint32 frameHash) { return _animResource.getFrameIndex(frameHash); }
	void setNewHashListIndex(int value) { _newStickFrameIndex = value; }
	void startAnimation(uint32 fileHash, int16 plFirstFrameIndex, int16 plLastFrameIndex);
protected:
	typedef void (AnimatedSprite::*AnimationCb)();
	AnimResource _animResource;
	uint32 _currAnimFileHash, _newAnimFileHash, _nextAnimFileHash;
	int16 _currFrameIndex, _lastFrameIndex;
	int16 _plFirstFrameIndex, _plLastFrameIndex;
	uint32 _plFirstFrameHash, _plLastFrameHash;
	int16 _animStatus;
	int16 _currFrameTicks;
	int _currStickFrameIndex, _newStickFrameIndex;
	uint32 _newStickFrameHash;
	int16 _deltaX, _deltaY;
	byte _replOldColor, _replNewColor;
	bool _playBackwards, _frameChanged;
	AnimationCb _finalizeStateCb;
	AnimationCb _currStateCb;
	AnimationCb _nextStateCb;
	// For debugging purposes
	Common::String _finalizeStateCbName;
	Common::String _currStateCbName;
	Common::String _nextStateCbName;
	void init();
	void updateAnim();
	void updatePosition();
	void updateFrameIndex();
	void updateFrameInfo();
	void createSurface1(uint32 fileHash, int surfacePriority);
	void createShadowSurface1(BaseSurface *shadowSurface, uint32 fileHash, int surfacePriority);
	void createShadowSurface(BaseSurface *shadowSurface, int16 width, int16 height, int surfacePriority);
	void stopAnimation();
	void startAnimationByHash(uint32 fileHash, uint32 plFirstFrameHash, uint32 plLastFrameHash);
	void nextAnimationByHash(uint32 fileHash2, uint32 plFirstFrameHash, uint32 plLastFrameHash);
	void setFinalizeState(AnimationCb finalizeStateCb);
	void gotoState(AnimationCb currStateCb);
	void gotoNextState();
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_SPRITE_H */
