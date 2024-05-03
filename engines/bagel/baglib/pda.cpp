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

extern bool g_allowAttachActiveObjectsFl;
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
	_xSDevType = SDEV_PDA;

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
		if (p->getFileName().Find(pMObj->getFileName()) == 0) {
			return;
		}
	}

	m_pMovieList->addToTail(pMObj);
}

CBagPDA::~CBagPDA() {
	Assert(IsValidObject(this));

	// Does not own list item, so no need to delete individual nodes
	if (m_pMovieList) {
		delete m_pMovieList;
		m_pMovieList = nullptr;
	}
}

ErrorCode CBagPDA::loadFile(const CBofString &sFile) {
	Assert(IsValidObject(this));

	ErrorCode error;

	error = CBagStorageDev::loadFile(sFile);
	if (_mooWnd) {
		RemoveObject(_mooWnd);
	}
	if (_invWnd) {
		RemoveObject(_invWnd);
	}
	if (_mapWnd) {
		RemoveObject(_mapWnd);
	}
	if (_logWnd) {
		RemoveObject(_logWnd);
	}

	return error;
}

#define MOOWLD      "MOO_WLD"
#define INVWLD      "INV_WLD"
#define MAPWLD      "MAP_WLD"
#define LOGWLD      "LOG_WLD"

ErrorCode CBagPDA::attach() {
	CBagStorageDev *pSDev;
	ErrorCode rc = CBagStorageDevBmp::attach();

	// Get PDA state info
	getPdaState();

	// Calculate the position for the pda
	CBofRect bmpRect = getBitmap()->getRect();
	CBofWindow *pGameWin = CBagel::getBagApp()->getMasterWnd()->getCurrentGameWindow();
	CBofRect GameRect = pGameWin->getRect();

	// When the pda is active it should sit flush with the bottom of the screen
	m_nActiveHeight = GameRect.height() - bmpRect.height();
	// When it is deactivated it should be the active height less the total movement distance
	m_nDeactiveHeight = GameRect.height() - (bmpRect.height() - (_moveDist * _numMoves));

	// Should be allowed to not find one.
	if (!_mooWnd) {
		if ((pSDev = SDEV_MANAGER->GetStorageDevice(MOOWLD)) != nullptr) {
			_mooWnd = (CBagStorageDevBmp *)pSDev;
			_mooWnd->SetAssociateWnd(GetAssociateWnd());
			_mooWnd->SetTransparent(false);
			_mooWnd->SetVisible(false);
			rc = _mooWnd->attach();
		}
	}

	if (!_invWnd) {
		if ((pSDev = SDEV_MANAGER->GetStorageDevice(INVWLD)) != nullptr) {
			_invWnd = (CBagStorageDevBmp *)pSDev;
			_invWnd->SetAssociateWnd(GetAssociateWnd());

			_invWnd->SetTransparent(false);
			_invWnd->SetVisible(false);
			rc = _invWnd->attach();
		} else {
			BofMessageBox("No PDA INVENTORY found", __FILE__);
			rc = ERR_UNKNOWN;
		}
	}

	if (!_mapWnd) {
		if ((pSDev = SDEV_MANAGER->GetStorageDevice(MAPWLD)) != nullptr) {
			_mapWnd = (CBagStorageDevBmp *)pSDev;
			_mapWnd->SetAssociateWnd(GetAssociateWnd());

			_mapWnd->SetTransparent(false);
			_mapWnd->SetVisible(false);
			rc = _mapWnd->attach();
		} else {
			BofMessageBox("No PDA MAP found", __FILE__);
			rc = ERR_UNKNOWN;
		}
	}
	if (!_logWnd) {
		if ((pSDev = SDEV_MANAGER->GetStorageDevice(LOGWLD)) != nullptr) {
			_logWnd = (CBagStorageDevBmp *)pSDev;
			_logWnd->SetAssociateWnd(GetAssociateWnd());

			_logWnd->SetTransparent(false);
			_logWnd->SetVisible(false);
			rc = _logWnd->attach();
		}
	}
	if (_pdaMode == INVMODE) {
		showInventory();
	} else if (_pdaMode == MAPMODE) {
		showMap();
	} else if (_pdaMode == LOGMODE) {
		showLog();
	}

	return rc;
}

void CBagPDA::setPosInWindow(int cx, int cy, int nDist) {
	CBofBitmap *pBmp = getBitmap();

	if (!pBmp)
		return;

	CBofRect bmpRect = pBmp->getRect();

	_moveDist = nDist;
	CBofPoint pt;
	pt.x = (cx - bmpRect.width()) / 2;

	if (_activated)
		pt.y = cy - bmpRect.height();
	else
		pt.y = cy - bmpRect.height() + _moveDist * _numMoves;

	SetRect(CBofRect(pt.x, pt.y, pt.x + pBmp->width() - 1, pt.y + pBmp->height() - 1));
}

bool CBagPDA::hideCurDisplay() {
	SBBasePda::hideCurDisplay();
	return true;
}

bool CBagPDA::restoreCurDisplay() {
	SBBasePda::restoreCurDisplay();
	return true;
}

