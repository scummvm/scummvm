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

#ifndef NEVERHOOD_SPRITE_H
#define NEVERHOOD_SPRITE_H

#include "neverhood/neverhood.h"
#include "neverhood/entity.h"
#include "neverhood/graphics.h"
#include "neverhood/resource.h"

namespace Neverhood {

#define SetSpriteCallback(callback) _spriteUpdateCb = static_cast <void (Sprite::*)(void)> (callback)
#define SetFilterX(callback) _filterXCb = static_cast <int16 (Sprite::*)(int16)> (callback)
#define SetFilterY(callback) _filterYCb = static_cast <int16 (Sprite::*)(int16)> (callback)

const int16 kDefPosition = -32768;

class Sprite : public Entity {
public:
	Sprite(NeverhoodEngine *vm, int objectPriority);
	~Sprite();
	BaseSurface *getSurface() { return _surface; }
	void processDelta();
	void setDoDeltaX(int type);
	void setDoDeltaY(int type);
	bool isPointInside(int16 x, int16 y);
	int16 getX() const { return _x; }
	int16 getY() const { return _y; }
protected:
	void (Sprite::*_spriteUpdateCb)();
	int16 (Sprite::*_filterXCb)(int16);
	int16 (Sprite::*_filterYCb)(int16);
	BaseSurface *_surface;
	int16 _x, _y;
	bool _doDeltaX, _doDeltaY;
	bool _needRefresh;
	//0000002B field_2B		db ?
	//0000002C field2C		dd ? // unused
	NDrawRect _drawRect;
	NDrawRect _deltaRect;
	NRect _rect;
	uint16 _flags;
	//0000004A field4A		dw ? // seems to be unused except in ctor
	//0000004C rectResource	RectResource ?
	//void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
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

class StaticSprite : public Sprite {
public:
	StaticSprite(NeverhoodEngine *vm, int objectPriority);
	StaticSprite(NeverhoodEngine *vm, const char *filename, int surfacePriority, int16 x = kDefPosition, int16 y = kDefPosition, int16 width = 0, int16 height = 0);
	StaticSprite(NeverhoodEngine *vm, uint32 fileHash, int surfacePriority, int16 x = kDefPosition, int16 y = kDefPosition, int16 width = 0, int16 height = 0);
	void load(uint32 fileHash, bool dimensions, bool position);
protected:
	SpriteResource _spriteResource;
	void init(uint32 fileHash, int surfacePriority, int16 x, int16 y, int16 width, int16 height);
	void update();
};

#define SetAnimationCallback1(callback) _callback1Cb = static_cast <void (AnimatedSprite::*)(void)> (callback)
#define SetAnimationCallback2(callback) _callback2Cb = static_cast <void (AnimatedSprite::*)(void)> (callback)
#define SetAnimationCallback3(callback) _callback3Cb = static_cast <void (AnimatedSprite::*)(void)> (callback)
#define AnimationCallback(callback) static_cast <void (AnimatedSprite::*)()> (callback)

class AnimatedSprite : public Sprite {
public:
	AnimatedSprite(NeverhoodEngine *vm, int objectPriority);
	AnimatedSprite(NeverhoodEngine *vm, uint32 fileHash, int surfacePriority, int16 x, int16 y);
	void update();
	void updateDeltaXY();
protected:
	typedef void (AnimatedSprite::*AnimationCb)();
	AnimResource _animResource;
	uint32 _fileHash1;
	uint32 _fileHash2;
	uint32 _fileHash3;
	int16 _frameIndex;
	int16 _frameIndex3;
	int16 _frameIndex2;
	int16 _frameIndex4;
	uint32 _fileHash6;
	uint32 _fileHash5;
	int16 _animStatus;
	int16 _counter;
	int _hashListIndex;
	int _newHashListIndex;
	uint32 _fileHash4;
	int16 _deltaX, _deltaY;
	byte _replOldByte;
	byte _replNewByte;
	bool _playBackwards;
	bool _flag;
	/* TODO
	callbackListIndex dw ?
	callbackListCount dw ?
	callbackList	dd ?
	*/
	AnimationCb _callback1Cb;
	AnimationCb _callback2Cb;
	AnimationCb _callback3Cb;
	void init();
	void updateAnim();
	void updatePosition();
	void updateFrameIndex();
	void updateFrameInfo();
	void createSurface1(uint32 fileHash, int surfacePriority);
	void setFileHash(uint32 fileHash, int16 frameIndex3, int16 frameIndex4);
	void setFileHash1();
	void setFileHash2(uint32 fileHash, uint32 fileHash6, uint32 fileHash5);
	void setFileHash3(uint32 fileHash2, uint32 fileHash6, uint32 fileHash5);
	void setCallback1(AnimationCb callback1);
	void setCallback2(AnimationCb callback2);
	void removeCallbacks();
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_SPRITE_H */
