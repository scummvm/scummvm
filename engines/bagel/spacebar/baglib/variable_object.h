
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

#include "bagel/spacebar/baglib/object.h"

namespace Bagel {
namespace SpaceBar {

/**
 * CBagVariableObject is an object that can be place within the slide window.
 */
class CBagVariableObject : public CBagObject {
private:
	CBofSize _xSize;
	int _nPointSize;
	COLORREF _nFGColor;

public:
	CBagVariableObject();
	virtual ~CBagVariableObject();

	// Return true if the Object had members that are properly initialized/de-initialized
	ErrorCode attach() override;
	ErrorCode detach() override;

	CBofRect getRect() override;

	CBofSize getSize() {
		return _xSize;
	}

	void setSize(const CBofSize &xSize) override {
		_xSize = xSize;
	}

	ParseCodes setInfo(CBagIfstream &istr) override;

	ErrorCode update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect, int) override;
	ErrorCode update(CBofWindow *pWnd, CBofPoint pt, CBofRect *pSrcRect, int);

	int mapFontPointSize(int size) const {
		return size;
	}
};

} // namespace SpaceBar
} // namespace Bagel

#endif
