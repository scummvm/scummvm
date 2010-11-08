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

#ifndef ASYLUM_TEXT_H
#define ASYLUM_TEXT_H

#include "asylum/shared.h"

#include "common/scummsys.h"

namespace Asylum {

class AsylumEngine;
class GraphicResource;
class ResourcePack;

class Text {
public:
	Text(AsylumEngine *engine);
	~Text();

	void loadFont(ResourceId resourceId);

	void  setPosition(int32 x, int32 y);
	int32 getWidth(const char *text);
	int32 getWidth(ResourceId resourceId);
    char *get(ResourceId resourceId);

	void drawChar(unsigned char character);
	void drawText(const char *text);
	void drawText(int32 x, int32 y, const char *text);
	void drawResText(ResourceId resourceId);
	void drawResText(int32 x, int32 y, ResourceId resourceId);

	void drawTextCentered(int32 x, int32 y, int32 width, const char *text);
	void drawResTextCentered(int32 x, int32 y, int32 width, ResourceId resourceId);
	void drawResTextWithValueCentered(int32 x, int32 y, int32 width, ResourceId resourceId, int32 value);

	void drawTextAlignRight(int32 x, int32 y, const char *text);
	void drawResTextAlignRight(int32 x, int32 y, ResourceId resourceId);

private:
	AsylumEngine *_vm;

	GraphicResource *_fontResource;

	int32 _posX;
	int32 _posY;
	uint8  _curFontFlags;

}; // end of class Text

} // end of namespace Asylum

#endif
