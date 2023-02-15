/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/rational.h"
#include "common/unicode-bidi.h"

#include "asylum/system/text.h"

#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"

#include "asylum/asylum.h"
#include "asylum/respack.h"

namespace Asylum {

Text::Text(AsylumEngine *engine) : _vm(engine) {
	_curFontFlags = 0;
	_fontResource = nullptr;
	_transTableNum = 0;
}

Text::~Text() {
	delete _fontResource;
}

ResourceId Text::loadFont(ResourceId resourceId) {
	if (_fontResource && resourceId == _fontResource->getResourceId())
		return resourceId;

	ResourceId previousFont = _fontResource ? _fontResource->getResourceId() : kResourceNone;

	delete _fontResource;
	_fontResource = nullptr;

	if (resourceId != kResourceNone) {
		_fontResource = new GraphicResource(_vm, resourceId);
		_curFontFlags = Common::Rational(_fontResource->getData().flags, 16).toInt() & 0x0F;
	}

	return previousFont;
}

void Text::setPosition(const Common::Point &point) {
	_position = point;
}

int16 Text::getWidth(char c) {
	if (!_fontResource)
		error("[Text::getWidth] Font not initialized properly");

	GraphicFrame *font = _fontResource->getFrame((uint8)c);

	return (int16)(font->surface.w + font->x - _curFontFlags);
}

int16 Text::getWidth(const char *text) {
	if (!_fontResource)
		error("[Text::getWidth] font resource hasn't been loaded yet!");

	int16 width = 0;
	char character = *text;

	while (character) {
		GraphicFrame *font = _fontResource->getFrame((uint8)character);
		width += (int16)(font->surface.w + font->x - _curFontFlags);

		text++;
		character = *text;
	}

	return width;
}

int16 Text::getWidth(const char *text, int16 length) {
	if (!_fontResource)
		error("[Text::getWidth] font resource hasn't been loaded yet!");

	if (length == 0)
		return 0;

	int16 width = 0;
	char character = *text;

	while (character && length > 0) {
		GraphicFrame *font = _fontResource->getFrame((uint8)character);
		width += (int16)(font->surface.w + font->x - _curFontFlags);

		text++;
		character = *text;
		length--;
	}

	return width;
}

int16 Text::getWidth(ResourceId resourceId) {
	return getWidth(get(resourceId));
}

char *Text::get(ResourceId resourceId) {
	ResourceEntry *textRes = getResource()->get(resourceId);
	return (char *)textRes->data;
}

void Text::drawChar(char character) {
	if (!_fontResource)
		error("[Text::drawChar] font resource hasn't been loaded yet!");

	if (_transTableNum) {
		getScreen()->drawTransparent(_fontResource, (uint8)character, _position, kDrawFlagNone, _transTableNum);
	} else {
		getScreen()->draw(_fontResource, (uint8)character, _position);
	}

	GraphicFrame *fontLetter = _fontResource->getFrame((uint8)character);
	_position.x += (int16)fontLetter->surface.w + fontLetter->x - _curFontFlags;
}

void Text::draw(const char *text) {
	if (!text)
		return;

	if (_vm->getLanguage() == Common::HE_ISR)
		text = Common::convertBiDiString(text, Common::kWindows1255).c_str();
	while (*text) {
		drawChar(text[0]);
		text++;
	}
}

void Text::draw(const char *text, int16 length) {
	if (length == 0)
		return;

	if (!text)
		return;

	if (_vm->getLanguage() == Common::HE_ISR)
		text = Common::convertBiDiString(Common::String(text, length), Common::kWindows1255).c_str();
	for (int16 i = 0; i < length; i++)
		drawChar(text[i]);
}

void Text::draw(ResourceId resourceId) {
	ResourceEntry *textRes = getResource()->get(resourceId);
	draw((char *)textRes->data);
}

void Text::draw(const Common::Point &point, const char *text) {
	setPosition(Common::Point(point.x - (int16)getWidth(text), point.y));
	draw(text);
}

void Text::draw(const Common::Point &point, ResourceId resourceId) {
	draw(point, get(resourceId));
}

void Text::draw(const char *text, ResourceId fontResourceId, int16 y) {
	if (text) {
		loadFont(fontResourceId);
		draw(kTextCenter, Common::Point(20, y), 16, 600, text);
	}
}

int16 Text::draw(TextCentering centering, const Common::Point &point, int16 spacing, int16 width, const char *text) {
	return draw(0, 99, centering, point, spacing, width, text);
}

int16 Text::draw(int16 a1, int16 a2, TextCentering centering, const Common::Point &point, int16 spacing, int16 width, const char *text) {
	if (!text || !*text)
		return 0;

	Common::Point coords = point;

	int16 printed = 0;
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
					drawCentered(coords, width, (int16)(endText - string), string);
					break;

				case kTextNormal:
					setPosition(coords);
					draw(text, (int16)(endText - text));
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

void Text::drawCentered(const Common::Point &point, int16 width, const char *text) {
	setPosition(Common::Point(point.x + (width - getWidth(text)) / 2, point.y));
	draw(text);
}

void Text::drawCentered(const Common::Point &point, int16 width, int16 length, const char *text) {
	setPosition(Common::Point(point.x + (width - getWidth(text, length)) / 2, point.y));
	draw(text, length);
}

void Text::drawCentered(const Common::Point &point, int16 width, ResourceId resourceId) {
	drawCentered(point, width, get(resourceId));
}

} // end of namespace Asylum
