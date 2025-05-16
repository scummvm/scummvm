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

#include "bagel/spacebar/baglib/pan_window.h"
#include "bagel/spacebar/baglib/res.h"
#include "bagel/spacebar/baglib/master_win.h"
#include "bagel/spacebar/baglib/bagel.h"
#include "bagel/spacebar/baglib/wield.h"
#include "bagel/spacebar/boflib/gfx/sprite.h"
#include "bagel/spacebar/baglib/button_object.h"
#include "bagel/spacebar/baglib/character_object.h"
#include "bagel/spacebar/baglib/text_object.h"
#include "bagel/spacebar/boflib/std_keys.h"

namespace Bagel {
namespace SpaceBar {

CBagPDA *CBagPanWindow::_pPDABmp;      // Pointer to the PDA object
int CBagPanWindow::_nCorrection;
int CBagPanWindow::_nPanSpeed;
CBagWield *CBagPanWindow::_pWieldBmp;  // Pointer to the WIELD object

void CBagPanWindow::initialize() {
	_pPDABmp = nullptr;
	_nCorrection = 4;
	_nPanSpeed = 1;
	_pWieldBmp = nullptr;
}

CBagPanWindow::CBagPanWindow() : CBagStorageDevWnd() {
	_xViewPortPos = CBofPoint(0, 20);
	_xMovementRect.setRectEmpty();

	_nCorrection = CBagMasterWin::getCorrection();
	_nPanSpeed = CBagMasterWin::getPanSpeed();
	_pSlideBitmap = nullptr;
	_pPalette = nullptr;
	_bPaintToBackdrop = true;
	_bStretchToScreen = false;
	_bDraggingObject = false;
	_bDraggingStart = CBofPoint(0, 0);
	_pViewPortBitmap = nullptr;

	// Objects to be painted to the window
	_pFGObjectList = new CBofList<CBagObject *>();

	// Declare this to be type game win, because it has a pda
	_xSDevType = SDEV_GAMEWIN;

	// Make sure the prefilter gets called.
	setPreFilterPan(true);

	CBagStorageDev::loadObjects();
}

CBofRect CBagPanWindow::unSetSlideBitmap() {
	CBofRect        viewRect;

	setLActiveObject(nullptr);

	if (_pSlideBitmap) {
		viewRect = _pSlideBitmap->getCurrView();
		delete _pSlideBitmap;
		_pSlideBitmap = nullptr;
	}
	if (_pViewPortBitmap) {
		delete _pViewPortBitmap;
		_pViewPortBitmap = nullptr;
	}

	CBofSprite::closeLibrary();

	// There is no palette right now
	CBofApp::getApp()->setPalette(nullptr);

	return viewRect;
}

CBofPalette *CBagPanWindow::setSlideBitmap(const CBofString &xSlideBmp, const CBofRect &xSlideRect) {
	if (!xSlideBmp.isEmpty()) {
		CBofRect viewRect = unSetSlideBitmap();
		CBofRect cRect = getWindowRect();

		if ((cRect.width() <= 0) || (cRect.width() > DEF_WIDTH))
			cRect.setRect(cRect.left, cRect.top, cRect.left + DEF_WIDTH, cRect.bottom);

		cRect.bottom = cRect.top + 3 * cRect.width() / 4 - 1;

		if ((xSlideRect.width() <= 0) || (xSlideRect.width() > DEF_WIDTH))
			viewRect.setRect(xSlideRect.left, xSlideRect.top, xSlideRect.left + DEF_WIDTH, DEF_HEIGHT);
		else
			viewRect = xSlideRect;

		// Open Pan bitmap
		_pSlideBitmap = new CBagPanBitmap(xSlideBmp, nullptr, viewRect);

		// Make sure the file was found
		if (!_pSlideBitmap->isValid()) {
			_pPalette = nullptr;
			reportError(ERR_FOPEN, "Unable to open file %s", xSlideBmp.getBuffer());
		} else {
			// Set the bagel crap
			_pPalette = _pSlideBitmap->getPalette();
			CBofApp::getApp()->setPalette(_pPalette);
			CBagel::getBagApp()->getMasterWnd()->selectPalette(_pPalette);
			CBofApp::getApp()->getMainWindow()->selectPalette(_pPalette);
			selectPalette(_pPalette);

			CBofSprite::openLibrary(_pPalette);

			_pSlideBitmap->setCorrWidth(_nCorrection);

			CBofBitmap *pBackDropBitmap = new CBofBitmap(DEF_WIDTH + 1, DEF_HEIGHT + 1, _pPalette);
			if (pBackDropBitmap->height() <= 0 || pBackDropBitmap->width() <= 0) {
				reportError(ERR_FOPEN, "Error opening bitmap");
			}
			_pViewPortBitmap = new CBofBitmap(DEF_WIDTH + 1, _pSlideBitmap->height() + 1, _pPalette);
			if (!_pViewPortBitmap->height() || !_pViewPortBitmap->width()) {
				reportError(ERR_FOPEN, "Error opening bitmap");
			}
			setBackdrop(pBackDropBitmap);

			move(cRect.left, cRect.top);

		}
	}

	return _pPalette;
}

CBagPanWindow::~CBagPanWindow() {
	CBagStorageDev::releaseObjects();   // Delete all master sprite objects

	if (_pSlideBitmap) {
		delete _pSlideBitmap;
		_pSlideBitmap = nullptr;
	}
	if (_pViewPortBitmap) {
		delete _pViewPortBitmap;
		_pViewPortBitmap = nullptr;
	}

	CBofSprite::closeLibrary();     // Free the off screen bitmap

	delete _pFGObjectList;
}

void CBagPanWindow::onClose() {
	CBagStorageDevWnd::onClose();

	destroyWindow();                // Destruct the main window
}

ErrorCode CBagPanWindow::runModal(CBagObject *pObj) {
	if (_pSlideBitmap != nullptr) {

		_pSlideBitmap->updateView();
		CBagStorageDevWnd::runModal(pObj);

		// Make sure we update the entire screen after this,
		// we're not really sure what got trashed, also,
		// the cursor background will contain bogus information,
		// so make sure that gets overwritten
		setPreFilterPan(true);
	}

	return _errCode;
}

ErrorCode CBagPanWindow::onRender(CBofBitmap *pBmp, CBofRect *pRect) {
	assert(isValidObject(this));
	assert(pBmp != nullptr);

	if (_pSlideBitmap != nullptr) {
		_pSlideBitmap->updateView();

		CBofRect dstRect(CBofPoint(0, 0), _pSlideBitmap->getViewSize());
		CBofRect clientArea = getClientRect();

		if (_pSlideBitmap->isPan()) {

			// Paint uncorrected view to backdrop
			CBofRect currViewRect = _pSlideBitmap->getCurrView();  // Hold current view
			CBofRect offsetRect;                // Size of viewportbmp
			CBofRect srcRect = dstRect;         // Src will be same as dst

			// If we've been given the go ahead to call prefilter, then
			// do so but don't do it again unless asked.
			if (preFilterPan()) {
				preFilter(pBmp, pRect, _pFGObjectList);
				setPreFilterPan(false);
			}

			_pSlideBitmap->paintUncorrected(_pViewPortBitmap, offsetRect);  // Paint and return size
			srcRect.offsetRect(0, currViewRect.top - offsetRect.top);            //   less the offset from full

			// Paint the objects to the backdrop
			paintObjects(getObjectList(), _pViewPortBitmap, offsetRect, nullptr);

			dstRect.offsetRect(_xViewPortPos);

			// No correction ?
			if (_nCorrection == 0) {
				_pViewPortBitmap->paint(pBmp, &dstRect, &srcRect);

			} else {
				// Warp the backdrop to itself
				_pSlideBitmap->paintWarped(pBmp, dstRect, srcRect, 0, _pViewPortBitmap, offsetRect);
			}

			if (isFiltered()) {
				uint16 nFilterId = getFilterId();
				(*_pBitmapFilter)(nFilterId, pBmp, pRect);
			}


			// Now paint it
			paintObjects(_pFGObjectList, pBmp, clientArea, nullptr, false);

		} else {
			// Close-Ups
			_pSlideBitmap->setCorrWidth(0, false);

			// If we've been given the go ahead to call prefilter, then
			// do so but don't do it again unless asked.
			if (preFilterPan()) {
				preFilter(pBmp, pRect, _pFGObjectList);
				setPreFilterPan(false);

				// Only paint the slide the first time around, if we paint
				// it in subsequent calls, then we will trash our PDA in this closeup,
				// and that will be a bad thing.
				dstRect.offsetRect(_xViewPortPos);
				((CBofBitmap *)_pSlideBitmap)->paint(pBmp, _xViewPortPos.x, _xViewPortPos.y);
			}

			clientArea.offsetRect(-_xViewPortPos.x, -_xViewPortPos.y);
			paintObjects(getObjectList(), pBmp, clientArea, nullptr);

			if (isFiltered()) {
				uint16 nFilterId = getFilterId();
				bool bFiltered = (*_pBitmapFilter)(nFilterId, pBmp, pRect);

				if (bFiltered) {
					setPreFilterPan(true);
				}
			}

			// This must be changed so that the foreground objects are actually
			// a list of child windows.  The on paint message is then called with
			// the background bitmap
			paintObjects(_pFGObjectList, pBmp, clientArea, nullptr);

			_pSlideBitmap->setCorrWidth(_nCorrection, false);
		}
	}

	return _errCode;
}

ErrorCode CBagPanWindow::paintObjects(CBofList<CBagObject *> *list, CBofBitmap *pBmp, CBofRect &viewRect, CBofList<CBofRect> *pUpdateArea, bool tempVar) {
	ErrorCode errorCode = ERR_NONE;

	// can't use a null pointer
	assert(pBmp != nullptr);

	int nCount = list->getCount();
	if (nCount != 0) {
		int nW = _pSlideBitmap->width();
		CBofPoint xCursorLocation = devPtToViewPort(*_xCursorLocation);
		int nMouseOverObj = -1;

		for (int i = 0; i < nCount; ++i) {
			CBagObject *pObj = list->getNodeItem(i);
			CBofRect xBmpRect = pObj->getRect();

			// If it is a panorama we have to check for exceeded bounds
			if (!pObj->isForeGround() && (nW > 1024) && (viewRect.right > nW) && (xBmpRect.left < (nW / 2)))
				xBmpRect.offsetRect(nW, 0);

			CBofRect xIntrRect;
			if (xIntrRect.intersectRect(&viewRect, &xBmpRect) || pObj->isForeGround() || pObj->isModal()) {
				CBofPoint pt = xBmpRect.topLeft();
				if (!pObj->isForeGround()) {
					pt = xBmpRect.topLeft() - viewRect.topLeft();
				}

				xIntrRect = xBmpRect;
				xIntrRect.offsetRect(-xBmpRect.left, -xBmpRect.top);

				// Only update dirty objects...
				if (pObj->isDirty() || pObj->isAlwaysUpdate()) {
					if (!(pObj->getType() == TEXT_OBJ && ((CBagTextObject *)pObj)->isCaption() && pObj->isImmediateRun())) {
						pObj->update(pBmp, pt, &xIntrRect);

						// If we're in a closeup and we have a character animation
						// playing, then make sure we redraw
						CBagCharacterObject *pCharObj = (CBagCharacterObject *)pObj;

						// Only in closeups
						if (!_pSlideBitmap->isPan() && pObj->getType() == CHAR_OBJ) {
							// Handle non-modal movies also...
							if (pCharObj->getNumberOfLoops() == 1) {
								setPreFilterPan(true);
							} else if (pObj->isAttached() &&                       // don't care if it's not running...
							           (pCharObj->isStationary() == false) &&
							           (pCharObj->getNumberOfLoops() != 0) &&     // Plays multiple or infinite (fly == -1)
							           ((pObj->getRect().width() != 480) &&
							            (pObj->getRect().height() != 360))) {

								// Redraw everything inside of the closeup... but not the PDA...
								// only want to redraw the closeup, not everything else.
								//
								// if our prefilter pan is already dirty, then don't mess with dirty
								// object list.

								bool b = preFilterPan();
								setPreFilterPan(true);

								if (!b) {
									setDirtyAllObjects(false);
								}
							}
						}
					}
				}

				if (pUpdateArea) {
					xIntrRect.offsetRect(pt);
					pUpdateArea->addToTail(xIntrRect);
				}

				if (pObj->getRect().ptInRect(xCursorLocation)) {
					pObj->onMouseOver(0, xCursorLocation, this);
					nMouseOverObj = i;
				}
			}  // if on screen

		}  // While a valid object

		if ((nMouseOverObj < 0) && tempVar)
			noObjectsUnderMouse();
	}

	return errorCode;
}

// Delete the foreground objects
void CBagPanWindow::deleteFGObjects() {
	_pFGObjectList->removeAll();
}

ErrorCode CBagPanWindow::insertFGObjects(CBagObject *pBmp) {
	// Can't use a nullptr pointer
	assert(pBmp != nullptr);

	_pFGObjectList->addToTail(pBmp);

	return _errCode;
}

CBagObject *CBagPanWindow::getFGObjects(const CBofString &sObjName) {
	int nListLen = _pFGObjectList->getCount();

	for (int i = 0; i < nListLen; ++i) {
		CBagObject *pObj = (*_pFGObjectList)[i];
		if ((pObj->getRefName().find(sObjName)) != -1)
			return pObj;
	}

	return nullptr;
}


void CBagPanWindow::activateView() {
	if (_pSlideBitmap) {
		_pSlideBitmap->activateScrolling();
		_pSlideBitmap->setRotateRate(CBofPoint(32, 16));
	}
}

void CBagPanWindow::deActivateView() {
	if (_pSlideBitmap) {
		_pSlideBitmap->deActivateScrolling();
	}
}

void CBagPanWindow::enable() {
	activateView();
}

void CBagPanWindow::disable() {
	flushAllMessages();
	deActivateView();
}

ErrorCode CBagPanWindow::onCursorUpdate(int nCurrObj) {
	assert(isValidObject(this));

	if (nCurrObj >= 0) {
		CBagObject *pObj = getObjectByPos(nCurrObj);
		if (pObj != nullptr)
			CBagMasterWin::setActiveCursor(pObj->getOverCursor());

	} else if (CBagWield::getWieldCursor() >= 0) {
		CBagMasterWin::setActiveCursor(CBagWield::getWieldCursor());
	}

	return _errCode;
}

void CBagPanWindow::onKeyHit(uint32 lKey, uint32 lRepCount) {
	assert(isValidObject(this));

	if (_pSlideBitmap != nullptr) {
		switch (lKey) {
		case BKEY_MINUS:
			if (_nCorrection == 0) {
				_nCorrection = 64;
			}
			_nCorrection >>= 1;
			_pSlideBitmap->setCorrWidth(_nCorrection);
			CBagMasterWin::setCorrection(_nCorrection);
			break;

		case BKEY_PLUS:
			if (_nCorrection == 0) {
				_nCorrection = 1;

			} else if ((_nCorrection <<= 1) > 32) {
				_nCorrection = 0;
			}
			_pSlideBitmap->setCorrWidth(_nCorrection);
			CBagMasterWin::setCorrection(_nCorrection);
			break;

		case BKEY_LEFT:
			_pSlideBitmap->setRotateRate(CBofPoint(16, 8));
			_pSlideBitmap->setDirection(CBagPanBitmap::kDirRIGHT);
			break;

		case BKEY_RIGHT:
			_pSlideBitmap->setRotateRate(CBofPoint(16, 8));
			_pSlideBitmap->setDirection(CBagPanBitmap::kDirLEFT);
			break;

		case BKEY_UP:
			_pSlideBitmap->setRotateRate(CBofPoint(16, 8));
			_pSlideBitmap->setDirection(CBagPanBitmap::kDirDOWN);
			break;

		case BKEY_DOWN:
			_pSlideBitmap->setRotateRate(CBofPoint(16, 8));
			_pSlideBitmap->setDirection(CBagPanBitmap::kDirUP);
			break;
		}
	}

	CBagStorageDevWnd::onKeyHit(lKey, lRepCount);
}

void CBagPanWindow::onMouseMove(uint32 nFlags, CBofPoint *p, void *) {
	assert(isValidObject(this));
	assert(p != nullptr);

	CBofPoint xPoint = *p;

	CBagStorageDevWnd::onMouseMove(nFlags, &xPoint);

	if (!isCloseup()) {
		if (_pSlideBitmap != nullptr) {
			if (_cRightRect.ptInRect(xPoint)) {
				_pSlideBitmap->setRotateRate(CBofPoint(((xPoint.x - _xMovementRect.right) * (_nPanSpeed + 1)) / 2, 0));
				_pSlideBitmap->setDirection(CBagPanBitmap::kDirLEFT);
				CBagMasterWin::setActiveCursor(BOF_RT_CURSOR);

			} else if (_cLeftRect.ptInRect(xPoint)) {
				_pSlideBitmap->setRotateRate(CBofPoint(((_xMovementRect.left - xPoint.x) * (_nPanSpeed + 1)) / 2, 0));
				_pSlideBitmap->setDirection(CBagPanBitmap::kDirRIGHT);
				CBagMasterWin::setActiveCursor(BOF_LT_CURSOR);

			} else if (_cBottomRect.ptInRect(xPoint)) {
				_pSlideBitmap->setRotateRate(CBofPoint(0, (xPoint.y - _xMovementRect.bottom)));
				_pSlideBitmap->setDirection(CBagPanBitmap::kDirUP);
				CBagMasterWin::setActiveCursor(BOF_DN_CURSOR);

			} else if (_cTopRect.ptInRect(xPoint)) {
				_pSlideBitmap->setRotateRate(CBofPoint(0, (_xMovementRect.top - xPoint.y)));
				_pSlideBitmap->setDirection(CBagPanBitmap::kDirDOWN);
				CBagMasterWin::setActiveCursor(BOF_UP_CURSOR);

			} else {
				_pSlideBitmap->setDirection(CBagPanBitmap::kDirNONE);
			}

			if (getLActiveObject() && getLActivity()) {
				_pSlideBitmap->setDirection((CBagPanBitmap::Direction)(_pSlideBitmap->getDirection() | CBagPanBitmap::kDirVIEW));
			}
		}
	}


	// Change cursor based on the Foreground object list
	// Run through background object list and find if the cursor is over an object
	CBofList<CBagObject *> *pList = _pFGObjectList;
	if (pList != nullptr) {
		CBagObject *pOverObj = nullptr;
		int nCount = pList->getCount();
		for (int i = 0; i < nCount; ++i) {
			CBagObject *pObj = pList->getNodeItem(i);

			if (pObj->isInside(xPoint)) {
				pOverObj = pObj;
			}
		}

		// If we just happen to be over the pda then let our
		// PDA code decide... oh, and we're wielding...
		if (pOverObj != nullptr) {
			int nCursorID = -1;

			// the logz case is handled by onmousemove in zoompda
			if (pOverObj->getRefName().find("BPDA_WLD") != -1) {
				CBagPDA *pPda = (CBagPDA *)g_SDevManager->getStorageDevice(pOverObj->getRefName());
				if (pPda != nullptr) {
					CBofRect cRect = pOverObj->getRect();
					nCursorID = pPda->getProperCursor(xPoint, cRect);
				}
			}

			// Still no luck, if we're wielding, use that cursor.
			if (nCursorID == -1) {
				nCursorID = pOverObj->getOverCursor();
				if (CBagWield::getWieldCursor() >= 0 && ((pOverObj->getRefName().find("BWIELD_WLD") != -1) || (pOverObj->getRefName().find("THUD_WLD") != -1))) {
					nCursorID = CBagWield::getWieldCursor();
				}
			}

			CBagMasterWin::setActiveCursor(nCursorID);
		}
	}

	if (_bDraggingObject) {
		CBagObject *pObj = _pFGObjectList->getNodeItem(_pFGObjectList->getCount() - 1);
		pObj->setPosition(xPoint);
	}

	*_xCursorLocation = xPoint;
}

void CBagPanWindow::onLButtonDown(uint32 nFlags, CBofPoint *xPoint, void *) {
	int nCount = _pFGObjectList->getCount();
	if (nCount != 0) {
		for (int i = 0; i < nCount; ++i) {
			CBagObject *pObj = _pFGObjectList->getNodeItem(i);

			if (pObj->isInside(*xPoint)) {
				pObj->onLButtonDown(nFlags, xPoint, this);
				return;
			}
		}
	}
	CBagStorageDevWnd::onLButtonDown(nFlags, xPoint);

	MouseActivity  nMA = getLActivity();
	CBagObject *pActObj = getLActiveObject();

	if (nMA && pActObj && pActObj->isMovable()) {
		_bDraggingObject = true;
		_bDraggingStart = pActObj->getPosition();
		_pFGObjectList->addToTail(pActObj);
	}
}

void CBagPanWindow::onLButtonUp(uint32 nFlags, CBofPoint *xPoint, void *) {
	MouseActivity  nMA = getLActivity();
	CBagObject *pActObj = getLActiveObject();

	if ((pActObj != nullptr) && (nMA == kMouseDRAGGING) &&
	        (pActObj->getType() == BUTTON_OBJ) &&
	        (((CBagButtonObject *)pActObj)->getButtonType() == CBagButtonObject::BTN_SLIDER)) {
		pActObj->onLButtonUp(nFlags, xPoint, this);

		// We are not dragging a slider anymore
		setLActivity(kMouseNONE);
		setLActiveObject(nullptr);

	} else {
		bool bMoved = false;

		if (_bDraggingObject) {
			_bDraggingObject = false;
			pActObj = _pFGObjectList->removeTail();

			int nCount = _pFGObjectList->getCount();
			if (nCount != 0) {
				for (int i = 0; i < nCount; ++i) {
					CBagObject *pObj = _pFGObjectList->getNodeItem(i);
					CBofRect xBmpRect = pObj->getRect();
					if (xBmpRect.ptInRect(*xPoint)) {
						pObj->onObjInteraction(pActObj, this);
					}
				}
			}

			if (pActObj->getPosition() != _bDraggingStart) {
				pActObj->setPosition(_bDraggingStart);
				bMoved = true;
			}
		}

		if (!bMoved) {
			// Parse backwards to get topmost obj 1st
			int nCount = _pFGObjectList->getCount();
			if (nCount != 0) {
				for (int i = nCount - 1; i >= 0; --i) {
					CBagObject *pObj = _pFGObjectList->getNodeItem(i);
					if (pObj->isInside(*xPoint)) {
						pObj->onLButtonUp(nFlags, xPoint, this);
						return;
					}
				}
			}
		}

		CBagStorageDevWnd::onLButtonUp(nFlags, xPoint);
	}
}

void CBagPanWindow::onSize(uint32 nType, int cx, int cy) {
	// nRange defines the distance (in pixels) from a viewport edge
	// (toward the center of the viewport) that the viewport boundary
	// rectangles begin.  The boundary rectangles surround the edges
	// of the viewport.  When the mouse cursor enters a boundary rectangle,
	// it's time to pan the view.  Larger positive values of nRange will
	// cause the boundary rectangle to extend farther from the viewport edge.
	// A value of zero will cause the boundary rectangles to be at the edge
	// of the viewport.  If nRange has a negative value, the boundary
	// rectangle will start outside the viewport rectangle.
	//
	const int nRange = 16;
	CBofRect xMaxPanBmpRect;

	if (_pSlideBitmap)
		xMaxPanBmpRect = _pSlideBitmap->getMaxView();
	else
		xMaxPanBmpRect.setRect(0, 0, DEF_WIDTH, DEF_HEIGHT);

	CBofPoint vp;
	CBofSize  vs = CBofSize(cx, cy);

	// Check if the current view is greater than the max view
	if (vs.cx > xMaxPanBmpRect.width())
		vs.cx = xMaxPanBmpRect.width();

	if (_pSlideBitmap && _pSlideBitmap->width() > 480)
		vs.cy = 3 * vs.cx / 4;

	if (vs.cy > xMaxPanBmpRect.height())
		vs.cy = xMaxPanBmpRect.height();

	setViewPortSize(vs);

	vp.x = (cx - vs.cx) / 2;
	vp.y = 10;

	setViewPortPos(vp);

	vp.x += nRange;
	vp.y += nRange;
	vs.cx -= (2 * nRange); // Subtract 2*nRange from vs because there's
	vs.cy -= (2 * nRange); // a boundary rectangle on the left and right
	// *and* the top and bottom.

	CBofRect cRect(vp, vs);

	_cTopRect.setRect(cRect.left, 0, cRect.right, cRect.top);
	_cBottomRect.setRect(cRect.left, cRect.bottom, cRect.right, 480 - 1);
	_cLeftRect.setRect(0, cRect.top, cRect.left, cRect.bottom);
	_cRightRect.setRect(cRect.right, cRect.top, 640 - 1, cRect.bottom);

	setMovementRect(cRect);
}


void CBagPanWindow::OnWindowPosChanging(WindowPos *lpwndpos) {
	const int MENUNBORDER = 50;
	CBofRect xSlideBmpRect;

	xSlideBmpRect.setRect(0, 0, DEF_WIDTH, DEF_HEIGHT);

	if (lpwndpos->cx > xSlideBmpRect.width())
		lpwndpos->cx = xSlideBmpRect.width();
	if (lpwndpos->cy > xSlideBmpRect.height() + MENUNBORDER)
		lpwndpos->cy = xSlideBmpRect.height() + MENUNBORDER;
}

void CBagPanWindow::flushInputEvents() {
	g_system->getEventManager()->purgeKeyboardEvents();
	g_system->getEventManager()->purgeMouseEvents();
}

#define PAN_WIDTH   2048
#define PAN_HEIGHT   512

#define HALF_PAN_WIDTH   (PAN_WIDTH/2)

uint32 CBagPanWindow::rotateTo(CBofPoint &xPoint, int nRate) {
	assert(nRate > 0);

	if (_pSlideBitmap && (xPoint.x != -1) && (xPoint.y != -1)) {
		CBofRect r = _pSlideBitmap->getCurrView();

		for (;;) {
			CBofPoint xCurrPos = r.topLeft();
			int x = (xPoint.x - xCurrPos.x);
			int y = (xPoint.y - xCurrPos.y);

			if (abs(x) > HALF_PAN_WIDTH) {
				if (x > 0) {
					x -= PAN_WIDTH;
				} else {
					x += PAN_WIDTH;
				}
			}

			// Are we there yet?
			if (x == 0 && y == 0)
				break;

			int nRateX = MIN(x, nRate);
			if (x < 0)
				nRateX = MAX(x, -nRate);

			int nRateY = MIN(y, nRate);
			if (y < 0)
				nRateY = MAX(y, -nRate);

			r.offsetRect(nRateX, nRateY);

			_pSlideBitmap->setCurrView(r);
			paintScreen();
		}
	}

	return 0;
}


uint32 CBagPanWindow::benchmark() {
	deActivateView();
	_pSlideBitmap->activateScrolling();

	_pSlideBitmap->setRotateRate(CBofPoint(8, 0));
	_pSlideBitmap->setDirection(CBagPanBitmap::kDirLEFT);

	timerStart();
	for (int i = 0; i < 50; i++)
		paintScreen();
	uint32 dTime = timerStop();

	activateView();
	return dTime;
}

bool CBagPanWindow::deactivatePDA() {
	// If we have a PDA and the pda is active
	if (_pPDABmp && _pPDABmp->isActivated()) {
		// deactivate it
		_pPDABmp->deactivate();

		// PDA successfully deactivated
		return true;
	}

	// PDA already deactivated
	return false;
}

bool CBagPanWindow::activatePDA() {
	// If we have a BMP and the pda is not active
	if (_pPDABmp && (!_pPDABmp->isActivated() || _pPDABmp->isActivating())) {
		_pPDABmp->activate();  // activate it

		// PDA successfully activated
		return true;
	}

	// PDA already activated
	return false;

}

const CBofPoint CBagPanWindow::devPtToViewPort(const CBofPoint &xPoint) {
	CBofRect r = _pSlideBitmap->getCurrView();

	CBofPoint p;
	p.x = xPoint.x + r.left - _xViewPortPos.x;
	p.y = xPoint.y + r.top - _xViewPortPos.y;

	if (_pSlideBitmap->isPan()) {
		if (p.x >= _pSlideBitmap->width())
			p.x -= _pSlideBitmap->width();

		// If this view is being painted with a correction, we need
		// to correct the point to the warped view space.
		if (_nCorrection != 0) {
			p = _pSlideBitmap->warpedPoint(p);
		}
	}

	return p;
}


const CBofPoint CBagPanWindow::viewPortToDevPt(const CBofPoint &xPoint) {
	CBofRect r = _pSlideBitmap->getCurrView();
	CBofPoint p(xPoint.x - r.left, xPoint.y - r.top);

	if (p.x < 0)
		p.x += _pSlideBitmap->width();

	return p;
}

ErrorCode CBagPanWindow::attachActiveObjects() {
	CBagStorageDev::attachActiveObjects();

	if (_pPDABmp != nullptr) {
		_pPDABmp->attachActiveObjects();
	}

	return _errCode;
}

void CBagPanWindow::setViewPortSize(const CBofSize &xViewSize) {
	if (_pSlideBitmap)
		_pSlideBitmap->setViewSize(xViewSize);
}

const CBofRect CBagPanWindow::getViewPort() {
	CBofRect r;
	if (_pSlideBitmap)
		r = _pSlideBitmap->getCurrView();
	return r;
}

const CBofSize CBagPanWindow::getViewPortSize() {
	CBofRect r;
	if (_pSlideBitmap)
		r = _pSlideBitmap->getCurrView();
	return r.size();
}

void CBagPanWindow::waitForPDA() {
	// Make sure we have a non-null pda
	while (_pPDABmp && _pPDABmp->isActivating()) {
		setPreFilterPan(true);
		_pPDABmp->setDirty(true);
		paintScreen();
	}
}

} // namespace SpaceBar
} // namespace Bagel
