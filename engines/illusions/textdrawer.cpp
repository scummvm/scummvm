/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "illusions/illusions.h"
#include "illusions/textdrawer.h"
#include "illusions/screen.h"

namespace Illusions {

bool TextDrawer::wrapText(FontResource *font, uint16 *text, WidthHeight *dimensions, Common::Point offsPt,
	uint textFlags, uint16 *&outTextPtr) {
	_font = font;	
	_text = text;
	_dimensions = dimensions;
	_offsPt = offsPt;
	_textFlags = textFlags;
	// TODO Calc max width/height using textFlags (border, shadow)
	_textLines.clear();
	bool done = wrapTextIntern(0, 0, dimensions->_width, dimensions->_height, outTextPtr);
	// TODO Adjust text dimensions
	return done;
}

void TextDrawer::drawText(Screen *screen, Graphics::Surface *surface, uint16 color2, uint16 color1) {
	// TODO Fill box, draw borders and shadow if flags are set
	for (Common::Array<TextLine>::iterator it = _textLines.begin(); it != _textLines.end(); ++it) {
		const TextLine &textLine = *it;
		if (textLine._text)
			screen->drawText(_font, surface, textLine._x, textLine._y, textLine._text, textLine._length);
#if 0
		for (int16 linePos = 0; linePos < textLine._length; ++linePos) {
			const uint16 c = textLine._text[linePos];
			debugN("%c", c);
		}
		debug(" ");
#endif		
	}
}
	
bool TextDrawer::wrapTextIntern(int16 x, int16 y, int16 maxWidth, int16 maxHeight, uint16 *&outTextPtr) {

	bool lineBreak = false;
	bool done = false;
	bool hasChar13 = textHasChar(13);

	uint16 *lineStartText = _text;
	uint16 *currText = _text;
	outTextPtr = _text;

	int16 textPosY = y;
	int16 currLineWidth = 0, currLineLen = 0;
	int16 currWordWidth = 0, currWordLen = 0;
	int16 maxLineWidth = 0;
	int16 spaceWidth = getSpaceWidth();

	while (*currText && !done) {

		currWordWidth = 0;
		currWordLen = 0;
		do {
			currWordWidth += getCharWidth(*currText);
			++currText;
			++currWordLen;
		} while (*currText != 32 && *(currText - 1) != 32 && !hasChar13 && *currText != 13 && *currText);

		if (currWordWidth - _font->_widthC > maxWidth) {
			while (currWordWidth + currLineWidth - _font->_widthC > maxWidth) {
				--currText;
				--currWordLen;
				currWordWidth -= getCharWidth(*currText);
			}
			lineBreak = true;
		}

		if (!lineBreak && currWordWidth + currLineWidth - _font->_widthC > maxWidth) {
			lineBreak = true;
		} else {
			currLineWidth += currWordWidth;
			currLineLen += currWordLen;
			if (*currText == 0 || *currText == 13)
				lineBreak = true;
			if (lineBreak) {
				currWordLen = 0;
				currWordWidth = 0;
			}
		}
		
		while (lineBreak) {

			currLineWidth -= _font->_widthC;

			if (textPosY + _font->_charHeight <= maxHeight) {
				int16 textPosX;

				if (_textFlags & 2) {
					textPosX = (_dimensions->_width - currLineWidth) / 2;
					maxLineWidth = _dimensions->_width;
				} else {
					textPosX = x;
				}
				
				_textLines.push_back(TextLine(lineStartText, currLineLen, textPosX, textPosY));

				if (*currText == 13) {
					++currText;
					if (*currText == 10)
						++currText;
					while (*currText == 13) {
						++currText;
						if (*currText == 10)
							++currText;
						_textLines.push_back(TextLine());
						textPosY += _font->_lineIncr + _font->_charHeight;
					}
					lineStartText = currText;
				} else {
					lineStartText = currText - currWordLen;
					if (*lineStartText == 32) {
						++lineStartText;
						--currWordLen;
						currWordWidth -= spaceWidth;
					}
				}

				outTextPtr = lineStartText;

				if (maxLineWidth < currLineWidth)
					maxLineWidth = currLineWidth;

				currLineWidth = currWordWidth;
				currLineLen = currWordLen;
				currWordWidth = 0;
				currWordLen = 0;
				textPosY += _font->_charHeight + _font->_lineIncr;
				if (*currText || !currLineLen)
					lineBreak = false;

			} else {
				lineBreak = false;
				done = true;
			}
		}

	}

	_dimensions->_width = maxLineWidth;
	_dimensions->_height = textPosY - _font->_lineIncr;

	return !done;
}

bool TextDrawer::textHasChar(uint16 c) {
	// TODO
	return false;
}

int16 TextDrawer::getSpaceWidth() {
	return getCharWidth(32);
}

int16 TextDrawer::getCharWidth(uint16 c) {
	return _font->getCharInfo(c)->_width + _font->_widthC;
}

// TODO
/*
int16 offsX = (int16)(offsPt.x * 0.75);
int16 offsY = (int16)(offsPt.y * 1.5);
*/

} // End of namespace Illusions
