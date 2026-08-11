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

#include "bagel/spacebar/baglib/storage_dev_bmp.h"
#include "bagel/spacebar/baglib/bagel.h"

namespace Bagel {
namespace SpaceBar {

CBagStorageDevBmp::CBagStorageDevBmp(CBofWindow *pParent, const CBofRect &xRect, bool bTrans)
	: CBagBmpObject() {
	_cSrcRect.setRect(0, 0, 0, 0);
	_bTrans = bTrans;
	_xSDevType = SDEV_BMP;
	setRect(xRect);
	_pWorkBmp = nullptr;
	_nMaskColor = CBagel::getBagApp()->getChromaColor();

	CBagStorageDev::setAssociateWnd(pParent);

	setVisible();       // This object is visible
}

CBagStorageDevBmp::~CBagStorageDevBmp() {
	// Delete any current work bmp
	killWorkBmp();
}

ErrorCode CBagStorageDevBmp::attach() {
	_nMaskColor = CBagel::getBagApp()->getChromaColor();
	return CBagStorageDev::attach();
}


ErrorCode CBagStorageDevBmp::detach() {
	killWorkBmp();
	return CBagStorageDev::detach();
}

ErrorCode CBagStorageDevBmp::setBackground(CBofBitmap *pBmp) {
	if (pBmp != nullptr) {
		setBitmap(pBmp);
		setWorkBmp();
	} else {
		// Hope and pray that this is the right thing to do
		if (getBitmap()) {
			delete getBitmap();
		}

		setBitmap(nullptr);
		killWorkBmp();
	}

	return _errCode;
}

ErrorCode CBagStorageDevBmp::setWorkBmp() {
	// Delete any previous work area
	killWorkBmp();

	CBofBitmap *pBmp = getBackground();
	if (pBmp != nullptr) {
		_pWorkBmp = new CBofBitmap(pBmp->width(), pBmp->height(), pBmp->getPalette());
		pBmp->paint(_pWorkBmp);
	}

	return _errCode;
}

ErrorCode CBagStorageDevBmp::killWorkBmp() {
	delete _pWorkBmp;
	_pWorkBmp = nullptr;

	return _errCode;
}

ErrorCode CBagStorageDevBmp::loadFileFromStream(CBagIfstream &fpInput, const CBofString &sWldName, bool bAttach) {
	setFileName(sWldName);
	setRefName(sWldName);

	return CBagStorageDev::loadFileFromStream(fpInput, sWldName, bAttach);
}

void CBagStorageDevBmp::onLButtonDown(uint32 nFlags, CBofPoint *xPoint, void *info) {
	CBagStorageDev::onLButtonDown(nFlags, xPoint, info);
}

void CBagStorageDevBmp::onLButtonUp(uint32 nFlags, CBofPoint *xPoint, void *info) {
	CBagStorageDev::onLButtonUp(nFlags, xPoint, info);
}

const CBofPoint CBagStorageDevBmp::devPtToViewPort(const CBofPoint &xPoint) {
	// Get the storage device rect
	CBofRect SDevDstRect = getRect();

	// move point relative to storage device top, left
	CBofPoint p;
	p.x = xPoint.x - SDevDstRect.left;
	p.y = xPoint.y - SDevDstRect.top;

	return p;
}

ErrorCode CBagStorageDevBmp::update(CBofBitmap *pBmp, CBofPoint /*xPoint*/, CBofRect * /*pSrcRect*/, int /*nMaskColor*/) {
	// if this object is visible
	if (isVisible() && isAttached()) {
		// Paint the storage device
		CBofBitmap *pSrcBmp = getBitmap();
		if (pSrcBmp != nullptr) {
			assert(getWorkBmp() != nullptr);
			// Erase everything from the background
			getWorkBmp()->paint(pSrcBmp);

			// Paint all the objects to the background
			CBofRect r = pSrcBmp->getRect();
			paintStorageDevice(nullptr, pSrcBmp, &r);

			// Paint child storage devices
			paintFGObjects(pSrcBmp);

			// Paint to screen
			if (_bTrans)
				pSrcBmp->paint(pBmp, getPosition().x, getPosition().y, nullptr, _nMaskColor);
			else
				pSrcBmp->paint(pBmp, getPosition().x, getPosition().y, nullptr, -1);
		}
	}

	// Set the firstpaint flag and attach objects to allow for immediate run objects to run
	if (_bFirstPaint) {
		_bFirstPaint = false;
		attachActiveObjects();
	}

	return _errCode;
}

} // namespace SpaceBar
} // namespace Bagel
