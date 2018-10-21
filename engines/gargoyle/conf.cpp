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

#include "gargoyle/conf.h"
#include "gargoyle/fonts.h"
#include "gargoyle/string.h"
#include "common/config-manager.h"

namespace Gargoyle {

const byte WHITE[3] = { 0xff, 0xff, 0xff };
const byte BLUE[3] = { 0x00, 0x00, 0x60 };
const byte SCROLL_BG[3] = { 0xb0, 0xb0, 0xb0 };
const byte SCROLL_FG[3] = { 0x80, 0x80, 0x80 };

Conf::Conf() {
	g_conf = this;

	get("moreprompt", _morePrompt, "\207 more \207");
	get("morecolor", _moreColor);
	get("morecolor", _moreSave);
	get("morefont", _moreFont, PROPB);
	get("morealign", _moreAlign);
	get("monoaspect", _monoAspect, 1.0);
	get("propaspect", _propAspect, 1.0);
	get("monosize", _monoSize, 12.5);
	get("monor", _monoR);
	get("monob", _monoR);
	get("monoi", _monoI);
	get("monoz", _monoZ);
	get("monofont", _monoFont, "Liberation Mono");
	get("propsize", _propSize, 15.5);
	get("propr", _propR);
	get("propb", _propR);
	get("propi", _propI);
	get("propz", _propZ);
	get("propfont", _propFont, "Linux Libertine O");
	get("leading", _leading);
	get("baseline", _baseLine);
	get("rows", _rows, 25);
	get("cols", _cols, 60);

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
	get("wmarginx", _wMarginX, 15);
	get("wmarginy", _wMarginY, 15);
	_wMarginSaveX = _wMarginX;
	_wMarginSaveY = _wMarginY;

	get("wpaddingx", _wPaddingX);
	get("wpaddingy", _wPaddingY);
	get("wborderx", _wBorderX, 1);
	get("wbordery", _wBorderY, 1);
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
	get("graphics", _graphics, 1);
	get("sound", _sound, 1);
	get("speak", _speak);
	get("speak_input", _speakInput);
	get("speak_language", _speakLanguage);
	get("stylehint", _styleHint, 1);

}

void Conf::get(const Common::String &key, Common::String &field, const char *defaultVal) {
	field = ConfMan.hasKey(key) ? ConfMan.get(key) : defaultVal;
	field.trim();
}

void Conf::get(const Common::String &key, byte *color, const byte *defaultColor) {
	char r[3], g[3], b[3];
	Common::String str;

	if (ConfMan.hasKey(key) && (str = ConfMan.get(key)).size() == 6) {
		r[0] = str[0]; r[1] = str[1]; r[2] = 0;
		g[0] = str[2]; g[1] = str[3]; g[2] = 0;
		b[0] = str[4]; b[1] = str[5]; b[2] = 0;

		color[0] = strtol(r, NULL, 16);
		color[1] = strtol(g, NULL, 16);
		color[2] = strtol(b, NULL, 16);
	} else if (defaultColor) {
		Common::copy(defaultColor, defaultColor + 3, color);
	} else {
		Common::fill(color, color + 3, 0);
	}
}

void Conf::get(const Common::String &key, int &field, int defaultVal) {
	field = ConfMan.hasKey(key) ? strToInt(ConfMan.get(key).c_str()) : defaultVal;
}

void Conf::get(const Common::String &key, FACES &field, FACES defaultFont) {
	field = ConfMan.hasKey(key) ? Fonts::getId(ConfMan.get(key)) : defaultFont;
}

void Conf::get(const Common::String &key, double &field, double defaultVal) {
	field = ConfMan.hasKey(key) ?  atof(ConfMan.get(key).c_str()) : defaultVal;
}

} // End of namespace Gargoyle
