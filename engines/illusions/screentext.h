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
 */

#ifndef ILLUSIONS_SCREENTEXT_H
#define ILLUSIONS_SCREENTEXT_H

#include "illusions/graphics.h"
#include "common/list.h"
#include "common/rect.h"
#include "graphics/surface.h"

namespace Illusions {

#define TEXT_FLAG_LEFT_ALIGN 1
#define TEXT_FLAG_CENTER_ALIGN 2
#define TEXT_FLAG_RIGHT_ALIGN 4
#define TEXT_FLAG_BORDER_DECORATION 24

class IllusionsEngine;
class FontResource;

struct ScreenTextInfo {
	Common::Point _position;
	WidthHeight _dimensions;
	Common::Point _offsPt;
	uint32 _fontId;
	uint16 _backgroundColor;
	uint16 _borderColor;
	byte _colorR, _colorG, _colorB;
	uint _flags;
};

struct ScreenTextEntry {
	ScreenTextInfo _info;
	uint16 _text[1024];
};

class ScreenText {
public:
	ScreenText(IllusionsEngine *vm);
	~ScreenText();
	void getTextInfoDimensions(WidthHeight &textInfoDimensions);
	void getTextInfoPosition(Common::Point &position);
	void setTextInfoPosition(Common::Point position);
	void updateTextInfoPosition(Common::Point position);
	void clipTextInfoPosition(Common::Point &position);
	bool refreshScreenText(FontResource *font, WidthHeight dimensions, Common::Point offsPt,
		uint16 *text, uint textFlags, uint16 backgroundColor, uint16 borderColor, uint16 *&outTextPtr);
	bool insertText(uint16 *text, uint32 fontId, WidthHeight dimensions, Common::Point offsPt, uint flags,
		uint16 backgroundColor, uint16 borderColor, byte colorR, byte colorG, byte colorB, uint16 *&outTextPtr);
	void removeText();
	void clearText();
public:
	IllusionsEngine *_vm;
	Common::Point _position;
	WidthHeight _dimensions;
	Graphics::Surface *_surface;
	Common::List<ScreenTextEntry*> _screenTexts;
	void freeTextSurface();
};

} // End of namespace Illusions

#endif // ILLUSIONS_SCREENTEXT_H
