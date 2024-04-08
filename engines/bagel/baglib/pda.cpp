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

bool CBagPDA::m_bFlashing;
bool CBagPDA::m_bSoundsPaused;
CBofList<CBagMovieObject *> *CBagPDA::m_pMovieList;

extern bool g_bAllowAAO;
static bool g_bAutoUpdate;

void CBagPDA::initialize() {
	m_bFlashing = false;
	m_bSoundsPaused = false;
	m_pMovieList = nullptr;

	g_bAutoUpdate = false;
}

CBagPDA::CBagPDA(CBofWindow *pParent, const CBofRect &xRect, bool bActivated)
	: CBagStorageDevBmp(pParent, xRect),
	  SBBasePda(pParent, xRect, bActivated) {
	m_xSDevType = SDEV_PDA;

	m_nActiveHeight = 0;
	m_nDeactiveHeight = 0;
}

void CBagPDA::AddToMovieQueue(CBagMovieObject *pMObj) {
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
	int nCount = m_pMovieList->GetCount();
	for (int i = 0; i < nCount; i++) {
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

ErrorCode CBagPDA::LoadFile(const CBofString &sFile) {
	Assert(IsValidObject(this));

	ErrorCode error;

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

#define MOOWLD      "MOO_WLD"
#define INVWLD      "INV_WLD"
#define MAPWLD      "MAP_WLD"
#define LOGWLD      "LOG_WLD"

ErrorCode CBagPDA::Attach() {
	CBagStorageDev *pSDev;
	ErrorCode rc = CBagStorageDevBmp::Attach();

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

	// Should be allowed to not find one.
	if (!m_xMooWnd) {
		if ((pSDev = SDEVMNGR->GetStorageDevice(MOOWLD)) != nullptr) {
			m_xMooWnd = (CBagStorageDevBmp *)pSDev;
			m_xMooWnd->SetAssociateWnd(GetAssociateWnd());
			m_xMooWnd->SetTransparent(false);
			m_xMooWnd->SetVisible(false);
			rc = m_xMooWnd->Attach();
		}
	}

	if (!m_xInvWnd) {
		if ((pSDev = SDEVMNGR->GetStorageDevice(INVWLD)) != nullptr) {
			m_xInvWnd = (CBagStorageDevBmp *)pSDev;
			m_xInvWnd->SetAssociateWnd(GetAssociateWnd());

			m_xInvWnd->SetTransparent(false);
			m_xInvWnd->SetVisible(false);
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

			m_xMapWnd->SetTransparent(false);
			m_xMapWnd->SetVisible(false);
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

			m_xLogWnd->SetTransparent(false);
			m_xLogWnd->SetVisible(false);
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

void CBagPDA::SetPosInWindow(int cx, int cy, int nDist) {
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

bool CBagPDA::HideCurDisplay() {
	SBBasePda::HideCurDisplay();
	return true;
}

bool CBagPDA::RestoreCurDisplay() {
	SBBasePda::RestoreCurDisplay();
	return true;
}

bool CBagPDA::HideInventory() {
	SBBasePda::HideInventory();
	return true;
}

bool CBagPDA::ShowInventory() {
	SBBasePda::ShowInventory();

	return true;
}

ErrorCode CBagPDA::Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect, int /* nMaskColor */) {
	ErrorCode errCode;
	CBofRect r;
	CBofRect *pr = pSrcRect;
	bool bMoviePlaying = false;

	// Update the zoom button (it might need to blink).

	HandleZoomButton(false);
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

		bool bUpdate = true;
		bool bIsMovieWaiting = IsMovieWaiting();

		if ((!IsActivated()) &&                             // Must be down
		        ((m_ePdaMode == MAPMODE) ||
		         (bIsMovieWaiting && m_ePdaMode != MOOMODE))) {

			// Reset to reflect we know it happened
			SetPreFiltered(false);

			// Play the movie if it is ready.
			if (bIsMovieWaiting == true) {
				RunWaitingMovie();
			}
		} else {
			// If we're playing a pda movie, then make sure we continue to update.
			if (m_ePdaMode == MOOMODE) {
				bMoviePlaying = true;
				bUpdate = true;
			}
		}

		// If the official decree from on high has been given to update, do so!
		if (bUpdate) {
			errCode = CBagStorageDevBmp::Update(pBmp, pt, pr, m_nMaskColor);
		}

		// If the PDA is activating then redraw our black background
		bool bWandAnimating = CBagCharacterObject::PDAWandAnimating();

		if (IsActivating() || bWandAnimating || bMoviePlaying) {
			CBagStorageDevWnd *pMainWin = (CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev());
			((CBagPanWindow *)pMainWin)->SetPreFilterPan(true);
		} else {
			// If it is not activated, then don't bother redrawing it or the objects
			// inside of it.
			if (!IsActivated()) {
				if (SBBasePda::m_ePdaMode != MAPMODE) {
					SetDirty(false);
				}
			}
		}
	}

	return errCode;
}


bool CBagPDA::IsInside(const CBofPoint &xPoint) {
	CBofBitmap *pSrcBmp = GetBitmap();

	if (GetRect().PtInRect(xPoint) && m_nMaskColor >= 0) {
		if (pSrcBmp) {
			int x = xPoint.x - GetRect().left;
			int y = xPoint.y - GetRect().top;
			int c = pSrcBmp->ReadPixel(x, y);
			return (c != m_nMaskColor);
		} else return true;
	}
	return false;
}

void CBagPDA::OnLButtonUp(uint32 nFlags, CBofPoint *xPoint, void *info) {
	CBagStorageDevWnd *pMainWin = (CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev());

	if (!IsActivated() && m_ePdaMode != INVMODE) {          // if the PDA is not active, activate it
		if (IsInside(*xPoint)) {
			// Make sure the entire screen gets redrawn for an activate
			((CBagPanWindow *)pMainWin)->SetPreFilterPan(true);

			Activate();
			SetDirty(true);
			AttachActiveObjects();  // Forces PDA to be reevaluated.
		}
		return;

	} else {
		// Else, call the default func
		CBofPoint RealPt = DevPtToViewPort(*xPoint);

		if (m_xCurDisplay && m_xCurDisplay->GetRect().PtInRect(RealPt)) {
			m_xCurDisplay->OnLButtonUp(nFlags, &RealPt, info);
		} else {
			// if not in the PDA view port then check and make sure it is activated.
			if (SBBasePda::m_ePdaMode == INVMODE && !IsActivated()) {
				if (IsInside(*xPoint)) {
					// Make sure the entire screen gets redrawn for an activate
					((CBagPanWindow *)pMainWin)->SetPreFilterPan(true);

					Activate();
					SetDirty(true);
				}
				return;
			}

			// If it's in one of the buttons, then pass it off to the
			// sdev bmp code.
			if (IsActivated()) {
				bool bButtonHit = false;
				CBofList<CBagObject *> *pList = GetObjectList();
				int  nCount = (pList == nullptr ? 0 : pList->GetCount());
				CBagObject *pObj;

				// Go through all the buttons and see if we hit any of them.
				for (int i = 0; i < nCount; i++) {
					pObj = pList->GetNodeItem(i);
					if (pObj->GetType() == BUTTONOBJ && pObj->GetRect().PtInRect(RealPt)) {
						bButtonHit = true;
						break;
					}
				}

				// Deactivate the PDA if we didn't hit a button.
				if (bButtonHit || m_ePdaMode == NOMODE) {
					CBagStorageDevBmp::OnLButtonUp(nFlags, xPoint, info);
				} else {
					((CBagPanWindow *)pMainWin)->SetPreFilterPan(true);
					Deactivate();
				}
			}
		}

		// After a change of state, check if we should be flashing our
		// zoom button or not.
		HandleZoomButton(false);
	}
}

void CBagPDA::OnLButtonDown(uint32 nFlags, CBofPoint *xPoint, void *info) {
	// All we want to do here is if we had a mouse down on our
	// zoom button, then make sure we have the real zoom button current (that
	// is, if we have the inventory front and center).

	HandleZoomButton(true);

	CBagStorageDevBmp::OnLButtonDown(nFlags, xPoint, info);
}

CBagObject *CBagPDA::OnNewButtonObject(const CBofString &) {
	CBagButtonObject *PdaButtObj;

	PdaButtObj = new CBagButtonObject();

	PdaButtObj->SetCallBack(fPdaButtonHandler, (SBBasePda *)this);

	return PdaButtObj;
}

bool  CBagPDA::PaintFGObjects(CBofBitmap *pBmp) {
	if (m_xCurDisplay) {
		// If we get here, then we are guaranteed that our pda
		// needs updating, so dirty the whole list before updating...
		// this assures that all objects will be updated (that are active).
		MakeListDirty(m_xCurDisplay->GetObjectList());

		CBofRect tmp = GetRect();
		m_xCurDisplay->Update(pBmp, GetPosition(), &tmp);
	}

	return true;
}

CBagObject *CBagPDA::OnNewUserObject(const CBofString &sInit) {
	CBagTimeObject *pTimeObj;

	pTimeObj = nullptr;
	if (sInit == "TIME") {
		pTimeObj = new CBagTimeObject();
	}

	return pTimeObj;
}

void CBagPDA::HandleZoomButton(bool bButtonDown) {
	CBagButtonObject *pZoomRegular = nullptr;
	CBagButtonObject *pZoomFlash = nullptr;
	CBagStorageDev *pPda = nullptr;

	char szLocalBuff[256];
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
		if (bButtonDown == false && m_ePdaMode == INVMODE && (m_ePDAPos == PDAUP) && m_xInvWnd && m_xInvWnd->GetNumFloatPages() > 1) {
			// Make the zoom button blink, to indicate more icons
			if (m_bFlashing == false) {
				// Don't allow AttachActiveObjects() to be called in here
				g_bAllowAAO = false;
				pPda->ActivateLocalObject(pZoomFlash);
				pPda->DeactivateLocalObject(pZoomRegular);
				g_bAllowAAO = true;

				pZoomFlash->SetActive(true);
				pZoomRegular->SetActive(false);

				pZoomFlash->SetAnimated(true);
				pZoomFlash->SetAlwaysUpdate(true);

				m_bFlashing = true;
			}
		} else {
			if (m_bFlashing) {
				// Don't allow AttachActiveObjects() to be called in here
				g_bAllowAAO = false;
				pPda->DeactivateLocalObject(pZoomFlash);
				pPda->ActivateLocalObject(pZoomRegular);
				g_bAllowAAO = true;

				pZoomFlash->SetActive(false);
				pZoomRegular->SetActive(true);

				pZoomFlash->SetAnimated(false);
				pZoomFlash->SetAlwaysUpdate(false);

				m_bFlashing = false;
			}
		}
	}
}

void CBagPDA::RemoveFromMovieQueue(CBagMovieObject *pMObj) {
	if (m_pMovieList != nullptr) {
		int         nCount = m_pMovieList->GetCount();
		for (int i = 0; i < nCount; i++) {
			CBagMovieObject *p = m_pMovieList->GetNodeItem(i);
			if (pMObj == p) {
				m_pMovieList->Remove(i);
				break;
			}
		}
	}
}

bool CBagPDA::IsMovieWaiting() {
	bool bMovieWaiting = false;

	if (m_pMovieList) {
		bMovieWaiting = (m_pMovieList->GetCount() > 0);
	}

	// If our sounds are paused, and our movie is done playing,
	// then start up our sounds again.
	if (m_bSoundsPaused == true && IsMoviePlaying() == false) {
		CSound::ResumeSounds();
		m_bSoundsPaused = false;
	}

	return bMovieWaiting;
}

void CBagPDA::RunWaitingMovie() {
	// Will only run a movie if it is ready to be run
	CBagMovieObject *pMObj;
	if (m_pMovieList) {
		int nCount;
		if ((nCount = m_pMovieList->GetCount()) > 0) {
			for (int i = 0; i < nCount; i++) {
				pMObj = m_pMovieList->GetNodeItem(i);
				if (pMObj->AsynchPDAMovieCanPlay()) {
					m_bSoundsPaused = true;
					CSound::PauseSounds();              // pause all sounds
					pMObj->RunObject();
					RemoveFromMovieQueue(pMObj);
				}
			}
		}
	}
}

ErrorCode CBagPDA::AttachActiveObjects() {
	static bool bAlready = false;

	// If not already in this function
	if (!bAlready) {
		// Stop recursion
		bAlready = true;

		SBBasePda::AttachActiveObjects();
		CBagStorageDevBmp::AttachActiveObjects();

		bAlready = false;
	}

	return m_errCode;
}

ErrorCode CBagPDA::DetachActiveObjects() {
	SBBasePda::DetachActiveObjects();
	return CBagStorageDevBmp::DetachActiveObjects();
}

} // namespace Bagel
