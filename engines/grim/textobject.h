/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#ifndef GRIM_TEXTOBJECT_H
#define GRIM_TEXTOBJECT_H

#include "engines/grim/font.h"
#include "engines/grim/gfx_base.h"

namespace Grim {

class SaveGame;

struct TextObjectDefaults {
	Color fgColor;
	int x, y;
	int width, height;
	int justify;
	bool disabled;
	Font *font;
};

#define TEXT_NULL   ' '

extern TextObjectDefaults sayLineDefaults;
extern TextObjectDefaults printLineDefaults;
extern TextObjectDefaults blastTextDefaults;

class TextObject {
public:
	TextObject(bool blastDraw);
	~TextObject();
	void createBitmap();
	void destroyBitmap();
	void setDefaults(TextObjectDefaults *defaults);
	void setText(const char *text);
	void setX(int x) { _x = x; }
	void setY(int y) { _y = y; }
	void subBaseOffsetY() {
		if (_font)
			_y -= _font->getBaseOffsetY();
		else
			_y -= 5;
	}
	int getBaseOffsetY() {
		if (_font)
			return _font->getBaseOffsetY();
		else
			return 5;
	}
	void setWidth(int width) { _width = width; }
	void setHeight(int height) { _height = height; }
	void setFGColor(Color *fgColor) { _fgColor = fgColor; }
	void setFont(Font *font) { _font = font; }
	void setJustify(int justify) { _justify = justify; }
	void setDisabled(bool disabled) { _disabled = disabled; }
	int getBitmapWidth();
	int getBitmapHeight();
	int getTextCharPosition(int pos);
	void saveState(SaveGame *savedState);

	const char *name() const { return _textID; }
	void draw();

	enum Justify {
		NONE,
		CENTER,
		LJUSTIFY,
		RJUSTIFY
	};

protected:
	bool _created;
	Color _fgColor;
	int _x, _y;
	int _width, _height;
	int _justify, _numberLines;
	bool _disabled;
	bool _blastDraw;
	Font *_font;
	char _textID[256];
	uint8 *_textBitmap;
	int *_bitmapWidthPtr;
	GfxBase::TextObjectHandle **_textObjectHandle;
};

} // end of namespace Grim

#endif
