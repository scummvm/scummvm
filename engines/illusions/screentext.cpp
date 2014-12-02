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
#include "illusions/screentext.h"
#include "illusions/dictionary.h"
#include "illusions/resources/fontresource.h"
#include "illusions/screen.h"
#include "illusions/textdrawer.h"
#include "engines/util.h"
#include "graphics/palette.h"

namespace Illusions {

ScreenText::ScreenText(IllusionsEngine *vm)
	: _vm(vm), _surface(0) {
}

ScreenText::~ScreenText() {
}

void ScreenText::getTextInfoDimensions(WidthHeight &textInfoDimensions) {
	textInfoDimensions = _dimensions;
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
	// TODO Set min/max for BBDOU
	if (position.x < 2)
		position.x = 2;
	else if (position.x + _dimensions._width > 318)
		position.x = 318 - _dimensions._width;
	if (position.y < 2)
		position.y = 2;
	else if (position.y + _dimensions._height > 198)
		position.y = 198 - _dimensions._height;
}

bool ScreenText::refreshScreenText(FontResource *font, WidthHeight dimensions, Common::Point offsPt,
	uint16 *text, uint textFlags, uint16 color2, uint16 color1, uint16 *&outTextPtr) {
	TextDrawer textDrawer;
	bool done = textDrawer.wrapText(font, text, &dimensions, offsPt, textFlags, outTextPtr);
	_surface = _vm->_screen->allocSurface(dimensions._width, dimensions._height);
	_dimensions = dimensions;
	textDrawer.drawText(_vm->_screen, _surface, color2, color1);
	return done;
}

bool ScreenText::insertText(uint16 *text, uint32 fontId, WidthHeight dimensions, Common::Point offsPt, uint flags,
	uint16 color2, uint16 color1, byte colorR, byte colorG, byte colorB, uint16 *&outTextPtr) {
	
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
	screenText->_info._color2 = color2;
	screenText->_info._color1 = color1;
	screenText->_info._colorR = colorR;
	screenText->_info._colorG = colorG;
	screenText->_info._colorB = colorB;
	_screenTexts.push_back(screenText);

	FontResource *font = _vm->_dict->findFont(screenText->_info._fontId);
	bool done = refreshScreenText(font, screenText->_info._dimensions, screenText->_info._offsPt,
		text, screenText->_info._flags, screenText->_info._color2, screenText->_info._color1,
		outTextPtr);
	debug("font->getColorIndex(): %d", font->getColorIndex());
	_vm->_screen->setPaletteEntry(font->getColorIndex(), screenText->_info._colorR, screenText->_info._colorG, screenText->_info._colorB);

	uint16 *textPart = screenText->_text;
	while (text != outTextPtr)
		*textPart++ = *text++;
	*textPart = 0;

	updateTextInfoPosition(Common::Point(160, 100));

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
				screenText->_text, screenText->_info._flags, screenText->_info._color2, screenText->_info._color1,
				outTextPtr);
			_vm->_screen->setPaletteEntry(font->getColorIndex(), screenText->_info._colorR, screenText->_info._colorG, screenText->_info._colorB);
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
		_surface = 0;
	}
}

} // End of namespace Illusions
