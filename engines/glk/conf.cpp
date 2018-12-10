/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software{} you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation{} either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY{} without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program{} if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/conf.h"
#include "glk/utils.h"
#include "glk/windows.h"
#include "common/config-manager.h"
#include "common/system.h"

namespace Glk {

const byte WHITE[3] = { 0xff, 0xff, 0xff };
const byte BLUE[3] = { 0x00, 0x00, 0x60 };
const byte SCROLL_BG[3] = { 0xb0, 0xb0, 0xb0 };
const byte SCROLL_FG[3] = { 0x80, 0x80, 0x80 };

WindowStyle T_STYLES[style_NUMSTYLES] = {
	{ PROPR, { 0xff, 0xff, 0xff }, { 0x00, 0x00, 0x00 }, 0 }, ///< Normal
	{ PROPI, { 0xff, 0xff, 0xff }, { 0x00, 0x00, 0x00 }, 0 }, ///< Emphasized
	{ MONOR, { 0xff, 0xff, 0xff }, { 0x00, 0x00, 0x00 }, 0 }, ///< Preformatted
	{ PROPB, { 0xff, 0xff, 0xff }, { 0x00, 0x00, 0x00 }, 0 }, ///< Header
	{ PROPB, { 0xff, 0xff, 0xff }, { 0x00, 0x00, 0x00 }, 0 }, ///< Subheader
	{ PROPZ, { 0xff, 0xff, 0xff }, { 0x00, 0x00, 0x00 }, 0 }, ///< Alert
	{ PROPR, { 0xff, 0xff, 0xff }, { 0x00, 0x00, 0x00 }, 0 }, ///< Note
	{ PROPR, { 0xff, 0xff, 0xff }, { 0x00, 0x00, 0x00 }, 0 }, ///< BlockQuote
	{ PROPB, { 0xff, 0xff, 0xff }, { 0x00, 0x60, 0x00 }, 0 }, ///< Input
	{ MONOR, { 0xff, 0xff, 0xff }, { 0x00, 0x00, 0x00 }, 0 }, ///< User1
	{ MONOR, { 0xff, 0xff, 0xff }, { 0x00, 0x00, 0x00 }, 0 }, ///< User2
};

WindowStyle G_STYLES[style_NUMSTYLES] = {
	{ MONOR, { 0xff, 0xff, 0xff }, { 0x60, 0x60, 0x60 }, 0 }, ///< Normal
	{ MONOI, { 0xff, 0xff, 0xff }, { 0x60, 0x60, 0x60 }, 0 }, ///< Emphasized
	{ MONOR, { 0xff, 0xff, 0xff }, { 0x60, 0x60, 0x60 }, 0 }, ///< Preformatted
	{ MONOB, { 0xff, 0xff, 0xff }, { 0x60, 0x60, 0x60 }, 0 }, ///< Header
	{ MONOB, { 0xff, 0xff, 0xff }, { 0x60, 0x60, 0x60 }, 0 }, ///< Subheader
	{ MONOR, { 0xff, 0xff, 0xff }, { 0x60, 0x60, 0x60 }, 0 }, ///< Alert
	{ MONOR, { 0xff, 0xff, 0xff }, { 0x60, 0x60, 0x60 }, 0 }, ///< Note
	{ MONOR, { 0xff, 0xff, 0xff }, { 0x60, 0x60, 0x60 }, 0 }, ///< BlockQuote
	{ MONOR, { 0xff, 0xff, 0xff }, { 0x60, 0x60, 0x60 }, 0 }, ///< Input
	{ MONOR, { 0xff, 0xff, 0xff }, { 0x60, 0x60, 0x60 }, 0 }, ///< User1
	{ MONOR, { 0xff, 0xff, 0xff }, { 0x60, 0x60, 0x60 }, 0 }, ///< User2
};

Conf *g_conf;

Conf::Conf(InterpreterType interpType) {
	g_conf = this;
	_imageW = g_system->getWidth();
	_imageH = g_system->getHeight();
	_cellW = _cellH = 8;
	_leading = 0;
	_baseLine = 0;

	get("moreprompt", _morePrompt, "\207 more \207");
	get("morecolor", _moreColor);
	get("morecolor", _moreSave);
	get("morefont", _moreFont, PROPB);
	get("morealign", _moreAlign);
	get("monoaspect", _monoAspect, 1.0);
	get("propaspect", _propAspect, 1.0);
	get("monosize", _monoSize, 11);
	get("monor", _monoR);
	get("monob", _monoR);
	get("monoi", _monoI);
	get("monoz", _monoZ);
	get("monofont", _monoFont, "Liberation Mono");
	get("propsize", _propSize, 12);
	get("propr", _propR);
	get("propb", _propR);
	get("propi", _propI);
	get("propz", _propZ);
	get("propfont", _propFont, "Linux Libertine O");
	get("rows", _rows, 25);
	get("cols", _cols, 60);

	if (ConfMan.hasKey("leading"))
		_leading = static_cast<int>(atof(ConfMan.get("leading").c_str()) + 0.5);
	if (ConfMan.hasKey("baseline"))
		_baseLine = static_cast<int>(atof(ConfMan.get("baseline").c_str()) + 0.5);

	if (ConfMan.hasKey("minrows"))
		_rows = MAX(_rows, strToInt(ConfMan.get("minrows").c_str()));
	if (ConfMan.hasKey("maxrows"))
		_rows = MIN(_rows, strToInt(ConfMan.get("maxrows").c_str()));
	if (ConfMan.hasKey("mincols"))
		_cols = MAX(_cols, strToInt(ConfMan.get("mincols").c_str()));
	if (ConfMan.hasKey("maxcols"))
		_cols = MIN(_cols, strToInt(ConfMan.get("maxcols").c_str()));

	const int DEFAULT_MARGIN_X = (interpType == INTERPRETER_FROTZ) ? 0 : 15;
	const int DEFAULT_MARGIN_Y = (interpType == INTERPRETER_FROTZ) ? 0 : 15;

	get("lockrows", _lockRows);
	get("lockcols", _lockCols);
	get("wmarginx", _wMarginX, DEFAULT_MARGIN_X);
	get("wmarginy", _wMarginY, DEFAULT_MARGIN_Y);
	_wMarginSaveX = _wMarginX;
	_wMarginSaveY = _wMarginY;

	get("wpaddingx", _wPaddingX);
	get("wpaddingy", _wPaddingY);
	get("wborderx", _wBorderX);
	get("wbordery", _wBorderY);
	get("tmarginx", _tMarginX, 7);
	get("tmarginy", _tMarginY, 7);
	get("gamma", _gamma, 1.0);

	get("caretcolor", _caretColor);
	get("caretcolor", _caretSave);
	get("linkcolor", _linkColor, BLUE);
	get("linkcolor", _linkSave, BLUE);
	get("bordercolor", _borderColor);
	get("bordercolor", _borderSave);
	get("windowcolor", _windowColor, WHITE);
	get("windowcolor", _windowSave, WHITE);
	get("lcd", _lcd, 1);
	get("caretshape", _caretShape, 2);

	_linkStyle = ConfMan.hasKey("linkstyle") && !strToInt(ConfMan.get("linkstyle").c_str()) ? 0 : 1;

	get("scrollwidth", _scrollWidth);
	get("scrollbg", _scrollBg, SCROLL_BG);
	get("scrollfg", _scrollFg, SCROLL_FG);
	get("justify", _justify);
	get("quotes", _quotes, 1);
	get("dashes", _dashes, 1);
	get("spaces", _spaces);
	get("caps", _caps);
	get("graphics", _graphics, true);
	get("sound", _sound, true);
	get("speak", _speak);
	get("speak_input", _speakInput);
	get("speak_language", _speakLanguage);
	get("stylehint", _styleHint, 1);
	get("safeclicks", _safeClicks);

	// For simplicity's sake, only allow graphics when in non-paletted graphics modes
	if (g_system->getScreenFormat().bytesPerPixel == 1)
		_graphics = false;

	Common::copy(T_STYLES, T_STYLES + style_NUMSTYLES, _tStyles);
	Common::copy(G_STYLES, G_STYLES + style_NUMSTYLES, _gStyles);

	char buffer[256];
	const char *const TG_COLOR[2] = { "tcolor_%d", "gcolor_%d" };
	for (int tg = 0; tg < 2; ++tg) {
		for (int style = 0; style <= 10; ++style) {
			Common::String key = Common::String::format(TG_COLOR[tg], style);
			if (!ConfMan.hasKey(key))
				continue;

			strncpy(buffer, ConfMan.get(key).c_str(), 254);
			buffer[255] = '\0';
			char *fg = strtok(buffer, "\r\n\t ");
			char *bg = strtok(nullptr, "\r\n\t ");

			if (tg == 0) {
				parseColor(fg, _tStyles[style].fg);
				parseColor(bg, _tStyles[style].bg);
			} else {
				parseColor(fg, _gStyles[style].fg);
				parseColor(bg, _gStyles[style].bg);
			}
		}
	}

	const char *const TG_FONT[2] = { "tfont_%d", "gfont_%d" };
	for (int tg = 0; tg < 2; ++tg) {
		for (int style = 0; style <= 10; ++style) {
			Common::String key = Common::String::format(TG_FONT[tg], style);
			if (!ConfMan.hasKey(key))
				continue;

			strncpy(buffer, ConfMan.get(key).c_str(), 254);
			buffer[255] = '\0';
			char *font = strtok(buffer, "\r\n\t ");

			if (tg == 0)
				_tStyles[style].font = Screen::getFontId(font);
			else
				_gStyles[style].font = Screen::getFontId(font);
		}
	}

	Common::copy(_tStyles, _tStyles + style_NUMSTYLES, _tStylesDefault);
	Common::copy(_gStyles, _gStyles + style_NUMSTYLES, _gStylesDefault);
}

void Conf::get(const Common::String &key, Common::String &field, const char *defaultVal) {
	field = ConfMan.hasKey(key) ? ConfMan.get(key) : defaultVal;
	field.trim();
}

void Conf::get(const Common::String &key, byte *color, const byte *defaultColor) {
	if (ConfMan.hasKey(key)) {
		parseColor(ConfMan.get(key), color);
	} else if (defaultColor) {
		Common::copy(defaultColor, defaultColor + 3, color);
	} else {
		Common::fill(color, color + 3, 0);
	}
}

void Conf::get(const Common::String &key, int &field, int defaultVal) {
	field = ConfMan.hasKey(key) ? strToInt(ConfMan.get(key).c_str()) : defaultVal;
}

void Conf::get(const Common::String &key, bool &field, bool defaultVal) {
	field = ConfMan.hasKey(key) ? strToInt(ConfMan.get(key).c_str()) != 0 : defaultVal;
}

void Conf::get(const Common::String &key, FACES &field, FACES defaultFont) {
	field = ConfMan.hasKey(key) ? Screen::getFontId(ConfMan.get(key)) : defaultFont;
}

void Conf::get(const Common::String &key, double &field, double defaultVal) {
	field = ConfMan.hasKey(key) ?  atof(ConfMan.get(key).c_str()) : defaultVal;
}

void Conf::parseColor(const Common::String &str, byte *color) {
	char r[3], g[3], b[3];

	if (str.size() == 6) {
		r[0] = str[0];
		r[1] = str[1];
		r[2] = 0;
		g[0] = str[2];
		g[1] = str[3];
		g[2] = 0;
		b[0] = str[4];
		b[1] = str[5];
		b[2] = 0;

		color[0] = strtol(r, nullptr, 16);
		color[1] = strtol(g, nullptr, 16);
		color[2] = strtol(b, nullptr, 16);
	}
}

} // End of namespace Glk
