
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

	void SetText(const CBofString &cText);
	ErrorCode SetBox(const CBofRect *pRect);
	void SetDisplay(CBofWindow *pWindow);
	void SetDisplay(CBofBitmap *pBitmap);

	void SetTextAttribs(const int nSize, const int nWeight, const RGBCOLOR cColor = CTEXT_COLOR, const int nFont = FONT_DEFAULT);

	void SetPointSize(const int nSize) {
		m_nPointSize = nSize;
	}
	int GetPointSize() {
		return (m_nPointSize);
	}

	void SetWeight(const int nWeight) {
		m_nWeight = nWeight;
	}
	int GetWeight() {
		return (m_nWeight);
	}

	void SetPageLength(const int nSize) {
		m_nPageSize = nSize;
	}
	int GetPageLength() {
		return (m_nPageSize);
	}

	void SetColor(const RGBCOLOR cColor) {
		m_cTextColor = cColor;
	}
	RGBCOLOR GetColor() {
		return (m_cTextColor);
	}

	void SetFont(int nFont) {
		m_nTextFont = nFont;
	}
	int GetFont() {
		return m_nTextFont;
	}

	ErrorCode LineUp() {
		return (ScrollUp(1));
	}
	ErrorCode LineDown() {
		return (ScrollDown(1));
	}

	ErrorCode PageUp() {
		return (ScrollUp(m_nPageSize));
	}
	ErrorCode PageDown() {
		return (ScrollDown(m_nPageSize));
	}

	ErrorCode ScrollUp(const int nLines);
	ErrorCode ScrollDown(const int nLines) {
		return (ScrollUp(-nLines));
	}

	ErrorCode ScrollTo(const int nLine);

	ErrorCode Display();
	ErrorCode Erase();

	void FlushBackground();

	int GetCurrLine() {
		return (m_nCurrentLine);
	}
	ErrorCode SetCurrLine(const int nLine) {
		return (ScrollTo(nLine));
	}

protected:
	int GetIndex(const int nLines);

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
