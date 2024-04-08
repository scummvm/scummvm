
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

#ifndef BAGEL_BAGLIB_VARIABLE_OBJECT_H
#define BAGEL_BAGLIB_VARIABLE_OBJECT_H

#include "bagel/baglib/text_object.h"
#include "bagel/baglib/object.h"

namespace Bagel {

/**
 * CBagVariableObject is an object that can be place within the slide window.
 */
class CBagVariableObject : public CBagObject {
private:
	CBofSize m_xSize;
	int m_nPointSize;
	RGBCOLOR m_nFGColor;

public:
	CBagVariableObject();
	virtual ~CBagVariableObject();

	// Return true if the Object had members that are properly initialized/de-initialized
	ERROR_CODE Attach();
	ERROR_CODE Detach();

	CBofRect GetRect();

	CBofSize GetSize() {
		return m_xSize;
	}

	void SetSize(const CBofSize &xSize) {
		m_xSize = xSize;
	}

	PARSE_CODES SetInfo(bof_ifstream &istr);

	ERROR_CODE Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect, int);
	ERROR_CODE Update(CBofWindow *pWnd, CBofPoint pt, CBofRect *pSrcRect, int);

	int MapWindowsPointSize(int);
};

} // namespace Bagel

#endif