bool CBagPDA::hideInventory() {
	SBBasePda::hideInventory();
	return true;
}

bool CBagPDA::showInventory() {
	SBBasePda::showInventory();

	return true;
}

ErrorCode CBagPDA::update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect, int /* nMaskColor */) {
	// Update the zoom button (it might need to blink).
	HandleZoomButton(false);
	ErrorCode errCode = ERR_NONE;
	if (!m_bHidePDA) {
		CBofRect *pr = pSrcRect;

		if (_activating) {

			CBofPoint loc = getPosition();
			_activating--;

			if (_activated) {
				if (loc.y > m_nActiveHeight) {
					loc.y -= _moveDist;
					if (pSrcRect) {
						CBofRect r = *pSrcRect;
						pSrcRect->bottom += _moveDist;
						pr = &r;
					}
				}
			} else if (loc.y < m_nDeactiveHeight) {
				loc.y += _moveDist;
				if (pSrcRect) {
					CBofRect r = *pSrcRect;
					pSrcRect->top -= _moveDist;
					pr = &r;
				}
			}

			setPosition(loc);

			pt = loc;
		}

		// We have gotten back from a zoom and we need to straighten up
		if (SBBasePda::_pdaMode == INVMODE && _curDisplay != _invWnd) {
			showInventory();

		} else if (SBBasePda::_pdaMode == MAPMODE && _curDisplay != _mapWnd) {

			showMap();
		} else if (SBBasePda::_pdaMode == LOGMODE && _curDisplay != _logWnd) {
			showLog();
		}

		bool bUpdate = true;
		bool bIsMovieWaiting = IsMovieWaiting();
		bool bMoviePlaying = false;

		if ((!isActivated()) &&                             // Must be down
		        ((_pdaMode == MAPMODE) ||
		         (bIsMovieWaiting && _pdaMode != MOOMODE))) {

			// Reset to reflect we know it happened
			SetPreFiltered(false);

			// Play the movie if it is ready.
			if (bIsMovieWaiting == true) {
				RunWaitingMovie();
			}
		} else if (_pdaMode == MOOMODE) {
			// If we're playing a pda movie, then make sure we continue to update.
			bMoviePlaying = true;
			bUpdate = true;
		}

		// If the official decree from on high has been given to update, do so!
		if (bUpdate) {
			errCode = CBagStorageDevBmp::update(pBmp, pt, pr, m_nMaskColor);
		}

		// If the PDA is activating then redraw our black background
		bool bWandAnimating = CBagCharacterObject::pdaWandAnimating();

		if (isActivating() || bWandAnimating || bMoviePlaying) {
			CBagStorageDevWnd *pMainWin = (CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev());
			((CBagPanWindow *)pMainWin)->SetPreFilterPan(true);
		} else if (!isActivated() && (SBBasePda::_pdaMode != MAPMODE)) {
			// If it is not activated, then don't bother redrawing it or the objects
			// inside of it.
			SetDirty(false);
		}
	}

	return errCode;
}


bool CBagPDA::isInside(const CBofPoint &xPoint) {
	CBofBitmap *pSrcBmp = getBitmap();

	if (getRect().PtInRect(xPoint) && m_nMaskColor >= 0) {
		if (pSrcBmp) {
			int x = xPoint.x - getRect().left;
			int y = xPoint.y - getRect().top;
			int c = pSrcBmp->readPixel(x, y);
			return (c != m_nMaskColor);
		}

		return true;
	}
	return false;
}

void CBagPDA::onLButtonUp(uint32 nFlags, CBofPoint *xPoint, void *info) {
	CBagStorageDevWnd *pMainWin = (CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev());

	if (!isActivated() && _pdaMode != INVMODE) {          // if the PDA is not active, activate it
		if (isInside(*xPoint)) {
			// Make sure the entire screen gets redrawn for an activate
			((CBagPanWindow *)pMainWin)->SetPreFilterPan(true);

			activate();
			SetDirty(true);
			attachActiveObjects();  // Forces PDA to be reevaluated.
		}
		return;

	}

	// Else, call the default func
	CBofPoint RealPt = DevPtToViewPort(*xPoint);

	if (_curDisplay && _curDisplay->getRect().PtInRect(RealPt)) {
		_curDisplay->onLButtonUp(nFlags, &RealPt, info);
	} else {
		// if not in the PDA view port then check and make sure it is activated.
		if (SBBasePda::_pdaMode == INVMODE && !isActivated()) {
			if (isInside(*xPoint)) {
				// Make sure the entire screen gets redrawn for an activate
				((CBagPanWindow *)pMainWin)->SetPreFilterPan(true);

				activate();
				SetDirty(true);
			}
			return;
		}

		// If it's in one of the buttons, then pass it off to the
		// sdev bmp code.
		if (isActivated()) {
			bool bButtonHit = false;
			CBofList<CBagObject *> *pList = GetObjectList();
			int  nCount = (pList == nullptr ? 0 : pList->GetCount());

			// Go through all the buttons and see if we hit any of them.
			for (int i = 0; i < nCount; i++) {
				CBagObject *pObj = pList->GetNodeItem(i);
				if (pObj->GetType() == BUTTONOBJ && pObj->getRect().PtInRect(RealPt)) {
					bButtonHit = true;
					break;
				}
			}

			// Deactivate the PDA if we didn't hit a button.
			if (bButtonHit || _pdaMode == NOMODE) {
				CBagStorageDevBmp::onLButtonUp(nFlags, xPoint, info);
			} else {
				((CBagPanWindow *)pMainWin)->SetPreFilterPan(true);
				deactivate();
			}
		}
	}

	// After a change of state, check if we should be flashing our
	// zoom button or not.
	HandleZoomButton(false);
}

