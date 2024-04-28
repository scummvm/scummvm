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
		_holdMode = _pdaMode;

		_curDisplay = nullptr;
		_pdaMode = NOMODE;
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
		_pdaMode = _holdMode;

		_curDisplay->SetVisible(true);
		setPdaState();
		return true;
	}

	return false;
}

bool SBBasePda::hideMovie() {
	synchronizePdaState();

	if (_mooWnd) {
		// if we have an inventory window
		_mooWnd->SetVisible(false);
		_curDisplay = nullptr;
		_pdaMode = NOMODE;
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
		if (_curDisplay) {
			// Turn off the current display
			_curDisplay->SetVisible(false);
		}

		// Save the current PDA mode so we can return to it when done.
		((CBagMoo *)_mooWnd)->SavePDAMode(_pdaMode);
		((CBagMoo *)_mooWnd)->SavePDAPosition(_pdaPos);

		// Turn on the inventory
		_mooWnd->SetVisible(true);

		// Set the current display = Inventory
		_curDisplay = _mooWnd;
		_pdaMode = MOOMODE;
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
		_pdaMode = NOMODE;
		setPdaState();
		return true;
	}

	return false;
}

bool SBBasePda::showInventory() {
	synchronizePdaState();
	
	// if a movie is playing, then stop it.
	stopMovie(false);

	// If we have an inventory window
	if (_invWnd) {
		if (_curDisplay) {
			// Turn off the current display
			_curDisplay->SetVisible(false);
		}
		// Turn on the inventory
		_invWnd->SetVisible(true);

		// Set the current display = Inventory
		_curDisplay = _invWnd;
		_pdaMode = INVMODE;
		setPdaState();
		return true;
	}

	return false;
}

bool SBBasePda::showMap() {
	synchronizePdaState();
	
	// If a movie is playing, then stop it.
	stopMovie(false);

	// if we have a map window
	if (_mapWnd)  {
		if (_curDisplay) {
			// Turn off the current display
			_curDisplay->SetVisible(false);
		}

		// Turn on the map
		_mapWnd->SetVisible(true);	

		// Set the current display = Map
		_curDisplay = _mapWnd;
		_pdaMode = MAPMODE;
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
		_pdaMode = NOMODE;
		return true;
	}

	return false;
}

