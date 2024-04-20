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
#include "bagel/boflib/sound.h"
#include "bagel/baglib/button_object.h"
#include "bagel/baglib/character_object.h"
#include "bagel/baglib/text_object.h"

namespace Bagel {

#if defined(PAINT_TIMER) && !BOF_MAC
#define CSLDE_TIMER_ID      11
#define CSLDE_TIMER_SPEED   50
#endif


CBofPoint g_cInitLoc;       // This is the initial location for the next new pan (only option at this point)
bool g_bUseInitLoc;

CBagPDA *CBagPanWindow::m_pPDABmp;      // Pointer to the PDA object
int CBagPanWindow::m_nCorrection;
int CBagPanWindow::m_nPanSpeed;
CBagWield *CBagPanWindow::m_pWieldBmp;  // Pointer to the WEILD object

void CBagPanWindow::initialize() {
	g_bUseInitLoc = false;
	m_pPDABmp = nullptr;
	m_nCorrection = 4;
	m_nPanSpeed = 1;
	m_pWieldBmp = nullptr;
}

CBagPanWindow::CBagPanWindow() : CBagStorageDevWnd() {
	CBofRect tmpRect;

#if BOF_MAC || BOF_WINMAC
	m_xVeiwPortPos = CBofPoint(66, 20);
#else
	m_xVeiwPortPos = CBofPoint(0, 20);
#endif
	m_xMovementRect.SetRectEmpty();

	m_nCorrection = CBagMasterWin::GetCorrection();
	m_nPanSpeed = CBagMasterWin::GetPanSpeed();
	m_pSlideBitmap = nullptr;
	m_pPalette = nullptr;
	m_bPaintToBackdrop = true;
	m_bStretchToScreen = false;
	m_bDraggingObject = false;
	m_bDraggingStart = CBofPoint(0, 0);
	m_pVeiwPortBitmap = nullptr;

	// Objects to be painted to the window
	m_pFGObjectList = new CBofList<CBagObject *>();

	// Declare this to be type game win, because it has a pda
	m_xSDevType = SDEV_GAMEWIN;

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

	CBofSprite::CloseLibrary();

	// There is no palette right now
	CBofApp::GetApp()->SetPalette(nullptr);

	return viewRect;
}

CBofPalette *CBagPanWindow::SetSlidebitmap(const CBofString &xSlideBmp, const CBofRect &xSlideRect) {
	if (!xSlideBmp.IsEmpty()) {
		CBofRect cliWnd = GetWindowRect();
		CBofRect viewRect = UnSetSlidebitmap();
		CBofRect cRect = GetWindowRect();

		if ((cRect.Width() <= 0) || (cRect.Width() > DEF_WIDTH))
			cRect.SetRect(cRect.left, cRect.top, cRect.left + DEF_WIDTH, cRect.bottom);

		cRect.bottom = cRect.top + 3 * cRect.Width() / 4 - 1;

		if ((xSlideRect.Width() <= 0) || (xSlideRect.Width() > DEF_WIDTH))
			viewRect.SetRect(xSlideRect.left, xSlideRect.top, xSlideRect.left + DEF_WIDTH, DEF_HEIGHT);
		else
			viewRect = xSlideRect;

		// Open Pan bitmap
		m_pSlideBitmap = new CBagPanBitmap(xSlideBmp, nullptr, viewRect);

		// Make sure the file was found
		if (m_pSlideBitmap == nullptr || !m_pSlideBitmap->IsValid()) {
			m_pPalette = nullptr;
			ReportError(ERR_FOPEN);
		} else {
			// Set the bagel crap
			m_pPalette = m_pSlideBitmap->GetPalette();
			CBofApp::GetApp()->SetPalette(m_pPalette);
			CBagel::GetBagApp()->GetMasterWnd()->SelectPalette(m_pPalette);
			CBofApp::GetApp()->GetMainWindow()->SelectPalette(m_pPalette);
			SelectPalette(m_pPalette);

			CBofSprite::OpenLibrary(m_pPalette);

			m_pSlideBitmap->SetCorrWidth(m_nCorrection);

			CBofBitmap *pBackDropBitmap = new CBofBitmap(DEF_WIDTH + 1, DEF_HEIGHT + 1, m_pPalette);
			if (!pBackDropBitmap || pBackDropBitmap->Height() <= 0 || pBackDropBitmap->Width() <= 0) {
				ReportError(ERR_FOPEN, "Error opening bitmap");
			}
			m_pVeiwPortBitmap = new CBofBitmap(DEF_WIDTH + 1, m_pSlideBitmap->Height() + 1, m_pPalette);
			if (!m_pVeiwPortBitmap || !m_pVeiwPortBitmap->Height() || !m_pVeiwPortBitmap->Width()) {
				ReportError(ERR_FOPEN);
			}
			SetBackdrop(pBackDropBitmap);

			Move(cRect.left, cRect.top);

		}
	}

	return m_pPalette;
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

#if BOF_MAC || BOF_WINMAC
	m_pPalette = nullptr;
#endif

	CBofSprite::CloseLibrary();                 // free the off screen bitmap

	delete m_pFGObjectList;
}

void CBagPanWindow::OnClose() {
	CBagStorageDevWnd::OnClose();

	DestroyWindow();                            // destruct the main window
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

	return m_errCode;
}

ErrorCode CBagPanWindow::OnRender(CBofBitmap *pBmp, CBofRect *pRect) {
	Assert(IsValidObject(this));
	Assert(pBmp != nullptr);

	if (m_pSlideBitmap != nullptr) {
		m_pSlideBitmap->UpdateView();

		CBofRect dstRect(CBofPoint(0, 0), m_pSlideBitmap->GetViewSize());
		CBofRect clientArea = GetClientRect();

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
				m_pVeiwPortBitmap->Paint(pBmp, &dstRect, &srcRect);

			} else {
				// Warp the backdrop to itself
				m_pSlideBitmap->PaintWarped(pBmp, dstRect, srcRect, 0, m_pVeiwPortBitmap, offsetRect);
			}

			if (IsFiltered()) {
				uint16 nFilterId = GetFilterId();
#if BOF_MAC && __POWERPC__
				CallUniversalProc(m_pBitmapFilter,
				                  uppFilterProcInfo,
				                  nFilterId, pBmp, pRect);
#else
				(*m_pBitmapFilter)(nFilterId, pBmp, pRect);
#endif
			}


			// Now paint it
			PaintObjects(m_pFGObjectList, pBmp, clientArea, nullptr, false);

		} else {
			// Close-Ups
			m_pSlideBitmap->SetCorrWidth(0, false);

			// If we've been given the go ahead to call prefilter, then
			// do so but don't do it again unless asked.
			if (PreFilterPan()) {
				PreFilter(pBmp, pRect, m_pFGObjectList);
				SetPreFilterPan(false);

				// Only paint the slide the first time around, if we paint
				// it in subsequent calls, then we will trash our PDA in this closeup,
				// and that will be a bad thing.
				dstRect.OffsetRect(m_xVeiwPortPos);
				((CBofBitmap *)m_pSlideBitmap)->Paint(pBmp, m_xVeiwPortPos.x, m_xVeiwPortPos.y);
			}

			clientArea.OffsetRect(-m_xVeiwPortPos.x, -m_xVeiwPortPos.y);
			PaintObjects(GetObjectList(), pBmp, clientArea, nullptr);

			if (IsFiltered()) {
				uint16 nFilterId = GetFilterId();
				bool bFiltered = false;
#if BOF_MAC && __POWERPC__
				bFiltered = CallUniversalProc(m_pBitmapFilter,
				                              uppFilterProcInfo,
				                              nFilterId, pBmp, pRect);
#else
				bFiltered = (*m_pBitmapFilter)(nFilterId, pBmp, pRect);
#endif

				if (bFiltered) {
					SetPreFilterPan(true);
				}
			}

			// This must be changed so that the forground objects are actually
			// a list of child windows.  The on paint message is then called with
			// the background bitmap
			PaintObjects(m_pFGObjectList, pBmp, clientArea, nullptr);

			m_pSlideBitmap->SetCorrWidth(m_nCorrection, false);
		}
	}

	return m_errCode;
}

