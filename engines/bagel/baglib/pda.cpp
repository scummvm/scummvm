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

#include "bagel/baglib/pda.h"
#include "bagel/baglib/pan_window.h"
#include "bagel/boflib/app.h"
#include "bagel/boflib/sound.h"
#include "bagel/baglib/master_win.h"
#include "bagel/baglib/button_object.h"
#include "bagel/baglib/character_object.h"
#include "bagel/baglib/time_object.h"

namespace Bagel {

INT CBagPDA::m_nLastUpdate;
BOOL CBagPDA::m_bFlashing;
BOOL CBagPDA::m_bSoundsPaused;
CBofList<CBagMovieObject *> *CBagPDA::m_pMovieList;

extern BOOL g_bAllowAAO;
static BOOL g_bAutoUpdate;

void CBagPDA::initStatics() {
	m_nLastUpdate = 0;
	m_bFlashing = FALSE;
	m_bSoundsPaused = FALSE;
	m_pMovieList = nullptr;

	g_bAutoUpdate = false;
}

CBagPDA::CBagPDA(CBofWindow *pParent, const CBofRect &xRect, BOOL bActivated)
	: CBagStorageDevBmp(pParent, xRect),
	SBBasePda(pParent, xRect, bActivated) {
	m_xSDevType = SDEV_PDA;

	m_nActiveHeight = 0;
	m_nDeactiveHeight = 0;
}

VOID CBagPDA::AddToMovieQueue(CBagMovieObject *pMObj) {
	// Make sure we've got a list.
	if (m_pMovieList == nullptr) {
		m_pMovieList = new CBofList<CBagMovieObject *>;
	}

	// Handle simple case first, if it is marked for don't queue, then don't
	// add it at all.  This is done to insure that insignificant movies
	// such as "have you noticed your message light has been blinking" do not 
	// get queued (and subsequently play when they are no longer appropriate).
	if (pMObj->IsDontQueue()) {
		return;
	}

	Assert(m_pMovieList != nullptr);

	// Go through the whole movie list, make sure there are no dup's of 
	// this movie. 
	INT	nCount = m_pMovieList->GetCount();
	for (INT i = 0; i < nCount; i++) {
		CBagMovieObject *p = m_pMovieList->GetNodeItem(i);
		if (p->GetFileName().Find(pMObj->GetFileName()) == 0) {
			return;
		}
	}

	m_pMovieList->AddToTail(pMObj);
}

CBagPDA::~CBagPDA() {
	Assert(IsValidObject(this));

	// Does not own list item, so no need to delete individual nodes
	if (m_pMovieList) {
		delete m_pMovieList;
		m_pMovieList = nullptr;
	}
}

ERROR_CODE CBagPDA::LoadFile(const CBofString &sFile) {
	Assert(IsValidObject(this));

	ERROR_CODE error;

	error = CBagStorageDev::LoadFile(sFile);
	if (m_xMooWnd) {
		RemoveObject(m_xMooWnd);
	}
	if (m_xInvWnd) {
		RemoveObject(m_xInvWnd);
	}
	if (m_xMapWnd) {
		RemoveObject(m_xMapWnd);
	}
	if (m_xLogWnd) {
		RemoveObject(m_xLogWnd);
	}

	return error;
}

#define MOOWLD		"MOO_WLD"
#define INVWLD		"INV_WLD"
#define MAPWLD		"MAP_WLD"
#define LOGWLD		"LOG_WLD"

ERROR_CODE CBagPDA::Attach() {
	CBagStorageDev *pSDev;
	ERROR_CODE rc = CBagStorageDevBmp::Attach();

	// Get PDA state info
	GetPDAState();

	// Calculate the position for the pda
	CBofRect bmpRect = GetBitmap()->GetRect();
	CBofWindow *pGameWin = CBagel::GetBagApp()->GetMasterWnd()->GetCurrentGameWindow();
	CBofRect GameRect = pGameWin->GetRect();

	// When the pda is active it should sit flush with the bottom of the screen	
	m_nActiveHeight = GameRect.Height() - bmpRect.Height();
	// When it is deactivated it should be the active height less the total movement distance
	m_nDeactiveHeight = GameRect.Height() - (bmpRect.Height() - (m_nMoveDist * m_nNumMoves));

	CBofPoint cpt = GetPosition();
	CBofRect cRect = GetRect();

	// Should be allowed to not find one.
	if (!m_xMooWnd) {
		if ((pSDev = SDEVMNGR->GetStorageDevice(MOOWLD)) != nullptr) {
			m_xMooWnd = (CBagStorageDevBmp *)pSDev;
			m_xMooWnd->SetAssociateWnd(GetAssociateWnd());
			m_xMooWnd->SetTransparent(FALSE);
			m_xMooWnd->SetVisible(FALSE);
			rc = m_xMooWnd->Attach();
		}
	}

	if (!m_xInvWnd) {
		if ((pSDev = SDEVMNGR->GetStorageDevice(INVWLD)) != nullptr) {
			m_xInvWnd = (CBagStorageDevBmp *)pSDev;
			m_xInvWnd->SetAssociateWnd(GetAssociateWnd());

			m_xInvWnd->SetTransparent(FALSE);
			m_xInvWnd->SetVisible(FALSE);
			rc = m_xInvWnd->Attach();
		} else {
			BofMessageBox("No PDA INVENTORY found", __FILE__);
			rc = ERR_UNKNOWN;
		}
	}

	if (!m_xMapWnd) {
		if ((pSDev = SDEVMNGR->GetStorageDevice(MAPWLD)) != nullptr) {
			m_xMapWnd = (CBagStorageDevBmp *)pSDev;
			m_xMapWnd->SetAssociateWnd(GetAssociateWnd());

			m_xMapWnd->SetTransparent(FALSE);
			m_xMapWnd->SetVisible(FALSE);
			rc = m_xMapWnd->Attach();
		} else {
			BofMessageBox("No PDA MAP found", __FILE__);
			rc = ERR_UNKNOWN;
		}
	}
	if (!m_xLogWnd) {
		if ((pSDev = SDEVMNGR->GetStorageDevice(LOGWLD)) != nullptr) {
			m_xLogWnd = (CBagStorageDevBmp *)pSDev;
			m_xLogWnd->SetAssociateWnd(GetAssociateWnd());

			m_xLogWnd->SetTransparent(FALSE);
			m_xLogWnd->SetVisible(FALSE);
			rc = m_xLogWnd->Attach();
		}
	}
	if (m_ePdaMode == INVMODE) {
		ShowInventory();
	} else if (m_ePdaMode == MAPMODE) {
		ShowMap();
	} else if (m_ePdaMode == LOGMODE) {
		ShowLog();
	}

	return rc;
}

VOID CBagPDA::SetPosInWindow(int cx, int cy, int nDist) {
	CBofPoint pt;
	CBofBitmap *pBmp = GetBitmap();

	if (pBmp) {
		CBofRect bmpRect = pBmp->GetRect();

		m_nMoveDist = nDist;
		pt.x = (cx - bmpRect.Width()) / 2;

		if (m_bActivated)
			pt.y = cy - bmpRect.Height();
		else
			pt.y = cy - bmpRect.Height() + m_nMoveDist * m_nNumMoves;

		SetRect(CBofRect(pt.x, pt.y, pt.x + pBmp->Width() - 1, pt.y + pBmp->Height() - 1));
	}
}

BOOL CBagPDA::HideCurDisplay() {
	SBBasePda::HideCurDisplay();
	return TRUE;
}

BOOL CBagPDA::RestoreCurDisplay() {
	SBBasePda::RestoreCurDisplay();
	return TRUE;
}

BOOL CBagPDA::HideInventory() {
	SBBasePda::HideInventory();
	return TRUE;
}

BOOL CBagPDA::ShowInventory() {
	SBBasePda::ShowInventory();

	return TRUE;
}

ERROR_CODE CBagPDA::Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect, INT /* nMaskColor */) {
	ERROR_CODE errCode;
	CBofRect r;
	CBofRect *pr = pSrcRect;
	BOOL bMoviePlaying = FALSE;

	// Update the zoom button (it might need to blink). 

	HandleZoomButton(FALSE);
	errCode = ERR_NONE;
	if (!m_bHidePDA) {

		if (m_bActivating) {

			CBofPoint loc = GetPosition();
			m_bActivating--;

			if (m_bActivated) {
				if (loc.y > m_nActiveHeight) {
					loc.y -= m_nMoveDist;
					if (pSrcRect) {
						r = *pSrcRect;
						pSrcRect->bottom += m_nMoveDist;
						pr = &r;
					}
				}
			} else {
				if (loc.y < m_nDeactiveHeight) {
					loc.y += m_nMoveDist;
					if (pSrcRect) {
						r = *pSrcRect;
						pSrcRect->top -= m_nMoveDist;
						pr = &r;
					}
				}
			}

			SetPosition(loc);

			pt = loc;
		}

		// We have gotten back from a zoom and we need to straighten up
		if (SBBasePda::m_ePdaMode == INVMODE && m_xCurDisplay != m_xInvWnd) {
			ShowInventory();

		} else if (SBBasePda::m_ePdaMode == MAPMODE && m_xCurDisplay != m_xMapWnd) {

			ShowMap();
		} else if (SBBasePda::m_ePdaMode == LOGMODE && m_xCurDisplay != m_xLogWnd) {
			ShowLog();
		}

#define kPDAMapUpdateRate 250		// 4 frames / second

		BOOL bUpdate = TRUE;
		BOOL bIsMovieWaiting = IsMovieWaiting();

		if ((!IsActivated()) && 							// Must be down
			((m_ePdaMode == MAPMODE) ||
				(bIsMovieWaiting && m_ePdaMode != MOOMODE))) {

			INT nCurTime = GetTimer();
			INT nFrameInterval = kPDAMapUpdateRate;
			if ((nCurTime > m_nLastUpdate + nFrameInterval) ||
				(GetPreFiltered())) {

				// Record time of last update
				m_nLastUpdate = nCurTime;

				// Reset to reflect we know it happened
				SetPreFiltered(FALSE);

				// Play the movie if it is ready.
				if (bIsMovieWaiting == TRUE) {
					RunWaitingMovie();
				}
			} else {
				// Don't update, not time yet
				bUpdate = FALSE;
			}
		} else {
			// If we're playing a pda movie, then make sure we continue to update.
			if (m_ePdaMode == MOOMODE) {
				bMoviePlaying = TRUE;
				bUpdate = TRUE;
			}
		}

		// If the official decree from on high has been given to update, do so!
		if (bUpdate) {
			errCode = CBagStorageDevBmp::Update(pBmp, pt, pr, m_nMaskColor);
		}

		// If the PDA is activating then redraw our black background
		BOOL bWandAnimating = CBagCharacterObject::PDAWandAnimating();

		if (IsActivating() || bWandAnimating || bMoviePlaying) {
			CBagStorageDevWnd *pMainWin = (CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev());
			((CBagPanWindow *)pMainWin)->SetPreFilterPan(TRUE);
		} else {
			// If it is not activated, then don't bother redrawing it or the objects 
			// inside of it.  
			if (!IsActivated()) {
				if (SBBasePda::m_ePdaMode != MAPMODE) {
					SetDirty(FALSE);
				}
			}
		}
	}

	return errCode;
}


