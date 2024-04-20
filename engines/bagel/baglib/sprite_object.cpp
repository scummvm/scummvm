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
#include "bagel/baglib/storage_dev_win.h"

namespace Bagel {

CBagSpriteObject::CBagSpriteObject() : CBagObject() {
	m_xObjType = SPRITEOBJ;
	m_xSprite = nullptr;
	m_nCels = 1;
	m_nWieldCursor = -1;

	// Transparent by default
	SetTransparent();
	SetOverCursor(1);
	SetAnimated();
	SetTimeless(true);

	// implement sprite framerates
	SetFrameRate(0);
	m_nLastUpdate = 0;
}

CBagSpriteObject::~CBagSpriteObject() {
	Detach();
}

ErrorCode CBagSpriteObject::Attach() {
	// If it's not already attached
	if (!IsAttached()) {
		// Could not already have a sprite
		Assert(m_xSprite == nullptr);

		if ((m_xSprite = new CBofSprite()) != nullptr) {
			if (m_xSprite->LoadSprite(GetFileName(), GetCels()) != false && (m_xSprite->Width() != 0) && (m_xSprite->Height() != 0)) {
				if (IsTransparent()) {
					int nMaskColor = CBagel::GetBagApp()->GetChromaColor();

					m_xSprite->SetMaskColor(nMaskColor);
				}

				// Set animated of the sprite to be the same as it's parent
				m_xSprite->SetAnimated(IsAnimated());

				CBofPoint p = CBagObject::GetPosition();

				if (p.x == -1 && p.y == -1) // Fixed to allow for [0,0] positioning
					SetFloating();
				else
					m_xSprite->SetPosition(p.x, p.y);

				SetProperty("CURR_CEL", GetState());

				// This might add something to the PDA, make sure it gets redrawn.
				CBagStorageDevWnd *pMainWin = (CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev());

				if (pMainWin != nullptr) {
					pMainWin->SetPreFilterPan(true);
				}
			} else {
				ReportError(ERR_FOPEN, "Could Not Open Sprite: %s", m_xSprite->GetFileName());
			}

		} else {
			ReportError(ERR_MEMORY, "Could not allocate sprite");
		}
	}

	return CBagObject::Attach();
}

ErrorCode CBagSpriteObject::Detach() {
	if (m_xSprite != nullptr) {
		delete m_xSprite;
		m_xSprite = nullptr;
	}
	return CBagObject::Detach();
}

void CBagSpriteObject::SetCels(int nCels) {
	m_nCels = nCels;
	if (m_xSprite)
		m_xSprite->SetupCels(nCels);
}

void CBagSpriteObject::SetPosition(const CBofPoint &pos) {
	CBagObject::SetPosition(pos);
	if (m_xSprite)
		m_xSprite->SetPosition(pos.x, pos.y);
}

CBofRect CBagSpriteObject::GetRect() {
	CBofPoint p = GetPosition();
	CBofSize s;
	if (m_xSprite)
		s = m_xSprite->GetSize();
	return CBofRect(p, s);
}

//
// SetInfo(bof_ifstream& istr)
//   Takes in info and then removes the relative information and returns the info
//   without the relevant info.
PARSE_CODES CBagSpriteObject::SetInfo(bof_ifstream &istr) {
	bool nObjectUpdated = false;

	while (!istr.eof()) {
		istr.EatWhite(); // not sure why this WAS NOT here.

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
		}
		break;
		case '#': {
			int curs;
			istr.Get();
			GetIntFromStream(istr, curs);
			SetWieldCursor(curs);
			nObjectUpdated = true;
		}
		break;
		case 'N': { // NOANIM
			char szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256); // performance improvement
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("NOANIM")) {
				istr.EatWhite();
				SetAnimated(false);
				nObjectUpdated = true;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
		}
		break;

		// handle a maximum framerate...
		case 'F': { // NOANIM
			char szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256);
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("FRAMERATE")) {
				int nFrameRate;
				istr.EatWhite();
				GetIntFromStream(istr, nFrameRate);

				// The framerate is expressed in frames/second, so do some division
				// here to store the number of milliseconds.
				SetFrameRate(1000 / nFrameRate);

				nObjectUpdated = true;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
		}
		break;
		//
		//  no match return from funtion
		//
		default: {
			PARSE_CODES rc = CBagObject::SetInfo(istr);
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

ErrorCode CBagSpriteObject::Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect * /*pSrcRect*/, int) {
	if (m_xSprite) {
		bool b = true;
		int nFrameInterval = GetFrameRate();

		if (nFrameInterval != 0) {
			uint32 nCurTime = GetTimer();
			if (nCurTime > m_nLastUpdate + nFrameInterval) {
				m_xSprite->SetBlockAdvance(false);
				m_nLastUpdate = nCurTime;
			} else {
				m_xSprite->SetBlockAdvance(true);
			}
		}

		b = m_xSprite->PaintSprite(pBmp, pt.x, pt.y);

		// Don't have to redraw this item...
		// SetDirty (false);

		if (!b)
			return ERR_UNKNOWN;
	}
	return ERR_NONE;
}

ErrorCode CBagSpriteObject::Update(CBofWindow *pWnd, CBofPoint pt, CBofRect *, int) {
	if (m_xSprite) {
		bool b = m_xSprite->PaintSprite(pWnd, pt.x, pt.y);

		// don't have to redraw this item...
		// SetDirty (false);

		if (!b)
			return ERR_UNKNOWN;
	}
	return ERR_NONE;
}

bool CBagSpriteObject::IsInside(const CBofPoint &xPoint) {
	if (m_xSprite && GetRect().PtInRect(xPoint)) {
		if (IsTransparent()) {
			int x = xPoint.x - GetRect().left;
			int y = xPoint.y - GetRect().top;
			int c = m_xSprite->ReadPixel(x, y);
			int d = m_xSprite->GetMaskColor();
			return (c != d);
		}

		return true;
	}
	return false;
}

void CBagSpriteObject::SetProperty(const CBofString &sProp, int nVal) {
	if (!sProp.Find("STATE")) {
		SetState(nVal);
		if (m_xSprite)
			m_xSprite->SetCel(nVal);
	} else if (!sProp.Find("CURR_CEL")) {
		SetState(nVal);
		if (m_xSprite)
			m_xSprite->SetCel(nVal);
	} else
		CBagObject::SetProperty(sProp, nVal);
}

int CBagSpriteObject::GetProperty(const CBofString &sProp) {
	if (!sProp.Find("CURR_CEL")) {
		if (m_xSprite) {
			return m_xSprite->GetCelIndex();
		}
		return 0;
	}

	return CBagObject::GetProperty(sProp);
}

void CBagSpriteObject::SetAnimated(bool b) {
	m_bAnimated = b;
	if (m_xSprite)
		m_xSprite->SetAnimated(b);
}

} // namespace Bagel
