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

#include "bagel/spacebar/baglib/button_object.h"
#include "bagel/spacebar/baglib/master_win.h"
#include "bagel/spacebar/baglib/time_object.h"
#include "bagel/spacebar/baglib/rp_object.h"
#include "bagel/spacebar/baglib/wield.h"
#include "bagel/spacebar/baglib/zoom_pda.h"

namespace Bagel {
namespace SpaceBar {

#define ZOOM_MOO_WLD      "MOOZ_WLD"
#define ZOOM_INV_WLD      "INVZ_WLD"
#define ZOOM_MAP_WLD      "MAPZ_WLD"
#define ZOOM_LOG_WLD      "LOGZ_WLD"

// Keep track of updates...
static uint32 g_lZoomPDALastUpdate;

void SBZoomPda::initialize() {
	g_lZoomPDALastUpdate = 0;
}

SBZoomPda::SBZoomPda(CBofWindow *pParent, const CBofRect &xRect, bool bActivated)
	: CBagStorageDevWnd(),
	  SBBasePda(pParent, xRect, bActivated) {
	_xSDevType = SDEV_ZOOMPDA;
}

ErrorCode SBZoomPda::onRender(CBofBitmap *pBmp, CBofRect *pRect) {
	assert(isValidObject(this));
	assert(pBmp != nullptr);

	// Paint the storage device
	if (pBmp != nullptr) {
		bool bUpdate = false;
		// Only clear the background and paint the backdrop if we've
		// been instructed to.

		if (preFilterPan()) {
			preFilter(pBmp, pRect, (_curDisplay == nullptr ? nullptr : _curDisplay->getObjectList()));

			// Paint our storage device the first time through and the next time
			// through, this takes care of multiple text drawing problems (trust me!).
			if (_bFirstPaint == false) {
				setPreFilterPan(false);
			}

			if (getWorkBmp() != nullptr) {
				getWorkBmp()->paint(pBmp, pRect, pRect);
			}

			bUpdate = true;
		}

		paintStorageDevice(nullptr, pBmp, pRect);

		// Paint the inventory or Map to backdrop
		if (bUpdate && (_curDisplay != nullptr))
			_curDisplay->update(pBmp, _curDisplay->getPosition(), pRect);
	}

	return _errCode;
}

ErrorCode SBZoomPda::loadFile(const CBofString &sFile) {
	ErrorCode errorCode = CBagStorageDev::loadFile(sFile);
	removeObject(_mooWnd);
	removeObject(_invWnd);
	removeObject(_mapWnd);
	removeObject(_logWnd);

	return errorCode;
}

ErrorCode SBZoomPda::detach() {
	bool bLogZoomed = (_logWnd == _curDisplay);

	// Other classes need to know if we're zoomed
	setZoomed(false);
	if (_invWnd) {
		_invWnd->detach();
		_invWnd = nullptr;
	}

	if (_mooWnd) {
		_mooWnd->detach();
		_mooWnd = nullptr;
	}

	if (_mapWnd) {
		_mapWnd->detach();
		_mapWnd = nullptr;
	}

	if (_logWnd) {
		_logWnd->detach();
		_logWnd = nullptr;
	}

	// Since the regular PDA does not have a detach method (it doesn't get
	// flushed out until you go to a flashback or quit the game), go through
	// the entire list of RPO's (residue print objects) and restore the saved
	// variable values.  We do this so that any values that were changed in
	// the zoomed version are propagated down to the un-zoomed pda.

	CBagRPObject::synchronizeResiduePrintedObjects(bLogZoomed);

	CBagStorageDevWnd::detach();

	return ERR_NONE;
}

ErrorCode SBZoomPda::attach() {
	// Other classes need to know if we're zoomed
	setZoomed(true);

	ErrorCode errorCode = CBagStorageDevWnd::attach();
	if (errorCode == ERR_NONE) {
		CBagStorageDev *pSDev;
		if (!_mooWnd) {
			pSDev = g_SDevManager->getStorageDevice(ZOOM_MOO_WLD);

			if (pSDev != nullptr) {
				_mooWnd = (CBagStorageDevBmp *)pSDev;
				_mooWnd->setAssociateWnd(getAssociateWnd());
				_mooWnd->setTransparent(false);
				_mooWnd->setVisible(false);
				errorCode = _mooWnd->attach();
			}
		} else {
			// Already attached just update
			_mooWnd->attachActiveObjects();
		}

		if (!_invWnd) {
			pSDev = g_SDevManager->getStorageDevice(ZOOM_INV_WLD);
			if (pSDev != nullptr) {
				_invWnd = (CBagStorageDevBmp *)pSDev;
				_invWnd->setAssociateWnd(getAssociateWnd());

				_invWnd->setTransparent(false);
				_invWnd->setVisible(false);
				errorCode = _invWnd->attach();
			} else {
				bofMessageBox("No PDA INVENTORY found", __FILE__);
				errorCode = ERR_UNKNOWN;
			}
		} else {
			// Already attached just update
			_invWnd->attachActiveObjects();
		}

		if (!_mapWnd) {
			pSDev = g_SDevManager->getStorageDevice(ZOOM_MAP_WLD);
			if (pSDev != nullptr) {
				_mapWnd = (CBagStorageDevBmp *)pSDev;
				_mapWnd->setAssociateWnd(getAssociateWnd());

				_mapWnd->setTransparent(false);
				_mapWnd->setVisible(false);
				errorCode = _mapWnd->attach();
			} else {
				bofMessageBox("No PDA MAP found", __FILE__);
				errorCode = ERR_UNKNOWN;
			}
		} else {
			// Already attached just update
			_mapWnd->attachActiveObjects();
		}

		if (!_logWnd) {
			pSDev = g_SDevManager->getStorageDevice(ZOOM_LOG_WLD);
			if (pSDev != nullptr) {
				_logWnd = (CBagStorageDevBmp *)pSDev;
				_logWnd->setAssociateWnd(getAssociateWnd());

				_logWnd->setTransparent(false);
				_logWnd->setVisible(false);
				errorCode = _logWnd->attach();
			}
		} else {
			// Already attached just update
			_logWnd->attachActiveObjects();
		}

		if (_pdaMode == PDA_INV_MODE) {
			showInventory();
		} else if (_pdaMode == PDA_MAP_MODE) {
			showMap();
		} else if (_pdaMode == PDA_LOG_MODE) {
			showLog();
		}

		show();
		invalidateRect(nullptr);
		updateWindow();
	}

	// Keep track of updates...
	g_lZoomPDALastUpdate = 0;

	return errorCode;
}

CBagObject *SBZoomPda::onNewButtonObject(const CBofString &) {
	CBagButtonObject *PdaButtObj = new CBagButtonObject();
	PdaButtObj->setCallBack(pdaButtonHandler, (SBBasePda *)this);

	return PdaButtObj;
}

void SBZoomPda::onLButtonUp(uint32 nFlags, CBofPoint *xPoint, void *) {
	// Need to override the CBagStorageDevWnd::onLButtonUp(nFlags, xPoint)
	// to do our own thing.

	*_xCursorLocation = *xPoint;
	CBofPoint xCursorLocation = devPtToViewPort(*xPoint);

	CBagObject *pObj = getObject(xCursorLocation, true);
	if (pObj != nullptr) {
		if (pObj->isActive()) {
			pObj->onLButtonUp(nFlags, xPoint);
			setLActiveObject(pObj);
		}
	} else {
		setLActivity(kMouseNONE);

		if (_curDisplay) {
			CBofRect offset = CBagStorageDev::getRect();
			xPoint->x -= offset.topLeft().x;
			xPoint->y -= offset.topLeft().y;

			// Make sure this stuff is nice and dirty before calling off
			// to the button handling routine, this assures that if we go from one screen
			// to the next, then we'll get redrawn.
			setPreFilterPan(true);
			makeListDirty(_curDisplay->getObjectList());

			_curDisplay->onLButtonUp(nFlags, xPoint, nullptr);
		} else {
			// We have no mode yet, then pass it to the default method
			if (_pdaMode == PDA_NO_MODE) {
				CBagStorageDevWnd::onLButtonUp(nFlags, xPoint);
			}
		}
	}

	// This thing could get trashed by the underlying code...
	if (isCreated()) {
		invalidateRect(nullptr);
		updateWindow();
	}
}

void SBZoomPda::onMouseMove(uint32 nFlags, CBofPoint *pPoint, void *) {
	assert(isValidObject(this));

	CBagStorageDev::onMouseMove(nFlags, pPoint, getAssociateWnd());


	// This should be on update cursor virtual func
	if (getExitOnEdge() && (pPoint->x < getExitOnEdge()) && (pPoint->y < 300) && !(getPrevSDev().isEmpty())) {
		CBagMasterWin::setActiveCursor(10);
	} else {
		CBofRect cRect = getBackdrop()->getRect();
		CBagMasterWin::setActiveCursor(getProperCursor(*pPoint, cRect));
	}
}

void SBZoomPda::onMainLoop() {
	uint32 nCurTime = getTimer();

	// Force an update every 1/4 second
	if (_pdaMode == PDA_INV_MODE || _pdaMode == PDA_MAP_MODE) {
		if (nCurTime > (g_lZoomPDALastUpdate + 250)) {
			g_lZoomPDALastUpdate = nCurTime;

			setPreFilterPan(true);
		}
	}

	CBagStorageDevWnd::onMainLoop();
}

ErrorCode SBZoomPda::attachActiveObjects() {
	SBBasePda::attachActiveObjects();
	return CBagStorageDevWnd::attachActiveObjects();
}

ErrorCode SBZoomPda::detachActiveObjects() {
	SBBasePda::detachActiveObjects();

	return CBagStorageDevWnd::detachActiveObjects();
}

} // namespace SpaceBar
} // namespace Bagel
