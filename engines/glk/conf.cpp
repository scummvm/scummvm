/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "glk/conf.h"
#include "glk/utils.h"
#include "glk/windows.h"
#include "common/config-manager.h"

namespace Glk {

const byte WHITE[3] = { 0xff, 0xff, 0xff };
const byte BLUE[3] = { 0x00, 0x00, 0x60 };
const byte SCROLL_BG[3] = { 0xb0, 0xb0, 0xb0 };
const byte SCROLL_FG[3] = { 0x80, 0x80, 0x80 };

WindowStyleStatic T_STYLES[style_NUMSTYLES] = {
	{ PROPR, { 0xff, 0xff, 0xff }, { 0x00, 0x00, 0x00 }, false }, ///< Normal
	{ PROPI, { 0xff, 0xff, 0xff }, { 0x00, 0x00, 0x00 }, false }, ///< Emphasized
	{ MONOR, { 0xff, 0xff, 0xff }, { 0x00, 0x00, 0x00 }, false }, ///< Preformatted
	{ PROPB, { 0xff, 0xff, 0xff }, { 0x00, 0x00, 0x00 }, false }, ///< Header
	{ PROPB, { 0xff, 0xff, 0xff }, { 0x00, 0x00, 0x00 }, false }, ///< Subheader
	{ PROPZ, { 0xff, 0xff, 0xff }, { 0x00, 0x00, 0x00 }, false }, ///< Alert
	{ PROPR, { 0xff, 0xff, 0xff }, { 0x00, 0x00, 0x00 }, false }, ///< Note
	{ PROPR, { 0xff, 0xff, 0xff }, { 0x00, 0x00, 0x00 }, false }, ///< BlockQuote
	{ PROPB, { 0xff, 0xff, 0xff }, { 0x00, 0x60, 0x00 }, false }, ///< Input
	{ MONOR, { 0xff, 0xff, 0xff }, { 0x00, 0x00, 0x00 }, false }, ///< User1
	{ MONOR, { 0xff, 0xff, 0xff }, { 0x00, 0x00, 0x00 }, false }  ///< User2
};

WindowStyleStatic G_STYLES[style_NUMSTYLES] = {
	{ MONOR, { 0xff, 0xff, 0xff }, { 0x60, 0x60, 0x60 }, false }, ///< Normal
	{ MONOI, { 0xff, 0xff, 0xff }, { 0x60, 0x60, 0x60 }, false }, ///< Emphasized
	{ MONOR, { 0xff, 0xff, 0xff }, { 0x60, 0x60, 0x60 }, false }, ///< Preformatted
	{ MONOB, { 0xff, 0xff, 0xff }, { 0x60, 0x60, 0x60 }, false }, ///< Header
	{ MONOB, { 0xff, 0xff, 0xff }, { 0x60, 0x60, 0x60 }, false }, ///< Subheader
	{ MONOR, { 0xff, 0xff, 0xff }, { 0x60, 0x60, 0x60 }, false }, ///< Alert
	{ MONOR, { 0xff, 0xff, 0xff }, { 0x60, 0x60, 0x60 }, false }, ///< Note
	{ MONOR, { 0xff, 0xff, 0xff }, { 0x60, 0x60, 0x60 }, false }, ///< BlockQuote
	{ MONOR, { 0xff, 0xff, 0xff }, { 0x60, 0x60, 0x60 }, false }, ///< Input
	{ MONOR, { 0xff, 0xff, 0xff }, { 0x60, 0x60, 0x60 }, false }, ///< User1
	{ MONOR, { 0xff, 0xff, 0xff }, { 0x60, 0x60, 0x60 }, false }  ///< User2
};

Conf *g_conf;

Conf::Conf(InterpreterType interpType, bool installGlobal,
		const Graphics::PixelFormat &format) : _interpType(interpType),
		_installedGlobal(installGlobal), _preferences(nullptr), _graphics(true),
#ifndef USE_HIGHRES
		_width(320), _height(200),
#else
		_width(640), _height(400),
#endif
		_screenFormat(format),
#ifndef USE_HIGHRES
		_rows(12), _cols(30),
#else
		_rows(25), _cols(60),
#endif
		_lockRows(0), _lockCols(0), _wPaddingX(0), _wPaddingY(0),
		_wBorderX(0), _wBorderY(0), _tMarginX(7), _tMarginY(7), _gamma(1.0),
		_borderColor(0), _borderSave(0),
		_windowColor(parseColor(WHITE)), _windowSave(parseColor(WHITE)),
		_windowColorOverride(false), _borderColorOverride(false),
		_sound(true), _speak(false), _speakInput(false), _styleHint(1),
		_scrollBg(parseColor(SCROLL_BG)), _scrollFg(parseColor(SCROLL_FG)),
		_scrollWidth(0), _safeClicks(false) {
	if (_installedGlobal)
		g_conf = this;
	_imageW = _width;
	_imageH = _height;

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
	if (_screenFormat.bytesPerPixel == 1)
		_graphics = false;

	for (int i = 0; i < style_NUMSTYLES; ++i) {
		_tStyles[i].fg = parseColor(T_STYLES[i].fg);
		_tStyles[i].bg = parseColor(T_STYLES[i].bg);
		_tStyles[i].font = T_STYLES[i].font;
		_tStyles[i].reverse = T_STYLES[i].reverse;

		_gStyles[i].fg = parseColor(G_STYLES[i].fg);
		_gStyles[i].bg = parseColor(G_STYLES[i].bg);
		_gStyles[i].font = G_STYLES[i].font;
		_gStyles[i].reverse = G_STYLES[i].reverse;
	}

	Common::copy(_tStyles, _tStyles + style_NUMSTYLES, _tStylesDefault);
	Common::copy(_gStyles, _gStyles + style_NUMSTYLES, _gStylesDefault);
}

void Conf::synchronize() {
	syncAsInt("width", _width);
	syncAsInt("height", _height);
	syncAsString("moreprompt", _propInfo._morePrompt);
	syncAsColor("morecolor", _propInfo._moreColor);
	syncAsColor("morecolor", _propInfo._moreSave);
	syncAsFont("morefont", _propInfo._moreFont);
	syncAsInt("morealign", _propInfo._moreAlign);
	syncAsDouble("monoaspect", _monoInfo._aspect);
	syncAsDouble("propaspect", _propInfo._aspect);
	syncAsDouble("monosize", _monoInfo._size);
	syncAsDouble("propsize", _propInfo._size);
	syncAsInt("rows", _rows);
	syncAsInt("cols", _cols);

	_imageW = _width;
	_imageH = _height;

	syncAsInt("leading", _monoInfo._leading);
	syncAsInt("leading", _propInfo._leading);
	syncAsInt("baseline", _propInfo._baseLine);

	if (exists("minrows"))
		_rows = MAX(_rows, getInt("minrows"));
	if (exists("maxrows"))
		_rows = MIN(_rows, getInt("maxrows"));
	if (exists("mincols"))
		_cols = MAX(_cols, getInt("mincols"));
	if (exists("maxcols"))
		_cols = MIN(_cols, getInt("maxcols"));

	syncAsInt("lockrows", _lockRows);
	syncAsInt("lockcols", _lockCols);
	syncAsInt("wmarginx", _wMarginX);
	syncAsInt("wmarginy", _wMarginY);

	_wMarginSaveX = _wMarginX;
	_wMarginSaveY = _wMarginY;

	syncAsInt("wpaddingx", _wPaddingX);
	syncAsInt("wpaddingy", _wPaddingY);
	syncAsInt("wborderx", _wBorderX);
	syncAsInt("wbordery", _wBorderY);
	syncAsInt("tmarginx", _tMarginX);
	syncAsInt("tmarginy", _tMarginY);
	syncAsDouble("gamma", _gamma);

	syncAsColor("linkcolor", _propInfo._linkColor);
	_monoInfo._linkColor = _propInfo._linkColor;
	_propInfo._linkSave = _propInfo._linkColor;

	syncAsColor("bordercolor", _borderColor);
	syncAsColor("bordercolor", _borderSave);
	syncAsColor("windowcolor", _windowColor);
	syncAsColor("windowcolor", _windowSave);

	syncAsColor("caretcolor", _propInfo._caretColor);
	syncAsInt("caretshape", _propInfo._caretShape);
	syncAsInt("linkstyle", _propInfo._linkStyle);
	{
		_propInfo._caretSave = _propInfo._caretColor;

		_monoInfo._caretColor = _propInfo._caretColor;
		_monoInfo._caretSave = _propInfo._caretSave;
		_monoInfo._caretShape = _propInfo._caretShape;
		_monoInfo._linkStyle = _propInfo._linkStyle;
	}

	syncAsInt("scrollwidth", _scrollWidth);
	syncAsColor("scrollbg", _scrollBg);
	syncAsColor("scrollfg", _scrollFg);
	syncAsInt("justify", _propInfo._justify);
	syncAsInt("quotes", _propInfo._quotes);
	syncAsInt("dashes", _propInfo._dashes);
	syncAsInt("spaces", _propInfo._spaces);
	syncAsInt("caps", _propInfo._caps);

	syncAsBool("graphics", _graphics);
	syncAsBool("sound", _sound);
	syncAsBool("speak", _speak);
	syncAsBool("speak_input", _speakInput);
	syncAsString("speak_language", _speakLanguage);
	syncAsInt("stylehint", _styleHint);
	syncAsBool("safeclicks", _safeClicks);

	for (int grid = 0; grid < 2; ++grid) {
		WindowStyle *styles = grid ? _gStyles : _tStyles;
		for (int style = 0; style < style_NUMSTYLES; ++style) {
			Common::String color;
			if (_preferences->styleColor(grid, style, true, color) != kGlkInterpreterDefault) {
				styles[style].fg = parseColor(color);
			}
			if (_preferences->styleColor(grid, style, false, color) != kGlkInterpreterDefault) {
				styles[style].bg = parseColor(color);
			}
			const Common::String key = Common::String::format("%cfont_%d", grid ? 'g' : 't', style);
			FACES font = styles[style].font;
			if (exists(key) && GlkPreferences::parseFont(getString(key), font)) {
				styles[style].font = font;
			}
		}
	}
}

void Conf::load(const Common::String &domain) {
	GlkPreferences preferences(domain.empty() ? ConfMan.getActiveDomain() : ConfMan.getDomain(domain), domain.empty());
	load(preferences);
}

void Conf::load(const GlkPreferences &preferences) {
	_preferences = &preferences;
	synchronize();
	_preferences = nullptr;
	Common::copy(_tStyles, _tStyles + style_NUMSTYLES, _tStylesDefault);
	Common::copy(_gStyles, _gStyles + style_NUMSTYLES, _gStylesDefault);
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
		return _screenFormat.RGBToColor(rv, gv, bv);
	}

