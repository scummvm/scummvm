/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ASYLUM_SYSTEM_TEXT_H
#define ASYLUM_SYSTEM_TEXT_H

#include "common/rect.h"
#include "common/scummsys.h"

#include "asylum/shared.h"

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

	ResourceId loadFont(ResourceId resourceId);

	void   setPosition(const Common::Point &point);
	int16  getWidth(char c);
	int16  getWidth(const char *text);
	int16  getWidth(const char *text, int16 length);
	int16  getWidth(ResourceId resourceId);
	char  *get(ResourceId resourceId);

	void drawASCII(char character) { drawChar(character); }
	void draw(const char *text);
	void draw(const Common::Point &point, const char *text);
	void draw(ResourceId resourceId);
	void draw(const Common::Point &point, ResourceId resourceId);
	void draw(const char *text, ResourceId fontResourceId, int16 y);
	void draw(const char *text, int16 length);
	int16 draw(TextCentering centering, const Common::Point &point, int16 spacing, int16 width, const char *text);
	int16 draw(int16 a1, int16 a2, TextCentering centering, const Common::Point &point, int16 spacing, int16 width, const char *text);

	void drawCentered(const Common::Point &point, int16 width, const char *text);
	void drawCentered(const Common::Point &point, int16 width, ResourceId resourceId);
	void drawCentered(const Common::Point &point, int16 width, int16 length, const char *text);

	void setTransTableNum(uint32 val) { _transTableNum = val; }

private:
	void drawChar(char character);

	AsylumEngine *_vm;

	GraphicResource *_fontResource;

	uint32 _transTableNum;

	Common::Point _position;
	uint8 _curFontFlags;

};

} // end of namespace Asylum

#endif // ASYLUM_SYSTEM_TEXT_H
