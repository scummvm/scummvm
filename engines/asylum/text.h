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

#ifndef ASYLUM_TEXT_H_
#define ASYLUM_TEXT_H_

#include "asylum/screen.h"
#include "asylum/respack.h"

namespace Asylum {

class Screen;

class Text {
public:
	Text(Screen *screen);
	~Text();

	void loadFont(ResourcePack *resPack, uint32 resId);

	void setTextPos(uint32 x, uint32 y);
	uint32 getTextWidth(char *text);
	uint32 getResTextWidth(uint32 resId);

	void drawChar(char character);
	void drawText(char *text);
	void drawText(uint32 x, uint32 y, char *text);
	void drawResText(uint32 resId);
	void drawResText(uint32 x, uint32 y, uint32 resId);

	void drawTextCentered(uint32 x, uint32 y, uint32 width, char *text);
	void drawResTextCentered(uint32 x, uint32 y, uint32 width, uint32 resId);
	void drawResTextWithValueCentered(uint32 x, uint32 y, uint32 width, uint32 resId, uint32 value);

	void drawTextAlignRight(uint32 x, uint32 y, char *text);
	void drawResTextAlignRight(uint32 x, uint32 y, uint32 resId);

private:
	Screen			*_screen;
	GraphicResource *_fontResource;
	ResourcePack	*_textPack;

	uint32 _posX;
	uint32 _posY;
	uint8  _curFontFlags;

}; // end of class Text

} // end of namespace Asylum

#endif
