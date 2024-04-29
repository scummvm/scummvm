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
#include "bagel/boflib/app.h"
#include "bagel/boflib/gfx/text.h"

namespace Bagel {

#define START_SIZE 8
#define MONO_FONT "LiberationMono-Regular.ttf"
#define SERIF_FONT_REGULAR "LiberationSans-Regular.ttf"
#define SERIF_FONT_BOLD "LiberationSans-Bold.ttf"
#define TAB_SIZE 50

int CBofText::_tabStop;
bool CBofText::_initialized;
Graphics::Font *CBofText::_defaultFonts[NUM_POINT_SIZES];
Graphics::Font *CBofText::_fixedFonts[NUM_POINT_SIZES];


ErrorCode CBofText::initialize() {
	_initialized = true;
	_tabStop = 20;		// tabstops every 20 pixels
	Common::fill(_defaultFonts, _defaultFonts + NUM_POINT_SIZES,
		(Graphics::Font *)nullptr);
	Common::fill(_fixedFonts, _fixedFonts + NUM_POINT_SIZES,
		(Graphics::Font *)nullptr);

	return ERR_NONE;
}

ErrorCode CBofText::shutdown() {
	for (int i = 0; i < NUM_POINT_SIZES; i++) {
		delete _defaultFonts[i];
		delete _fixedFonts[i];
	}

	_initialized = false;

	return ERR_NONE;
}

CBofText::CBofText() {
	InitializeFields();		// Initialize stuff
}

CBofText::CBofText(const CBofRect *pRect, int nJustify, uint32 nFormatFlags) {
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

void CBofText::InitializeFields() {
	m_pBackground = nullptr;
	m_pWork = nullptr;
	m_bSaved = false;

	m_cPosition = CBofPoint(0, 0);
	m_cSize = CBofSize(0, 0);
	_cRect.SetRect(0, 0, 0, 0);

	m_cShadowColor = RGB(0, 0, 0);
	m_nShadow_DX = 0;
	m_nShadow_DY = 0;

	m_nJustify = JUSTIFY_LEFT;

	m_nFormatFlags = FORMAT_DEFAULT;
	m_bMultiLine = false;

	m_nCurSize = 10;
	m_nCurWeight = TEXT_DONTCARE;
	m_cTextColor = CTEXT_COLOR;
}

ErrorCode CBofText::SetupText(const CBofRect *pRect, int nJustify, uint32 nFormatFlags) {
	// Can't access null pointers
	Assert(pRect != nullptr);

	m_nJustify = nJustify;

	// Setup the fields for location and size of the text area
	_cRect = *pRect;
	m_cSize.cx = _cRect.width();
	m_cSize.cy = _cRect.Height();

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

	return _errCode;
}

ErrorCode CBofText::SetupTextOpt(const CBofRect *pRect, int nJustify, uint32 nFormatFlags) {
	// Can't access null pointers
	Assert(pRect != nullptr);

	m_nJustify = nJustify;
	m_nFormatFlags = nFormatFlags;

	// Setup the fields for location and size of the text area
	_cRect = *pRect;
	m_cSize.cx = _cRect.width();
	m_cSize.cy = _cRect.Height();

	return _errCode;
}

ErrorCode CBofText::erase(CBofWindow *pWnd) {
	// Can't access null pointers
	Assert(pWnd != nullptr);

	if (m_pBackground != nullptr && m_bSaved) {
		// Simply splat the background art back where it came from
		_errCode = m_pBackground->paint(pWnd, &_cRect);
	}

	return _errCode;
}

ErrorCode CBofText::erase(CBofBitmap *pBmp) {
	// Can't access null pointers
	Assert(pBmp != nullptr);

	if (m_pBackground != nullptr && m_bSaved) {
		// Simply splat the background art back where it came from
		_errCode = m_pBackground->paint(pBmp, &_cRect);
	}

	return _errCode;
}

ErrorCode CBofText::display(CBofWindow *pWnd, const char *pszText, const int nSize, const int nWeight, const RGBCOLOR cColor, int nFont) {
	Assert(IsValidObject(this));

	// Can't access null pointers
	Assert(pWnd != nullptr);

	m_cTextColor = cColor;

	return DisplayText(pWnd, pszText, &_cRect, nSize, nWeight, false, nFont);
}

ErrorCode CBofText::display(CBofWindow *pWnd) {
	Assert(IsValidObject(this));
	Assert(pWnd != nullptr);

	return display(pWnd, m_cCurString, m_nCurSize, m_nCurWeight, m_cTextColor);
}

ErrorCode CBofText::display(CBofBitmap *pBmp) {
	Assert(IsValidObject(this));
	Assert(pBmp != nullptr);

	return display(pBmp, m_cCurString, m_nCurSize, m_nCurWeight, m_cTextColor);
}

ErrorCode CBofText::display(CBofBitmap *pBmp, const char *pszText, const int nSize, const int nWeight, const RGBCOLOR cColor, int nFont) {
	// Can't access null pointers
	Assert(pBmp != nullptr);

	m_cTextColor = cColor;

	return DisplayText(pBmp, pszText, &_cRect, nSize, nWeight, false, nFont);
}

ErrorCode CBofText::DisplayShadowed(CBofWindow *pWnd, const char *pszText, const int nSize, const int nWeight, const RGBCOLOR cColor, const RGBCOLOR cShadow, const int nDX, const int nDY, int nFont) {
	// Can't access null pointers
	Assert(pWnd != nullptr);

	m_cTextColor = cColor;
	m_cShadowColor = cShadow;
	m_nShadow_DX = nDX;
	m_nShadow_DY = nDY;

	return DisplayText(pWnd, pszText, &_cRect, nSize, nWeight, true, nFont);
}

ErrorCode CBofText::DisplayShadowed(CBofBitmap *pBmp, const char *pszText, const int nSize, const int nWeight, const RGBCOLOR cColor, const RGBCOLOR cShadow, const int nDX, const int nDY, int nFont) {
	// Can't access null pointers
	Assert(pBmp != nullptr);

	m_cTextColor = cColor;
	m_cShadowColor = cShadow;
	m_nShadow_DX = nDX;
	m_nShadow_DY = nDY;

	return DisplayText(pBmp, pszText, &_cRect, nSize, nWeight, true, nFont);
}

ErrorCode CBofText::DisplayText(CBofWindow *pWnd, const char *pszText, CBofRect *pRect, const int nSize, const int nWeight, const bool bShadowed, int nFont) {
	Assert(IsValidObject(this));
	Assert(pWnd != nullptr);
	Assert(pszText != nullptr);
	Assert(pRect != nullptr);

	CBofRect cRect(0, 0, pRect->width() - 1, pRect->Height() - 1);

	Assert(m_pBackground != nullptr);
	Assert(m_pWork != nullptr);

	if (!m_bSaved) {
		CBofBitmap::SetUseBackdrop(true);
		m_pBackground->CaptureScreen(pWnd, pRect);
		CBofBitmap::SetUseBackdrop(false);
		m_bSaved = true;
	}

	m_pBackground->paint(m_pWork, 0, 0);

	DisplayTextEx(m_pWork, pszText, &cRect, nSize, nWeight, bShadowed, nFont);

	m_pWork->paint(pWnd, pRect);

	return _errCode;
}

ErrorCode CBofText::DisplayText(CBofBitmap *pBmp, const char *pszText, CBofRect *pRect, const int nSize, const int nWeight, const bool bShadowed, int nFont) {
	Assert(IsValidObject(this));
	Assert(pBmp != nullptr);
	Assert(pszText != nullptr);
	Assert(pRect != nullptr);

	CBofRect cRect(0, 0, pRect->width() - 1, pRect->Height() - 1);

	Assert(m_pWork != nullptr);

	Assert(m_pBackground != nullptr);

	if (!m_bSaved) {
		CBofRect r = m_pBackground->getRect();
		pBmp->paint(m_pBackground, &r, pRect);
		m_bSaved = true;
	}

	m_pBackground->paint(m_pWork, 0, 0);

	DisplayTextEx(m_pWork, pszText, &cRect, nSize, nWeight, bShadowed, nFont);

	m_pWork->paint(pBmp, pRect);

	return _errCode;
}

Graphics::Font *CBofText::getFont(int nFont, int nSize, int nWeight) {
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

ErrorCode CBofText::DisplayTextEx(CBofBitmap *pBmp, const char *pszText, CBofRect *pRect, const int nSize, const int nWeight, const bool bShadowed, int nFont) {
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
	font->wordWrapText(Common::U32String(pszText, Common::kWindows1252), pRect->width(), lines);

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
		m_bMultiLine = true;
		break;
	}

	// text starts relative to area for painting
	m_cPosition += pRect->TopLeft();

	// Note: Under ScummVM, even single line drawing uses the multiLine code
	Common::Rect newRect = *pRect;

	if ((m_nFormatFlags & FORMAT_TOP_CENTER) == FORMAT_TOP_CENTER) {
		int h = lines.size() * font->getFontHeight();
		newRect.top = (newRect.top + newRect.bottom) / 2 - h / 2;
		newRect.bottom = newRect.top + h;
	}

	Common::Rect shadowRect = newRect;
	shadowRect.translate(m_nShadow_DX, m_nShadow_DY);

	for (uint i = 0; i < lines.size(); ++i) {
		const Common::U32String &line = lines[i];

		if (bShadowed) {
			color = CBofApp::GetApp()->GetPalette()->GetNearestIndex(m_cShadowColor);
			displayLine(font, surface, line, shadowRect.left, shadowRect.top,
				shadowRect.width(), color, align);
		}

		color = CBofApp::GetApp()->GetPalette()->GetNearestIndex(m_cTextColor);
		displayLine(font, surface, line, newRect.left, newRect.top,
			newRect.width(), color, align);

		newRect.top += font->getFontHeight();
		shadowRect.top += font->getFontHeight();
	}

	return _errCode;
}

void CBofText::displayLine(Graphics::Font *font, Graphics::ManagedSurface &surface,
		const Common::String &line, int left, int top, int width, int color, Graphics::TextAlign align) {
	if (!line.contains('\t')) {
		font->drawString(&surface, line, left, top, width, color, align);

	} else {
		// Special rendering of tabbed text
		Common::String str = line;

		while (!str.empty()) {
			if (str.hasPrefix("\t")) {
				// Move to next tab stop
				left = (left + TAB_SIZE) / TAB_SIZE * TAB_SIZE;
				str.deleteChar(0);

			} else {
				Common::String fragment;
				size_t tab = str.findFirstOf('\t');
				if (tab == Common::String::npos) {
					fragment = str;
					str.clear();
				} else {
					fragment = Common::String(str.c_str(), str.c_str() + tab);
					str = Common::String(str.c_str() + tab);
				}

				int fragmentWidth = font->getStringWidth(fragment);
				font->drawString(&surface, fragment, left, top, width, color, align);

				left += fragmentWidth;
				width -= fragmentWidth;
			}
		}
	}
}

ErrorCode PaintText(CBofWindow *pWnd, CBofRect *pRect, const char *pszString, const int nSize, const int nWeight, const RGBCOLOR cColor, int nJustify, uint32 nFormatFlags, int nFont) {
	Assert(pWnd != nullptr);
	Assert(pRect != nullptr);

	CBofText cText(pRect, nJustify, nFormatFlags);
	return cText.display(pWnd, pszString, nSize, nWeight, cColor, nFont);
}

ErrorCode PaintText(CBofBitmap *pBmp, CBofRect *pRect, const char *pszString, const int nSize, const int nWeight, const RGBCOLOR cColor, int nJustify, uint32 nFormatFlags, int nFont) {
	Assert(pBmp != nullptr);
	Assert(pRect != nullptr);

	CBofText cText;
	cText.SetupTextOpt(pRect, nJustify, nFormatFlags);

	cText.setColor(cColor);

	return cText.DisplayTextEx(pBmp, pszString, pRect, nSize, nWeight, false, nFont);
}

ErrorCode PaintShadowedText(CBofWindow *pWnd, CBofRect *pRect, const char *pszString, const int nSize, const int nWeight, const RGBCOLOR cColor, int nJustify, uint32 nFormatFlags, int /*nFont*/) {
	Assert(pWnd != nullptr);
	Assert(pRect != nullptr);

	CBofText cText(pRect, nJustify, nFormatFlags);
	return cText.DisplayShadowed(pWnd, pszString, nSize, nWeight, cColor);
}

ErrorCode PaintShadowedText(CBofBitmap *pBmp, CBofRect *pRect, const char *pszString, const int nSize, const int nWeight, const RGBCOLOR cColor, int nJustify, uint32 nFormatFlags, int nFont) {
	Assert(pBmp != nullptr);
	Assert(pRect != nullptr);

	CBofText cText;
	cText.SetupTextOpt(pRect, nJustify, nFormatFlags);

	cText.setColor(cColor);
	cText.SetShadowColor(CTEXT_SHADOW_COLOR);
	cText.SetShadowSize(CTEXT_SHADOW_DX, CTEXT_SHADOW_DY);

	return cText.DisplayTextEx(pBmp, pszString, pRect, nSize, nWeight, true, nFont);
}

CBofRect CalculateTextRect(CBofWindow *pWnd, const CBofString *pStr, int nSize, int nFont) {
	return CalculateTextRect(pWnd->getRect(), pStr, nSize, nFont);
}

CBofRect CalculateTextRect(CBofRect rect, const CBofString *pStr, int nSize, int nFont) {
	// Get the font to use
	Graphics::Font *font = CBofText::getFont(nFont, nSize, TEXT_NORMAL);

	// Wrap the text as necessary
	Common::U32StringArray lines;
	font->wordWrapText(Common::U32String(pStr->GetBuffer(), Common::kWindows1252), rect.width(), lines);

	// Iterate the lines to get the maximum width
	int maxWidth = 0;
	for (uint i = 0; i < lines.size(); ++i)
		maxWidth = MAX(maxWidth, font->getStringWidth(lines[i]));

	return CBofRect(0, 0, maxWidth, (int)lines.size() * font->getFontHeight());
}

} // namespace Bagel
