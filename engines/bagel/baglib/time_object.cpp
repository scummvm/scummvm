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

#include "bagel/baglib/time_object.h"
#include "bagel/baglib/bagel.h"
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
	Detach();
}

ErrorCode CBagTimeObject::Attach() {
	CBofPoint p = CBagObject::GetPosition();

	if ((m_xDig1 = new CBofSprite()) != nullptr) {
		if (m_xDig1->LoadSprite(GetFileName(), GetCels()) != 0 && (m_xDig1->Width() != 0) && (m_xDig1->Height() != 0)) {
			m_xDig1->SetAnimated(false);
			m_xDig1->SetPosition(p.x, p.y);

			p.Offset(m_xDig1->Width(), 0);

		} else {
			ReportError(ERR_FOPEN, "Could Not Open Dig1 Sprite: %s", m_xDig1->GetFileName());
		}

	} else {
		ReportError(ERR_MEMORY, "Could not allocate  Dig1 sprite");
	}
	if ((m_xDig2 = new CBofSprite()) != nullptr) {

		if (m_xDig2->LoadSprite(GetFileName(), GetCels()) != 0 && (m_xDig2->Width() != 0) && (m_xDig2->Height() != 0)) {

			m_xDig2->SetAnimated(false);

			m_xDig2->SetPosition(p.x, p.y);

			p.Offset(m_xDig2->Width(), 0);

		} else {
			ReportError(ERR_FOPEN, "Could Not Open Dig2 Sprite: %s", m_xDig2->GetFileName());
		}

	} else {
		ReportError(ERR_MEMORY, "Could not allocate  Dig2 sprite");
	}
	if ((m_xColon = new CBofSprite()) != nullptr) {

		if (m_xColon->LoadSprite(GetFileName(), GetCels()) != 0 && (m_xColon->Width() != 0) && (m_xColon->Height() != 0)) {

			m_xColon->SetAnimated(false);
			// The time sprite should start with 0 and go to 9 followed by the :
			m_xColon->SetCel(m_nCels - 1);
			m_xColon->SetPosition(p.x, p.y);

			p.Offset(m_xColon->Width(), 0);
		} else {
			ReportError(ERR_FOPEN, "Could Not Open Colon Sprite: %s", m_xColon->GetFileName());
		}

	} else {
		ReportError(ERR_MEMORY, "Could not allocate  Colon sprite");
	}
	if ((m_xDig3 = new CBofSprite()) != nullptr) {

		if (m_xDig3->LoadSprite(GetFileName(), GetCels()) != 0 && (m_xDig3->Width() != 0) && (m_xDig3->Height() != 0)) {

			m_xDig3->SetAnimated(false);

			m_xDig3->SetPosition(p.x, p.y);

			p.Offset(m_xDig3->Width(), 0);
		} else {
			ReportError(ERR_FOPEN, "Could Not Open Dig3 Sprite: %s", m_xDig3->GetFileName());
		}

	} else {
		ReportError(ERR_MEMORY, "Could not allocate  Dig3 sprite");
	}
	if ((m_xDig4 = new CBofSprite()) != nullptr) {

		if (m_xDig4->LoadSprite(GetFileName(), GetCels()) != 0 && (m_xDig4->Width() != 0) && (m_xDig4->Height() != 0)) {

			m_xDig4->SetAnimated(false);

			m_xDig4->SetPosition(p.x, p.y);

			p.Offset(m_xDig4->Width(), 0);
		} else {
			ReportError(ERR_FOPEN, "Could Not Open Dig4 Sprite: %s", m_xDig4->GetFileName());
		}

	} else {
		ReportError(ERR_MEMORY, "Could not allocate  Dig4 sprite");
	}

	return CBagObject::Attach();
}

ErrorCode CBagTimeObject::Detach() {
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

	return CBagObject::Detach();
}

void CBagTimeObject::SetCels(int nCels) {
	m_nCels = nCels;

	if (m_xDig1)
		m_xDig1->SetupCels(nCels);
	if (m_xDig2)
		m_xDig2->SetupCels(nCels);
	if (m_xColon)
		m_xColon->SetupCels(nCels);
	if (m_xDig3)
		m_xDig3->SetupCels(nCels);
	if (m_xDig4)
		m_xDig4->SetupCels(nCels);
}

void CBagTimeObject::SetPosition(const CBofPoint &pos) {
	CBagObject::SetPosition(pos);
	// Deal with this if you can ever change the time position
	// after the attach
}

CBofRect CBagTimeObject::GetRect() {
	CBofPoint p = GetPosition();
	CBofSize s;

	if (m_xDig1) {
		s = m_xDig1->GetSize();
		// increase the width to accomadate all 5 sprites
		s.cx = s.cx * 5;
	}
	return CBofRect(p, s);
}

