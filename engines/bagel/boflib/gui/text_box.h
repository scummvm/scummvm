
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

	void setTextAttribs(const int nSize, const int nWeight, const RGBCOLOR cColor = CTEXT_COLOR, const int nFont = FONT_DEFAULT);

	void setPointSize(const int nSize) {
		m_nPointSize = nSize;
	}
	int getPointSize() {
		return m_nPointSize;
	}

	void setWeight(const int nWeight) {
		m_nWeight = nWeight;
	}
	int getWeight() {
		return m_nWeight;
	}

	void setPageLength(const int nSize) {
		m_nPageSize = nSize;
	}
	int getPageLength() {
		return m_nPageSize;
	}

	void setColor(const RGBCOLOR cColor) {
		m_cTextColor = cColor;
	}
	RGBCOLOR getColor() {
		return m_cTextColor;
	}

	void setFont(int nFont) {
		m_nTextFont = nFont;
	}
	int getFont() {
		return m_nTextFont;
	}

	ErrorCode lineUp() {
		return scrollUp(1);
	}
	ErrorCode lineDown() {
		return scrollDown(1);
	}

	ErrorCode pageUp() {
		return scrollUp(m_nPageSize);
	}
	ErrorCode pageDown() {
		return scrollDown(m_nPageSize);
	}

	ErrorCode scrollUp(const int nLines);
	ErrorCode scrollDown(const int nLines) {
		return scrollUp(-nLines);
	}

	ErrorCode scrollTo(const int nLine);

	ErrorCode display();
	ErrorCode erase();

	void flushBackground();

	int getCurrLine() {
		return m_nCurrentLine;
	}
	ErrorCode setCurrLine(const int nLine) {
		return scrollTo(nLine);
	}

protected:
	int getIndex(const int nLines);

	// Data
	CBofString m_cBuffer;
	CBofText *m_pTextField;
	CBofWindow *m_pDestWindow;
	CBofBitmap *m_pDestBitmap;

	int m_nCurrentLine;
	int m_nCurrentIndex;
	int m_nNumLines;
	int m_nPageSize;

	RGBCOLOR m_cTextColor;
	int m_nPointSize;
	int m_nWeight;
	int m_nTextFont;
};

} // namespace Bagel

#endif
