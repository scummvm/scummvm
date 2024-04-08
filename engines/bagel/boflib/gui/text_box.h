
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
	ERROR_CODE SetBox(const CBofRect *pRect);
	void SetDisplay(CBofWindow *pWindow);
	void SetDisplay(CBofBitmap *pBitmap);

	void SetTextAttribs(const INT nSize, const INT nWeight, const RGBCOLOR cColor = CTEXT_COLOR, const INT nFont = FONT_DEFAULT);

	void SetPointSize(const INT nSize) {
		m_nPointSize = nSize;
	}
	INT GetPointSize() {
		return (m_nPointSize);
	}

	void SetWeight(const INT nWeight) {
		m_nWeight = nWeight;
	}
	INT GetWeight() {
		return (m_nWeight);
	}

	void SetPageLength(const INT nSize) {
		m_nPageSize = nSize;
	}
	INT GetPageLength() {
		return (m_nPageSize);
	}

	void SetColor(const RGBCOLOR cColor) {
		m_cTextColor = cColor;
	}
	RGBCOLOR GetColor() {
		return (m_cTextColor);
	}

	void SetFont(INT nFont) {
		m_nTextFont = nFont;
	}
	INT GetFont() {
		return m_nTextFont;
	}

	ERROR_CODE LineUp() {
		return (ScrollUp(1));
	}
	ERROR_CODE LineDown() {
		return (ScrollDown(1));
	}

	ERROR_CODE PageUp() {
		return (ScrollUp(m_nPageSize));
	}
	ERROR_CODE PageDown() {
		return (ScrollDown(m_nPageSize));
	}

	ERROR_CODE ScrollUp(const INT nLines);
	ERROR_CODE ScrollDown(const INT nLines) {
		return (ScrollUp(-nLines));
	}

	ERROR_CODE ScrollTo(const INT nLine);

	ERROR_CODE Display();
	ERROR_CODE Erase();

	void FlushBackground();

	INT GetCurrLine() {
		return (m_nCurrentLine);
	}
	ERROR_CODE SetCurrLine(const INT nLine) {
		return (ScrollTo(nLine));
	}

protected:
	INT GetIndex(const INT nLines);

	// Data
	CBofString m_cBuffer;
	CBofText *m_pTextField;
	CBofWindow *m_pDestWindow;
	CBofBitmap *m_pDestBitmap;

	INT m_nCurrentLine;
	INT m_nCurrentIndex;
	INT m_nNumLines;
	INT m_nPageSize;

	RGBCOLOR m_cTextColor;
	INT m_nPointSize;
	INT m_nWeight;
	INT m_nTextFont;
};

} // namespace Bagel

#endif
