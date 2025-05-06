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

#include "common/file.h"
#include "graphics/palette.h"
#include "image/bmp.h"
#include "bagel/boflib/palette.h"
#include "bagel/spacebar/boflib/app.h"
#include "bagel/spacebar/boflib/file.h"
#include "bagel/boflib/file_functions.h"

namespace Bagel {
namespace SpaceBar {

CBofPalette *CBofPalette::_pSharedPalette;
char CBofPalette::_szSharedPalFile[MAX_FNAME];

PaletteData::PaletteData(int16 numColors) : _numColors(numColors) {
	Common::fill(&_data[0], &_data[Graphics::PALETTE_SIZE], 0);
}

void CBofPalette::initialize() {
	_pSharedPalette = nullptr;
	_szSharedPalFile[0] = '\0';
}

CBofPalette::CBofPalette() {
	Common::fill(&_palette._data[0], &_palette._data[Graphics::PALETTE_SIZE], 0);
}

CBofPalette::CBofPalette(const char *pszFileName) {
	Common::fill(&_palette._data[0], &_palette._data[Graphics::PALETTE_SIZE], 0);
	assert(pszFileName != nullptr);

	loadPalette(pszFileName);
}

CBofPalette::CBofPalette(const PaletteData &paletteData) {
	_palette = paletteData;
}

CBofPalette::~CBofPalette() {
	assert(isValidObject(this));

	// If we trash the games palette, then reset it back to nullptr.
	CBofApp *pApp = CBofApp::getApp();
	if (pApp != nullptr) {
		if (this == pApp->getPalette()) {
			pApp->setPalette(nullptr);
		}
	}

	ReleasePalette();
}

void CBofPalette::setPalette(const PaletteData &PaletteData) {
	assert(isValidObject(this));

	ReleasePalette();
	_palette = PaletteData;
}

ErrorCode CBofPalette::loadPalette(const char *pszFileName, uint16 nFlags) {
	assert(isValidObject(this));

	// Validate input
	assert(pszFileName != nullptr);

	ReleasePalette();

	// Load in new bitmap file to get the palette
	Common::File f;
	Image::BitmapDecoder decoder;

	if (f.open(pszFileName) && decoder.loadStream(f)) {
		// Copy the palette
		const Graphics::Palette &pal = decoder.getPalette();
		_palette._numColors = pal.size();
		pal.grab(_palette._data, 0, pal.size());

		_errCode = ERR_NONE;

	} else {
		_errCode = ERR_FREAD;
	}

	return _errCode;
}

void CBofPalette::ReleasePalette() {
	Common::fill(_palette._data, _palette._data + Graphics::PALETTE_SIZE, 0);
}

CBofPalette *CBofPalette::copyPalette() {
	assert(isValidObject(this));

	return nullptr;
}

byte CBofPalette::getNearestIndex(COLORREF stRGB) {
	Graphics::PaletteLookup lookup(_palette._data, Graphics::PALETTE_COUNT);
	return lookup.findBestColor(GetRed(stRGB), GetGreen(stRGB), GetBlue(stRGB));
}

COLORREF CBofPalette::getColor(byte nIndex) {
	const byte *rgb = &_palette._data[nIndex * 3];

	COLORREF cColor = RGB(rgb[0], rgb[1], rgb[2]);
	return cColor;
}

ErrorCode CBofPalette::createDefault(uint16 nFlags) {
	assert(isValidObject(this));

	byte *pal = _palette._data;
	for (int i = 0; i < 256; ++i, pal += 3)
		pal[0] = pal[1] = pal[2] = (byte)i;

	return ERR_NONE;
}

ErrorCode CBofPalette::setSharedPalette(const char *pszFileName) {
	delete _pSharedPalette;
	_pSharedPalette = nullptr;

	// Save name of file used to get the shared palette
	if (pszFileName != nullptr) {
		Common::strcpy_s(_szSharedPalFile, pszFileName);
	}

	return ERR_NONE;
}

CBofPalette *CBofPalette::getSharedPalette() {
	// Do we need to load the shared palette?
	if (_pSharedPalette == nullptr && fileExists(_szSharedPalFile))
		_pSharedPalette = new CBofPalette(_szSharedPalFile);

	return _pSharedPalette;
}

} // namespace SpaceBar
} // namespace Bagel
