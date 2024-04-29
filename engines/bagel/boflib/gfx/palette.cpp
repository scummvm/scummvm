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
#include "bagel/boflib/gfx/palette.h"
#include "bagel/boflib/app.h"
#include "bagel/boflib/file.h"
#include "bagel/boflib/file_functions.h"

namespace Bagel {

CBofPalette *CBofPalette::m_pSharedPalette;
char CBofPalette::m_szSharedPalFile[MAX_FNAME];

HPALETTE::HPALETTE(int16 numColors) : _numColors(numColors) {
	Common::fill(&_data[0], &_data[PALETTE_SIZE], 0);
}

void CBofPalette::initialize() {
	m_pSharedPalette = nullptr;
	m_szSharedPalFile[0] = '\0';
}

CBofPalette::CBofPalette() {
	Common::fill(&_palette._data[0], &_palette._data[PALETTE_SIZE], 0);
}

CBofPalette::CBofPalette(const char *pszFileName) {
	Common::fill(&_palette._data[0], &_palette._data[PALETTE_SIZE], 0);
	Assert(pszFileName != nullptr);

	LoadPalette(pszFileName);
}

CBofPalette::CBofPalette(const HPALETTE &hPalette) {
	_palette = hPalette;
}

CBofPalette::~CBofPalette() {
	Assert(IsValidObject(this));

	// If we trash the games palette, then reset it back to nullptr.
	CBofApp *pApp;
	if ((pApp = CBofApp::GetApp()) != nullptr) {
		if (this == pApp->getPalette()) {
			pApp->setPalette(nullptr);
		}
	}

	ReleasePalette();
}

void CBofPalette::setPalette(const HPALETTE &hPalette) {
	Assert(IsValidObject(this));

	ReleasePalette();
	_palette = hPalette;
}

ErrorCode CBofPalette::LoadPalette(const char *pszFileName, uint16 nFlags) {
	Assert(IsValidObject(this));

	// Validate input
	Assert(pszFileName != nullptr);

	ReleasePalette();

	// Load in new bitmap file to get the palette
	Common::File f;
	Image::BitmapDecoder decoder;

	if (f.open(pszFileName) && decoder.loadStream(f)) {
		// Copy the palette
		_palette._numColors = decoder.getPaletteColorCount();
		const byte *src = decoder.getPalette();
		Common::copy(src, src + _palette._numColors * 3, _palette._data);

		_errCode = ERR_NONE;

	} else {
		_errCode = ERR_FREAD;
	}

	return _errCode;
}

void CBofPalette::ReleasePalette() {
	Common::fill(_palette._data, _palette._data + PALETTE_SIZE, 0);
}

CBofPalette *CBofPalette::CopyPalette() {
	Assert(IsValidObject(this));

	return nullptr;
}

byte CBofPalette::GetNearestIndex(RGBCOLOR stRGB) {
	Graphics::PaletteLookup lookup(_palette._data, PALETTE_COUNT);
	return lookup.findBestColor(GetRed(stRGB), GetGreen(stRGB), GetBlue(stRGB));
}

RGBCOLOR CBofPalette::getColor(byte nIndex) {
	const byte *rgb = &_palette._data[nIndex * 3];

	RGBCOLOR cColor = RGB(rgb[0], rgb[1], rgb[2]);
	return cColor;
}

ErrorCode CBofPalette::CreateDefault(uint16 nFlags) {
	Assert(IsValidObject(this));

	byte *pal = _palette._data;
	for (int i = 0; i < 256; ++i, pal += 3)
		pal[0] = pal[1] = pal[2] = (byte)i;

	return ERR_NONE;
}

ErrorCode CBofPalette::SetSharedPalette(const char *pszFileName) {
	if (m_pSharedPalette != nullptr) {
		delete m_pSharedPalette;
		m_pSharedPalette = nullptr;
	}

	// Save name of file used to get the shared palette
	if (pszFileName != nullptr) {
		Common::strcpy_s(m_szSharedPalFile, pszFileName);
	}

	return ERR_NONE;
}

CBofPalette *CBofPalette::GetSharedPalette() {
	// Do we need to load the shared palette?
	if (m_pSharedPalette == nullptr) {
		if (FileExists(m_szSharedPalFile)) {
			m_pSharedPalette = new CBofPalette(m_szSharedPalFile);
		}
	}

	return m_pSharedPalette;
}

} // namespace Bagel
