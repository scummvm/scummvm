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

#include "asylum/text.h"
#include "common/endian.h"

namespace Asylum {

Text::Text(AsylumEngine *vm) : _vm(vm) {
    _posX = 0;
    _posY = 0;
    _curFontFlags = 0;
	_fontResource = 0;
}

Text::~Text() {
	delete _fontResource;
}

// loadFont at address 00435640
void Text::loadFont(ResourcePack *resPack, uint32 resId) {
	ResourceEntry *fontRes = resPack->getResourceFromId(resId);
	_fontResource = new GraphicResource(fontRes->data, fontRes->size);

    if (resId > 0) {
        // load font flag data
        _curFontFlags = (_fontResource->getFlags() >> 4) & 0x0F; 
    }
}
    
void Text::setPosition(uint32 x, uint32 y) {
    _posX = x;
    _posY = y;
}

// getTextWidth at address 004357C0
uint32 Text::getWidth(uint8 *text) {
	assert (_fontResource);

    int width = 0;
    uint8 character = *text;
    while (character){
		GraphicFrame *font = _fontResource->getFrame(character);
        width += font->surface.w + font->x - _curFontFlags;

        text++;
        character = *text;
    }
    return 0;
}

#if 0
// TODO: Is this needed at all?
uint32 Text::getResWidth(uint32 resId) {
    uint32 offset = resId & 0xFFFF;
    // FIXME this shouldn't be here. We need a static getResource by resId
    ResourcePack *res0 = new ResourcePack("res.000");

    uint8* text = res0->getResource(offset)->data;
    return getWidth(text);
}
#endif

void Text::drawChar(byte character){
	assert (_fontResource);

	GraphicFrame *fontLetter = _fontResource->getFrame(character);
	_vm->getScreen()->copyRectToScreenWithTransparency((byte *)fontLetter->surface.pixels, _posX, _posY, fontLetter->surface.w, fontLetter->surface.h);
    _posX += fontLetter->surface.w + fontLetter->x - _curFontFlags;
}

void Text::drawText(char *text){
    while (*text) {
        drawChar(*text);
        text++;
    }
}

void Text::drawResText(uint32 resId){
}

void Text::drawTextCentered(uint32 x, uint32 y, uint32 width, uint8 *text){
}  

void Text::drawResTextCentered(uint32 resId){
}

void Text::drawTextAlignRight(uint32 x, uint32 y, uint8 *text){
}

void Text::drawResTextAlignRight(uint32 x, uint32 y, uint32 resId){
}

} // end of namespace Asylum
