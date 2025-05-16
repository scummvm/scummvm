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

#include "bagel/hodjnpodj/hnplibs/dibapi.h"
#include "bagel/hodjnpodj/hnplibs/stdafx.h"

namespace Bagel {
namespace HodjNPodj {

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
 * BOOL             - TRUE if DIB was drawn, FALSE otherwise
 *
 * Description:
 *   Painting routine for a DIB.  Calls StretchDIBits() or
 *   SetDIBitsToDevice() to paint the DIB.  The DIB is
 *   output to the specified DC, at the coordinates given
 *   in lpDCRect.  The area of the DIB to be output is
 *   given by lpDIBRect.
 *
 ************************************************************************/

BOOL WINAPI PaintDIB(HDC     hDC,
                     LPRECT  lpDCRect,
                     HDIB    hDIB,
                     LPRECT  lpDIBRect,
                     CPalette *pPal) {
	LPSTR    lpDIBHdr;            // Pointer to BITMAPINFOHEADER
	LPSTR    lpDIBBits;           // Pointer to DIB bits
	BOOL     bSuccess = FALSE;      // Success/fail flag
	HPALETTE hPal = NULL;           // Our DIB's palette
	HPALETTE hOldPal = NULL;        // Previous palette
	HPALETTE hOldPal2 = NULL;        // Previous palette
	HBITMAP  hBitmap, hBitmapOld;
	HDC      hdcMem;                     /* memory device context */
	int      nDevCaps;

	/* Check for valid DIB handle */
	if (hDIB == NULL)
		return FALSE;

	/* Lock down the DIB, and get a pointer to the beginning of the bit
	 *  buffer
	 */
	lpDIBHdr = (LPSTR)GlobalLock((HGLOBAL)hDIB);
	lpDIBBits = FindDIBBits(lpDIBHdr);

	// Get the palette, then select it into DC
	if (pPal != NULL) {
		hPal = (HPALETTE)pPal->m_hObject;

		// Select as foreground and realize it
		hOldPal = SelectPalette(hDC, hPal, FALSE);
		(void) RealizePalette(hDC);
	}

	nDevCaps = GetDeviceCaps(hDC, RASTERCAPS);
	if (!(nDevCaps & RC_STRETCHDIB)) {
		hBitmap = DIBtoBitmap(hDC, NULL, (LPBITMAPINFO)lpDIBHdr);
		if (hBitmap) {
			hdcMem = CreateCompatibleDC(hDC);
			if (hdcMem) {
				hOldPal2 = SelectPalette(hdcMem, hPal, FALSE);
				(void) RealizePalette(hdcMem);
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
					bSuccess = FALSE;
				(void)SelectBitmap(hdcMem, hBitmapOld);
				(void) SelectPalette(hdcMem, hOldPal2, FALSE);
				DeleteDC(hdcMem);
			}
		}
		if (hBitmap != NULL)
			DeleteBitmap(hBitmap);
		if (pPal != NULL)
			SelectPalette(hDC, hOldPal, FALSE);
		GlobalUnlock((HGLOBAL)hDIB);
		return (bSuccess);
	}

	/* Make sure to use the stretching mode best for color pictures */
	SetStretchBltMode(hDC, COLORONCOLOR);

	#ifdef HIDE // looks like some graphics cards do not support this to offscreen bitmaps
	/* Determine whether to call StretchDIBits() or SetDIBitsToDevice() */
	if ((RECTWIDTH(lpDCRect) == RECTWIDTH(lpDIBRect)) &&
	        (RECTHEIGHT(lpDCRect) == RECTHEIGHT(lpDIBRect)))
		bSuccess = ::SetDIBitsToDevice(hDC,                    // hDC
		                               lpDCRect->left,             // DestX
		                               lpDCRect->top,              // DestY
		                               RECTWIDTH(lpDCRect),        // nDestWidth
		                               RECTHEIGHT(lpDCRect),       // nDestHeight
		                               lpDIBRect->left,            // SrcX
		                               (int)DIBHeight(lpDIBHdr) -
		                               lpDIBRect->top -
		                               RECTHEIGHT(lpDIBRect),   // SrcY
		                               0,                          // nStartScan
		                               (WORD)DIBHeight(lpDIBHdr),  // nNumScans
		                               lpDIBBits,                  // lpBits
		                               (LPBITMAPINFO)lpDIBHdr,     // lpBitsInfo
		                               DIB_RGB_COLORS);            // wUsage
	else
	#endif
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
		                         (LPBITMAPINFO)lpDIBHdr,         // lpBitsInfo
		                         DIB_RGB_COLORS,                 // wUsage
		                         SRCCOPY);                       // dwROP

