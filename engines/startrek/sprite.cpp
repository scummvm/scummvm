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

#include "startrek/sprite.h"

namespace StarTrek {

Sprite::Sprite() :
	pos(),
	drawPriority(),
	drawPriority2(),
	field8(),
	bitmap(),
	drawMode(),
	textColor(),
	bitmapChanged(),
	rect2Valid(),
	isOnScreen(),
	field16(),
	lastDrawRect(),
	drawRect(),
	rectangle2(),
	drawX(),
	drawY()
	{}

void Sprite::setBitmap(SharedPtr<Bitmap> b) {
	bitmap = b;
	bitmapChanged = true;
}

void Sprite::setXYAndPriority(int16 x, int16 y, int16 priority) {
	pos.x = x;
	pos.y = y;
	drawPriority = priority;
	bitmapChanged = true;
}

void Sprite::dontDrawNextFrame() {
	field16 = true;
	bitmapChanged = true;
}
/**
 * Returns a rect containing the sprite's bitmap on the screen.
 */
Common::Rect Sprite::getRect() {
	Common::Rect rect(bitmap->width, bitmap->height);
	rect.translate(pos.x - bitmap->xoffset, pos.y - bitmap->yoffset);
	return rect;
}

}
