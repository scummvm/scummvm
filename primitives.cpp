// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2005 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#include "stdafx.h"
#include "debug.h"
#include "font.h"
#include "color.h"
#include "driver.h"
#include "primitives.h"

#include <string>

PrimitiveObject::PrimitiveObject() {
	_x1 = 0;
	_y1 = 0;
	_x2 = 0;
	_y2 = 0;
	_color._vals[0] = 0;
	_color._vals[1] = 0;
	_color._vals[2] = 0;
	_filled = false;
	_type = 0;
	_bitmap = NULL;
}

PrimitiveObject::~PrimitiveObject() {
	if (_type == 2)
		g_driver->destroyBitmap(_bitmap);
}

void PrimitiveObject::createRectangle(int x1, int x2, int y1, int y2, Color color, bool filled) {
	_x1 = x1;
	_y1 = y1;
	_x2 = x2;
	_y2 = y2;
	_color = color;
	_filled = filled;
	_type = 1;
}

void PrimitiveObject::createBitmap(Bitmap *bitmap, int x, int y, bool /*transparent*/) {
	_type = 2;
	_bitmap = bitmap;
	_bitmap->setX(x);
	_bitmap->setY(y);
	// transparent: what to do ?
	g_driver->createBitmap(_bitmap);
}

void PrimitiveObject::createLine(int x1, int x2, int y1, int y2, Color color) {
	_x1 = x1;
	_y1 = y1;
	_x2 = x2;
	_y2 = y2;
	_color = color;
	_type = 3;
}

void PrimitiveObject::draw() {
	assert(_type);

	if (_type == 1)
		g_driver->drawRectangle(this);
	else if (_type == 2)
		g_driver->drawBitmap(_bitmap);
	else if (_type == 3)
		g_driver->drawLine(this);
}
