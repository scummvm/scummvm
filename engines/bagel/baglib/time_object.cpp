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
	m_xObjType = SPRITEOBJ;
	m_xDig1 = nullptr;
	m_xDig2 = nullptr;
	m_xColon = nullptr;
	m_xDig3 = nullptr;
	m_xDig4 = nullptr;
	m_nCels = 1;

	SetOverCursor(1);
	SetTimeless(true);
}

CBagTimeObject::~CBagTimeObject() {
	detach();
}

ErrorCode CBagTimeObject::attach() {
	CBofPoint p = CBagObject::getPosition();

	if ((m_xDig1 = new CBofSprite()) != nullptr) {
		if (m_xDig1->loadSprite(getFileName(), GetCels()) != 0 && (m_xDig1->width() != 0) && (m_xDig1->height() != 0)) {
			m_xDig1->setAnimated(false);
			m_xDig1->setPosition(p.x, p.y);

			p.Offset(m_xDig1->width(), 0);

		} else {
			ReportError(ERR_FOPEN, "Could Not Open Dig1 Sprite: %s", m_xDig1->getFileName());
		}

	} else {
		ReportError(ERR_MEMORY, "Could not allocate  Dig1 sprite");
	}
	if ((m_xDig2 = new CBofSprite()) != nullptr) {

		if (m_xDig2->loadSprite(getFileName(), GetCels()) != 0 && (m_xDig2->width() != 0) && (m_xDig2->height() != 0)) {

			m_xDig2->setAnimated(false);

			m_xDig2->setPosition(p.x, p.y);

			p.Offset(m_xDig2->width(), 0);

		} else {
			ReportError(ERR_FOPEN, "Could Not Open Dig2 Sprite: %s", m_xDig2->getFileName());
		}

	} else {
		ReportError(ERR_MEMORY, "Could not allocate  Dig2 sprite");
	}
	if ((m_xColon = new CBofSprite()) != nullptr) {

		if (m_xColon->loadSprite(getFileName(), GetCels()) != 0 && (m_xColon->width() != 0) && (m_xColon->height() != 0)) {

			m_xColon->setAnimated(false);
			// The time sprite should start with 0 and go to 9 followed by the :
			m_xColon->setCel(m_nCels - 1);
			m_xColon->setPosition(p.x, p.y);

			p.Offset(m_xColon->width(), 0);
		} else {
			ReportError(ERR_FOPEN, "Could Not Open Colon Sprite: %s", m_xColon->getFileName());
		}

	} else {
		ReportError(ERR_MEMORY, "Could not allocate  Colon sprite");
	}
	if ((m_xDig3 = new CBofSprite()) != nullptr) {

		if (m_xDig3->loadSprite(getFileName(), GetCels()) != 0 && (m_xDig3->width() != 0) && (m_xDig3->height() != 0)) {

			m_xDig3->setAnimated(false);

			m_xDig3->setPosition(p.x, p.y);

			p.Offset(m_xDig3->width(), 0);
		} else {
			ReportError(ERR_FOPEN, "Could Not Open Dig3 Sprite: %s", m_xDig3->getFileName());
		}

	} else {
		ReportError(ERR_MEMORY, "Could not allocate  Dig3 sprite");
	}
	if ((m_xDig4 = new CBofSprite()) != nullptr) {

		if (m_xDig4->loadSprite(getFileName(), GetCels()) != 0 && (m_xDig4->width() != 0) && (m_xDig4->height() != 0)) {

			m_xDig4->setAnimated(false);

			m_xDig4->setPosition(p.x, p.y);

			p.Offset(m_xDig4->width(), 0);
		} else {
			ReportError(ERR_FOPEN, "Could Not Open Dig4 Sprite: %s", m_xDig4->getFileName());
		}

	} else {
		ReportError(ERR_MEMORY, "Could not allocate  Dig4 sprite");
	}

	return CBagObject::attach();
}

ErrorCode CBagTimeObject::detach() {
	if (m_xDig1) {
		delete m_xDig1;
		m_xDig1 = nullptr;
	}
	if (m_xDig2) {
		delete m_xDig2;
		m_xDig2 = nullptr;
	}
	if (m_xColon) {
		delete m_xColon;
		m_xColon = nullptr;
	}
	if (m_xDig3) {
		delete m_xDig3;
		m_xDig3 = nullptr;
	}
	if (m_xDig4) {
		delete m_xDig4;
		m_xDig4 = nullptr;
	}

	return CBagObject::detach();
}

void CBagTimeObject::setCels(int nCels) {
	m_nCels = nCels;

	if (m_xDig1)
		m_xDig1->setupCels(nCels);
	if (m_xDig2)
		m_xDig2->setupCels(nCels);
	if (m_xColon)
		m_xColon->setupCels(nCels);
	if (m_xDig3)
		m_xDig3->setupCels(nCels);
	if (m_xDig4)
		m_xDig4->setupCels(nCels);
}

