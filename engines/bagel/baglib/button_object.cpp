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

#include "bagel/baglib/button_object.h"
#include "bagel/baglib/bagel.h"
#include "bagel/baglib/pan_window.h"

namespace Bagel {

CBagButtonObject::CBagButtonObject() {
	m_xObjType = BUTTONOBJ;
	m_xButtonType = PUSH;

	SetState(0); // Set to first cel

	m_bActive = FALSE;
	m_bActiveDown = FALSE;
	m_bActiveUp = FALSE;

	m_nNumPos = 0;
	m_bDragging = FALSE;
	SetCallBack(nullptr, nullptr);
	SetAlwaysUpdate(TRUE);

	SetTimeless(TRUE);
}

CBagButtonObject::~CBagButtonObject() {
	Detach();
}

ERROR_CODE CBagButtonObject::Attach() {
	ERROR_CODE rc = CBagSpriteObject::Attach();

	if (GetSprite()) {
		GetSprite()->SetAnimated(FALSE);
	}

	if (m_xButtonType == VLEVER || m_xButtonType == HLEVER) {
		m_MidPoint.x = GetRect().TopLeft().x + (GetRect().Width() / 2);
		m_MidPoint.y = GetRect().TopLeft().y + (GetRect().Height() / 2);
	}

	if (GetSprite()->GetCelCount() == 1 && m_xButtonType != SLIDER) { // Only given down state
		SetVisible(FALSE);
	}
	// if this is a slider button make sure it is in the correct position
	if (m_xButtonType == SLIDER) {
		CBofPoint NewPoint = GetPosition();
		int xIncrement = m_SlideRect.Width() / (m_nNumPos - 1);
		NewPoint.x = m_SlideRect.left + (GetState() * xIncrement);
		SetPosition(NewPoint);
	}

	return rc;
}

ERROR_CODE CBagButtonObject::Detach() {
	return CBagSpriteObject::Detach();
}

extern BOOL g_bNoMenu;

BOOL CBagButtonObject::RunObject() {
	// Reset wield
	g_bNoMenu = FALSE;

	if (m_xButtonType == PUSH) {

		if (m_bActive && !m_bActiveUp) {
			m_bActiveUp = TRUE;
		}
		if (GetSprite() && (GetSprite()->GetCelCount() == 1)) {
			// Only given down state
			SetVisible(FALSE);
		}

		SetState(0); // Set to first cel
	}

	RunCallBack();

	return CBagObject::RunObject();
}

void CBagButtonObject::OnLButtonDown(UINT /*nFlags*/, CBofPoint *xPoint, void *) {
	if (m_xButtonType == PUSH) {
		if (!m_bActive && !m_bActiveDown) {
			m_bActiveDown = TRUE;
			m_bActive = TRUE;
		}
		if (GetSprite() && (GetSprite()->GetCelCount() == 1)) {
			// Only given down state
			SetVisible();
		}

		SetState(1); // Set to clicked down

	} else if (m_xButtonType == HLEVER || m_xButtonType == VLEVER) {
		if (!m_bActiveDown && !m_bActiveUp) {
			if ((m_xButtonType == HLEVER && xPoint->x > m_MidPoint.x) || // right of midpoint
			        (m_xButtonType == VLEVER && xPoint->y > m_MidPoint.y)) { // below midpoint
				m_bActiveDown = TRUE;
			} else {
				m_bActiveUp = TRUE;
			}
		}

	} else if (m_xButtonType == SLIDER) {
		m_bDragging = TRUE;
	}

	SetDirty();
}

void CBagButtonObject::OnLButtonUp(UINT nFlags, CBofPoint *xPoint, void *info) {
	CBagStorageDevWnd *pMainWin = (CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev());

	if (pMainWin != nullptr) {
		pMainWin->SetPreFilterPan(TRUE);
	}

	if ((m_xButtonType == SLIDER) && m_bDragging) {

		// Snap to place
		CBofPoint mLoc;
		CBofPoint NewPoint = GetPosition();

		// Get the mouse point relative to the pan window
		CBagPanWindow *pWnd = (CBagPanWindow *)info;
		CBofRect r = pWnd->GetSlideBitmap()->GetCurrView();

		mLoc.x = xPoint->x + r.left - pWnd->GetViewPortPos().x;
		mLoc.y = xPoint->y + r.top - pWnd->GetViewPortPos().y;

		int xIncrement = m_SlideRect.Width() / (m_nNumPos - 1);

		int slidePos = m_SlideRect.left;
		int i;
		for (i = 0; (i < m_nNumPos) && (slidePos < mLoc.x); i++)
			slidePos = m_SlideRect.left + (i * xIncrement);

		// We Went to far
		i--;

		// Find the previous position was closer to the mouse
		if ((i > 0) && (slidePos - mLoc.x > mLoc.x - (slidePos - xIncrement)))
			i--; // Go back one

		NewPoint.x = m_SlideRect.left + (i * xIncrement);
		if (NewPoint.x < m_SlideRect.left) {
			NewPoint.x = m_SlideRect.left;
		} else if (NewPoint.x > m_SlideRect.right) {
			NewPoint.x = m_SlideRect.right;
		}

		SetPosition(NewPoint);
		if (i < 0) {
			i = 0;
		} else if (i >= m_nNumPos) {
			i = m_nNumPos - 1;
		}

		SetState(i);
		m_bDragging = FALSE;

	} else if (m_xButtonType == CHECKBOX) {

		if (!m_bActive) {
			m_bActive = TRUE;
			m_bActiveDown = !m_bActiveDown;

			if (GetSprite() && (GetSprite()->GetCelCount() == 1)) { // Only given down state
				SetVisible(m_bActiveDown);
				m_bActive = FALSE;
			}

			if (m_bActiveDown)
				SetState(1);
			else
				SetState(0);
		}
	}

	SetDirty();

	CBagSpriteObject::OnLButtonUp(nFlags, xPoint, info);
}

BOOL CBagButtonObject::OnMouseMove(UINT /*nFlags*/, CBofPoint xPoint, void *info) {
	CBagStorageDevWnd *pMainWin = (CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev());

	if (m_xButtonType == SLIDER && m_bDragging) {
		if (!m_SlideRect.IsRectEmpty()) {
			if (pMainWin != nullptr) {
				pMainWin->SetPreFilterPan(TRUE);
			}
			SetDirty();

			CBofPoint mLoc;
			CBofPoint NewPoint = GetPosition();
			CBagPanWindow *pWnd = (CBagPanWindow *)info;
			CBofRect r = pWnd->GetSlideBitmap()->GetCurrView();

			mLoc.x = xPoint.x + r.left - pWnd->GetViewPortPos().x;
			mLoc.y = xPoint.y + r.top - pWnd->GetViewPortPos().y;

			int NewXPos = mLoc.x;

			// Constrict Dragging to width of slidebar
			if (NewXPos > m_SlideRect.right)
				NewXPos = m_SlideRect.right;
			if (NewXPos < m_SlideRect.left)
				NewXPos = m_SlideRect.left;

			NewPoint.x = NewXPos;
			SetPosition(NewPoint);

			// We need to set the state here as well as LButtonUP
			// because there is a chance we won't get it
			int xIncrement = m_SlideRect.Width() / (m_nNumPos - 1);
			int i = (NewPoint.x - m_SlideRect.left) / xIncrement;
			SetState(i);
		}
	}

	if (m_xButtonType == PUSH) {
		if (GetSprite() && (GetSprite()->GetCelCount() > 1)) {
			if (!this->GetRect().PtInRect(xPoint) &&
			        m_bActive && !m_bActiveUp) {
				m_bActiveUp = TRUE;
			}
		}
		if (GetSprite() && (GetSprite()->GetCelCount() == 1)) { // Only given down state
			SetVisible(FALSE);
		}

		SetState(0); // Set to first cel
	}

	return TRUE;
}

ERROR_CODE CBagButtonObject::Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect, INT nMaskColor) {
	BOOL bDirty = FALSE;

	if (m_xButtonType == PUSH) {

		if (GetSprite() && (GetSprite()->GetCelCount() > 1)) {

			if (m_bActive) { // If the button is doing something
				if (m_bActiveDown) {
					GetSprite()->NextCel(); //  increment frame
					// If this is animated, the bring it back up immediately
					if (GetSprite()->GetCelIndex() == GetSprite()->GetCelCount() - 1 || GetSprite()->GetAnimated()) {
						m_bActiveDown = FALSE;
					}
				} else if (m_bActiveUp) {   // else (going back up)
					GetSprite()->PrevCel(); //  decrement frame
					// If this is animated, the let it go immediately
					if (GetSprite()->GetCelIndex() == 0 || GetSprite()->GetAnimated()) {
						m_bActiveUp = FALSE;
						m_bActive = FALSE;
					}
				}
			}
		}

	} else if (m_xButtonType == CHECKBOX) {
		if (GetSprite() && (GetSprite()->GetCelCount() > 1) && m_bActive) {
			if (m_bActiveDown) {
				GetSprite()->NextCel(); //  increment frame
				if (GetSprite()->GetCelIndex() == GetSprite()->GetCelCount() - 1) {
					m_bActive = FALSE;
				}
			} else {                    // else (going back up)
				GetSprite()->PrevCel(); //  decrement frame
				if (GetSprite()->GetCelIndex() == 0) {
					m_bActive = FALSE;
				}
			}
		}
		// for checkboxes, we have to make sure that they are always
		// redrawn as they will otherwise return to their state on the pan (in the
		// background of the closeup).
		bDirty = TRUE;

	} else if (m_xButtonType == HLEVER || m_xButtonType == VLEVER) {

		if (GetSprite() && (GetSprite()->GetCelCount() > 1)) {
			if (m_bActiveDown) {
				if (GetSprite()->GetCelIndex() < (GetSprite()->GetCelCount() - 1))
					GetSprite()->NextCel();
				m_bActiveDown = FALSE;
			} else if (m_bActiveUp) {
				if (GetSprite()->GetCelIndex() > 0)
					GetSprite()->PrevCel();
				m_bActiveUp = FALSE;
			}

			SetState(GetSprite()->GetCelIndex());
			//          RunCallBack();
		}
	}

	if (GetSprite() && ((GetSprite()->GetCelCount() > 1) || IsVisible())) {
		ERROR_CODE err = CBagSpriteObject::Update(pBmp, pt, pSrcRect, nMaskColor);
		SetDirty(bDirty);
		return err;
	}

	return m_errCode;
}

