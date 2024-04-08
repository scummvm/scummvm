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

ErrorCode CBagStorageDevBmp::Attach() {
	m_nMaskColor = CBagel::GetBagApp()->GetChromaColor();
	return CBagStorageDev::Attach();
}


ErrorCode CBagStorageDevBmp::Detach() {
	KillWorkBmp();
	return CBagStorageDev::Detach();
}

ErrorCode CBagStorageDevBmp::SetBackground(CBofBitmap *pBmp) {
	if (pBmp != nullptr) {
		SetBitmap(pBmp);
		SetWorkBmp();
	} else {
		// Hope and pray that this is the right thing to do
		if (GetBitmap()) {
			delete GetBitmap();
		}

		SetBitmap(nullptr);
		KillWorkBmp();
	}

	return m_errCode;
}

ErrorCode CBagStorageDevBmp::SetWorkBmp() {
	// Delete any previous work area
	KillWorkBmp();

	CBofBitmap *pBmp = GetBackground();
	if (pBmp != nullptr) {
		m_pWorkBmp = new CBofBitmap(pBmp->Width(), pBmp->Height(), pBmp->GetPalette());
		pBmp->Paint(m_pWorkBmp);
	}

	return m_errCode;
}

ErrorCode CBagStorageDevBmp::KillWorkBmp() {
	if (m_pWorkBmp != nullptr) {
		delete m_pWorkBmp;
		m_pWorkBmp = nullptr;
	}

	return m_errCode;
}

ErrorCode CBagStorageDevBmp::LoadFileFromStream(bof_ifstream &fpInput, const CBofString &sWldName, bool bAttach) {
	SetFileName(sWldName);
	SetRefName(sWldName);

	return CBagStorageDev::LoadFileFromStream(fpInput, sWldName, bAttach);
}

CBofPoint CBagStorageDevBmp::GetScaledPt(CBofPoint xPoint) {
	CBofRect        SDevDstRect;
	CBofRect        SDevSrcRect;
	CBofPoint           pt;
	CBofRect        rDestRect = GetRect();

	SDevDstRect = GetRect();                // Get the destination (screen) rect
	SDevSrcRect = CBagBmpObject::GetRect(); // Get the source (origin) rect

	pt.x = m_cSrcRect.Width() * xPoint.x / rDestRect.Width();
	pt.y = m_cSrcRect.Height() * xPoint.y / rDestRect.Height();

	pt.x += m_cSrcRect.left;
	pt.y += m_cSrcRect.top;

	return pt;
}

void CBagStorageDevBmp::OnLButtonDown(uint32 nFlags, CBofPoint *xPoint, void *info) {
	CBagStorageDev::OnLButtonDown(nFlags, xPoint, info);
}

void CBagStorageDevBmp::OnLButtonUp(uint32 nFlags, CBofPoint *xPoint, void *info) {
	CBagStorageDev::OnLButtonUp(nFlags, xPoint, info);
}

const CBofPoint CBagStorageDevBmp::DevPtToViewPort(const CBofPoint &xPoint) {
	CBofPoint p;

	// Get the storage device rect
	CBofRect SDevDstRect = GetRect();

	// move point relative to storage device top, left
	p.x = xPoint.x - SDevDstRect.left;
	p.y = xPoint.y - SDevDstRect.top;

	return p;
}

ErrorCode CBagStorageDevBmp::Update(CBofBitmap *pBmp, CBofPoint /*xPoint*/, CBofRect * /*pSrcRect*/, int /*nMaskColor*/) {
	CBofBitmap *pSrcBmp = nullptr;

	// if this object is visible
	if (IsVisible() && IsAttached()) {
		// Paint the storage device
		if ((pSrcBmp = GetBitmap()) != nullptr) {
			Assert(GetWorkBmp() != nullptr);
			// Erase everything from the background
			GetWorkBmp()->Paint(pSrcBmp);

			// Paint all the objects to the background
			CBofRect r = pSrcBmp->GetRect();
			PaintStorageDevice(nullptr, pSrcBmp, &r);

			// Paint child storage devices
			PaintFGObjects(pSrcBmp);

			// Paint to screen
			if (m_bTrans)
				pSrcBmp->Paint(pBmp, GetPosition().x, GetPosition().y, nullptr, m_nMaskColor);
			else
				pSrcBmp->Paint(pBmp, GetPosition().x, GetPosition().y, nullptr, -1);
		}
	}

	// Set the firstpaint flag and attach objects to allow for immediate run objects to run
	if (m_bFirstPaint) {
		m_bFirstPaint = false;
		AttachActiveObjects();
	}

	return m_errCode;
}

} // namespace Bagel
