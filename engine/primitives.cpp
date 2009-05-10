/* Residual - Virtual machine to run LucasArts' 3D adventure games
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

#include "engine/gfx_base.h"
#include "engine/primitives.h"

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

void PrimitiveObject::createRectangle(Common::Point p1, Common::Point p2, Color color, bool filled) {
	_p1 = p1;
	_p2 = p2;
	_color = color;
	_filled = filled;
	_type = 1;
}

void PrimitiveObject::createBitmap(Bitmap *bitmap, Common::Point p, bool /*transparent*/) {
	_type = 2;
	_bitmap = bitmap;
	_bitmap->setX(p.x);
	_bitmap->setY(p.y);
	// transparent: what to do ?
	g_driver->createBitmap(_bitmap);
}

void PrimitiveObject::createLine(Common::Point p1, Common::Point p2, Color color) {
	_p1 = p1;
	_p2 = p2;
	_color = color;
	_type = 3;
}

void PrimitiveObject::createPolygon(Common::Point p1, Common::Point p2, Common::Point p3, Common::Point p4, Color color) {
	_p1 = p1;
	_p2 = p2;
	_p3 = p3;
	_p4 = p4;
	_color = color;
	_type = 4;
}

void PrimitiveObject::draw() {
	assert(_type);

	if (_type == 1)
		g_driver->drawRectangle(this);
	else if (_type == 2)
		g_driver->drawBitmap(_bitmap);
	else if (_type == 3)
		g_driver->drawLine(this);
	else if (_type == 4)
		g_driver->drawPolygon(this);
}
