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
#include "bagel/hodjnpodj/hnplibs/dibapi.h"
#include "bagel/hodjnpodj/hnplibs/stdafx.h"

namespace Bagel {
namespace HodjNPodj {

static BITMAPINFOHEADER getDIBInfoHeader(HDIB hDib) {
	BITMAPINFOHEADER h;
	h.biSize = 40;
	h.biWidth = hDib->w;
	h.biHeight = hDib->h;
	h.biPlanes = 1;
	h.biBitCount = 8;
	h.biCompression = BI_RGB;
	h.biSizeImage = 0;
	h.biXPelsPerMeter = 0;
	h.biYPelsPerMeter = 0;
	h.biClrUsed = !hDib->hasPalette() ? 0 :
		hDib->grabPalette()->size();
	h.biClrImportant = 0;

	return h;
}

static BITMAPINFO *getDIBInfo(HDIB hDib) {
	BITMAPINFO *h = (BITMAPINFO *)malloc(
		sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD));

	h->bmiHeader = getDIBInfoHeader(hDib);

	const Graphics::Palette *pal = hDib->grabPalette();
	for (uint i = 0; i < h->bmiHeader.biClrUsed; ++i) {
		auto &col = h->bmiColors[i];
		pal->get(i, col.rgbRed, col.rgbGreen, col.rgbBlue);
		col.rgbReserved = 0;
	}

