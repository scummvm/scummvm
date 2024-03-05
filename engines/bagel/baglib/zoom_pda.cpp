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
static DWORD g_lZoomPDALastUpdate;

void SBZoomPda::initStatics() {
	g_lZoomPDALastUpdate = 0;
}

SBZoomPda::SBZoomPda(CBofWindow *pParent, const CBofRect &xRect, BOOL bActivated)
	: CBagStorageDevWnd(),
	  SBBasePda(pParent, xRect, bActivated) {
	m_xSDevType = SDEV_ZOOMPDA;
}

ERROR_CODE SBZoomPda::OnRender(CBofBitmap *pBmp, CBofRect *pRect) {
	Assert(IsValidObject(this));
	Assert(pBmp != NULL);

	// Paint the storage device
	if (pBmp != NULL) {
		BOOL bUpdate = FALSE;
		// Only clear the background and paint the backdrop if we've
		// been instructed to.

		if (PreFilterPan()) {
			PreFilter(pBmp, pRect, (m_xCurDisplay == NULL ? NULL : m_xCurDisplay->GetObjectList()));

			// Paint our storage device the first time through and the next time
			// through, this takes care of multiple text drawing problems (trust me!).
			if (m_bFirstPaint == FALSE) {
				SetPreFilterPan(FALSE);
			}

			if (GetWorkBmp() != NULL) {
				GetWorkBmp()->Paint(pBmp, pRect, pRect);
			}

			bUpdate = TRUE;
		}

		PaintStorageDevice(NULL, pBmp, pRect);

		// Paint the inventory or Map to backdrop
		if (bUpdate) {
			if (m_xCurDisplay != NULL) {
				m_xCurDisplay->Update(pBmp, m_xCurDisplay->GetPosition(), pRect);
			}
		}
	}

	return m_errCode;
}

ERROR_CODE SBZoomPda::LoadFile(const CBofString &sFile) {
	ERROR_CODE error;

	error = CBagStorageDev::LoadFile(sFile);
	RemoveObject(m_xMooWnd);
	RemoveObject(m_xInvWnd);
	RemoveObject(m_xMapWnd);
	RemoveObject(m_xLogWnd);

	return (error);
}

