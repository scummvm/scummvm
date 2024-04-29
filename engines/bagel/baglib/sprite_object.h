
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

#ifndef BAGEL_BAGLIB_SPRITE_OBJECT_H
#define BAGEL_BAGLIB_SPRITE_OBJECT_H

#include "bagel/baglib/object.h"
#include "bagel/boflib/gfx/sprite.h"

namespace Bagel {

/**
 * CBagSpriteObject is an object that can be place within the slide window.
 */
class CBagSpriteObject : public CBagObject {
private:
	CBofSprite *m_xSprite;
	int m_nCels;
	int m_nWieldCursor; // Ref Id for the objects over cursor
	bool m_bAnimated : 1;
	int m_nMaxFrameRate;

public:
	CBagSpriteObject();
	virtual ~CBagSpriteObject();

	ErrorCode attach();
	ErrorCode detach();
	bool isAttached() {
		return m_xSprite != nullptr;
	}
	PARSE_CODES setInfo(bof_ifstream &istr);

	bool isInside(const CBofPoint &xPoint);

	int GetWieldCursor() {
		return m_nWieldCursor;
	}
	void SetWieldCursor(int n) {
		m_nWieldCursor = n;
	}

	CBofSprite *GetSprite() {
		return m_xSprite;
	}
	CBofRect getRect();
	int GetCels() {
		return m_nCels;
	}

	bool IsAnimated() {
		return m_bAnimated;
	}

	void SetAnimated(bool b = true);
	void SetCels(int nCels);
	virtual void SetPosition(const CBofPoint &pos);

	virtual ErrorCode update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect = nullptr, int /*nMaskColor*/ = -1);
	virtual ErrorCode Update(CBofWindow *pWnd, CBofPoint pt, CBofRect * /*pSrcRect*/ = nullptr, int /*nMaskColor*/ = -1);

	void setProperty(const CBofString &sProp, int nVal);
	int getProperty(const CBofString &sProp);

	int getFrameRate() {
		return m_nMaxFrameRate;
	}
	void setFrameRate(int nFR) {
		m_nMaxFrameRate = nFR;
	}

	uint32 m_nLastUpdate;
};

} // namespace Bagel

#endif
