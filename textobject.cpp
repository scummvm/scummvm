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
		_numberLines(1), _disabled(false), _font(NULL), _textBitmap(NULL),
		_bitmapWidthPtr(NULL), _bitmapHeightPtr(NULL), _textObjectHandle(NULL) {
	memset(_textID, 0, sizeof(_textID));
	_fgColor._vals[0] = 0;
	_fgColor._vals[1] = 0;
	_fgColor._vals[2] = 0;
}

void TextObject::setText(char *text) {
	if (strlen(text) < sizeof(_textID))
		strcpy(_textID, text);
	else {
		error("Text ID exceeded maximum length (%d): %s\n", sizeof(_textID), text);
		// this should be good enough to still be unique
		// but for debug purposes lets make this crash the program so we know about it
		strncpy(_textID, text, sizeof(_textID));
		_textID[sizeof(_textID)] = 0;
	}
}

TextObject::~TextObject() {
	destroyBitmap();
}

void TextObject::setDefaults(TextObjectDefaults *defaults) {
	_x = defaults->x;
	_y = defaults->x;
	_font = defaults->font;
	_fgColor = defaults->fgColor;
	_justify = defaults->justify;
	_disabled = defaults->disabled;
}

int TextObject::getBitmapWidth() {
	if (_bitmapWidthPtr == NULL)
		return 0;

	int width = 0;

	for (int i = 0; i < _numberLines; i++) {
		if (_bitmapWidthPtr[i] > width)
			width = _bitmapWidthPtr[i];
	}
	return width;
}

int TextObject::getBitmapHeight() {
	if (_bitmapHeightPtr == NULL)
		return 0;

	uint height = 0;

	for (int i = 0; i < _numberLines; i++) {
		height += _bitmapHeightPtr[i];
	}
	return height;
}

int TextObject::getTextCharPosition(int pos) {
	int width = 0;
	std::string msg = parseMsgText(_textID, NULL);
	for (int i = 0; (msg[i] != '\0') && (i < pos); ++i) {
		width += _font->getCharLogicalWidth(msg[i]);
	}

	return width;
}

