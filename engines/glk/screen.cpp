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

#include "glk/screen.h"
#include "glk/conf.h"
#include "common/compression/unzip.h"
#include "image/bmp.h"
#include "graphics/fonts/ttf.h"
#include "graphics/fontman.h"

namespace Glk {


#define FONTS_FILENAME "fonts.dat"

Screen::~Screen() {
	for (uint index = 0; index < _fonts.size(); ++index)
		delete _fonts[index];
}

void Screen::initialize() {
	loadFonts();

	for (int idx = 0; idx < 2; ++idx) {
		FontInfo *i = (idx == 0) ? &g_conf->_monoInfo : &g_conf->_propInfo;
		const Graphics::Font *f = (idx == 0) ? _fonts[0] : _fonts[7];

		measureFont(*i, *f, *_fonts[0], idx == 0 ? 0 : g_conf->_propInfo._lineSeparation);
	}
}

void Screen::fill(uint color) {
	clear(color);
}

void Screen::fillRect(const Rect &box, uint color) {
	if (color != zcolor_Transparent)
		Graphics::Screen::fillRect(box, color);
}

void Screen::measureFont(FontInfo &info, const Graphics::Font &font,
		const Graphics::Font &fixedFont, int lineSeparation) {
	const Common::Rect baseline = font.getBoundingBox('o');
	const Common::Rect descender = font.getBoundingBox('y');
	info._leading = MAX(info._leading, descender.bottom + lineSeparation);
	info._baseLine = MAX(info._baseLine, (int)baseline.bottom);
	info._cellW = fixedFont.getMaxCharWidth();
	info._cellH = info._leading;
}

Common::Archive *Screen::openFontArchive(const Common::Archive *resources) {
	Common::Archive *archive = resources ?
		Common::makeZipArchive(resources->createReadStreamForMember(FONTS_FILENAME)) :
		Common::makeZipArchive(FONTS_FILENAME);
	if (!archive)
		return nullptr;
	Common::File version;
	char buffer[5] = { 0, 0, 0, 0, 0 };
	if (!version.open("version.txt", *archive) || version.read(buffer, 4) != 4 ||
			buffer[1] != '.' || buffer[0] < '1' || (buffer[0] == '1' && atoi(buffer + 2) < 2)) {
		delete archive;
		return nullptr;
	}
	return archive;
}

void Screen::loadFonts() {
	Common::Archive *archive = openFontArchive();
	if (!archive)
		error("Could not load compatible GLK fonts from %s (version 1.2 or newer required)", FONTS_FILENAME);
	loadFonts(archive);
	delete archive;
}

void Screen::loadFonts(Common::Archive *archive) {
	// R ead in the fonts
	double monoAspect = g_conf->_monoInfo._aspect;
	double propAspect = g_conf->_propInfo._aspect;
	double monoSize = g_conf->_monoInfo._size;
	double propSize = g_conf->_propInfo._size;

	for (uint index = 0; index < _fonts.size(); ++index)
		delete _fonts[index];
	_fonts.clear();
	_fonts.resize(FONTS_TOTAL);
	_fonts[0] = loadFont(MONOR, archive, monoSize, monoAspect, FONTR);
	_fonts[1] = loadFont(MONOB, archive, monoSize, monoAspect, FONTB);
	_fonts[2] = loadFont(MONOI, archive, monoSize, monoAspect, FONTI);
	_fonts[3] = loadFont(MONOZ, archive, monoSize, monoAspect, FONTZ);

	_fonts[4] = loadFont(PROPR, archive, propSize, propAspect, FONTR);
	_fonts[5] = loadFont(PROPB, archive, propSize, propAspect, FONTB);
	_fonts[6] = loadFont(PROPI, archive, propSize, propAspect, FONTI);
	_fonts[7] = loadFont(PROPZ, archive, propSize, propAspect, FONTZ);
}

const Graphics::Font *Screen::loadFont(FACES face, Common::Archive *archive, double size, double aspect, int style) {
	const Graphics::Font *font = loadFontFromArchive(face, archive, size);
	if (!font)
		error("Could not load GLK font %s", getFontName(face).c_str());
	return font;
}

const Graphics::Font *Screen::loadFontFromArchive(FACES face,
		Common::Archive *archive, double size) {
	if (!archive || face < MONOR || face > PROPZ || !(size >= 1.0 && size <= 32767.0))
		return nullptr;
	Common::File *f = new Common::File();
	const char *const FILENAMES[8] = {
		"GoMono-Regular.ttf", "GoMono-Bold.ttf", "GoMono-Italic.ttf", "GoMono-Bold-Italic.ttf",
		"NotoSerif-Regular.ttf", "NotoSerif-Bold.ttf", "NotoSerif-Italic.ttf", "NotoSerif-Bold-Italic.ttf"
	};

	if (!f->open(FILENAMES[face], *archive)) {
		delete f;
		return nullptr;
	}

	const Graphics::Font *font = Graphics::loadTTFFont(f, DisposeAfterUse::YES, (int)size, Graphics::kTTFSizeModeCharacter);
	if (!font)
		delete f;
	return font;
}

Common::String Screen::getFontName(FACES font) {
	if (font == MONOR) return "monor";
	if (font == MONOB) return "monob";
	if (font == MONOI) return "monoi";
	if (font == MONOZ) return "monoz";
	if (font == PROPR) return "propr";
	if (font == PROPB) return "propb";
	if (font == PROPI) return "propi";
	if (font == PROPZ) return "propz";
	return "monor";
}

int Screen::drawString(const Point &pos, int fontIdx, uint color, const Common::String &text, int spw) {
	int baseLine = (fontIdx >= PROPR) ? g_conf->_propInfo._baseLine : g_conf->_monoInfo._baseLine;
	Point pt(pos.x / GLI_SUBPIX, pos.y - baseLine);
	const Graphics::Font *font = _fonts[fontIdx];
	font->drawString(this, text, pt.x, pt.y, w - pt.x, color);

	pt.x += font->getStringWidth(text);
	return MIN((int)pt.x, (int)w) * GLI_SUBPIX;
}

int Screen::drawStringUni(const Point &pos, int fontIdx, uint color, const Common::U32String &text, int spw) {
	int baseLine = (fontIdx >= PROPR) ? g_conf->_propInfo._baseLine : g_conf->_monoInfo._baseLine;
	Point pt(pos.x / GLI_SUBPIX, pos.y - baseLine);
	const Graphics::Font *font = _fonts[fontIdx];
	
	int xSub = pos.x;
	for (auto c : text) {
		int xPix = xSub / GLI_SUBPIX;
		font->drawString(this, Common::U32String(1, c), xPix, pt.y, w - xPix, color);
		int charWidthPx = font->getStringWidth(Common::U32String(1, c));
		xSub += charWidthPx * GLI_SUBPIX;
		if (spw > 0 && c == ' ')
			xSub += spw;
	}
	int xFinalPix = MIN(xSub / GLI_SUBPIX, (int)w);
	return xFinalPix * GLI_SUBPIX;
}

size_t Screen::stringWidth(int fontIdx, const Common::String &text, int spw) {
	const Graphics::Font *font = _fonts[fontIdx];
	return font->getStringWidth(text) * GLI_SUBPIX;
}

size_t Screen::stringWidthUni(int fontIdx, const Common::U32String &text, int spw) {
	const Graphics::Font *font = _fonts[fontIdx];
	size_t width = font->getStringWidth(text)*GLI_SUBPIX;

	if (spw > 0) {
        int spaces = 0;
        for (auto c : text)
            if (c == ' ')
                spaces++;

        width += (size_t)spaces * (size_t)spw;
    }
    return width;
}

} // End of namespace Glk
