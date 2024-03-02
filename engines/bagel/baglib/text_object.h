
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

	CBofRect GetRect(VOID);

	CBofSize GetSize() { return (CBofSize(m_nDX, m_nDY)); }
	VOID SetSize(const CBofSize &xSize) {
		m_nDX = (SHORT)xSize.cx;
		m_nDY = (SHORT)xSize.cy;
	}

	INT GetPointSize() { return m_nPointSize; }
	VOID SetPointSize(INT xSize) {
		Assert(xSize >= 0 && xSize <= 255);
		m_nPointSize = (UBYTE)xSize;
	}
	INT GetColor() { return 0; }
	VOID SetColor(int nColor);

	// jwl 10.19.96 add font support for tx objects
	INT GetFont() { return m_nTextFont; }
	VOID SetFont(INT nFont) { m_nTextFont = nFont; }

	virtual BOOL RunObject();

	const CBofString *GetInitInfo() { return m_psInitInfo; }
	VOID SetInitInfo(const CBofString &info) {
		if (m_psInitInfo)
			delete m_psInitInfo;
		m_psInitInfo = new CBofString();
		*m_psInitInfo = info;
	}

	// jwl 11.09.96 set the ps text field.
	CBofString *GetPSText() { return (m_psText); }
	VOID SetPSText(CBofString *p);

	const CBofString &GetText();       //{ return GetFileName();}
	VOID SetText(const CBofString &s); //{ SetFileName(s);}

	// jwl 11.13.96
	VOID RecalcTextRect(BOOL);

	virtual ERROR_CODE Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect * /*pSrcRect*/ = NULL, INT /*nMaskColor*/ = -1);

	ERROR_CODE Attach();
	ERROR_CODE Detach();

	PARSE_CODES SetInfo(bof_ifstream &istr);

	VOID SetProperty(const CBofString &sProp, int nVal);
	INT GetProperty(const CBofString &sProp);

	BOOL IsCaption() { return (m_bCaption); }

	// jwl 11.22.96 gotta get at that title!
	VOID SetTitle(BOOL b = TRUE) { m_bTitle = b; }
	BOOL IsTitle() { return (m_bTitle); }

	// jwl 11.11.96 silly extra stuff to handle mouse downs on floaters
	// in the log pda/residue printing code.

	virtual BOOL OnLButtonUp(UINT, CBofPoint, void * = NULL);

	VOID SetRPObject(CBagObject *prp) { m_pRPObject = prp; }
	CBagObject *GetRPObject(VOID) { return m_pRPObject; }
};

} // namespace Bagel

#endif
