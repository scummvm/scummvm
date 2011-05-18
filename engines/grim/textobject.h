/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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
 */

#ifndef GRIM_TEXTOBJECT_H
#define GRIM_TEXTOBJECT_H

#include "engines/grim/gfx_base.h"

namespace Grim {

class SaveGame;
class Font;

class TextObjectCommon {
public:
	void setX(int x) { _x = x; }
	int getX() { return _x; }

	void setY(int y) { _y = y; }
	int getY() { return _y; }

	void setFont(Font *font) { _font = font; }
	Font *getFont() { return _font; }

	void setFGColor(Color *fgColor) { _fgColor = fgColor; }
	Color *getFGColor() { return _fgColor; }

	void setJustify(int justify) { _justify = justify; }
	int getJustify() { return _justify; }

	void setDisabled(bool disabled) { _disabled = disabled; }
	bool getDisabled() { return _disabled; }

	void setWidth(int width) { _width = width; }
	int getWidth() { return _width; }

	void setHeight(int height) { _height = height; }
	int getHeight() { return _height; }

protected:
	TextObjectCommon();

	Color *_fgColor;
	int _x, _y;
	int _width, _height;
	int _justify;
	bool _disabled;
	Font *_font;
};

class TextObjectDefaults : public TextObjectCommon {

};

#define TEXT_NULL   ' '

class TextObject : public Object, public TextObjectCommon {
public:
	TextObject(bool blastDraw, bool isSpeech = false);
	TextObject();
	~TextObject();
	void createBitmap();
	void destroyBitmap();
	void setDefaults(TextObjectDefaults *defaults);
	void setText(const char *text);

	void subBaseOffsetY();
	int getBaseOffsetY();

	int getBitmapWidth();
	int getBitmapHeight();
	int getTextCharPosition(int pos);

	const char *getName() const { return _textID; }
	void draw();

	void saveState(SaveGame *state) const;
	bool restoreState(SaveGame *state);

	enum Justify {
		NONE,
		CENTER,
		LJUSTIFY,
		RJUSTIFY
	};

protected:
	bool _created;
	int _numberLines;
	bool _blastDraw;
	bool _isSpeech;
	char _textID[256];
	uint8 *_textBitmap;
	int *_bitmapWidthPtr;
	GfxBase::TextObjectHandle **_textObjectHandle;

	friend class GrimEngine;
};

} // end of namespace Grim

#endif
