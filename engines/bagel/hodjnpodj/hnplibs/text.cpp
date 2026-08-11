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

#include "bagel/hodjnpodj/hnplibs/stdafx.h"
#include "bagel/hodjnpodj/hnplibs/text.h"

namespace Bagel {
namespace HodjNPodj {

int     CText::m_nTabStop = 20;             // tabstops every 20 pixels
bool    CText::m_bFontLoaded = false;       // no font initially loaded


IMPLEMENT_DYNCREATE(CText, CObject)


/*************************************************************************
 *
 * CText()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Constructor for text class.  Initialize all fields.
 *                  SetupText must be called next to build the primary
 *                  data objects and work areas.
 *
 ************************************************************************/

CText::CText() {
	InitializeFields();                         // initialize stuff
}


/*************************************************************************
 *
 * CText()
 *
 * Parameters:
 *
 *  CDC *pDC            device context where the text will be displayed
 *  CPalette *pPalette  palette to associate with the device context
 *  CRect *pRect        rectangular area encompassed by the text object
 *  int nJustify        alignment of text in the rectangle
 *
 * Return Value:    none
 *
 * Description:     Constructor for text class.  Initialize all fields,
 *                  and then build primary data objects and work areas.
 *
 ************************************************************************/

CText::CText(CDC *pDC, CPalette *pPalette, CRect *pRect, int nJustify) {
	InitializeFields();                             // initialize stuff
	SetupText(pDC, pPalette, pRect, nJustify);      // build the work areas
}


/*************************************************************************
 *
 * ~CText()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Destructor for text class.  Tears down all objects created
 *                  for use by the text object; e.g. device contexts, bitmaps,
 *                  and palettes.
 *
 ************************************************************************/

CText::~CText() {
	ReleaseContexts();

	if (m_pWork != nullptr) {
		(*m_pWork).DeleteObject();
		delete m_pWork;
	}
	if (m_pBackground != nullptr) {
		(*m_pBackground).DeleteObject();
		delete m_pBackground;
	}
}


/*************************************************************************
 *
 * InitializeFields
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Initializes key fields to zero or nullptr states.
 *
 ************************************************************************/

void CText::InitializeFields() {
	m_pBackgroundDC = nullptr;
	m_pBackground = nullptr;
	m_pBackgroundOld = nullptr;

	m_pWorkDC = nullptr;
	m_pWork = nullptr;
	m_pWorkOld = nullptr;

	m_pPalette = nullptr;
	m_pPalBackOld = nullptr;
	m_pPalWorkOld = nullptr;

	m_cPosition = CPoint(0, 0);
	m_cSize = CSize(0, 0);
	m_cRect.SetRect(0, 0, 0, 0);

	m_cShadowColor = RGB(0, 0, 0);
	m_nShadow_DX = 0;
	m_nShadow_DY = 0;

	m_bBounded = true;
	m_bHaveBackground = false;
	m_nJustify = JUSTIFY_LEFT;

	m_pFont = nullptr;
}


/*************************************************************************
 *
 * SetupText()
 *
 * Parameters:
 *
 *  CDC *pDC            device context where the text will be displayed
 *  CPalette *pPalette  palette to associate with the device context
 *  CRect *pRect        rectangular area encompassed by the text object
 *  int nJustify        alignment of text in the rectangle
 *
 * Return Value:
 *
 *  bool                success/failure condition
 *
 * Description:         build primary data objects and work areas; text
 *                      will be displayed centered within the defined
 *                      rectangular area, hence it is up to the caller to
 *                      ensure that the text fits (excess is cropped).
 *
 ************************************************************************/

bool CText::SetupText(CDC *pDC, CPalette *pPalette, CRect *pRect, int nJustify) {
	CPalette *pPalOld = nullptr;

	m_bBounded = true;                          // set for bounded area painting
	m_pPalette = pPalette;
	m_nJustify = nJustify;

	if (m_pPalette != nullptr) {
		pPalOld = (*pDC).SelectPalette(m_pPalette, false);
		(*pDC).RealizePalette();
	}

	if (!m_bFontLoaded) {                       // load the font if we have not
		m_bFontLoaded = true;                   // ... done so already
		AddFontResource("msserif.fon");
	}

	m_cRect = *pRect;                           // setup the fields for location
	m_cSize.cx = m_cRect.right - m_cRect.left;  // ... and size of the text area
	m_cSize.cy = m_cRect.bottom - m_cRect.top;

	m_pWork = new CBitmap();                    // create a bitmap to serve as our
	if ((m_pWork == nullptr) ||                    // ... work area as we output text
	        !(*m_pWork).CreateCompatibleBitmap(pDC, m_cSize.cx, m_cSize.cy))
		return false;

	m_pBackground = new CBitmap();              // create a bitmap to hold the
	if ((m_pBackground == nullptr) ||              // ... background we overwrite
	        !(*m_pBackground).CreateCompatibleBitmap(pDC, m_cSize.cx, m_cSize.cy))
		return false;

	if (m_pPalette != nullptr)
		(*pDC).SelectPalette(pPalOld, false);

	return true;                              // return status
}


/*************************************************************************
 *
 * RestoreBackground()
 *
 * Parameters:
 *
 *  CDC *pDC        device context where the text was displayed
 *
 * Return Value:
 *
 *  bool            success/failure condition
 *
 * Description:     repaint the background art, thereby erasing the text.
 *
 ************************************************************************/

bool CText::RestoreBackground(CDC *pDC) {
	bool        bSuccess = false;
	CPalette *pPalOld = nullptr;

	if (m_pPalette != nullptr) {
		pPalOld = (*pDC).SelectPalette(m_pPalette, false);
		(*pDC).RealizePalette();
	}

	if ((m_pBackground != nullptr) &&
	        SetupContexts(pDC)) {
		bSuccess = (*pDC).BitBlt(                   // simply splat the background art
		               m_cRect.left,           // ... back where it came from
		               m_cRect.top,
		               m_cSize.cx,
		               m_cSize.cy,
		               m_pBackgroundDC,
		               0,
		               0,
		               SRCCOPY);
		ReleaseContexts();
	}

	if (m_pPalette != nullptr)
		(*pDC).SelectPalette(pPalOld, false);

	return bSuccess;
}


/*************************************************************************
 *
 * DisplayString()
 *
 * Parameters:
 *
 *  CDC *pDC            device context where the text was displayed
 *  char *pszText       point to text string to be displayed
 *  int nSize           point size of the text to be used
 *  int nWeight         weighting of the font (FW_ identifier)
 *  COLORREF crColor    color that the text will be
 *
 * Return Value:
 *
 *  bool            success/failure condition
 *
 * Description:     display a text string, formatted in the text object area.
 *
 ************************************************************************/

bool CText::DisplayString(CDC *pDC, const char *pszText, const int nSize, const int nWeight, const COLORREF crColor) {
	bool    bSuccess;

	m_cTextColor = crColor;
	bSuccess = DisplayText(pDC, pszText, nSize, nWeight, false);
	return bSuccess;
}


/*************************************************************************
 *
 * DisplayShadowedString()
 *
 * Parameters:
 *
 *  CDC *pDC            evice context where the text was displayed
 *  char *pszText       point to text string to be displayed
 *  int nSize           point size of the text to be used
 *  int nWeight         weighting of the font (FW_ identifier)
 *  COLORREF crColor    color that the text will be
 *  COLORREF crShadow   color that the text's shadow will be
 *
 * Return Value:
 *
 *  bool            success/failure condition
 *
 * Description:     display a shadowed text string, formatted in the text object area.
 *
 ************************************************************************/

bool CText::DisplayShadowedString(CDC *pDC, const char *pszText, const int nSize, const int nWeight, const COLORREF crColor, const COLORREF crShadow, const int nDX, const int nDY) {
	bool    bSuccess;

	m_cTextColor = crColor;
	m_cShadowColor = crShadow;
	m_nShadow_DX = nDX;
	m_nShadow_DY = nDY;
	bSuccess = DisplayText(pDC, pszText, nSize, nWeight, true);
	return bSuccess;
}


/*************************************************************************
 *
 * DisplayText()
 *
 * Parameters:
 *
 *  CDC *pDC            evice context where the text was displayed
 *  char *pszText       point to text string to be displayed
 *  int nSize           point size of the text to be used
 *  int nWeight         weighting of the font (FW_ identifier)
 *  bool bShadowed      whether the text is shadowed
 *
 * Return Value:
 *
 *  bool            success/failure condition
 *
 * Description:     display a text string, formatted in the text object area.
 *
 ************************************************************************/

bool CText::DisplayText(CDC *pDC, const char *pszText, const int nSize, const int nWeight, const bool bShadowed) {
	CFont *pFontOld = nullptr;               // font that was mapped to the context
	CSize       textInfo;                       // font info about the text to be displayed
	TEXTMETRIC  fontMetrics;                    // info about the font itself
	CRect       unionRect;
	CRect       newRect;
	CPalette *pPalOld = nullptr;

	if (m_pPalette != nullptr) {
		pPalOld = (*pDC).SelectPalette(m_pPalette, false);
		(*pDC).RealizePalette();
	}

	if (!SetupContexts(pDC))                        // setup the device contexts and map in
		return false;                             // ... the various bitmaps

	if (!m_bHaveBackground) {
		(*m_pBackgroundDC).BitBlt(           // grab what the background looks like
		    0,                      // ... putting it in the work area
		    0,
		    m_cSize.cx,
		    m_cSize.cy,
		    pDC,
		    m_cRect.left,
		    m_cRect.top,
		    SRCCOPY);
		m_bHaveBackground = true;
	}

	// Create an instance of the specified font
	m_pFont = new CFont();
	(*m_pFont).CreateFont(nSize, 0, 0, 0, nWeight,
		0, 0, 0, 0, OUT_RASTER_PRECIS, 0,
		PROOF_QUALITY, FF_ROMAN, "MS Sans Serif");

	pFontOld = (*m_pWorkDC).SelectObject(m_pFont);  // select it into our context
	(*m_pWorkDC).GetTextMetrics(&fontMetrics);      // get some info about the font
	(*m_pWorkDC).SetBkMode(TRANSPARENT);            // make the text overlay transparently

	textInfo = (*m_pWorkDC).GetTextExtent(pszText, strlen(pszText));  // get the area spanned by the text

	(*m_pWorkDC).BitBlt(                     // copy the saved background to the work area
	    0,
	    0,
	    m_cSize.cx,
	    m_cSize.cy,
	    m_pBackgroundDC,
	    0,
	    0,
	    SRCCOPY);

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
	}

