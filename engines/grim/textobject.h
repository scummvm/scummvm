/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#ifndef GRIM_TEXTOBJECT_H
#define GRIM_TEXTOBJECT_H

#include "engines/grim/pool.h"
#include "engines/grim/color.h"

#include "common/endian.h"

namespace Grim {

class SaveGame;
class Font;

class TextObjectCommon {
public:
	void setX(int x) { _x = x; _positioned = false; }
	int getX() const { return _x; }

	void setY(int y) { _y = y; _positioned = false; }
	int getY() const { return _y; }

	void setFont(Font *font) { _font = font; }
	const Font *getFont() const { return _font; }

	void setFGColor(const Color &fgColor) { _fgColor = fgColor; }
	Color getFGColor() const { return _fgColor; }

	void setJustify(int justify) { _justify = justify; }
	int getJustify() const { return _justify; }

	void setWidth(int width) { _width = width; }
	int getWidth() const { return _width; }

	void setHeight(int height) { _height = height; }
	int getHeight() const { return _height; }

	void setDuration(int duration) { _duration = duration; }
	int getDuration() const { return _duration; }

protected:
	TextObjectCommon();

	const Font *_font;
	int _x, _y;
	int _posX, _posY;
	int _width, _height;
	int _justify;
	int _duration;
	Color _fgColor;
	bool _positioned;
};

class TextObjectDefaults : public TextObjectCommon {

};

class TextObject : public PoolObject<TextObject>,
                   public TextObjectCommon {
public:
	TextObject();
	~TextObject();

	static int32 getStaticTag() { return MKTAG('T', 'E', 'X', 'T'); }

	void setDefaults(TextObjectDefaults *defaults);
	void setText(const Common::String &text);
	void reset();

	int getBitmapWidth() const;
	int getBitmapHeight() const;
	int getTextCharPosition(int pos);

	int getLineX(int line) const;
	int getLineY(int line) const;

	void setIsSpeach() { _isSpeech = true; }
	void setBlastDraw() { _blastDraw = true; }

	const void *getUserData() const { return _userData; }
	void setUserData(void *data) { _userData = data; }

	const Common::String *getLines() const { return _lines; }
	int getNumLines() const { return _numberLines; }

	const Common::String &getName() const { return _textID; }
	void draw();
	void update();

	void destroy();
	void reposition();

	void saveState(SaveGame *state) const;
	bool restoreState(SaveGame *state);

	enum Justify {
		NONE,
		CENTER,
		LJUSTIFY,
		RJUSTIFY
	};

protected:
	void setupText();

	Common::String _textID;

	Common::String *_lines;

	void *_userData;

	int _numberLines;
	int _elapsedTime;
	int _maxLineWidth;

	bool _blastDraw;
	bool _isSpeech;
	bool _created;
};

} // end of namespace Grim

#endif
