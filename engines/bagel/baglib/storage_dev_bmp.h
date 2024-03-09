
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

#ifndef BAGEL_BAGLIB_STORAGE_DEV_BMP_H
#define BAGEL_BAGLIB_STORAGE_DEV_BMP_H

#include "bagel/baglib/bmp_object.h"
#include "bagel/baglib/storage_dev_win.h"
#include "bagel/boflib/list.h"

namespace Bagel {

class CBagStorageDevBmp : public CBagBmpObject, public CBagStorageDev {
protected:
	INT m_nMaskColor;
	CBofPoint m_xCursorLocation; // Current cursor location in bmp.
	CBofRect m_cSrcRect;
	BOOL m_bTrans;
	CBofBitmap *m_pWorkBmp;

public:
	CBagStorageDevBmp(CBofWindow *pParent = nullptr, const CBofRect &xRect = CBofRect(), BOOL bTrans = TRUE);
	virtual ~CBagStorageDevBmp();

	CBofBitmap *GetWorkBmp() {
		return (m_pWorkBmp);
	}
	ERROR_CODE SetWorkBmp();
	ERROR_CODE KillWorkBmp();

	CBofRect GetRect() {
		return CBagStorageDev::GetRect();
	}
	VOID SetRect(const CBofRect &xRect) {
		CBagStorageDev::SetRect(xRect);
	}
	CBofPoint GetPosition() {
		return CBagStorageDev::GetPosition();
	}
	VOID SetPosition(const CBofPoint &pos) {
		CBagStorageDev::SetPosition(pos);
	}

	BOOL GetTransparent() {
		return m_bTrans;
	}
	VOID SetTransparent(BOOL bTrans = TRUE) {
		m_bTrans = bTrans;
	}

	/**
	 * Return a scaled point from the screen to the bitmap
	 */
	CBofPoint GetScaledPt(CBofPoint xPoint);

	ERROR_CODE SetBackground(CBofBitmap *pBmp);
	CBofBitmap *GetBackground() {
		return (GetBitmap());
	}

	virtual ERROR_CODE LoadFileFromStream(bof_ifstream &fpInput, const CBofString &sWldName, BOOL bAttach = TRUE);

	virtual BOOL IsAttached() {
		return CBagBmpObject::IsAttached();
	}

	virtual BOOL PaintFGObjects(CBofBitmap *) {
		return TRUE;
	}

	virtual ERROR_CODE Attach();
	virtual ERROR_CODE Detach();

	virtual ERROR_CODE Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect = nullptr, INT nMaskColor = -1);

	/**
	 * Called on the mouse left button up of the bagbmobj
	 *  and redirected to the lbutton up of the CBagStorageDev
	 */
	void OnLButtonUp(UINT nFlags, CBofPoint *xPoint, void *info);

	/**
	 * Called on the mouse left button down of the bagbmobj
	 * and redirected to the lbutton down of the CBagStorageDev
	 */
	virtual void OnLButtonDown(UINT nFlags, CPoint *xPoint, void *info = nullptr);

	virtual const CBofPoint DevPtToViewPort(const CBofPoint &xPoint);
};

} // namespace Bagel

#endif
