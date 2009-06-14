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

#ifndef ASYLUM_TEXT_H_
#define ASYLUM_TEXT_H_

#include "asylum/asylum.h"
#include "asylum/resourcepack.h"

namespace Asylum {

class Text {
public:
	Text(AsylumEngine *vm);
	~Text();

    void loadFont(ResourcePack *resPack, uint32 resId);
    
    void setPosition(uint32 x, uint32 y);
    uint32 getWidth(uint8 *text);
    //uint32 getResWidth(uint32 resId);

    void drawChar(uint8 character);
    void drawText(uint8 *text);
    void drawResText(uint32 resId);

    void drawTextCentered(uint32 x, uint32 y, uint32 width, uint8 *text);  
    void drawResTextCentered(uint32 resId);

    void drawTextAlignRight(uint32 x, uint32 y, uint8 *text);
    void drawResTextAlignRight(uint32 x, uint32 y, uint32 resId);
private:
    AsylumEngine *_vm;
	GraphicResource *_fontResource;

	uint32 _posX;
    uint32 _posY;
    uint8 _curFontFlags;
};

} // end of namespace Asylum

#endif
