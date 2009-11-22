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

#include "asylum/text.h"
#include "common/endian.h"

namespace Asylum {

Text::Text(Screen *screen) : _screen(screen) {
	_posX = 0;
	_posY = 0;
	_curFontFlags = 0;
	_fontResource = 0;

	_textPack = new ResourcePack(0);
}

Text::~Text() {
	delete _textPack;
	delete _fontResource;
}

// loadFont at address 00435640
void Text::loadFont(ResourcePack *resPack, uint32 resId) {
	if (_fontResource && resId == _fontResource->getEntryNum())
		return;

	delete _fontResource;

	_fontResource = new GraphicResource(resPack, resId);

	if (resId > 0) {
		// load font flag data
		_curFontFlags = (_fontResource->getFlags() >> 4) & 0x0F;
	}
}

void Text::setTextPos(uint32 x, uint32 y) {
	_posX = x;
	_posY = y;
}

// getTextWidth at address 004357C0
uint32 Text::getTextWidth(char *text) {
	assert(_fontResource);

	int width = 0;
	uint8 character = *text;
	while (character) {
		GraphicFrame *font = _fontResource->getFrame(character);
		width += font->surface.w + font->x - _curFontFlags;

		text++;
		character = *text;
	}
	return width;
}

uint32 Text::getResTextWidth(uint32 resId) {
	ResourceEntry *textRes = _textPack->getResource(resId);
	return getTextWidth((char*)textRes->data);
}

char* Text::getResText(uint32 resId) {
	ResourceEntry *textRes = _textPack->getResource(resId);
	return (char*)textRes->data;
}

void Text::drawChar(char character) {
	assert(_fontResource);

	GraphicFrame *fontLetter = _fontResource->getFrame(character);
	_screen->copyRectToScreenWithTransparency((byte *)fontLetter->surface.pixels, fontLetter->surface.w, _posX, _posY + fontLetter->y, fontLetter->surface.w, fontLetter->surface.h);
	_posX += fontLetter->surface.w + fontLetter->x - _curFontFlags;
}

void Text::drawText(char *text) {
	while (*text) {
		drawChar(*text);
		text++;
	}
}

void Text::drawResText(uint32 resId) {
	ResourceEntry *textRes = _textPack->getResource(resId);
	drawText((char*)textRes->data);
}

void Text::drawTextCentered(uint32 x, uint32 y, uint32 width, char *text) {
	int textWidth = getTextWidth(text);
	setTextPos(x + (width - textWidth) / 2, y);
	drawText(text);
}

void Text::drawResTextWithValueCentered(uint32 x, uint32 y, uint32 width, uint32 resId, uint32 value) {
	ResourceEntry *textRes = _textPack->getResource(resId);
	char *text = (char *)textRes->data;
	char txt[100];
	sprintf(txt, text, value);
	drawTextCentered(x, y, width, txt);
}

void Text::drawResTextCentered(uint32 x, uint32 y, uint32 width, uint32 resId) {
	ResourceEntry *textRes = _textPack->getResource(resId);
	drawTextCentered(x, y, width, (char *)textRes->data);
}

void Text::drawText(uint32 x, uint32 y, char *text) {
    if (text) {
	    int textWidth = getTextWidth(text);
	    setTextPos(x - textWidth, y);
	    drawText(text);
    }
}

void Text::drawResText(uint32 x, uint32 y, uint32 resId) {
	ResourceEntry *textRes = _textPack->getResource(resId);
	drawText(x, y, (char *)textRes->data);
}

void Text::drawTextAlignRight(uint32 x, uint32 y, char *text) {
	int textWidth = getTextWidth(text);
	setTextPos(x - textWidth, y);
	drawText(text);
}

void Text::drawResTextAlignRight(uint32 x, uint32 y, uint32 resId) {
	ResourceEntry *textRes = _textPack->getResource(resId);
	drawTextAlignRight(x, y, (char *)textRes->data);
}

} // end of namespace Asylum