BOOL CBagPDA::IsInside(const CBofPoint &xPoint) {
	CBofBitmap *pSrcBmp = GetBitmap();

	if (GetRect().PtInRect(xPoint) && m_nMaskColor >= 0) {
		if (pSrcBmp) {
			int x = xPoint.x - GetRect().left;
			int y = xPoint.y - GetRect().top;
			int c = pSrcBmp->ReadPixel(x, y);
			return (c != m_nMaskColor);
		} else return TRUE;
	}
	return FALSE;
}

BOOL CBagPDA::OnLButtonUp(UINT nFlags, CBofPoint  xPoint, void *info) {
	CBagStorageDevWnd *pMainWin = (CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev());
	BOOL rc;

	if (!IsActivated() && m_ePdaMode != INVMODE) { 			// if the PDA is not active, activate it
		if (IsInside(xPoint)) {
			// Make sure the entire screen gets redrawn for an activate
			((CBagPanWindow *)pMainWin)->SetPreFilterPan(TRUE);

			Activate();
			SetDirty(TRUE);
			AttachActiveObjects();  // Forces PDA to be reevaluated.
		}
		return TRUE;

	} else {
		// Else, call the default func
		CBofPoint RealPt = DevPtToViewPort(xPoint);

		rc = FALSE;
		if (m_xCurDisplay && m_xCurDisplay->GetRect().PtInRect(RealPt)) {
			m_xCurDisplay->OnLButtonUp(nFlags, RealPt, info);
		} else {
			// if not in the PDA view port then check and make sure it is activated.
			if (SBBasePda::m_ePdaMode == INVMODE && !IsActivated()) {
				if (IsInside(xPoint)) {
					// Make sure the entire screen gets redrawn for an activate
					((CBagPanWindow *)pMainWin)->SetPreFilterPan(TRUE);

					Activate();
					SetDirty(TRUE);
				}
				return TRUE;
			}

			// If it's in one of the buttons, then pass it off to the 
			// sdev bmp code. 
			if (IsActivated()) {
				BOOL bButtonHit = FALSE;
				CBofList<CBagObject *> *pList = GetObjectList();
				INT  nCount = (pList == nullptr ? 0 : pList->GetCount());
				CBagObject *pObj;

				// Go through all the buttons and see if we hit any of them.
				for (INT i = 0; i < nCount; i++) {
					pObj = pList->GetNodeItem(i);
					if (pObj->GetType() == BUTTONOBJ && pObj->GetRect().PtInRect(RealPt)) {
						bButtonHit = TRUE;
						break;
					}
				}

				// Deactivate the PDA if we didn't hit a button.
				if (bButtonHit || m_ePdaMode == NOMODE) {
					rc = CBagStorageDevBmp::OnLButtonUp(nFlags, xPoint, info);
				} else {
					((CBagPanWindow *)pMainWin)->SetPreFilterPan(TRUE);
					Deactivate();
				}
			}
		}

		// After a change of state, check if we should be flashing our 
		// zoom button or not. 
		HandleZoomButton(FALSE);
		return rc;
	}
}

