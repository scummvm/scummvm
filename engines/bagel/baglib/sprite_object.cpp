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

#include "bagel/baglib/sprite_object.h"
#include "bagel/baglib/bagel.h"
#include "bagel/baglib/master_win.h"
#include "bagel/baglib/storage_dev_win.h"

namespace Bagel {

CBagSpriteObject::CBagSpriteObject() : CBagObject() {
	_xObjType = SPRITEOBJ;
	m_xSprite = nullptr;
	m_nCels = 1;
	m_nWieldCursor = -1;

	// Transparent by default
	setTransparent();
	setOverCursor(1);
	setAnimated();
	setTimeless(true);

	// implement sprite framerates
	setFrameRate(0);
	m_nLastUpdate = 0;
}

CBagSpriteObject::~CBagSpriteObject() {
	detach();
}

ErrorCode CBagSpriteObject::attach() {
	// If it's not already attached
	if (!isAttached()) {
		// Could not already have a sprite
		assert(m_xSprite == nullptr);

		if ((m_xSprite = new CBofSprite()) != nullptr) {
			if (m_xSprite->loadSprite(getFileName(), getCels()) != false && (m_xSprite->width() != 0) && (m_xSprite->height() != 0)) {
				if (isTransparent()) {
					int nMaskColor = CBagel::getBagApp()->getChromaColor();

					m_xSprite->setMaskColor(nMaskColor);
				}

				// Set animated of the sprite to be the same as it's parent
				m_xSprite->setAnimated(IsAnimated());

				CBofPoint p = CBagObject::getPosition();

				if (p.x == -1 && p.y == -1) // Fixed to allow for [0,0] positioning
					setFloating();
				else
					m_xSprite->setPosition(p.x, p.y);

				setProperty("CURR_CEL", getState());

				// This might add something to the PDA, make sure it gets redrawn.
				CBagStorageDevWnd *pMainWin = (CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev());

				if (pMainWin != nullptr) {
					pMainWin->SetPreFilterPan(true);
				}
			} else {
				reportError(ERR_FOPEN, "Could Not Open Sprite: %s", m_xSprite->getFileName());
			}

		} else {
			reportError(ERR_MEMORY, "Could not allocate sprite");
		}
	}

	return CBagObject::attach();
}

ErrorCode CBagSpriteObject::detach() {
	if (m_xSprite != nullptr) {
		delete m_xSprite;
		m_xSprite = nullptr;
	}
	return CBagObject::detach();
}

void CBagSpriteObject::setCels(int nCels) {
	m_nCels = nCels;
	if (m_xSprite)
		m_xSprite->setupCels(nCels);
}

void CBagSpriteObject::setPosition(const CBofPoint &pos) {
	CBagObject::setPosition(pos);
	if (m_xSprite)
		m_xSprite->setPosition(pos.x, pos.y);
}

CBofRect CBagSpriteObject::getRect() {
	CBofPoint p = getPosition();
	CBofSize s;
	if (m_xSprite)
		s = m_xSprite->getSize();
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
			SetWieldCursor(curs);
			nObjectUpdated = true;
		}
		break;
		case 'N': { // NOANIM
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
		case 'F': { // NOANIM
			char szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256);
			getAlphaNumFromStream(istr, sStr);

			if (!sStr.find("FRAMERATE")) {
				int nFrameRate;
				istr.eatWhite();
				getIntFromStream(istr, nFrameRate);

				// The framerate is expressed in frames/second, so do some division
				// here to store the number of milliseconds.
				setFrameRate(1000 / nFrameRate);

				nObjectUpdated = true;
			} else {
				putbackStringOnStream(istr, sStr);
			}
		}
		break;
		//
		//  no match return from funtion
		//
		default: {
			ParseCodes rc = CBagObject::setInfo(istr);
			if (rc == PARSING_DONE) {
				return PARSING_DONE;
			}

			if (rc == UPDATED_OBJECT) {
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
	if (m_xSprite) {
		bool b = true;
		int nFrameInterval = getFrameRate();

		if (nFrameInterval != 0) {
			uint32 nCurTime = getTimer();
			if (nCurTime > m_nLastUpdate + nFrameInterval) {
				m_xSprite->setBlockAdvance(false);
				m_nLastUpdate = nCurTime;
			} else {
				m_xSprite->setBlockAdvance(true);
			}
		}

		b = m_xSprite->paintSprite(pBmp, pt.x, pt.y);

		// Don't have to redraw this item...
		// setDirty (false);

		if (!b)
			return ERR_UNKNOWN;
	}
	return ERR_NONE;
}

ErrorCode CBagSpriteObject::Update(CBofWindow *pWnd, CBofPoint pt, CBofRect *, int) {
	if (m_xSprite) {
		bool b = m_xSprite->paintSprite(pWnd, pt.x, pt.y);

		// don't have to redraw this item...
		// setDirty (false);

		if (!b)
			return ERR_UNKNOWN;
	}
	return ERR_NONE;
}

bool CBagSpriteObject::isInside(const CBofPoint &xPoint) {
	if (m_xSprite && getRect().ptInRect(xPoint)) {
		if (isTransparent()) {
			int x = xPoint.x - getRect().left;
			int y = xPoint.y - getRect().top;
			int c = m_xSprite->readPixel(x, y);
			int d = m_xSprite->getMaskColor();
			return (c != d);
		}

		return true;
	}
	return false;
}

void CBagSpriteObject::setProperty(const CBofString &sProp, int nVal) {
	if (!sProp.find("STATE")) {
		setState(nVal);
		if (m_xSprite)
			m_xSprite->setCel(nVal);
	} else if (!sProp.find("CURR_CEL")) {
		setState(nVal);
		if (m_xSprite)
			m_xSprite->setCel(nVal);
	} else
		CBagObject::setProperty(sProp, nVal);
}

int CBagSpriteObject::getProperty(const CBofString &sProp) {
	if (!sProp.find("CURR_CEL")) {
		if (m_xSprite) {
			return m_xSprite->getCelIndex();
		}
		return 0;
	}

	return CBagObject::getProperty(sProp);
}

void CBagSpriteObject::setAnimated(bool b) {
	m_bAnimated = b;
	if (m_xSprite)
		m_xSprite->setAnimated(b);
}

} // namespace Bagel
