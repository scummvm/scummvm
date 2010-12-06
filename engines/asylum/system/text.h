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

enum TextCentering {
	kTextNormal,
	kTextCenter,
	kTextCalculate
};

class Text {
public:
	Text(AsylumEngine *engine);
	~Text();

	void loadFont(ResourceId resourceId);

	void   setPosition(int32 x, int32 y);
	int32  getWidth(char c);
	int32  getWidth(const char *text);
	int32  getWidth(const char *text, uint32 length);
	int32  getWidth(ResourceId resourceId);
    char  *get(ResourceId resourceId);

	void drawChar(char character);
	void draw(const char *text);
	void draw(int32 x, int32 y, const char *text);
	void draw(ResourceId resourceId);
	void draw(int32 x, int32 y, ResourceId resourceId);
	void draw(const char *text, ResourceId fontResourceId, int32 y);
	void draw(const char *text, uint32 length);
	uint32 draw(TextCentering centering, int32 x, int32 y, int32 spacing, int32 width, const char *text);
	uint32 draw(int32 a1, int32 a2, TextCentering centering, int32 x, int32 y, int32 spacing, int32 width, const char *text);

	void drawCentered(int32 x, int32 y, int32 width, const char *text);
	void drawCentered(int32 x, int32 y, int32 width, ResourceId resourceId);
	void drawCentered(int32 x, int32 y, int32 width, uint32 length, const char *text);

	void setTransTableNum(int32 val) { _transTableNum = val; }

private:
	AsylumEngine *_vm;

	GraphicResource *_fontResource;

	int32 _transTableNum;

	int32 _posX;
	int32 _posY;
	uint8 _curFontFlags;

};

} // end of namespace Asylum

#endif
