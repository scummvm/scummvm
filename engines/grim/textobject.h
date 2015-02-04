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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
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
	void setX(int x) { _x = x; }
	int getX() const { return _x; }

	void setY(int y) { _y = y; }
	int getY() const { return _y; }

	void setFont(const Font *font) { _font = font; }
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

	void setLayer(int layer);
	int getLayer() const { return _layer; }

	void setCoords(int coords) { _coords = coords; }
	int getCoords() const { return _coords; }

protected:
	TextObjectCommon();

	const Font *_font;
	int _x, _y;
	int _width, _height;
	int _justify;
	int _duration;
	int _layer;
	int _coords;
	Color _fgColor;
};

class TextObjectDefaults : public TextObjectCommon {

};

class TextObject : public PoolObject<TextObject>,
                   public TextObjectCommon {
public:
	TextObject();
	~TextObject();

	static int32 getStaticTag() { return MKTAG('T', 'E', 'X', 'T'); }

	void setDefaults(const TextObjectDefaults *defaults);
	void setText(const Common::String &text, bool delaySetup);
	void reset();

	int getBitmapWidth() const;
	int getBitmapHeight() const;
	int getTextCharPosition(int pos);

	int getLineX(int line) const;
	int getLineY(int line) const;

	void setIsSpeech() { _isSpeech = true; }
	void setBlastDraw() { _blastDraw = true; }
	bool isBlastDraw() { return _blastDraw; }

	const void *getUserData() const { return _userData; }
	void setUserData(void *data) { _userData = data; }

	const Common::String *getLines() const { return _lines; }
	int getNumLines() const { return _numberLines; }

	const Common::String &getName() const { return _textID; }
	void draw();
	void update();

	void destroy();

	void saveState(SaveGame *state) const;
	bool restoreState(SaveGame *state);

	int getStackLevel() { return _stackLevel; }
	void incStackLevel() { _stackLevel++; }
	void decStackLevel() { assert(_stackLevel > 0); _stackLevel--; }

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

	int _stackLevel;
};

} // end of namespace Grim

#endif
