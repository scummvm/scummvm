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

#include "bagel/baglib/bagel.h"
#include "bagel/baglib/master_win.h"
#include "bagel/baglib/time_object.h"
#include "bagel/boflib/log.h"

namespace Bagel {

CBagTimeObject::CBagTimeObject() : CBagObject() {
	_xObjType = SPRITEOBJ;
	_xDig1 = nullptr;
	_xDig2 = nullptr;
	_xColon = nullptr;
	_xDig3 = nullptr;
	_xDig4 = nullptr;
	_nCels = 1;

	setOverCursor(1);
	setTimeless(true);
}

CBagTimeObject::~CBagTimeObject() {
	detach();
}

ErrorCode CBagTimeObject::attach() {
	CBofPoint p = CBagObject::getPosition();

	if ((_xDig1 = new CBofSprite()) != nullptr) {
		if (_xDig1->loadSprite(getFileName(), getCels()) != 0 && (_xDig1->width() != 0) && (_xDig1->height() != 0)) {
			_xDig1->setAnimated(false);
			_xDig1->setPosition(p.x, p.y);

			p.offset(_xDig1->width(), 0);

		} else {
			reportError(ERR_FOPEN, "Could Not Open Dig1 Sprite: %s", _xDig1->getFileName());
		}

	} else {
		reportError(ERR_MEMORY, "Could not allocate  Dig1 sprite");
	}
	if ((_xDig2 = new CBofSprite()) != nullptr) {

		if (_xDig2->loadSprite(getFileName(), getCels()) != 0 && (_xDig2->width() != 0) && (_xDig2->height() != 0)) {

			_xDig2->setAnimated(false);

			_xDig2->setPosition(p.x, p.y);

			p.offset(_xDig2->width(), 0);

		} else {
			reportError(ERR_FOPEN, "Could Not Open Dig2 Sprite: %s", _xDig2->getFileName());
		}

	} else {
		reportError(ERR_MEMORY, "Could not allocate  Dig2 sprite");
	}
	if ((_xColon = new CBofSprite()) != nullptr) {

		if (_xColon->loadSprite(getFileName(), getCels()) != 0 && (_xColon->width() != 0) && (_xColon->height() != 0)) {

			_xColon->setAnimated(false);
			// The time sprite should start with 0 and go to 9 followed by the :
			_xColon->setCel(_nCels - 1);
			_xColon->setPosition(p.x, p.y);

			p.offset(_xColon->width(), 0);
		} else {
			reportError(ERR_FOPEN, "Could Not Open Colon Sprite: %s", _xColon->getFileName());
		}

	} else {
		reportError(ERR_MEMORY, "Could not allocate  Colon sprite");
	}
	if ((_xDig3 = new CBofSprite()) != nullptr) {

		if (_xDig3->loadSprite(getFileName(), getCels()) != 0 && (_xDig3->width() != 0) && (_xDig3->height() != 0)) {

			_xDig3->setAnimated(false);

			_xDig3->setPosition(p.x, p.y);

			p.offset(_xDig3->width(), 0);
		} else {
			reportError(ERR_FOPEN, "Could Not Open Dig3 Sprite: %s", _xDig3->getFileName());
		}

	} else {
		reportError(ERR_MEMORY, "Could not allocate  Dig3 sprite");
	}
	if ((_xDig4 = new CBofSprite()) != nullptr) {

		if (_xDig4->loadSprite(getFileName(), getCels()) != 0 && (_xDig4->width() != 0) && (_xDig4->height() != 0)) {

			_xDig4->setAnimated(false);

			_xDig4->setPosition(p.x, p.y);

			p.offset(_xDig4->width(), 0);
		} else {
			reportError(ERR_FOPEN, "Could Not Open Dig4 Sprite: %s", _xDig4->getFileName());
		}

	} else {
		reportError(ERR_MEMORY, "Could not allocate  Dig4 sprite");
	}

	return CBagObject::attach();
}

ErrorCode CBagTimeObject::detach() {
	if (_xDig1) {
		delete _xDig1;
		_xDig1 = nullptr;
	}
	if (_xDig2) {
		delete _xDig2;
		_xDig2 = nullptr;
	}
	if (_xColon) {
		delete _xColon;
		_xColon = nullptr;
	}
	if (_xDig3) {
		delete _xDig3;
		_xDig3 = nullptr;
	}
	if (_xDig4) {
		delete _xDig4;
		_xDig4 = nullptr;
	}

	return CBagObject::detach();
}

void CBagTimeObject::setCels(int nCels) {
	_nCels = nCels;

	if (_xDig1)
		_xDig1->setupCels(nCels);
	if (_xDig2)
		_xDig2->setupCels(nCels);
	if (_xColon)
		_xColon->setupCels(nCels);
	if (_xDig3)
		_xDig3->setupCels(nCels);
	if (_xDig4)
		_xDig4->setupCels(nCels);
}

void CBagTimeObject::setPosition(const CBofPoint &pos) {
	CBagObject::setPosition(pos);
}

CBofRect CBagTimeObject::getRect() {
	CBofPoint p = getPosition();
	CBofSize s;

	if (_xDig1) {
		s = _xDig1->getSize();

		// Increase the width to accomadate all 5 sprites
		s.cx = s.cx * 5;
	}
	return CBofRect(p, s);
}

ParseCodes CBagTimeObject::setInfo(CBagIfstream &istr) {
	bool nObjectUpdated = false;

	while (!istr.eof()) {
		istr.eatWhite();

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
			break;
		}

		case 'V': {
			char szLocalBuff[256];
			szLocalBuff[0] = '\0';
			CBofString sStr(szLocalBuff, 256);

			getAlphaNumFromStream(istr, sStr);

			if (!sStr.find("VALUE")) {
				istr.eatWhite();
				char szLocalBuff1[256];
				szLocalBuff[0] = '\0';
				CBofString s(szLocalBuff1, 256);
				getAlphaNumFromStream(istr, s);

				setVariable(s);

				nObjectUpdated = true;
			} else {
				putbackStringOnStream(istr, sStr);
			}
			break;
		}

		// No match return from funtion
		default: {
			ParseCodes rc = CBagObject::setInfo(istr);
			if (rc == PARSING_DONE) {
				return PARSING_DONE;
			}

			if (rc == UPDATED_OBJECT) {
				nObjectUpdated = true;
			} else {
				// rc==UNKNOWN_TOKEN
				if (nObjectUpdated)
					return UPDATED_OBJECT;

				return UNKNOWN_TOKEN;
			}
			break;
		}
		}
	}

	return PARSING_DONE;
}