BOOL CBagPDA::OnLButtonDown(UINT nFlags, CBofPoint xPoint, void *info) {
	// All we want to do here is if we had a mouse down on our 
	// zoom button, then make sure we have the real zoom button current (that 
	// is, if we have the inventory front and center).

	HandleZoomButton(TRUE);

	return CBagStorageDevBmp::OnLButtonDown(nFlags, xPoint, info);
}

CBagObject *CBagPDA::OnNewButtonObject(const CBofString &) {
	CBagButtonObject *PdaButtObj;

	PdaButtObj = new CBagButtonObject();

	PdaButtObj->SetCallBack(fPdaButtonHandler, (SBBasePda *)this);

	return PdaButtObj;
}

BOOL  CBagPDA::PaintFGObjects(CBofBitmap *pBmp) {
	if (m_xCurDisplay) {
		// If we get here, then we are guaranteed that our pda 
		// needs updating, so dirty the whole list before updating...
		// this assures that all objects will be updated (that are active). 
		MakeListDirty(m_xCurDisplay->GetObjectList());

		CBofRect tmp = GetRect();
		m_xCurDisplay->Update(pBmp, GetPosition(), &tmp);
	}

	return TRUE;
}

CBagObject *CBagPDA::OnNewUserObject(const CBofString &sInit) {
	CBagTimeObject *pTimeObj;

	pTimeObj = nullptr;
	if (sInit == "TIME") {
		pTimeObj = new CBagTimeObject();
	}

	return pTimeObj;
}

