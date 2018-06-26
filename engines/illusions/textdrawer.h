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

#ifndef ILLUSIONS_TEXTDRAWER_H
#define ILLUSIONS_TEXTDRAWER_H

#include "illusions/graphics.h"
#include "illusions/resources/fontresource.h"
#include "common/array.h"
#include "common/rect.h"
#include "graphics/surface.h"

namespace Illusions {

class IllusionsEngine;

struct TextLine {
	uint16 *_text;
	int16 _length;
	int16 _x, _y;
	TextLine() : _text(0) {}
	TextLine(uint16 *text, int16 length, int16 x, int16 y)
		: _text(text), _length(length), _x(x), _y(y) {}
};

class TextDrawer {
public:
	bool wrapText(FontResource *font, uint16 *text, WidthHeight *dimensions, Common::Point offsPt,
		uint textFlags, uint16 *&outTextPtr);
	void drawText(Screen *screen, Graphics::Surface *surface, uint16 backgroundColor, uint16 borderColor);
protected:
	FontResource *_font;
	uint16 *_text;
	WidthHeight *_dimensions;
	Common::Point _offsPt;
	uint _textFlags;
	Graphics::Surface *_surface;

	Common::Array<TextLine> _textLines;

	bool textHasChar(uint16 c);
	int16 getSpaceWidth();
	int16 getCharWidth(uint16 c);
	bool wrapTextIntern(int16 x, int16 y, int16 maxWidth, int16 maxHeight, uint16 *&outTextPtr);
};

} // End of namespace Illusions

#endif // ILLUSIONS_TALKRESOURCE_H
