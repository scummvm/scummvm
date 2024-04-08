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

namespace Bagel {

CBofPalette *CBofPalette::m_pSharedPalette;
char CBofPalette::m_szSharedPalFile[MAX_FNAME];

HPALETTE::HPALETTE(int16 numColors) : _numColors(numColors) {
	Common::fill(&_data[0], &_data[PALETTE_SIZE], 0);
}

HPALETTE CreatePalette(const LOGPALETTE *pal) {
	HPALETTE result(pal->palNumEntries);

	const PALETTEENTRY *src = &pal->palPalEntry[0];
	for (int i = 0; i < pal->palNumEntries; ++i, ++src) {
		result._data[i * 3 + 0] = src->peRed;
		result._data[i * 3 + 1] = src->peGreen;
		result._data[i * 3 + 2] = src->peBlue;
	}

	return result;
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
		if (this == pApp->GetPalette()) {
			pApp->SetPalette(nullptr);
		}
	}

	ReleasePalette();
}

void CBofPalette::SetPalette(const HPALETTE &hPalette) {
	Assert(IsValidObject(this));

	ReleasePalette();
	_palette = hPalette;
}

ErrorCode CBofPalette::LoadPalette(const char *pszFileName, uint16 nFlags) {
	Assert(IsValidObject(this));

	// validate input
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

		m_errCode = ERR_NONE;

	} else {
		m_errCode = ERR_FREAD;
	}

	return m_errCode;
}

void CBofPalette::ReleasePalette() {
	Common::fill(_palette._data, _palette._data + PALETTE_SIZE, 0);
}

CBofPalette *CBofPalette::CopyPalette() {
	Assert(IsValidObject(this));

	CBofPalette *pBofPal;

	pBofPal = nullptr;

#if BOF_WINDOWS
	HPALETTE hPal;

	if ((hPal = CopyWindowsPalette(_palette)) != nullptr) {

		if ((pBofPal = new CBofPalette(hPal)) != nullptr) {

		} else {
			LogError("Unable to allocate a new CBofPalette");
		}

	} else {
		LogError("CopyWindowsPalette failed");
	}
#elif BOF_MAC

#endif

	return pBofPal;
}

byte CBofPalette::GetNearestIndex(RGBCOLOR stRGB) {
	Graphics::PaletteLookup lookup(_palette._data, PALETTE_COUNT);
	return lookup.findBestColor(GetRed(stRGB), GetGreen(stRGB), GetBlue(stRGB));
}

#if BOF_WINDOWS
/**
 * Creates a new palette based on specified palette
 * @param hPal      Palette to copy
 * @return          New palette
 */
HPALETTE CopyWindowsPalette(HPALETTE hPal) {
	LPLOGPALETTE ppal;
	WORD nNumEntries = 0;

	if (hPal != nullptr) {

		GetObject(hPal, sizeof(WORD), (LPSTR)&nNumEntries);

		if (nNumEntries == 0)
			return nullptr;

		if ((ppal = (LPLOGPALETTE)BofAlloc(sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * nNumEntries)) != nullptr) {

			ppal->palVersion = 0x0300;
			ppal->palNumEntries = (WORD)nNumEntries;

			GetPaletteEntries(hPal, 0, nNumEntries, ppal->palPalEntry);

			hPal = CreatePalette(ppal);
			BofFree(ppal);
		}
	}

	return hPal;
}
#endif

RGBCOLOR CBofPalette::GetColor(byte nIndex) {
	const byte *rgb = &_palette._data[nIndex * 3];

	RGBCOLOR cColor = RGB(rgb[0], rgb[1], rgb[2]);
	return cColor;
}

void CBofPalette::AnimateEntry(byte nIndex, RGBCOLOR cColor) {
	Assert(IsValidObject(this));

#if BOF_WINDOWS

// Palette Animation is not supported in the cross compiler
#if !BOF_WINMAC

	PALETTEENTRY stColor;

	stColor.peRed = GetRed(cColor);
	stColor.peGreen = GetGreen(cColor);
	stColor.peBlue = GetBlue(cColor);
	stColor.peFlags = PC_RESERVED;

	::AnimatePalette(_palette, nIndex, 1, &stColor);
#endif

#elif BOF_MAC
	RGBColor stColor;
	stColor.red = (uint16)GetRed(cColor) << 8;
	stColor.green = (uint16)GetGreen(cColor) << 8;
	stColor.blue = (uint16)GetBlue(cColor) << 8;

	::AnimateEntry(CBofApp::GetApp()->GetMainWindow()->GetMacWindow(), nIndex, &stColor);

#endif
}

void CBofPalette::AnimateToPalette(CBofPalette *pSrcPal) {
	Assert(IsValidObject(this));
	Assert(pSrcPal != nullptr);

	RGBCOLOR cColor;
	int i;

	for (i = 0; i < 256; i++) {
		cColor = pSrcPal->GetColor((byte)i);
		AnimateEntry((byte)i, cColor);
	}
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
	//
	if (pszFileName != nullptr) {
		Common::strcpy_s(m_szSharedPalFile, pszFileName);

		// Don't load it yet
	}

	return ERR_NONE;
}

CBofPalette *CBofPalette::GetSharedPalette() {
	// Do we need to load the shared palette?
	//
	if (m_pSharedPalette == nullptr) {

		if (FileExists(m_szSharedPalFile)) {
			m_pSharedPalette = new CBofPalette(m_szSharedPalFile);
		}
	}

	return m_pSharedPalette;
}

} // namespace Bagel
