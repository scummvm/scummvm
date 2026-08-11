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

#if defined(USE_FREETYPE2)
#include "graphics/fonts/ttf.h"
#endif

namespace Asylum {

Text::Text(AsylumEngine *engine) : _vm(engine), _fontResource(nullptr), _transTableNum(0), _curFontFlags(0), _chineseFontLoadAttempted(false) {
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

void Text::loadChineseFont() {
	if (_chineseFontLoadAttempted)
		return;

	_chineseFontLoadAttempted = true;

#if defined(USE_FREETYPE2)
	_chineseFont.reset(Graphics::loadTTFFontFromArchive("NotoSansSC-Regular.otf", 16, Graphics::kTTFSizeModeCharacter, 0, 0, Graphics::kTTFRenderModeLight));
#endif
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

int16 Text::getChineseWidth(const Common::U32String &utext) {
	loadChineseFont();
	if (!_chineseFont)
		return 0;

	return _chineseFont->getStringWidth(utext);
}

int16 Text::getWidth(const char *text) {
	if (!_fontResource)
		error("[Text::getWidth] font resource hasn't been loaded yet!");

	if (_vm->getLanguage() == Common::Language::ZH_CHN) {
		return getChineseWidth(Common::U32String(text, Common::CodePage::kGBK));
	}

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

	if (_vm->getLanguage() == Common::Language::ZH_CHN) {
		return getChineseWidth(Common::U32String(text, length, Common::CodePage::kGBK));
	}

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

void Text::drawChinese(const Common::U32String &utext) {
	loadChineseFont();
	if (!_chineseFont)
		return;
	Graphics::Surface *surf = getScreen()->getSurface();
	uint8 color = 0;
	// TODO: Add more colors
	switch ((uint32_t)_fontResource->getResourceId()) {
	case 0: // Case to quiet VS C4065 warning
	default:
		debug(5, "Unrecognized font resource 0x%x for string %s", _fontResource->getResourceId(), utext.encode().c_str());
		color = 1;
		break;
	case 0x80010039:
		color = 0xff;
		break;
	case 0x8005000d:
		color = 0x10;
		break;
	case 0x8005000e:
		color = 0x25;
		break;
	case 0x8005000f:
		color = 0x1f;
		break;
	case 0x80120012:
		color = 0x69;
		break;		
	}

	_chineseFont->drawString(surf, utext, _position.x, _position.y, surf->w - _position.x, color);
	_position.x += _chineseFont->getStringWidth(utext);
}

void Text::draw(const char *text) {
	if (!text)
		return;

	if (_vm->getLanguage() == Common::Language::ZH_CHN) {
		drawChinese(Common::U32String(text, Common::CodePage::kGBK));
		return;
	}

	Common::String textRef;

	if (_vm->getLanguage() == Common::HE_ISR) {
		textRef = Common::convertBiDiString(text, Common::kWindows1255);
		text = textRef.c_str();
	}
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

	if (_vm->getLanguage() == Common::Language::ZH_CHN) {
		drawChinese(Common::U32String(text, length, Common::CodePage::kGBK));
		return;
	}

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

	// TODO: Make non-Chinese into Graphics::Font as well.
	if (_vm->getLanguage() == Common::Language::ZH_CHN) {
		Common::Array<Common::U32String> lines;
		Common::Point coords = point;
		int16 printed = 0;

		loadChineseFont();
		if (!_chineseFont)
			return 0;
		char *buf = scumm_strdup(text);
		for (char *ptr = buf; *ptr; ptr++)
			if (*ptr == 1 || *ptr == 2) // Start of heading (SOH) or start of text (STX)
				*ptr = '\n';
		Common::U32String utext(buf, Common::CodePage::kGBK);
		free(buf);

		_chineseFont->wordWrapText(utext, width, lines);

		for (int index = a1; index <= (a1 + a2) && index < (int)lines.size(); index++) {
			switch (centering) {
			default:
			case kTextCalculate:
				break;

			case kTextCenter:
				setPosition(coords + Common::Point((width - getChineseWidth(lines[index])) / 2, 0));
				drawChinese(lines[index]);
				break;

			case kTextNormal:
				setPosition(coords);
				drawChinese(lines[index]);
				break;
			}

			coords.y += spacing;
			++printed;
		}

		return printed;
	}

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
