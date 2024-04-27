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
#include "bagel/baglib/master_win.h"
#include "bagel/baglib/pan_window.h"

namespace Bagel {

CBagButtonObject::CBagButtonObject() {
	m_xObjType = BUTTONOBJ;
	_buttonType = BTN_PUSH;

	// Set to first cel
	CBagObject::SetState(0);

	_active = false;
	_activeDown = false;
	_activeUp = false;

	_numPos = 0;
	_dragging = false;
	setCallBack(nullptr, nullptr);
	SetAlwaysUpdate(true);

	SetTimeless(true);
}

CBagButtonObject::~CBagButtonObject() {
	CBagButtonObject::detach();
}

ErrorCode CBagButtonObject::attach() {
	ErrorCode rc = CBagSpriteObject::attach();

	if (GetSprite()) {
		GetSprite()->SetAnimated(false);
	}

	if (_buttonType == BTN_VLEVER || _buttonType == BTN_HLEVER) {
		_midPoint.x = getRect().TopLeft().x + (getRect().Width() / 2);
		_midPoint.y = getRect().TopLeft().y + (getRect().Height() / 2);
	}

	if (GetSprite()->GetCelCount() == 1 && _buttonType != BTN_SLIDER) {
		// Only given down state
		SetVisible(false);
	}

	// If this is a slider button make sure it is in the correct position
	if (_buttonType == BTN_SLIDER) {
		CBofPoint NewPoint = GetPosition();
		int xIncrement = _slideRect.Width() / (_numPos - 1);
		NewPoint.x = _slideRect.left + (GetState() * xIncrement);
		SetPosition(NewPoint);
	}

	return rc;
}

ErrorCode CBagButtonObject::detach() {
	return CBagSpriteObject::detach();
}

extern bool g_bNoMenu;

bool CBagButtonObject::runObject() {
	// Reset wield
	g_bNoMenu = false;

	if (_buttonType == BTN_PUSH) {

		if (_active && !_activeUp) {
			_activeUp = true;
		}
		if (GetSprite() && (GetSprite()->GetCelCount() == 1)) {
			// Only given down state
			SetVisible(false);
		}

		// Set to first cel
		SetState(0);
	}

	runCallBack();

	return CBagObject::runObject();
}

void CBagButtonObject::onLButtonDown(uint32 /*nFlags*/, CBofPoint *point, void *) {
	if (_buttonType == BTN_PUSH) {
		if (!_active && !_activeDown) {
			_activeDown = true;
			_active = true;
		}
		if (GetSprite() && (GetSprite()->GetCelCount() == 1)) {
			// Only given down state
			SetVisible();
		}

		// Set to clicked down
		SetState(1);

	} else if (_buttonType == BTN_HLEVER || _buttonType == BTN_VLEVER) {
		if (!_activeDown && !_activeUp) {
			if ((_buttonType == BTN_HLEVER && point->x > _midPoint.x) || // right of midpoint
			        (_buttonType == BTN_VLEVER && point->y > _midPoint.y)) { // below midpoint
				_activeDown = true;
			} else {
				_activeUp = true;
			}
		}

	} else if (_buttonType == BTN_SLIDER) {
		_dragging = true;
	}

	SetDirty();
}

void CBagButtonObject::onLButtonUp(uint32 flags, CBofPoint *point, void *extraInfo) {
	CBagStorageDevWnd *pMainWin = (CBagel::getBagApp()->getMasterWnd()->GetCurrentStorageDev());

	if (pMainWin != nullptr) {
		pMainWin->SetPreFilterPan(true);
	}

	if ((_buttonType == BTN_SLIDER) && _dragging) {
		// Snap to place
		CBofPoint NewPoint = GetPosition();

		// Get the mouse point relative to the pan window
		CBagPanWindow *pWnd = (CBagPanWindow *)extraInfo;
		CBofRect r = pWnd->GetSlideBitmap()->GetCurrView();

		CBofPoint mLoc;
		mLoc.x = point->x + r.left - pWnd->GetViewPortPos().x;
		mLoc.y = point->y + r.top - pWnd->GetViewPortPos().y;

		int xIncrement = _slideRect.Width() / (_numPos - 1);

		int slidePos = _slideRect.left;
		int i;
		for (i = 0; (i < _numPos) && (slidePos < mLoc.x); i++)
			slidePos = _slideRect.left + (i * xIncrement);

		// We Went too far
		i--;

		// Find the previous position was closer to the mouse
		if ((i > 0) && (slidePos - mLoc.x > mLoc.x - (slidePos - xIncrement)))
			i--; // Go back one

		NewPoint.x = _slideRect.left + (i * xIncrement);
		if (NewPoint.x < _slideRect.left) {
			NewPoint.x = _slideRect.left;
		} else if (NewPoint.x > _slideRect.right) {
			NewPoint.x = _slideRect.right;
		}

		SetPosition(NewPoint);
		if (i < 0) {
			i = 0;
		} else if (i >= _numPos) {
			i = _numPos - 1;
		}

		SetState(i);
		_dragging = false;

	} else if (_buttonType == BTN_CHECKBOX) {

		if (!_active) {
			_active = true;
			_activeDown = !_activeDown;

			if (GetSprite() && (GetSprite()->GetCelCount() == 1)) {
				// Only given down state
				SetVisible(_activeDown);
				_active = false;
			}

			if (_activeDown)
				SetState(1);
			else
				SetState(0);
		}
	}

	SetDirty();

	CBagSpriteObject::onLButtonUp(flags, point, extraInfo);
}

bool CBagButtonObject::onMouseMove(uint32 /*nFlags*/, CBofPoint point, void *extraInfo) {
	CBagStorageDevWnd *pMainWin = (CBagel::getBagApp()->getMasterWnd()->GetCurrentStorageDev());

	if (_buttonType == BTN_SLIDER && _dragging) {
		if (!_slideRect.IsRectEmpty()) {
			if (pMainWin != nullptr) {
				pMainWin->SetPreFilterPan(true);
			}
			SetDirty();

			CBofPoint mLoc;
			CBofPoint NewPoint = GetPosition();
			CBagPanWindow *pWnd = (CBagPanWindow *)extraInfo;
			CBofRect r = pWnd->GetSlideBitmap()->GetCurrView();

			mLoc.x = point.x + r.left - pWnd->GetViewPortPos().x;
			mLoc.y = point.y + r.top - pWnd->GetViewPortPos().y;

			int NewXPos = mLoc.x;

			// Constrict Dragging to width of slidebar
			if (NewXPos > _slideRect.right)
				NewXPos = _slideRect.right;
			if (NewXPos < _slideRect.left)
				NewXPos = _slideRect.left;

			NewPoint.x = NewXPos;
			SetPosition(NewPoint);

			// We need to set the state here as well as LButtonUP
			// because there is a chance we won't get it
			int xIncrement = _slideRect.Width() / (_numPos - 1);
			int i = (NewPoint.x - _slideRect.left) / xIncrement;
			SetState(i);
		}
	}

	if (_buttonType == BTN_PUSH) {
		if (GetSprite() && (GetSprite()->GetCelCount() > 1)) {
			if (!this->getRect().PtInRect(point) &&
			        _active && !_activeUp) {
				_activeUp = true;
			}
		}
		if (GetSprite() && (GetSprite()->GetCelCount() == 1)) { // Only given down state
			SetVisible(false);
		}

		SetState(0); // Set to first cel
	}

	return true;
}

ErrorCode CBagButtonObject::update(CBofBitmap *bmp, CBofPoint pt, CBofRect *srcRect, int maskColor) {
	bool bDirty = false;

	if (_buttonType == BTN_PUSH) {

		if (GetSprite() && (GetSprite()->GetCelCount() > 1)) {

			if (_active) { // If the button is doing something
				if (_activeDown) {
					GetSprite()->NextCel(); //  Increment frame
					// If this is animated, the bring it back up immediately
					if (GetSprite()->GetCelIndex() == GetSprite()->GetCelCount() - 1 || GetSprite()->GetAnimated()) {
						_activeDown = false;
					}
				} else if (_activeUp) {   // else (going back up)
					GetSprite()->PrevCel(); //  decrement frame
					// If this is animated, the let it go immediately
					if (GetSprite()->GetCelIndex() == 0 || GetSprite()->GetAnimated()) {
						_activeUp = false;
						_active = false;
					}
				}
			}
		}

	} else if (_buttonType == BTN_CHECKBOX) {
		if (GetSprite() && (GetSprite()->GetCelCount() > 1) && _active) {
			if (_activeDown) {
				GetSprite()->NextCel(); // Increment frame
				if (GetSprite()->GetCelIndex() == GetSprite()->GetCelCount() - 1) {
					_active = false;
				}
			} else {                    // else (going back up)
				GetSprite()->PrevCel(); // decrement frame
				if (GetSprite()->GetCelIndex() == 0) {
					_active = false;
				}
			}
		}

		// For checkboxes, we have to make sure that they are always
		// redrawn as they will otherwise return to their state on the pan (in the
		// background of the closeup).
		bDirty = true;

	} else if (_buttonType == BTN_HLEVER || _buttonType == BTN_VLEVER) {

		if (GetSprite() && (GetSprite()->GetCelCount() > 1)) {
			if (_activeDown) {
				if (GetSprite()->GetCelIndex() < (GetSprite()->GetCelCount() - 1))
					GetSprite()->NextCel();
				_activeDown = false;
			} else if (_activeUp) {
				if (GetSprite()->GetCelIndex() > 0)
					GetSprite()->PrevCel();
				_activeUp = false;
			}

			SetState(GetSprite()->GetCelIndex());
		}
	}

	if (GetSprite() && ((GetSprite()->GetCelCount() > 1) || IsVisible())) {
		ErrorCode err = CBagSpriteObject::update(bmp, pt, srcRect, maskColor);
		SetDirty(bDirty);
		return err;
	}

	return m_errCode;
}

void CBagButtonObject::setSize(const CBofSize &size) {
	if (_buttonType == BTN_SLIDER)
		_slideRect = CBofRect(GetPosition(), size);

	CBagSpriteObject::setSize(size);
}

PARSE_CODES CBagButtonObject::setInfo(bof_ifstream &istr) {
	bool nObjectUpdated = false;

	while (!istr.eof()) {
		char ch = (char)istr.peek();
		switch (ch) {
		//
		//  +n  - n number of slides in sprite
		//
		case '+': {
			int cels;
			istr.Get();
			GetIntFromStream(istr, cels);

			if (_buttonType == BTN_SLIDER)
				_numPos = cels;
			else
				SetCels(cels);
			nObjectUpdated = true;
			break;
		}

		// Handle a maximum framerate...
		case 'F': {
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
			break;
		}

		//
		//  AS [LINK|CLOSEUP]  - how to run the link
		//
		case 'A': {
			char szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256);
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("AS")) {
				istr.EatWhite();
				GetAlphaNumFromStream(istr, sStr);
				if (!sStr.Find("PUSH")) {
					_buttonType = BTN_PUSH;
					nObjectUpdated = true;
				} else if (!sStr.Find("CHECKBOX")) {
					_buttonType = BTN_CHECKBOX;
					nObjectUpdated = true;
				} else if (!sStr.Find("HLEVER")) {
					_buttonType = BTN_HLEVER;
					nObjectUpdated = true;
				} else if (!sStr.Find("VLEVER")) {
					_buttonType = BTN_VLEVER;
					nObjectUpdated = true;
				} else if (!sStr.Find("DIAL")) {
					_buttonType = BTN_DIAL;
					nObjectUpdated = true;
				} else if (!sStr.Find("SLIDER")) {
					_buttonType = BTN_SLIDER;
					nObjectUpdated = true;
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
		// No match return from function
		//
		default: {
			PARSE_CODES rc = CBagObject::setInfo(istr);
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
			break;
		}
		} // end switch
	} // end while

	return PARSING_DONE;
}

void CBagButtonObject::setProperty(const CBofString &prop, int val) {
	if (!prop.Find("STATE")) {
		if (GetSprite()) {
			if (_buttonType == BTN_CHECKBOX) {
				_active = true;
				if (val == 0)
					_activeDown = false;
				else
					_activeDown = true;

				if (GetSprite() && (GetSprite()->GetCelCount() == 1)) { // Only given down state
					SetVisible(_activeDown);
					_active = false;
				}

				if (val == 0)
					SetState(0);
				else
					SetState(1);
			} else {
				if (_buttonType == BTN_SLIDER) {
					CBofPoint cPos = GetPosition();

					cPos.x = _slideRect.left + (val * (_slideRect.Width() / (_numPos - 1)));
					SetPosition(cPos);
					SetDirty(true);
				}
				SetState(val);
				GetSprite()->SetCel(val);
			}
		}
	} else if (!prop.Find("CURR_CEL")) {
		SetState(val);
		if (GetSprite())
			GetSprite()->SetCel(val);
	} else
		CBagObject::setProperty(prop, val);
}

int CBagButtonObject::getProperty(const CBofString &prop) {
	if (!prop.Find("CURR_CEL")) {
		if (GetSprite()) {
			return GetSprite()->GetCelIndex();
		}
		return 0;
	}

	return CBagObject::getProperty(prop);
}

} // namespace Bagel
