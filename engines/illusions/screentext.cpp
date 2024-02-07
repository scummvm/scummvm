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

#include "illusions/illusions.h"
#include "illusions/screentext.h"
#include "illusions/dictionary.h"
#include "illusions/resources/fontresource.h"
#include "illusions/screen.h"
#include "illusions/textdrawer.h"
#include "engines/util.h"

namespace Illusions {

ScreenText::ScreenText(IllusionsEngine *vm)
	: _vm(vm), _surface(nullptr) {
}

ScreenText::~ScreenText() {
	freeTextSurface();
}

void ScreenText::getTextInfoDimensions(WidthHeight &textInfoDimensions) {
	textInfoDimensions = _dimensions;
}

void ScreenText::getTextInfoPosition(Common::Point &position) {
	position = _position;
}

void ScreenText::setTextInfoPosition(Common::Point position) {
	_position = position;
	clipTextInfoPosition(_position);
}

void ScreenText::updateTextInfoPosition(Common::Point position) {
	WidthHeight dimensions;
	getTextInfoDimensions(dimensions);
	position.x = position.x - dimensions._width / 2;
	position.y = position.y - dimensions._height / 2;
	setTextInfoPosition(position);
}

void ScreenText::clipTextInfoPosition(Common::Point &position) {
	if (_vm->getGameId() == kGameIdBBDOU) {
		// BBDOU
		if (position.x < 2)
			position.x = 2;
		else if (position.x + _dimensions._width > 638)
			position.x = 638 - _dimensions._width;
		if (position.y < 2)
			position.y = 2;
		else if (position.y + _dimensions._height > 478)
			position.y = 478 - _dimensions._height;
	} else {
		// Duckman
		if (position.x < 2)
			position.x = 2;
		else if (position.x + _dimensions._width > 318)
			position.x = 318 - _dimensions._width;
		if (position.y < 2)
			position.y = 2;
		else if (position.y + _dimensions._height > 198)
			position.y = 198 - _dimensions._height;
	}
}

bool ScreenText::refreshScreenText(FontResource *font, WidthHeight dimensions, Common::Point offsPt,
	uint16 *text, uint textFlags, uint16 backgroundColor, uint16 borderColor, uint16 *&outTextPtr) {
	TextDrawer textDrawer;
	bool done = textDrawer.wrapText(font, text, &dimensions, offsPt, textFlags, outTextPtr);
	if (textFlags & TEXT_FLAG_BORDER_DECORATION) {
		dimensions._width += 11;
		dimensions._height += 14;
	}
	_surface = _vm->_screen->allocSurface(dimensions._width, dimensions._height);
	_surface->fillRect(Common::Rect(0, 0, _surface->w, _surface->h), _vm->_screen->getColorKey1());
	_dimensions = dimensions;
	if (_vm->getGameId() == kGameIdBBDOU) {
		if (backgroundColor == 0) {
			backgroundColor = _vm->_screen->getColorKey1();
			borderColor = g_system->getScreenFormat().RGBToColor(128, 128, 128);
		} else if (backgroundColor == 218) {
			backgroundColor = g_system->getScreenFormat().RGBToColor(50, 50, 180);
			borderColor = 0;
		} else {
			borderColor = g_system->getScreenFormat().RGBToColor(128, 128, 128);
		}
	}
	textDrawer.drawText(_vm->_screen, _surface, backgroundColor, borderColor);
	return done;
}

bool ScreenText::insertText(uint16 *text, uint32 fontId, WidthHeight dimensions, Common::Point offsPt, uint flags,
	uint16 backgroundColor, uint16 borderColor, byte colorR, byte colorG, byte colorB, uint16 *&outTextPtr) {

	if (!_screenTexts.empty()) {
		ScreenTextEntry *screenText = _screenTexts.back();
		screenText->_info._position = _position;
		freeTextSurface();
	}

	ScreenTextEntry *screenText = new ScreenTextEntry();
	screenText->_info._fontId = fontId;
	screenText->_info._dimensions = dimensions;
	screenText->_info._offsPt = offsPt;
	screenText->_info._flags = 0;
	if (flags & 8)
		screenText->_info._flags |= 8;
	if (flags & 0x10)
		screenText->_info._flags |= 0x10;
	if (flags & 1)
		screenText->_info._flags |= 1;
	else
		screenText->_info._flags |= 2;
	screenText->_info._backgroundColor = backgroundColor;
	screenText->_info._borderColor = borderColor;
	screenText->_info._colorR = colorR;
	screenText->_info._colorG = colorG;
	screenText->_info._colorB = colorB;
	_screenTexts.push_back(screenText);

	FontResource *font = _vm->_dict->findFont(screenText->_info._fontId);
	bool done = refreshScreenText(font, screenText->_info._dimensions, screenText->_info._offsPt,
		text, screenText->_info._flags, screenText->_info._backgroundColor, screenText->_info._borderColor,
		outTextPtr);
	_vm->_screenPalette->setPaletteEntry(font->getColorIndex(), screenText->_info._colorR, screenText->_info._colorG, screenText->_info._colorB);

	uint16 *textPart = screenText->_text;
	while (text != outTextPtr) {
		*textPart++ = *text++;
	}
	*textPart = 0;

	if (_vm->getGameId() == kGameIdBBDOU) {
		updateTextInfoPosition(Common::Point(320, 240));
	} else {
		updateTextInfoPosition(Common::Point(160, 100));
	}

	return done;
}

void ScreenText::removeText() {
	freeTextSurface();

	if (!_screenTexts.empty()) {
		ScreenTextEntry *screenText = _screenTexts.back();
		delete screenText;
		_screenTexts.pop_back();
	}

	if (!_screenTexts.empty()) {
		ScreenTextEntry *screenText = _screenTexts.back();
		if (screenText->_info._fontId) {
			uint16 *outTextPtr;
			FontResource *font = _vm->_dict->findFont(screenText->_info._fontId);
			refreshScreenText(font, screenText->_info._dimensions, screenText->_info._offsPt,
				screenText->_text, screenText->_info._flags, screenText->_info._backgroundColor, screenText->_info._borderColor,
				outTextPtr);
			_vm->_screenPalette->setPaletteEntry(font->getColorIndex(), screenText->_info._colorR, screenText->_info._colorG, screenText->_info._colorB);
			setTextInfoPosition(screenText->_info._position);
		}
	}

}

void ScreenText::clearText() {

	if (!_screenTexts.empty()) {
		ScreenTextEntry *screenText = _screenTexts.back();
		screenText->_info._position = _position;
		freeTextSurface();
	}

	ScreenTextEntry *screenText = new ScreenTextEntry();
	screenText->_info._fontId = 0;
	_screenTexts.push_back(screenText);

}

void ScreenText::freeTextSurface() {
	if (_surface) {
		_surface->free();
		delete _surface;
		_surface = nullptr;
	}
}

} // End of namespace Illusions
