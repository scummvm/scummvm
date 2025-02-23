
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

#include "bagel/spacebar/baglib/bmp_object.h"
#include "bagel/spacebar/baglib/storage_dev_win.h"

namespace Bagel {
namespace SpaceBar {

class CBagStorageDevBmp : public CBagBmpObject, public CBagStorageDev {
protected:
	int _nMaskColor;
	CBofPoint _xCursorLocation; // Current cursor location in bmp.
	CBofRect _cSrcRect;
	bool _bTrans;
	CBofBitmap *_pWorkBmp;

public:
	CBagStorageDevBmp(CBofWindow *pParent = nullptr, const CBofRect &xRect = CBofRect(), bool bTrans = true);
	virtual ~CBagStorageDevBmp();

	CBofBitmap *getWorkBmp() const {
		return _pWorkBmp;
	}
	ErrorCode setWorkBmp();
	ErrorCode killWorkBmp();

	CBofRect getRect() override {
		return CBagStorageDev::getRect();
	}
	void setRect(const CBofRect &xRect) override {
		CBagStorageDev::setRect(xRect);
	}
	CBofPoint getPosition() override {
		return CBagStorageDev::getPosition();
	}
	void setPosition(const CBofPoint &pos) override {
		CBagStorageDev::setPosition(pos);
	}

	bool getTransparent() const {
		return _bTrans;
	}
	void setTransparent(bool bTrans = true) override {
		_bTrans = bTrans;
	}

	ErrorCode setBackground(CBofBitmap *pBmp) override;
	CBofBitmap *getBackground() override {
		return getBitmap();
	}

	ErrorCode loadFileFromStream(CBagIfstream &fpInput, const CBofString &sWldName, bool bAttach = true) override;

	bool isAttached() override {
		return CBagBmpObject::isAttached();
	}

	virtual bool paintFGObjects(CBofBitmap *) {
		return true;
	}

	ErrorCode attach() override;
	ErrorCode detach() override;

	ErrorCode update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect = nullptr, int nMaskColor = -1) override;

	/**
	 * Called on the mouse left button up of the bagbmobj
	 *  and redirected to the lbutton up of the CBagStorageDev
	 */
	void onLButtonUp(uint32 nFlags, CBofPoint *xPoint, void *info) override;

	/**
	 * Called on the mouse left button down of the bagbmobj
	 * and redirected to the lbutton down of the CBagStorageDev
	 */
	void onLButtonDown(uint32 nFlags, CBofPoint *xPoint, void *info = nullptr) override;

	const CBofPoint devPtToViewPort(const CBofPoint &xPoint) override;
};

} // namespace SpaceBar
} // namespace Bagel

#endif
