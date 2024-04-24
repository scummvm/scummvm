
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

#ifndef BAGEL_BAGLIB_TIME_OBJECT_H
#define BAGEL_BAGLIB_TIME_OBJECT_H

#include "bagel/baglib/object.h"
#include "bagel/boflib/gfx/bitmap.h"
#include "bagel/boflib/gui/window.h"
#include "bagel/boflib/gfx/sprite.h"

namespace Bagel {

/**
 * CBagTimeObject is an object that can be place within the slide window.
 */
class CBagTimeObject : public CBagObject {
private:
	CBofString m_sVariable;

	CBofSprite *m_xDig1;  // x-:--
	CBofSprite *m_xDig2;  // -x:--
	CBofSprite *m_xColon; // --:--
	CBofSprite *m_xDig3;  // --:x-
	CBofSprite *m_xDig4;  // --:-x

	int m_nCels;

public:
	CBagTimeObject();
	virtual ~CBagTimeObject();

	/**
	 * Create all 5 sprite objects for the clock and set their positions
	 */
	ErrorCode Attach();
	ErrorCode Detach();
	bool IsAttached() {
		return m_xDig1 != nullptr;
	}

	/**
	 * Takes in info and then removes the relative information and returns the info
	 * without the relevant info.
	 */
	PARSE_CODES SetInfo(bof_ifstream &istr);

	CBofRect GetRect();
	int GetCels() {
		return m_nCels;
	}

	void SetCels(int nCels);
	virtual void SetPosition(const CBofPoint &pos);

	/**
	 * Read in the value of the associated variable and set the time equal
	 * to the first 4 digits of the variable if the variable is less the 4 digits
	 * the time is padded with 0's if it is greater the 4 we truncate to remaining digits
	 */
	virtual ErrorCode Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect = nullptr, int /*nMaskColor*/ = -1);
	virtual ErrorCode Update(CBofWindow *pWnd, CBofPoint pt, CBofRect * /*pSrcRect*/ = nullptr, int /*nMaskColor*/ = -1);

	void SetVariable(const CBofString &sProp) {
		m_sVariable = sProp;
	}
};

} // namespace Bagel

#endif
