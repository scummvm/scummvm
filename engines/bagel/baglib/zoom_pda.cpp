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

#include "bagel/baglib/zoom_pda.h"
#include "bagel/boflib/app.h"
#include "bagel/baglib/wield.h"
#include "bagel/baglib/button_object.h"
#include "bagel/baglib/rp_object.h"

namespace Bagel {

#define ZOOMMOOWLD      "MOOZ_WLD"
#define ZOOMINVWLD      "INVZ_WLD"
#define ZOOMMAPWLD      "MAPZ_WLD"
#define ZOOMLOGWLD      "LOGZ_WLD"

CBofRect DisplayZMRect(24, 49, 623, 348);     // Rect for display on zoom display

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
			PreFilter(pBmp, pRect, (m_xCurDisplay == nullptr ? nullptr : m_xCurDisplay->GetObjectList()));

			// Paint our storage device the first time through and the next time
			// through, this takes care of multiple text drawing problems (trust me!).
			if (m_bFirstPaint == false) {
				SetPreFilterPan(false);
			}

			if (GetWorkBmp() != nullptr) {
				GetWorkBmp()->Paint(pBmp, pRect, pRect);
			}

			bUpdate = true;
		}

		PaintStorageDevice(nullptr, pBmp, pRect);

		// Paint the inventory or Map to backdrop
		if (bUpdate) {
			if (m_xCurDisplay != nullptr) {
				m_xCurDisplay->Update(pBmp, m_xCurDisplay->GetPosition(), pRect);
			}
		}
	}

	return m_errCode;
}

ErrorCode SBZoomPda::LoadFile(const CBofString &sFile) {
	ErrorCode error = CBagStorageDev::LoadFile(sFile);
	RemoveObject(m_xMooWnd);
	RemoveObject(m_xInvWnd);
	RemoveObject(m_xMapWnd);
	RemoveObject(m_xLogWnd);

	return error;
}

ErrorCode SBZoomPda::Detach() {
	bool bLogZoomed = (m_xLogWnd == m_xCurDisplay);

	// Other classes need to know if we're zoomed
	SetZoomed(false);
	if (m_xInvWnd) {
		m_xInvWnd->Detach();
		m_xInvWnd = nullptr;
	}

	if (m_xMooWnd) {
		m_xMooWnd->Detach();
		m_xMooWnd = nullptr;
	}

	if (m_xMapWnd) {
		m_xMapWnd->Detach();
		m_xMapWnd = nullptr;
	}

	if (m_xLogWnd) {
		m_xLogWnd->Detach();
		m_xLogWnd = nullptr;
	}

	// Since the regular PDA does not have a detach method (it doesn't get
	// flushed out until you go to a flashback or quit the game), go through
	// the entire list of RPO's (residue print objects) and restore the saved
	// varible values.  We do this so that any values that were changed in
	// the zoomed version are propagated down to the unzoomed pda.

	CBagRPObject::SynchronizeRPObjects(bLogZoomed);

	CBagStorageDevWnd::Detach();

	return ERR_NONE;
}

ErrorCode SBZoomPda::Attach() {
	// Other classes need to know if we're zoomed
	SetZoomed(true);

	ErrorCode rc = CBagStorageDevWnd::Attach();
	if (rc == ERR_NONE) {
		CBagStorageDev *pSDev;
		if (!m_xMooWnd) {
			pSDev = SDEVMNGR->GetStorageDevice(ZOOMMOOWLD);

			if (pSDev != nullptr) {
				m_xMooWnd = (CBagStorageDevBmp *)pSDev;
				m_xMooWnd->SetAssociateWnd(GetAssociateWnd());
				m_xMooWnd->SetTransparent(false);
				m_xMooWnd->SetVisible(false);
				rc = m_xMooWnd->Attach();
			}
		} else {
			// Already attached just update
			m_xMooWnd->AttachActiveObjects();
		}

		if (!m_xInvWnd) {
			pSDev = SDEVMNGR->GetStorageDevice(ZOOMINVWLD);
			if (pSDev != nullptr) {
				m_xInvWnd = (CBagStorageDevBmp *)pSDev;
				m_xInvWnd->SetAssociateWnd(GetAssociateWnd());

				m_xInvWnd->SetTransparent(false);
				m_xInvWnd->SetVisible(false);
				rc = m_xInvWnd->Attach();
			} else {
				BofMessageBox("No PDA INVENTORY found", __FILE__);
				rc = ERR_UNKNOWN;
			}
		} else {
			// Already attached just update
			m_xInvWnd->AttachActiveObjects();
		}

		if (!m_xMapWnd) {
			pSDev = SDEVMNGR->GetStorageDevice(ZOOMMAPWLD);
			if (pSDev != nullptr) {
				m_xMapWnd = (CBagStorageDevBmp *)pSDev;
				m_xMapWnd->SetAssociateWnd(GetAssociateWnd());

				m_xMapWnd->SetTransparent(false);
				m_xMapWnd->SetVisible(false);
				rc = m_xMapWnd->Attach();
			} else {
				BofMessageBox("No PDA MAP found", __FILE__);
				rc = ERR_UNKNOWN;
			}
		} else {
			// Already attached just update
			m_xMapWnd->AttachActiveObjects();
		}

		if (!m_xLogWnd) {
			pSDev = SDEVMNGR->GetStorageDevice(ZOOMLOGWLD);
			if (pSDev != nullptr) {
				m_xLogWnd = (CBagStorageDevBmp *)pSDev;
				m_xLogWnd->SetAssociateWnd(GetAssociateWnd());

				m_xLogWnd->SetTransparent(false);
				m_xLogWnd->SetVisible(false);
				rc = m_xLogWnd->Attach();
			}
		} else {
			// Already attached just update
			m_xLogWnd->AttachActiveObjects();
		}

		if (m_ePdaMode == INVMODE) {
			ShowInventory();
		} else if (m_ePdaMode == MAPMODE) {
			ShowMap();
		} else if (m_ePdaMode == LOGMODE) {
			ShowLog();
		}

		Show();
		InvalidateRect(nullptr);
		UpdateWindow();
	}

	// Keep track of updates...
	g_lZoomPDALastUpdate = 0;

	return rc;
}

