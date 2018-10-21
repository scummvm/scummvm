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

#ifndef GARGOYLE_CONF_H
#define GARGOYLE_CONF_H

#include "gargoyle/glk_types.h"
#include "gargoyle/fonts.h"
#include "gargoyle/windows.h"

namespace Gargoyle {

class Conf {
private:
	/**
	 * Get a string
	 */
	void get(const Common::String &key, Common::String &field, const char *defaultVal = nullptr);

	/**
	 * Get a color
	 */
	void get(const Common::String &key, byte *color, const byte *defaultColor = nullptr);

	/**
	 * Get a font name into a font Id
	 */
	void get(const Common::String &key, FACES &field, FACES defaultFont);

	/**
	 * Get a numeric value
	 */
	void get(const Common::String &key, int &field, int defaultVal = 0);

	/**
	 * Get a double
	 */
	void get(const Common::String &key, double &field, double defaultVal = 0.0);

	/**
	 * Parse a color
	 */
	void parseColor(const Common::String &str, byte *color);
public:
	Common::String _morePrompt;
	byte _moreColor[3], _moreSave[3];
	FACES _moreFont;
	int _moreAlign;
	double _monoAspect;
	double _propAspect;
	double _monoSize;
	Common::String _monoR;
	Common::String _monoB;
	Common::String _monoI;
	Common::String _monoZ;
	Common::String _monoFont;
	double _propSize;
	Common::String _propR;
	Common::String _propB;
	Common::String _propI;
	Common::String _propZ;
	Common::String _propFont;
	int _leading;
	int _baseLine;
	int _cols, _rows;
	int _lockCols, _lockRows;
	int _wMarginX, _wMarginY;
	int _wMarginSaveX, _wMarginSaveY;
	int _wPaddingX, _wPaddingY;
	int _wBorderX, _wBorderY;
	int _tMarginX, _tMarginY;
	double _gamma;
	byte _caretColor[3], _caretSave[3];
	byte _linkColor[3], _linkSave[3];
	byte _borderColor[3], _borderSave[3];
	byte _windowColor[3], _windowSave[3];
	int _lcd;
	int _caretShape;
	int _linkStyle;
	int _scrollWidth;
	byte _scrollBg[3], _scrollFg[3];
	int _justify;
	int _quotes;
	int _dashes;
	int _spaces;
	int _caps;
	int _graphics;
	int _sound;
	int _speak;
	int _speakInput;
	Common::String _speakLanguage;
	int _styleHint;
	WindowStyle _tStyles[style_NUMSTYLES];
	WindowStyle _gStyles[style_NUMSTYLES];
	WindowStyle _tStylesDefault[style_NUMSTYLES];
	WindowStyle _gStylesDefault[style_NUMSTYLES];
public:
	/**
	 * Constructor
	 */
	Conf();
};

extern Conf *g_conf;

} // End of namespace Gargoyle

#endif
