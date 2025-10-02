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

#ifndef HODJNPODJ_HNPLIBS_TEXT_H
#define HODJNPODJ_HNPLIBS_TEXT_H

#include "bagel/afxwin.h"

namespace Bagel {
namespace HodjNPodj {

// text color and offset definitions

#define CTEXT_COLOR         RGB(0,0,0)
#define CTEXT_SHADOW_COLOR  RGB(0,0,0)
#define CTEXT_SHADOW_DX     2
#define CTEXT_SHADOW_DY     2

// text justification definitions
enum {
	JUSTIFY_CENTER      = 0,
	JUSTIFY_LEFT        = 1,
	JUSTIFY_RIGHT       = 2
};

// text weight definitions

#define TEXT_DONTCARE       0
#define TEXT_THIN           FW_THIN
#define TEXT_EXTRALIGHT     FW_EXTRALIGHT
#define TEXT_ULTRALIGHT     FW_ULTRALIGHT
#define TEXT_LIGHT          FW_LIGHT
#define TEXT_NORMAL         FW_NORMAL
#define TEXT_REGULAR        FW_REGULAR
#define TEXT_MEDIUM         FW_MEDIUM
#define TEXT_SEMIBOLD       FW_SEMIBOLD
#define TEXT_DEMIBOLD       FW_DEMIBOLD
#define TEXT_BOLD           FW_BOLD
#define TEXT_EXTRABOLD      FW_EXTRABOLD
#define TEXT_ULTRABOLD      FW_ULTRABOLD
#define TEXT_BLACK          FW_BLACK
#define TEXT_HEAVY          FW_HEAVY

class CText : public CObject {
	DECLARE_DYNCREATE(CText)

public:
	CText();
	CText(CDC *pDC, CPalette *pPalette, CRect *pRect, int nJustify = JUSTIFY_CENTER);
	~CText();

	bool SetupText(CDC *pDC, CPalette * pPalette, CRect *pRect, int nJustify = JUSTIFY_CENTER);
	bool RestoreBackground(CDC *pDC);
	bool DisplayString(CDC *pDC, const char* pszText, const int nSize, const int nWeight, const COLORREF crColor = CTEXT_COLOR);
	bool DisplayShadowedString(CDC *pDC, const char* pszText, const int nSize, const int nWeight, const COLORREF crColor, const COLORREF crShadow = CTEXT_SHADOW_COLOR, const int DX = CTEXT_SHADOW_DX, const int DY = CTEXT_SHADOW_DY);

private:
	void InitializeFields();
	bool SetupContexts(CDC *pDC);
	void ReleaseContexts();
	bool DisplayText(CDC *pDC, const char* pszText, const int nSize, const int nWeight, const bool bShadowed);

private:
	CDC         *m_pBackgroundDC = nullptr;	// offscreen bitmap device context for background
	CBitmap     *m_pBackground = nullptr;	// bitmap for the text's background
	CBitmap     *m_pBackgroundOld = nullptr; // previous bitmap mapped in the DC
	CDC         *m_pWorkDC = nullptr;		// offscreen bitmap device context for work area
	CBitmap     *m_pWork = nullptr;			// bitmap for the work area
	CBitmap     *m_pWorkOld = nullptr;		// previous bitmap mapped in the DC
	CPalette    *m_pPalette = nullptr;		// color palette for the text
	CPalette    *m_pPalBackOld = nullptr;	// previous palette mapped to background DC
	CPalette    *m_pPalWorkOld = nullptr;	// previous palette mapped to work area DC
	CPoint      m_cPosition;				// upper left corner of text displayed
	CSize       m_cSize;					// dx/dy size of the text bitmap
	CRect       m_cRect;					// bounding rectangle of text area
	CFont       *m_pFont = nullptr;         // font to use for the text
	int         m_nJustify = 0;				// positioning within the rectangle
	bool        m_bBounded = false;			// bounded versus free-form text output
	bool        m_bHaveBackground = false;	// whether the background has been saved
	COLORREF    m_cTextColor = 0;			// color to use for the text itself
	COLORREF    m_cShadowColor = 0;			// color to use for the text's shadow
	int         m_nShadow_DX = 0;			// horizontal offset for shadow
	int         m_nShadow_DY = 0;			// vertical offset for shadow

	static  int     m_nTabStop;				// tabstop table
	static  bool    m_bFontLoaded;			// font loaded flag
};

} // namespace HodjNPodj
} // namespace Bagel

#endif
