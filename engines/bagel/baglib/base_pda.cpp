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
PdaMode SBBasePda::_pdaMode;
PdaPos SBBasePda::_pdaPos;
PdaMode SBBasePda::_holdMode;

void SBBasePda::initialize() {
	_pdaMode = PDA_NO_MODE;
	_pdaPos = PDA_POS_UNINITIALIZED;
	_holdMode = PDA_NO_MODE;
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
	_holdDisplay = nullptr;

	// Other classes need to know if we're zoomed
	setZoomed(false);
	setDeactivate(false);
	
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
		_curDisplay->setVisible(false);

		// Hold this info.
		_holdDisplay = _curDisplay;
		_holdMode = _pdaMode;

		_curDisplay = nullptr;
		_pdaMode = PDA_NO_MODE;
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

		_curDisplay->setVisible(true);
		setPdaState();
		return true;
	}

	return false;
}

bool SBBasePda::hideMovie() {
	synchronizePdaState();

	if (_mooWnd) {
		// if we have an inventory window
		_mooWnd->setVisible(false);
		_curDisplay = nullptr;
		_pdaMode = PDA_NO_MODE;
		setPdaState();
		return true;
	}

	return false;
}

bool SBBasePda::showMovie() {
	synchronizePdaState();

	// if we're already playing a movie, then return false
	if (_pdaMode == PDA_MOO_MODE) {
		return false;
	}

	if (_mooWnd) {
		if (_curDisplay) {
			// Turn off the current display
			_curDisplay->setVisible(false);
		}

		// Save the current PDA mode so we can return to it when done.
		((CBagMoo *)_mooWnd)->savePDAMode(_pdaMode);
		((CBagMoo *)_mooWnd)->savePDAPosition(_pdaPos);

		// Turn on the inventory
		_mooWnd->setVisible(true);

		// Set the current display = Inventory
		_curDisplay = _mooWnd;
		_pdaMode = PDA_MOO_MODE;
		setPdaState();

		// Set default state of movie to don't deactivate PDA
		setDeactivate(false);

		return true;
	}
	return false;
}

void SBBasePda::stopMovie(bool bResetPDA) const {
	if (_mooWnd && _mooWnd == _curDisplay) {
		((CBagMoo *)_mooWnd)->stopMovie(bResetPDA);
	}
}

bool SBBasePda::setMovie(CBofString &movieName) const {
	if (_mooWnd) {
		((CBagMoo *)_mooWnd)->setPDAMovie(movieName);
		return true;
	}
	return false;
}

bool SBBasePda::hideInventory() {
	synchronizePdaState();

	// if we have an inventory window
	if (_invWnd) {
		_invWnd->setVisible(false);
		_curDisplay = nullptr;
		_pdaMode = PDA_NO_MODE;
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
			_curDisplay->setVisible(false);
		}
		// Turn on the inventory
		_invWnd->setVisible(true);

		// Set the current display = Inventory
		_curDisplay = _invWnd;
		_pdaMode = PDA_INV_MODE;
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
			_curDisplay->setVisible(false);
		}

		// Turn on the map
		_mapWnd->setVisible(true);	

		// Set the current display = Map
		_curDisplay = _mapWnd;
		_pdaMode = PDA_MAP_MODE;
		_mapWnd->attachActiveObjects();
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
		_mapWnd->setVisible(false);

		// set the current display to nullptr
		_curDisplay = nullptr;
		setPdaState();
		_pdaMode = PDA_NO_MODE;
		return true;
	}

	return false;
}

bool SBBasePda::showLog() {
	if (CBagStorageDevWnd::_pEvtSDev != nullptr) {
		CBagStorageDevWnd::_pEvtSDev->evaluateExpressions();
	}

	synchronizePdaState();

	// If a movie is playing, then stop it.
	stopMovie(false);

	// if we have a map window
	if (_logWnd) {
		if (_curDisplay) {
			// Turn off the current display
			_curDisplay->setVisible(false);
		}

		// Turn on the map
		_logWnd->setVisible(true);	

		// Set the current display = Map
		_curDisplay = _logWnd;
		_pdaMode = PDA_LOG_MODE;
		_logWnd->attachActiveObjects();
		setPdaState();

		return true;
	}

	return false;
}

