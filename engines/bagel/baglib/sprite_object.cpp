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
	m_xSprite = NULL;
	m_nCels = 1;
	m_nWieldCursor = -1;

	// Transparent by default
	SetTransparent();
	SetOverCursor(1);
	SetAnimated();
	SetTimeless(TRUE);

	// jwl 10.24.96 implement sprite framerates
	SetFrameRate(0);
	m_nLastUpdate = 0;
}

CBagSpriteObject::~CBagSpriteObject() {
	Detach();
}

ERROR_CODE CBagSpriteObject::Attach() {
	// If it's not already attached
	//
	if (!IsAttached()) {

		// Could not already have a sprite
		Assert(m_xSprite == NULL);

		if ((m_xSprite = new CBofSprite()) != NULL) {

			if (m_xSprite->LoadSprite(GetFileName(), GetCels()) != NULL && (m_xSprite->Width() != 0) && (m_xSprite->Height() != 0)) {

				if (IsTransparent()) {
					int nMaskColor = CBagel::GetBagApp()->GetChromaColor();

					m_xSprite->SetMaskColor(nMaskColor);
				}

				// jwl 11.01.96 set animated of the sprite to be the same as it's parent
				m_xSprite->SetAnimated(IsAnimated());

				CBofPoint p = CBagObject::GetPosition();

				if (p.x == -1 && p.y == -1) // MDM 7/17 - Fixed to allow for [0,0] positioning
					SetFloating();
				else
					m_xSprite->SetPosition(p.x, p.y);

				SetProperty("CURR_CEL", GetState());

				// jwl 12.09.96 this might add something to the PDA, make sure it gets
				// redrawn.
				CBagStorageDevWnd *pMainWin = (CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev());

				if (pMainWin != NULL) {
					pMainWin->SetPreFilterPan(TRUE);
				}
			} else {
				ReportError(ERR_FOPEN, "Could Not Open Sprite: %s", m_xSprite->GetFileName());
			}

		} else {
			ReportError(ERR_MEMORY, "Could not allocate sprite");
		}
	}

	return (CBagObject::Attach());
}

ERROR_CODE CBagSpriteObject::Detach() {
	if (m_xSprite != NULL) {
		delete m_xSprite;
		m_xSprite = NULL;
	}
	return CBagObject::Detach();
}

VOID CBagSpriteObject::SetCels(int nCels) {
	m_nCels = nCels;
	if (m_xSprite)
		m_xSprite->SetupCels(nCels);
}

VOID CBagSpriteObject::SetPosition(CBofPoint &pos) {
	CBagObject::SetPosition(pos);
	if (m_xSprite)
		m_xSprite->SetPosition(pos.x, pos.y);
}

CBofRect CBagSpriteObject::GetRect(VOID) {
	CBofPoint p = GetPosition();
	CBofSize s;
	if (m_xSprite)
		s = m_xSprite->GetSize();
	return (CBofRect(p, s));
}

//
// SetInfo(bof_ifstream& istr)
//   Takes in info and then removes the relative information and returns the info
//   without the relevant info.
PARSE_CODES CBagSpriteObject::SetInfo(bof_ifstream &istr) {
	int nChanged;
	BOOL nObjectUpdated = FALSE;
	char ch;

	while (!istr.eof()) {
		nChanged = 0;

		istr.EatWhite(); // jwl 10.24.96 not sure why this WAS NOT here.

		switch (ch = (char)istr.peek()) {
		//
		//  +n  - n number of slides in sprite
		//
		case '+': {
			int cels;
			istr.Get();
			GetIntFromStream(istr, cels);
			SetCels(cels);
			nChanged++;
			nObjectUpdated = TRUE;
		}
		break;
		case '#': {
			int curs;
			istr.Get();
			GetIntFromStream(istr, curs);
			SetWieldCursor(curs);
			nChanged++;
			nObjectUpdated = TRUE;
		}
		break;
		case 'N': { // NOANIM
			CHAR szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256); // jwl 08.28.96 performance improvement
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("NOANIM")) {
				istr.EatWhite();
				SetAnimated(FALSE);
				nChanged++;
				nObjectUpdated = TRUE;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
		}
		break;

		// jwl 10.24.96 handle a maximum framerate...
		case 'F': { // NOANIM
			CHAR szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256);
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("FRAMERATE")) {
				INT nFrameRate;
				istr.EatWhite();
				GetIntFromStream(istr, nFrameRate);

				// the framerate is expressed in frames/second, so do some division
				// here to store the number of milliseconds.

				SetFrameRate(1000 / nFrameRate);

				nChanged++;
				nObjectUpdated = TRUE;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
		}
		break;
		//
		//  no match return from funtion
		//
		default: {
			PARSE_CODES rc;
			if ((rc = CBagObject::SetInfo(istr)) == PARSING_DONE) {
				return PARSING_DONE;
			} else if (rc == UPDATED_OBJECT) {
				nObjectUpdated = TRUE;
			} else if (!nChanged) { // rc==UNKNOWN_TOKEN
				if (nObjectUpdated)
					return UPDATED_OBJECT;
				else
					return UNKNOWN_TOKEN;
			}
		}
		break;
		}
	}

	return PARSING_DONE;
}

