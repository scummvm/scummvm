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

#include "graphics/fonts/ttf.h"
#include "bagel/boflib/boffo.h"
#include "bagel/boflib/app.h"
#include "bagel/boflib/gfx/text.h"

namespace Bagel {

#define START_SIZE 8
#define MONO_FONT "LiberationMono-Regular.ttf"
#define SERIF_FONT_REGULAR "LiberationSans-Regular.ttf"
#define SERIF_FONT_BOLD "LiberationSans-Bold.ttf"

INT CBofText::_tabStop;
BOOL CBofText::_initialized;
Graphics::Font *CBofText::_defaultFonts[NUM_POINT_SIZES];
Graphics::Font *CBofText::_fixedFonts[NUM_POINT_SIZES];


ERROR_CODE CBofText::Initialize() {
	_initialized = TRUE;
	_tabStop = 20;		// tabstops every 20 pixels
	Common::fill(_defaultFonts, _defaultFonts + NUM_POINT_SIZES,
		(Graphics::Font *)nullptr);
	Common::fill(_fixedFonts, _fixedFonts + NUM_POINT_SIZES,
		(Graphics::Font *)nullptr);

	return ERR_NONE;
}

ERROR_CODE CBofText::ShutDown() {
	for (int i = 0; i < NUM_POINT_SIZES; i++) {
		delete _defaultFonts[i];
		delete _fixedFonts[i];
	}

	_initialized = FALSE;

	return ERR_NONE;
}

CBofText::CBofText() {
	InitializeFields();		// initialize stuff
}

CBofText::CBofText(CBofRect *pRect, INT nJustify, UINT nFormatFlags) {
	// Can't access null pointers
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
	// Can't access null pointers
	Assert(pRect != nullptr);

	m_nJustify = nJustify;

	// Setup the fields for location and size of the text area
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

	// Create a bitmap to serve as our work area as we output text
	if ((m_pWork = new CBofBitmap(m_cSize.cx, m_cSize.cy, pPalette)) != nullptr) {
		// Create a bitmap to hold the background we overwrite
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
	// Can't access null pointers
	Assert(pRect != nullptr);

	m_nJustify = nJustify;

	// Setup the fields for location and size of the text area
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
	// Can't access null pointers
	Assert(pBmp != nullptr);

	if (m_pBackground != nullptr && m_bSaved) {
		// Simply splat the background art back where it came from
		m_errCode = m_pBackground->Paint(pBmp, &m_cRect);
	}

	return m_errCode;
}

ERROR_CODE CBofText::Display(CBofWindow *pWnd, const CHAR *pszText, const INT nSize, const INT nWeight, const RGBCOLOR cColor, INT nFont) {
	Assert(IsValidObject(this));

	// Can't access null pointers
	Assert(pWnd != nullptr);

	m_cTextColor = cColor;

	return DisplayText(pWnd, pszText, &m_cRect, nSize, nWeight, FALSE, nFont);
}

ERROR_CODE CBofText::Display(CBofWindow *pWnd) {
	Assert(IsValidObject(this));
	Assert(pWnd != nullptr);

	return Display(pWnd, m_cCurString, m_nCurSize, m_nCurWeight, m_cTextColor);
}

ERROR_CODE CBofText::Display(CBofBitmap *pBmp) {
	Assert(IsValidObject(this));
	Assert(pBmp != nullptr);

	return Display(pBmp, m_cCurString, m_nCurSize, m_nCurWeight, m_cTextColor);
}

ERROR_CODE CBofText::Display(CBofBitmap *pBmp, const CHAR *pszText, const INT nSize, const INT nWeight, const RGBCOLOR cColor, INT nFont) {
	// Can't access null pointers
	Assert(pBmp != nullptr);

	m_cTextColor = cColor;

	return DisplayText(pBmp, pszText, &m_cRect, nSize, nWeight, FALSE, nFont);
}

ERROR_CODE CBofText::DisplayShadowed(CBofWindow *pWnd, const CHAR *pszText, const INT nSize, const INT nWeight, const RGBCOLOR cColor, const RGBCOLOR cShadow, const INT nDX, const INT nDY, INT nFont) {
	// Can't access null pointers
	Assert(pWnd != nullptr);

	m_cTextColor = cColor;
	m_cShadowColor = cShadow;
	m_nShadow_DX = nDX;
	m_nShadow_DY = nDY;

	return DisplayText(pWnd, pszText, &m_cRect, nSize, nWeight, TRUE, nFont);
}

ERROR_CODE CBofText::DisplayShadowed(CBofBitmap *pBmp, const CHAR *pszText, const INT nSize, const INT nWeight, const RGBCOLOR cColor, const RGBCOLOR cShadow, const INT nDX, const INT nDY, INT nFont) {
	// Can't access null pointers
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

Graphics::Font *CBofText::getFont(INT nFont, INT nSize, INT nWeight) {
	Graphics::Font *font;

	// Attempt to use one of the fonts that we pre-allocated
	if (nFont != FONT_MONO) {
		font = _defaultFonts[nSize - START_SIZE];
	} else {
		font = _fixedFonts[nSize - START_SIZE];
	}

	// Last resort - create the font now
	if (font == nullptr) {
		if (nFont != FONT_MONO) {
			font = Graphics::loadTTFFontFromArchive(SERIF_FONT_REGULAR, nSize, Graphics::kTTFSizeModeCell);
			_defaultFonts[nSize - START_SIZE] = font;
		} else {
			font = Graphics::loadTTFFontFromArchive(MONO_FONT, nSize, Graphics::kTTFSizeModeCell);
			_fixedFonts[nSize - START_SIZE] = font;
		}
	}

	return font;
}

ERROR_CODE CBofText::DisplayTextEx(CBofBitmap *pBmp, const CHAR *pszText, CBofRect *pRect, const INT nSize, const INT nWeight, const BOOL bShadowed, INT nFont) {
	Assert(IsValidObject(this));

	// can't access null pointers
	Assert(pBmp != nullptr);
	Assert(pszText != nullptr);
	Assert(pRect != nullptr);

	Graphics::ManagedSurface surface = pBmp->getSurface();
	Graphics::Font *font = getFont(nFont, nSize, nWeight);
	CBofRect cRect;
	int color;

	// Split lines
	Common::U32StringArray lines;
	font->wordWrapText(Common::U32String(pszText, Common::kWindows1252), pRect->Width(), lines);

	// Iterate the lines to get the maximum width
	int maxWidth = 0;
	for (uint i = 0; i < lines.size(); ++i)
		maxWidth = MAX(maxWidth, font->getStringWidth(lines[i]));
	Common::Point textInfo(maxWidth, (int)lines.size() * font->getFontHeight());

	m_cPosition.y = (m_cSize.cy - textInfo.y) >> 1;

	Graphics::TextAlign align = Graphics::kTextAlignLeft;
	switch (m_nJustify) {
	case JUSTIFY_CENTER:
		m_cPosition.x = (m_cSize.cx - textInfo.x) >> 1;
		align = Graphics::kTextAlignCenter;
		break;

	case JUSTIFY_LEFT:
		m_cPosition.x = 0;
		align = Graphics::kTextAlignLeft;
		break;

	case JUSTIFY_RIGHT:
		m_cPosition.x = m_cSize.cx - textInfo.x;
		align = Graphics::kTextAlignRight;
		break;

	case JUSTIFY_WRAP:
		m_bMultiLine = TRUE;
		break;
	}

	// text starts relative to area for painting
	m_cPosition += pRect->TopLeft();

	if (!m_bMultiLine) {
		// TODO: For single line drawing, it uses Windows TabbedTextOut.
		// Until we can determine if tabs are used anymore, simply use
		// the multi-line text writing code even for single lines
		m_bMultiLine = true;
	}

	if (m_bMultiLine) {
		Common::Rect newRect = *pRect;
		Common::Rect shadowRect = newRect;
		shadowRect.translate(m_nShadow_DX, m_nShadow_DY);

		for (uint i = 0; i < lines.size(); ++i) {
			const Common::U32String &line = lines[i];

			if (bShadowed) {
				color = CBofApp::GetApp()->GetPalette()->GetNearestIndex(m_cShadowColor);
				font->drawString(&surface, line, shadowRect.left, shadowRect.top,
					shadowRect.width(), color, align);
			}

			color = CBofApp::GetApp()->GetPalette()->GetNearestIndex(m_cTextColor);
			font->drawString(&surface, line, newRect.left, newRect.top,
				newRect.width(), color, align);

			newRect.top += font->getFontHeight();
			shadowRect.top += font->getFontHeight();
		}
	} else {
		// TODO
	}

	return m_errCode;
}

ERROR_CODE PaintText(CBofWindow *pWnd, CBofRect *pRect, const CHAR *pszString, const INT nSize, const INT nWeight, const RGBCOLOR cColor, INT nJustify, UINT nFormatFlags, INT nFont) {
	Assert(pWnd != nullptr);
	Assert(pRect != nullptr);

	CBofText cText(pRect, nJustify, nFormatFlags);
	return cText.Display(pWnd, pszString, nSize, nWeight, cColor, nFont);
}

ERROR_CODE PaintText(CBofBitmap *pBmp, CBofRect *pRect, const CHAR *pszString, const INT nSize, const INT nWeight, const RGBCOLOR cColor, INT nJustify, UINT nFormatFlags, INT nFont) {
	Assert(pBmp != nullptr);
	Assert(pRect != nullptr);

	CBofText cText;
	cText.SetupTextOpt(pRect, nJustify, nFormatFlags);

	cText.SetColor(cColor);

	return cText.DisplayTextEx(pBmp, pszString, pRect, nSize, nWeight, FALSE, nFont);
}

ERROR_CODE PaintShadowedText(CBofWindow *pWnd, CBofRect *pRect, const CHAR *pszString, const INT nSize, const INT nWeight, const RGBCOLOR cColor, INT nJustify, UINT nFormatFlags, INT /*nFont*/) {
	Assert(pWnd != nullptr);
	Assert(pRect != nullptr);

	CBofText cText(pRect, nJustify, nFormatFlags);
	return cText.DisplayShadowed(pWnd, pszString, nSize, nWeight, cColor);
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

CBofRect CalculateTextRect(CBofWindow *pWnd, const CBofString *pStr, INT nSize, INT nFont) {
	return CalculateTextRect(pWnd->GetRect(), pStr, nSize, nFont);
}

CBofRect CalculateTextRect(CBofRect rect, const CBofString *pStr, INT nSize, INT nFont) {
	// Get the font to use
	Graphics::Font *font = CBofText::getFont(nFont, nSize, TEXT_NORMAL);

	// Wrap the text as necessary
	Common::U32StringArray lines;
	font->wordWrapText(Common::U32String(pStr->GetBuffer(), Common::kWindows1252), rect.Width(), lines);

	// Iterate the lines to get the maximum width
	int maxWidth = 0;
	for (uint i = 0; i < lines.size(); ++i)
		maxWidth = MAX(maxWidth, font->getStringWidth(lines[i]));

	return CBofRect(0, 0, maxWidth, (int)lines.size() * font->getFontHeight());
}

} // namespace Bagel
