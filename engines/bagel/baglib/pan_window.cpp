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

#include "bagel/baglib/pan_window.h"
#include "bagel/baglib/res.h"
#include "bagel/baglib/master_win.h"
#include "bagel/baglib/bagel.h"
#include "bagel/baglib/wield.h"
#include "bagel/boflib/gfx/sprite.h"
#include "bagel/baglib/button_object.h"
#include "bagel/baglib/character_object.h"
#include "bagel/baglib/text_object.h"
#include "bagel/boflib/std_keys.h"

namespace Bagel {

CBagPDA *CBagPanWindow::_pPDABmp;      // Pointer to the PDA object
int CBagPanWindow::m_nCorrection;
int CBagPanWindow::m_nPanSpeed;
CBagWield *CBagPanWindow::_pWieldBmp;  // Pointer to the WEILD object

void CBagPanWindow::initialize() {
	_pPDABmp = nullptr;
	m_nCorrection = 4;
	m_nPanSpeed = 1;
	_pWieldBmp = nullptr;
}

CBagPanWindow::CBagPanWindow() : CBagStorageDevWnd() {
	CBofRect tmpRect;

	m_xVeiwPortPos = CBofPoint(0, 20);
	m_xMovementRect.SetRectEmpty();

	m_nCorrection = CBagMasterWin::getCorrection();
	m_nPanSpeed = CBagMasterWin::getPanSpeed();
	m_pSlideBitmap = nullptr;
	_pPalette = nullptr;
	m_bPaintToBackdrop = true;
	m_bStretchToScreen = false;
	m_bDraggingObject = false;
	m_bDraggingStart = CBofPoint(0, 0);
	m_pVeiwPortBitmap = nullptr;

	// Objects to be painted to the window
	m_pFGObjectList = new CBofList<CBagObject *>();

	// Declare this to be type game win, because it has a pda
	_xSDevType = SDEV_GAMEWIN;

	// Make sure the prefilter gets called.
	SetPreFilterPan(true);

	LoadObjects();
}

CBofRect CBagPanWindow::UnSetSlidebitmap() {
	CBofRect        viewRect;

	SetLActiveObject(nullptr);

	if (m_pSlideBitmap) {
		viewRect = m_pSlideBitmap->GetCurrView();
		delete m_pSlideBitmap;
		m_pSlideBitmap = nullptr;
	}
	if (m_pVeiwPortBitmap) {
		delete m_pVeiwPortBitmap;
		m_pVeiwPortBitmap = nullptr;
	}

	CBofSprite::closeLibrary();

	// There is no palette right now
	CBofApp::getApp()->setPalette(nullptr);

	return viewRect;
}

CBofPalette *CBagPanWindow::SetSlidebitmap(const CBofString &xSlideBmp, const CBofRect &xSlideRect) {
	if (!xSlideBmp.isEmpty()) {
		CBofRect viewRect = UnSetSlidebitmap();
		CBofRect cRect = getWindowRect();

		if ((cRect.width() <= 0) || (cRect.width() > DEF_WIDTH))
			cRect.SetRect(cRect.left, cRect.top, cRect.left + DEF_WIDTH, cRect.bottom);

		cRect.bottom = cRect.top + 3 * cRect.width() / 4 - 1;

		if ((xSlideRect.width() <= 0) || (xSlideRect.width() > DEF_WIDTH))
			viewRect.SetRect(xSlideRect.left, xSlideRect.top, xSlideRect.left + DEF_WIDTH, DEF_HEIGHT);
		else
			viewRect = xSlideRect;

		// Open Pan bitmap
		m_pSlideBitmap = new CBagPanBitmap(xSlideBmp, nullptr, viewRect);

		// Make sure the file was found
		if (m_pSlideBitmap == nullptr || !m_pSlideBitmap->IsValid()) {
			_pPalette = nullptr;
			reportError(ERR_FOPEN);
		} else {
			// Set the bagel crap
			_pPalette = m_pSlideBitmap->getPalette();
			CBofApp::getApp()->setPalette(_pPalette);
			CBagel::getBagApp()->getMasterWnd()->selectPalette(_pPalette);
			CBofApp::getApp()->getMainWindow()->selectPalette(_pPalette);
			selectPalette(_pPalette);

			CBofSprite::openLibrary(_pPalette);

			m_pSlideBitmap->setCorrWidth(m_nCorrection);

			CBofBitmap *pBackDropBitmap = new CBofBitmap(DEF_WIDTH + 1, DEF_HEIGHT + 1, _pPalette);
			if (!pBackDropBitmap || pBackDropBitmap->height() <= 0 || pBackDropBitmap->width() <= 0) {
				reportError(ERR_FOPEN, "Error opening bitmap");
			}
			m_pVeiwPortBitmap = new CBofBitmap(DEF_WIDTH + 1, m_pSlideBitmap->height() + 1, _pPalette);
			if (!m_pVeiwPortBitmap || !m_pVeiwPortBitmap->height() || !m_pVeiwPortBitmap->width()) {
				reportError(ERR_FOPEN);
			}
			setBackdrop(pBackDropBitmap);

			move(cRect.left, cRect.top);

		}
	}

	return _pPalette;
}

CBagPanWindow::~CBagPanWindow() {
	ReleaseObjects();   // Delete all master sprite objects

	if (m_pSlideBitmap) {
		delete m_pSlideBitmap;
		m_pSlideBitmap = nullptr;
	}
	if (m_pVeiwPortBitmap) {
		delete m_pVeiwPortBitmap;
		m_pVeiwPortBitmap = nullptr;
	}

	CBofSprite::closeLibrary();		// Free the off screen bitmap

	delete m_pFGObjectList;
}

void CBagPanWindow::onClose() {
	CBagStorageDevWnd::onClose();

	destroyWindow();				// Destruct the main window
}

ErrorCode CBagPanWindow::RunModal(CBagObject *pObj) {
	if (m_pSlideBitmap != nullptr) {

		m_pSlideBitmap->UpdateView();
		CBagStorageDevWnd::RunModal(pObj);

		// Make sure we update the entire screen after this,
		// we're not really sure what got trashed, also,
		// the cursor background will contain bogus information,
		// so make sure that gets overwritten
		SetPreFilterPan(true);
	}

	return _errCode;
}

ErrorCode CBagPanWindow::onRender(CBofBitmap *pBmp, CBofRect *pRect) {
	Assert(IsValidObject(this));
	Assert(pBmp != nullptr);

	if (m_pSlideBitmap != nullptr) {
		m_pSlideBitmap->UpdateView();

		CBofRect dstRect(CBofPoint(0, 0), m_pSlideBitmap->GetViewSize());
		CBofRect clientArea = getClientRect();

		if (m_pSlideBitmap->IsPan()) {

			// Paint uncorrected view to backdrop
			CBofRect currViewRect = m_pSlideBitmap->GetCurrView();  // Hold current view
			CBofRect offsetRect;                // Size of viewportbmp
			CBofRect srcRect = dstRect;         // Src will be same as dst

			// If we've been given the go ahead to call prefilter, then
			// do so but don't do it again unless asked.
			if (PreFilterPan()) {
				PreFilter(pBmp, pRect, m_pFGObjectList);
				SetPreFilterPan(false);
			}

			m_pSlideBitmap->PaintUncorrected(m_pVeiwPortBitmap, offsetRect);  // Paint and return size
			srcRect.OffsetRect(0, currViewRect.top - offsetRect.top);            //   less the offset from full

			// Paint the objects to the backdrop
			PaintObjects(GetObjectList(), m_pVeiwPortBitmap, offsetRect, nullptr);

			dstRect.OffsetRect(m_xVeiwPortPos);

			// No correction ?
			if (m_nCorrection == 0) {
				m_pVeiwPortBitmap->paint(pBmp, &dstRect, &srcRect);

			} else {
				// Warp the backdrop to itself
				m_pSlideBitmap->PaintWarped(pBmp, dstRect, srcRect, 0, m_pVeiwPortBitmap, offsetRect);
			}

			if (IsFiltered()) {
				uint16 nFilterId = GetFilterId();
				(*m_pBitmapFilter)(nFilterId, pBmp, pRect);
			}


			// Now paint it
			PaintObjects(m_pFGObjectList, pBmp, clientArea, nullptr, false);

		} else {
			// Close-Ups
			m_pSlideBitmap->setCorrWidth(0, false);

			// If we've been given the go ahead to call prefilter, then
			// do so but don't do it again unless asked.
			if (PreFilterPan()) {
				PreFilter(pBmp, pRect, m_pFGObjectList);
				SetPreFilterPan(false);

				// Only paint the slide the first time around, if we paint
				// it in subsequent calls, then we will trash our PDA in this closeup,
				// and that will be a bad thing.
				dstRect.OffsetRect(m_xVeiwPortPos);
				((CBofBitmap *)m_pSlideBitmap)->paint(pBmp, m_xVeiwPortPos.x, m_xVeiwPortPos.y);
			}

			clientArea.OffsetRect(-m_xVeiwPortPos.x, -m_xVeiwPortPos.y);
			PaintObjects(GetObjectList(), pBmp, clientArea, nullptr);

			if (IsFiltered()) {
				uint16 nFilterId = GetFilterId();
				bool bFiltered = false;
				bFiltered = (*m_pBitmapFilter)(nFilterId, pBmp, pRect);

				if (bFiltered) {
					SetPreFilterPan(true);
				}
			}

			// This must be changed so that the forground objects are actually
			// a list of child windows.  The on paint message is then called with
			// the background bitmap
			PaintObjects(m_pFGObjectList, pBmp, clientArea, nullptr);

			m_pSlideBitmap->setCorrWidth(m_nCorrection, false);
		}
	}

	return _errCode;
}

ErrorCode CBagPanWindow::PaintObjects(CBofList<CBagObject *> *list, CBofBitmap *pBmp, CBofRect &viewRect, CBofList<CBofRect> *pUpdateArea, bool tempVar) {
	ErrorCode errCode = ERR_NONE;

	// can't use a null pointer
	Assert(pBmp != nullptr);

	int nCount = list->getCount();
	if (nCount != 0) {
		int nW = m_pSlideBitmap->width();
		CBofPoint xCursorLocation = DevPtToViewPort(*m_xCursorLocation);
		int nMouseOverObj = -1;

		for (int i = 0; i < nCount; ++i) {
			CBagObject *pObj = list->getNodeItem(i);
			CBofRect xBmpRect = pObj->getRect();

			// If it is a panorama we have to check for exceeded bounds
			if (!pObj->IsForeGround() && (nW > 1024) && (viewRect.right > nW) && (xBmpRect.left < (nW / 2)))
				xBmpRect.OffsetRect(nW, 0);

			CBofRect xIntrRect;
			if (xIntrRect.IntersectRect(&viewRect, &xBmpRect) || pObj->IsForeGround() || pObj->IsModal()) {
				CBofPoint pt = xBmpRect.TopLeft();
				if (!pObj->IsForeGround()) {
					pt = xBmpRect.TopLeft() - viewRect.TopLeft();
				}

				xIntrRect = xBmpRect;
				xIntrRect.OffsetRect(-xBmpRect.left, -xBmpRect.top);

				// Only update dirty objects...
				if (pObj->IsDirty() || pObj->IsAlwaysUpdate()) {
					if (!(pObj->GetType() == TEXTOBJ && ((CBagTextObject *)pObj)->IsCaption() && pObj->IsImmediateRun())) {
						pObj->update(pBmp, pt, &xIntrRect);

						// If we're in a closeup and we have a character animation
						// playing, then make sure we redraw
						CBagCharacterObject *pCharObj = (CBagCharacterObject *)pObj;

						// Only in closeups
						if (!m_pSlideBitmap->IsPan() && pObj->GetType() == CHAROBJ) {
							// Handle non-modal movies also...
							if (pCharObj->getNumberOfLoops() == 1) {
								SetPreFilterPan(true);
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

								bool b = PreFilterPan();
								SetPreFilterPan(true);

								if (!b) {
									SetDirtyAllObjects(false);
								}
							}
						}
					}
				}

				if (pUpdateArea) {
					xIntrRect.OffsetRect(pt);
					pUpdateArea->addToTail(xIntrRect);
				}

				if (pObj->getRect().PtInRect(xCursorLocation)) {
					pObj->OnMouseOver(0, xCursorLocation, this);
					nMouseOverObj = i;
				}
			}  // if on screen

		}  // While a valid object

		if ((nMouseOverObj < 0) && tempVar)
			NoObjectsUnderMouse();
	}

	return errCode;
}

// Delete the foreground objects
void CBagPanWindow::DeleteFGObjects() {
	m_pFGObjectList->removeAll();
}

ErrorCode CBagPanWindow::InsertFGObjects(CBagObject *pBmp) {
	// Can't use a nullptr pointer
	Assert(pBmp != nullptr);

	m_pFGObjectList->addToTail(pBmp);

	return _errCode;
}

CBagObject *CBagPanWindow::GetFGObjects(const CBofString &sObjName) {
	int nListLen = m_pFGObjectList->getCount();

	for (int i = 0; i < nListLen; ++i) {
		CBagObject *pObj = (*m_pFGObjectList)[i];
		if ((pObj->GetRefName().Find(sObjName)) != -1)
			return pObj;
	}

	return nullptr;
}


void CBagPanWindow::ActivateView() {
	if (m_pSlideBitmap) {
		m_pSlideBitmap->ActivateScrolling();
		m_pSlideBitmap->SetRotateRate(CBofPoint(32, 16));
	}
}

void CBagPanWindow::DeActivateView() {
	if (m_pSlideBitmap) {
		m_pSlideBitmap->DeActivateScrolling();
	}
}

void CBagPanWindow::enable() {
	ActivateView();
}

void CBagPanWindow::disable() {
	flushAllMessages();
	DeActivateView();
}

ErrorCode CBagPanWindow::onCursorUpdate(int nCurrObj) {
	Assert(IsValidObject(this));
	CBagObject *pObj;

	if ((nCurrObj >= 0) && ((pObj = GetObjectByPos(nCurrObj)) != nullptr)) {
		CBagMasterWin::setActiveCursor(pObj->GetOverCursor());

	} else if (CBagWield::GetWieldCursor() >= 0) {
		CBagMasterWin::setActiveCursor(CBagWield::GetWieldCursor());
	}

	return _errCode;
}

void CBagPanWindow::onKeyHit(uint32 lKey, uint32 lRepCount) {
	Assert(IsValidObject(this));

	if (m_pSlideBitmap != nullptr) {
		switch (lKey) {
		case BKEY_MINUS:
			if (m_nCorrection == 0) {
				m_nCorrection = 64;
			}
			m_nCorrection >>= 1;
			m_pSlideBitmap->setCorrWidth(m_nCorrection);
			CBagMasterWin::setCorrection(m_nCorrection);
			break;

		case BKEY_PLUS:
			if (m_nCorrection == 0) {
				m_nCorrection = 1;

			} else if ((m_nCorrection <<= 1) > 32) {
				m_nCorrection = 0;
			}
			m_pSlideBitmap->setCorrWidth(m_nCorrection);
			CBagMasterWin::setCorrection(m_nCorrection);
			break;

		case BKEY_LEFT:
			m_pSlideBitmap->SetRotateRate(CBofPoint(16, 8));
			m_pSlideBitmap->SetDirection(CBagPanBitmap::kDirRIGHT);
			break;

		case BKEY_RIGHT:
			m_pSlideBitmap->SetRotateRate(CBofPoint(16, 8));
			m_pSlideBitmap->SetDirection(CBagPanBitmap::kDirLEFT);
			break;

		case BKEY_UP:
			m_pSlideBitmap->SetRotateRate(CBofPoint(16, 8));
			m_pSlideBitmap->SetDirection(CBagPanBitmap::kDirDOWN);
			break;

		case BKEY_DOWN:
			m_pSlideBitmap->SetRotateRate(CBofPoint(16, 8));
			m_pSlideBitmap->SetDirection(CBagPanBitmap::kDirUP);
			break;
		}
	}

	CBagStorageDevWnd::onKeyHit(lKey, lRepCount);
}

void CBagPanWindow::onMouseMove(uint32 nFlags, CBofPoint *p, void *) {
	Assert(IsValidObject(this));
	Assert(p != nullptr);

	CBofPoint xPoint = *p;

	CBagStorageDevWnd::onMouseMove(nFlags, &xPoint);

	if (!IsCloseup()) {
		if (m_pSlideBitmap != nullptr) {
			if (m_cRightRect.PtInRect(xPoint)) {
				m_pSlideBitmap->SetRotateRate(CBofPoint(((xPoint.x - m_xMovementRect.right) * (m_nPanSpeed + 1)) / 2, 0));
				m_pSlideBitmap->SetDirection(CBagPanBitmap::kDirLEFT);
				CBagMasterWin::setActiveCursor(BOFRTCURSOR);

			} else if (m_cLeftRect.PtInRect(xPoint)) {
				m_pSlideBitmap->SetRotateRate(CBofPoint(((m_xMovementRect.left - xPoint.x) * (m_nPanSpeed + 1)) / 2, 0));
				m_pSlideBitmap->SetDirection(CBagPanBitmap::kDirRIGHT);
				CBagMasterWin::setActiveCursor(BOFLTCURSOR);

			} else if (m_cBottomRect.PtInRect(xPoint)) {
				m_pSlideBitmap->SetRotateRate(CBofPoint(0, (xPoint.y - m_xMovementRect.bottom)));
				m_pSlideBitmap->SetDirection(CBagPanBitmap::kDirUP);
				CBagMasterWin::setActiveCursor(BOFDNCURSOR);

			} else if (m_cTopRect.PtInRect(xPoint)) {
				m_pSlideBitmap->SetRotateRate(CBofPoint(0, (m_xMovementRect.top - xPoint.y)));
				m_pSlideBitmap->SetDirection(CBagPanBitmap::kDirDOWN);
				CBagMasterWin::setActiveCursor(BOFUPCURSOR);

			} else {
				m_pSlideBitmap->SetDirection(CBagPanBitmap::kDirNONE);
			}

			if (GetLActiveObject() && GetLActivity()) {
				m_pSlideBitmap->SetDirection((CBagPanBitmap::Direction)(m_pSlideBitmap->GetDirection() | CBagPanBitmap::kDirVIEW));
			}
		}
	}


	// Change cursor based on the Foreground object list
	// Run thru background object list and find if the cursor is over an object
	CBofList<CBagObject *> *pList = m_pFGObjectList;
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
			if (pOverObj->GetRefName().Find("BPDA_WLD") != -1) {
				CBagPDA *pPda = (CBagPDA *)SDEV_MANAGER->GetStorageDevice(pOverObj->GetRefName());
				if (pPda != nullptr) {
					CBofRect cRect = pOverObj->getRect();
					nCursorID = pPda->getProperCursor(xPoint, cRect);
				}
			}

			// Still no luck, if we're wielding, use that cursor.
			if (nCursorID == -1) {
				nCursorID = pOverObj->GetOverCursor();
				if (CBagWield::GetWieldCursor() >= 0 && ((pOverObj->GetRefName().Find("BWIELD_WLD") != -1) || (pOverObj->GetRefName().Find("THUD_WLD") != -1))) {
					nCursorID = CBagWield::GetWieldCursor();
				}
			}

			CBagMasterWin::setActiveCursor(nCursorID);
		}
	}

	CBagObject *pObj = GetLActiveObject();

	if (m_bDraggingObject) {
		pObj = m_pFGObjectList->getNodeItem(m_pFGObjectList->getCount() - 1);
		pObj->setPosition(xPoint);
	}

	*m_xCursorLocation = xPoint;
}

