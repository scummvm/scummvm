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
#include "bagel/baglib/master_win.h"
#include "bagel/baglib/time_object.h"
#include "bagel/baglib/rp_object.h"
#include "bagel/baglib/wield.h"
#include "bagel/baglib/zoom_pda.h"
#include "bagel/boflib/app.h"

namespace Bagel {

#define ZOOMMOOWLD      "MOOZ_WLD"
#define ZOOMINVWLD      "INVZ_WLD"
#define ZOOMMAPWLD      "MAPZ_WLD"
#define ZOOMLOGWLD      "LOGZ_WLD"

// Keep track of updates...
static uint32 g_lZoomPDALastUpdate;

void SBZoomPda::initialize() {
	g_lZoomPDALastUpdate = 0;
}

SBZoomPda::SBZoomPda(CBofWindow *pParent, const CBofRect &xRect, bool bActivated)
	: CBagStorageDevWnd(),
	  SBBasePda(pParent, xRect, bActivated) {
	m_xSDevType = SDEV_ZOOMPDA;
}

ErrorCode SBZoomPda::OnRender(CBofBitmap *pBmp, CBofRect *pRect) {
	Assert(IsValidObject(this));
	Assert(pBmp != nullptr);

	// Paint the storage device
	if (pBmp != nullptr) {
		bool bUpdate = false;
		// Only clear the background and paint the backdrop if we've
		// been instructed to.

		if (PreFilterPan()) {
			PreFilter(pBmp, pRect, (_curDisplay == nullptr ? nullptr : _curDisplay->GetObjectList()));

			// Paint our storage device the first time through and the next time
			// through, this takes care of multiple text drawing problems (trust me!).
			if (m_bFirstPaint == false) {
				SetPreFilterPan(false);
			}

			if (GetWorkBmp() != nullptr) {
				GetWorkBmp()->paint(pBmp, pRect, pRect);
			}

			bUpdate = true;
		}

		PaintStorageDevice(nullptr, pBmp, pRect);

		// Paint the inventory or Map to backdrop
		if (bUpdate) {
			if (_curDisplay != nullptr) {
				_curDisplay->update(pBmp, _curDisplay->getPosition(), pRect);
			}
		}
	}

	return _errCode;
}

ErrorCode SBZoomPda::LoadFile(const CBofString &sFile) {
	ErrorCode error = CBagStorageDev::LoadFile(sFile);
	RemoveObject(_mooWnd);
	RemoveObject(_invWnd);
	RemoveObject(_mapWnd);
	RemoveObject(_logWnd);

	return error;
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
	// varible values.  We do this so that any values that were changed in
	// the zoomed version are propagated down to the unzoomed pda.

	CBagRPObject::SynchronizeRPObjects(bLogZoomed);

	CBagStorageDevWnd::detach();

	return ERR_NONE;
}

ErrorCode SBZoomPda::attach() {
	// Other classes need to know if we're zoomed
	setZoomed(true);

	ErrorCode rc = CBagStorageDevWnd::attach();
	if (rc == ERR_NONE) {
		CBagStorageDev *pSDev;
		if (!_mooWnd) {
			pSDev = SDEVMNGR->GetStorageDevice(ZOOMMOOWLD);

			if (pSDev != nullptr) {
				_mooWnd = (CBagStorageDevBmp *)pSDev;
				_mooWnd->SetAssociateWnd(GetAssociateWnd());
				_mooWnd->SetTransparent(false);
				_mooWnd->SetVisible(false);
				rc = _mooWnd->attach();
			}
		} else {
			// Already attached just update
			_mooWnd->AttachActiveObjects();
		}

		if (!_invWnd) {
			pSDev = SDEVMNGR->GetStorageDevice(ZOOMINVWLD);
			if (pSDev != nullptr) {
				_invWnd = (CBagStorageDevBmp *)pSDev;
				_invWnd->SetAssociateWnd(GetAssociateWnd());

				_invWnd->SetTransparent(false);
				_invWnd->SetVisible(false);
				rc = _invWnd->attach();
			} else {
				BofMessageBox("No PDA INVENTORY found", __FILE__);
				rc = ERR_UNKNOWN;
			}
		} else {
			// Already attached just update
			_invWnd->AttachActiveObjects();
		}

		if (!_mapWnd) {
			pSDev = SDEVMNGR->GetStorageDevice(ZOOMMAPWLD);
			if (pSDev != nullptr) {
				_mapWnd = (CBagStorageDevBmp *)pSDev;
				_mapWnd->SetAssociateWnd(GetAssociateWnd());

				_mapWnd->SetTransparent(false);
				_mapWnd->SetVisible(false);
				rc = _mapWnd->attach();
			} else {
				BofMessageBox("No PDA MAP found", __FILE__);
				rc = ERR_UNKNOWN;
			}
		} else {
			// Already attached just update
			_mapWnd->AttachActiveObjects();
		}

		if (!_logWnd) {
			pSDev = SDEVMNGR->GetStorageDevice(ZOOMLOGWLD);
			if (pSDev != nullptr) {
				_logWnd = (CBagStorageDevBmp *)pSDev;
				_logWnd->SetAssociateWnd(GetAssociateWnd());

				_logWnd->SetTransparent(false);
				_logWnd->SetVisible(false);
				rc = _logWnd->attach();
			}
		} else {
			// Already attached just update
			_logWnd->AttachActiveObjects();
		}

		if (_pdaMode == INVMODE) {
			showInventory();
		} else if (_pdaMode == MAPMODE) {
			showMap();
		} else if (_pdaMode == LOGMODE) {
			showLog();
		}

		show();
		InvalidateRect(nullptr);
		UpdateWindow();
	}

	// Keep track of updates...
	g_lZoomPDALastUpdate = 0;

	return rc;
}

CBagObject *SBZoomPda::OnNewButtonObject(const CBofString &) {
	CBagButtonObject *PdaButtObj = new CBagButtonObject();
	PdaButtObj->setCallBack(pdaButtonHandler, (SBBasePda *)this);

	return PdaButtObj;
}

void SBZoomPda::onLButtonUp(uint32 nFlags, CBofPoint *xPoint, void *) {
	// Need to override the CBagStorageDevWnd::onLButtonUp(nFlags, xPoint)
	// to do our own thing.

	*m_xCursorLocation = *xPoint;
	CBofPoint xCursorLocation = DevPtToViewPort(*xPoint);

	CBagObject *pObj = GetObject(xCursorLocation, true);
	if (pObj != nullptr) {
		if (pObj->IsActive()) {
			pObj->onLButtonUp(nFlags, xPoint);
			SetLActiveObject(pObj);
		}
	} else {
		SetLActivity(kMouseNONE);

		if (_curDisplay) {
			CBofRect offset = CBagStorageDev::getRect();
			xPoint->x -= offset.TopLeft().x;
			xPoint->y -= offset.TopLeft().y;

			// Make sure this stuff is nice and dirty before calling off
			// to the button handling routine, this assures that if we go from one screen
			// to the next, then we'll get redrawn.
			SetPreFilterPan(true);
			MakeListDirty(_curDisplay->GetObjectList());

			_curDisplay->onLButtonUp(nFlags, xPoint, nullptr);
		} else {
			// We have no mode yet, then pass it to the default method
			if (_pdaMode == NOMODE) {
				CBagStorageDevWnd::onLButtonUp(nFlags, xPoint);
			}
		}
	}

	// This thing could get trashed by the underlying code...
	if (IsCreated()) {
		InvalidateRect(nullptr);
		UpdateWindow();
	}
}

void SBZoomPda::onMouseMove(uint32 nFlags, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));

	CBagStorageDev::onMouseMove(nFlags, pPoint, GetAssociateWnd());


	// This should be on update cursor virtual func
	if (GetExitOnEdge() && (pPoint->x < GetExitOnEdge()) && (pPoint->y < 300) && !(GetPrevSDev().IsEmpty())) {
		CBagMasterWin::setActiveCursor(10);
	} else {
		CBofRect cRect = GetBackdrop()->getRect();
		CBagMasterWin::setActiveCursor(getProperCursor(*pPoint, cRect));
	}
}

void SBZoomPda::onMainLoop() {
	uint32 nCurTime = GetTimer();

	// Force an update every 1/4 second
	if (_pdaMode == INVMODE || _pdaMode == MAPMODE) {
		if (nCurTime > (g_lZoomPDALastUpdate + 250)) {
			g_lZoomPDALastUpdate = nCurTime;

			SetPreFilterPan(true);
		}
	}

	CBagStorageDevWnd::onMainLoop();
}

ErrorCode SBZoomPda::attachActiveObjects() {
	SBBasePda::attachActiveObjects();
	return CBagStorageDevWnd::AttachActiveObjects();
}

ErrorCode SBZoomPda::detachActiveObjects() {
	SBBasePda::detachActiveObjects();

	return CBagStorageDevWnd::DetachActiveObjects();
}

} // namespace Bagel
