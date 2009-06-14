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
    _curFontResId = 0;
    _curFontFlags = 0;
}

Text::~Text() {
    if(_resPack){
        delete _resPack;
    }
}

// loadFont at address 00435640
uint32 Text::loadFont(uint32 resId){
    // FIXME this should be handle outside. We don't need to refer the file here.
    uint32 offset = resId & 0xFFFF;
    _resPack = new ResourcePack("res.001");
    // end fixme

    uint32 oldFontResId = _curFontResId;
    _curFontResId = resId;
    if(resId > 0){
        // load font flag data
	    uint32 flag = READ_UINT32(_resPack->getResource(offset)->data + 4);
        _curFontFlags = (flag >> 4) & 0x0F; 
    }

    return oldFontResId;
}
    
void Text::setPosition(uint32 x, uint32 y){
    _posX = x;
    _posY = y;
}

// getTextWidth at address 004357C0
uint32 Text::getWidth(uint8 *text){
    int width = 0;
    uint8 character = *text;
    while(character){
        // FIXME this should be masked inside resources
        uint32 fileNum = (_curFontResId >> 16) & 0x7FFF;
        uint32 offset = _curFontResId & 0xFFFF;

		// TODO
        //GraphicResource *font = _resMgr->getGraphic(fileNum, offset, character);
        //width += font->width + font->x - _curFontFlags;

        text++;
        character = *text;
    }
    return 0;
}

uint32 Text::getResWidth(uint32 resId){
    uint32 offset = resId & 0xFFFF;
    // FIXME this shouldn't be here. We need a static getResource by resId
    ResourcePack *res0 = new ResourcePack("res.000");

    uint8* text = res0->getResource(offset)->data;
    return getWidth(text);
}

void Text::drawChar(uint8 character){
    // FIXME this should be masked inside resources
    uint32 fileNum = (_curFontResId >> 16) & 0x7FFF;
    uint32 offset = _curFontResId & 0xFFFF;
    // FIXME this shouldn't be here. We need a static getResource by resId
	// TODO
    //GraphicResource *font = _resMgr->getGraphic(fileNum, offset, character);
	//_vm->getScreen()->copyToBackBuffer(font->data, 0, 0, font->width, font->height);
    //_posX += font->width + font->x - _curFontFlags;
}

void Text::drawText(uint8 *text){
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