VOID CBagButtonObject::SetSize(const CBofSize &xSize) {
	if (m_xButtonType == SLIDER)
		m_SlideRect = CBofRect(GetPosition(), xSize);

	CBagSpriteObject::SetSize(xSize);
}

PARSE_CODES CBagButtonObject::SetInfo(bof_ifstream &istr) {
	int nChanged;
	BOOL nObjectUpdated = FALSE;
	char ch;

	while (!istr.eof()) {
		nChanged = 0;

		switch (ch = (char)istr.peek()) {
		//
		//  +n  - n number of slides in sprite
		//
		case '+': {
			int cels;
			istr.Get();
			GetIntFromStream(istr, cels);

			if (m_xButtonType == SLIDER)
				m_nNumPos = cels;
			else
				SetCels(cels);
			nChanged++;
			nObjectUpdated = TRUE;
			break;
		}

		// handle a maximum framerate...
		case 'F': {
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
			break;
		}

		//
		//  AS [LINK|CLOSEUP]  - how to run the link
		//
		case 'A': {
			CHAR szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256);
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("AS")) {
				istr.EatWhite();
				GetAlphaNumFromStream(istr, sStr);
				if (!sStr.Find("PUSH")) {
					m_xButtonType = PUSH;
					nChanged++;
					nObjectUpdated = TRUE;
				} else if (!sStr.Find("CHECKBOX")) {
					m_xButtonType = CHECKBOX;
					nChanged++;
					nObjectUpdated = TRUE;
				} else if (!sStr.Find("HLEVER")) {
					m_xButtonType = HLEVER;
					nChanged++;
					nObjectUpdated = TRUE;
				} else if (!sStr.Find("VLEVER")) {
					m_xButtonType = VLEVER;
					nChanged++;
					nObjectUpdated = TRUE;
				} else if (!sStr.Find("DIAL")) {
					m_xButtonType = DIAL;
					nChanged++;
					nObjectUpdated = TRUE;
				} else if (!sStr.Find("SLIDER")) {
					m_xButtonType = SLIDER;
					nChanged++;
					nObjectUpdated = TRUE;
				} else {
					PutbackStringOnStream(istr, sStr);
					PutbackStringOnStream(istr, "AS ");
				}
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;
		}

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
			break;
		}
		} // end switch
	} // end while

	return PARSING_DONE;
}

