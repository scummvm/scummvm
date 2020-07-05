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

WindowStyleStatic T_STYLES[style_NUMSTYLES] = {
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
	{ MONOR, { 0xff, 0xff, 0xff }, { 0x00, 0x00, 0x00 }, 0 }  ///< User2
};

WindowStyleStatic G_STYLES[style_NUMSTYLES] = {
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
	{ MONOR, { 0xff, 0xff, 0xff }, { 0x60, 0x60, 0x60 }, 0 }  ///< User2
};

Conf *g_conf;

Conf::Conf(InterpreterType interpType) : _interpType(interpType), _graphics(true),
		_rows(25), _cols(60), _lockRows(0), _lockCols(0), _wPaddingX(0),
		_wPaddingY(0), _wBorderX(0), _wBorderY(0), _tMarginX(7), _tMarginY(7),
		_gamma(1.0), _borderColor(0), _borderSave(0),
		_windowColor(parseColor(WHITE)), _windowSave(parseColor(WHITE)),
		_sound(true), _speak(false), _speakInput(false), _styleHint(1),
		_scrollBg(parseColor(SCROLL_BG)), _scrollFg(parseColor(SCROLL_FG)),
		_lcd(1), _scrollWidth(0), _safeClicks(false)
{
	g_conf = this;
	_imageW = g_system->getWidth();
	_imageH = g_system->getHeight();

	_propInfo._morePrompt = "\207 more \207";
	_propInfo._moreColor = 0;
	_propInfo._moreSave = 0;
	_propInfo._moreFont = PROPB;
	_propInfo._moreAlign = 0;
	_monoInfo._aspect = 1.0;
	_propInfo._aspect = 1.0;
	_monoInfo._size = 11;
	_propInfo._size = 12;
	_propInfo._linkColor = parseColor(BLUE);
	_monoInfo._linkColor = _propInfo._linkColor;
	_propInfo._linkSave = _propInfo._linkColor;
	_propInfo._caretColor = 0;
	_propInfo._caretSave = 0;
	_propInfo._caretShape = 2;
	_propInfo._linkStyle = 1;
	_monoInfo._linkStyle = 1;
	_propInfo._justify = 0;
	_propInfo._quotes = 1;
	_propInfo._dashes = 1;
	_propInfo._spaces = 0;
	_propInfo._caps = 0;

	const int DEFAULT_MARGIN_X = (_interpType == INTERPRETER_ZCODE) ? 0 : 15;
	const int DEFAULT_MARGIN_Y = (_interpType == INTERPRETER_ZCODE) ? 0 : 15;
	_wMarginX = _wMarginSaveX = DEFAULT_MARGIN_X;
	_wMarginY = _wMarginSaveY = DEFAULT_MARGIN_Y;

	// For simplicity's sake, only allow graphics when in non-paletted graphics modes
	if (g_system->getScreenFormat().bytesPerPixel == 1)
		_graphics = false;

	Common::copy(T_STYLES, T_STYLES + style_NUMSTYLES, _tStyles);
	Common::copy(G_STYLES, G_STYLES + style_NUMSTYLES, _gStyles);

	Common::copy(_tStyles, _tStyles + style_NUMSTYLES, _tStylesDefault);
	Common::copy(_gStyles, _gStyles + style_NUMSTYLES, _gStylesDefault);
}

