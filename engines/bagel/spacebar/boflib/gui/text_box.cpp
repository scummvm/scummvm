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

#include "bagel/spacebar/boflib/gui/text_box.h"

namespace Bagel {
namespace SpaceBar {

#define DEFAULT_PAGE_SIZE   10
#define DEFAULT_WEIGHT      TEXT_NORMAL
#define DEFAULT_POINT_SIZE  FONT_12POINT
#define DEFAULT_COLOR       CTEXT_COLOR


CBofTextBox::CBofTextBox() {
	// Inits
	_pDestWindow = nullptr;
	_pDestBitmap = nullptr;
	_pTextField = nullptr;
	_nWeight = DEFAULT_WEIGHT;
	_nPointSize = DEFAULT_POINT_SIZE;
	_cTextColor = DEFAULT_COLOR;
	_nTextFont = FONT_DEFAULT;
	_nPageSize = DEFAULT_PAGE_SIZE;
	_nCurrentLine = 0;
	_nCurrentIndex = 0;
	_nNumLines = 0;
}


CBofTextBox::CBofTextBox(CBofWindow *pWindow, const CBofRect *pRect, const CBofString &cText) {
	assert(pWindow != nullptr);
	assert(pRect != nullptr);

	// Inits
	_pDestWindow = nullptr;
	_pDestBitmap = nullptr;
	_pTextField = nullptr;
	_nWeight = DEFAULT_WEIGHT;
	_nPointSize = DEFAULT_POINT_SIZE;
	_cTextColor = DEFAULT_COLOR;
	_nPageSize = DEFAULT_PAGE_SIZE;
	_nCurrentLine = 0;
	_nCurrentIndex = 0;
	_nNumLines = 0;
	_nTextFont = FONT_DEFAULT;

	setText(cText);
	setBox(pRect);
	setDisplay(pWindow);
}


CBofTextBox::CBofTextBox(CBofBitmap *pBitmap, const CBofRect *pRect, const CBofString &cText) {
	assert(pBitmap != nullptr);
	assert(pRect != nullptr);

	// Inits
	_pDestWindow = nullptr;
	_pDestBitmap = nullptr;
	_pTextField = nullptr;
	_nWeight = DEFAULT_WEIGHT;
	_nPointSize = DEFAULT_POINT_SIZE;
	_cTextColor = DEFAULT_COLOR;
	_nPageSize = DEFAULT_PAGE_SIZE;
	_nCurrentLine = 0;
	_nCurrentIndex = 0;
	_nNumLines = 0;
	_nTextFont = FONT_DEFAULT;

	setText(cText);
	setBox(pRect);
	setDisplay(pBitmap);
}


CBofTextBox::~CBofTextBox() {
	assert(isValidObject(this));

	if (_pTextField != nullptr) {
		delete _pTextField;
		_pTextField = nullptr;
	}
	_pDestWindow = nullptr;
	_pDestBitmap = nullptr;
}


ErrorCode CBofTextBox::setBox(const CBofRect *pRect) {
	assert(isValidObject(this));
	assert(pRect != nullptr);

	// Remove previous text field (if any)
	delete _pTextField;
	_pTextField = nullptr;

	// Create a new text field the size of the box we want
	_pTextField = new CBofText(pRect, JUSTIFY_WRAP);

	return _errCode;
}


void CBofTextBox::setDisplay(CBofWindow *pWindow) {
	assert(isValidObject(this));
	assert(pWindow != nullptr);

	_pDestWindow = pWindow;
	_pDestBitmap = nullptr;
}


void CBofTextBox::setDisplay(CBofBitmap *pBitmap) {
	assert(isValidObject(this));
	assert(pBitmap != nullptr);

	_pDestBitmap = pBitmap;
	_pDestWindow = nullptr;
}


void CBofTextBox::setTextAttribs(const int nSize, const int nWeight, const COLORREF cColor, const int nFont) {
	assert(isValidObject(this));

	_nPointSize = nSize;
	_nWeight = nWeight;
	_cTextColor = cColor;
	_nTextFont = nFont;
}


void CBofTextBox::setText(const CBofString &cString) {
	_cBuffer = cString;
	assert(_cBuffer.getLength() != 0);

	_cBuffer.replaceStr("\r\n", "\n");
	_cBuffer.replaceStr("\r", "\n");

	_nCurrentLine = 0;
	_nCurrentIndex = 0;
	_nNumLines = _cBuffer.findNumOccurrences("\n");
}

int CBofTextBox::getIndex(const int nLine) {
	assert(nLine >= 0 && nLine <= _nNumLines);

	// Find the index into our buffer that represents the top left of the
	// buffer that is nLine from current the beginning of the buffer.
	const char *pszCur, *pszBuffer;
	const char *pszLast = pszCur = pszBuffer = _cBuffer;
	for (int i = 0; i < nLine; i++) {
		pszLast = pszCur;
		pszCur = strstr(pszCur, "\n");

		// Make sure we don't go too far (nLines is invalid)
		assert(pszCur != nullptr);

		pszCur++;
	}
	int nChars = pszCur - pszBuffer;
	if (nLine == _nNumLines) {
		nChars = pszLast - pszBuffer;
		_nCurrentLine--;
	}

	return nChars;
}


ErrorCode CBofTextBox::scrollUp(const int nLines) {
	// Make scroll a no-op if all the lines in the box appear on one screen.
	if (_nNumLines <= _nPageSize) {
		return scrollTo(_nCurrentLine);
	}

	int nNewLine = _nCurrentLine - nLines;

	if (nNewLine < 0) {
		nNewLine = 0;
	} else if (nNewLine > (_nNumLines - _nPageSize)) {
		// If the line requested to be the top of the page
		// would cause fewer than _nPageSize lines to be displayed,
		// snap nNewLine to be the top of the last full page.
		//
		nNewLine = (_nNumLines - _nPageSize);
	}
	return scrollTo(nNewLine);
}


ErrorCode CBofTextBox::scrollTo(const int nLine) {
	assert(isValidObject(this));
	assert(nLine >= 0 && nLine <= _nNumLines);

	_nCurrentIndex = getIndex(nLine);
	_nCurrentLine = nLine;

	// Show the text box
	display();

	return _errCode;
}


ErrorCode CBofTextBox::display() {
	assert(isValidObject(this));
	assert(_nCurrentLine >= 0 && _nCurrentLine <= _nNumLines);
	assert(_nCurrentIndex >= 0 && _nCurrentIndex < _cBuffer.getLength());

	// The actual text box must have been created before it can be displayed
	assert(_pTextField != nullptr);

	// If painting to a window
	if (_pDestWindow != nullptr) {
		_pTextField->display(_pDestWindow, _cBuffer.mid(_nCurrentIndex), _nPointSize, _nWeight, _cTextColor, _nTextFont);

	} else {
		// Otherwise, must be painting to a bitmap
		assert(_pDestBitmap != nullptr);

		_pTextField->display(_pDestBitmap, _cBuffer.mid(_nCurrentIndex), _nPointSize, _nWeight, _cTextColor, _nTextFont);
	}

	return _errCode;
}


ErrorCode CBofTextBox::erase() {
	assert(isValidObject(this));

	// The actual text box must have been created before it can be displayed
	assert(_pTextField != nullptr);

	if (_pDestWindow != nullptr) {
		_errCode = _pTextField->erase(_pDestWindow);

	} else {
		assert(_pDestBitmap != nullptr);
		_errCode = _pTextField->erase(_pDestBitmap);
	}

	return _errCode;
}


void CBofTextBox::flushBackground() {
	assert(isValidObject(this));

	if (_pTextField != nullptr) {
		_pTextField->flushBackground();
	}
}

} // namespace SpaceBar
} // namespace Bagel