void TextObject::createBitmap() {
	if (_created)
		destroyBitmap();

	std::string msg = parseMsgText(_textID, NULL);
	std::string message;
	char *c = (char *)msg.c_str();

	int lineWidth = 0;

	// remove spaces (NULL_TEXT) from the end of the string,
	// while this helps make the string unique it screws up
	// text justification
	for(int i = (int) msg.length() - 1; c[i] == TEXT_NULL; i--)
		msg.erase(msg.length() - 1, msg.length());

	// format the output message to incorporate line wrapping
	// (if necessary) for the text object
	_numberLines = 1;
	lineWidth = 0;
	for (int i = 0; msg[i] != '\0'; ++i) {
		lineWidth += _font->getCharLogicalWidth(msg[i]);
		if ((_width != 0 && lineWidth > (_width - _x))
				|| (_justify == CENTER && (_x - lineWidth/2 < 0 || _x + lineWidth/2 > 640))
				|| (_justify == LJUSTIFY && (_x + lineWidth > 640))
				|| (_justify == RJUSTIFY && (_x - lineWidth < 0))) {
			lineWidth = 0;
			for (; msg[i] != ' '; i--)
				message.erase(message.length() - 1, message.length());
			message += '\n';
			_numberLines++;
			continue; // don't add the space back
		}
		message += msg[i];
	}
	_textObjectHandle = (Driver::TextObjectHandle **) malloc(sizeof(long) * _numberLines);
	_bitmapWidthPtr = (int *)malloc(sizeof(int) * _numberLines);
	_bitmapHeightPtr = (int *)malloc(sizeof(int) * _numberLines);

	for (int j = 0; j < _numberLines; j++) {
		int nextLinePos = message.find_first_of('\n');
		std::string currentLine = message.substr(0, nextLinePos);

		_bitmapWidthPtr[j] = 0;
		_bitmapHeightPtr[j] = 0;
		for (int i = 0; currentLine[i] != '\0'; ++i) {
			_bitmapWidthPtr[j] += _font->getCharLogicalWidth(currentLine[i]);
			int h = _font->getCharHeight(currentLine[i]) + _font->getCharStartingLine(currentLine[i]);
			if (h > _bitmapHeightPtr[j])
				_bitmapHeightPtr[j] = h;
		}
		//printf("creating textobject: %s\nheight: %d\nwidth: %d\n", currentLine.c_str(), _bitmapHeight[j], _bitmapWidth[j]);

		// Due to the size of charWidth we need to allocate one more byte than we plan on using
		_textBitmap = new uint8[_bitmapHeightPtr[j] * _bitmapWidthPtr[j] + 1];
		memset(_textBitmap, 0, _bitmapHeightPtr[j] * _bitmapWidthPtr[j] + 1);

		// Fill bitmap
		int offset = 0;
		for (int line = 0; line < _bitmapHeightPtr[j]; ++line) {
			for (int c = 0; currentLine[c] != '\0'; ++c) {
				int32 charWidth = _font->getCharWidth(currentLine[c]);
				int32 charLogicalWidth = _font->getCharLogicalWidth(currentLine[c]);
				int8 startingCol = _font->getCharStartingCol(currentLine[c]);
				int8 startingLine = _font->getCharStartingLine(currentLine[c]);

				if (startingLine < line + 1 && _font->getCharHeight(currentLine[c]) + startingLine > line) {
					memcpy(&_textBitmap[offset + startingCol],
						_font->getCharData(currentLine[c]) + charWidth * (line - startingLine), charWidth);
				}

				offset += charLogicalWidth;
			}
		}
		_textObjectHandle[j] = g_driver->createTextBitmap(_textBitmap, _bitmapWidthPtr[j], _bitmapHeightPtr[j], _fgColor);
		delete[] _textBitmap;
		message = message.substr(nextLinePos+1, message.length()-(nextLinePos+1));
	}
	_created = true;
}

void TextObject::destroyBitmap() {
	_created = false;
	if (_textObjectHandle) {
		for (int i = 0; i < _numberLines; i++) {
			g_driver->destroyTextBitmap(_textObjectHandle[i]);
			delete _textObjectHandle[i];
		}
		free(_textObjectHandle);
		_textObjectHandle = NULL;
	}
	if (_bitmapWidthPtr) {
		free(_bitmapWidthPtr);
		_bitmapWidthPtr = NULL;
	}
	if (_bitmapHeightPtr) {
		free(_bitmapHeightPtr);
		_bitmapHeightPtr = NULL;
	}
}

void TextObject::draw() {
	int height = 0;

	if (!_created)
		return;
	// render multi-line (wrapped) text
	for (int i = 0; i < _numberLines; i++) {
		int y;

		if (_height != 0)
			y = (int) (_y - _bitmapHeightPtr[i] / 2.0);
		else
			y = _y;
		if (y < 0)
			y = 0;
		
		if (_justify == LJUSTIFY || _justify == NONE)
			g_driver->drawTextBitmap(_x, height + y, _textObjectHandle[i]);
		else if (_justify == CENTER) {
			int x = (int) (_x - (1 / 2.0) * _bitmapWidthPtr[i]);
			if (x < 0)
				x = 0;

			g_driver->drawTextBitmap(x, height + y, _textObjectHandle[i]);
		} else if (_justify == RJUSTIFY) {
			int x = _x - getBitmapWidth();
			if (x < 0)
				x = 0;

			g_driver->drawTextBitmap(x, height + _y, _textObjectHandle[i]);
		} else if (debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
			warning("TextObject::draw: Unknown justification code (%d)!", _justify);

		height += _bitmapHeightPtr[i];
	}
}