ERROR_CODE CBagSpriteObject::Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect * /*pSrcRect*/, INT) {
	if (m_xSprite) { // && IsVisible()
#if 0                // THIS HAS BEEN REMOVED BECAUSE BOFSPRITES CLIP ON THIER OWN
		if (pSrcRect) {
			CBofSize s = pBmp->GetSize();
			if ((m_xSprite->Width() + pt.x - s.cx) >= 0) {
				pt.x = s.cx - m_xSprite->Width();
			}
			if ((m_xSprite->Height() + pt.y - s.cy) >= 0) {
				pt.y = s.cy - m_xSprite->Height();
			}
		}
#endif
		// allow maximum framerates
		BOOL b = TRUE;
		INT nFrameInterval = GetFrameRate();

		if (nFrameInterval != 0) {
			DWORD nCurTime = GetTimer();
			if (nCurTime > m_nLastUpdate + nFrameInterval) {
				m_xSprite->SetBlockAdvance(FALSE);
				m_nLastUpdate = nCurTime;
			} else {
				m_xSprite->SetBlockAdvance(TRUE);
			}
		}

		b = m_xSprite->PaintSprite(pBmp, pt.x, pt.y);

		// jwl 10.11.96 don't have to redraw this item...
		// SetDirty (FALSE);

		if (!b)
			return (ERR_UNKNOWN);
	}
	return (ERR_NONE);
}

ERROR_CODE CBagSpriteObject::Update(CBofWindow *pWnd, CBofPoint pt, CBofRect *, INT) {
	if (m_xSprite) {
		BOOL b;
		b = m_xSprite->PaintSprite(pWnd, pt.x, pt.y);

		// jwl 10.11.96 don't have to redraw this item...
		// SetDirty (FALSE);
		if (!b)
			return (ERR_UNKNOWN);
	}
	return (ERR_NONE);
}

BOOL CBagSpriteObject::IsInside(const CBofPoint &xPoint) {
	if (m_xSprite && GetRect().PtInRect(xPoint)) {
		if (IsTransparent()) {
			int x = xPoint.x - GetRect().left;
			int y = xPoint.y - GetRect().top;
			int c = m_xSprite->ReadPixel(x, y);
			int d = m_xSprite->GetMaskColor();
			return (c != d);
		} else
			return TRUE;
	}
	return FALSE;
}

VOID CBagSpriteObject::SetProperty(const CBofString &sProp, int nVal) {
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

INT CBagSpriteObject::GetProperty(const CBofString &sProp) {
	if (!sProp.Find("CURR_CEL")) {
		if (m_xSprite) {
			return m_xSprite->GetCelIndex();
		}
		return 0;
	} else {
		return CBagObject::GetProperty(sProp);
	}
}

VOID CBagSpriteObject::SetAnimated(BOOL b) {
	m_bAnimated = b;
	if (m_xSprite)
		m_xSprite->SetAnimated(b);
}

} // namespace Bagel
