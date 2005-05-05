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

#ifndef PRIMITIVESOBJECT_H
#define PRIMITIVESOBJECT_H

#include "stdafx.h"
#include "debug.h"
#include "font.h"
#include "color.h"
#include "driver.h"

#include <string>
#include <SDL.h>

class PrimitiveObject {
public:
	PrimitiveObject();
	~PrimitiveObject();

	void createRectangle(int x1, int x2, int y1, int y2, Color color, bool filled);
	void createBitmap(Bitmap *bitmap, int x, int y, bool transparent);
	void createLine(int x1, int x2, int y1, int y2, Color color);
	int getX1() { return _x1; }
	int getX2() { return _x2; }
	int getY1() { return _y1; }
	int getY2() { return _y2; }
	void setY1(int coord) { _y1 = coord; }
	void setY2(int coord) { _y2 = coord; }
	void setColor(Color color) { _color = color; } 	Color getColor() { return _color; }
	bool isFilled() { return _filled; }
	void draw();
	bool isBitmap() { return _type == 2; }
	Bitmap *getBitmapHandle() { assert(_bitmap); return _bitmap; }

private:
	int _x1, _x2, _y1, _y2;
	Color _color;
	bool _filled;
	int _type;
	Bitmap *_bitmap;
};

#endif
