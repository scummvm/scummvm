
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

#include "bagel/spacebar/baglib/object.h"
#include "bagel/spacebar/boflib/gfx/bitmap.h"
#include "bagel/spacebar/boflib/gfx/sprite.h"

namespace Bagel {
namespace SpaceBar {

/**
 * CBagTimeObject is an object that can be place within the slide window.
 */
class CBagTimeObject : public CBagObject {
private:
	CBofString _sVariable;

	CBofSprite *_xDig1;  // x-:--
	CBofSprite *_xDig2;  // -x:--
	CBofSprite *_xColon; // --:--
	CBofSprite *_xDig3;  // --:x-
	CBofSprite *_xDig4;  // --:-x

	int _nCels;

public:
	CBagTimeObject();
	virtual ~CBagTimeObject();

	/**
	 * Create all 5 sprite objects for the clock and set their positions
	 */
	ErrorCode attach() override;
	ErrorCode detach() override;
	bool isAttached() override {
		return _xDig1 != nullptr;
	}

	/**
	 * Takes in info and then removes the relative information and returns the info
	 * without the relevant info.
	 */
	ParseCodes setInfo(CBagIfstream &istr) override;

	CBofRect getRect() override;
	int getCels() {
		return _nCels;
	}

	void setCels(int nCels);
	void setPosition(const CBofPoint &pos) override;

	/**
	 * Read in the value of the associated variable and set the time equal
	 * to the first 4 digits of the variable if the variable is less the 4 digits
	 * the time is padded with 0's if it is greater the 4 we truncate to remaining digits
	 */
	ErrorCode update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect = nullptr, int /*nMaskColor*/ = -1) override;

	void setVariable(const CBofString &sProp) {
		_sVariable = sProp;
	}
};

} // namespace SpaceBar
} // namespace Bagel

#endif
