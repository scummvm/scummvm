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

#include "bagel/baglib/base_pda.h"
#include "bagel/baglib/event_sdev.h"
#include "bagel/dialogs/opt_window.h"
#include "bagel/baglib/log_msg.h"
#include "bagel/baglib/bagel.h"
#include "bagel/baglib/moo.h"
#include "bagel/baglib/storage_dev_win.h"
#include "bagel/baglib/wield.h"

namespace Bagel {

// Init static members
PDAMODE SBBasePda::m_ePdaMode;
PDAPOS SBBasePda::m_ePDAPos;
PDAMODE SBBasePda::m_eHoldMode;

const CBofRect xEmptyRect(0, 0, 0, 0);

void SBBasePda::initialize() {
	m_ePdaMode = NOMODE;
	m_ePDAPos = UNINITIALIZED;
	m_eHoldMode = NOMODE;
}

SBBasePda::SBBasePda(CBofWindow *pParent, const CBofRect & /* xRect*/, bool bActivated) {
	m_bActivated = bActivated;
	m_bActivating = false;
	m_nMoveDist = 13;
	m_nNumMoves = 10;

	m_pParent = pParent;
	m_xMooWnd = nullptr;
	m_xInvWnd = nullptr;
	m_xMapWnd = nullptr;
	m_xLogWnd = nullptr;

	m_xCurDisplay = nullptr;

	// Other classes need to know if we're zoomed
	SetZoomed(false);
}

SBBasePda::~SBBasePda() {
	m_xInvWnd = nullptr;
	m_xMapWnd = nullptr;
	m_xLogWnd = nullptr;
	m_xMooWnd = nullptr;
}

bool SBBasePda::HideCurDisplay() {
	SynchronizePDAState();

	if (m_xCurDisplay) {
		// If we have an inventory window
		m_xCurDisplay->SetVisible(false);

		// Hold this info.
		m_xHoldDisplay = m_xCurDisplay;
		m_eHoldMode = SBBasePda::m_ePdaMode;

		m_xCurDisplay = nullptr;
		SBBasePda::m_ePdaMode = NOMODE;
		SetPDAState();
		return true;
	}

	return false;
}

bool SBBasePda::RestoreCurDisplay() {
	SynchronizePDAState();

	// If we saved a display
	if (m_xHoldDisplay) {
		// Restore the display info.
		m_xCurDisplay = m_xHoldDisplay;
		SBBasePda::m_ePdaMode = m_eHoldMode;

		m_xCurDisplay->SetVisible(true);
		SetPDAState();
		return true;
	}

	return false;
}

bool SBBasePda::HideMovie() {
	SynchronizePDAState();

	if (m_xMooWnd) {  // if we have an inventory window

		m_xMooWnd->SetVisible(false);
		m_xCurDisplay = nullptr;
		SBBasePda::m_ePdaMode = NOMODE;
		SetPDAState();
		return true;
	}

	return false;
}

bool SBBasePda::ShowMovie() {
	SynchronizePDAState();

	// if we're already playing a movie, then return false
	if (m_ePdaMode == MOOMODE) {
		return false;
	}

	if (m_xMooWnd) {
		if (m_xCurDisplay)
			m_xCurDisplay->SetVisible(false);       // Turn off the current display

		// Save the current PDA mode so we can return to it when done.
		((CBagMoo *)m_xMooWnd)->SavePDAMode(m_ePdaMode);
		((CBagMoo *)m_xMooWnd)->SavePDAPosition(m_ePDAPos);

		m_xMooWnd->SetVisible(true);                // Turn on the inventory

		// set the current display object
		m_xCurDisplay = m_xMooWnd;                 // Set the current display = Inventory
		SBBasePda::m_ePdaMode = MOOMODE;
		SetPDAState();

		// Set default state of movie to don't deactivate PDA
		SetDeactivate(false);

		return true;
	}
	return false;
}

void SBBasePda::StopMovie(bool bResetPDA) {
	if (m_xMooWnd && m_xMooWnd == m_xCurDisplay) {
		((CBagMoo *)m_xMooWnd)->StopMovie(bResetPDA);
	}
}

bool SBBasePda::SetMovie(CBofString &s) {
	if (m_xMooWnd) {
		((CBagMoo *)m_xMooWnd)->SetPDAMovie(s);
		return true;
	}
	return false;
}

bool SBBasePda::HideInventory() {
	SynchronizePDAState();

	// if we have an inventory window
	if (m_xInvWnd) {
		m_xInvWnd->SetVisible(false);
		m_xCurDisplay = nullptr;
		SBBasePda::m_ePdaMode = NOMODE;
		SetPDAState();
		return true;
	}

	return false;
}

bool SBBasePda::ShowInventory() {
	SynchronizePDAState();
	StopMovie(false);		// if a movie is playing, then stop it.

	// If we have an inventory window
	if (m_xInvWnd) {
		if (m_xCurDisplay)
			m_xCurDisplay->SetVisible(false);	// Turn off the current display

		m_xInvWnd->SetVisible(true);			// Turn on the inventory

		// set the current display object
		m_xCurDisplay = m_xInvWnd;				// Set the current display = Inventory
		SBBasePda::m_ePdaMode = INVMODE;
		SetPDAState();
		return true;
	}

	return false;
}

bool SBBasePda::ShowMap() {
	SynchronizePDAState();
	StopMovie(false);		// If a movie is playing, then stop it.

	// if we have a map window
	if (m_xMapWnd)  {
		if (m_xCurDisplay)
			m_xCurDisplay->SetVisible(false);	// Turn off the current display

		m_xMapWnd->SetVisible(true);			// Turn on the map

		// set the current display object
		m_xCurDisplay = m_xMapWnd;				// Set the current display = Map
		SBBasePda::m_ePdaMode = MAPMODE;
		m_xMapWnd->AttachActiveObjects();
		SetPDAState();

		return true;
	}

	return false;
}

bool SBBasePda::HideMap() {
	SynchronizePDAState();

	// if we have a map window
	if (m_xMapWnd) {
		// Turn off the Map
		m_xMapWnd->SetVisible(false);

		// set the current display to nullptr
		m_xCurDisplay = nullptr;
		SetPDAState();
		SBBasePda::m_ePdaMode = NOMODE;
		return true;
	}

	return false;
}

bool SBBasePda::ShowLog() {
	if (CBagStorageDevWnd::m_pEvtSDev != nullptr) {
		CBagStorageDevWnd::m_pEvtSDev->EvaluateExpressions();
	}

	SynchronizePDAState();
	StopMovie(false);		// If a movie is playing, then stop it.

	// if we have a map window
	if (m_xLogWnd) {
		if (m_xCurDisplay)
			m_xCurDisplay->SetVisible(false);	// Turn off the current display

		m_xLogWnd->SetVisible(true);			// Turn on the map

		// Set the current display object
		m_xCurDisplay = m_xLogWnd;				// Set the current display = Map
		SBBasePda::m_ePdaMode = LOGMODE;
		m_xLogWnd->AttachActiveObjects();
		SetPDAState();

		return true;
	}

	return false;
}

bool SBBasePda::MsgLight() {
	// If we have a map window
	if (m_xLogWnd) {
		((CBagLog *)m_xLogWnd)->PlayMsgQue();
	}

	return true;
}

void *SBBasePda::fPdaButtonHandler(int nRefId, void *pvInfo) {
	Assert(pvInfo != nullptr);

	SBBasePda *pPDA;

	pPDA = (SBBasePda *)pvInfo;

	switch (nRefId) {
	case MAP:
		pPDA->ShowMap();
		break;

	case STASH:
		pPDA->ShowInventory();
		break;

	case ZOOM:
		pPDA->Zoom();
		break;

	case SYSTEM: {
		CBagel *pApp;
		if ((pApp = CBagel::GetBagApp()) != nullptr) {

			CBagMasterWin *pWnd;
			if ((pWnd = pApp->GetMasterWnd()) != nullptr) {
				pWnd->PostUserMessage(WM_SHOWSYSTEMDLG, 0);
			}
		}
		break;
	}

	case LOG:
		pPDA->ShowLog();
		break;

	case OFF:
		pPDA->Deactivate();
		break;

	case MSGLIGHT:
		pPDA->MsgLight();
		break;

	default:
		break;
	}

	return pvInfo;
}

void SBBasePda::SynchronizePDAState() {
	if (m_ePDAPos == PDADOWN && IsActivated()) {
		Deactivate();
	} else {
		if (m_ePDAPos == PDAUP && IsActivated() == false) {
			Activate();
		}
	}
}

void SBBasePda::SetPDAState() {
	const char *pPDAMode;
	const char *pPDAPos;

	CBagVar *pVar = VARMNGR->GetVariable("INBAR");

	if (pVar != nullptr) {
		pPDAMode = "BARPDAMODE";        // Defined as global
		pPDAPos = "BARPDAPOSITION";     // Defined as global
	} else {
		pPDAMode = "PDAMODE";
		pPDAPos = "PDAPOSITION";
	}

	// Save the pda state and position
	pVar = VARMNGR->GetVariable(pPDAMode);
	if (pVar != nullptr) {
		switch (m_ePdaMode) {
		case NOMODE:
			pVar->SetValue("NOMODE");
			break;
		case MAPMODE:
			pVar->SetValue("MAPMODE");
			break;
		case INVMODE:
			pVar->SetValue("INVMODE");
			break;
		case LOGMODE:
			pVar->SetValue("LOGMODE");
			break;
		case MOOMODE:
			pVar->SetValue("MOOMODE");
			break;
		}
	}

	pVar = VARMNGR->GetVariable(pPDAPos);
	if (pVar != nullptr) {
		switch (m_ePDAPos) {
		case PDAUP:
			pVar->SetValue("UP");
			break;
		case PDADOWN:
			pVar->SetValue("DOWN");
			break;
		default:
			break;
		}
	}
}

void SBBasePda::GetPDAState() {
	char szLocalBuff[256];
	CBofString sPDAState(szLocalBuff, 256);
	const char *pPDAMode;
	const char *pPDAPos;

	CBagVar *pVar = VARMNGR->GetVariable("INBAR");

	if (pVar != nullptr) {
		pPDAMode = "BARPDAMODE";        // Defined as global
		pPDAPos = "BARPDAPOSITION";     // Defined as global
	} else {
		pPDAMode = "PDAMODE";
		pPDAPos = "PDAPOSITION";
	}

	pVar = VARMNGR->GetVariable(pPDAMode);

	if (pVar) {
		sPDAState = pVar->GetValue();
		// Now set the internal PDA state based on this info.
		// If we saved during a movie, then restore to map mode.
		if (sPDAState.Find("MAP") != -1 || sPDAState.Find("MOO") != -1) {
			m_ePdaMode = MAPMODE;
		} else {
			if (sPDAState.Find("INV") != -1) {
				m_ePdaMode = INVMODE;
			} else {
				if (sPDAState.Find("LOG") != -1) {
					m_ePdaMode = LOGMODE;
				} else {
					if (sPDAState.Find("MOO") != -1) {
						m_ePdaMode = MOOMODE;
					} else {
						m_ePdaMode = NOMODE;
					}
				}
			}
		}
	}

	// Get the PDA up/down position
	pVar = VARMNGR->GetVariable(pPDAPos);
	if (pVar) {
		sPDAState = pVar->GetValue();
		if (sPDAState.Find("UP") != -1) {
			m_ePDAPos = PDAUP;
		} else {
			m_ePDAPos = PDADOWN;
		}
	}
}

#define NULLCURSOR 0
#define HANDCURSOR 1

int SBBasePda::GetProperCursor(const CBofPoint &xPoint, CBofRect &pdaRect) {
	int nWieldCursor = CBagWield::GetWieldCursor();
	CBofRect cRect;
	CBofList<CBagObject *> *pList;
	CBofPoint pt;

	// Assume can't click
	int nCursorID = NULLCURSOR;

	// If we're in the map, return the nullptr cursor, if on the pda but not in the
	// map window, return the hand.  Same rules for nomode.
	switch (m_ePdaMode) {
	case MAPMODE:
	case NOMODE:
	case MOOMODE:
		if (m_xMapWnd) {
			cRect = m_xMapWnd->GetRect() + pdaRect.TopLeft();
			if (cRect.PtInRect(xPoint)) {
				if (nWieldCursor >= 0) {
					return nWieldCursor;
				}
				return NULLCURSOR;
			}
			return HANDCURSOR;
		}
		break;

	case INVMODE:
	case LOGMODE:
		// Make sure our cur display is synchronized.
		Assert(m_ePdaMode == INVMODE ? m_xCurDisplay == m_xInvWnd : m_xCurDisplay == m_xLogWnd);

		// If we have a display, then parouse it's list and see if we're over something worth
		// mousedowning on.
		if (m_xCurDisplay) {
			CBagObject *pOverObj = nullptr;
			cRect = m_xCurDisplay->GetRect() + pdaRect.TopLeft();
			if (cRect.PtInRect(xPoint)) {
				CBofRect lRect;
				// Go through the list of inventory items and see if we're over any of them
				// in particular.
				if ((pList = m_xCurDisplay->GetObjectList()) != nullptr) {
					// Localize pda view rect
					cRect = m_xCurDisplay->GetRect() + pdaRect.TopLeft();

					int nCount = pList->GetCount();
					for (int i = 0; i < nCount; ++i) {
						CBagObject *pObj = pList->GetNodeItem(i);
						if (pObj->IsActive()) {
							lRect = pObj->GetRect() + cRect.TopLeft();      // localize icon rectangle
							if (lRect.PtInRect(xPoint)) {
								pOverObj = pObj;
							}
						}
					}
				}

				if (m_ePdaMode == LOGMODE) {
					if (pOverObj) {
						return pOverObj->GetOverCursor();
					}
					if (nWieldCursor >= 0) {
						return nWieldCursor;
					}
				}

				if (m_ePdaMode == INVMODE) {
					if (nWieldCursor >= 0) {
						return nWieldCursor;
					}
					if (pOverObj) {
						return pOverObj->GetOverCursor();
					}
				}

				return NULLCURSOR;
			}

			return HANDCURSOR;
		}
	}

	return nCursorID;
}

CBofRect SBBasePda::GetViewRect() {
	return (m_xCurDisplay == nullptr ? xEmptyRect : m_xCurDisplay->GetRect());
}

ErrorCode SBBasePda::AttachActiveObjects() {
	if (CBagStorageDevWnd::m_pEvtSDev != nullptr) {
		CBagStorageDevWnd::m_pEvtSDev->EvaluateExpressions();
	}

	if (m_xMooWnd)
		m_xMooWnd->AttachActiveObjects();
	if (m_xInvWnd)
		m_xInvWnd->AttachActiveObjects();
	if (m_xMapWnd)
		m_xMapWnd->AttachActiveObjects();
	if (m_xLogWnd)
		m_xLogWnd->AttachActiveObjects();

	return ERR_NONE;
}

ErrorCode SBBasePda::DetachActiveObjects() {
	if (m_xMooWnd)
		m_xMooWnd->DetachActiveObjects();
	if (m_xInvWnd)
		m_xInvWnd->DetachActiveObjects();
	if (m_xMapWnd)
		m_xMapWnd->DetachActiveObjects();
	if (m_xLogWnd)
		m_xLogWnd->DetachActiveObjects();

	return ERR_NONE;
}

} // namespace Bagel
