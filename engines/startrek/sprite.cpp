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

#include "startrek/common.h"
#include "startrek/sprite.h"

namespace StarTrek {

Sprite::Sprite() :
	pos(), drawPriority(0), drawPriority2(0), field8(""),
	bitmap(nullptr), drawMode(0), textColor(0), bitmapChanged(false),
	rect2Valid(false), isOnScreen(false), field16(false), lastDrawRect(),
	drawRect(), rectangle2(), drawX(0), drawY(0)
{}

Sprite::~Sprite() {
	delete bitmap;
	bitmap = nullptr;
}

void Sprite::setBitmap(Bitmap *b) {
	if (bitmap)
		delete bitmap;
	bitmap = b;
	bitmapChanged = true;
}

void Sprite::setBitmap(Common::MemoryReadStreamEndian *stream) {
	setBitmap(new Bitmap(stream));
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

Common::Rect Sprite::getRect() {
	Common::Rect rect(bitmap->width, bitmap->height);
	rect.translate(pos.x - bitmap->xoffset, pos.y - bitmap->yoffset);
	return rect;
}

void Sprite::saveLoadWithSerializer(Common::Serializer &ser) {
	ser.syncAsSint16LE(pos.x);
	ser.syncAsSint16LE(pos.y);
	ser.syncAsUint16LE(drawPriority);
	ser.syncAsUint16LE(drawPriority2);
	ser.syncString(field8);
	// Note: bitmap must be reloaded
	ser.syncAsUint16LE(drawMode);
	ser.syncAsUint16LE(textColor);
	ser.syncAsUint16LE(bitmapChanged);
	ser.syncAsUint16LE(rect2Valid);
	ser.syncAsUint16LE(isOnScreen);
	ser.syncAsUint16LE(field16);
	serializeRect(lastDrawRect, ser);
	serializeRect(drawRect, ser);
	serializeRect(rectangle2, ser);
	ser.syncAsSint16LE(drawX);
	ser.syncAsSint16LE(drawY);
}

} // End of namespace StarTrek
