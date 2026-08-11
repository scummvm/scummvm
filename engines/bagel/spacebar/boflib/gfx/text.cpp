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
#include "bagel/spacebar/boflib/app.h"
#include "bagel/spacebar/boflib/gfx/text.h"

namespace Bagel {
namespace SpaceBar {

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
	_tabStop = 20;      // tabstops every 20 pixels
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
	initializeFields();     // Initialize stuff
}

CBofText::CBofText(const CBofRect *pRect, int nJustify, uint32 nFormatFlags) {
	// Can't access null pointers
	assert(pRect != nullptr);

	// Initialize stuff
	initializeFields();

	// Build the work areas
	setupText(pRect, nJustify, nFormatFlags);
}

CBofText::~CBofText() {
	delete _pWork;
	_pWork = nullptr;

	delete _pBackground;
	_pBackground = nullptr;
}

void CBofText::initializeFields() {
	_pBackground = nullptr;
	_pWork = nullptr;
	_bSaved = false;

	_cPosition = CBofPoint(0, 0);
	_cSize = CBofSize(0, 0);
	_cRect.setRect(0, 0, 0, 0);

	_cShadowColor = RGB(0, 0, 0);
	_nShadow_DX = 0;
	_nShadow_DY = 0;

	_nJustify = JUSTIFY_LEFT;

	_nFormatFlags = FORMAT_DEFAULT;
	_bMultiLine = false;

	_nCurSize = 10;
	_nCurWeight = TEXT_DONTCARE;
	_cTextColor = CTEXT_COLOR;
}

ErrorCode CBofText::setupText(const CBofRect *pRect, int nJustify, uint32 nFormatFlags) {
	// Can't access null pointers
	assert(pRect != nullptr);

	_nJustify = nJustify;

	// Setup the fields for location and size of the text area
	_cRect = *pRect;
	_cSize.cx = _cRect.width();
	_cSize.cy = _cRect.height();

	delete _pWork;
	_pWork = nullptr;

	delete _pBackground;
	_pBackground = nullptr;

	CBofPalette *pPalette = CBofApp::getApp()->getPalette();

	// Create a bitmap to serve as our work area as we output text
	_pWork = new CBofBitmap(_cSize.cx, _cSize.cy, pPalette);

	// Create a bitmap to hold the background we overwrite
	_pBackground = new CBofBitmap(_cSize.cx, _cSize.cy, pPalette);

	return _errCode;
}

ErrorCode CBofText::setupTextOpt(const CBofRect *pRect, int nJustify, uint32 nFormatFlags) {
	// Can't access null pointers
	assert(pRect != nullptr);

	_nJustify = nJustify;
	_nFormatFlags = nFormatFlags;

	// Setup the fields for location and size of the text area
	_cRect = *pRect;
	_cSize.cx = _cRect.width();
	_cSize.cy = _cRect.height();

	return _errCode;
}

ErrorCode CBofText::erase(CBofWindow *pWnd) {
	// Can't access null pointers
	assert(pWnd != nullptr);

	if (_pBackground != nullptr && _bSaved) {
		// Simply splat the background art back where it came from
		_errCode = _pBackground->paint(pWnd, &_cRect);
	}

	return _errCode;
}

ErrorCode CBofText::erase(CBofBitmap *pBmp) {
	// Can't access null pointers
	assert(pBmp != nullptr);

	if (_pBackground != nullptr && _bSaved) {
		// Simply splat the background art back where it came from
		_errCode = _pBackground->paint(pBmp, &_cRect);
	}

	return _errCode;
}

ErrorCode CBofText::display(CBofWindow *pWnd, const char *pszText, const int nSize, const int nWeight, const COLORREF cColor, int nFont) {
	assert(isValidObject(this));

	// Can't access null pointers
	assert(pWnd != nullptr);

	_cTextColor = cColor;

	return displayText(pWnd, pszText, &_cRect, nSize, nWeight, false, nFont);
}

ErrorCode CBofText::display(CBofWindow *pWnd) {
	assert(isValidObject(this));
	assert(pWnd != nullptr);

	return display(pWnd, _cCurString, _nCurSize, _nCurWeight, _cTextColor);
}

ErrorCode CBofText::display(CBofBitmap *pBmp) {
	assert(isValidObject(this));
	assert(pBmp != nullptr);

	return display(pBmp, _cCurString, _nCurSize, _nCurWeight, _cTextColor);
}

ErrorCode CBofText::display(CBofBitmap *pBmp, const char *pszText, const int nSize, const int nWeight, const COLORREF cColor, int nFont) {
	// Can't access null pointers
	assert(pBmp != nullptr);

	_cTextColor = cColor;

	return displayText(pBmp, pszText, &_cRect, nSize, nWeight, false, nFont);
}

ErrorCode CBofText::displayShadowed(CBofWindow *pWnd, const char *pszText, const int nSize, const int nWeight, const COLORREF cColor, const COLORREF cShadow, const int nDX, const int nDY, int nFont) {
	// Can't access null pointers
	assert(pWnd != nullptr);

	_cTextColor = cColor;
	_cShadowColor = cShadow;
	_nShadow_DX = nDX;
	_nShadow_DY = nDY;

	return displayText(pWnd, pszText, &_cRect, nSize, nWeight, true, nFont);
}

ErrorCode CBofText::displayText(CBofWindow *pWnd, const char *pszText, CBofRect *pRect, const int nSize, const int nWeight, const bool bShadowed, int nFont) {
	assert(isValidObject(this));
	assert(pWnd != nullptr);
	assert(pszText != nullptr);
	assert(pRect != nullptr);

	CBofRect cRect(0, 0, pRect->width() - 1, pRect->height() - 1);

	assert(_pBackground != nullptr);
	assert(_pWork != nullptr);

	if (!_bSaved) {
		CBofBitmap::setUseBackdrop(true);
		_pBackground->captureScreen(pWnd, pRect);
		CBofBitmap::setUseBackdrop(false);
		_bSaved = true;
	}

	_pBackground->paint(_pWork, 0, 0);

	displayTextEx(_pWork, pszText, &cRect, nSize, nWeight, bShadowed, nFont);

	_pWork->paint(pWnd, pRect);

	return _errCode;
}

ErrorCode CBofText::displayText(CBofBitmap *pBmp, const char *pszText, CBofRect *pRect, const int nSize, const int nWeight, const bool bShadowed, int nFont) {
	assert(isValidObject(this));
	assert(pBmp != nullptr);
	assert(pszText != nullptr);
	assert(pRect != nullptr);

	CBofRect cRect(0, 0, pRect->width() - 1, pRect->height() - 1);

	assert(_pWork != nullptr);

	assert(_pBackground != nullptr);

	if (!_bSaved) {
		CBofRect r = _pBackground->getRect();
		pBmp->paint(_pBackground, &r, pRect);
		_bSaved = true;
	}

	_pBackground->paint(_pWork, 0, 0);

	displayTextEx(_pWork, pszText, &cRect, nSize, nWeight, bShadowed, nFont);

	_pWork->paint(pBmp, pRect);

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

ErrorCode CBofText::displayTextEx(CBofBitmap *pBmp, const char *pszText, CBofRect *pRect, const int nSize, const int nWeight, const bool bShadowed, int nFont) {
	assert(isValidObject(this));

	// can't access null pointers
	assert(pBmp != nullptr);
	assert(pszText != nullptr);
	assert(pRect != nullptr);

	Graphics::ManagedSurface surface = pBmp->getSurface();
	Graphics::Font *font = getFont(nFont, nSize, nWeight);
	int color;

	// Split lines
	Common::U32StringArray lines;
	font->wordWrapText(Common::U32String(pszText, Common::kWindows1252), pRect->width(), lines);

	// Iterate the lines to get the maximum width
	int maxWidth = 0;
	for (uint i = 0; i < lines.size(); ++i)
		maxWidth = MAX(maxWidth, font->getStringWidth(lines[i]));
	Common::Point textInfo(maxWidth, (int)lines.size() * font->getFontHeight());

	_cPosition.y = (_cSize.cy - textInfo.y) >> 1;

	Graphics::TextAlign align = Graphics::kTextAlignLeft;
	switch (_nJustify) {
	case JUSTIFY_CENTER:
		_cPosition.x = (_cSize.cx - textInfo.x) >> 1;
		align = Graphics::kTextAlignCenter;
		break;

	case JUSTIFY_LEFT:
		// align left
		_cPosition.x = 0;
		break;

	case JUSTIFY_RIGHT:
		_cPosition.x = _cSize.cx - textInfo.x;
		align = Graphics::kTextAlignRight;
		break;

	case JUSTIFY_WRAP:
		// Align left
		_bMultiLine = true;
		break;

	default:
		break;
	}

	// text starts relative to area for painting
	_cPosition += pRect->topLeft();

	// Note: Under ScummVM, even single line drawing uses the multiLine code
	Common::Rect newRect = *pRect;

	if ((_nFormatFlags & FORMAT_TOP_CENTER) == FORMAT_TOP_CENTER) {
		int h = lines.size() * font->getFontHeight();
		newRect.top = (newRect.top + newRect.bottom) / 2 - h / 2;
		newRect.bottom = newRect.top + h;
	}

	Common::Rect shadowRect = newRect;
	shadowRect.translate(_nShadow_DX, _nShadow_DY);

	for (uint i = 0; i < lines.size(); ++i) {
		const Common::U32String &line = lines[i];

		if (bShadowed) {
			color = CBofApp::getApp()->getPalette()->getNearestIndex(_cShadowColor);
			displayLine(font, surface, line, shadowRect.left, shadowRect.top,
			            shadowRect.width(), color, align);
		}

		color = CBofApp::getApp()->getPalette()->getNearestIndex(_cTextColor);
		displayLine(font, surface, line, newRect.left, newRect.top,
		            newRect.width(), color, align);

		newRect.top += font->getFontHeight();
		shadowRect.top += font->getFontHeight();
	}

	return _errCode;
}

void CBofText::displayLine(Graphics::Font *font, Graphics::ManagedSurface &surface,
                           const Common::U32String &line, int left, int top, int width, int color, Graphics::TextAlign align) {
	if (!line.contains('\t')) {
		font->drawString(&surface, line, left, top, width, color, align);

	} else {
		// Special rendering of tabbed text
		Common::U32String str = line;

		while (!str.empty()) {
			if (str[0] == '\t') {
				// Move to next tab stop
				left = (left + TAB_SIZE) / TAB_SIZE * TAB_SIZE;
				str.deleteChar(0);

			} else {
				Common::U32String fragment;
				size_t tab = str.findFirstOf('\t');
				if (tab == Common::U32String::npos) {
					fragment = str;
					str.clear();
				} else {
					fragment = Common::U32String(str.c_str(), str.c_str() + tab);
					str = Common::U32String(str.c_str() + tab);
				}

				int fragmentWidth = font->getStringWidth(fragment);
				font->drawString(&surface, fragment, left, top, width, color, align);

				left += fragmentWidth;
				width -= fragmentWidth;
			}
		}
	}
}

ErrorCode paintText(CBofWindow *pWnd, CBofRect *pRect, const char *pszString, const int nSize, const int nWeight, const COLORREF cColor, int nJustify, uint32 nFormatFlags, int nFont) {
	assert(pWnd != nullptr);
	assert(pRect != nullptr);

	CBofText cText(pRect, nJustify, nFormatFlags);
	return cText.display(pWnd, pszString, nSize, nWeight, cColor, nFont);
}

ErrorCode paintText(CBofBitmap *pBmp, CBofRect *pRect, const char *pszString, const int nSize, const int nWeight, const COLORREF cColor, int nJustify, uint32 nFormatFlags, int nFont) {
	assert(pBmp != nullptr);
	assert(pRect != nullptr);

	CBofText cText;
	cText.setupTextOpt(pRect, nJustify, nFormatFlags);

	cText.setColor(cColor);

	return cText.displayTextEx(pBmp, pszString, pRect, nSize, nWeight, false, nFont);
}

ErrorCode paintShadowedText(CBofBitmap *pBmp, CBofRect *pRect, const char *pszString, const int nSize, const int nWeight, const COLORREF cColor, int nJustify, uint32 nFormatFlags, int nFont) {
	assert(pBmp != nullptr);
	assert(pRect != nullptr);

	CBofText cText;
	cText.setupTextOpt(pRect, nJustify, nFormatFlags);

	cText.setColor(cColor);
	cText.setShadowColor(CTEXT_SHADOW_COLOR);
	cText.setShadowSize(CTEXT_SHADOW_DX, CTEXT_SHADOW_DY);

	return cText.displayTextEx(pBmp, pszString, pRect, nSize, nWeight, true, nFont);
}

CBofRect calculateTextRect(CBofWindow *pWnd, const CBofString *pStr, int nSize, int nFont) {
	return calculateTextRect(pWnd->getRect(), pStr, nSize, nFont);
}

CBofRect calculateTextRect(CBofRect rect, const CBofString *pStr, int nSize, int nFont) {
	// Get the font to use
	Graphics::Font *font = CBofText::getFont(nFont, nSize, TEXT_NORMAL);

	// Wrap the text as necessary
	Common::U32StringArray lines;
	font->wordWrapText(Common::U32String(pStr->getBuffer(), Common::kWindows1252), rect.width(), lines);

	// Iterate the lines to get the maximum width
	int maxWidth = 0;
	for (uint i = 0; i < lines.size(); ++i)
		maxWidth = MAX(maxWidth, font->getStringWidth(lines[i]));

	return CBofRect(0, 0, maxWidth, (int)lines.size() * font->getFontHeight());
}

} // namespace SpaceBar
} // namespace Bagel