void CBagPanWindow::onLButtonDown(uint32 nFlags, CBofPoint *xPoint, void *) {
	int nCount = m_pFGObjectList->getCount();
	if (nCount != 0) {
		for (int i = 0; i < nCount; ++i) {
			CBagObject *pObj = m_pFGObjectList->getNodeItem(i);

			if (pObj->isInside(*xPoint)) {
				pObj->onLButtonDown(nFlags, xPoint, this);
				return;
			}
		}
	}
	CBagStorageDevWnd::onLButtonDown(nFlags, xPoint);

	MOUSE_ACTIVITY  nMA = GetLActivity();
	CBagObject *pActObj = GetLActiveObject();

	if (nMA && pActObj && pActObj->IsMovable()) {
		m_bDraggingObject = true;
		m_bDraggingStart = pActObj->getPosition();
		m_pFGObjectList->addToTail(pActObj);
	}
}

void CBagPanWindow::onLButtonUp(uint32 nFlags, CBofPoint *xPoint, void *) {
	MOUSE_ACTIVITY  nMA = GetLActivity();
	CBagObject *pActObj = GetLActiveObject();

	if ((pActObj != nullptr) && (nMA == kMouseDRAGGING) &&
			(pActObj->GetType() == BUTTONOBJ) &&
			(((CBagButtonObject *)pActObj)->getButtonType() == CBagButtonObject::BTN_SLIDER)) {
		pActObj->onLButtonUp(nFlags, xPoint, this);

		// We are not dragging a slider anymore
		SetLActivity(kMouseNONE);
		SetLActiveObject(nullptr);

	} else {
		bool bMoved = false;

		if (m_bDraggingObject) {
			m_bDraggingObject = false;
			pActObj = m_pFGObjectList->removeTail();

			int nCount = m_pFGObjectList->getCount();
			if (nCount != 0) {
				for (int i = 0; i < nCount; ++i) {
					CBagObject *pObj = m_pFGObjectList->getNodeItem(i);
					CBofRect xBmpRect = pObj->getRect();
					if (xBmpRect.PtInRect(*xPoint)) {
						pObj->onObjInteraction(pActObj, this);
					}
				}
			}

			if (pActObj->getPosition() != m_bDraggingStart) {
				pActObj->setPosition(m_bDraggingStart);
				bMoved = true;
			}
		}

		if (!bMoved) {
			// Parse backwards to get topmost obj 1st
			int nCount = m_pFGObjectList->getCount();
			if (nCount != 0) {
				for (int i = nCount - 1; i >= 0; --i) {
					CBagObject *pObj = m_pFGObjectList->getNodeItem(i);
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

	if (m_pSlideBitmap)
		xMaxPanBmpRect = m_pSlideBitmap->GetMaxView();
	else
		xMaxPanBmpRect.SetRect(0, 0, DEF_WIDTH, DEF_HEIGHT);

	CBofPoint vp;
	CBofSize  vs = CBofSize(cx, cy);

	// Check if the current view is greater than the max view
	if (vs.cx > xMaxPanBmpRect.width())
		vs.cx = xMaxPanBmpRect.width();

	if (GetSlideBitmap()->width() > 480)
		vs.cy = 3 * vs.cx / 4;

	if (vs.cy > xMaxPanBmpRect.height())
		vs.cy = xMaxPanBmpRect.height();

	SetViewPortSize(vs);

	vp.x = (cx - vs.cx) / 2;
	vp.y = 10;

	SetViewPortPos(vp);

	vp.x += nRange;
	vp.y += nRange;
	vs.cx -= (2 * nRange); // Subtract 2*nRange from vs because there's
	vs.cy -= (2 * nRange); // a boundary rectangle on the left and right
	// *and* the top and bottom.

	CBofRect cRect(vp, vs);

	m_cTopRect.SetRect(cRect.left, 0, cRect.right, cRect.top);
	m_cBottomRect.SetRect(cRect.left, cRect.bottom, cRect.right, 480 - 1);
	m_cLeftRect.SetRect(0, cRect.top, cRect.left, cRect.bottom);
	m_cRightRect.SetRect(cRect.right, cRect.top, 640 - 1, cRect.bottom);

	SetMovementRect(cRect);
}


void CBagPanWindow::OnWindowPosChanging(WindowPos *lpwndpos) {
	const int MENUNBORDER = 50;
	CBofRect xSlideBmpRect;

	xSlideBmpRect.SetRect(0, 0, DEF_WIDTH, DEF_HEIGHT);

	if (lpwndpos->cx > xSlideBmpRect.width())
		lpwndpos->cx = xSlideBmpRect.width();
	if (lpwndpos->cy > xSlideBmpRect.height() + MENUNBORDER)
		lpwndpos->cy = xSlideBmpRect.height() + MENUNBORDER;
}

void CBagPanWindow::FlushInputEvents() {
	g_system->getEventManager()->purgeKeyboardEvents();
	g_system->getEventManager()->purgeMouseEvents();
}

#define PAN_WIDTH   2048
#define PAN_HEIGHT   512

#define HALF_PAN_WIDTH   (PAN_WIDTH/2)

uint32 CBagPanWindow::RotateTo(CBofPoint xPoint, int nRate) {
	Assert(nRate > 0);

	if (m_pSlideBitmap && (xPoint.x != -1) && (xPoint.y != -1)) {
		CBofRect r = m_pSlideBitmap->GetCurrView();

		for (;;) {
			CBofPoint xCurrPos = r.TopLeft();
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

			r.OffsetRect(nRateX, nRateY);

			m_pSlideBitmap->SetCurrView(r);
			PaintScreen();
		}
	}

	return 0;
}


uint32 CBagPanWindow::Benchmark() {
	DeActivateView();
	m_pSlideBitmap->ActivateScrolling();

	m_pSlideBitmap->SetRotateRate(CBofPoint(8, 0));
	m_pSlideBitmap->SetDirection(CBagPanBitmap::kDirLEFT);

	TimerStart();
	for (int i = 0; i < 50; i++)
		PaintScreen();
	uint32 dTime = timerStop();

	ActivateView();
	return dTime;
}

bool CBagPanWindow::DeactivatePDA() {
	// If we have a PDA and the pda is active
	if (_pPDABmp && _pPDABmp->isActivated()) {
		// deactivate it
		_pPDABmp->deactivate();

		return true;        // PDA successfully deactivated
	}
	return false;               // PDA already deactivated
}

bool CBagPanWindow::ActivatePDA() {
	// If we have a BMP and the pda is not active
	if (_pPDABmp&& (!_pPDABmp->isActivated() || _pPDABmp->isActivating())) {
		_pPDABmp->activate();  // activate it

		return true;            // PDA successfully activated
	}

	return false;                   // PDA already activated
}

const CBofPoint CBagPanWindow::DevPtToViewPort(const CBofPoint &xPoint) {
	CRect r = m_pSlideBitmap->GetCurrView();
	CBofPoint p;

	p.x = xPoint.x + r.left - m_xVeiwPortPos.x;
	p.y = xPoint.y + r.top - m_xVeiwPortPos.y;

	if (m_pSlideBitmap->IsPan()) {
		if (p.x >= m_pSlideBitmap->width())
			p.x -= m_pSlideBitmap->width();

		// If this view is being painted with a correction, we need
		// to correct the point to the warped view space.
		if (m_nCorrection != 0) {
			p = m_pSlideBitmap->WarpedPoint(p);
		}
	}

	return p;
}


const CBofPoint CBagPanWindow::ViewPortToDevPt(const CBofPoint &xPoint) {
	CRect r = m_pSlideBitmap->GetCurrView();
	CBofPoint p(xPoint.x - r.left, xPoint.y - r.top);

	if (p.x < 0)
		p.x += m_pSlideBitmap->width();

	return p;
}

ErrorCode CBagPanWindow::AttachActiveObjects() {
	CBagStorageDev::AttachActiveObjects();

	if (_pPDABmp != nullptr) {
		_pPDABmp->attachActiveObjects();
	}

	return _errCode;
}

} // namespace Bagel
