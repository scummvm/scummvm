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
#include "textobject.h"
#include "engine.h"
#include "localize.h"
#include "driver.h"

std::string parseMsgText(const char *msg, char *msgId);

TextObjectDefaults sayLineDefaults;
TextObjectDefaults printLineDefaults;
TextObjectDefaults textObjectDefaults;

TextObject::TextObject() :
		_created(false), _x(0), _y(0), _width(0), _height(0), _justify(0),
		_font(NULL), _textBitmap(NULL), _bitmapWidth(0),
		_bitmapHeight(0), _textObjectHandle(NULL) {
	memset(_textID, 0, sizeof(_textID));
	_fgColor._vals[0] = 0;
	_fgColor._vals[1] = 0;
	_fgColor._vals[2] = 0;
}

TextObject::~TextObject() {
	destroyBitmap();
}

void TextObject::setDefaults(TextObjectDefaults *defaults) {
	_x = defaults->x;
	_y = defaults->x;
	_width = defaults->width;
	_height = defaults->height;
	_font = defaults->font;
	_fgColor = defaults->fgColor;
	_justify = defaults->justify;
}

void TextObject::createBitmap() {
	if (_created)
		destroyBitmap();

	std::string msg = parseMsgText(_textID, NULL);

	_bitmapWidth = 0;
	_bitmapHeight = 0;

	for (int i = 0; msg[i] != '\0'; ++i) {
		_bitmapWidth += _font->getCharLogicalWidth(msg[i]) + _font->getCharStartingCol(msg[i]);

		uint h = _font->getCharHeight(msg[i]) + _font->getCharStartingLine(msg[i]);
		if (h > _bitmapHeight)
			_bitmapHeight = h;
	}

	//printf("creating textobject: %s\nheight: %d\nwidth: %d\n", msg.c_str(), _bitmapHeight, _bitmapWidth);

	_textBitmap = new uint8[_bitmapHeight * _bitmapWidth];
	memset(_textBitmap, 0, _bitmapHeight * _bitmapWidth);

	// Fill bitmap
	int offset = 0;
	for (uint line = 0; line < _bitmapHeight; ++line) {
		for (int c = 0; msg[c] != '\0'; ++c) {
			uint32 charWidth = _font->getCharWidth(msg[c]);
			uint32 charLogicalWidth = _font->getCharLogicalWidth(msg[c]);
			uint8 startingCol = _font->getCharStartingCol(msg[c]);
			uint8 startingLine = _font->getCharStartingLine(msg[c]);

			if (startingLine < line + 1 && _font->getCharHeight(msg[c]) + startingLine > line) {
				memcpy(&_textBitmap[offset + startingCol],
					_font->getCharData(msg[c]) + charWidth * (line - startingLine), charWidth);
			}

			offset += charLogicalWidth + startingCol;
		}
	}

	_textObjectHandle = g_driver->createTextBitmap(_textBitmap, _bitmapWidth, _bitmapHeight, _fgColor);

	delete[] _textBitmap;
	_created = true;
}

void TextObject::destroyBitmap() {
	if (_textObjectHandle) {
		g_driver->destroyTextBitmap(_textObjectHandle);
		delete _textObjectHandle;
		_textObjectHandle = NULL;
	}
	_created = false;
}

void TextObject::draw() {
	g_driver->drawTextBitmap(_x, _y, _textObjectHandle);
}