VOID CBagButtonObject::SetProperty(const CBofString &sProp, int nVal) {
	if (!sProp.Find("STATE")) {
		if (GetSprite()) {
			if (m_xButtonType == CHECKBOX) {
				m_bActive = TRUE;
				if (nVal == 0)
					m_bActiveDown = FALSE;
				else
					m_bActiveDown = TRUE;

				if (GetSprite() && (GetSprite()->GetCelCount() == 1)) { // Only given down state
					SetVisible(m_bActiveDown);
					m_bActive = FALSE;
				}

				if (nVal == 0)
					SetState(0);
				else
					SetState(1);
			} else {
				if (m_xButtonType == SLIDER) {
					CBofPoint cPos;
					cPos = GetPosition();

					cPos.x = m_SlideRect.left + (nVal * (m_SlideRect.Width() / (m_nNumPos - 1)));
					SetPosition(cPos);
					SetDirty(TRUE);
				}
				SetState(nVal);
				GetSprite()->SetCel(nVal);
			}
		}
	} else if (!sProp.Find("CURR_CEL")) {
		SetState(nVal);
		if (GetSprite())
			GetSprite()->SetCel(nVal);
	} else
		CBagObject::SetProperty(sProp, nVal);
}

INT CBagButtonObject::GetProperty(const CBofString &sProp) {
	if (!sProp.Find("CURR_CEL")) {
		if (GetSprite()) {
			return GetSprite()->GetCelIndex();
		}
		return 0;
	} else {
		return CBagObject::GetProperty(sProp);
	}
}

} // namespace Bagel
