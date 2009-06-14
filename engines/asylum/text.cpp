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

namespace Asylum {

Text::Text() {
}

Text::~Text() {
}

uint32 Text::loadFont(uint32 resId){
    return 0;
}
    
uint32 Text::setPosition(uint32 x, uint32 y){
    return 0;
}

uint32 Text::getWidth(uint8 *text){
    return 0;
}

uint32 Text::getResWidth(uint32 resId){
    return 0;
}

void Text::drawChar(uint8 c){
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