	if (pPal != NULL)
		SelectPalette(hDC, hOldPal, FALSE);

	GlobalUnlock((HGLOBAL)hDIB);

	return bSuccess;
}

/*************************************************************************
 *
 * CreateDIBPalette()
 *
 * Parameter:
 *
 * HDIB hDIB        - specifies the DIB
 *
 * Return Value:
 *
 * HPALETTE         - specifies the palette
 *
 * Description:
 *
 * This function creates a palette from a DIB by allocating memory for the
 * logical palette, reading and storing the colors from the DIB's color table
 * into the logical palette, creating a palette from this logical palette,
 * and then returning the palette's handle. This allows the DIB to be
 * displayed using the best possible colors (important for DIBs with 256 or
 * more colors).
 *
 ************************************************************************/


BOOL WINAPI CreateDIBPalette(HDIB hDIB, CPalette *pPal) {
	LPLOGPALETTE lpPal;      // pointer to a logical palette
	HANDLE hLogPal;          // handle to a logical palette
	//HPALETTE hPal = NULL;    // handle to a palette
	int i;                   // loop index
	WORD wNumColors;         // number of colors in color table
	LPSTR lpbi;              // pointer to packed-DIB
	LPBITMAPINFO lpbmi;      // pointer to BITMAPINFO structure (Win3.0)
	LPBITMAPCOREINFO lpbmc;  // pointer to BITMAPCOREINFO structure (old)
	BOOL bWinStyleDIB;       // flag which signifies whether this is a Win3.0 DIB
	BOOL bRetry = FALSE;
	BOOL bResult = FALSE;

	/* if handle to DIB is invalid, return FALSE */

	if (hDIB == NULL)
		return FALSE;

	lpbi = (LPSTR) GlobalLock((HGLOBAL)hDIB);

	/* get pointer to BITMAPINFO (Win 3.0) */
	lpbmi = (LPBITMAPINFO)lpbi;

	/* get pointer to BITMAPCOREINFO (old 1.x) */
	lpbmc = (LPBITMAPCOREINFO)lpbi;

	/* get the number of colors in the DIB */
	wNumColors = DIBNumColors(lpbi);

	if (wNumColors != 0) {
try_again:
		/* allocate memory block for logical palette */
		hLogPal = GlobalAlloc(GPTR, sizeof(LOGPALETTE)
		                      + sizeof(PALETTEENTRY)
		                      * wNumColors);

		/* if not enough memory, clean up and return NULL */
		if (hLogPal == 0) {
			if (!bRetry) {
				bRetry = TRUE;
				(void)GlobalCompact(1000000L);
				goto try_again;
			}
			GlobalUnlock((HGLOBAL)hDIB);
			return FALSE;
		}

		lpPal = (LPLOGPALETTE) GlobalLock((HGLOBAL)hLogPal);

		/* set version and number of palette entries */
		lpPal->palVersion = PALVERSION;
		lpPal->palNumEntries = (WORD)wNumColors;

		/* is this a Win 3.0 DIB? */
		bWinStyleDIB = IS_WIN30_DIB(lpbi);
		for (i = 0; i < (int)wNumColors; i++) {
			if (bWinStyleDIB) {
				lpPal->palPalEntry[i].peRed = lpbmi->bmiColors[i].rgbRed;
				lpPal->palPalEntry[i].peGreen = lpbmi->bmiColors[i].rgbGreen;
				lpPal->palPalEntry[i].peBlue = lpbmi->bmiColors[i].rgbBlue;
				lpPal->palPalEntry[i].peFlags = 0;
			} else {
				lpPal->palPalEntry[i].peRed = lpbmc->bmciColors[i].rgbtRed;
				lpPal->palPalEntry[i].peGreen = lpbmc->bmciColors[i].rgbtGreen;
				lpPal->palPalEntry[i].peBlue = lpbmc->bmciColors[i].rgbtBlue;
				lpPal->palPalEntry[i].peFlags = 0;
			}
		}

		/* create the palette and get handle to it */
		bResult = pPal->CreatePalette(lpPal);

		GlobalUnlock((HGLOBAL)hLogPal);
		GlobalFree((HGLOBAL)hLogPal);
	}

	GlobalUnlock((HGLOBAL)hDIB);

	return bResult;
}




CPalette *DuplicatePalette(CPalette *pOrigPal) {
	CPalette *pPal;
	LPLOGPALETTE lpPal;      // pointer to a logical palette
	HANDLE hLogPal;          // handle to a logical palette
	WORD wNumColors;         // number of colors in color table
	BOOL bRetry = FALSE;
	BOOL bResult;
	int  nResult;

try_again:

	nResult = (*pOrigPal).GetObject(sizeof(WORD), &wNumColors);
	if (nResult == 0)
		return (NULL);

	/* allocate memory block for logical palette */
	hLogPal = GlobalAlloc(GPTR, sizeof(LOGPALETTE)
	                      + sizeof(PALETTEENTRY)
	                      * wNumColors);

	/* if not enough memory, clean up and return NULL */
	if (hLogPal == 0) {
		if (!bRetry) {
			bRetry = TRUE;
			(void)GlobalCompact(1000000L);
			goto try_again;
		}
		return (NULL);
	}

	lpPal = (LPLOGPALETTE) GlobalLock((HGLOBAL)hLogPal);

	/* set version and number of palette entries */
	lpPal->palVersion = PALVERSION;
	lpPal->palNumEntries = (WORD)wNumColors;

	(*pOrigPal).GetPaletteEntries(0, wNumColors - 1, &lpPal->palPalEntry[0]);

	/* create the palette and get handle to it */

	pPal = new CPalette();
	bResult = pPal->CreatePalette(lpPal);
	if (!bResult) {
		delete pPal;
		pPal = NULL;
	}

	GlobalUnlock((HGLOBAL)hLogPal);
	GlobalFree((HGLOBAL)hLogPal);

	return (pPal);
}


/*************************************************************************
 *
 * FindDIBBits()
 *
 * Parameter:
 *
 * LPSTR lpbi       - pointer to packed-DIB memory block
 *
 * Return Value:
 *
 * LPSTR            - pointer to the DIB bits
 *
 * Description:
 *
 * This function calculates the address of the DIB's bits and returns a
 * pointer to the DIB bits.
 *
 ************************************************************************/


LPSTR WINAPI FindDIBBits(LPSTR lpbi) {
	return (lpbi + * (LPDWORD)lpbi + PaletteSize(lpbi));
}


/*************************************************************************
 *
 * DIBWidth()
 *
 * Parameter:
 *
 * LPSTR lpbi       - pointer to packed-DIB memory block
 *
 * Return Value:
 *
 * DWORD            - width of the DIB
 *
 * Description:
 *
 * This function gets the width of the DIB from the BITMAPINFOHEADER
 * width field if it is a Windows 3.0-style DIB or from the BITMAPCOREHEADER
 * width field if it is an other-style DIB.
 *
 ************************************************************************/


DWORD WINAPI DIBWidth(LPSTR lpDIB) {
	LPBITMAPINFOHEADER lpbmi;  // pointer to a Win 3.0-style DIB
	LPBITMAPCOREHEADER lpbmc;  // pointer to an other-style DIB

	/* point to the header (whether Win 3.0 and old) */

	lpbmi = (LPBITMAPINFOHEADER)lpDIB;
	lpbmc = (LPBITMAPCOREHEADER)lpDIB;

	/* return the DIB width if it is a Win 3.0 DIB */
	if (IS_WIN30_DIB(lpDIB))
		return lpbmi->biWidth;
	else  /* it is an other-style DIB, so return its width */
		return (DWORD)lpbmc->bcWidth;
}


/*************************************************************************
 *
 * DIBHeight()
 *
 * Parameter:
 *
 * LPSTR lpbi       - pointer to packed-DIB memory block
 *
 * Return Value:
 *
 * DWORD            - height of the DIB
 *
 * Description:
 *
 * This function gets the height of the DIB from the BITMAPINFOHEADER
 * height field if it is a Windows 3.0-style DIB or from the BITMAPCOREHEADER
 * height field if it is an other-style DIB.
 *
 ************************************************************************/


DWORD WINAPI DIBHeight(LPSTR lpDIB) {
	LPBITMAPINFOHEADER lpbmi;  // pointer to a Win 3.0-style DIB
	LPBITMAPCOREHEADER lpbmc;  // pointer to an other-style DIB

	/* point to the header (whether old or Win 3.0 */

	lpbmi = (LPBITMAPINFOHEADER)lpDIB;
	lpbmc = (LPBITMAPCOREHEADER)lpDIB;

	/* return the DIB height if it is a Win 3.0 DIB */
	if (IS_WIN30_DIB(lpDIB))
		return lpbmi->biHeight;
	else  /* it is an other-style DIB, so return its height */
		return (DWORD)lpbmc->bcHeight;
}


/*************************************************************************
 *
 * PaletteSize()
 *
 * Parameter:
 *
 * LPSTR lpbi       - pointer to packed-DIB memory block
 *
 * Return Value:
 *
 * WORD             - size of the color palette of the DIB
 *
 * Description:
 *
 * This function gets the size required to store the DIB's palette by
 * multiplying the number of colors by the size of an RGBQUAD (for a
 * Windows 3.0-style DIB) or by the size of an RGBTRIPLE (for an other-
 * style DIB).
 *
 ************************************************************************/


WORD WINAPI PaletteSize(LPSTR lpbi) {
	/* calculate the size required by the palette */
	if (IS_WIN30_DIB(lpbi))
		return (WORD)(DIBNumColors(lpbi) * sizeof(RGBQUAD));
	else
		return (WORD)(DIBNumColors(lpbi) * sizeof(RGBTRIPLE));
}


/*************************************************************************
 *
 * DIBNumColors()
 *
 * Parameter:
 *
 * LPSTR lpbi       - pointer to packed-DIB memory block
 *
 * Return Value:
 *
 * WORD             - number of colors in the color table
 *
 * Description:
 *
 * This function calculates the number of colors in the DIB's color table
 * by finding the bits per pixel for the DIB (whether Win3.0 or other-style
 * DIB). If bits per pixel is 1: colors=2, if 4: colors=16, if 8: colors=256,
 * if 24, no colors in color table.
 *
 ************************************************************************/


WORD WINAPI DIBNumColors(LPSTR lpbi) {
	WORD wBitCount;  // DIB bit count

	/*  If this is a Windows-style DIB, the number of colors in the
	 *  color table can be less than the number of bits per pixel
	 *  allows for (i.e. lpbi->biClrUsed can be set to some value).
	 *  If this is the case, return the appropriate value.
	 */

	if (IS_WIN30_DIB(lpbi)) {
		DWORD dwClrUsed;

		dwClrUsed = ((LPBITMAPINFOHEADER)lpbi)->biClrUsed;
		if (dwClrUsed != 0)
			return (WORD)dwClrUsed;
	}

	/*  Calculate the number of colors in the color table based on
	 *  the number of bits per pixel for the DIB.
	 */
	if (IS_WIN30_DIB(lpbi))
		wBitCount = ((LPBITMAPINFOHEADER)lpbi)->biBitCount;
	else
		wBitCount = ((LPBITMAPCOREHEADER)lpbi)->bcBitCount;

	/* return number of colors based on bits per pixel */
	switch (wBitCount) {
	case 1:
		return 2;

	case 4:
		return 16;

	case 8:
		return 256;

	default:
		return 0;
	}
}


/*************************************************************************
        DIB TO Bitmap
        Convert a device-independent bitmap (DIB) to a device-dependent
        bitmap (DDB), with the desired color palette mapped to the context.
        The DIB must be packed; i.e. same as a .BMP file.

        RETURN
        A handle to the Bitmap (CBitmap *).  If an error occurs, the return
        value will be NULL.
**************************************************************************/

CBitmap *WINAPI ConvertDIB(CDC *pDC,
                           HDIB hDIB,
                           CPalette *pPal) {
	LPSTR    lpDIBHdr;            // Pointer to BITMAPINFOHEADER
	//LPSTR    lpDIBBits;           // Pointer to DIB bits
	//BOOL     bSuccess = FALSE;      // Success/fail flag
	HPALETTE hPal = NULL;           // Our DIB's palette
	HPALETTE hOldPal = NULL;        // Previous palette
	HDC      hDC;
	HBITMAP  hBitmap = NULL;
	CBitmap *pBitmap = NULL;

	hDC = (*pDC).m_hDC;

	// Get the palette, then select it into DC
	if (pPal != NULL) {
		hPal = (HPALETTE)pPal->m_hObject;

		// Select as foreground and realize it
		hOldPal = SelectPalette(hDC, hPal, FALSE);
		(void) RealizePalette(hDC);
	}

	/* Lock down the DIB, and get a pointer to the beginning of the bit
	 *  buffer
	 */
	lpDIBHdr = (LPSTR) GlobalLock((HGLOBAL)hDIB);
	/*lpDIBBits =*/(void)FindDIBBits(lpDIBHdr);

	hBitmap = DIBtoBitmap(hDC, NULL, (LPBITMAPINFO)lpDIBHdr);

	GlobalUnlock((HGLOBAL)hDIB);

	if (hBitmap != NULL) {
		pBitmap = new CBitmap();
		if (pBitmap != NULL)
			(*pBitmap).Attach(hBitmap);
	}

	/* Reselect old palette */
	if (pPal != NULL)
		SelectPalette(hDC, hOldPal, FALSE);

	return (pBitmap);
}


/*************************************************************************
        DIB TO DDB
        Convert a device-independent bitmap (DIB) to a device-dependent
        bitmap (DDB).  The DIB must be packed; i.e. same as a .BMP file.

        RETURN
        A handle to the DDB (HBITMAP).  If an error occurs, the return
        value will be NULL.
**************************************************************************/

HBITMAP WINAPI DIBtoBitmap(HDC hDC,                     // where DDB will be displayed
                           HPALETTE hPalette,
                           LPBITMAPINFO lpbih) {       // pointer to a packed DIB
	BOOL     bRetry = FALSE;
	HBITMAP  hBitmap;                                   // Handle to our DDB bitmap
	LPSTR    lpbihBits;                                 // Pointer to DIB bits

	lpbihBits = FindDIBBits((LPSTR)lpbih);

try_again:
	hBitmap = CreateDIBitmap(hDC,
	                         (LPBITMAPINFOHEADER)lpbih,
	                         CBM_INIT,
	                         lpbihBits,
	                         (LPBITMAPINFO)lpbih,
	                         DIB_RGB_COLORS);

	if (hBitmap == NULL) {
		if (!bRetry) {
			bRetry = TRUE;
			(void)GlobalCompact(1000000L);
			goto try_again;
		}
		ShowMemoryInfo("Unable to convert artwork", "Internal Problem");
	}

	return (hBitmap);
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
                                 DWORD dwWidth,
                                 DWORD dwHeight,
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


//---------------------------------------------------------------------
//
// Function:   BitmapToDIB
//
// Purpose:    Given a device dependent bitmap and a palette, returns
//             a handle to global memory with a DIB spec in it.  The
//             DIB is rendered using the colors of the palette passed in.
//
//             Stolen almost verbatim from ShowDIB.
//
// Parms:      hBitmap == Handle to device dependent bitmap compatible
//                        with default screen display device.
//             hPal    == Palette to render the DDB with.  If it's NULL,
//                        use the default palette.
//
// History:   Date      Reason
//             6/01/91  Created
//
//---------------------------------------------------------------------

HANDLE WINAPI BitmapToDIB(HBITMAP hBitmap, HPALETTE hPal) {
	BITMAP             Bitmap;
	BITMAPINFOHEADER   bmInfoHdr;
	LPBITMAPINFOHEADER lpbmInfoHdr;
	LPSTR              lpBits;
	HDC                hMemDC;
	HANDLE             hDIB;
	HPALETTE           hOldPal = NULL;

	// Do some setup -- make sure the Bitmap passed in is valid,
	//  get info on the bitmap (like its height, width, etc.),
	//  then setup a BITMAPINFOHEADER.

	if (!hBitmap)
		return NULL;

	if (!GetObject(hBitmap, sizeof(Bitmap), (LPSTR)&Bitmap))
		return NULL;

	InitBitmapInfoHeader(&bmInfoHdr,
	                     Bitmap.bmWidth,
	                     Bitmap.bmHeight,
	                     Bitmap.bmPlanes * Bitmap.bmBitsPixel);


	// Now allocate memory for the DIB.  Then, set the BITMAPINFOHEADER
	//  into this memory, and find out where the bitmap bits go.

	hDIB = GlobalAlloc(GHND, sizeof(BITMAPINFOHEADER) +
	                   PaletteSize((LPSTR)&bmInfoHdr) + bmInfoHdr.biSizeImage);

	if (!hDIB)
		return NULL;

	lpbmInfoHdr = (LPBITMAPINFOHEADER) GlobalLock(hDIB);
	*lpbmInfoHdr = bmInfoHdr;
	lpBits = FindDIBBits((LPSTR)lpbmInfoHdr);


	// Now, we need a DC to hold our bitmap.  If the app passed us
	//  a palette, it should be selected into the DC.

	hMemDC = MFC::GetDC(NULL);

	if (hPal) {
		hOldPal = SelectPalette(hMemDC, hPal, FALSE);
		RealizePalette(hMemDC);
	}



	// We're finally ready to get the DIB.  Call the driver and let
	//  it party on our bitmap.  It will fill in the color table,
	//  and bitmap bits of our global memory block.

	if (!GetDIBits(hMemDC,
	               hBitmap,
	               0,
	               Bitmap.bmHeight,
	               lpBits,
	               (LPBITMAPINFO)lpbmInfoHdr,
	               DIB_RGB_COLORS)) {
		GlobalUnlock(hDIB);
		GlobalFree(hDIB);
		hDIB = NULL;
	} else
		GlobalUnlock(hDIB);


	// Finally, clean up and return.

	if (hOldPal)
		SelectPalette(hMemDC, hOldPal, FALSE);

	MFC::ReleaseDC(NULL, hMemDC);

	return hDIB;
}


//////////////////////////////////////////////////////////////////////////
//// Clipboard support

//---------------------------------------------------------------------
//
// Function:   CopyHandle (from SDK DibView sample clipbrd.c)
//
// Purpose:    Makes a copy of the given global memory block.  Returns
//             a handle to the new memory block (NULL on error).
//
//             Routine stolen verbatim out of ShowDIB.
//
// Parms:      h == Handle to global memory to duplicate.
//
// Returns:    Handle to new global memory block.
//
//---------------------------------------------------------------------

HANDLE WINAPI CopyHandle(HANDLE h) {
	BYTE *lpCopy;
	BYTE *lp;
	HANDLE hCopy;
	DWORD dwLen;

	if (h == NULL)
		return NULL;

	dwLen = GlobalSize((HGLOBAL)h);

	if ((hCopy = (HANDLE) GlobalAlloc(GHND, dwLen)) != NULL) {
		lpCopy = (BYTE *) GlobalLock((HGLOBAL)hCopy);
		lp = (BYTE *) GlobalLock((HGLOBAL)h);

		while (dwLen--)
			*lpCopy++ = *lp++;

		GlobalUnlock((HGLOBAL)hCopy);
		GlobalUnlock((HGLOBAL)h);
	}

	return hCopy;
}


void WINAPI ShowMemoryInfo(const char *chMessage, const char *chTitle) {
	#ifdef _DEBUG
	char    buf[256];

	MessageBox(NULL, chMessage, chTitle, MB_ICONEXCLAMATION);
	Common::sprintf_s(buf, "Free Memory = %ld\nLargest Memory Block = %ld",
	                  GetFreeSpace(0), GlobalCompact(0));
	MessageBox(NULL, buf, "Internal Status", MB_ICONINFORMATION);
	//  Common::sprintf_s(buf,"Largest Memory Block = %ld",GlobalCompact(0));
	//  MessageBox(NULL,buf,"Internal Problem",MB_ICONINFORMATION);
	#endif
}

} // namespace HodjNPodj
} // namespace Bagel
