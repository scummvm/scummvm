
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

#include "bagel/spacebar/baglib/object.h"

namespace Bagel {
namespace SpaceBar {

/**
 * CBagBmpObject is an object that can be place within the slide window.
 */
class CBagBmpObject : public CBagObject {
private:
	CBofBitmap *_bmp;
	int _transparency;

protected:
	void setBitmap(CBofBitmap *bmp) {
		_bmp = bmp;
	}

public:
	CBagBmpObject();
	virtual ~CBagBmpObject();

	ErrorCode attach() override {
		return attach(nullptr);
	}

	ErrorCode attach(CBofPalette *palette);
	ErrorCode detach() override;

	bool isAttached() override {
		return _bmp != nullptr;
	}

	bool isInside(const CBofPoint &pt) override;

	CBofBitmap *getBitmap() const {
		return _bmp;
	}
	CBofRect getRect() override;

	ErrorCode update(CBofBitmap *bmp, CBofPoint pt, CBofRect *srcRect = nullptr, int maskColor = -1) override;
};

} // namespace SpaceBar
} // namespace Bagel

#endif