CBagObject *SBZoomPda::OnNewButtonObject(const CBofString &) {
	CBagButtonObject *PdaButtObj = new CBagButtonObject();
	PdaButtObj->SetCallBack(fPdaButtonHandler, (SBBasePda *)this);

	return PdaButtObj;
}

void SBZoomPda::OnLButtonUp(uint32 nFlags, CBofPoint *xPoint, void *) {
	// Need to override the CBagStorageDevWnd::OnLButtonUp(nFlags, xPoint)
	// to do our own thing.

	*m_xCursorLocation = *xPoint;
	CBofPoint xCursorLocation = DevPtToViewPort(*xPoint);

	CBagObject *pObj = GetObject(xCursorLocation, true);
	if (pObj != nullptr) {
		if (pObj->IsActive()) {
			pObj->OnLButtonUp(nFlags, xPoint);
			SetLActiveObject(pObj);
		}
	} else {
		SetLActivity(kMouseNONE);

		if (m_xCurDisplay) {
			CBofRect offset = CBagStorageDev::GetRect();
			xPoint->x -= offset.TopLeft().x;
			xPoint->y -= offset.TopLeft().y;

			// Make sure this stuff is nice and dirty before calling off
			// to the button handling routine, this assures that if we go from one screen
			// to the next, then we'll get redrawn.
			SetPreFilterPan(true);
			MakeListDirty(m_xCurDisplay->GetObjectList());

			m_xCurDisplay->OnLButtonUp(nFlags, xPoint, nullptr);
		} else {
			// We have no mode yet, then pass it to the default method
			if (m_ePdaMode == NOMODE) {
				CBagStorageDevWnd::OnLButtonUp(nFlags, xPoint);
			}
		}
	}

	// This thing could get trashed by the underlying code...
	if (IsCreated()) {
		InvalidateRect(nullptr);
		UpdateWindow();
	}
}

void SBZoomPda::OnMouseMove(uint32 nFlags, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));

	CBagStorageDev::OnMouseMove(nFlags, pPoint, GetAssociateWnd());


	// This should be on update cursor virtual func
	if (GetExitOnEdge() && (pPoint->x < GetExitOnEdge()) && (pPoint->y < 300) && !(GetPrevSDev().IsEmpty())) {
		CBagMasterWin::SetActiveCursor(10);
	} else {
		CBofRect cRect = GetBackdrop()->GetRect();
		CBagMasterWin::SetActiveCursor(GetProperCursor(*pPoint, cRect));
	}
}

void SBZoomPda::OnMainLoop() {
	uint32 nCurTime = GetTimer();

	// Force an update every 1/4 second
	if (m_ePdaMode == INVMODE || m_ePdaMode == MAPMODE) {
		if (nCurTime > (g_lZoomPDALastUpdate + 250)) {
			g_lZoomPDALastUpdate = nCurTime;

			SetPreFilterPan(true);
		}
	}

	CBagStorageDevWnd::OnMainLoop();
}

ErrorCode SBZoomPda::AttachActiveObjects() {
	SBBasePda::AttachActiveObjects();
	return CBagStorageDevWnd::AttachActiveObjects();
}

ErrorCode SBZoomPda::DetachActiveObjects() {
	SBBasePda::DetachActiveObjects();

	return CBagStorageDevWnd::DetachActiveObjects();
}

} // namespace Bagel
