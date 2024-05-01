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

#include "bagel/baglib/storage_dev_bmp.h"
#include "bagel/baglib/bagel.h"

namespace Bagel {

CBagStorageDevBmp::CBagStorageDevBmp(CBofWindow *pParent, const CBofRect &xRect, bool bTrans)
	: CBagBmpObject() {
	m_cSrcRect.SetRect(0, 0, 0, 0);
	m_bTrans = bTrans;
	m_xSDevType = SDEV_BMP;
	SetRect(xRect);
	m_pWorkBmp = nullptr;

	SetAssociateWnd(pParent);

	SetVisible();       // This object is visible
}

CBagStorageDevBmp::~CBagStorageDevBmp() {
	// Delete any current work bmp
	KillWorkBmp();
}

ErrorCode CBagStorageDevBmp::attach() {
	m_nMaskColor = CBagel::getBagApp()->getChromaColor();
	return CBagStorageDev::attach();
}


ErrorCode CBagStorageDevBmp::detach() {
	KillWorkBmp();
	return CBagStorageDev::detach();
}

ErrorCode CBagStorageDevBmp::setBackground(CBofBitmap *pBmp) {
	if (pBmp != nullptr) {
		setBitmap(pBmp);
		SetWorkBmp();
	} else {
		// Hope and pray that this is the right thing to do
		if (getBitmap()) {
			delete getBitmap();
		}

		setBitmap(nullptr);
		KillWorkBmp();
	}

	return _errCode;
}

ErrorCode CBagStorageDevBmp::SetWorkBmp() {
	// Delete any previous work area
	KillWorkBmp();

	CBofBitmap *pBmp = getBackground();
	if (pBmp != nullptr) {
		m_pWorkBmp = new CBofBitmap(pBmp->width(), pBmp->height(), pBmp->getPalette());
		pBmp->paint(m_pWorkBmp);
	}

	return _errCode;
}

ErrorCode CBagStorageDevBmp::KillWorkBmp() {
	if (m_pWorkBmp != nullptr) {
		delete m_pWorkBmp;
		m_pWorkBmp = nullptr;
	}

	return _errCode;
}

ErrorCode CBagStorageDevBmp::LoadFileFromStream(CBagIfstream &fpInput, const CBofString &sWldName, bool bAttach) {
	SetFileName(sWldName);
	SetRefName(sWldName);

	return CBagStorageDev::LoadFileFromStream(fpInput, sWldName, bAttach);
}

CBofPoint CBagStorageDevBmp::GetScaledPt(CBofPoint xPoint) {
	CBofRect        SDevDstRect;
	CBofRect        SDevSrcRect;
	CBofPoint           pt;
	CBofRect        rDestRect = getRect();

	SDevDstRect = getRect();                // Get the destination (screen) rect
	SDevSrcRect = CBagBmpObject::getRect(); // Get the source (origin) rect

	pt.x = m_cSrcRect.width() * xPoint.x / rDestRect.width();
	pt.y = m_cSrcRect.height() * xPoint.y / rDestRect.height();

	pt.x += m_cSrcRect.left;
	pt.y += m_cSrcRect.top;

	return pt;
}

void CBagStorageDevBmp::onLButtonDown(uint32 nFlags, CBofPoint *xPoint, void *info) {
	CBagStorageDev::onLButtonDown(nFlags, xPoint, info);
}

void CBagStorageDevBmp::onLButtonUp(uint32 nFlags, CBofPoint *xPoint, void *info) {
	CBagStorageDev::onLButtonUp(nFlags, xPoint, info);
}

const CBofPoint CBagStorageDevBmp::DevPtToViewPort(const CBofPoint &xPoint) {
	CBofPoint p;

	// Get the storage device rect
	CBofRect SDevDstRect = getRect();

	// move point relative to storage device top, left
	p.x = xPoint.x - SDevDstRect.left;
	p.y = xPoint.y - SDevDstRect.top;

	return p;
}

ErrorCode CBagStorageDevBmp::update(CBofBitmap *pBmp, CBofPoint /*xPoint*/, CBofRect * /*pSrcRect*/, int /*nMaskColor*/) {
	CBofBitmap *pSrcBmp = nullptr;

	// if this object is visible
	if (isVisible() && isAttached()) {
		// Paint the storage device
		if ((pSrcBmp = getBitmap()) != nullptr) {
			Assert(GetWorkBmp() != nullptr);
			// Erase everything from the background
			GetWorkBmp()->paint(pSrcBmp);

			// Paint all the objects to the background
			CBofRect r = pSrcBmp->getRect();
			PaintStorageDevice(nullptr, pSrcBmp, &r);

			// Paint child storage devices
			PaintFGObjects(pSrcBmp);

			// Paint to screen
			if (m_bTrans)
				pSrcBmp->paint(pBmp, getPosition().x, getPosition().y, nullptr, m_nMaskColor);
			else
				pSrcBmp->paint(pBmp, getPosition().x, getPosition().y, nullptr, -1);
		}
	}

	// Set the firstpaint flag and attach objects to allow for immediate run objects to run
	if (_bFirstPaint) {
		_bFirstPaint = false;
		AttachActiveObjects();
	}

	return _errCode;
}

} // namespace Bagel