VOID CBagPDA::HandleZoomButton(BOOL bButtonDown) {
	CBagButtonObject *pZoomRegular = nullptr;
	CBagButtonObject *pZoomFlash = nullptr;
	CBagStorageDev *pPda = nullptr;

	CHAR szLocalBuff[256];
	CBofString sDevice(szLocalBuff, 256);
	sDevice = "BPDA_WLD";

	pPda = SDEVMNGR->GetStorageDevice(sDevice);
	if (pPda) {
		sDevice = ZOOMFLASH;
		pZoomFlash = (CBagButtonObject *)pPda->GetObject(sDevice);

		pZoomRegular = (CBagButtonObject *)pPda->GetObject(ZOOM);
	}

	// Only change the flashing state if we're not in a button down situation
	if (pZoomFlash && pZoomRegular && pZoomRegular->GetState() != 1) {
		if (bButtonDown == FALSE && m_ePdaMode == INVMODE && (m_ePDAPos == PDAUP) && m_xInvWnd && m_xInvWnd->GetNumFloatPages() > 1) {
			// Make the zoom button blink, to indicate more icons
			if (m_bFlashing == FALSE) {
				// Don't allow AttachActiveObjects() to be called in here
				g_bAllowAAO = FALSE;
				pPda->ActivateLocalObject(pZoomFlash);
				pPda->DeactivateLocalObject(pZoomRegular);
				g_bAllowAAO = TRUE;

				pZoomFlash->SetActive(TRUE);
				pZoomRegular->SetActive(FALSE);

				pZoomFlash->SetAnimated(TRUE);
				pZoomFlash->SetAlwaysUpdate(TRUE);

				m_bFlashing = TRUE;
			}
		} else {
			if (m_bFlashing) {
				// Don't allow AttachActiveObjects() to be called in here
				g_bAllowAAO = FALSE;
				pPda->DeactivateLocalObject(pZoomFlash);
				pPda->ActivateLocalObject(pZoomRegular);
				g_bAllowAAO = TRUE;

				pZoomFlash->SetActive(FALSE);
				pZoomRegular->SetActive(TRUE);

				pZoomFlash->SetAnimated(FALSE);
				pZoomFlash->SetAlwaysUpdate(FALSE);

				m_bFlashing = FALSE;
			}
		}
	}
}