ErrorCode CBagTimeObject::update(CBofBitmap *pBmp, CBofPoint pt, CBofRect * /*pSrcRect*/, int) {
	char szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sTimeString(szLocalBuff, 256);
	ErrorCode rc = ERR_NONE;

	CBagVar *xVar = VAR_MANAGER->GetVariable(_sVariable);

	// If everything looks good
	if (isAttached() && xVar && !(xVar->GetValue().isEmpty())) {
		int nTimeVal = xVar->GetNumValue();
		sTimeString = buildString("%04d", nTimeVal);
		char sDigString[2] = "0";

		// Digit 1
		if (_xDig1) {
			sDigString[0] = sTimeString[0];
			_xDig1->setCel(atoi(sDigString));
			_xDig1->paintSprite(pBmp, pt.x, pt.y);
			pt.offset(_xDig1->width(), 0);
		}
		// Digit 2
		if (_xDig2) {
			sDigString[0] = sTimeString[1];
			_xDig2->setCel(atoi(sDigString));
			_xDig2->paintSprite(pBmp, pt.x, pt.y);
			pt.offset(_xDig2->width(), 0);
		}
		if (_xColon) {
			_xColon->paintSprite(pBmp, pt.x, pt.y);
			pt.offset(_xColon->width(), 0);
		}
		// Digit 3
		if (_xDig3) {
			sDigString[0] = sTimeString[2];
			_xDig3->setCel(atoi(sDigString));
			_xDig3->paintSprite(pBmp, pt.x, pt.y);
			pt.offset(_xDig3->width(), 0);
		}
		// Digit 4
		if (_xDig4) {
			sDigString[0] = sTimeString[3];
			_xDig4->setCel(atoi(sDigString));
			_xDig4->paintSprite(pBmp, pt.x, pt.y);
		}
	}

	return rc;
}

ErrorCode CBagTimeObject::update(CBofWindow *pWnd, CBofPoint pt, CBofRect *, int) {
	char szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sTimeString(szLocalBuff, 256);

	ErrorCode rc = ERR_NONE;

	CBagVar *xVar = VAR_MANAGER->GetVariable(_sVariable);

	// If everything looks good
	if (isAttached() && xVar && !(xVar->GetValue().isEmpty())) {
		int nTimeVal = xVar->GetNumValue();
		sTimeString = buildString("%04d", nTimeVal);
		char sDigString[2] = "0";

		// Digit 1
		if (_xDig1) {
			sDigString[0] = sTimeString[0];
			_xDig1->setCel(atoi(sDigString));
			_xDig1->paintSprite(pWnd, pt.x, pt.y);
			pt.offset(_xDig1->width(), 0);
		}
		// Digit 2
		if (_xDig2) {
			sDigString[0] = sTimeString[1];
			_xDig2->setCel(atoi(sDigString));
			_xDig2->paintSprite(pWnd, pt.x, pt.y);
			pt.offset(_xDig2->width(), 0);
		}
		if (_xColon) {
			_xColon->paintSprite(pWnd, pt.x, pt.y);
			pt.offset(_xColon->width(), 0);
		}
		// Digit 3
		if (_xDig3) {
			sDigString[0] = sTimeString[2];
			_xDig3->setCel(atoi(sDigString));
			_xDig3->paintSprite(pWnd, pt.x, pt.y);
			pt.offset(_xDig3->width(), 0);
		}
		// Digit 4
		if (_xDig4) {
			sDigString[0] = sTimeString[3];
			_xDig4->setCel(atoi(sDigString));
			_xDig4->paintSprite(pWnd, pt.x, pt.y);
		}
	}

	return rc;
}

} // namespace Bagel
