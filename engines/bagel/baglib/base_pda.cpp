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
#include "bagel/baglib/master_win.h"
#include "bagel/baglib/moo.h"
#include "bagel/baglib/storage_dev_win.h"
#include "bagel/baglib/wield.h"

namespace Bagel {

// Init static members
PDAMODE SBBasePda::_pdaMode;
PDAPOS SBBasePda::_pdaPos;
PDAMODE SBBasePda::_holdMode;

void SBBasePda::initialize() {
	_pdaMode = NOMODE;
	_pdaPos = UNINITIALIZED;
	_holdMode = NOMODE;
}

SBBasePda::SBBasePda(CBofWindow *parent, const CBofRect & /* xRect*/, bool activated) {
	_activated = activated;
	_activating = false;
	_moveDist = 13;
	_numMoves = 10;

	_parent = parent;
	_mooWnd = nullptr;
	_invWnd = nullptr;
	_mapWnd = nullptr;
	_logWnd = nullptr;

	_curDisplay = nullptr;

	// Other classes need to know if we're zoomed
	setZoomed(false);
}

SBBasePda::~SBBasePda() {
	_invWnd = nullptr;
	_mapWnd = nullptr;
	_logWnd = nullptr;
	_mooWnd = nullptr;
}

bool SBBasePda::hideCurDisplay() {
	synchronizePdaState();

	if (_curDisplay) {
		// If we have an inventory window
		_curDisplay->SetVisible(false);

		// Hold this info.
		_holdDisplay = _curDisplay;
		_holdMode = SBBasePda::_pdaMode;

		_curDisplay = nullptr;
		SBBasePda::_pdaMode = NOMODE;
		setPdaState();
		return true;
	}

	return false;
}

bool SBBasePda::restoreCurDisplay() {
	synchronizePdaState();

	// If we saved a display
	if (_holdDisplay) {
		// Restore the display info.
		_curDisplay = _holdDisplay;
		SBBasePda::_pdaMode = _holdMode;

		_curDisplay->SetVisible(true);
		setPdaState();
		return true;
	}

	return false;
}

bool SBBasePda::hideMovie() {
	synchronizePdaState();

	if (_mooWnd) {  // if we have an inventory window

		_mooWnd->SetVisible(false);
		_curDisplay = nullptr;
		SBBasePda::_pdaMode = NOMODE;
		setPdaState();
		return true;
	}

	return false;
}

bool SBBasePda::showMovie() {
	synchronizePdaState();

	// if we're already playing a movie, then return false
	if (_pdaMode == MOOMODE) {
		return false;
	}

	if (_mooWnd) {
		if (_curDisplay)
			_curDisplay->SetVisible(false);       // Turn off the current display

		// Save the current PDA mode so we can return to it when done.
		((CBagMoo *)_mooWnd)->SavePDAMode(_pdaMode);
		((CBagMoo *)_mooWnd)->SavePDAPosition(_pdaPos);

		_mooWnd->SetVisible(true);                // Turn on the inventory

		// set the current display object
		_curDisplay = _mooWnd;                 // Set the current display = Inventory
		SBBasePda::_pdaMode = MOOMODE;
		setPdaState();

		// Set default state of movie to don't deactivate PDA
		setDeactivate(false);

		return true;
	}
	return false;
}

void SBBasePda::stopMovie(bool bResetPDA) {
	if (_mooWnd && _mooWnd == _curDisplay) {
		((CBagMoo *)_mooWnd)->StopMovie(bResetPDA);
	}
}

bool SBBasePda::setMovie(CBofString &movieName) {
	if (_mooWnd) {
		((CBagMoo *)_mooWnd)->SetPDAMovie(movieName);
		return true;
	}
	return false;
}

bool SBBasePda::hideInventory() {
	synchronizePdaState();

	// if we have an inventory window
	if (_invWnd) {
		_invWnd->SetVisible(false);
		_curDisplay = nullptr;
		SBBasePda::_pdaMode = NOMODE;
		setPdaState();
		return true;
	}

	return false;
}

bool SBBasePda::showInventory() {
	synchronizePdaState();
	stopMovie(false);		// if a movie is playing, then stop it.

	// If we have an inventory window
	if (_invWnd) {
		if (_curDisplay)
			_curDisplay->SetVisible(false);	// Turn off the current display

		_invWnd->SetVisible(true);			// Turn on the inventory

		// set the current display object
		_curDisplay = _invWnd;				// Set the current display = Inventory
		SBBasePda::_pdaMode = INVMODE;
		setPdaState();
		return true;
	}

	return false;
}

bool SBBasePda::showMap() {
	synchronizePdaState();
	stopMovie(false);		// If a movie is playing, then stop it.

	// if we have a map window
	if (_mapWnd)  {
		if (_curDisplay)
			_curDisplay->SetVisible(false);	// Turn off the current display

		_mapWnd->SetVisible(true);			// Turn on the map

		// set the current display object
		_curDisplay = _mapWnd;				// Set the current display = Map
		SBBasePda::_pdaMode = MAPMODE;
		_mapWnd->AttachActiveObjects();
		setPdaState();

		return true;
	}

	return false;
}

bool SBBasePda::hideMap() {
	synchronizePdaState();

	// if we have a map window
	if (_mapWnd) {
		// Turn off the Map
		_mapWnd->SetVisible(false);

		// set the current display to nullptr
		_curDisplay = nullptr;
		setPdaState();
		SBBasePda::_pdaMode = NOMODE;
		return true;
	}

	return false;
}

bool SBBasePda::showLog() {
	if (CBagStorageDevWnd::m_pEvtSDev != nullptr) {
		CBagStorageDevWnd::m_pEvtSDev->EvaluateExpressions();
	}

	synchronizePdaState();
	stopMovie(false);		// If a movie is playing, then stop it.

	// if we have a map window
	if (_logWnd) {
		if (_curDisplay)
			_curDisplay->SetVisible(false);	// Turn off the current display

		_logWnd->SetVisible(true);			// Turn on the map

		// Set the current display object
		_curDisplay = _logWnd;				// Set the current display = Map
		SBBasePda::_pdaMode = LOGMODE;
		_logWnd->AttachActiveObjects();
		setPdaState();

		return true;
	}

	return false;
}

bool SBBasePda::msgLight() {
	// If we have a map window
	if (_logWnd) {
		((CBagLog *)_logWnd)->PlayMsgQue();
	}

	return true;
}

void *SBBasePda::fPdaButtonHandler(int nRefId, void *pvInfo) {
	Assert(pvInfo != nullptr);

	SBBasePda *pPDA = (SBBasePda *)pvInfo;

	switch (nRefId) {
	case PDA_MAP:
		pPDA->showMap();
		break;

	case PDA_STASH:
		pPDA->showInventory();
		break;

	case PDA_ZOOM:
		pPDA->zoom();
		break;

	case PDA_SYSTEM: {
		CBagel *pApp = CBagel::getBagApp();
		if (pApp != nullptr) {
			CBagMasterWin *pWnd = pApp->getMasterWnd();
			if (pWnd != nullptr) {
				pWnd->PostUserMessage(WM_SHOWSYSTEMDLG, 0);
			}
		}
		break;
	}

	case PDA_LOG:
		pPDA->showLog();
		break;

	case PDA_OFF:
		pPDA->deactivate();
		break;

	case PDA_MSGLIGHT:
		pPDA->msgLight();
		break;

	default:
		break;
	}

	return pvInfo;
}

void SBBasePda::synchronizePdaState() {
	if (_pdaPos == PDADOWN && isActivated()) {
		deactivate();
	} else if (_pdaPos == PDAUP && isActivated() == false) {
		activate();
	}
}

void SBBasePda::setPdaState() {
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
		switch (_pdaMode) {
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
		switch (_pdaPos) {
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

void SBBasePda::getPdaState() {
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
			_pdaMode = MAPMODE;
		} else if (sPDAState.Find("INV") != -1) {
			_pdaMode = INVMODE;
		} else if (sPDAState.Find("LOG") != -1) {
			_pdaMode = LOGMODE;
		} else if (sPDAState.Find("MOO") != -1) {
			_pdaMode = MOOMODE;
		} else {
			_pdaMode = NOMODE;
		}
	}

	// Get the PDA up/down position
	pVar = VARMNGR->GetVariable(pPDAPos);
	if (pVar) {
		sPDAState = pVar->GetValue();
		if (sPDAState.Find("UP") != -1) {
			_pdaPos = PDAUP;
		} else {
			_pdaPos = PDADOWN;
		}
	}
}

#define NULLCURSOR 0
#define HANDCURSOR 1

int SBBasePda::getProperCursor(const CBofPoint &xPoint, CBofRect &pdaRect) {
	int nWieldCursor = CBagWield::GetWieldCursor();

	// Assume can't click
	int nCursorID = NULLCURSOR;

	// If we're in the map, return the nullptr cursor, if on the pda but not in the
	// map window, return the hand.  Same rules for nomode.
	switch (_pdaMode) {
	case MAPMODE:
	case NOMODE:
	case MOOMODE:
		if (_mapWnd) {
			CBofRect cRect = _mapWnd->getRect() + pdaRect.TopLeft();
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
		Assert(_pdaMode == INVMODE ? _curDisplay == _invWnd : _curDisplay == _logWnd);

		// If we have a display, then parouse it's list and see if we're over something worth
		// mousedowning on.
		if (_curDisplay) {
			CBagObject *pOverObj = nullptr;
			CBofRect cRect = _curDisplay->getRect() + pdaRect.TopLeft();
			if (cRect.PtInRect(xPoint)) {
				// Go through the list of inventory items and see if we're over any of them
				// in particular.
				CBofList<CBagObject *> *pList = _curDisplay->GetObjectList();
				if (pList != nullptr) {
					// Localize pda view rect
					cRect = _curDisplay->getRect() + pdaRect.TopLeft();

					int nCount = pList->GetCount();
					for (int i = 0; i < nCount; ++i) {
						CBagObject *pObj = pList->GetNodeItem(i);
						if (pObj->IsActive()) {
							CBofRect lRect = pObj->getRect() + cRect.TopLeft();      // localize icon rectangle
							if (lRect.PtInRect(xPoint)) {
								pOverObj = pObj;
							}
						}
					}
				}

				if (_pdaMode == LOGMODE) {
					if (pOverObj) {
						return pOverObj->GetOverCursor();
					}
					if (nWieldCursor >= 0) {
						return nWieldCursor;
					}
				}

				if (_pdaMode == INVMODE) {
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

CBofRect SBBasePda::getViewRect() {
	return (_curDisplay == nullptr ? CBofRect() : _curDisplay->getRect());
}

ErrorCode SBBasePda::attachActiveObjects() {
	if (CBagStorageDevWnd::m_pEvtSDev != nullptr) {
		CBagStorageDevWnd::m_pEvtSDev->EvaluateExpressions();
	}

	if (_mooWnd)
		_mooWnd->AttachActiveObjects();
	if (_invWnd)
		_invWnd->AttachActiveObjects();
	if (_mapWnd)
		_mapWnd->AttachActiveObjects();
	if (_logWnd)
		_logWnd->AttachActiveObjects();

	return ERR_NONE;
}

ErrorCode SBBasePda::detachActiveObjects() {
	if (_mooWnd)
		_mooWnd->DetachActiveObjects();
	if (_invWnd)
		_invWnd->DetachActiveObjects();
	if (_mapWnd)
		_mapWnd->DetachActiveObjects();
	if (_logWnd)
		_logWnd->DetachActiveObjects();

	return ERR_NONE;
}

} // namespace Bagel