	return 0;
}

Common::String Conf::encodeColor(uint color) const {
	byte r, g, b;
	_screenFormat.colorToRGB(color, r, g, b);
	return Common::String::format("%.2x%.2x%.2x", (int)r, (int)g, (int)b);
}

bool Conf::exists(const Common::String &key) const {
	return _preferences->source(key) != kGlkInterpreterDefault;
}

Common::String Conf::getString(const Common::String &key) const {
	return _preferences->get(key);
}

int Conf::getInt(const Common::String &key) const {
	int value = 0;
	GlkPreferences::parseInteger(getString(key), value);
	return value;
}

uint Conf::parseColor(const byte *rgb) {
	return _screenFormat.RGBToColor(rgb[0], rgb[1], rgb[2]);
}

uint Conf::parseColor(const uint32 rgb) {
	byte r = (rgb >> 16) & 0xff,
		g = (rgb >> 8) & 0xff,
		b = rgb & 0xff;

	return _screenFormat.RGBToColor(r, g, b);
}

void Conf::syncAsString(const Common::String &name, Common::String &val) {
	if (exists(name))
		val = getString(name);
}

void Conf::syncAsInt(const Common::String &name, int &val) {
	if (exists(name)) {
		bool boolean;
		if (GlkPreferences::parseBool(getString(name), boolean))
			val = boolean;
		else
			GlkPreferences::parseInteger(getString(name), val);
	}
}

void Conf::syncAsInt(const Common::String &name, uint &val) {
	int parsed = 0;
	if (exists(name) && GlkPreferences::parseInteger(getString(name), parsed) && parsed >= 0)
		val = parsed;
}

void Conf::syncAsDouble(const Common::String &name, double &val) {
	if (exists(name))
		GlkPreferences::parseFloat(getString(name), 0.0, 32767.0, val);
}

void Conf::syncAsBool(const Common::String &name, bool &val) {
	if (exists(name))
		GlkPreferences::parseBool(getString(name), val);
}

void Conf::syncAsColor(const Common::String &name, uint &val) {
	Common::String normalized;
	if (exists(name) && GlkPreferences::parseColor(getString(name), normalized))
		val = parseColor(normalized);
}

void Conf::syncAsFont(const Common::String &name, FACES &val) {
	if (exists(name))
		GlkPreferences::parseFont(getString(name), val);
}

Conf::~Conf() {
	if (_installedGlobal && g_conf == this)
		g_conf = nullptr;
}

} // End of namespace Glk
