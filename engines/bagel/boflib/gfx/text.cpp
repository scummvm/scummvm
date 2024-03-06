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

#include "bagel/boflib/boffo.h"

#include "bagel/boflib/app.h"
#include "bagel/boflib/gfx/text.h"

namespace Bagel {

#define START_SIZE 8

INT CBofText::m_nTabStop;
BOOL CBofText::m_bInitialized;

#if BOF_WINDOWS
HFONT CBofText::m_hDefaultFont[NUM_POINT_SIZES];
HFONT CBofText::m_hFixedFont[NUM_POINT_SIZES];
INT  CBofText::m_nMonoType = 0;
#endif

void CBofText::initStatics() {
	m_nTabStop = 20;		// tabstops every 20 pixels
	m_bInitialized = FALSE;
}

CBofText::CBofText() {
	InitializeFields();		// initialize stuff
}

CBofText::CBofText(CBofRect *pRect, INT nJustify, UINT nFormatFlags) {
	// Can't access nullptr pointers
	Assert(pRect != nullptr);

	// Initialize stuff
	InitializeFields();

	// Build the work areas
	SetupText(pRect, nJustify, nFormatFlags);
}

CBofText::~CBofText() {
	if (m_pWork != nullptr) {
		delete m_pWork;
		m_pWork = nullptr;
	}
	if (m_pBackground != nullptr) {
		delete m_pBackground;
		m_pBackground = nullptr;
	}
}

VOID CBofText::InitializeFields() {
	m_pBackground = nullptr;
	m_pWork = nullptr;
	m_bSaved = FALSE;

	m_cPosition = CBofPoint(0, 0);
	m_cSize = CBofSize(0, 0);
	m_cRect.SetRect(0, 0, 0, 0);

	m_cShadowColor = RGB(0, 0, 0);
	m_nShadow_DX = 0;
	m_nShadow_DY = 0;

	m_nJustify = JUSTIFY_LEFT;

	m_nFormatFlags = FORMAT_DEFAULT;
	m_bMultiLine = FALSE;

	m_nCurSize = 10;
	m_nCurWeight = TEXT_DONTCARE;
	m_cTextColor = CTEXT_COLOR;
}

ERROR_CODE CBofText::SetupText(CBofRect *pRect, INT nJustify, UINT nFormatFlags) {
	// Can't access nullptr pointers
	Assert(pRect != nullptr);

	m_nJustify = nJustify;

#if BOF_WINDOWS && !BOF_WINMAC

	m_nFormatFlags = nFormatFlags | DT_WORDBREAK | DT_EXPANDTABS;

#endif

	// setup the fields for location and size of the text area
	m_cRect = *pRect;
	m_cSize.cx = m_cRect.Width();
	m_cSize.cy = m_cRect.Height();

	if (m_pWork != nullptr) {
		delete m_pWork;
		m_pWork = nullptr;
	}
	if (m_pBackground != nullptr) {
		delete m_pBackground;
		m_pBackground = nullptr;
	}
	CBofPalette *pPalette;
	pPalette = CBofApp::GetApp()->GetPalette();

	// create a bitmap to serve as our work area as we output text
	//
	if ((m_pWork = new CBofBitmap(m_cSize.cx, m_cSize.cy, pPalette)) != nullptr) {

		// create a bitmap to hold the background we overwrite
		//
		if ((m_pBackground = new CBofBitmap(m_cSize.cx, m_cSize.cy, pPalette)) != nullptr) {

		} else {
			ReportError(ERR_MEMORY, "Could not allocate a (%d x %d) CBofBitmap", m_cSize.cx, m_cSize.cy);
		}
	} else {
		ReportError(ERR_MEMORY, "Could not allocate a (%d x %d) CBofBitmap", m_cSize.cx, m_cSize.cy);
	}

	return m_errCode;
}

ERROR_CODE CBofText::SetupTextOpt(CBofRect *pRect, INT nJustify, UINT nFormatFlags) {
	// can't access nullptr pointers
	Assert(pRect != nullptr);

	m_nJustify = nJustify;

#if BOF_WINDOWS && !BOF_WINMAC

	m_nFormatFlags = nFormatFlags | DT_WORDBREAK | DT_EXPANDTABS;

#endif

	// setup the fields for location and size of the text area
	m_cRect = *pRect;
	m_cSize.cx = m_cRect.Width();
	m_cSize.cy = m_cRect.Height();

	return m_errCode;
}

ERROR_CODE CBofText::Erase(CBofWindow *pWnd) {
	// Can't access null pointers
	Assert(pWnd != nullptr);

	if (m_pBackground != nullptr && m_bSaved) {
		// Simply splat the background art back where it came from
		m_errCode = m_pBackground->Paint(pWnd, &m_cRect);
	}

	return m_errCode;
}

ERROR_CODE CBofText::Erase(CBofBitmap *pBmp) {
	// can't access nullptr pointers
	Assert(pBmp != nullptr);

	if (m_pBackground != nullptr && m_bSaved) {
		// Simply splat the background art back where it came from
		m_errCode = m_pBackground->Paint(pBmp, &m_cRect);
	}

	return m_errCode;
}

ERROR_CODE CBofText::Display(CBofWindow *pWnd, const CHAR *pszText, const INT nSize, const INT nWeight, const RGBCOLOR cColor, INT nFont) {
	Assert(IsValidObject(this));

	// can't access nullptr pointers
	Assert(pWnd != nullptr);

	m_cTextColor = cColor;

	return DisplayText(pWnd, pszText, &m_cRect, nSize, nWeight, FALSE, nFont);
}

ERROR_CODE CBofText::Display(CBofWindow *pWnd) {
	Assert(IsValidObject(this));
	Assert(pWnd != nullptr);

	return (Display(pWnd, m_cCurString, m_nCurSize, m_nCurWeight, m_cTextColor));
}

ERROR_CODE CBofText::Display(CBofBitmap *pBmp) {
	Assert(IsValidObject(this));
	Assert(pBmp != nullptr);

	return Display(pBmp, m_cCurString, m_nCurSize, m_nCurWeight, m_cTextColor);
}

ERROR_CODE CBofText::Display(CBofBitmap *pBmp, const CHAR *pszText, const INT nSize, const INT nWeight, const RGBCOLOR cColor, INT nFont) {
	// can't access nullptr pointers
	Assert(pBmp != nullptr);

	m_cTextColor = cColor;

	return (DisplayText(pBmp, pszText, &m_cRect, nSize, nWeight, FALSE, nFont));
}

ERROR_CODE CBofText::DisplayShadowed(CBofWindow *pWnd, const CHAR *pszText, const INT nSize, const INT nWeight, const RGBCOLOR cColor, const RGBCOLOR cShadow, const INT nDX, const INT nDY, INT nFont) {
	// can't access nullptr pointers
	Assert(pWnd != nullptr);

	m_cTextColor = cColor;
	m_cShadowColor = cShadow;
	m_nShadow_DX = nDX;
	m_nShadow_DY = nDY;

	return (DisplayText(pWnd, pszText, &m_cRect, nSize, nWeight, TRUE, nFont));
}

ERROR_CODE CBofText::DisplayShadowed(CBofBitmap *pBmp, const CHAR *pszText, const INT nSize, const INT nWeight, const RGBCOLOR cColor, const RGBCOLOR cShadow, const INT nDX, const INT nDY, INT nFont) {
	// Can't access nullptr pointers
	Assert(pBmp != nullptr);

	m_cTextColor = cColor;
	m_cShadowColor = cShadow;
	m_nShadow_DX = nDX;
	m_nShadow_DY = nDY;

	return DisplayText(pBmp, pszText, &m_cRect, nSize, nWeight, TRUE, nFont);
}

ERROR_CODE CBofText::DisplayText(CBofWindow *pWnd, const CHAR *pszText, CBofRect *pRect, const INT nSize, const INT nWeight, const BOOL bShadowed, INT nFont) {
	Assert(IsValidObject(this));
	Assert(pWnd != nullptr);
	Assert(pszText != nullptr);
	Assert(pRect != nullptr);

	CBofRect cRect(0, 0, pRect->Width() - 1, pRect->Height() - 1);

	Assert(m_pBackground != nullptr);
	Assert(m_pWork != nullptr);

	if (!m_bSaved) {
		CBofBitmap::SetUseBackdrop(TRUE);
		m_pBackground->CaptureScreen(pWnd, pRect);
		CBofBitmap::SetUseBackdrop(FALSE);
		m_bSaved = TRUE;
	}

	m_pBackground->Paint(m_pWork, 0, 0);

	DisplayTextEx(m_pWork, pszText, &cRect, nSize, nWeight, bShadowed, nFont);

	m_pWork->Paint(pWnd, pRect);

	return m_errCode;
}

ERROR_CODE CBofText::DisplayText(CBofBitmap *pBmp, const CHAR *pszText, CBofRect *pRect, const INT nSize, const INT nWeight, const BOOL bShadowed, INT nFont) {
	Assert(IsValidObject(this));
	Assert(pBmp != nullptr);
	Assert(pszText != nullptr);
	Assert(pRect != nullptr);

	CBofRect cRect(0, 0, pRect->Width() - 1, pRect->Height() - 1);

	Assert(m_pWork != nullptr);

	Assert(m_pBackground != nullptr);

	if (!m_bSaved) {
		CBofRect r = m_pBackground->GetRect();
		pBmp->Paint(m_pBackground, &r, pRect);
		m_bSaved = TRUE;
	}

	m_pBackground->Paint(m_pWork, 0, 0);

	DisplayTextEx(m_pWork, pszText, &cRect, nSize, nWeight, bShadowed, nFont);

	m_pWork->Paint(pBmp, pRect);

	return m_errCode;
}

ERROR_CODE CBofText::DisplayTextEx(CBofBitmap *pBmp, const CHAR *pszText, CBofRect *pRect, const INT nSize, const INT nWeight, const BOOL bShadowed, INT nFont) {
	Assert(IsValidObject(this));

	// can't access nullptr pointers
	Assert(pBmp != nullptr);
	Assert(pszText != nullptr);
	Assert(pRect != nullptr);

#if BOF_WINDOWS && !BOF_WINMAC

	HFONT       hFont, hFontOld;                // font that was mapped to the context
	SIZE        textInfo;                       // font info about the text to be displayed
	TEXTMETRIC  fontMetrics;                    // info about the font itself
	CBofRect    cRect;
	HDC hDC;
	BOOL bTempFont;

	//cRect.SetRect(0, 0, pRect->Width() - 1, pRect->Height() - 1);
	//pBmp->Paint(m_pBackground, &cRect, pRect);

	// Attempt to use one of the fonts that we pre-allocated
	//
	hFont = nullptr;
	if (nWeight == TEXT_NORMAL) {

		hFont = m_hDefaultFont[nSize - START_SIZE];
		if (nFont == FONT_MONO) {
			hFont = m_hFixedFont[nSize - START_SIZE];
		}

	}

	// Last resort - create the font now
	//
	bTempFont = FALSE;
	if (hFont == nullptr) {
		bTempFont = TRUE;

		if (nFont != FONT_MONO) {

#if BOF_WINMAC

			hFont = ::CreateFont(nSize, 0, 0, 0, nWeight, 0, 0, 0, 0, OUT_RASTER_PRECIS, 0, PROOF_QUALITY, FF_ROMAN, "Modern");
#else
			hFont = ::CreateFont(nSize, 0, 0, 0, nWeight, 0, 0, 0, 0, OUT_RASTER_PRECIS, 0, PROOF_QUALITY, FF_ROMAN, "MS Sans Serif");
#endif
		} else {

			hFont = GetMonoFont(nSize, nWeight);
		}
	}

	if ((hDC = pBmp->GetDC()) != nullptr) {

		hFontOld = (HFONT)SelectObject(hDC, hFont);    // select it into our context
		GetTextMetrics(hDC, &fontMetrics);      // get some info about the font
		SetBkMode(hDC, TRANSPARENT);            // make the text overlay transparently

		// get the area spanned by the text
#if BOF_WIN16
		GetTextExtentPoint(hDC, pszText, strlen(pszText), &textInfo);
#else
		GetTextExtentPoint32(hDC, pszText, strlen(pszText), &textInfo);
#endif

		m_cPosition.y = (m_cSize.cy - textInfo.cy) >> 1;
		switch (m_nJustify) {

		case JUSTIFY_CENTER:
			m_cPosition.x = (m_cSize.cx - textInfo.cx) >> 1;
			break;

		case JUSTIFY_LEFT:
			m_cPosition.x = 0;
			break;

		case JUSTIFY_RIGHT:
			m_cPosition.x = m_cSize.cx - textInfo.cx;
			break;

		case JUSTIFY_WRAP:
			m_bMultiLine = TRUE;
			break;
		}

		// text starts relative to area for painting
		m_cPosition += pRect->TopLeft();

		if (m_bMultiLine) {
			RECT newRect;

			newRect = *pRect;

			if (bShadowed) {
				RECT rShadowRect;

				rShadowRect.left = newRect.left + m_nShadow_DX;
				rShadowRect.top = newRect.top + m_nShadow_DY;
				rShadowRect.right = newRect.right + m_nShadow_DX;
				rShadowRect.bottom = newRect.bottom + m_nShadow_DY;

				SetTextColor(hDC, m_cShadowColor);      // set the color of the shadow
				DrawText(hDC, (LPCSTR)pszText, strlen(pszText), &rShadowRect, m_nFormatFlags);
			}

			SetTextColor(hDC, m_cTextColor);            // set the color of the text
			DrawText(hDC, (LPCSTR)pszText, strlen(pszText), &newRect, m_nFormatFlags);

		} else {
			if (bShadowed) {
				SetTextColor(hDC, m_cShadowColor);      // set the color of the shadow
				// zap the shadow to the work area
				//
				TabbedTextOut(hDC, m_cPosition.x + m_nShadow_DX, m_cPosition.y + m_nShadow_DY, (LPCSTR) pszText, strlen(pszText), 1, &m_nTabStop, 0);
			}

			SetTextColor(hDC, m_cTextColor);            // set the color of the text

			// zap the text to the work area
			//
			TabbedTextOut(hDC, m_cPosition.x, m_cPosition.y, (LPCSTR) pszText, strlen(pszText), 1, &m_nTabStop, 0);
		}
		::SelectObject(hDC, hFontOld);         // map out the font

		pBmp->ReleaseDC(hDC);
	}

	if (bTempFont) {
		if (::DeleteObject(hFont) == FALSE) {
			LogError(BuildString("::DeleteObject() failed"));
		}
	}

#elif BOF_MAC || BOF_WINMAC
#define TEXTBMPSIZE 16 * 1024

	CGrafPort       stPort;
	Rect            stRect, stOldBounds;
	PixMapHandle    hOldPix;
	GrafPtr         pOldPort;
	BOOL            whiteText = false;
	PaletteHandle   pmh = nullptr;
	ColorInfo       ci;
	UBYTE           privateTextBuff[TEXTBMPSIZE];
	UBYTE          *pPrivateTextBuff;

	// save contents of current port
	GetPort(&pOldPort);

	OpenCPort(&stPort);
	Assert(MemError() == noErr);        // jwl 07.17.96 opencport can fail due to memory restraints

	::SetPalette((WindowPtr) &stPort, pmh, false);
	SetPort((GrafPtr)(&stPort));

	hOldPix = stPort.portPixMap;

	// create an offscreen work area to print the text to
	//
	// jwl 12.06.96 if possible, use a local buffer.
	if ((pRect->Width() * pRect->Height()) < TEXTBMPSIZE) {
		pPrivateTextBuff = privateTextBuff;
	} else {
		pPrivateTextBuff = nullptr;
	}
	CBofBitmap cBitmap(pRect->Width(), pRect->Height(), pBmp->GetPalette(), FALSE, pPrivateTextBuff);
	cBitmap.FillRect(nullptr, COLOR_WHITE);

#if !COPYBITS
	cBitmap.FlipVerticalFast();
#endif

	// set our bitmap as the new port
	PixMapHandle pPixMap = cBitmap.GetMacPixMap();
	SetPortPix(pPixMap);

	// Set text point size, and style
	// jwl 07.02.96 sometimes the size comes in negative, absolute
	// it.  This is a 'windows thing'.

	::TextSize((SHORT)ABS(nSize));
	if (bShadowed)                  // jwl 06.27.96 account for shadowing
		::TextFace(shadow);
	else
		::TextFace((SHORT)nWeight);
	::TextMode(srcCopy);
	::TextFont(nFont);

	RGBColor stColor;

	if (m_cTextColor == 0x00ffffff)
		whiteText = true;

	if (whiteText) {
		stColor.red = 0;
		stColor.blue = 0;
		stColor.green = 0;
	} else {
		// Force background color to be white (which is our transparency color)
		// so that we can do a transparent paint onto the target bitmap
		//
		stColor.red = 0xFFFF;
		stColor.blue = 0xFFFF;
		stColor.green = 0xFFFF;
	}
	RGBBackColor(&stColor);

	// Set the text color
	//
	if (whiteText) {
		stColor.red = 0xFFFF;
		stColor.blue = 0xFFFF;
		stColor.green = 0xFFFF;
	} else {
		UBYTE nearestMatch = CBofApp::GetApp()->GetPalette()->GetNearestIndex(m_cTextColor);
		stColor = (*(*pPixMap)->pmTable)->ctTable[nearestMatch].rgb;
	}
	RGBForeColor(&stColor);


	// put text in this box
	//
	stRect.top = 0;
	stRect.left = 0;
	stRect.bottom = (SHORT)pRect->Height();
	stRect.right = (SHORT)pRect->Width();

	// do the actual drawing to our bitmap
	//

	::TextBox(pszText, strlen(pszText), &stRect, (SHORT)m_nJustify);

	if (whiteText == true) {
		cBitmap.Paint(pBmp, pRect, nullptr, COLOR_BLACK);
	} else {
		cBitmap.Paint(pBmp, pRect, nullptr, COLOR_WHITE);
	}

	// put back current port
	SetPortPix(hOldPix);

	SetPort(pOldPort);
	CloseCPort(&stPort);
#endif

	return m_errCode;
}

BOOL CBofText::WillTextFit(CBofWindow *pWnd, const CHAR *pszText) const {
	CBofRect    cTempRect;
	CBofPalette *pPalette;
	CBofBitmap *pBmp;
	BOOL bFit;
	INT nOldHeight = 0;

	// can't access nullptr pointers
	Assert(pWnd != nullptr);
	Assert(pszText != nullptr);

	// assume won't fit
	bFit = FALSE;

	nOldHeight = m_cRect.Height();

	pPalette = CBofApp::GetApp()->GetPalette();

	//  create the offscreen DC for all splattings
	//
	cTempRect = pWnd->GetWindowRect();
	if ((pBmp  = new CBofBitmap(cTempRect.Width(), cTempRect.Height(), pPalette)) != nullptr) {
#if BOF_WINDOWS
		INT nNewHeight;
		hDC = pBmp->GetDC();
		RECT cRect;

		cRect = m_cRect;

		nNewHeight = ::DrawText(hDC, (LPCSTR)pszText, strlen(pszText), &cRect, (m_nFormatFlags | DT_CALCRECT));
		pBmp->ReleaseDC(hDC);

		delete pBmp;

		if (nNewHeight <= nOldHeight)
			bFit = TRUE;
#endif
	}

	return bFit;
}

ERROR_CODE CBofText::Initialize() {
	ERROR_CODE errCode;

	// Assume no error
	errCode = ERR_NONE;

	// Pre-allocate a bunch of standard fonts/sizes
	//
	if (!m_bInitialized) {

#if BOF_WINDOWS

		INT n;

		n = ::AddFontResource("sserife.fon");

		// Get a mono-spaced font
		m_nMonoType = 0;
		if ((n = ::AddFontResource("modern.fon")) == 0) {

			LogWarning("Unable to add MODERN.FON.  Attemping to add LINEDRAW.TTF...");

			m_nMonoType = 1;
			if ((n = ::AddFontResource("linedraw.ttf")) == 0) {
				LogWarning("Unable to add LINEDRAW.TTF.  Attemping to add COURE.FON...");
				m_nMonoType = 2;

				n = ::AddFontResource("coure.fon");
			}
		}

#if 0
		INT i;

		// Allocate several sizes of a normal font (non-mono-space)
		//
		for (i = 0; i < NUM_POINT_SIZES; i++) {
			Assert(m_hDefaultFont[i] == nullptr);

#if BOF_WINMAC
			m_hDefaultFont[i] = CreateFont(i + START_SIZE, 0, 0, 0, TEXT_NORMAL, 0, 0, 0, 0, OUT_RASTER_PRECIS, 0, PROOF_QUALITY, FF_ROMAN, "Modern");
#else
			m_hDefaultFont[i] = CreateFont(i + START_SIZE, 0, 0, 0, TEXT_NORMAL, 0, 0, 0, 0, OUT_RASTER_PRECIS, 0, PROOF_QUALITY, FF_ROMAN, "MS Sans Serif");
#endif
			if (m_hDefaultFont[i] == nullptr) {
				errCode = ERR_UNKNOWN;
			}
		}

		// Allocate several sizes of a mono-spaced font
		//
		for (i = 0; i < NUM_POINT_SIZES; i++) {
			Assert(m_hFixedFont[i] == nullptr);

			if (m_nMonoType == 0) {
				m_hFixedFont[i] = CreateFont(i + START_SIZE, 0, 0, 0, TEXT_NORMAL, 0, 0, 0, 0, OUT_RASTER_PRECIS, 0, PROOF_QUALITY, FIXED_PITCH | FF_MODERN, "Modern");

			} else if (m_nMonoType == 1) {
				m_hFixedFont[i] = CreateFont(i + START_SIZE, 0, 0, 0, TEXT_NORMAL, 0, 0, 0, 0, OUT_RASTER_PRECIS, 0, PROOF_QUALITY, FIXED_PITCH | FF_MODERN, "MS LineDraw");

			} else if (m_nMonoType == 2) {
				m_hFixedFont[i] = CreateFont(i + START_SIZE, 0, 0, 0, TEXT_NORMAL, 0, 0, 0, 0, OUT_RASTER_PRECIS, 0, PROOF_QUALITY, FIXED_PITCH | FF_MODERN, "Courier");
			}

			if (m_hFixedFont[i] == nullptr) {
				errCode = ERR_UNKNOWN;
			}
		}
#endif

#endif
		m_bInitialized = TRUE;
	}

	return errCode;
}

ERROR_CODE CBofText::ShutDown() {
	ERROR_CODE errCode;

	// Assume no error
	errCode = ERR_NONE;

#if BOF_WINDOWS

	for (int i = 0; i < NUM_POINT_SIZES; i++) {

		if (m_hDefaultFont[i] != nullptr) {
			if (::DeleteObject(m_hDefaultFont[i]) == FALSE) {
				LogError(BuildString("::DeleteObject() failed"));
			}
			m_hDefaultFont[i] = nullptr;
		}
		if (m_hFixedFont[i] != nullptr) {
			if (::DeleteObject(m_hFixedFont[i]) == FALSE) {
				LogError(BuildString("::DeleteObject() failed"));
			}
			m_hFixedFont[i] = nullptr;
		}
	}

#endif

	m_bInitialized = FALSE;

	return (errCode);
}

ERROR_CODE PaintText(CBofWindow *pWnd, CBofRect *pRect, const CHAR *pszString, const INT nSize, const INT nWeight, const RGBCOLOR cColor, INT nJustify, UINT nFormatFlags, INT nFont) {
	Assert(pWnd != nullptr);
	Assert(pRect != nullptr);

	CBofText cText(pRect, nJustify, nFormatFlags);
	return (cText.Display(pWnd, pszString, nSize, nWeight, cColor, nFont));
}

ERROR_CODE PaintText(CBofBitmap *pBmp, CBofRect *pRect, const CHAR *pszString, const INT nSize, const INT nWeight, const RGBCOLOR cColor, INT nJustify, UINT nFormatFlags, INT nFont) {
	Assert(pBmp != nullptr);
	Assert(pRect != nullptr);

	CBofText cText;
	cText.SetupTextOpt(pRect, nJustify, nFormatFlags);

	cText.SetColor(cColor);

	return (cText.DisplayTextEx(pBmp, pszString, pRect, nSize, nWeight, FALSE, nFont));
}

ERROR_CODE PaintShadowedText(CBofWindow *pWnd, CBofRect *pRect, const CHAR *pszString, const INT nSize, const INT nWeight, const RGBCOLOR cColor, INT nJustify, UINT nFormatFlags, INT /*nFont*/) {
	Assert(pWnd != nullptr);
	Assert(pRect != nullptr);

	CBofText cText(pRect, nJustify, nFormatFlags);
	return (cText.DisplayShadowed(pWnd, pszString, nSize, nWeight, cColor));
}

ERROR_CODE PaintShadowedText(CBofBitmap *pBmp, CBofRect *pRect, const CHAR *pszString, const INT nSize, const INT nWeight, const RGBCOLOR cColor, INT nJustify, UINT nFormatFlags, INT nFont) {
	Assert(pBmp != nullptr);
	Assert(pRect != nullptr);

	CBofText cText;
	cText.SetupTextOpt(pRect, nJustify, nFormatFlags);

	cText.SetColor(cColor);
	cText.SetShadowColor(CTEXT_SHADOW_COLOR);
	cText.SetShadowSize(CTEXT_SHADOW_DX, CTEXT_SHADOW_DY);

	return cText.DisplayTextEx(pBmp, pszString, pRect, nSize, nWeight, TRUE, nFont);
}

#if BOF_WINDOWS && !BOF_WINMAC
HFONT CBofText::GetMonoFont(INT nSize, INT nWeight) {
	HFONT       hFont = nullptr;

	switch (m_nMonoType) {
	case 0:
		hFont = CreateFont(nSize, 0, 0, 0, nWeight, 0, 0, 0, 0, OUT_RASTER_PRECIS, 0, PROOF_QUALITY, FIXED_PITCH | FF_MODERN, "Modern");
		break;

	case 1:
		hFont = CreateFont(nSize, 0, 0, 0, nWeight, 0, 0, 0, 0, OUT_RASTER_PRECIS, 0, PROOF_QUALITY, FIXED_PITCH | FF_MODERN, "MS LineDraw");
		break;

	default:
		hFont = CreateFont(nSize, 0, 0, 0, nWeight, 0, 0, 0, 0, OUT_RASTER_PRECIS, 0, PROOF_QUALITY, FIXED_PITCH | FF_MODERN, "Courier");
		break;
	}

	return hFont;
}
#endif

// Adding a utility routine that will calculate the rectangle
// that a text string will fit in (given point size and font).  May have to
// incorporate weight in here someday.  Fixes bug 6640

CBofRect CalculateTextRect(CBofWindow *pWnd, CBofString *pStr, INT nSize, INT nFont) {
	CBofRect    textRect(0, 0, 0, 0);

	warning("STUB: CalculateTextRect()");
#if 0
	// will be completely different for mac vs. pc.
#if BOF_MAC
	FontInfo        fInfo;
	GrafPtr         curPort;

	::GetPort(&curPort);

	// Set the text characteristics before calling textwidth.

	short saveTxSize = curPort->txSize;
	short saveTxFont = curPort->txFont;

	::TextFont(nFont);
	::TextSize(nSize);

	// Use toolbox to find text width.

	::GetFontInfo(&fInfo);

	textRect.right = ::TextWidth(pStr->GetBuffer(), 0, pStr->GetLength());
	textRect.bottom = fInfo.ascent + fInfo.descent + fInfo.leading;

	::TextFont(saveTxFont);
	::TextSize(saveTxSize);
#else
	// Get a font struct reflecting what we want to draw.  Make sure
	// if we're in mono-land, that we use the one that was determined at load time.
	SIZE     stTextSize;
	HDC      hDC;
	HFONT    hFont, hFontOld;

	if ((hDC = pWnd->GetDC()) != nullptr) {
		hFont = nullptr;

		if (nFont == FONT_MONO) {
			hFont = CBofText::GetMonoFont(nSize, 0);
		} else {
			hFont = CreateFont(nSize, 0, 0, 0, 0, 0, 0, 0, 0, OUT_RASTER_PRECIS, 0, PROOF_QUALITY, FF_ROMAN, "MS Sans Serif");
		}
		if (hFont != nullptr) {
			hFontOld = (HFONT)SelectObject(hDC, hFont);    // select it into our context

#if BOF_WIN16 || BOF_WINMAC
			GetTextExtentPoint(hDC, pStr->GetBuffer(), pStr->GetLength(), &stTextSize);
#else
			GetTextExtentPoint32(hDC, pStr->GetBuffer(), pStr->GetLength(), &stTextSize);
#endif
			textRect.right = stTextSize.cx;
			textRect.bottom = stTextSize.cy;

			if (hFontOld != nullptr) {
				::SelectObject(hDC, hFontOld);
			}

			if (::DeleteObject(hFont) == FALSE) {
				LogError(BuildString("::DeleteObject() failed"));
			}
		}
		pWnd->ReleaseDC(hDC);
	}

#endif

#endif
	return textRect;
}

} // namespace Bagel
