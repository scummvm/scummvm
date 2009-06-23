/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#include "engines/grim/gfx_base.h"
#include "engines/grim/primitives.h"
#include "engines/grim/savegame.h"
#include "engines/grim/lua.h"

namespace Grim {

PrimitiveObject::PrimitiveObject() {
	memset(&_color, 0, sizeof(Color));
	_filled = false;
	_type = 0;
	_bitmap = NULL;
}

PrimitiveObject::~PrimitiveObject() {
	if (_type == 2)
		g_driver->destroyBitmap(_bitmap);
}

void PrimitiveObject::saveState(SaveGame *savedState) {
	PointerId ptr;

	savedState->writeLESint32(_type);
	savedState->writeLEUint32(_color.red());
	savedState->writeLEUint32(_color.green());
	savedState->writeLEUint32(_color.blue());
	savedState->writeLEUint32(_filled);
	ptr = makeIdFromPointer(_bitmap);
	savedState->writeLEUint32(ptr.low);
	savedState->writeLEUint32(ptr.hi);
	savedState->writeLEUint32(_p1.x);
	savedState->writeLEUint32(_p1.y);
	savedState->writeLEUint32(_p2.x);
	savedState->writeLEUint32(_p2.y);
	savedState->writeLEUint32(_p3.x);
	savedState->writeLEUint32(_p3.y);
	savedState->writeLEUint32(_p4.x);
	savedState->writeLEUint32(_p4.y);
}

void PrimitiveObject::createRectangle(Common::Point p1, Common::Point p2, Color color, bool filled) {
	_type = RECTANGLE;
	_p1 = p1;
	_p2 = p2;
	_color = color;
	_filled = filled;
}

void PrimitiveObject::createBitmap(Bitmap *bitmap, Common::Point p, bool /*transparent*/) {
	_type = BITMAP;
	_bitmap = bitmap;
	_bitmap->setX(p.x);
	_bitmap->setY(p.y);
	// transparent: what to do ?
	g_driver->createBitmap(_bitmap);
}

void PrimitiveObject::createLine(Common::Point p1, Common::Point p2, Color color) {
	_type = LINE;
	_p1 = p1;
	_p2 = p2;
	_color = color;
}

void PrimitiveObject::createPolygon(Common::Point p1, Common::Point p2, Common::Point p3, Common::Point p4, Color color) {
	_type = POLYGON;
	_p1 = p1;
	_p2 = p2;
	_p3 = p3;
	_p4 = p4;
	_color = color;
}

void PrimitiveObject::draw() {
	assert(_type);

	if (_type == RECTANGLE)
		g_driver->drawRectangle(this);
	else if (_type == BITMAP)
		g_driver->drawBitmap(_bitmap);
	else if (_type == LINE)
		g_driver->drawLine(this);
	else if (_type == POLYGON)
		g_driver->drawPolygon(this);
}

void PrimitiveObject::setPos(int x, int y) {
	if (x != -1) {
		int dx = x - _p1.x;
		_p1.x += dx;
		if (_type == RECTANGLE || _type == LINE || _type == POLYGON)
			_p2.x += dx;
		if (_type == POLYGON) {
			_p3.x += dx;
			_p4.x += dx;
		}
	}
	if (y != -1) {
		int dy = y - _p1.y;
		_p1.y += dy;
		if (_type == RECTANGLE || _type == LINE || _type == POLYGON)
			_p2.y += dy;
		if (_type == POLYGON) {
			_p3.y += dy;
			_p4.y += dy;
		}
	}
}

} // end of namespace Grim
