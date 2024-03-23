
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

#ifndef BAGEL_BAGLIB_TEXT_OBJECT_H
#define BAGEL_BAGLIB_TEXT_OBJECT_H

#include "bagel/baglib/object.h"
#include "bagel/boflib/gfx/text.h"

namespace Bagel {

// local prototypes...
INT MapFont(INT nFont);

/**
 * CBagTextObject is an object that can be place within the slide window.
 */
class CBagTextObject : public CBagObject {
private:
	CBofString *m_psInitInfo; // Aux info

	RGBCOLOR m_nFGColor;

	SHORT m_nDX;
	SHORT m_nDY;

	UBYTE m_nPointSize;
	INT m_nTextFont;

	CBagObject *m_pRPObject;

protected:
	BOOL m_bCaption : 1;
	BOOL m_bTitle : 1;
	BOOL m_bReAttach : 1;
	CBofString *m_psText;

public:
	CBagTextObject();
	virtual ~CBagTextObject();

	CBofRect GetRect();

	CBofSize GetSize() const {
		return CBofSize(m_nDX, m_nDY);
	}
	VOID SetSize(const CBofSize &xSize) {
		m_nDX = (SHORT)xSize.cx;
		m_nDY = (SHORT)xSize.cy;
	}

	INT GetPointSize() const {
		return m_nPointSize;
	}
	VOID SetPointSize(INT xSize) {
		Assert(xSize >= 0 && xSize <= 255);
		m_nPointSize = (UBYTE)xSize;
	}
	INT GetColor() const {
		return 0;
	}
	VOID SetColor(int nColor);

	// Font support for text objects
	INT GetFont() const {
		return m_nTextFont;
	}
	VOID SetFont(INT nFont) {
		m_nTextFont = nFont;
	}

	virtual BOOL RunObject();

	const CBofString *GetInitInfo() const {
		return m_psInitInfo;
	}
	VOID SetInitInfo(const CBofString &info) {
		if (m_psInitInfo)
			delete m_psInitInfo;
		m_psInitInfo = new CBofString();
		*m_psInitInfo = info;
	}

	CBofString *GetPSText() const {
		return m_psText;
	}
	VOID SetPSText(CBofString *p);

	const CBofString &GetText();
	VOID SetText(const CBofString &s);

	/**
	 * Calculate the required bounds to display text
	 */
	VOID RecalcTextRect(BOOL bTextFromFile);

	virtual ERROR_CODE Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect * /*pSrcRect*/ = nullptr, INT /*nMaskColor*/ = -1);

	ERROR_CODE Attach();
	ERROR_CODE Detach();

	PARSE_CODES SetInfo(bof_ifstream &istr);

	VOID SetProperty(const CBofString &sProp, int nVal);
	INT GetProperty(const CBofString &sProp);

	BOOL IsCaption() const {
		return m_bCaption;
	}

	VOID SetTitle(BOOL b = TRUE) {
		m_bTitle = b;
	}
	BOOL IsTitle() const {
		return m_bTitle;
	}

	// Wxtra stuff to handle mouse downs on floaters in the log pda/residue printing code.
	virtual void OnLButtonUp(UINT, CBofPoint *, void * = nullptr);

	VOID SetRPObject(CBagObject *prp) {
		m_pRPObject = prp;
	}
	CBagObject *GetRPObject() const {
		return m_pRPObject;
	}
};

} // namespace Bagel

#endif
