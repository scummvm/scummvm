
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

namespace Bagel {

class CBagStorageDevBmp : public CBagBmpObject, public CBagStorageDev {
protected:
	int m_nMaskColor;
	CBofPoint m_xCursorLocation; // Current cursor location in bmp.
	CBofRect m_cSrcRect;
	bool m_bTrans;
	CBofBitmap *m_pWorkBmp;

public:
	CBagStorageDevBmp(CBofWindow *pParent = nullptr, const CBofRect &xRect = CBofRect(), bool bTrans = true);
	virtual ~CBagStorageDevBmp();

	CBofBitmap *GetWorkBmp() {
		return m_pWorkBmp;
	}
	ErrorCode SetWorkBmp();
	ErrorCode KillWorkBmp();

	CBofRect getRect() {
		return CBagStorageDev::getRect();
	}
	void SetRect(const CBofRect &xRect) {
		CBagStorageDev::SetRect(xRect);
	}
	CBofPoint getPosition() {
		return CBagStorageDev::getPosition();
	}
	void setPosition(const CBofPoint &pos) {
		CBagStorageDev::setPosition(pos);
	}

	bool GetTransparent() {
		return m_bTrans;
	}
	void SetTransparent(bool bTrans = true) {
		m_bTrans = bTrans;
	}

	/**
	 * Return a scaled point from the screen to the bitmap
	 */
	CBofPoint GetScaledPt(CBofPoint xPoint);

	ErrorCode setBackground(CBofBitmap *pBmp);
	CBofBitmap *getBackground() {
		return getBitmap();
	}

	virtual ErrorCode LoadFileFromStream(CBagIfstream &fpInput, const CBofString &sWldName, bool bAttach = true);

	virtual bool isAttached() {
		return CBagBmpObject::isAttached();
	}

	virtual bool PaintFGObjects(CBofBitmap *) {
		return true;
	}

	virtual ErrorCode attach();
	virtual ErrorCode detach();

	virtual ErrorCode update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect = nullptr, int nMaskColor = -1);

	/**
	 * Called on the mouse left button up of the bagbmobj
	 *  and redirected to the lbutton up of the CBagStorageDev
	 */
	void onLButtonUp(uint32 nFlags, CBofPoint *xPoint, void *info);

	/**
	 * Called on the mouse left button down of the bagbmobj
	 * and redirected to the lbutton down of the CBagStorageDev
	 */
	virtual void onLButtonDown(uint32 nFlags, CPoint *xPoint, void *info = nullptr);

	virtual const CBofPoint DevPtToViewPort(const CBofPoint &xPoint);
};

} // namespace Bagel

#endif
