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
#include "bagel/boflib/sound.h"
#include "bagel/baglib/master_win.h"
#include "bagel/baglib/button_object.h"
#include "bagel/baglib/character_object.h"
#include "bagel/baglib/time_object.h"

namespace Bagel {

bool CBagPDA::_flashingFl;
bool CBagPDA::_soundsPausedFl;
CBofList<CBagMovieObject *> *CBagPDA::_movieList;

extern bool g_allowattachActiveObjectsFl;
static bool g_bAutoUpdate;

void CBagPDA::initialize() {
	_flashingFl = false;
	_soundsPausedFl = false;
	_movieList = nullptr;

	g_bAutoUpdate = false;
}

CBagPDA::CBagPDA(CBofWindow *pParent, const CBofRect &xRect, bool bActivated)
	: CBagStorageDevBmp(pParent, xRect),
	  SBBasePda(pParent, xRect, bActivated) {
	_xSDevType = SDEV_PDA;

	_activeHeight = 0;
	_deactiveHeight = 0;
}

void CBagPDA::addToMovieQueue(CBagMovieObject *pMObj) {
	// Make sure we've got a list.
	if (_movieList == nullptr) {
		_movieList = new CBofList<CBagMovieObject *>;
	}

	// Handle simple case first, if it is marked for don't queue, then don't
	// add it at all.  This is done to insure that insignificant movies
	// such as "have you noticed your message light has been blinking" do not
	// get queued (and subsequently play when they are no longer appropriate).
	if (pMObj->isDontQueue()) {
		return;
	}

	assert(_movieList != nullptr);

	// Go through the whole movie list, make sure there are no dup's of
	// this movie.
	int nCount = _movieList->getCount();
	for (int i = 0; i < nCount; i++) {
		CBagMovieObject *p = _movieList->getNodeItem(i);
		if (p->getFileName().find(pMObj->getFileName()) == 0) {
			return;
		}
	}

	_movieList->addToTail(pMObj);
}

CBagPDA::~CBagPDA() {
	assert(isValidObject(this));

	// Does not own list item, so no need to delete individual nodes
	delete _movieList;
	_movieList = nullptr;
}

ErrorCode CBagPDA::loadFile(const CBofString &sFile) {
	assert(isValidObject(this));

	ErrorCode errorCode = CBagStorageDev::loadFile(sFile);
	
	if (_mooWnd) {
		removeObject(_mooWnd);
	}
	if (_invWnd) {
		removeObject(_invWnd);
	}
	if (_mapWnd) {
		removeObject(_mapWnd);
	}
	if (_logWnd) {
		removeObject(_logWnd);
	}

	return errorCode;
}

ErrorCode CBagPDA::attach() {
	CBagStorageDev *pSDev;
	ErrorCode errorCode = CBagStorageDevBmp::attach();

	// Get PDA state info
	getPdaState();

	// Calculate the position for the pda
	CBofRect bmpRect = getBitmap()->getRect();
	CBofWindow *pGameWin = CBagel::getBagApp()->getMasterWnd()->getCurrentGameWindow();
	CBofRect GameRect = pGameWin->getRect();

	// When the pda is active it should sit flush with the bottom of the screen
	_activeHeight = GameRect.height() - bmpRect.height();
	// When it is deactivated it should be the active height less the total movement distance
	_deactiveHeight = GameRect.height() - (bmpRect.height() - (_moveDist * _numMoves));

	// Should be allowed to not find one.
	if (!_mooWnd) {
		pSDev = g_SDevManager->getStorageDevice(MOO_WLD);
		if (pSDev != nullptr) {
			_mooWnd = (CBagStorageDevBmp *)pSDev;
			_mooWnd->setAssociateWnd(getAssociateWnd());
			_mooWnd->setTransparent(false);
			_mooWnd->setVisible(false);
			errorCode = _mooWnd->attach();
		}
	}

	if (!_invWnd) {
		pSDev = g_SDevManager->getStorageDevice(INV_WLD);
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
	}

	if (!_mapWnd) {
		pSDev = g_SDevManager->getStorageDevice(MAP_WLD);
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
	}
	if (!_logWnd) {
		pSDev = g_SDevManager->getStorageDevice(LOG_WLD);
		if (pSDev != nullptr) {
			_logWnd = (CBagStorageDevBmp *)pSDev;
			_logWnd->setAssociateWnd(getAssociateWnd());

			_logWnd->setTransparent(false);
			_logWnd->setVisible(false);
			errorCode = _logWnd->attach();
		}
	}
	if (_pdaMode == PDA_INV_MODE) {
		showInventory();
	} else if (_pdaMode == PDA_MAP_MODE) {
		showMap();
	} else if (_pdaMode == PDA_LOG_MODE) {
		showLog();
	}

	return errorCode;
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

	setRect(CBofRect(pt.x, pt.y, pt.x + pBmp->width() - 1, pt.y + pBmp->height() - 1));
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
	handleZoomButton(false);
	ErrorCode errorCode = ERR_NONE;

	if (_hidePdaFl)
		return errorCode;
	
	CBofRect r;
	CBofRect *pr = pSrcRect;

	if (_activating) {

		CBofPoint loc = getPosition();
		_activating--;

		if (_activated) {
			if (loc.y > _activeHeight) {
				loc.y -= _moveDist;
				if (pSrcRect) {
					r = *pSrcRect;
					pSrcRect->bottom += _moveDist;
					pr = &r;
				}
			}
		} else if (loc.y < _deactiveHeight) {
			loc.y += _moveDist;
			if (pSrcRect) {
				r = *pSrcRect;
				pSrcRect->top -= _moveDist;
				pr = &r;
			}
		}

		setPosition(loc);

		pt = loc;
	}

	// We have gotten back from a zoom and we need to straighten up
	if (SBBasePda::_pdaMode == PDA_INV_MODE && _curDisplay != _invWnd) {
		showInventory();

	} else if (SBBasePda::_pdaMode == PDA_MAP_MODE && _curDisplay != _mapWnd) {

		showMap();
	} else if (SBBasePda::_pdaMode == PDA_LOG_MODE && _curDisplay != _logWnd) {
		showLog();
	}

	bool bIsMovieWaiting = isMovieWaiting();
	bool bMoviePlaying = false;

	if ((!isActivated()) &&                             // Must be down
	        ((_pdaMode == PDA_MAP_MODE) ||
	         (bIsMovieWaiting && _pdaMode != PDA_MOO_MODE))) {

		// Reset to reflect we know it happened
		setPreFiltered(false);

		// Play the movie if it is ready.
		if (bIsMovieWaiting == true) {
			runWaitingMovie();
		}
	} else if (_pdaMode == PDA_MOO_MODE) {
		// If we're playing a pda movie, then make sure we continue to update.
		bMoviePlaying = true;
	}

	// If the official decree from on high has been given to update, do so!
	errorCode = CBagStorageDevBmp::update(pBmp, pt, pr, _nMaskColor);

	// If the PDA is activating then redraw our black background
	bool bWandAnimating = CBagCharacterObject::pdaWandAnimating();

	if (isActivating() || bWandAnimating || bMoviePlaying) {
		CBagStorageDevWnd *pMainWin = (CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev());
		((CBagPanWindow *)pMainWin)->setPreFilterPan(true);
	} else if (!isActivated() && (SBBasePda::_pdaMode != PDA_MAP_MODE)) {
		// If it is not activated, then don't bother redrawing it or the objects
		// inside of it.
		setDirty(false);
	}

	return errorCode;
}


bool CBagPDA::isInside(const CBofPoint &xPoint) {
	CBofBitmap *pSrcBmp = getBitmap();

	if (getRect().ptInRect(xPoint) && _nMaskColor >= 0) {
		if (pSrcBmp) {
			int x = xPoint.x - getRect().left;
			int y = xPoint.y - getRect().top;
			int c = pSrcBmp->readPixel(x, y);
			return (c != _nMaskColor);
		}

		return true;
	}
	return false;
}

void CBagPDA::onLButtonUp(uint32 nFlags, CBofPoint *xPoint, void *info) {
	CBagStorageDevWnd *pMainWin = (CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev());

	if (!isActivated() && _pdaMode != PDA_INV_MODE) {          // if the PDA is not active, activate it
		if (isInside(*xPoint)) {
			// Make sure the entire screen gets redrawn for an activate
			((CBagPanWindow *)pMainWin)->setPreFilterPan(true);

			activate();
			setDirty(true);
			attachActiveObjects();  // Forces PDA to be reevaluated.
		}
		return;

	}

	// Else, call the default func
	CBofPoint RealPt = devPtToViewPort(*xPoint);

	if (_curDisplay && _curDisplay->getRect().ptInRect(RealPt)) {
		_curDisplay->onLButtonUp(nFlags, &RealPt, info);
	} else {
		// if not in the PDA view port then check and make sure it is activated.
		if (SBBasePda::_pdaMode == PDA_INV_MODE && !isActivated()) {
			if (isInside(*xPoint)) {
				// Make sure the entire screen gets redrawn for an activate
				((CBagPanWindow *)pMainWin)->setPreFilterPan(true);

				activate();
				setDirty(true);
			}
			return;
		}

		// If it's in one of the buttons, then pass it off to the
		// sdev bmp code.
		if (isActivated()) {
			bool bButtonHit = false;
			CBofList<CBagObject *> *pList = getObjectList();
			int  nCount = (pList == nullptr ? 0 : pList->getCount());

			// Go through all the buttons and see if we hit any of them.
			for (int i = 0; i < nCount; i++) {
				CBagObject *pObj = pList->getNodeItem(i);
				if (pObj->getType() == BUTTON_OBJ && pObj->getRect().ptInRect(RealPt)) {
					bButtonHit = true;
					break;
				}
			}

			// Deactivate the PDA if we didn't hit a button.
			if (bButtonHit || _pdaMode == PDA_NO_MODE) {
				CBagStorageDevBmp::onLButtonUp(nFlags, xPoint, info);
			} else {
				((CBagPanWindow *)pMainWin)->setPreFilterPan(true);
				deactivate();
			}
		}
	}

	// After a change of state, check if we should be flashing our
	// zoom button or not.
	handleZoomButton(false);
}

void CBagPDA::onLButtonDown(uint32 nFlags, CBofPoint *xPoint, void *info) {
	// All we want to do here is if we had a mouse down on our
	// zoom button, then make sure we have the real zoom button current (that
	// is, if we have the inventory front and center).

	handleZoomButton(true);

	CBagStorageDevBmp::onLButtonDown(nFlags, xPoint, info);
}

CBagObject *CBagPDA::onNewButtonObject(const CBofString &) {
	CBagButtonObject *PdaButtObj = new CBagButtonObject();
	PdaButtObj->setCallBack(pdaButtonHandler, (SBBasePda *)this);

	return PdaButtObj;
}

bool  CBagPDA::paintFGObjects(CBofBitmap *pBmp) {
	if (_curDisplay) {
		// If we get here, then we are guaranteed that our pda
		// needs updating, so dirty the whole list before updating...
		// this assures that all objects will be updated (that are active).
		makeListDirty(_curDisplay->getObjectList());

		CBofRect tmp = getRect();
		_curDisplay->update(pBmp, getPosition(), &tmp);
	}

	return true;
}

CBagObject *CBagPDA::onNewUserObject(const CBofString &sInit) {
	CBagTimeObject *pTimeObj = nullptr;
	if (sInit == "TIME") {
		pTimeObj = new CBagTimeObject();
	}

	return pTimeObj;
}

void CBagPDA::handleZoomButton(bool bButtonDown) {
	CBagButtonObject *pZoomRegular = nullptr;
	CBagButtonObject *pZoomFlash = nullptr;

	char szLocalBuff[256];
	CBofString sDevice(szLocalBuff, 256);
	sDevice = "BPDA_WLD";

	CBagStorageDev *pPda = g_SDevManager->getStorageDevice(sDevice);
	if (pPda) {
		sDevice = PDA_ZOOMFLASH;
		pZoomFlash = (CBagButtonObject *)pPda->getObject(sDevice);
		pZoomRegular = (CBagButtonObject *)pPda->getObject(PDA_ZOOM);
	}

	// Only change the flashing state if we're not in a button down situation
	if (pZoomFlash && pZoomRegular && pZoomRegular->getState() != 1) {
		if (bButtonDown == false && _pdaMode == PDA_INV_MODE && (_pdaPos == PDA_UP) && _invWnd && _invWnd->getNumFloatPages() > 1) {
			// Make the zoom button blink, to indicate more icons
			if (_flashingFl == false) {
				// Don't allow attachActiveObjects() to be called in here
				g_allowattachActiveObjectsFl = false;
				pPda->activateLocalObject(pZoomFlash);
				pPda->deactivateLocalObject(pZoomRegular);
				g_allowattachActiveObjectsFl = true;

				pZoomFlash->setActive(true);
				pZoomRegular->setActive(false);

				pZoomFlash->setAnimated(true);
				pZoomFlash->setAlwaysUpdate(true);

				_flashingFl = true;
			}
		} else if (_flashingFl) {
			// Don't allow attachActiveObjects() to be called in here
			g_allowattachActiveObjectsFl = false;
			pPda->deactivateLocalObject(pZoomFlash);
			pPda->activateLocalObject(pZoomRegular);
			g_allowattachActiveObjectsFl = true;

			pZoomFlash->setActive(false);
			pZoomRegular->setActive(true);

			pZoomFlash->setAnimated(false);
			pZoomFlash->setAlwaysUpdate(false);

			_flashingFl = false;
		}
	}
}

void CBagPDA::removeFromMovieQueue(CBagMovieObject *pMObj) {
	if (_movieList == nullptr)
		return;

	int nCount = _movieList->getCount();
	for (int i = 0; i < nCount; i++) {
		CBagMovieObject *p = _movieList->getNodeItem(i);
		if (pMObj == p) {
			_movieList->remove(i);
			break;
		}
	}
}

bool CBagPDA::isMovieWaiting() {
	bool bMovieWaiting = false;

	if (_movieList) {
		bMovieWaiting = (_movieList->getCount() > 0);
	}

	// If our sounds are paused, and our movie is done playing,
	// then start up our sounds again.
	if (_soundsPausedFl == true && isMoviePlaying() == false) {
		CSound::resumeSounds();
		_soundsPausedFl = false;
	}

	return bMovieWaiting;
}

void CBagPDA::runWaitingMovie() {
	// Will only run a movie if it is ready to be run
	if (!_movieList)
	return;
	
	int nCount = _movieList->getCount();
	for (int i = 0; i < nCount; i++) {
		CBagMovieObject *pMObj = _movieList->getNodeItem(i);
		if (pMObj->asynchPDAMovieCanPlay()) {
			_soundsPausedFl = true;
			// pause all sounds
			CSound::pauseSounds();
			pMObj->runObject();
			removeFromMovieQueue(pMObj);
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
		CBagStorageDevBmp::attachActiveObjects();

		bAlready = false;
	}

	return _errCode;
}

ErrorCode CBagPDA::detachActiveObjects() {
	SBBasePda::detachActiveObjects();
	return CBagStorageDevBmp::detachActiveObjects();
}

} // namespace Bagel
