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

TextObject::TextObject(const char *text, const int x, const int y, /*const*/ Font *font, const Color& fgColor) :
		_fgColor(fgColor), _x(x), _y(y) {

	strcpy(_textID, text);
	char msgId[32];
	std::string msg = parseMsgText(_textID, msgId);

	// Calculate bitmap dimensions
	_bitmapHeight = _bitmapWidth = 0;

	for (int i = 0; msg[i] != '\0'; ++i) {
		_bitmapWidth += font->getCharLogicalWidth(msg[i]) + font->getCharStartingCol(msg[i]);

		int h = font->getCharHeight(msg[i]) + font->getCharStartingLine(msg[i]);
		if (h > _bitmapHeight)
			_bitmapHeight = h;
	}

	//printf("creating textobject: %s\nheight: %d\nwidth: %d\n", msg.c_str(), _bitmapHeight, _bitmapWidth);

	_textBitmap = new uint8[_bitmapHeight * _bitmapWidth];
	memset(_textBitmap, 0, _bitmapHeight * _bitmapWidth);

	// Fill bitmap
	int offset = 0;
	for (int line = 0; line < _bitmapHeight; ++line) {
		for (int c = 0; msg[c] != '\0'; ++c) {
			uint32 charWidth = font->getCharWidth(msg[c]);
			uint32 charLogicalWidth = font->getCharLogicalWidth(msg[c]);
			uint8 startingCol = font->getCharStartingCol(msg[c]);
			uint8 startingLine = font->getCharStartingLine(msg[c]);

			if (startingLine < line + 1 && font->getCharHeight(msg[c]) + startingLine > line) {
				memcpy(_textBitmap + offset + startingCol,
					font->getCharData(msg[c]) + charWidth * (line - startingLine), charWidth);
			}

			offset += charLogicalWidth + startingCol;
		}
	}

	_textObjectHandle = g_driver->prepareToTextBitmap(_textBitmap, _bitmapWidth, _bitmapHeight, _fgColor);

	g_engine->registerTextObject(this);
}

TextObject::~TextObject() {
	delete[] _textBitmap;
	if (_textObjectHandle->bitmapData)
		delete _textObjectHandle->bitmapData;
	if (_textObjectHandle->surface)
		SDL_FreeSurface((SDL_Surface *)_textObjectHandle->surface);
	if (_textObjectHandle->texIds)
		delete _textObjectHandle->texIds;

	delete _textObjectHandle;
}

void TextObject::draw() {
	g_driver->drawTextBitmap(_x, _y, _textObjectHandle);
}
