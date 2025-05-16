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

#include "bagel/spacebar/baglib/sprite_object.h"
#include "bagel/spacebar/baglib/bagel.h"
#include "bagel/spacebar/baglib/master_win.h"
#include "bagel/spacebar/baglib/storage_dev_win.h"

namespace Bagel {
namespace SpaceBar {

CBagSpriteObject::CBagSpriteObject() : CBagObject() {
	_xObjType = SPRITE_OBJ;
	_xSprite = nullptr;
	_nCels = 1;
	_nWieldCursor = -1;

	// Transparent by default
	setTransparent();
	CBagObject::setOverCursor(1);
	setAnimated();
	setTimeless(true);

	// implement sprite framerates
	setFrameRate(0);
	_nLastUpdate = 0;
}

CBagSpriteObject::~CBagSpriteObject() {
	CBagSpriteObject::detach();
}

ErrorCode CBagSpriteObject::attach() {
	// If it's not already attached
	if (!isAttached()) {
		// Could not already have a sprite
		assert(_xSprite == nullptr);

		_xSprite = new CBofSprite();

		if (_xSprite->loadSprite(getFileName(), getCels()) != false && (_xSprite->width() != 0) && (_xSprite->height() != 0)) {
			if (isTransparent()) {
				int nMaskColor = CBagel::getBagApp()->getChromaColor();

				_xSprite->setMaskColor(nMaskColor);
			}

			// Set animated of the sprite to be the same as it's parent
			_xSprite->setAnimated(isAnimated());

			CBofPoint p = CBagObject::getPosition();

			if (p.x == -1 && p.y == -1) // Fixed to allow for [0,0] positioning
				setFloating();
			else
				_xSprite->setPosition(p.x, p.y);

			setProperty("CURR_CEL", getState());

			// This might add something to the PDA, make sure it gets redrawn.
			CBagStorageDevWnd *pMainWin = (CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev());

			if (pMainWin != nullptr) {
				pMainWin->setPreFilterPan(true);
			}
		} else {
			reportError(ERR_FOPEN, "Could Not Open Sprite: %s", _xSprite->getFileName());
		}
	}

	return CBagObject::attach();
}

ErrorCode CBagSpriteObject::detach() {
	delete _xSprite;
	_xSprite = nullptr;

	return CBagObject::detach();
}

void CBagSpriteObject::setCels(int nCels) {
	_nCels = nCels;
	if (_xSprite)
		_xSprite->setupCels(nCels);
}

void CBagSpriteObject::setPosition(const CBofPoint &pos) {
	CBagObject::setPosition(pos);
	if (_xSprite)
		_xSprite->setPosition(pos.x, pos.y);
}

CBofRect CBagSpriteObject::getRect() {
	CBofPoint p = getPosition();
	CBofSize s;
	if (_xSprite)
		s = _xSprite->getSize();
	return CBofRect(p, s);
}

//
//   Takes in info and then removes the relative information and returns the info
//   without the relevant info.
ParseCodes CBagSpriteObject::setInfo(CBagIfstream &istr) {
	bool nObjectUpdated = false;

	while (!istr.eof()) {
		istr.eatWhite(); // not sure why this WAS NOT here.

		char ch = (char)istr.peek();
		switch (ch) {
		//
		//  +n  - n number of slides in sprite
		//
		case '+': {
			int cels;
			istr.getCh();
			getIntFromStream(istr, cels);
			setCels(cels);
			nObjectUpdated = true;
		}
		break;
		case '#': {
			int curs;
			istr.getCh();
			getIntFromStream(istr, curs);
			setWieldCursor(curs);
			nObjectUpdated = true;
		}
		break;
		case 'N': {
			// NOANIM
			char szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256); // performance improvement
			getAlphaNumFromStream(istr, sStr);

			if (!sStr.find("NOANIM")) {
				istr.eatWhite();
				setAnimated(false);
				nObjectUpdated = true;
			} else {
				putbackStringOnStream(istr, sStr);
			}
		}
		break;

		// handle a maximum framerate...
		case 'F': {
			// NOANIM
			char szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256);
			getAlphaNumFromStream(istr, sStr);

			if (!sStr.find("FRAMERATE")) {
				int nFrameRate;
				istr.eatWhite();
				getIntFromStream(istr, nFrameRate);

				// The frame rate is expressed in frames/second, so do some division
				// here to store the number of milliseconds.
				setFrameRate(1000 / nFrameRate);

				nObjectUpdated = true;
			} else {
				putbackStringOnStream(istr, sStr);
			}
		}
		break;
		//
		//  no match return from function
		//
		default: {
			ParseCodes parseCode = CBagObject::setInfo(istr);
			if (parseCode == PARSING_DONE) {
				return PARSING_DONE;
			}

			if (parseCode == UPDATED_OBJECT) {
				nObjectUpdated = true;
			} else { // rc==UNKNOWN_TOKEN
				if (nObjectUpdated)
					return UPDATED_OBJECT;

				return UNKNOWN_TOKEN;
			}
		}
		break;
		}
	}

	return PARSING_DONE;
}

ErrorCode CBagSpriteObject::update(CBofBitmap *pBmp, CBofPoint pt, CBofRect * /*pSrcRect*/, int) {
	if (_xSprite) {
		int nFrameInterval = getFrameRate();

		if (nFrameInterval != 0) {
			uint32 nCurTime = getTimer();
			if (nCurTime > _nLastUpdate + nFrameInterval) {
				_xSprite->setBlockAdvance(false);
				_nLastUpdate = nCurTime;
			} else {
				_xSprite->setBlockAdvance(true);
			}
		}

		bool bPaintResult = _xSprite->paintSprite(pBmp, pt.x, pt.y);

		// Don't have to redraw this item...
		// setDirty (false);

		if (!bPaintResult)
			return ERR_UNKNOWN;
	}
	return ERR_NONE;
}

bool CBagSpriteObject::isInside(const CBofPoint &xPoint) {
	if (_xSprite && getRect().ptInRect(xPoint)) {
		if (isTransparent()) {
			int x = xPoint.x - getRect().left;
			int y = xPoint.y - getRect().top;
			int c = _xSprite->readPixel(x, y);
			int d = _xSprite->getMaskColor();
			return (c != d);
		}

		return true;
	}
	return false;
}

void CBagSpriteObject::setProperty(const CBofString &sProp, int nVal) {
	if (!sProp.find("STATE") || !sProp.find("CURR_CEL")) {
		setState(nVal);
		if (_xSprite)
			_xSprite->setCel(nVal);
	} else
		CBagObject::setProperty(sProp, nVal);
}

int CBagSpriteObject::getProperty(const CBofString &sProp) {
	if (!sProp.find("CURR_CEL")) {
		if (_xSprite) {
			return _xSprite->getCelIndex();
		}
		return 0;
	}

	return CBagObject::getProperty(sProp);
}

void CBagSpriteObject::setAnimated(bool b) {
	_bAnimated = b;
	if (_xSprite)
		_xSprite->setAnimated(b);
}

} // namespace SpaceBar
} // namespace Bagel