ErrorCode CBagPanWindow::PaintObjects(CBofList<CBagObject *> *list, CBofBitmap *pBmp, CBofRect &viewRect, CBofList<CBofRect> *pUpdateArea, bool tempVar) {
	ErrorCode errCode = ERR_NONE;

	// can't use a nullptr pointer
	Assert(pBmp != nullptr);
	int nCount = list->GetCount();
	if (nCount != 0) {
		int nW = m_pSlideBitmap->Width();
		CBofPoint xCursorLocation = DevPtToViewPort(m_xCursorLocation);
		int nMouseOverObj = -1;

		for (int i = 0; i < nCount; ++i) {
			CBagObject *pObj = list->GetNodeItem(i);
			CBofRect xIntrRect;
			CBofRect xBmpRect = pObj->GetRect();

			// If it is a panorama we have to chech for exceeded bounds
			if (!pObj->IsForeGround() && (nW > 1024) && (viewRect.right > nW) && (xBmpRect.left < (nW / 2)))
				xBmpRect.OffsetRect(nW, 0);

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
						pObj->Update(pBmp, pt, &xIntrRect);

						// If we're in a closeup and we have a character animation
						// playing, then make sure we redraw
						CBofRect cRect = pObj->GetRect();
						CBagCharacterObject *pCharObj = (CBagCharacterObject *)pObj;

						// Only in closeups
						if (!m_pSlideBitmap->IsPan() && pObj->GetType() == CHAROBJ) {
							// Handle non-modal movies also...
							if (pCharObj->GetNumOfLoops() == 1) {
								SetPreFilterPan(true);
							} else {
								if (pObj->IsAttached() &&                       // don't care if it's not running...
								        (pCharObj->IsStationary() == false) &&
								        (pCharObj->GetNumOfLoops() != 0) &&     // Plays multiple or infinite (fly == -1)
								        ((pObj->GetRect().Width() != 480) &&
								         (pObj->GetRect().Height() != 360))) {

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
				}

				if (pUpdateArea) {
					xIntrRect.OffsetRect(pt);
					pUpdateArea->AddToTail(xIntrRect);
				}

				if (pObj->GetRect().PtInRect(xCursorLocation)) {
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

//  delete the foreground objects
void CBagPanWindow::DeleteFGObjects() {
	m_pFGObjectList->RemoveAll();
}

ErrorCode CBagPanWindow::InsertFGObjects(CBagObject *pBmp) {
	// Can't use a nullptr pointer
	Assert(pBmp != nullptr);

	m_pFGObjectList->AddToTail(pBmp);

	return m_errCode;
}

CBagObject *CBagPanWindow::GetFGObjects(const CBofString &sObjName) {
	int nListLen = m_pFGObjectList->GetCount();

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

bool CBagPanWindow::CheckMessages() {
	bool bEndTask = false;

#ifdef SCUMMVM_TODO
#if BOF_MAC
#if DEVELOPMENT
	DebugStr("\pCBagPanWindow::CheckMessages not yet implemented");
#else
	MacMessageBox("CBagPanWindow::CheckMessages not yet implemented", nullptr);
#endif
#else
	MSG msg;

#ifndef RMS_PORT
	if (PeekMessage(&msg, nullptr, MM_MCINOTIFY, MM_MCINOTIFY, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
#else
	if (PeekMessage(&msg, nullptr, WM_CREATE, WM_MOUSELAST, PM_REMOVE)) {
		if (msg.message == WM_CLOSE || msg.message == WM_QUIT) {
			bEndTask = true;
		} else {
			if (!PreTranslateMessage(&msg))
				DispatchMessage(&msg);
		}
	}
#endif
#endif
#endif

	return bEndTask;
}


void CBagPanWindow::Enable() {
	ActivateView();

#if BOF_MAC
	// Don't know how this tin can ran so long without doing this.
	m_bEnabled = true;
#endif
}

void CBagPanWindow::Disable() {
	FlushAllMessages();
	DeActivateView();

#if BOF_MAC
	m_bEnabled = false;
#endif
}

#if BOF_MAC
void CBagPanWindow::OnActivate() {
	Enable();
}

void CBagPanWindow::OnDeActivate() {
	Disable();
}
#endif

ErrorCode CBagPanWindow::OnCursorUpdate(int nCurrObj) {
	Assert(IsValidObject(this));
	CBagObject *pObj;

	if ((nCurrObj >= 0) && ((pObj = GetObjectByPos(nCurrObj)) != nullptr)) {
		CBagMasterWin::SetActiveCursor(pObj->GetOverCursor());

	} else if (CBagWield::GetWieldCursor() >= 0) {
		CBagMasterWin::SetActiveCursor(CBagWield::GetWieldCursor());
	}

	return m_errCode;
}

void CBagPanWindow::OnKeyHit(uint32 lKey, uint32 lRepCount) {
	Assert(IsValidObject(this));

	if (m_pSlideBitmap != nullptr) {
		switch (lKey) {
		case BKEY_MINUS:
			if (m_nCorrection == 0) {
				m_nCorrection = 64;
			}
			m_nCorrection >>= 1;
			m_pSlideBitmap->SetCorrWidth(m_nCorrection);
			CBagMasterWin::SetCorrection(m_nCorrection);
			break;

		case BKEY_PLUS:
			if (m_nCorrection == 0) {
				m_nCorrection = 1;

			} else if ((m_nCorrection <<= 1) > 32) {
				m_nCorrection = 0;
			}
			m_pSlideBitmap->SetCorrWidth(m_nCorrection);
			CBagMasterWin::SetCorrection(m_nCorrection);
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

	CBagStorageDevWnd::OnKeyHit(lKey, lRepCount);
}

#if !BOF_MAC && defined(PAINT_TIMER)
void CBagPanWindow::OnTimer(uint32 nEventID) {
	if (nEventID == CSLDE_TIMER_ID)
		PaintScreen();
}
#endif

void CBagPanWindow::OnMouseMove(uint32 nFlags, CBofPoint *p, void *) {
	Assert(IsValidObject(this));
	Assert(p != nullptr);

	CBofPoint xPoint = *p;

	CBagStorageDevWnd::OnMouseMove(nFlags, &xPoint);

	if (!IsCloseup()) {
		if (m_pSlideBitmap != nullptr) {
			if (m_cRightRect.PtInRect(xPoint)) {
				m_pSlideBitmap->SetRotateRate(CBofPoint(((xPoint.x - m_xMovementRect.right) * (m_nPanSpeed + 1)) / 2, 0));
				m_pSlideBitmap->SetDirection(CBagPanBitmap::kDirLEFT);
				CBagMasterWin::SetActiveCursor(BOFRTCURSOR);

			} else if (m_cLeftRect.PtInRect(xPoint)) {
				m_pSlideBitmap->SetRotateRate(CBofPoint(((m_xMovementRect.left - xPoint.x) * (m_nPanSpeed + 1)) / 2, 0));
				m_pSlideBitmap->SetDirection(CBagPanBitmap::kDirRIGHT);
				CBagMasterWin::SetActiveCursor(BOFLTCURSOR);

			} else if (m_cBottomRect.PtInRect(xPoint)) {
				m_pSlideBitmap->SetRotateRate(CBofPoint(0, (xPoint.y - m_xMovementRect.bottom)));
				m_pSlideBitmap->SetDirection(CBagPanBitmap::kDirUP);
				CBagMasterWin::SetActiveCursor(BOFDNCURSOR);

			} else if (m_cTopRect.PtInRect(xPoint)) {
				m_pSlideBitmap->SetRotateRate(CBofPoint(0, (m_xMovementRect.top - xPoint.y)));
				m_pSlideBitmap->SetDirection(CBagPanBitmap::kDirDOWN);
				CBagMasterWin::SetActiveCursor(BOFUPCURSOR);

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
		int nCount = pList->GetCount();
		for (int i = 0; i < nCount; ++i) {
			CBagObject *pObj = pList->GetNodeItem(i);

			if (pObj->IsInside(xPoint)) {
				pOverObj = pObj;
			}
		}

		// If we just happen to be over the pda then let our
		// PDA code decide... oh, and we're wielding...
		if (pOverObj != nullptr) {
			int nCursorID = -1;

			// the logz case is handled by onmousemove in zoompda
			if (pOverObj->GetRefName().Find("BPDA_WLD") != -1) {
				CBagPDA *pPda = (CBagPDA *)SDEVMNGR->GetStorageDevice(pOverObj->GetRefName());
				if (pPda != nullptr) {
					CBofRect cRect = pOverObj->GetRect();
					nCursorID = pPda->GetProperCursor(xPoint, cRect);
				}
			}

			// Still no luck, if we're wielding, use that cursor.
			if (nCursorID == -1) {
				nCursorID = pOverObj->GetOverCursor();
				if (CBagWield::GetWieldCursor() >= 0 && ((pOverObj->GetRefName().Find("BWIELD_WLD") != -1) || (pOverObj->GetRefName().Find("THUD_WLD") != -1))) {
					nCursorID = CBagWield::GetWieldCursor();
				}
			}

			CBagMasterWin::SetActiveCursor(nCursorID);
		}
	}

	CBagObject *pObj = GetLActiveObject();

	if (m_bDraggingObject) {
		pObj = m_pFGObjectList->GetNodeItem(m_pFGObjectList->GetCount() - 1);
		pObj->SetPosition(xPoint);
	}

	m_xCursorLocation = xPoint;
}

void CBagPanWindow::OnLButtonDown(uint32 nFlags, CBofPoint *xPoint, void *) {
	int nCount = m_pFGObjectList->GetCount();
	if (nCount != 0) {
		for (int i = 0; i < nCount; ++i) {
			CBagObject *pObj = m_pFGObjectList->GetNodeItem(i);

			// Replace the PtInRect call with the BagObject IsInside call, which does
			// a PtInRect but discounts any hits in a chroma color
#if 0
			CBofRect xBmpRect = pObj->GetRect();
			if (xBmpRect.PtInRect(*xPoint)) {
#else
			if (pObj->IsInside(*xPoint)) {
#endif
				pObj->OnLButtonDown(nFlags, xPoint, this);
				return;
			}
		}
	}
	CBagStorageDevWnd::OnLButtonDown(nFlags, xPoint);

	MOUSE_ACTIVITY  nMA = GetLActivity();
	CBagObject *pActObj = GetLActiveObject();

	if (nMA && pActObj && pActObj->IsMovable()) {
		m_bDraggingObject = true;
		m_bDraggingStart = pActObj->GetPosition();
		m_pFGObjectList->AddToTail(pActObj);
	}
}

void CBagPanWindow::OnLButtonUp(uint32 nFlags, CBofPoint *xPoint, void *) {
	MOUSE_ACTIVITY  nMA = GetLActivity();
	CBagObject *pActObj = GetLActiveObject();

	if ((pActObj != nullptr) && (nMA == kMouseDRAGGING) &&
			(pActObj->GetType() == BUTTONOBJ) &&
			(((CBagButtonObject *)pActObj)->GetButtonType() == CBagButtonObject::SLIDER)) {
		pActObj->OnLButtonUp(nFlags, xPoint, this);

		// We are not dragging a slider anymore
		SetLActivity(kMouseNONE);
		SetLActiveObject(nullptr);

	} else {
		bool bMoved = false;

		if (m_bDraggingObject) {
			m_bDraggingObject = false;
			pActObj = m_pFGObjectList->RemoveTail();

			int nCount = m_pFGObjectList->GetCount();
			if (nCount != 0) {
				for (int i = 0; i < nCount; ++i) {
					CBagObject *pObj = m_pFGObjectList->GetNodeItem(i);
					CBofRect xBmpRect = pObj->GetRect();
					if (xBmpRect.PtInRect(*xPoint)) {
						pObj->OnObjInteraction(pActObj, this);
					}
				}
			}

			if (pActObj->GetPosition() != m_bDraggingStart) {
				pActObj->SetPosition(m_bDraggingStart);
				bMoved = true;
			}
		}

		if (!bMoved) {
			// Parse backwards to get topmost obj 1st
			int nCount = m_pFGObjectList->GetCount();
			if (nCount != 0) {
				for (int i = nCount - 1; i >= 0; --i) {
					CBagObject *pObj = m_pFGObjectList->GetNodeItem(i);
					if (pObj->IsInside(*xPoint)) {
						pObj->OnLButtonUp(nFlags, xPoint, this);
						return;
					}
				}
			}
		}

		CBagStorageDevWnd::OnLButtonUp(nFlags, xPoint);
	}
}

void CBagPanWindow::OnSize(uint32 nType, int cx, int cy) {
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
#ifndef STRETCHTOBACK
	if (m_pSlideBitmap)
		xMaxPanBmpRect = m_pSlideBitmap->GetMaxView();
	else
#endif
		xMaxPanBmpRect.SetRect(0, 0, DEF_WIDTH, DEF_HEIGHT);

#if BOF_MAC     // seems to work fine...
	cx = DEF_WIDTH;
	cy = DEF_HEIGHT;
#elif SCUMMVM_TODO
	if (nType == SIZE_MAXIMIZED) {
		//    Window has been maximized.
		cx = DEF_WIDTH;
		cy = DEF_HEIGHT;
	} else if (nType == SIZE_MINIMIZED) {
		//   Window has been minimized.

	}

#endif
	CBofPoint vp;
	CBofSize  vs = CBofSize(cx, cy);

	// Check if the current view is greater than the max view
	if (vs.cx > xMaxPanBmpRect.Width())
		vs.cx = xMaxPanBmpRect.Width();

	if (GetSlideBitmap()->Width() > 480)
		vs.cy = 3 * vs.cx / 4;

	if (vs.cy > xMaxPanBmpRect.Height())
		vs.cy = xMaxPanBmpRect.Height();

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

	if (lpwndpos->cx > xSlideBmpRect.Width())
		lpwndpos->cx = xSlideBmpRect.Width();
	if (lpwndpos->cy > xSlideBmpRect.Height() + MENUNBORDER)
		lpwndpos->cy = xSlideBmpRect.Height() + MENUNBORDER;

#ifdef RMS_PORT
	CBagStorageDevWnd::OnWindowPosChanging(lpwndpos);
#endif // RMS_PORT
}


#if !BOF_MAC
int32 CBagPanWindow::OnDefWinProc(uint32 nMessage, int16 wParam, int32 lParam) {
	return 0;
}
#endif


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
		CBofPoint xCurrPos;

		for (;;) {
			xCurrPos = r.TopLeft();
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
	uint32 dTime = TimerStop();

	ActivateView();
	return dTime;
}

bool CBagPanWindow::DeactivatePDA() {
	// If we have a PDA
	if (m_pPDABmp) {
		// and the pda is active
		if (m_pPDABmp->IsActivated()) {
			// deactivate it
			m_pPDABmp->Deactivate();

			return true;        // PDA successfully deactivated
		}
	}
	return false;               // PDA already deactivated
}

bool CBagPanWindow::ActivatePDA() {
	// If we have a BMP
	if (m_pPDABmp) {
		// and the pda is not active
		if (!m_pPDABmp->IsActivated() || m_pPDABmp->IsActivating()) {
			m_pPDABmp->Activate();  // activate it

			return true;            // PDA successfully activated
		}
	}

	return false;                   // PDA already activated
}

const CBofPoint CBagPanWindow::DevPtToViewPort(const CBofPoint &xPoint) {
	CRect r = m_pSlideBitmap->GetCurrView();
	CBofPoint p;

	p.x = xPoint.x + r.left - m_xVeiwPortPos.x;
	p.y = xPoint.y + r.top - m_xVeiwPortPos.y;

	if (m_pSlideBitmap->IsPan()) {
		if (p.x >= m_pSlideBitmap->Width())
			p.x -= m_pSlideBitmap->Width();

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
		p.x += m_pSlideBitmap->Width();

	return p;
}

ErrorCode CBagPanWindow::AttachActiveObjects() {
	CBagStorageDev::AttachActiveObjects();

	if (m_pPDABmp != nullptr) {
		m_pPDABmp->AttachActiveObjects();
	}

	return m_errCode;
}

} // namespace Bagel