PARSE_CODES CBagTimeObject::SetInfo(bof_ifstream &istr) {
	bool nObjectUpdated = false;

	while (!istr.eof()) {
		istr.EatWhite();

		char ch = (char)istr.peek();
		switch (ch) {
		//
		//  +n  - n number of slides in sprite
		//
		case '+': {
			int cels;
			istr.Get();
			GetIntFromStream(istr, cels);
			SetCels(cels);
			nObjectUpdated = true;
			break;
		}

		case 'V': {
			char szLocalBuff[256];
			szLocalBuff[0] = '\0';
			CBofString sStr(szLocalBuff, 256);

			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("VALUE")) {
				istr.EatWhite();
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
			PARSE_CODES rc = CBagObject::SetInfo(istr);
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

ErrorCode CBagTimeObject::Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect * /*pSrcRect*/, int) {
	char szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sTimeString(szLocalBuff, 256);
	ErrorCode rc = ERR_NONE;

	CBagVar *xVar = VARMNGR->GetVariable(m_sVariable);

	// if everything looks good
	if (IsAttached() && xVar && !(xVar->GetValue().IsEmpty())) {

		int nTimeVal = xVar->GetNumValue();
		sTimeString = BuildString("%04d", nTimeVal);
		char sDigString[2] = "0";

		// Digit 1
		if (m_xDig1) {
			sDigString[0] = sTimeString[0];
			m_xDig1->SetCel(atoi(sDigString));
			m_xDig1->PaintSprite(pBmp, pt.x, pt.y);
			pt.Offset(m_xDig1->Width(), 0);
		}
		// Digit 2
		if (m_xDig2) {
			sDigString[0] = sTimeString[1];
			m_xDig2->SetCel(atoi(sDigString));
			m_xDig2->PaintSprite(pBmp, pt.x, pt.y);
			pt.Offset(m_xDig2->Width(), 0);
		}
		if (m_xColon) {
			m_xColon->PaintSprite(pBmp, pt.x, pt.y);
			pt.Offset(m_xColon->Width(), 0);
		}
		// Digit 3
		if (m_xDig3) {
			sDigString[0] = sTimeString[2];
			m_xDig3->SetCel(atoi(sDigString));
			m_xDig3->PaintSprite(pBmp, pt.x, pt.y);
			pt.Offset(m_xDig3->Width(), 0);
		}
		// Digit 4
		if (m_xDig4) {
			sDigString[0] = sTimeString[3];
			m_xDig4->SetCel(atoi(sDigString));
			m_xDig4->PaintSprite(pBmp, pt.x, pt.y);
		}
	}

	return rc;
}

ErrorCode CBagTimeObject::Update(CBofWindow *pWnd, CBofPoint pt, CBofRect *, int) {
	char szLocalBuff[256];
	szLocalBuff[0] = '\0';
	CBofString sTimeString(szLocalBuff, 256);

	ErrorCode rc = ERR_NONE;

	CBagVar *xVar = VARMNGR->GetVariable(m_sVariable);

	// If everything looks good
	if (IsAttached() && xVar && !(xVar->GetValue().IsEmpty())) {

		int nTimeVal = xVar->GetNumValue();
		sTimeString = BuildString("%04d", nTimeVal);
		char sDigString[2] = "0";

		// Digit 1
		if (m_xDig1) {
			sDigString[0] = sTimeString[0];
			m_xDig1->SetCel(atoi(sDigString));
			m_xDig1->PaintSprite(pWnd, pt.x, pt.y);
			pt.Offset(m_xDig1->Width(), 0);
		}
		// Digit 2
		if (m_xDig2) {
			sDigString[0] = sTimeString[1];
			m_xDig2->SetCel(atoi(sDigString));
			m_xDig2->PaintSprite(pWnd, pt.x, pt.y);
			pt.Offset(m_xDig2->Width(), 0);
		}
		if (m_xColon) {
			m_xColon->PaintSprite(pWnd, pt.x, pt.y);
			pt.Offset(m_xColon->Width(), 0);
		}
		// Digit 3
		if (m_xDig3) {
			sDigString[0] = sTimeString[2];
			m_xDig3->SetCel(atoi(sDigString));
			m_xDig3->PaintSprite(pWnd, pt.x, pt.y);
			pt.Offset(m_xDig3->Width(), 0);
		}
		// Digit 4
		if (m_xDig4) {
			sDigString[0] = sTimeString[3];
			m_xDig4->SetCel(atoi(sDigString));
			m_xDig4->PaintSprite(pWnd, pt.x, pt.y);
		}
	}

	return rc;
}

} // namespace Bagel
