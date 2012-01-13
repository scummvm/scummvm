/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#include "engines/grim/pool.h"

namespace Grim {

class SaveGame;
class Font;
class PoolColor;

class TextObjectCommon {
public:
	void setX(int x) { _x = x; }
	int getX() { return _x; }

	void setY(int y) { _y = y; }
	int getY() { return _y; }

	void setFont(Font *font) { _font = font; }
	Font *getFont() { return _font; }

	void setFGColor(PoolColor *fgColor) { _fgColor = fgColor; }
	PoolColor *getFGColor() { return _fgColor; }

	void setJustify(int justify) { _justify = justify; }
	int getJustify() { return _justify; }

	void setWidth(int width) { _width = width; }
	int getWidth() { return _width; }

	void setHeight(int height) { _height = height; }
	int getHeight() { return _height; }

	void setDuration(int duration) { _duration = duration; }
	int getDuration() const { return _duration; }

protected:
	TextObjectCommon();

	PoolColor *_fgColor;
	int _x, _y;
	int _width, _height;
	int _justify;
	Font *_font;
	int _duration;
};

class TextObjectDefaults : public TextObjectCommon {

};

class TextObject : public PoolObject<TextObject, MKTAG('T', 'E', 'X', 'T')>,
                   public TextObjectCommon {
public:
	TextObject(bool blastDraw, bool isSpeech = false);
	TextObject();
	~TextObject();

	void setDefaults(TextObjectDefaults *defaults);
	void setText(const Common::String &text);
	void reset();

	int getBitmapWidth();
	int getBitmapHeight();
	int getTextCharPosition(int pos);

	int getLineX(int line);
	int getLineY(int line);

	void *getUserData() { return _userData; }
	void setUserData(void *data) { _userData = data; }

	const Common::String *getLines() { return _lines; }
	int getNumLines() { return _numberLines; }

	const Common::String &getName() const { return _textID; }
	void draw();
	void update();

	void destroy();

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
	void setupText();
	int _numberLines;
	bool _blastDraw;
	bool _isSpeech;
	Common::String _textID;
	int _elapsedTime;
	int _maxLineWidth;
	Common::String *_lines;
	void *_userData;
};

} // end of namespace Grim

#endif
