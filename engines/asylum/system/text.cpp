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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "asylum/system/text.h"

#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"

#include "asylum/asylum.h"
#include "asylum/respack.h"

#include "common/endian.h"
#include "common/rational.h"

namespace Asylum {

Text::Text(AsylumEngine *engine) : _vm(engine) {
	_posX = 0;
	_posY = 0;
	_curFontFlags = 0;
	_fontResource = 0;
}

Text::~Text() {
	delete _fontResource;

	// Zero-out passed pointers
	_vm = NULL;
}

void Text::loadFont(ResourceId resourceId) {
	if (_fontResource && resourceId == _fontResource->getResourceId())
		return;

	delete _fontResource;
	_fontResource = new GraphicResource(_vm, resourceId);

	if (resourceId != kResourceNone) {
		// load font flag data
		_curFontFlags = Common::Rational(_fontResource->getFlags(), 16).toInt() & 0x0F;
	}
}

void Text::setPosition(int32 x, int32 y) {
	_posX = x;
	_posY = y;
}

int32 Text::getWidth(char c) {
	GraphicFrame *font = _fontResource->getFrame((uint8)c);

	return font->surface.w + font->x - _curFontFlags;
}

int32 Text::getWidth(const char *text) {
	if (!_fontResource)
		error("[Text::getWidth] font resource hasn't been loaded yet!");

	int32 width = 0;
	char character = *text;
	while (character) {
		GraphicFrame *font = _fontResource->getFrame((uint8)character);
		width += font->surface.w + font->x - _curFontFlags;

		text++;
		character = *text;
	}
	return width;
}

int32 Text::getWidth(const char *text, uint32 length) {
	if (!_fontResource)
		error("[Text::getWidth] font resource hasn't been loaded yet!");

	if (length == 0)
		return 0;

	int32 width = 0;
	char character = *text;
	while (character) {
		GraphicFrame *font = _fontResource->getFrame((uint8)character);
		width += font->surface.w + font->x - _curFontFlags;

		text++;
		character = *text;
	}
	return width;
}

int32 Text::getWidth(ResourceId resourceId) {
	return getWidth(get(resourceId));
}

char* Text::get(ResourceId resourceId) {
	ResourceEntry *textRes = getResource()->get(resourceId);
	return (char*)textRes->data;
}

void Text::drawChar(char character) {
	if (!_fontResource)
		error("[Text::drawChar] font resource hasn't been loaded yet!");

	// TODO Add proper methods to screen class (also used by all other drawing calls)
	/*if (getScreen()->getCurrentTransTableNum()) {
		getScreen()->drawTransparent(_fontResource->getResourceId(), (uint8)character, _posX, _posY, 0, getScreen()->getCurrentTransTableNum());
	} else {
		getScreen()->draw(_fontResource->getResourceId(), (uint8)character, _posX, _posY, 0);
	}
	*/
	GraphicFrame *fontLetter = _fontResource->getFrame((uint8)character);
	getScreen()->copyRectToScreenWithTransparency((byte *)fontLetter->surface.pixels, fontLetter->surface.w, _posX, _posY + fontLetter->y, fontLetter->surface.w, fontLetter->surface.h);

	_posX += fontLetter->surface.w + fontLetter->x - _curFontFlags;
}

void Text::draw(const char *text) {
	if (!text)
		return;

	while (*text) {
		drawChar(text[0]);
		text++;
	}
}

void Text::draw(const char *text, uint32 length) {
	if (length == 0)
		return;

	if (!text)
		return;

	for (uint i = 0; i < length; i++)
		drawChar(text[i]);
}

void Text::draw(ResourceId resourceId) {
	ResourceEntry *textRes = getResource()->get(resourceId);
	draw((char*)textRes->data);
}

void Text::draw(int32 x, int32 y, const char *text) {
    setPosition(x - getWidth(text), y);
    draw(text);
}

void Text::draw(int32 x, int32 y, ResourceId resourceId) {
	draw(x, y, get(resourceId));
}

void Text::draw(const char *text, ResourceId fontResourceId, int32 y) {
	if (text) {
		loadFont(fontResourceId);
		draw(true, 20, y, 16, 600, text);
	}
}

void Text::draw(bool isCentered, int32 x, int32 y, int32 spacing, int32 width, const char *text) {
	draw(0, 99, isCentered, x, y, spacing, width, text);
}

void Text::draw(int32 a1, int32 a2, bool isCentered, int32 x, int32 y, int32 spacing, int32 width, const char *text) {
	if (!text || !*text)
		return;

	bool drawText = false;
	int32 charWidth = 0;
	int32 spaceWidth = 0;
	int32 index = 0;

	const char *string = text;
	const char *endText = text;

	for (;;) {
label_start:

		charWidth = 0;

		// Draw the text
		if (drawText) {
			char currentChar = *endText;

			if (index >= a1 && index <= (a1 + 2)) {
				if (isCentered) {
					drawCentered(x, y, width, endText - string, string);
				} else {
					setPosition(x, y);
					draw(text, endText - text);
				}

				y += spacing;
			}

			++index;

			if (!currentChar)
				break;

			charWidth = 0;
			spaceWidth = 0;
			text = endText + 1;

			// Skip spaces
			if (currentChar == ' ' && *text == ' ')
				do {
					++text;
				} while (*text == ' ');
		}

		const char *txt = text;
		int32 w = 0;

		for (;;) {
			char c = *txt;
			w += charWidth;
			charWidth = spaceWidth;
			const char *txt2 = txt;

			if (*txt != ' ') {
				do {
					if (!c)
						break;

					if (c == 1)
						break;

					charWidth += getWidth(c);
					txt++;
					c = txt[0];
				} while (c != ' ');
			}

			if (w + charWidth > width) {
				string = text;
				endText = txt2 - 1;
				drawText = true;
				goto label_start;
			}

			if (!*txt)
				break;

			if (*txt == 1 || *txt == 2)
				break;

			++txt;
			spaceWidth = getWidth(' ');
		}

		string = text;
		endText = txt;
		drawText = true;
	}
}

void Text::drawCentered(int32 x, int32 y, int32 width, const char *text) {
	setPosition(x + (width - getWidth(text)) / 2, y);
	draw(text);
}

void Text::drawCentered(int32 x, int32 y, int32 width, uint32 length, const char *text) {
	setPosition(x + (width - getWidth(text, length)) / 2, y);
	draw(text, length);
}

void Text::drawCentered(int32 x, int32 y, int32 width, ResourceId resourceId, int32 value) {
	ResourceEntry *textRes = getResource()->get(resourceId);
	char *text = (char *)textRes->data;
	char txt[100];
	sprintf(txt, text, value);
	drawCentered(x, y, width, txt);
}

void Text::drawCentered(int32 x, int32 y, int32 width, ResourceId resourceId) {
	drawCentered(x, y, width, get(resourceId));
}

void Text::drawAlignedRight(int32 x, int32 y, const char *text) {
	setPosition(x - getWidth(text), y);
	draw(text);
}

void Text::drawAlignedRight(int32 x, int32 y, ResourceId resourceId) {
	drawAlignedRight(x, y, get(resourceId));
}

} // end of namespace Asylum
