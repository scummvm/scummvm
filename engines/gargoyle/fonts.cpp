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

#include "gargoyle/fonts.h"
#include "gargoyle/glk_types.h"
#include "gargoyle/conf.h"

namespace Gargoyle {

const char *gli_conf_propr = "NotoSerif-Regular";
const char *gli_conf_propb = "NotoSerif-Bold";
const char *gli_conf_propi = "NotoSerif-Italic";
const char *gli_conf_propz = "NotoSerif-BoldItalic";

const char *gli_conf_monor = "GoMono-Regular";
const char *gli_conf_monob = "GoMono-Bold";
const char *gli_conf_monoi = "GoMono-Italic";
const char *gli_conf_monoz = "GoMono-BoldItalic";

#ifdef BUNDLED_FONTS
const char *gli_conf_monofont = "";
const char *gli_conf_propfont = "";
const double gli_conf_monosize = 12.5;	///< good size for GoMono
const double gli_conf_propsize = 13.4;	///< good size for NotoSerif
#else
const char *gli_conf_monofont = "Liberation Mono";
const char *gli_conf_propfont = "Linux Libertine O";
const double gli_conf_monosize = 12.5;	///< good size for LiberationMono
const double gli_conf_propsize = 15.5;	///< good size for Libertine
#endif

Fonts::Fonts() {
	double monoAspect = g_conf->_monoAspect;
	double propAspect = g_conf->_propAspect;
	double monoSize = g_conf->_monoSize;
	double propSize = g_conf->_propSize;

	_fontTable[0] = new Font(gli_conf_monor, monoSize, monoAspect, FONTR);
	_fontTable[1] = new Font(gli_conf_monob, monoSize, monoAspect, FONTB);
	_fontTable[2] = new Font(gli_conf_monoi, monoSize, monoAspect, FONTI);
	_fontTable[3] = new Font(gli_conf_monoz, monoSize, monoAspect, FONTZ);

	_fontTable[4] = new Font(gli_conf_propr, propSize, propAspect, FONTR);
	_fontTable[5] = new Font(gli_conf_propb, propSize, propAspect, FONTB);
	_fontTable[6] = new Font(gli_conf_propi, propSize, propAspect, FONTI);
	_fontTable[7] = new Font(gli_conf_propz, propSize, propAspect, FONTZ);
}

Fonts::~Fonts() {
	for (int idx = 0; idx < FONTS_TOTAL; ++idx)
		delete _fontTable[idx];
}

FACES Fonts::getId(const Common::String &name) {
	if (name == "monor") return MONOR;
	if (name == "monob") return MONOB;
	if (name == "monoi") return MONOI;
	if (name == "monoz") return MONOZ;
	if (name == "propr") return PROPR;
	if (name == "propb") return PROPB;
	if (name == "propi") return PROPI;
	if (name == "propz") return PROPZ;
	return MONOR;
}

/*--------------------------------------------------------------------------*/



} // End of namespace Gargoyle
