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

#ifndef TEXTOBJECT_H
#define TEXTOBJECT_H

#include "debug.h"
#include "font.h"
#include "color.h"
#include "driver.h"

#include <string>
#include <SDL.h>

class TextObject {
public:
	TextObject(const char *text, const int x, const int y, /*const*/ Font *font, const Color& fgColor);
	~TextObject();
	void setX(int x) {_x = x; }
	void setY(int y) {_y = y; }
	void setColor(Color *newColor) { _fgColor = newColor; }

	const char *name() const { return _textID; }
	void draw();

protected:
	Color _fgColor;
	int _x, _y;
	char _textID[10];
	uint8 *_textBitmap;
	int _bitmapHeight, _bitmapWidth;
	Driver::TextObjectHandle *_textObjectHandle;
};

#endif
