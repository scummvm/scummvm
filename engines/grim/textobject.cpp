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

#include "engines/grim/grim.h"
#include "engines/grim/textobject.h"
#include "engines/grim/savegame.h"
#include "engines/grim/lua.h"

namespace Grim {

Common::String parseMsgText(const char *msg, char *msgId);

TextObjectDefaults sayLineDefaults;
TextObjectDefaults printLineDefaults;
TextObjectDefaults blastTextDefaults;

TextObject::TextObject(bool blastDraw, bool isSpeech) :
		_created(false), _x(0), _y(0), _width(0), _height(0), _justify(0),
		_numberLines(1), _disabled(false), _font(NULL), _textBitmap(NULL),
		_bitmapWidthPtr(NULL), _textObjectHandle(NULL) {
	memset(_textID, 0, sizeof(_textID));
	_fgColor._vals[0] = 0;
	_fgColor._vals[1] = 0;
	_fgColor._vals[2] = 0;
	_blastDraw = blastDraw;
	_isSpeech = isSpeech;
}

void TextObject::setText(const char *text) {
	if (strlen(text) < sizeof(_textID))
		strcpy(_textID, text);
	else {
		error("Text ID exceeded maximum length (%d): %s", (int)sizeof(_textID), text);
		// this should be good enough to still be unique
		// but for debug purposes lets make this crash the program so we know about it
		strncpy(_textID, text, sizeof(_textID));
		_textID[sizeof(_textID)] = 0;
	}
}

TextObject::~TextObject() {
	destroyBitmap();
}

void TextObject::saveState(SaveGame *savedState) {
	int32 size;
	PointerId ptr;

	size = strlen(_textID);
	savedState->writeLESint32(size);
	savedState->write(_textID, size);
	ptr = makeIdFromPointer(_font);
	savedState->writeLEUint32(ptr.low);
	savedState->writeLEUint32(ptr.hi);
	savedState->writeLEUint32(_blastDraw);
	savedState->writeLEUint32(_disabled);
	savedState->writeLEUint32(_justify);
	savedState->writeLEUint32(_width);
	savedState->writeLEUint32(_height);
	savedState->writeLEUint32(_x);
	savedState->writeLEUint32(_y);
	savedState->writeLEUint32(_fgColor.red());
	savedState->writeLEUint32(_fgColor.green());
	savedState->writeLEUint32(_fgColor.blue());
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
	if (!_bitmapWidthPtr)
		return 0;

	int width = 0;

	for (int i = 0; i < _numberLines; i++) {
		if (_bitmapWidthPtr[i] > width)
			width = _bitmapWidthPtr[i];
	}
	return width;
}

int TextObject::getBitmapHeight() {
	return _numberLines * _font->getHeight();
}

int TextObject::getTextCharPosition(int pos) {
	int width = 0;
	Common::String msg = parseMsgText(_textID, NULL);
	for (int i = 0; (msg[i] != '\0') && (i < pos); ++i) {
		width += _font->getCharWidth(msg[i]);
	}
	return width;
}

void TextObject::createBitmap() {
	if (_created)
		destroyBitmap();

	Common::String msg = parseMsgText(_textID, NULL);
	Common::String message;
	const char *c = msg.c_str();

	// remove spaces (NULL_TEXT) from the end of the string,
	// while this helps make the string unique it screws up
	// text justification
	for (int i = (int)msg.size() - 1; c[i] == TEXT_NULL; i--)
		msg.deleteLastChar();

	// remove char of id 13 from the end of the string,
	for (int i = (int)msg.size() - 1; c[i] == 13; i--)
		msg.deleteLastChar();

	// format the output message to incorporate line wrapping
	// (if necessary) for the text object
	const int SCREEN_WIDTH = 640;
	const int SCREEN_MARGIN = 75;

	// If the speaker is too close to the edge of the screen we have to make
	// some room for the subtitles.
	if (_isSpeech){
		if (_x < SCREEN_MARGIN) {
			_x = SCREEN_MARGIN;
		} else if (SCREEN_WIDTH - _x < SCREEN_MARGIN) {
			_x = SCREEN_WIDTH - SCREEN_MARGIN;
		}
	}

	// The maximum width for any line of text is determined by the justification
	// mode. Note that there are no left/right margins -- this is consistent
	// with GrimE.
	int maxWidth;
	if (_justify == CENTER) {
		maxWidth = 2 * MIN(_x, SCREEN_WIDTH - _x);
	} else if (_justify == LJUSTIFY) {
		maxWidth = SCREEN_WIDTH - _x;
	} else if (_justify == RJUSTIFY) {
		maxWidth = _x;
	}

	// We break the message to lines not longer than maxWidth
	_numberLines = 1;
	int lineWidth = 0;
	int maxLineWidth = 0;
	for (int i = 0; i < (int)msg.size(); i++) {
		lineWidth += MAX(_font->getCharWidth(msg[i]), _font->getCharDataWidth(msg[i]));
		if (lineWidth > maxWidth) {
			if (message.contains(' ')) {
				while (msg[i] != ' ' && i > 0) {
					lineWidth -= MAX(_font->getCharWidth(msg[i]), _font->getCharDataWidth(msg[i]));
					message.deleteLastChar();
					--i;
				} 
			} else if (msg[i] != ' ') { // if it is a unique word
				int dashWidth = MAX(_font->getCharWidth('-'), _font->getCharDataWidth('-'));
				while (lineWidth + dashWidth > maxWidth) {
					lineWidth -= MAX(_font->getCharWidth(msg[i]), _font->getCharDataWidth(msg[i]));
					message.deleteLastChar();
					--i;
				}
				message += '-';
 			}
			message += '\n';
			_numberLines++;

			if (lineWidth > maxLineWidth) {
				maxLineWidth = lineWidth;
			}
			lineWidth = 0;

			continue; // don't add the space back
		}

		if (lineWidth > maxLineWidth)
			maxLineWidth = lineWidth;

		message += msg[i];
	}

	// If the text object is a speech subtitle, the y parameter is the
	// coordinate of the bottom of the text block (instead of the top). It means
	// that every extra line pushes the previous lines up, instead of being
	// printed further down the screen.
	const int SCREEN_TOP_MARGIN = 16;
	if (_isSpeech) {
		_y -= _numberLines * _font->getHeight();
		if (_y < SCREEN_TOP_MARGIN) {
			_y = SCREEN_TOP_MARGIN;
		}
	}

	_textObjectHandle = (GfxBase::TextObjectHandle **)malloc(sizeof(long) * _numberLines);
	_bitmapWidthPtr = new int[_numberLines];

	for (int j = 0; j < _numberLines; j++) {
		int nextLinePos, cutLen;
		const char *pos = strchr(message.c_str(), '\n');
		if (pos) {
			nextLinePos = pos - message.c_str();
			cutLen = nextLinePos + 1;
		} else {
			nextLinePos = message.size();
			cutLen = nextLinePos;
		}
		Common::String currentLine(message.c_str(), message.c_str() + nextLinePos);

		_bitmapWidthPtr[j] = 0;
		for (unsigned int i = 0; i < currentLine.size(); ++i) {
			_bitmapWidthPtr[j] += MAX(_font->getCharWidth(currentLine[i]), _font->getCharDataWidth(currentLine[i]));
		}

		_textBitmap = new uint8[_font->getHeight() * (_bitmapWidthPtr[j] + 1)];
		memset(_textBitmap, 0, _font->getHeight() * (_bitmapWidthPtr[j] + 1));

		// Fill bitmap
		int startOffset = 0;
		for (unsigned int d = 0; d < currentLine.size(); d++) {
			int ch = currentLine[d];
			int8 startingLine = _font->getCharStartingLine(ch) + _font->getBaseOffsetY();
			int32 charDataWidth = _font->getCharDataWidth(ch);
			int32 charWidth = _font->getCharWidth(ch);
			int8 startingCol = _font->getCharStartingCol(ch);
			for (int line = 0; line < _font->getCharDataHeight(ch); line++) {
				int offset = startOffset + ((_bitmapWidthPtr[j] + 1) * (line + startingLine));
				for (int r = 0; r < charDataWidth; r++) {
					const byte pixel = *(_font->getCharData(ch) + r + (charDataWidth * line));
					byte *dst = _textBitmap + offset + startingCol + r;
					if (*dst == 0 && pixel != 0)
						_textBitmap[offset + startingCol + r] = pixel;
				}
				if (line + startingLine >= _font->getHeight())
					break;
			}
			startOffset += charWidth;
		}

		_textObjectHandle[j] = g_driver->createTextBitmap(_textBitmap, _bitmapWidthPtr[j] + 1, _font->getHeight(), _fgColor);
		delete[] _textBitmap;
		for (int count = 0; count < cutLen; count++)
			message.deleteChar(0);
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
		delete[] _bitmapWidthPtr;
		_bitmapWidthPtr = NULL;
	}
}

void TextObject::draw() {
	int height = 0;

	if (!_created || _disabled)
		return;
	// render multi-line (wrapped) text
	for (int i = 0; i < _numberLines; i++) {
		int y;

		if (_blastDraw)
			y = _y + 5;
		else {
			if (_font->getHeight() == 21) // talk_font,verb_font
				y = _y - 6;
			else if (_font->getHeight() == 26) // special_font
				y = _y - 12;
			else if (_font->getHeight() == 13) // computer_font
				y = _y - 6;
			else if (_font->getHeight() == 19) // pt_font
				y = _y - 9;
			else
				y = _y;
		}

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
		} else if (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
			warning("TextObject::draw: Unknown justification code (%d)", _justify);

		height += _font->getHeight();
	}
}

} // end of namespace Grim