ERROR_CODE SBZoomPda::Detach() {
	BOOL bLogZoomed = (m_xLogWnd == m_xCurDisplay);

	// Other classes need to know if we're zoomed
	SetZoomed(FALSE);
	if (m_xInvWnd) {
		m_xInvWnd->Detach();
		m_xInvWnd = NULL;
	}

	if (m_xMooWnd) {
		m_xMooWnd->Detach();
		m_xMooWnd = NULL;
	}

	if (m_xMapWnd) {
		m_xMapWnd->Detach();
		m_xMapWnd = NULL;
	}

	if (m_xLogWnd) {
		m_xLogWnd->Detach();
		m_xLogWnd = NULL;
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

ERROR_CODE SBZoomPda::Attach() {
	CBagStorageDev *pSDev;
	ERROR_CODE rc;

	// Other classes need to know if we're zoomed
	SetZoomed(TRUE);

	if ((rc = CBagStorageDevWnd::Attach()) == ERR_NONE) {
		if (!m_xMooWnd) {
			if ((pSDev = SDEVMNGR->GetStorageDevice(ZOOMMOOWLD)) != NULL) {
				m_xMooWnd = (CBagStorageDevBmp *)pSDev;
				m_xMooWnd->SetAssociateWnd(GetAssociateWnd());
				m_xMooWnd->SetTransparent(FALSE);
				m_xMooWnd->SetVisible(FALSE);
				rc = m_xMooWnd->Attach();
			}
		} else {
			// Already attached just update
			m_xMooWnd->AttachActiveObjects();
		}

		if (!m_xInvWnd) {
			if ((pSDev = SDEVMNGR->GetStorageDevice(ZOOMINVWLD)) != NULL) {
				m_xInvWnd = (CBagStorageDevBmp *)pSDev;
				m_xInvWnd->SetAssociateWnd(GetAssociateWnd());

				m_xInvWnd->SetTransparent(FALSE);
				m_xInvWnd->SetVisible(FALSE);
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
			if ((pSDev = SDEVMNGR->GetStorageDevice(ZOOMMAPWLD)) != NULL) {
				m_xMapWnd = (CBagStorageDevBmp *)pSDev;
				m_xMapWnd->SetAssociateWnd(GetAssociateWnd());

				m_xMapWnd->SetTransparent(FALSE);
				m_xMapWnd->SetVisible(FALSE);
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
			if ((pSDev = SDEVMNGR->GetStorageDevice(ZOOMLOGWLD)) != NULL) {
				m_xLogWnd = (CBagStorageDevBmp *)pSDev;
				m_xLogWnd->SetAssociateWnd(GetAssociateWnd());

				m_xLogWnd->SetTransparent(FALSE);
				m_xLogWnd->SetVisible(FALSE);
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
		InvalidateRect(NULL);
		UpdateWindow();
	}

	// Keep track of updates...
	g_lZoomPDALastUpdate = 0;

	return rc;
}

CBagObject *SBZoomPda::OnNewButtonObject(const CBofString &) {
	CBagButtonObject *PdaButtObj;

	PdaButtObj = new CBagButtonObject();

	PdaButtObj->SetCallBack(fPdaButtonHandler, (SBBasePda *)this);

	return PdaButtObj;

}

VOID SBZoomPda::OnLButtonUp(UINT nFlags, CBofPoint *xPoint) {
	// Need to override the CBagStorageDevWnd::OnLButtonUp(nFlags, xPoint)
	// to do our own thing.
	CBagObject *pObj;

	m_xCursorLocation = *xPoint;
	CBofPoint xCursorLocation = DevPtToViewPort(*xPoint);

	if ((pObj = GetObject(xCursorLocation, TRUE)) != NULL) {
		if (pObj->IsActive()) {
			pObj->OnLButtonUp(nFlags, *xPoint);
			SetLActiveObject(pObj);
		}
	} else {
		SetLActivity(NONE);

		if (m_xCurDisplay) {
			CBofRect offset = CBagStorageDev::GetRect();
			xPoint->x -= offset.TopLeft().x;
			xPoint->y -= offset.TopLeft().y;

			// Make sure this stuff is nice and dirty before calling off
			// to the button handling routine, this assures that if we go from one screen
			// to the next, then we'll get redrawn.
			SetPreFilterPan(TRUE);
			MakeListDirty(m_xCurDisplay->GetObjectList());

			m_xCurDisplay->OnLButtonUp(nFlags, *xPoint, NULL);
		} else {
			// We have no mode yet, then pass it to the default method
			if (m_ePdaMode == NOMODE) {
				CBagStorageDevWnd::OnLButtonUp(nFlags, xPoint);
			}
		}
	}

	// This thing could get trashed by the underlying code...
	if (IsCreated()) {
		InvalidateRect(NULL);
		UpdateWindow();
	}
}

VOID SBZoomPda::OnMouseMove(UINT nFlags, CBofPoint *pPoint) {
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

VOID SBZoomPda::OnMainLoop() {
	DWORD       nCurTime = GetTimer();

	// Force an update every 1/4 second
	if (m_ePdaMode == INVMODE || m_ePdaMode == MAPMODE) {
		if (nCurTime > (g_lZoomPDALastUpdate + 250)) {
			g_lZoomPDALastUpdate = nCurTime;

			SetPreFilterPan(TRUE);
		}
	}

	CBagStorageDevWnd::OnMainLoop();
}

ERROR_CODE SBZoomPda::AttachActiveObjects() {
	SBBasePda::AttachActiveObjects();
	return CBagStorageDevWnd::AttachActiveObjects();
}

ERROR_CODE SBZoomPda::DetachActiveObjects() {
	SBBasePda::DetachActiveObjects();

	return CBagStorageDevWnd::DetachActiveObjects();
}

} // namespace Bagel