	return h;
}

/*************************************************************************
 *
 * PaintDIB()
 *
 * Parameters:
 *
 * HDC hDC          - DC to do output to
 *
 * LPRECT lpDCRect  - rectangle on DC to do output to
 *
 * HDIB hDIB        - handle to global memory with a DIB spec
 *                    in it followed by the DIB bits
 *
 * LPRECT lpDIBRect - rectangle of DIB to output into lpDCRect
 *
 * CPalette* pPal   - pointer to CPalette containing DIB's palette
 *
 * Return Value:
 *
 * bool             - true if DIB was drawn, false otherwise
 *
 * Description:
 *   Painting routine for a DIB.  Calls StretchDIBits() or
 *   SetDIBitsToDevice() to paint the DIB.  The DIB is
 *   output to the specified DC, at the coordinates given
 *   in lpDCRect.  The area of the DIB to be output is
 *   given by lpDIBRect.
 *
 ************************************************************************/

bool PaintDIB(HDC hDC, LPRECT lpDCRect, HDIB hDIB,
		LPRECT lpDIBRect, CPalette *pPal) {
	bool bSuccess = false;      // Success/fail flag
	HPALETTE hPal = nullptr;		// Our DIB's palette
	HPALETTE hOldPal = nullptr;		// Previous palette
	HPALETTE hOldPal2 = nullptr;	// Previous palette
	HBITMAP hBitmap, hBitmapOld;
	HDC hdcMem;                     // memory device context
	int nDevCaps;

	// Check for valid DIB handle
	if (hDIB == nullptr)
		return false;

	// Get the palette, then select it into DC
	if (pPal != nullptr) {
		hPal = (HPALETTE)pPal->m_hObject;

		// Select as foreground and realize it
		hOldPal = SelectPalette(hDC, hPal, false);
		RealizePalette(hDC);
	}

	nDevCaps = GetDeviceCaps(hDC, RASTERCAPS);
	if (!(nDevCaps & RC_STRETCHDIB)) {
		hBitmap = DIBtoBitmap(hDC, nullptr, hDIB);
		if (hBitmap) {
			hdcMem = CreateCompatibleDC(hDC);
			if (hdcMem) {
				hOldPal2 = SelectPalette(hdcMem, hPal, false);
				RealizePalette(hdcMem);
				hBitmapOld = SelectBitmap(hdcMem, hBitmap);
				if ((RECTWIDTH(lpDCRect) == RECTWIDTH(lpDIBRect)) &&
					(RECTHEIGHT(lpDCRect) == RECTHEIGHT(lpDIBRect)))
					bSuccess = BitBlt(hDC, lpDCRect->left, lpDCRect->top,
						RECTWIDTH(lpDIBRect),
						RECTHEIGHT(lpDIBRect),
						hdcMem, lpDIBRect->left, lpDIBRect->top, SRCCOPY);
				else if (nDevCaps & RC_STRETCHBLT)
					bSuccess = StretchBlt(hDC, lpDCRect->left, lpDCRect->top, RECTWIDTH(lpDCRect), RECTHEIGHT(lpDCRect),
						hdcMem, lpDIBRect->left, lpDIBRect->top, RECTWIDTH(lpDIBRect), RECTHEIGHT(lpDIBRect),
						SRCCOPY);
				else
					bSuccess = false;
				SelectBitmap(hdcMem, hBitmapOld);
				SelectPalette(hdcMem, hOldPal2, false);
				DeleteDC(hdcMem);
			}
		}

		if (hBitmap != nullptr)
			DeleteBitmap(hBitmap);
		if (pPal != nullptr)
			SelectPalette(hDC, hOldPal, false);

		return bSuccess;
	}

	error("TODO: Populate binfo and enable below if this is ever needed");
#if 0
	BITMAPINFO bInfo;

	// Lock down the DIB, and get a pointer to it
	Graphics::ManagedSurface *surf = (Graphics::ManagedSurface *)hDIB;
	void *lpDIBBits = surf->getPixels();

	// Make sure to use the stretching mode best for color pictures
	SetStretchBltMode(hDC, COLORONCOLOR);

	bSuccess = StretchDIBits(hDC,                          // hDC
		lpDCRect->left,                 // DestX
		lpDCRect->top,                  // DestY
		RECTWIDTH(lpDCRect),            // nDestWidth
		RECTHEIGHT(lpDCRect),           // nDestHeight
		lpDIBRect->left,                // SrcX
		lpDIBRect->top,                 // SrcY
		RECTWIDTH(lpDIBRect),           // wSrcWidth
		RECTHEIGHT(lpDIBRect),          // wSrcHeight
		lpDIBBits,                      // lpBits
		&bInfo,							// lpBitsInfo
		DIB_RGB_COLORS,                 // wUsage
		SRCCOPY);                       // dwROP

	if (pPal != nullptr)
		SelectPalette(hDC, hOldPal, false);

	return bSuccess;
#endif
}

bool CreateDIBPalette(HDIB hDIB, CPalette *pPal) {
	uint16 wNumColors;
	bool bResult = false;

	// If handle to DIB is invalid, return false
	if (hDIB == nullptr)
		return false;

	// Get the number of colors in the DIB
	wNumColors = DIBNumColors(hDIB);

	if (wNumColors != 0) {
		const Graphics::Palette *pal = hDIB->grabPalette();
		bResult = pPal->SetPaletteEntries(*pal);
	}

	return bResult;
}

CPalette *DuplicatePalette(CPalette *pOrigPal) {
	CPalette *pPal;
	LPLOGPALETTE lpPal;      // pointer to a logical palette
	HANDLE hLogPal;          // handle to a logical palette
	uint16 wNumColors;         // number of colors in color table
	bool bResult;

	wNumColors = pOrigPal->GetPaletteEntries(0, 0, nullptr);
	if (wNumColors == 0)
		return nullptr;

	/* allocate memory block for logical palette */
	hLogPal = GlobalAlloc(GPTR, sizeof(LOGPALETTE)
	                      + sizeof(PALETTEENTRY)
	                      * wNumColors);
	assert(hLogPal);

	lpPal = (LPLOGPALETTE) GlobalLock((HGLOBAL)hLogPal);

	/* set version and number of palette entries */
	lpPal->palVersion = PALVERSION;
	lpPal->palNumEntries = (uint16)wNumColors;

	(*pOrigPal).GetPaletteEntries(0, wNumColors - 1, &lpPal->palPalEntry[0]);

	/* create the palette and get handle to it */

	pPal = new CPalette();
	bResult = pPal->CreatePalette(lpPal);
	if (!bResult) {
		delete pPal;
		pPal = nullptr;
	}

	GlobalUnlock((HGLOBAL)hLogPal);
	GlobalFree((HGLOBAL)hLogPal);

	return pPal;
}

char *FindDIBBits(HDIB hDIB) {
	return (char *)hDIB->getPixels();
}

uint32 DIBWidth(HDIB hDIB) {
	return hDIB->w;
}

uint32 DIBHeight(HDIB hDIB) {
	return hDIB->h;
}

uint16 PaletteSize(HDIB hDIB) {
	const Graphics::Palette *pal = hDIB->grabPalette();
	return pal->size() * 3;
}

uint16 DIBNumColors(HDIB lpDIB) {
	const Graphics::Palette *pal = lpDIB->grabPalette();
	return pal->size();
}

CBitmap *ConvertDIB(CDC *pDC, HDIB hDIB, CPalette *pPal) {
	HPALETTE hPal = nullptr;           // Our DIB's palette
	HPALETTE hOldPal = nullptr;        // Previous palette
	HDC hDC;
	HBITMAP hBitmap = nullptr;
	CBitmap *pBitmap = nullptr;

	hDC = (*pDC).m_hDC;

	// Get the palette, then select it into DC
	if (pPal != nullptr) {
		hPal = (HPALETTE)pPal->m_hObject;

		// Select as foreground and realize it
		hOldPal = SelectPalette(hDC, hPal, false);
		RealizePalette(hDC);
	}

	// Convert the bit buffer to a bitmap
	hBitmap = DIBtoBitmap(hDC, nullptr, hDIB);

	if (hBitmap != nullptr) {
		pBitmap = new CBitmap();
		if (pBitmap != nullptr)
			(*pBitmap).Attach(hBitmap);
	}

	/* Reselect old palette */
	if (pPal != nullptr)
		SelectPalette(hDC, hOldPal, false);

	return pBitmap;
}

HBITMAP DIBtoBitmap(HDC hDC, HPALETTE hPalette, HDIB hDib) {
	BITMAPINFO *info = getDIBInfo(hDib);
	void *lpbihBits = FindDIBBits(hDib);
	HBITMAP hBitmap = CreateDIBitmap(hDC,
		&info->bmiHeader,
	    CBM_INIT,
	    lpbihBits,
	    info,
	    DIB_RGB_COLORS);
	assert(hBitmap);

	free(info);
	return hBitmap;
}


//---------------------------------------------------------------------
//
// Function:   InitBitmapInfoHeader
//
// Purpose:    Does a "standard" initialization of a BITMAPINFOHEADER,
//             given the Width, Height, and Bits per Pixel for the
//             DIB.
//
//             By standard, I mean that all the relevant fields are set
//             to the specified values.  biSizeImage is computed, the
//             biCompression field is set to "no compression," and all
//             other fields are 0.
//
//             Note that DIBs only allow BitsPixel values of 1, 4, 8, or
//             24.  This routine makes sure that one of these values is
//             used (whichever is most appropriate for the specified
//             nBPP).
//
// Parms:      lpBmInfoHdr == Far pointer to a BITMAPINFOHEADER structure
//                            to be filled in.
//             dwWidth     == Width of DIB (not in Win 3.0 & 3.1, high
//                            word MUST be 0).
//             dwHeight    == Height of DIB (not in Win 3.0 & 3.1, high
//                            word MUST be 0).
//             nBPP        == Bits per Pixel for the DIB.
//
// History:   Date      Reason
//            11/07/91  Created
//
//---------------------------------------------------------------------

void WINAPI InitBitmapInfoHeader(LPBITMAPINFOHEADER lpBmInfoHdr,
                                 uint32 dwWidth,
                                 uint32 dwHeight,
                                 int nBPP) {
	memset(lpBmInfoHdr, 0, sizeof(BITMAPINFOHEADER));

	lpBmInfoHdr->biSize = sizeof(BITMAPINFOHEADER);
	lpBmInfoHdr->biWidth = dwWidth;
	lpBmInfoHdr->biHeight = dwHeight;
	lpBmInfoHdr->biPlanes = 1;

	if (nBPP <= 1)
		nBPP = 1;
	else if (nBPP <= 4)
		nBPP = 4;
	else if (nBPP <= 8)
		nBPP = 8;
	else
		nBPP = 24;

	lpBmInfoHdr->biBitCount = nBPP;
	lpBmInfoHdr->biSizeImage = WIDTHBYTES(dwWidth * nBPP) * dwHeight;
}


//////////////////////////////////////////////////////////////////////////
//// Clipboard support

//---------------------------------------------------------------------
//
// Function:   CopyHandle (from SDK DibView sample clipbrd.c)
//
// Purpose:    Makes a copy of the given global memory block.  Returns
//             a handle to the new memory block (nullptr on error).
//
//             Routine stolen verbatim out of ShowDIB.
//
// Parms:      h == Handle to global memory to duplicate.
//
// Returns:    Handle to new global memory block.
//
//---------------------------------------------------------------------

HANDLE WINAPI CopyHandle(HANDLE h) {
	byte *lpCopy;
	byte *lp;
	HANDLE hCopy;
	uint32 dwLen;

	if (h == nullptr)
		return nullptr;

	dwLen = GlobalSize((HGLOBAL)h);

	if ((hCopy = (HANDLE) GlobalAlloc(GHND, dwLen)) != nullptr) {
		lpCopy = (byte *) GlobalLock((HGLOBAL)hCopy);
		lp = (byte *) GlobalLock((HGLOBAL)h);

		while (dwLen--)
			*lpCopy++ = *lp++;

		GlobalUnlock((HGLOBAL)hCopy);
		GlobalUnlock((HGLOBAL)h);
	}

	return hCopy;
}


void WINAPI ShowMemoryInfo(const char *chMessage, const char *chTitle) {
	// No implementation
}

} // namespace HodjNPodj
} // namespace Bagel