void Conf::load() {
	get("moreprompt", _propInfo._morePrompt);
	get("morecolor", _propInfo._moreColor);
	get("morecolor", _propInfo._moreSave);
	get("morefont", _propInfo._moreFont);
	get("morealign", _propInfo._moreAlign);
	get("monoaspect", _monoInfo._aspect);
	get("propaspect", _propInfo._aspect);
	get("monosize", _monoInfo._size);
	get("propsize", _propInfo._size);
	get("rows", _rows);
	get("cols", _cols);

	if (ConfMan.hasKey("leading"))
		_monoInfo._leading = _propInfo._leading = static_cast<int>(atof(ConfMan.get("leading").c_str()) + 0.5);
	if (ConfMan.hasKey("baseline"))
		_propInfo._baseLine = static_cast<int>(atof(ConfMan.get("baseline").c_str()) + 0.5);

	if (ConfMan.hasKey("minrows"))
		_rows = MAX(_rows, strToInt(ConfMan.get("minrows").c_str()));
	if (ConfMan.hasKey("maxrows"))
		_rows = MIN(_rows, strToInt(ConfMan.get("maxrows").c_str()));
	if (ConfMan.hasKey("mincols"))
		_cols = MAX(_cols, strToInt(ConfMan.get("mincols").c_str()));
	if (ConfMan.hasKey("maxcols"))
		_cols = MIN(_cols, strToInt(ConfMan.get("maxcols").c_str()));

	get("lockrows", _lockRows);
	get("lockcols", _lockCols);
	get("wmarginx", _wMarginX);
	get("wmarginy", _wMarginY);
	_wMarginSaveX = _wMarginX;
	_wMarginSaveY = _wMarginY;

	get("wpaddingx", _wPaddingX);
	get("wpaddingy", _wPaddingY);
	get("wborderx", _wBorderX);
	get("wbordery", _wBorderY);
	get("tmarginx", _tMarginX);
	get("tmarginy", _tMarginY);
	get("gamma", _gamma);

	get("linkcolor", _propInfo._linkColor);
	_monoInfo._linkColor = _propInfo._linkColor;
	_propInfo._linkSave = _propInfo._linkColor;

	get("bordercolor", _borderColor);
	get("bordercolor", _borderSave);
	get("windowcolor", _windowColor);
	get("windowcolor", _windowSave);
	get("lcd", _lcd);
	get("caretcolor", _propInfo._caretColor);
	get("caretcolor", _propInfo._caretSave);
	get("caretshape", _propInfo._caretShape);

	if (ConfMan.hasKey("linkstyle"))
		_propInfo._linkStyle = _monoInfo._linkStyle = 
			!strToInt(ConfMan.get("linkstyle").c_str()) ? 0 : 1;

	get("scrollwidth", _scrollWidth);
	get("scrollbg", _scrollBg);
	get("scrollfg", _scrollFg);
	get("justify", _propInfo._justify);
	get("quotes", _propInfo._quotes);
	get("dashes", _propInfo._dashes);
	get("spaces", _propInfo._spaces);
	get("caps", _propInfo._caps);
	get("graphics", _graphics);
	get("sound", _sound);
	get("speak", _speak);
	get("speak_input", _speakInput);
	get("speak_language", _speakLanguage);
	get("stylehint", _styleHint);
	get("safeclicks", _safeClicks);

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
				_tStyles[style].fg = parseColor(fg);
				_tStyles[style].bg = parseColor(bg);
			} else {
				_gStyles[style].fg = parseColor(fg);
				_gStyles[style].bg = parseColor(bg);
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

void Conf::get(const Common::String &key, Common::String &field) {
	if (ConfMan.hasKey(key)) {
		field = ConfMan.get(key);
		field.trim();
	}
}

void Conf::get(const Common::String &key, uint &color) {
	if (ConfMan.hasKey(key))
		color = parseColor(ConfMan.get(key));
}

void Conf::get(const Common::String &key, int &field) {
	if (ConfMan.hasKey(key))
		field = strToInt(ConfMan.get(key).c_str());
}

void Conf::get(const Common::String &key, bool &field) {
	if (ConfMan.hasKey(key))
		Common::parseBool(ConfMan.get(key), field);
}

void Conf::get(const Common::String &key, FACES &field) {
	if (ConfMan.hasKey(key))
		field = Screen::getFontId(ConfMan.get(key));
}

void Conf::get(const Common::String &key, double &field) {
	if (ConfMan.hasKey(key))
		field = atof(ConfMan.get(key).c_str());
}

uint Conf::parseColor(const Common::String &str) {
	char r[3], g[3], b[3];
	uint rv, gv, bv;

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

		rv = strtol(r, nullptr, 16);
		gv = strtol(g, nullptr, 16);
		bv = strtol(b, nullptr, 16);
		return g_system->getScreenFormat().RGBToColor(rv, gv, bv);
	}

	return 0;
}

uint Conf::parseColor(const byte *rgb) {
	return g_system->getScreenFormat().RGBToColor(rgb[0], rgb[1], rgb[2]);
}

} // End of namespace Glk
