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
 * $URL$
 * $Id$
 *
 */

#include "prisoner/prisoner.h"
#include "prisoner/resource.h"
#include "prisoner/screen.h"

namespace Prisoner {

/* Fonts */

int16 PrisonerEngine::addFont(Common::String &pakName, int16 pakSlot, int16 outlineIndex, int16 inkIndex) {

	int16 fontIndex = _fonts.getFreeSlot();
	Font *font = &_fonts[fontIndex];

	font->resourceCacheSlot = _res->load<FontResource>(pakName, pakSlot, 7);

	font->fontResource = _res->get<FontResource>(font->resourceCacheSlot);
	font->interletter = 1;
	font->outlineColorIdx = outlineIndex;
	font->inkColorIdx = inkIndex;
	font->outlineColor = outlineIndex;
	font->inkColor = inkIndex;
	font->unk2 = 0;
	font->height = 0;
	font->unk1 = font->unk2 + font->height;

	// TODO: Set as active font

	return fontIndex;
}

void PrisonerEngine::unloadFont(int16 fontIndex) {
	Font *font = &_fonts[fontIndex];
	if (font->resourceCacheSlot != -1) {
		_res->unload(font->resourceCacheSlot);
		font->resourceCacheSlot = -1;
	}
}

void PrisonerEngine::unloadFonts() {
	for (int16 i = 0; i < _fonts.count(); i++)
		unloadFont(i);
}

void PrisonerEngine::clearFonts() {
	_fonts.clear();
	// TODO: Set active font to -1
}

void PrisonerEngine::getFontColors(int16 fontIndex, int16 &outlineColor, int16 &inkColor) {
	Font *font = &_fonts[fontIndex];
	outlineColor = font->outlineColor;
	inkColor = font->inkColor;
}

void PrisonerEngine::setFontColors(int16 fontIndex, int16 outlineColor, int16 inkColor) {
	Font *font = &_fonts[fontIndex];
	font->outlineColor = outlineColor;
	font->inkColor = inkColor;
}

void PrisonerEngine::setActiveFont(int16 fontIndex) {
	Font *font = &_fonts[fontIndex];
	_activeFontIndex = fontIndex;
	_screen->setFontColorTable(font->outlineColorIdx, font->outlineColor);
	_screen->setFontColorTable(font->inkColorIdx, font->inkColor);
}

void PrisonerEngine::drawText(int16 x, int16 y, const Common::String &text) {
	_screen->drawText(_fonts[_activeFontIndex].fontResource, text, x, y);
}

void PrisonerEngine::drawTextEx(int16 x1, int16 x2, int16 y1, int16 y2, const Common::String &text) {
	if (x2 != -1) {
		int16 newX = (x1 + x2 - getTextWidth(text)) / 2;
		if (newX >= x1)
			x1 = newX;
	}
	if (y2 != -1) {
		y1 = (y1 + y2 + getActiveFontUnk1()) / 2 - getActiveFontUnk2();
	}
	drawText(x1, y1, text);
}

int16 PrisonerEngine::getTextWidth(const Common::String &text) {
	return _screen->getTextWidth(_fonts[_activeFontIndex].fontResource, text);
}

int16 PrisonerEngine::getActiveFontUnk1() {
	return _fonts[_activeFontIndex].fontResource->getUnk1();
}

int16 PrisonerEngine::getActiveFontUnk2() {
	return _fonts[_activeFontIndex].fontResource->getUnk2();
}

int16 PrisonerEngine::getActiveFontHeight() {
	return _fonts[_activeFontIndex].fontResource->getHeight();
}

} // End of namespace Prisoner