	m_cPosition.x = MAX<int>(m_cPosition.x, 0);
	m_cPosition.y = MAX<int>(m_cPosition.y, 0);

	if (bShadowed) {
		(*m_pWorkDC).SetTextColor(m_cShadowColor);      // set the color of the shadow
		(*m_pWorkDC).TabbedTextOut(                     // zap the shadow to the work area
		    m_cPosition.x + m_nShadow_DX,
		    m_cPosition.y + m_nShadow_DY,
		    (const char *)pszText,
		    strlen(pszText),
		    1, &m_nTabStop, 0);
	}

	(*m_pWorkDC).SetTextColor(m_cTextColor);            // set the color of the text
	(*m_pWorkDC).TabbedTextOut(                         // zap the text to the work area
	    m_cPosition.x,
	    m_cPosition.y,
	    (const char *)pszText,
	    strlen(pszText),
	    1, &m_nTabStop, 0);

	(*m_pWorkDC).SelectObject(pFontOld);         // map out the font

	delete m_pFont;                                     // release the font instance
	m_pFont = nullptr;

	(*pDC).BitBlt(                               // copy the result to the destination context
	    m_cRect.left,
	    m_cRect.top,
	    m_cSize.cx,
	    m_cSize.cy,
	    m_pWorkDC,
	    0,
	    0,
	    SRCCOPY);

