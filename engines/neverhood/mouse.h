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

#ifndef NEVERHOOD_MOUSE_H
#define NEVERHOOD_MOUSE_H

#include "neverhood/neverhood.h"
#include "neverhood/sprite.h"
#include "neverhood/graphics.h"
#include "neverhood/resource.h"

namespace Neverhood {

class Mouse433 : public StaticSprite {
public:
	Mouse433(NeverhoodEngine *vm, uint32 fileHash, NRect *mouseRect);
	void load(uint32 fileHash);
	void updateCursor();
protected:
	MouseCursorResource _mouseCursorResource;
	int _frameNum;
	NRect _mouseRect;	
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class Mouse435 : public StaticSprite {
public:
	Mouse435(NeverhoodEngine *vm, uint32 fileHash, int16 x1, int16 x2);
	void load(uint32 fileHash);
	void updateCursor();
protected:
	MouseCursorResource _mouseCursorResource;
	int _frameNum;
	int16 _x1;
	int16 _x2;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

class NavigationMouse : public StaticSprite {
public:
	NavigationMouse(NeverhoodEngine *vm, uint32 fileHash, int type);
	void load(uint32 fileHash);
	void updateCursor();
protected:
	MouseCursorResource _mouseCursorResource;
	int _frameNum;
	int _type;
	void update();
	uint32 handleMessage(int messageNum, const MessageParam &param, Entity *sender);
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MOUSE_H */