bool SBBasePda::showLog() {
	if (CBagStorageDevWnd::m_pEvtSDev != nullptr) {
		CBagStorageDevWnd::m_pEvtSDev->EvaluateExpressions();
	}

	synchronizePdaState();

	// If a movie is playing, then stop it.
	stopMovie(false);

	// if we have a map window
	if (_logWnd) {
		if (_curDisplay) {
			// Turn off the current display
			_curDisplay->SetVisible(false);
		}

		// Turn on the map
		_logWnd->SetVisible(true);	

		// Set the current display = Map
		_curDisplay = _logWnd;
		_pdaMode = LOGMODE;
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

void *SBBasePda::pdaButtonHandler(int refId, void *info) {
	Assert(info != nullptr);

	SBBasePda *curPda = (SBBasePda *)info;

	switch (refId) {
	case PDA_MAP:
		curPda->showMap();
		break;

	case PDA_STASH:
		curPda->showInventory();
		break;

	case PDA_ZOOM:
		curPda->zoom();
		break;

	case PDA_SYSTEM: {
		CBagel *curApp = CBagel::getBagApp();
		if (curApp != nullptr) {
			CBagMasterWin *curWnd = curApp->getMasterWnd();
			if (curWnd != nullptr) {
				curWnd->PostUserMessage(WM_SHOWSYSTEMDLG, 0);
			}
		}
		break;
	}

	case PDA_LOG:
		curPda->showLog();
		break;

	case PDA_OFF:
		curPda->deactivate();
		break;

	case PDA_MSGLIGHT:
		curPda->msgLight();
		break;

	default:
		break;
	}

	return info;
}

void SBBasePda::synchronizePdaState() {
	if (_pdaPos == PDADOWN && isActivated()) {
		deactivate();
	} else if (_pdaPos == PDAUP && isActivated() == false) {
		activate();
	}
}

void SBBasePda::activate() {
	if (!_activating) {
		_activating = _numMoves;
		_activated = true;
	}

	_pdaPos = PDAUP;
	setPdaState();
}

void SBBasePda::deactivate() {
	if (!_activating) {
		_activating = _numMoves;
		_activated = false;
	}

	_pdaPos = PDADOWN;
	setPdaState();
}

void SBBasePda::setPdaState() {
	const char *pdaMode;
	const char *pdaPos;

	CBagVar *curVar = VARMNGR->GetVariable("INBAR");

	if (curVar != nullptr) {
		// Defined as global
		pdaMode = "BARPDAMODE";
		pdaPos = "BARPDAPOSITION";
	} else {
		pdaMode = "PDAMODE";
		pdaPos = "PDAPOSITION";
	}

	// Save the pda state and position
	curVar = VARMNGR->GetVariable(pdaMode);
	if (curVar != nullptr) {
		switch (_pdaMode) {
		case NOMODE:
			curVar->SetValue("NOMODE");
			break;
		case MAPMODE:
			curVar->SetValue("MAPMODE");
			break;
		case INVMODE:
			curVar->SetValue("INVMODE");
			break;
		case LOGMODE:
			curVar->SetValue("LOGMODE");
			break;
		case MOOMODE:
			curVar->SetValue("MOOMODE");
			break;
		}
	}

	curVar = VARMNGR->GetVariable(pdaPos);
	if (curVar != nullptr) {
		switch (_pdaPos) {
		case PDAUP:
			curVar->SetValue("UP");
			break;
		case PDADOWN:
			curVar->SetValue("DOWN");
			break;
		default:
			break;
		}
	}
}

void SBBasePda::getPdaState() {
	char localBuff[256];
	CBofString pdaState(localBuff, 256);
	const char *pdaMode;
	const char *pdaPos;

	CBagVar *curVar = VARMNGR->GetVariable("INBAR");

	if (curVar != nullptr) {
		// Defined as global
		pdaMode = "BARPDAMODE";
		pdaPos = "BARPDAPOSITION";
	} else {
		pdaMode = "PDAMODE";
		pdaPos = "PDAPOSITION";
	}

	curVar = VARMNGR->GetVariable(pdaMode);
	if (curVar) {
		pdaState = curVar->GetValue();
		// Now set the internal PDA state based on this info.
		// If we saved during a movie, then restore to map mode.
		if (pdaState.Find("MAP") != -1 || pdaState.Find("MOO") != -1) {
			_pdaMode = MAPMODE;
		} else if (pdaState.Find("INV") != -1) {
			_pdaMode = INVMODE;
		} else if (pdaState.Find("LOG") != -1) {
			_pdaMode = LOGMODE;
		} else if (pdaState.Find("MOO") != -1) {
			_pdaMode = MOOMODE;
		} else {
			_pdaMode = NOMODE;
		}
	}

	// Get the PDA up/down position
	curVar = VARMNGR->GetVariable(pdaPos);
	if (curVar) {
		pdaState = curVar->GetValue();
		if (pdaState.Find("UP") != -1) {
			_pdaPos = PDAUP;
		} else {
			_pdaPos = PDADOWN;
		}
	}
}

#define NULLCURSOR 0
#define HANDCURSOR 1

int SBBasePda::getProperCursor(const CBofPoint &pos, CBofRect &pdaRect) {
	int wieldCursor = CBagWield::GetWieldCursor();

	// Assume can't click
	int cursorID = NULLCURSOR;

	// If we're in the map, return the nullptr cursor, if on the pda but not in the
	// map window, return the hand.  Same rules for nomode.
	switch (_pdaMode) {
	case MAPMODE:
	case NOMODE:
	case MOOMODE:
		if (_mapWnd) {
			CBofRect pdaViewRect = _mapWnd->getRect() + pdaRect.TopLeft();
			if (pdaViewRect.PtInRect(pos)) {
				if (wieldCursor >= 0) {
					return wieldCursor;
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

		// If we have a display, then parouse its list and see if we're over something worth
		// mousedowning on.
		if (_curDisplay) {
			CBagObject *overObj = nullptr;
			CBofRect pdaViewRect = _curDisplay->getRect() + pdaRect.TopLeft();
			if (pdaViewRect.PtInRect(pos)) {
				// Go through the list of inventory items and see if we're over any of them
				// in particular.
				CBofList<CBagObject *> *objList = _curDisplay->GetObjectList();
				if (objList != nullptr) {
					// Localize pda view rect
					pdaViewRect = _curDisplay->getRect() + pdaRect.TopLeft();

					int count = objList->GetCount();
					for (int i = 0; i < count; ++i) {
						CBagObject *curObj = objList->GetNodeItem(i);
						if (curObj->IsActive()) {
							// localize icon rectangle
							CBofRect iconRect = curObj->getRect() + pdaViewRect.TopLeft();
							if (iconRect.PtInRect(pos)) {
								overObj = curObj;
							}
						}
					}
				}

				if (_pdaMode == LOGMODE) {
					if (overObj) {
						return overObj->GetOverCursor();
					}
					if (wieldCursor >= 0) {
						return wieldCursor;
					}
				}

				if (_pdaMode == INVMODE) {
					if (wieldCursor >= 0) {
						return wieldCursor;
					}
					if (overObj) {
						return overObj->GetOverCursor();
					}
				}

				return NULLCURSOR;
			}

			return HANDCURSOR;
		}
	}

	return cursorID;
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
