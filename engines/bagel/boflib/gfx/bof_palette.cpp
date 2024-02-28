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

#include "graphics/palette.h"
#include "bagel/boflib/gfx/bof_palette.h"
#include "bagel/boflib/bof_app.h"
#include "bagel/boflib/bof_file.h"

namespace Bagel {

CBofPalette *CBofPalette::m_pSharedPalette;
CHAR CBofPalette::m_szSharedPalFile[MAX_FNAME];

void CBofPalette::initStatics() {
	m_pSharedPalette = nullptr;
	m_szSharedPalFile[0] = '\0';
}

CBofPalette::CBofPalette() {
	Common::fill(&m_hPalette._data[0], &m_hPalette._data[PALETTE_SIZE], 0);
}

CBofPalette::CBofPalette(const CHAR *pszFileName) {
	Common::fill(&m_hPalette._data[0], &m_hPalette._data[PALETTE_SIZE], 0);
	Assert(pszFileName != nullptr);

	LoadPalette(pszFileName);
}

CBofPalette::CBofPalette(const HPALETTE &hPalette) {
	m_hPalette = hPalette;
}

CBofPalette::~CBofPalette() {
	Assert(IsValidObject(this));

	// jwl 08.08.96 if we trash the games palette, then reset it back
	// to nullptr.
	CBofApp *pApp;
	if ((pApp = CBofApp::GetApp()) != nullptr) {

		if (this == pApp->GetPalette()) {
			pApp->SetPalette(nullptr);
		}
	}

	ReleasePalette();
}

VOID CBofPalette::SetPalette(const HPALETTE &hPalette) {
	Assert(IsValidObject(this));

	ReleasePalette();
	m_hPalette = hPalette;
}

ERROR_CODE CBofPalette::LoadPalette(const CHAR *pszFileName, USHORT nFlags) {
	Assert(IsValidObject(this));

	// validate input
	Assert(pszFileName != nullptr);

	ReleasePalette();

	static BOFRGBQUAD cRGB[256];
	BOFBITMAPFILEHEADER cBmpFileHeader;
	BOFBITMAPINFOHEADER cBmpInfoHeader;
	CBofFile *pFile;
	//INT i;
	INT nNumColors;

	// Open bitmap
	if ((pFile = new CBofFile(pszFileName, CBOFFILE_READONLY)) != nullptr) {
		// Read header
		if (pFile->Read(&cBmpFileHeader, sizeof(BOFBITMAPFILEHEADER)) == ERR_NONE) {

#if BOF_MAC || BOF_WINMAC
			//
			// swap bytes for Macintosh Big-Endian
			//
			cBmpFileHeader.bfType = SWAPWORD(cBmpFileHeader.bfType);
			cBmpFileHeader.bfSize = SWAPLONG(cBmpFileHeader.bfSize);
			cBmpFileHeader.bfOffBits = SWAPLONG(cBmpFileHeader.bfOffBits);
#endif

			if (pFile->Read(&cBmpInfoHeader, sizeof(BOFBITMAPINFOHEADER)) == ERR_NONE) {

#if BOF_MAC || BOF_WINMAC
				//
				// swap bytes for Macintosh Big-Endian
				//
				cBmpInfoHeader.biSize = SWAPLONG(cBmpInfoHeader.biSize);
				cBmpInfoHeader.biWidth = SWAPLONG(cBmpInfoHeader.biWidth);
				cBmpInfoHeader.biHeight = SWAPLONG(cBmpInfoHeader.biHeight);
				cBmpInfoHeader.biPlanes = SWAPWORD(cBmpInfoHeader.biPlanes);
				cBmpInfoHeader.biBitCount = SWAPWORD(cBmpInfoHeader.biBitCount);
				cBmpInfoHeader.biCompression = SWAPLONG(cBmpInfoHeader.biCompression);
				cBmpInfoHeader.biSizeImage = SWAPLONG(cBmpInfoHeader.biSizeImage);
				cBmpInfoHeader.biXPelsPerMeter = SWAPLONG(cBmpInfoHeader.biXPelsPerMeter);
				cBmpInfoHeader.biYPelsPerMeter = SWAPLONG(cBmpInfoHeader.biYPelsPerMeter);
				cBmpInfoHeader.biClrUsed = SWAPLONG(cBmpInfoHeader.biClrUsed);
				cBmpInfoHeader.biClrImportant = SWAPLONG(cBmpInfoHeader.biClrImportant);
#endif

				if ((nNumColors = (INT)cBmpInfoHeader.biClrUsed) == 0) {
					nNumColors = (INT)(1 << (INT)cBmpInfoHeader.biBitCount);
				}
				Assert(nNumColors <= 256);

				// load the actual palette colors from the bitmap
				//
				if (pFile->Read(&cRGB, sizeof(BOFRGBQUAD) * nNumColors) == ERR_NONE) {

#if BOF_MAC || BOF_WINMAC
					// Macintize this palette
					//
					/// BOFRGBQUAD stTempEntry;

					/// stTempEntry = cRGB[0];
					/// cRGB[0] = cRGB[255];
					/// cRGB[255] = stTempEntry;
#endif

#if BOF_WINDOWS
					LPLOGPALETTE pLogPal;
					HPALETTE hPal;

					// assume failure
					hPal = nullptr;

					// build a logical palette from these colors.
					// (this is needed to create a CPalette)
					//
					if ((pLogPal = (LPLOGPALETTE)BofAlloc(sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * nNumColors)) != nullptr) {

						pLogPal->palNumEntries = (WORD)nNumColors;
						pLogPal->palVersion = 0x300;

						if (nFlags & PAL_EXPLICIT) {

							for (i = 0; i < nNumColors; i++) {
								pLogPal->palPalEntry[i].peRed = cRGB[i].rgbRed;
								pLogPal->palPalEntry[i].peGreen = cRGB[i].rgbGreen;
								pLogPal->palPalEntry[i].peBlue = cRGB[i].rgbBlue;

								pLogPal->palPalEntry[i].peFlags = (UBYTE)(PC_EXPLICIT | i);
							}

						} else if (nFlags & PAL_ANIMATED) {

							for (i = 0; i < nNumColors; i++) {
								pLogPal->palPalEntry[i].peRed = cRGB[i].rgbRed;
								pLogPal->palPalEntry[i].peGreen = cRGB[i].rgbGreen;
								pLogPal->palPalEntry[i].peBlue = cRGB[i].rgbBlue;

								pLogPal->palPalEntry[i].peFlags = PC_RESERVED; // | PC_NOCOLLAPSE;
								if (i < 10 || i >= 245)
									pLogPal->palPalEntry[i].peFlags = 0;
							}

						} else {

							for (i = 0; i < nNumColors; i++) {
								pLogPal->palPalEntry[i].peRed = cRGB[i].rgbRed;
								pLogPal->palPalEntry[i].peGreen = cRGB[i].rgbGreen;
								pLogPal->palPalEntry[i].peBlue = cRGB[i].rgbBlue;

								pLogPal->palPalEntry[i].peFlags = 0;
							}
						}

						if ((hPal = ::CreatePalette(pLogPal)) != nullptr) {
							m_hPalette = hPal;
						}

						BofFree(pLogPal);

					} else {
						LogError(BuildString("Error allocating %ld bytes for palette", sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * nNumColors));
					}
#elif BOF_MAC

					if ((m_hPalette = NewPalette(nNumColors, 0, pmExplicit | pmAnimated, 0)) != nullptr) {
						// jwl 10.07.96 move this bad baby way up in the heap
						::MoveHHi((Handle)m_hPalette);

						RGBColor rgb;

						for (i = 0; i < nNumColors; i++) {

							rgb.red = cRGB[i].rgbRed << 8;
							rgb.green = cRGB[i].rgbGreen << 8;
							rgb.blue = cRGB[i].rgbBlue << 8;

							SetEntryColor(m_hPalette, i, &rgb);
						}

						// SetEntryUsage(m_hPalette, 0, pmExplicit | pmAnimated, 0);
						// SetEntryUsage(m_hPalette, 255, pmExplicit | pmAnimated, 0);

					} else {
						LogError(BuildString("Unable to allocate NewPalette() from %s", pszFileName));
					}

#endif

				} else {
					LogError(BuildString("Error reading palette entries from %s", pszFileName));
				}

			} else {
				LogError(BuildString("Error reading BOFBITMAPINFOHEADER from %s", pszFileName));
			}

			// read palette entries
			//
		} else {
			LogError(BuildString("Error reading BOFBITMAPFILEHEADER from %s", pszFileName));
		}

		// close bitmap
		delete pFile;

	} else {
		LogError(BuildString("Could not allocate a CBofFile for %s", pszFileName));
	}

	return (m_errCode);
}

VOID CBofPalette::ReleasePalette() {
#if BOF_WINDOWS
	if (m_hPalette != nullptr) {
		if (::DeleteObject(m_hPalette) == FALSE) {
			LogError(BuildString("::DeleteObject() failed"));
		}
		m_hPalette = nullptr;
	}
#elif BOF_MAC
	if (m_hPalette != nullptr) {
#if PALETTESHIFTFIX // scg 01.26.97

		CBofWindow::AddToPaletteShiftList(DISPOSEPALETTE, (LONG)m_hPalette);

#else
		extern BOOL gAllowPaletteShifts;

		gAllowPaletteShifts = false;
		::DisposePalette(m_hPalette);
		gAllowPaletteShifts = true;
#endif
		m_hPalette = nullptr;
	}
#endif
}

CBofPalette *CBofPalette::CopyPalette() {
	Assert(IsValidObject(this));

	CBofPalette *pBofPal;

	pBofPal = nullptr;

#if BOF_WINDOWS
	HPALETTE hPal;

	if ((hPal = CopyWindowsPalette(m_hPalette)) != nullptr) {

		if ((pBofPal = new CBofPalette(hPal)) != nullptr) {

		} else {
			LogError("Unable to allocate a new CBofPalette");
		}

	} else {
		LogError("CopyWindowsPalette failed");
	}
#elif BOF_MAC

#endif

	return (pBofPal);
}

UBYTE CBofPalette::GetNearestIndex(RGBCOLOR stRGB) {
	Graphics::PaletteLookup lookup(m_hPalette._data, PALETTE_COUNT);
	return lookup.findBestColor(GetRed(stRGB), GetGreen(stRGB), GetBlue(stRGB));
}

#if BOF_WINDOWS
/**
 * Creates a new palette based on specified palette
 * @param hPal		Palette to copy
 * @return			New palette
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

	return (hPal);
}
#endif

RGBCOLOR CBofPalette::GetColor(UBYTE nIndex) {
	const byte *rgb = &m_hPalette._data[nIndex * 3];

	RGBCOLOR cColor = RGB(rgb[0], rgb[1], rgb[2]);
	return cColor;
}

VOID CBofPalette::AnimateEntry(UBYTE nIndex, RGBCOLOR cColor) {
	Assert(IsValidObject(this));

#if BOF_WINDOWS

// Palette Animation is not supported in the cross compiler
#if !BOF_WINMAC

	PALETTEENTRY stColor;

	stColor.peRed = GetRed(cColor);
	stColor.peGreen = GetGreen(cColor);
	stColor.peBlue = GetBlue(cColor);
	stColor.peFlags = PC_RESERVED;

	::AnimatePalette(m_hPalette, nIndex, 1, &stColor);
#endif

#elif BOF_MAC
	RGBColor stColor;
	stColor.red = (USHORT)GetRed(cColor) << 8;
	stColor.green = (USHORT)GetGreen(cColor) << 8;
	stColor.blue = (USHORT)GetBlue(cColor) << 8;

	::AnimateEntry(CBofApp::GetApp()->GetMainWindow()->GetMacWindow(), nIndex, &stColor);

#endif
}

VOID CBofPalette::AnimateToPalette(CBofPalette *pSrcPal) {
	Assert(IsValidObject(this));
	Assert(pSrcPal != nullptr);

	RGBCOLOR cColor;
	INT i;

	for (i = 0; i < 256; i++) {
		cColor = pSrcPal->GetColor((UBYTE)i);
		AnimateEntry((UBYTE)i, cColor);
	}
}

ERROR_CODE CBofPalette::CreateDefault(USHORT nFlags) {
	Assert(IsValidObject(this));

	static BOFRGBQUAD cRGB[256];
	INT i, nNumColors;

	for (i = 0; i < 256; i++) {
		cRGB[i].rgbRed = (BYTE)i;
		cRGB[i].rgbGreen = (BYTE)i;
		cRGB[i].rgbBlue = (BYTE)i;
		cRGB[i].rgbReserved = 0;
	}
	nNumColors = 256;

#if BOF_WINDOWS
	LPLOGPALETTE pLogPal;
	HPALETTE hPal;

	// assume failure
	hPal = nullptr;

	// build a logical palette from these colors.
	// (this is needed to create a CPalette)
	//
	if ((pLogPal = (LPLOGPALETTE)BofAlloc(sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * nNumColors)) != nullptr) {

		pLogPal->palNumEntries = (WORD)nNumColors;
		pLogPal->palVersion = 0x300;

		if (nFlags & PAL_EXPLICIT) {

			for (i = 0; i < nNumColors; i++) {
				pLogPal->palPalEntry[i].peRed = cRGB[i].rgbRed;
				pLogPal->palPalEntry[i].peGreen = cRGB[i].rgbGreen;
				pLogPal->palPalEntry[i].peBlue = cRGB[i].rgbBlue;

				pLogPal->palPalEntry[i].peFlags = (UBYTE)(PC_EXPLICIT | i);
			}

		} else if (nFlags & PAL_ANIMATED) {

			for (i = 0; i < nNumColors; i++) {
				pLogPal->palPalEntry[i].peRed = cRGB[i].rgbRed;
				pLogPal->palPalEntry[i].peGreen = cRGB[i].rgbGreen;
				pLogPal->palPalEntry[i].peBlue = cRGB[i].rgbBlue;

				pLogPal->palPalEntry[i].peFlags = PC_RESERVED; // | PC_NOCOLLAPSE;
				if (i < 10 || i >= 245)
					pLogPal->palPalEntry[i].peFlags = 0;
			}

		} else {

			for (i = 0; i < nNumColors; i++) {
				pLogPal->palPalEntry[i].peRed = cRGB[i].rgbRed;
				pLogPal->palPalEntry[i].peGreen = cRGB[i].rgbGreen;
				pLogPal->palPalEntry[i].peBlue = cRGB[i].rgbBlue;

				pLogPal->palPalEntry[i].peFlags = 0;
			}
		}

		if ((hPal = ::CreatePalette(pLogPal)) != nullptr) {
			m_hPalette = hPal;
		}

		BofFree(pLogPal);

	} else {
		LogError(BuildString("Error allocating %ld bytes for palette", sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * nNumColors));
	}
#elif BOF_MAC
#if JWL
	CTabHandle hCTab = GetCTable(72);
#endif

	if ((m_hPalette = NewPalette(nNumColors, 0, pmExplicit | pmAnimated, 0)) != nullptr) {
		// jwl 10.07.96 move this bad baby way up in the heap
		::MoveHHi((Handle)m_hPalette);

		RGBColor rgb;
		BOFRGBQUAD cTempRGB;

		cTempRGB = cRGB[0];
		cRGB[0] = cRGB[255];
		cRGB[255] = cTempRGB;

		for (i = 0; i < nNumColors; i++) {

#if JWL
			rgb = (**hCTab).ctTable[i].rgb;
#else
			rgb.red = cRGB[i].rgbRed << 8;
			rgb.green = cRGB[i].rgbGreen << 8;
			rgb.blue = cRGB[i].rgbBlue << 8;
#endif
			SetEntryColor(m_hPalette, i, &rgb);
		}
		// SetEntryUsage(m_hPalette, 0, pmExplicit | pmAnimated, 0);
		// SetEntryUsage(m_hPalette, 255, pmExplicit | pmAnimated, 0);

	} else {
		LogError(BuildString("Unable to allocate NewPalette()")); // jwl 06.26.96 remove pszFileName
	}
#endif

#if JWL
	::DisposeCTable(hCTab);
#endif
	return (m_errCode);
}

ERROR_CODE CBofPalette::SetSharedPalette(const CHAR *pszFileName) {
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
