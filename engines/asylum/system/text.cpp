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
 */

#include "asylum/system/text.h"

#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"

#include "asylum/asylum.h"
#include "asylum/respack.h"

#include "common/rational.h"

namespace Asylum {

Text::Text(AsylumEngine *engine) : _vm(engine) {
	_curFontFlags = 0;
	_fontResource = 0;
	_transTableNum = 0;
}

Text::~Text() {
	delete _fontResource;

	// Zero-out passed pointers
	_vm = NULL;
}

ResourceId Text::loadFont(ResourceId resourceId) {
	if (_fontResource && resourceId == _fontResource->getResourceId())
		return resourceId;

	ResourceId previousFont = _fontResource ? _fontResource->getResourceId() : kResourceNone;

	delete _fontResource;
	_fontResource = NULL;

	if (resourceId != kResourceNone) {
		_fontResource = new GraphicResource(_vm, resourceId);
		_curFontFlags = Common::Rational(_fontResource->getData().flags, 16).toInt() & 0x0F;
	}

	return previousFont;
}

void Text::setPosition(const Common::Point &point) {
	_position = point;
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
	while (character && length > 0) {
		GraphicFrame *font = _fontResource->getFrame((uint8)character);
		width += font->surface.w + font->x - _curFontFlags;

		text++;
		character = *text;
		length--;
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

	if (_transTableNum) {
		getScreen()->draw(_fontResource, (uint8)character, _position, kDrawFlagNone, _transTableNum);
	} else {
		getScreen()->draw(_fontResource, (uint8)character, _position);
	}

	GraphicFrame *fontLetter = _fontResource->getFrame((uint8)character);
	_position.x += (int16)fontLetter->surface.w + fontLetter->x - _curFontFlags;
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

void Text::draw(const Common::Point &point, const char *text) {
    setPosition(Common::Point(point.x - getWidth(text), point.y));
    draw(text);
}

void Text::draw(const Common::Point &point, ResourceId resourceId) {
	draw(point, get(resourceId));
}

void Text::draw(const char *text, ResourceId fontResourceId, int32 y) {
	if (text) {
		loadFont(fontResourceId);
		draw(kTextCenter, Common::Point(20, y), 16, 600, text);
	}
}

uint32 Text::draw(TextCentering centering, const Common::Point &point, int32 spacing, int32 width, const char *text) {
	return draw(0, 99, centering, point, spacing, width, text);
}

uint32 Text::draw(int32 a1, int32 a2, TextCentering centering, const Common::Point &point, int32 spacing, int32 width, const char *text) {
	if (!text || !*text)
		return 0;

	Common::Point coords = point;

	uint32 printed = 0;
	bool drawText = false;
	int32 spaceWidth = 0;
	int32 index = 0;

	const char *string = text;
	const char *endText = text;

	for (;;) {
label_start:

		int32 charWidth = 0;

		// Draw the text
		if (drawText) {
			char currentChar = *endText;

			if (index >= a1 && index <= (a1 + a2)) {
				switch (centering) {
				default:
				case kTextCalculate:
					break;

				case kTextCenter:
					drawCentered(coords, width, endText - string, string);
					break;

				case kTextNormal:
					setPosition(coords);
					draw(text, endText - text);
					break;
				}

				coords.y += spacing;
				++printed;
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

					if (c == 1) // Start of heading (SOH)
						break;

					charWidth += getWidth(c);
					txt++;
					c = txt[0];

				} while (c != ' ');
			}

			if ((w + charWidth) > width) {
				string = text;
				endText = txt2 - 1;
				drawText = true;
				goto label_start;
			}

			if (!*txt)
				break;

			if (*txt == 1 || *txt == 2) // Start of heading (SOH) or start of text (STX)
				break;

			++txt;
			spaceWidth = getWidth(' ');
		}

		string = text;
		endText = txt;
		drawText = true;
	}

	return printed;
}

void Text::drawCentered(const Common::Point &point, int32 width, const char *text) {
	setPosition(Common::Point(point.x + (width - getWidth(text)) / 2, point.y));
	draw(text);
}

void Text::drawCentered(const Common::Point &point, int32 width, uint32 length, const char *text) {
	setPosition(Common::Point(point.x + (width - getWidth(text, length)) / 2, point.y));
	draw(text, length);
}

void Text::drawCentered(const Common::Point &point, int32 width, ResourceId resourceId) {
	drawCentered(point, width, get(resourceId));
}

} // end of namespace Asylum
