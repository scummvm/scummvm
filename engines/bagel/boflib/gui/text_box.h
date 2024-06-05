
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

#ifndef BAGEL_BOFLIB_GUI_TEXT_BOX_H
#define BAGEL_BOFLIB_GUI_TEXT_BOX_H

#include "bagel/boflib/gfx/bitmap.h"
#include "bagel/boflib/error.h"
#include "bagel/boflib/string.h"
#include "bagel/boflib/gfx/text.h"
#include "bagel/boflib/gui/window.h"

namespace Bagel {

class CBofTextBox : public CBofObject, public CBofError {
public:
	CBofTextBox();
	CBofTextBox(CBofWindow *pWindow, const CBofRect *pRect, const CBofString &cText);
	CBofTextBox(CBofBitmap *pBitmap, const CBofRect *pRect, const CBofString &cText);
	virtual ~CBofTextBox();

	void setText(const CBofString &cText);
	ErrorCode setBox(const CBofRect *pRect);
	void setDisplay(CBofWindow *pWindow);
	void setDisplay(CBofBitmap *pBitmap);

	void setTextAttribs(int nSize, int nWeight, RGBCOLOR cColor = CTEXT_COLOR, int nFont = FONT_DEFAULT);

	void setPointSize(const int nSize) {
		_nPointSize = nSize;
	}
	int getPointSize() const {
		return _nPointSize;
	}

	void setWeight(const int nWeight) {
		_nWeight = nWeight;
	}
	int getWeight() const {
		return _nWeight;
	}

	void setPageLength(const int nSize) {
		_nPageSize = nSize;
	}
	int getPageLength() const {
		return _nPageSize;
	}

	void setColor(const RGBCOLOR cColor) {
		_cTextColor = cColor;
	}
	RGBCOLOR getColor() const {
		return _cTextColor;
	}

	void setFont(int nFont) {
		_nTextFont = nFont;
	}
	int getFont() const {
		return _nTextFont;
	}

	ErrorCode lineUp() {
		return scrollUp(1);
	}
	ErrorCode lineDown() {
		return scrollDown(1);
	}

	ErrorCode pageUp() {
		return scrollUp(_nPageSize);
	}
	ErrorCode pageDown() {
		return scrollDown(_nPageSize);
	}

	ErrorCode scrollUp(int nLines);
	ErrorCode scrollDown(const int nLines) {
		return scrollUp(-nLines);
	}

	ErrorCode scrollTo(int nLine);

	ErrorCode display();
	ErrorCode erase();

	void flushBackground();

	int getCurrLine() {
		return _nCurrentLine;
	}
	ErrorCode setCurrLine(const int nLine) {
		return scrollTo(nLine);
	}

protected:
	int getIndex(int nLines);

	// Data
	CBofString _cBuffer;
	CBofText *_pTextField;
	CBofWindow *_pDestWindow;
	CBofBitmap *_pDestBitmap;

	int _nCurrentLine;
	int _nCurrentIndex;
	int _nNumLines;
	int _nPageSize;

	RGBCOLOR _cTextColor;
	int _nPointSize;
	int _nWeight;
	int _nTextFont;
};

} // namespace Bagel

#endif
