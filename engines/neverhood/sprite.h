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

#define SetSpriteCallback(callback) _spriteCallbackCb = static_cast <void (Sprite::*)(void)> (callback)
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
protected:
	void (Sprite::*_spriteUpdateCb)();
	int16 (Sprite::*_filterXCb)(int16);
	int16 (Sprite::*_filterYCb)(int16);
	BaseSurface *_surface;
	int16 _x, _y;
	bool _doDeltaX, _doDeltaY;
	bool _needRedraw;
	//0000002B field_2B		db ?
	//0000002C field2C		dd ? // unused
	NRect _rect1;
	int16 _deltaX1, _deltaY1;
	int16 _deltaX2, _deltaY2;
	NRect _rect;
	uint16 _flags;
	//0000004A field4A		dw ? // seems to be unused except in ctor
	//0000004C rectResource	RectResource ?
	//void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
	void createSurface(int surfacePriority, int16 width, int16 height);
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
	StaticSprite(NeverhoodEngine *vm, const char *filename, int surfacePriority, int16 x, int16 y, int16 width, int16 height);
	StaticSprite(NeverhoodEngine *vm, uint32 fileHash, int surfacePriority, int16 x, int16 y, int16 width, int16 height);
	void load(uint32 fileHash, bool dimensions, bool position);
protected:
	SpriteResource _spriteResource;
	void init(uint32 fileHash, int surfacePriority, int16 x, int16 y, int16 width, int16 height);
	void update();
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_SPRITE_H */
