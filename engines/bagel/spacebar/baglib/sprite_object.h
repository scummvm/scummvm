
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

#include "bagel/spacebar/baglib/object.h"
#include "bagel/spacebar/boflib/gfx/sprite.h"

namespace Bagel {
namespace SpaceBar {

/**
 * CBagSpriteObject is an object that can be place within the slide window.
 */
class CBagSpriteObject : public CBagObject {
private:
	CBofSprite *_xSprite;
	int _nCels;
	int _nWieldCursor; // Ref Id for the objects over cursor
	bool _bAnimated : 1;
	int _nMaxFrameRate;

public:
	CBagSpriteObject();
	virtual ~CBagSpriteObject();

	ErrorCode attach() override;
	ErrorCode detach() override;
	bool isAttached() override {
		return _xSprite != nullptr;
	}
	ParseCodes setInfo(CBagIfstream &istr) override;

	bool isInside(const CBofPoint &xPoint) override;

	int getWieldCursor() const {
		return _nWieldCursor;
	}
	void setWieldCursor(int n) {
		_nWieldCursor = n;
	}

	CBofSprite *getSprite() const {
		return _xSprite;
	}
	CBofRect getRect() override;
	int getCels() const {
		return _nCels;
	}

	bool isAnimated() const {
		return _bAnimated;
	}

	void setAnimated(bool b = true);
	void setCels(int nCels);
	void setPosition(const CBofPoint &pos) override;

	ErrorCode update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect = nullptr, int /*nMaskColor*/ = -1) override;

	void setProperty(const CBofString &sProp, int nVal) override;
	int getProperty(const CBofString &sProp) override;

	int getFrameRate() const {
		return _nMaxFrameRate;
	}
	void setFrameRate(int nFR) {
		_nMaxFrameRate = nFR;
	}

	uint32 _nLastUpdate;
};

} // namespace SpaceBar
} // namespace Bagel

#endif