	ReleaseContexts();

	if (m_pPalette != nullptr)
		(*pDC).SelectPalette(pPalOld, false);

	return true;
}


/*************************************************************************
 *
 * SetupContexts()
 *
 * Parameters:
 *
 *  CDC *pDC        device context where the text is displayed
 *
 * Return Value:
 *
 *  bool            success/failure condition
 *
 * Description:     create compatible device contexts for the background
 *                  and work areas, and map in their bitmaps.
 *
 ************************************************************************/

bool CText::SetupContexts(CDC *pDC) {
	if (m_pWorkDC == nullptr) {
		m_pWorkDC = new CDC();
		if ((m_pWorkDC == nullptr) ||
		        !(*m_pWorkDC).CreateCompatibleDC(pDC))
			return false;
		if (m_pPalette != nullptr) {
			m_pPalWorkOld = (*m_pWorkDC).SelectPalette(m_pPalette, false);
			(*m_pWorkDC).RealizePalette();
		}
		m_pWorkOld = (*m_pWorkDC).SelectObject(m_pWork);
		if (m_pWorkOld == nullptr)
			return false;
	}

	if (m_pBackgroundDC == nullptr) {
		m_pBackgroundDC = new CDC();
		if ((m_pBackgroundDC == nullptr) ||
		        !(*m_pBackgroundDC).CreateCompatibleDC(pDC))
			return false;
		if (m_pPalette != nullptr) {
			m_pPalBackOld = (*m_pBackgroundDC).SelectPalette(m_pPalette, false);
			(*m_pBackgroundDC).RealizePalette();
		}
		m_pBackgroundOld = (*m_pBackgroundDC).SelectObject(m_pBackground);
		if (m_pBackgroundOld == nullptr)
			return false;
	}

	return true;
}


/*************************************************************************
 *
 * ReleaseContexts()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     release all device contexts after mapping out palettes
 *                  and bitmaps.
 *
 ************************************************************************/

void CText::ReleaseContexts() {
	if (m_pWorkOld != nullptr) {
		(*m_pWorkDC).SelectObject(m_pWorkOld);
		m_pWorkOld = nullptr;
	}
	if (m_pBackgroundOld != nullptr) {
		(*m_pBackgroundDC).SelectObject(m_pBackgroundOld);
		m_pBackgroundOld = nullptr;
	}

	if (m_pPalWorkOld != nullptr) {
		(*m_pWorkDC).SelectPalette(m_pPalWorkOld, false);
		m_pPalWorkOld = nullptr;
	}
	if (m_pPalBackOld != nullptr) {
		(*m_pBackgroundDC).SelectPalette(m_pPalBackOld, false);
		m_pPalBackOld = nullptr;
	}

	if (m_pWorkDC != nullptr) {
		(*m_pWorkDC).DeleteDC();
		delete m_pWorkDC;
		m_pWorkDC = nullptr;
	}
	if (m_pBackgroundDC != nullptr) {
		(*m_pBackgroundDC).DeleteDC();
		delete m_pBackgroundDC;
		m_pBackgroundDC = nullptr;
	}
}

} // namespace HodjNPodj
} // namespace Bagel
