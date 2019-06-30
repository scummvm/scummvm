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

#ifndef GLK_CONF_H
#define GLK_CONF_H

#include "glk/glk_types.h"
#include "glk/fonts.h"
#include "glk/windows.h"

namespace Glk {

/**
 * Engine configuration
 */
class Conf {
private:
	/**
	 * Get a string
	 */
	void get(const Common::String &key, Common::String &field, const char *defaultVal = nullptr);

	/**
	 * Get a color
	 */
	void get(const Common::String &key, uint &color, const byte *defaultColor);

	/**
	 * Get a font name into a font Id
	 */
	void get(const Common::String &key, FACES &field, FACES defaultFont);

	/**
	 * Get a numeric value
	 */
	void get(const Common::String &key, int &field, int defaultVal = 0);

	/**
	 * Get a numeric value
	 */
	void get(const Common::String &key, bool &field, bool defaultVal = false);

	/**
	 * Get a double
	 */
	void get(const Common::String &key, double &field, double defaultVal = 0.0);

	/**
	 * Parse a color
	 */
	uint parseColor(const Common::String &str);
public:
	MonoFontInfo _monoInfo;
	PropFontInfo _propInfo;
	int _cols, _rows;
	int _lockCols, _lockRows;
	int _wMarginX, _wMarginY;
	int _wMarginSaveX, _wMarginSaveY;
	int _wPaddingX, _wPaddingY;
	int _wBorderX, _wBorderY;
	int _tMarginX, _tMarginY;
	double _gamma;
	uint _borderColor, _borderSave;
	uint _windowColor, _windowSave;
	int _lcd;
	int _scrollWidth;
	uint _scrollBg, _scrollFg;
	bool _graphics;
	bool _sound;
	bool _speak;
	bool _speakInput;
	Common::String _speakLanguage;
	int _styleHint;
	bool _safeClicks;
	WindowStyle _tStyles[style_NUMSTYLES];
	WindowStyle _gStyles[style_NUMSTYLES];
	WindowStyle _tStylesDefault[style_NUMSTYLES];
	WindowStyle _gStylesDefault[style_NUMSTYLES];

	int _imageW, _imageH;
public:
	/**
	 * Constructor
	 */
	Conf(InterpreterType interpType);
};

extern Conf *g_conf;

} // End of namespace Glk

#endif
