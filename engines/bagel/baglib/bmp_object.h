
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

#ifndef BAGEL_BAGLIB_BMP_OBJECT_H
#define BAGEL_BAGLIB_BMP_OBJECT_H

#include "bagel/baglib/object.h"

namespace Bagel {

/**
 * CBagBmpObject is an object that can be place within the slide window.
 */
class CBagBmpObject : public CBagObject {
private:
	CBofBitmap *m_xBmp;
	int m_nTrans;

protected:
	void SetBitmap(CBofBitmap *pBmp) {
		m_xBmp = pBmp;
	}

public:
	CBagBmpObject();
	virtual ~CBagBmpObject();

	ErrorCode Attach() {
		return Attach(nullptr);
	}
	ErrorCode Attach(CBofPalette *pPalette);
	ErrorCode Detach();
	bool IsAttached() {
		return m_xBmp != nullptr;
	}

	bool IsInside(const CBofPoint &xPoint);

	CBofBitmap *GetBitmap() {
		return m_xBmp;
	}
	CBofRect GetRect();

	virtual ErrorCode Update(CBofWindow *pWnd, CBofPoint pt, CBofRect *pSrcRect = nullptr, int nMaskColor = -1);
	virtual ErrorCode Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect = nullptr, int nMaskColor = -1);
};

} // namespace Bagel

#endif
