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

#include "bagel/baglib/bmp_object.h"
#include "bagel/boflib/app.h"

namespace Bagel {

CBagBmpObject::CBagBmpObject() : CBagObject() {
	m_xObjType = BMPOBJ;
	m_xBmp = nullptr;
	SetOverCursor(1);
	SetTimeless(TRUE);
}

CBagBmpObject::~CBagBmpObject() {
	Detach();
}

ERROR_CODE CBagBmpObject::Attach(CBofPalette *pPalette) {
	if ((m_xBmp = new CBofBitmap(GetFileName(), pPalette)) == nullptr) {
		BofMessageBox(m_xBmp->GetFileName(), __FILE__);
	}
	return CBagObject::Attach();
}

ERROR_CODE CBagBmpObject::Detach() {
	if (m_xBmp) {
		delete m_xBmp;
		m_xBmp = nullptr;
	}
	return CBagObject::Detach();
}

CBofRect
CBagBmpObject::GetRect() {
	CBofPoint p = GetPosition();
	CBofSize s;
	if (m_xBmp)
		s = m_xBmp->GetSize();
	return CBofRect(p, s);
}

ERROR_CODE
CBagBmpObject::Update(CBofWindow *pWnd, CBofPoint pt, CBofRect *pSrcRect, INT nMaskColor) {
	if (m_xBmp) {
		m_nTrans = nMaskColor;
		return m_xBmp->Paint(pWnd, pt.x, pt.y, pSrcRect, nMaskColor);
	} else
		return ERR_NONE;
}

ERROR_CODE
CBagBmpObject::Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect, INT nMaskColor) {
	if (pBmp) { // && IsAttached() && IsVisible()
		m_nTrans = nMaskColor;
		if (pSrcRect) {
			CBofSize s = pBmp->GetSize();
			if (pt.x < 0) {
				pSrcRect->left -= pt.x;
				pt.x = 0;
			}
			if (pt.y < 0) {
				pSrcRect->top -= pt.y;
				pt.y = 0;
			}
			int offset;
			if ((offset = pSrcRect->right + pt.x - s.cx) >= 0) {
				pSrcRect->right -= offset + 1;
			}
			if ((offset = pSrcRect->bottom + pt.y - s.cy) >= 0) {
				pSrcRect->bottom -= offset + 1;
			}
		}
		if (m_xBmp->Paint(pBmp, pt.x, pt.y, pSrcRect, nMaskColor))
			return ERR_UNKNOWN;
	}

	return ERR_NONE;
}

BOOL CBagBmpObject::IsInside(const CBofPoint &xPoint) {
	if (m_xBmp && GetRect().PtInRect(xPoint)) {
		if (m_nTrans >= 0) {
			int x = xPoint.x - GetRect().left;
			int y = xPoint.y - GetRect().top;
			int c = m_xBmp->ReadPixel(x, y);
			return c != m_nTrans;
		} else {
			return TRUE;
		}
	}

	return FALSE;
}

} // namespace Bagel
