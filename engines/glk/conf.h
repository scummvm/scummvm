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
#include "graphics/pixelformat.h"
#include "common/config-manager.h"

namespace Glk {

/**
 * Engine configuration
 */
class Conf {
	typedef uint Color;
private:
	InterpreterType _interpType;
	bool _isLoading;

	bool exists(const Common::String &key) const {
		return ConfMan.hasKey(key);
	}

	void syncAsString(const Common::String &name, Common::String &val);
	void syncAsInt(const Common::String &name, int &val);
	void syncAsInt(const Common::String &name, uint &val);
	void syncAsDouble(const Common::String &name, double &val);
	void syncAsBool(const Common::String &name, bool &val);
	void syncAsColor(const Common::String &name, uint &val);
	void syncAsFont(const Common::String &name, FACES &val);

	/**
	 * Loads or saves the settings
	 */
	void synchronize();
public:
	/**
	 * Parse a color
	 */
	uint parseColor(const Common::String &str);

	/**
	 * Convert an RGB tuplet to a color
	 */
	uint parseColor(const byte *rgb);

	/**
	 * Convert an RGB uint32 to a color
	 */
	uint parseColor(const uint32 rgb);

	/**
	 * Encode a color to an 6-character RGB hex string
	 */
	Common::String encodeColor(uint color);
public:
	uint _width, _height;
	Graphics::PixelFormat _screenFormat;
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

	/**
	 * Loads the configuration from the ScummVM configuration
	 */
	void load();

	/**
	 * The first time a game is played, flushes all the settings to game's
	 * entry in scummvm.ini. This will make it easier for users to manually
	 * modify scummvm.ini later on to see what options are available
	 */
	void flush();
};

extern Conf *g_conf;

} // End of namespace Glk

#endif