bool SBBasePda::msgLight() {
	// If we have a map window
	if (_logWnd) {
		((CBagLog *)_logWnd)->playMsgQueue();
	}

	return true;
}

void *SBBasePda::pdaButtonHandler(int refId, void *info) {
	assert(info != nullptr);

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
				curWnd->postUserMessage(WM_SHOW_SYSTEM_DLG, 0);
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
	if (_pdaPos == PDA_DOWN && isActivated()) {
		deactivate();
	} else if (_pdaPos == PDA_UP && isActivated() == false) {
		activate();
	}
}

void SBBasePda::activate() {
	if (!_activating) {
		_activating = _numMoves;
		_activated = true;
	}

	_pdaPos = PDA_UP;
	setPdaState();
}

void SBBasePda::deactivate() {
	if (!_activating) {
		_activating = _numMoves;
		_activated = false;
	}

	_pdaPos = PDA_DOWN;
	setPdaState();
}

void SBBasePda::setPdaState() {
	const char *pdaMode;
	const char *pdaPos;

	CBagVar *curVar = g_VarManager->getVariable("INBAR");

	if (curVar != nullptr) {
		// Defined as global
		pdaMode = "BARPDAMODE";
		pdaPos = "BARPDAPOSITION";
	} else {
		pdaMode = "PDAMODE";
		pdaPos = "PDAPOSITION";
	}

	// Save the pda state and position
	curVar = g_VarManager->getVariable(pdaMode);
	if (curVar != nullptr) {
		switch (_pdaMode) {
		case PDA_NO_MODE:
			curVar->setValue("NOMODE");
			break;
		case PDA_MAP_MODE:
			curVar->setValue("MAPMODE");
			break;
		case PDA_INV_MODE:
			curVar->setValue("INVMODE");
			break;
		case PDA_LOG_MODE:
			curVar->setValue("LOGMODE");
			break;
		case PDA_MOO_MODE:
			curVar->setValue("MOOMODE");
			break;
		}
	}

	curVar = g_VarManager->getVariable(pdaPos);
	if (curVar != nullptr) {
		switch (_pdaPos) {
		case PDA_UP:
			curVar->setValue("UP");
			break;
		case PDA_DOWN:
			curVar->setValue("DOWN");
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

	CBagVar *curVar = g_VarManager->getVariable("INBAR");

	if (curVar != nullptr) {
		// Defined as global
		pdaMode = "BARPDAMODE";
		pdaPos = "BARPDAPOSITION";
	} else {
		pdaMode = "PDAMODE";
		pdaPos = "PDAPOSITION";
	}

	curVar = g_VarManager->getVariable(pdaMode);
	if (curVar) {
		pdaState = curVar->getValue();
		// Now set the internal PDA state based on this info.
		// If we saved during a movie, then restore to map mode.
		if (pdaState.find("MAP") != -1 || pdaState.find("MOO") != -1) {
			_pdaMode = PDA_MAP_MODE;
		} else if (pdaState.find("INV") != -1) {
			_pdaMode = PDA_INV_MODE;
		} else if (pdaState.find("LOG") != -1) {
			_pdaMode = PDA_LOG_MODE;
		} else if (pdaState.find("MOO") != -1) {
			_pdaMode = PDA_MOO_MODE;
		} else {
			_pdaMode = PDA_NO_MODE;
		}
	}

	// Get the PDA up/down position
	curVar = g_VarManager->getVariable(pdaPos);
	if (curVar) {
		pdaState = curVar->getValue();
		if (pdaState.find("UP") != -1) {
			_pdaPos = PDA_UP;
		} else {
			_pdaPos = PDA_DOWN;
		}
	}
}

#define NULL_CURSOR 0
#define HAND_CURSOR 1

int SBBasePda::getProperCursor(const CBofPoint &pos, CBofRect &pdaRect) const {
	const int wieldCursor = CBagWield::getWieldCursor();

	// Assume can't click
	const int cursorID = NULL_CURSOR;

	// If we're in the map, return the nullptr cursor, if on the pda but not in the
	// map window, return the hand.  Same rules for nomode.
	switch (_pdaMode) {
	case PDA_MAP_MODE:
	case PDA_NO_MODE:
	case PDA_MOO_MODE:
		if (_mapWnd) {
			const CBofRect pdaViewRect = _mapWnd->getRect() + pdaRect.topLeft();
			if (pdaViewRect.ptInRect(pos)) {
				if (wieldCursor >= 0) {
					return wieldCursor;
				}
				return NULL_CURSOR;
			}
			return HAND_CURSOR;
		}
		break;

	case PDA_INV_MODE:
	case PDA_LOG_MODE:
		// Make sure our cur display is synchronized.
		assert(_pdaMode == PDA_INV_MODE ? _curDisplay == _invWnd : _curDisplay == _logWnd);

		// If we have a display, then parouse its list and see if we're over something worth
		// mousedowning on.
		if (_curDisplay) {
			CBagObject *overObj = nullptr;
			CBofRect pdaViewRect = _curDisplay->getRect() + pdaRect.topLeft();
			if (pdaViewRect.ptInRect(pos)) {
				// Go through the list of inventory items and see if we're over any of them
				// in particular.
				CBofList<CBagObject *> *objList = _curDisplay->getObjectList();
				if (objList != nullptr) {
					// Localize pda view rect
					pdaViewRect = _curDisplay->getRect() + pdaRect.topLeft();

					const int count = objList->getCount();
					for (int i = 0; i < count; ++i) {
						CBagObject *curObj = objList->getNodeItem(i);
						if (curObj->isActive()) {
							// localize icon rectangle
							CBofRect iconRect = curObj->getRect() + pdaViewRect.topLeft();
							if (iconRect.ptInRect(pos)) {
								overObj = curObj;
							}
						}
					}
				}

				if (_pdaMode == PDA_LOG_MODE) {
					if (overObj) {
						return overObj->getOverCursor();
					}
					if (wieldCursor >= 0) {
						return wieldCursor;
					}
				}

				if (_pdaMode == PDA_INV_MODE) {
					if (wieldCursor >= 0) {
						return wieldCursor;
					}
					if (overObj) {
						return overObj->getOverCursor();
					}
				}

				return NULL_CURSOR;
			}

			return HAND_CURSOR;
		}
	}

	return cursorID;
}

CBofRect SBBasePda::getViewRect() const {
	return (_curDisplay == nullptr ? CBofRect() : _curDisplay->getRect());
}

ErrorCode SBBasePda::attachActiveObjects() {
	if (CBagStorageDevWnd::_pEvtSDev != nullptr) {
		CBagStorageDevWnd::_pEvtSDev->evaluateExpressions();
	}

	if (_mooWnd)
		_mooWnd->attachActiveObjects();
	if (_invWnd)
		_invWnd->attachActiveObjects();
	if (_mapWnd)
		_mapWnd->attachActiveObjects();
	if (_logWnd)
		_logWnd->attachActiveObjects();

	return ERR_NONE;
}

ErrorCode SBBasePda::detachActiveObjects() {
	if (_mooWnd)
		_mooWnd->detachActiveObjects();
	if (_invWnd)
		_invWnd->detachActiveObjects();
	if (_mapWnd)
		_mapWnd->detachActiveObjects();
	if (_logWnd)
		_logWnd->detachActiveObjects();

	return ERR_NONE;
}

} // namespace Bagel
