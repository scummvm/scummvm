
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

#ifndef BAGEL_BAGLIB_CURSOR_H
#define BAGEL_BAGLIB_CURSOR_H

#include "bagel/boflib/gfx/bitmap.h"
#include "bagel/boflib/error.h"

namespace Bagel {

class CBagCursor : public CBofObject, public CBofError {
public:
	CBagCursor();
	CBagCursor(CBofBitmap *pBmp);
	CBagCursor(const CHAR *pszFileName, BOOL bSharedPal = FALSE);
	static void initStatics() {
		m_pCurrentCursor = nullptr;
	}

	~CBagCursor();

	VOID SetHotSpot(INT x, INT y) {
		m_nX = x;
		m_nY = y;
	}
	VOID SetHotSpot(CBofPoint cPoint) { SetHotSpot(cPoint.x, cPoint.y); }
	CBofPoint GetHotSpot() { return CBofPoint(m_nX, m_nY); }

	INT GetX() { return m_nX; }
	INT GetY() { return m_nY; }

	ERROR_CODE Load() { return Load(m_szFileName); }
	ERROR_CODE Load(CBofBitmap *pBmp);
	ERROR_CODE Load(const CHAR *pszFileName, CBofPalette *pPal = nullptr);

	VOID UnLoad();

	CBofBitmap *GetImage() { return m_pBmp; }
	ERROR_CODE SetImage(CBofBitmap *pBmp) { return Load(pBmp); }

	VOID SetCurrent() { m_pCurrentCursor = this; }
	static CBagCursor *GetCurrent() { return m_pCurrentCursor; }

	/**
	 * Flag whether this is a wielded cursor or not
	 */
	VOID SetWieldCursor(BOOL b = FALSE) { m_bWieldCursor = b; }
	BOOL IsWieldCursor() const { return m_bWieldCursor; }

	VOID Show() { SetCurrent(); }
	VOID Hide() { m_pCurrentCursor = nullptr; }

private:
	static CBagCursor *m_pCurrentCursor;

	CHAR m_szFileName[MAX_FNAME];
	CBofBitmap *m_pBmp;
	INT m_nX;
	INT m_nY;
	BOOL m_bSharedPal;
	BOOL m_bWieldCursor;
};

} // namespace Bagel

#endif