VOID CBagPDA::RemoveFromMovieQueue(CBagMovieObject *pMObj) {
	if (m_pMovieList != nullptr) {
		INT			nCount = m_pMovieList->GetCount();
		for (INT i = 0; i < nCount; i++) {
			CBagMovieObject *p = m_pMovieList->GetNodeItem(i);
			if (pMObj == p) {
				m_pMovieList->Remove(i);
				break;
			}
		}
	}
}

BOOL CBagPDA::IsMovieWaiting() {
	BOOL bMovieWaiting = FALSE;

	if (m_pMovieList) {
		bMovieWaiting = (m_pMovieList->GetCount() > 0);
	}

	// If our sounds are paused, and our movie is done playing, 
	// then start up our sounds again. 
	if (m_bSoundsPaused == TRUE && IsMoviePlaying() == FALSE) {
		CSound::ResumeSounds();
		m_bSoundsPaused = FALSE;
	}

	return bMovieWaiting;
}

VOID CBagPDA::RunWaitingMovie() {
	// Will only run a movie if it is ready to be run
	CBagMovieObject *pMObj;
	if (m_pMovieList) {
		INT nCount;
		if ((nCount = m_pMovieList->GetCount()) > 0) {
			for (INT i = 0; i < nCount; i++) {
				pMObj = m_pMovieList->GetNodeItem(i);
				if (pMObj->AsynchPDAMovieCanPlay()) {
					m_bSoundsPaused = TRUE;
					CSound::PauseSounds(); 				// pause all sounds
					pMObj->RunObject();
					RemoveFromMovieQueue(pMObj);
				}
			}
		}
	}
}

ERROR_CODE CBagPDA::AttachActiveObjects() {
	static BOOL bAlready = FALSE;

	// If not already in this function
	if (!bAlready) {
		// Stop recursion
		bAlready = TRUE;

		SBBasePda::AttachActiveObjects();
		CBagStorageDevBmp::AttachActiveObjects();

		bAlready = FALSE;
	}

	return(m_errCode);
}

ERROR_CODE CBagPDA::DetachActiveObjects() {
	SBBasePda::DetachActiveObjects();
	return CBagStorageDevBmp::DetachActiveObjects();
}

} // namespace Bagel
