/* Residual - Virtual machine to run LucasArts' 3D adventure games
 * Copyright (C) 2003-2006 The ScummVM-Residual Team (www.scummvm.org)
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
		_bitmapWidthPtr(NULL), _textObjectHandle(NULL) {
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
	uint height = 0;

	for (int i = 0; i < _numberLines; i++) {
		height += _font->getHeight();
	}
	return height;
}

int TextObject::getTextCharPosition(int pos) {
	int width = 0;
	std::string msg = parseMsgText(_textID, NULL);
	for (int i = 0; (msg[i] != '\0') && (i < pos); ++i) {
		width += _font->getCharWidth(msg[i]);
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
	for (int i = (int)msg.length() - 1; c[i] == TEXT_NULL; i--)
		msg.erase(msg.length() - 1, msg.length());

	// remove char of id 13 from the end of the string,
	for (int i = (int)msg.length() - 1; c[i] == 13; i--)
		msg.erase(msg.length() - 1, msg.length());

	// format the output message to incorporate line wrapping
	// (if necessary) for the text object
	_numberLines = 1;
	lineWidth = 0;
	for (int i = 0; msg[i] != '\0'; ++i) {
		lineWidth += MAX(_font->getCharWidth(msg[i]), _font->getCharDataWidth(msg[i]));
		if ((_width != 0 && lineWidth > (_width - _x))
				|| (_justify == CENTER && (_x - lineWidth / 2 < 0 || _x + lineWidth / 2 > 640))
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
	_textObjectHandle = (Driver::TextObjectHandle **)malloc(sizeof(long) * _numberLines);
	_bitmapWidthPtr = (int *)malloc(sizeof(int) * _numberLines);

	for (int j = 0; j < _numberLines; j++) {
		int nextLinePos = message.find_first_of('\n');
		std::string currentLine = message.substr(0, nextLinePos);

		_bitmapWidthPtr[j] = 0;
		for (int i = 0; currentLine[i] != '\0'; ++i) {
			_bitmapWidthPtr[j] += MAX(_font->getCharWidth(currentLine[i]),
										_font->getCharDataWidth(currentLine[i]));
		}

		_textBitmap = new uint8[_font->getHeight() * (_bitmapWidthPtr[j] + 1)];
		memset(_textBitmap, 0, _font->getHeight() * (_bitmapWidthPtr[j] + 1));

		// Fill bitmap
		int startOffset = 0;
		for (int c = 0; currentLine[c] != '\0'; c++) {
			int ch = currentLine[c];
			int8 startingLine = _font->getCharStartingLine(ch) + _font->getBaseOffsetY();
			int32 charDataWidth = _font->getCharDataWidth(ch);
			int32 charWidth = _font->getCharWidth(ch);
			int8 startingCol = _font->getCharStartingCol(ch);
			for (int line = 0; line < _font->getCharDataHeight(ch); line++) {
				int offset = startOffset + ((_bitmapWidthPtr[j] + 1) * (line + startingLine));
				for (int r = 0; r < charDataWidth; r++) {
					const byte pixel = *(_font->getCharData(ch) + r + (charDataWidth * line));
					byte *dst = _textBitmap + offset + startingCol + r;
					if ((*dst == 0) && (pixel != 0))
						_textBitmap[offset + startingCol + r] = pixel;
				}
				if (line + startingLine >= _font->getHeight())
					break;
			}
			startOffset += charWidth;
		}

		_textObjectHandle[j] = g_driver->createTextBitmap(_textBitmap, _bitmapWidthPtr[j] + 1, _font->getHeight(), _fgColor);
		delete[] _textBitmap;
		message = message.substr(nextLinePos + 1, message.length() - (nextLinePos + 1));
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
}

void TextObject::draw() {
	int height = 0;

	if (!_created)
		return;
	// render multi-line (wrapped) text
	for (int i = 0; i < _numberLines; i++) {
		int y;

		if (_height != 0)
			y = _y + 5;
		else
			y = _y + 5;
		if (y < 0)
			y = 0;
		
		if (_justify == LJUSTIFY || _justify == NONE)
			g_driver->drawTextBitmap(_x, height + y, _textObjectHandle[i]);
		else if (_justify == CENTER) {
			int x = _x - (_bitmapWidthPtr[i] / 2);
			if (x < 0)
				x = 0;

			g_driver->drawTextBitmap(x, height + y, _textObjectHandle[i]);
		} else if (_justify == RJUSTIFY) {
			int x = (_x - getBitmapWidth());
			if (x < 0)
				x = 0;

			g_driver->drawTextBitmap(x, height + y, _textObjectHandle[i]);
		} else if (debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
			warning("TextObject::draw: Unknown justification code (%d)!", _justify);

		height += _font->getHeight();
	}
}