void CBagTimeObject::setPosition(const CBofPoint &pos) {
	CBagObject::setPosition(pos);
}

CBofRect CBagTimeObject::getRect() {
	CBofPoint p = getPosition();
	CBofSize s;

	if (m_xDig1) {
		s = m_xDig1->getSize();

		// Increase the width to accomadate all 5 sprites
		s.cx = s.cx * 5;
	}
	return CBofRect(p, s);
}

PARSE_CODES CBagTimeObject::setInfo(CBagIfstream &istr) {
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
			GetIntFromStream(istr, cels);
			setCels(cels);
			nObjectUpdated = true;
			break;
		}

		case 'V': {
			char szLocalBuff[256];
			szLocalBuff[0] = '\0';
			CBofString sStr(szLocalBuff, 256);

			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("VALUE")) {
				istr.eatWhite();
				char szLocalBuff1[256];
				szLocalBuff[0] = '\0';
				CBofString s(szLocalBuff1, 256);
				GetAlphaNumFromStream(istr, s);

				SetVariable(s);

				nObjectUpdated = true;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;
		}

		// No match return from funtion
		default: {
			PARSE_CODES rc = CBagObject::setInfo(istr);
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

	CBagVar *xVar = VAR_MANAGER->GetVariable(m_sVariable);

	// If everything looks good
	if (isAttached() && xVar && !(xVar->GetValue().IsEmpty())) {
		int nTimeVal = xVar->GetNumValue();
		sTimeString = BuildString("%04d", nTimeVal);
		char sDigString[2] = "0";

		// Digit 1
		if (m_xDig1) {
			sDigString[0] = sTimeString[0];
			m_xDig1->setCel(atoi(sDigString));
			m_xDig1->paintSprite(pBmp, pt.x, pt.y);
			pt.Offset(m_xDig1->width(), 0);
		}
		// Digit 2
		if (m_xDig2) {
			sDigString[0] = sTimeString[1];
			m_xDig2->setCel(atoi(sDigString));
			m_xDig2->paintSprite(pBmp, pt.x, pt.y);
			pt.Offset(m_xDig2->width(), 0);
		}
		if (m_xColon) {
			m_xColon->paintSprite(pBmp, pt.x, pt.y);
			pt.Offset(m_xColon->width(), 0);
		}
		// Digit 3
		if (m_xDig3) {
			sDigString[0] = sTimeString[2];
			m_xDig3->setCel(atoi(sDigString));
			m_xDig3->paintSprite(pBmp, pt.x, pt.y);
			pt.Offset(m_xDig3->width(), 0);
		}
		// Digit 4
		if (m_xDig4) {
			sDigString[0] = sTimeString[3];
			m_xDig4->setCel(atoi(sDigString));
			m_xDig4->paintSprite(pBmp, pt.x, pt.y);
		}
	}

	return rc;
}

ErrorCode CBagTimeObject::Update(CBofWindow *pWnd, CBofPoint pt, CBofRect *, int) {
	char szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sTimeString(szLocalBuff, 256);

	ErrorCode rc = ERR_NONE;

	CBagVar *xVar = VAR_MANAGER->GetVariable(m_sVariable);

	// If everything looks good
	if (isAttached() && xVar && !(xVar->GetValue().IsEmpty())) {
		int nTimeVal = xVar->GetNumValue();
		sTimeString = BuildString("%04d", nTimeVal);
		char sDigString[2] = "0";

		// Digit 1
		if (m_xDig1) {
			sDigString[0] = sTimeString[0];
			m_xDig1->setCel(atoi(sDigString));
			m_xDig1->paintSprite(pWnd, pt.x, pt.y);
			pt.Offset(m_xDig1->width(), 0);
		}
		// Digit 2
		if (m_xDig2) {
			sDigString[0] = sTimeString[1];
			m_xDig2->setCel(atoi(sDigString));
			m_xDig2->paintSprite(pWnd, pt.x, pt.y);
			pt.Offset(m_xDig2->width(), 0);
		}
		if (m_xColon) {
			m_xColon->paintSprite(pWnd, pt.x, pt.y);
			pt.Offset(m_xColon->width(), 0);
		}
		// Digit 3
		if (m_xDig3) {
			sDigString[0] = sTimeString[2];
			m_xDig3->setCel(atoi(sDigString));
			m_xDig3->paintSprite(pWnd, pt.x, pt.y);
			pt.Offset(m_xDig3->width(), 0);
		}
		// Digit 4
		if (m_xDig4) {
			sDigString[0] = sTimeString[3];
			m_xDig4->setCel(atoi(sDigString));
			m_xDig4->paintSprite(pWnd, pt.x, pt.y);
		}
	}

	return rc;
}

} // namespace Bagel
