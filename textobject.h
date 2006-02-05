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

struct TextObjectDefaults {
	Color fgColor;
	int x, y;
	int width, height;
	int justify;
	bool disabled;
	Font *font;
};

#define TEXT_NULL   ' '

extern TextObjectDefaults sayLineDefaults;
extern TextObjectDefaults printLineDefaults;
extern TextObjectDefaults textObjectDefaults;

class TextObject {
public:
	TextObject();
	~TextObject();
	void createBitmap();
	void destroyBitmap();
	void setDefaults(TextObjectDefaults *defaults);
	void setText(char *text);
	void setX(int x) { _x = x; }
	void setY(int y) { _y = y; }
	void setWidth(int width) { _width = width; }
	void setHeight(int height) { _height = height; }
	void setFGColor(Color *fgColor) { _fgColor = fgColor; }
	void setFont(Font *font) { _font = font; }
	void setJustify(int justify) { _justify = justify; }
	void setDisabled(bool disabled) { _disabled = disabled; }
	int getBitmapWidth();
	int getBitmapHeight();
	int getTextCharPosition(int pos);

	const char *name() const { return _textID; }
	void draw();

	enum Justify {
		NONE,
		CENTER,
		LJUSTIFY,
		RJUSTIFY
	};

protected:
	bool _created;
	Color _fgColor;
	int _x, _y;
	int _width, _height;
	int _justify, _numberLines;
	bool _disabled;
	Font *_font;
	char _textID[256];
	uint8 *_textBitmap;
	int *_bitmapWidthPtr, *_bitmapHeightPtr;
	Driver::TextObjectHandle **_textObjectHandle;
};

#endif