void CBagPDA::onLButtonDown(uint32 nFlags, CBofPoint *xPoint, void *info) {
	// All we want to do here is if we had a mouse down on our
	// zoom button, then make sure we have the real zoom button current (that
	// is, if we have the inventory front and center).

	HandleZoomButton(true);

	CBagStorageDevBmp::onLButtonDown(nFlags, xPoint, info);
}

CBagObject *CBagPDA::onNewButtonObject(const CBofString &) {
	CBagButtonObject *PdaButtObj;

	PdaButtObj = new CBagButtonObject();

	PdaButtObj->setCallBack(pdaButtonHandler, (SBBasePda *)this);

	return PdaButtObj;
}

bool  CBagPDA::PaintFGObjects(CBofBitmap *pBmp) {
	if (_curDisplay) {
		// If we get here, then we are guaranteed that our pda
		// needs updating, so dirty the whole list before updating...
		// this assures that all objects will be updated (that are active).
		MakeListDirty(_curDisplay->GetObjectList());

		CBofRect tmp = getRect();
		_curDisplay->update(pBmp, getPosition(), &tmp);
	}

	return true;
}

CBagObject *CBagPDA::onNewUserObject(const CBofString &sInit) {
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

	char szLocalBuff[256];
	CBofString sDevice(szLocalBuff, 256);
	sDevice = "BPDA_WLD";

	CBagStorageDev *pPda = SDEV_MANAGER->GetStorageDevice(sDevice);
	if (pPda) {
		sDevice = PDA_ZOOMFLASH;
		pZoomFlash = (CBagButtonObject *)pPda->GetObject(sDevice);
		pZoomRegular = (CBagButtonObject *)pPda->GetObject(PDA_ZOOM);
	}

	// Only change the flashing state if we're not in a button down situation
	if (pZoomFlash && pZoomRegular && pZoomRegular->getState() != 1) {
		if (bButtonDown == false && _pdaMode == INVMODE && (_pdaPos == PDAUP) && _invWnd && _invWnd->GetNumFloatPages() > 1) {
			// Make the zoom button blink, to indicate more icons
			if (m_bFlashing == false) {
				// Don't allow attachActiveObjects() to be called in here
				g_allowAttachActiveObjectsFl = false;
				pPda->activateLocalObject(pZoomFlash);
				pPda->deactivateLocalObject(pZoomRegular);
				g_allowAttachActiveObjectsFl = true;

				pZoomFlash->setActive(true);
				pZoomRegular->setActive(false);

				pZoomFlash->setAnimated(true);
				pZoomFlash->SetAlwaysUpdate(true);

				m_bFlashing = true;
			}
		} else if (m_bFlashing) {
			// Don't allow attachActiveObjects() to be called in here
			g_allowAttachActiveObjectsFl = false;
			pPda->deactivateLocalObject(pZoomFlash);
			pPda->activateLocalObject(pZoomRegular);
			g_allowAttachActiveObjectsFl = true;

			pZoomFlash->setActive(false);
			pZoomRegular->setActive(true);

			pZoomFlash->setAnimated(false);
			pZoomFlash->SetAlwaysUpdate(false);

			m_bFlashing = false;
		}
	}
}

void CBagPDA::RemoveFromMovieQueue(CBagMovieObject *pMObj) {
	if (m_pMovieList != nullptr) {
		int nCount = m_pMovieList->GetCount();
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
	if (m_pMovieList) {
		int nCount = m_pMovieList->GetCount();
		if (nCount > 0) {
			for (int i = 0; i < nCount; i++) {
				CBagMovieObject *pMObj = m_pMovieList->GetNodeItem(i);
				if (pMObj->asynchPDAMovieCanPlay()) {
					m_bSoundsPaused = true;
					CSound::PauseSounds();              // pause all sounds
					pMObj->runObject();
					RemoveFromMovieQueue(pMObj);
				}
			}
		}
	}
}

ErrorCode CBagPDA::attachActiveObjects() {
	static bool bAlready = false;

	// If not already in this function
	if (!bAlready) {
		// Stop recursion
		bAlready = true;

		SBBasePda::attachActiveObjects();
		CBagStorageDevBmp::AttachActiveObjects();

		bAlready = false;
	}

	return _errCode;
}

ErrorCode CBagPDA::detachActiveObjects() {
	SBBasePda::detachActiveObjects();
	return CBagStorageDevBmp::DetachActiveObjects();
}

} // namespace Bagel
