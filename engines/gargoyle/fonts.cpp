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
#include "gargoyle/gargoyle.h"
#include "common/memstream.h"
#include "common/unzip.h"
#include "graphics/fonts/ttf.h"
#include "graphics/fontman.h"

namespace Gargoyle {

#define FONTS_VERSION 1.0
#define FONTS_FILENAME "fonts.dat"

Fonts::Fonts(Graphics::ManagedSurface *surface) : _surface(surface), _fontsMissing(false) {
	if (!loadFonts())
		error("Could not load data file");

	if (!g_conf->_leading)
		g_conf->_leading = g_conf->_propSize + 2;
	if (!g_conf->_baseLine)
		g_conf->_baseLine = g_conf->_propSize + 0.5;

	g_conf->_cellW = _fontTable[0]->getStringWidth("0");
	g_conf->_cellH = g_conf->_leading;
}

Fonts::~Fonts() {
	for (int idx = 0; idx < FONTS_TOTAL; ++idx)
		delete _fontTable[idx];
}

bool Fonts::loadFonts() {
	Common::Archive *archive = nullptr;

	if (!Common::File::exists(FONTS_FILENAME) || (archive = Common::makeZipArchive(FONTS_FILENAME)) == nullptr)
		return false;

	// Open the version.txt file within it to validate the version
	Common::File f;
	if (!f.open("version.txt", *archive)) {
		delete archive;
		return false;
	}

	// Validate the version
	char buffer[4];
	f.read(buffer, 3);
	buffer[3] = '\0';

	if (Common::String(buffer) != "1.0") {
		delete archive;
		return false;
	}

	// R ead in the fonts
	double monoAspect = g_conf->_monoAspect;
	double propAspect = g_conf->_propAspect;
	double monoSize = g_conf->_monoSize;
	double propSize = g_conf->_propSize;

	_fontTable[0] = loadFont(MONOR, archive, monoSize, monoAspect, FONTR);
	_fontTable[1] = loadFont(MONOB, archive, monoSize, monoAspect, FONTB);
	_fontTable[2] = loadFont(MONOI, archive, monoSize, monoAspect, FONTI);
	_fontTable[3] = loadFont(MONOZ, archive, monoSize, monoAspect, FONTZ);

	_fontTable[4] = loadFont(PROPR, archive, propSize, propAspect, FONTR);
	_fontTable[5] = loadFont(PROPB, archive, propSize, propAspect, FONTB);
	_fontTable[6] = loadFont(PROPI, archive, propSize, propAspect, FONTI);
	_fontTable[7] = loadFont(PROPZ, archive, propSize, propAspect, FONTZ);

	delete archive;
	return true;
}

const Graphics::Font *Fonts::loadFont(FACES face, Common::Archive *archive, double size, double aspect, int
 style) {
	const char *const FILENAMES[8] = {
		"GoMono-Regular.ttf", "GoMono-Bold.ttf", "GoMono-Italic.ttf", "GoMono-Bold-Italic.ttf",
		"NotoSerif-Regular.ttf", "NotoSerif-Bold.ttf", "NotoSerif-Italic.ttf", "NotoSerif-Bold-Italic.ttf"
	};
	
	Common::File f;
	if (!f.open(FILENAMES[face], *archive))
		error("Could not load font");

	return Graphics::loadTTFFont(f, size, Graphics::kTTFSizeModeCharacter);
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

int Fonts::drawString(const Point &pos, int fontIdx, const byte *rgb, const Common::String &text, int spw) {
	Point pt(pos.x / GLI_SUBPIX, pos.y - g_conf->_baseLine);
	const Graphics::Font *font = _fontTable[fontIdx];
	const uint32 color = _surface->format.RGBToColor(rgb[0], rgb[1], rgb[2]);
	font->drawString(_surface, text, pt.x, pt.y, _surface->w - pt.x, color);

	pt.x += font->getStringWidth(text);
	return MIN((int)pt.x, (int)_surface->w) * GLI_SUBPIX;
}

int Fonts::drawStringUni(const Point &pos, int fontIdx, const byte *rgb, const Common::U32String &text, int spw) {
	Point pt(pos.x / GLI_SUBPIX, pos.y - g_conf->_baseLine);
	const Graphics::Font *font = _fontTable[fontIdx];
	const uint32 color = _surface->format.RGBToColor(rgb[0], rgb[1], rgb[2]);
	font->drawString(_surface, text, pt.x, pt.y, _surface->w - pt.x, color);

	pt.x += font->getStringWidth(text);
	return MIN((int)pt.x, (int)_surface->w) * GLI_SUBPIX;
}

size_t Fonts::stringWidth(int fontIdx, const Common::String &text, int spw) {
	// TODO: Handle spw
	const Graphics::Font *font = _fontTable[fontIdx];
	return font->getStringWidth(text) * GLI_SUBPIX;
}

size_t Fonts::stringWidthUni(int fontIdx, const Common::U32String &text, int spw) {
	// TODO: Handle spw
	const Graphics::Font *font = _fontTable[fontIdx];
	return font->getStringWidth(text) * GLI_SUBPIX;
}

} // End of namespace Gargoyle
