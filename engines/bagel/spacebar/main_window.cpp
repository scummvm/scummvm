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

#include "bagel/spacebar/main_window.h"
#include "bagel/spacebar/main_link_object.h"
#include "bagel/spacebar/filter.h"
#include "bagel/baglib/event_sdev.h"

namespace Bagel {
namespace SpaceBar {

#define PLAYMODEONLY    1
#define PDAWLD          "BPDA_WLD"
#define WIELDWLD        "BWIELD_WLD"
#define THUDWLD         "THUD_WLD"

SBarThud *CMainWindow::m_pThudBmp = nullptr;    // Pointer to the WEILD object
int CMainWindow::m_nInstances = 0;      // Numver of space bar windows
BOOL CMainWindow::m_bZzazzlVision = FALSE;
CBofRect CMainWindow::m_xFilterRect;
BOOL CMainWindow::chipdisp;
INT CMainWindow::pause;

static INT g_nPDAIncrement = 13;


CMainWindow::CMainWindow(const char *sCommandLine) {
	CBofString WndClass;
	CBofRect tmpRect;
//	ERROR_CODE errCode = ERR_NONE;
//	int rc = 0;
	CBofString sCommLine;

	m_cLastLoc.x = 1195;
	m_cLastLoc.y = 105;

#if BOF_WINDOWS
	m_hDC = nullptr;
#endif

	m_pMenu = nullptr;
	m_pGamePalette = nullptr;

	m_nInstances++;

	if (sCommandLine)
		sCommLine = sCommandLine;

#ifdef BUILDMODE
	m_nGameMode = VRBUILDMODE;
#else
	m_nGameMode = VRPLAYMODE;
#endif

	//#if 0
	//  if (sCommLine.Find("-b")!=-1 || sCommLine.Find("-B")!=-1)
	//      m_nGameMode = VRBUILDMODE;
	//  else if (sCommLine.Find("-v")!=-1 || sCommLine.Find("-V")!=-1)
	//      m_nGameMode = VRPANMODE;
	//  else
	//      m_nGameMode = VRPLAYMODE;
	//#endif

}

CMainWindow::~CMainWindow() {
	if (m_pMenu != nullptr) {
		//delete m_pMenu;
		m_pMenu = nullptr;
	}

	// Delete the filter bitmaps.
	//
	DestroyFilters();

	if (--m_nInstances) {
		if (m_pPDABmp) {
			//delete m_pPDABmp;
			m_pPDABmp = nullptr;
		}

		if (m_pWieldBmp) {
			//delete m_pWieldBmp;
			m_pWieldBmp = nullptr;
		}

		if (m_pThudBmp) {
			//delete m_pThudBmp;
			m_pThudBmp = nullptr;
		}
	}
}

#define PAN_WIDTH 640
#define PAN_HEIGHT 480

ERROR_CODE CMainWindow::Attach() {
	CBofRect        tmpRect(0, 0, PAN_WIDTH - 1, PAN_HEIGHT - 1);
	CBofRect        rView;
	CBagStorageDev *pSDev;
	chipdisp = FALSE;
	pause = 0;

	// have to know if we're being activated from a zoom... if so, then
	// we don't want any foreground objects.
	BOOL bForegroundObj = TRUE;

	g_bAllowPaint = TRUE;

	pSDev = SDEVMNGR->GetStorageDevice(GetPrevSDev());
	if (pSDev && pSDev->GetDeviceType() == SDEV_ZOOMPDA) {
		bForegroundObj = FALSE;
	}

	// Ket rid of any extra mouse button clicks
	FlushInputEvents();

	// Always put game into upper left corner
	//
#if 0 //ndef _DEBUG

	CBofApp *pApp;

	if ((pApp = CBofApp::GetApp()) != nullptr) {
		tmpRect.left = (pApp->ScreenWidth() - PAN_WIDTH) / 2;
		tmpRect.top = (pApp->ScreenHeight() - PAN_HEIGHT) / 2;
		tmpRect.right = tmpRect.left + PAN_WIDTH - 1;
		tmpRect.bottom = tmpRect.top + PAN_HEIGHT - 1;
	}
#endif


	// Create the window as a POPUP so no boarders, title, or menu are present;
	// this is because the game's background art will fill the entire 640x480 area.
	CBofString s = GetName();


	Create(s.GetBuffer(), &tmpRect, CBagel::GetBagApp()->GetMasterWnd());
	//Create(s.GetBuffer(), &tmpRect, CBofApp::GetApp()->GetMainWindow());

	// This should actually be moved to sbarapp, but the load file will then
	// need to be removed from the constructor.
	//CBofApp::GetApp()->SetMainWindow(this);

	// Associtate this window with callbacks so that any public member function can
	// be accessed by objects inserted into this class.
	SetAssociateWnd(this);

	// Assume we will use the view we had last time.
	rView.SetRect(m_cLastLoc.x, m_cLastLoc.y, m_cLastLoc.x - 1, m_cLastLoc.y - 1);
	if (g_bUseInitLoc) {
		rView.SetRect(g_cInitLoc.x, g_cInitLoc.y, g_cInitLoc.x - 1, g_cInitLoc.y - 1);
		g_bUseInitLoc = FALSE;
	}

	CBofSound::AudioTask();

	CBofPalette *bofpal = SetSlidebitmap(GetBackgroundName(), rView);
	SetPalPtr(bofpal);
	//CBofApp::GetApp()->GetMainWindow()->SelectPalette(bofpal);
	CBagel::GetBagApp()->GetMasterWnd()->SelectPalette(bofpal);
	CBofApp::GetApp()->SetPalette(bofpal);

	ActivateView();

	//SetCorrection(4);

	CBofSound::AudioTask();

	g_pLastWindow = this;

#if PDALOC
	OnSize(0, tmpRect.Width(), tmpRect.Height());

	/*if (GetExitOnEdge()) {
	    SetExitOnEdge(GetViewPortPos().x);
	}*/

	//CBagPanWindow::Attach();

	// Set the first paint to true so the objects
	// won't run until the window is ready
	m_bFirstPaint = TRUE;
	AttachActiveObjects();

	// Now allow the run objects to run
	//m_bFirstPaint = FALSE;
	//AttachActiveObjects();
#endif

	if (m_nGameMode == VRPLAYMODE && bForegroundObj == TRUE) {

		//else {
		//  CBagStorageDev *pSDev;
		//    pSDev = nullptr;
		//}
		if (!m_pThudBmp) {
			if ((pSDev = SDEVMNGR->GetStorageDevice(THUDWLD)) != nullptr) {
				m_pThudBmp = (SBarThud *)pSDev;
				m_pThudBmp->SetAssociateWnd(this);
				if (!m_pThudBmp->IsAttached())
					m_pThudBmp->Attach();
				InsertFGObjects(m_pThudBmp);
				m_pThudBmp->SetVisible(TRUE);
			}
		}

		if (m_pThudBmp && (CBagObject *)nullptr == GetFGObjects(CBofString(THUDWLD))) {
			CBofRect r(1, tmpRect.Height() - 101, 101, tmpRect.Height() - 1);
			m_pThudBmp->SetAssociateWnd(this);
			InsertFGObjects(m_pThudBmp);
		}


		//#ifdef WIELD
		if (!m_pWieldBmp) {

			if ((pSDev = SDEVMNGR->GetStorageDevice(WIELDWLD)) != nullptr) {
				m_pWieldBmp = (CBagWield *)pSDev;
				m_pWieldBmp->SetAssociateWnd(this);
				if (!m_pWieldBmp->IsAttached())
					m_pWieldBmp->Attach();

				if (m_pWieldBmp->GetRect().IsRectEmpty()) {

					CBofRect r(0, 380, 0 + 100 - 1, 380 + 100 - 1);
					m_pWieldBmp->SetRect(r);
					r = GetClientRect();
				}

				InsertFGObjects(m_pWieldBmp);
				m_pWieldBmp->SetVisible(TRUE);

			} else {
				ReportError(ERR_UNKNOWN, "No Wield found");
#if 0
				if ((m_pWieldBmp = new CBagWield()) != nullptr) {

					BofMessageBox("No Wield found: searching local assets", __FILE__);

					m_pWieldBmp->SetAssociateWnd(this);

					m_pWieldBmp->LoadFile(WIELDWLDFILE);

					InsertFGObjects(m_pWieldBmp);
					m_pWieldBmp->SetVisible(TRUE);
				}
#endif
			}
		}

		if ((CBagObject *)nullptr == GetFGObjects(CBofString(WIELDWLD))) {
			// CBofRect r(1,tmpRect.Height()-101,101,tmpRect.Height()-1);
			m_pWieldBmp->SetAssociateWnd(this);
			InsertFGObjects(m_pWieldBmp);
		}

		// Create the PDA for the game
		// This needs to be changed to look for a pda in the master list first
		if (!m_pPDABmp) {

			if ((pSDev = SDEVMNGR->GetStorageDevice(PDAWLD)) != nullptr) {
				m_pPDABmp = (CBagPDA *)pSDev;
				CBofRect r(0, 0, 300, 200);
				m_pPDABmp->SetAssociateWnd(this);
				m_pPDABmp->SetRect(r);
				r = GetClientRect();
				if (!m_pPDABmp->IsAttached())
					m_pPDABmp->Attach();

				//allow the script to specify the increment height.
				CBagVar *pVar = VARMNGR->GetVariable("PDAINCREMENT");
				if (pVar) {
					g_nPDAIncrement = pVar->GetNumValue();
					m_pPDABmp->SetPosInWindow(r.Width(), r.Height(), g_nPDAIncrement);
				} else {
					g_nPDAIncrement = PDA_INCREMENT;
					m_pPDABmp->SetPosInWindow(r.Width(), r.Height(), g_nPDAIncrement);
				}
				InsertFGObjects(m_pPDABmp);
				DeactivatePDA();
				m_pPDABmp->SetVisible(TRUE);

			} else {
				ReportError(ERR_UNKNOWN, "No PDA found");
#if 0
				if ((m_pPDABmp = new CBagPDA(/* this, CBofRect(0,0,300,200)*/)) != nullptr) {

					BofMessageBox("No PDA found: searching local assets", __FILE__);

					CBofRect r(0, 0, 300, 200);
					m_pPDABmp->SetAssociateWnd(this);
					m_pPDABmp->SetRect(r);
					r = GetClientRect();

					m_pPDABmp->LoadFile(PDAWLDFILE);

					m_pPDABmp->SetPosInWindow(r.Width(), r.Height(), g_nPDAIncrement);
					InsertFGObjects(m_pPDABmp);
					DeactivatePDA();
					m_pPDABmp->SetVisible(TRUE);
				}
#endif
			}
		}

		if ((CBagObject *)nullptr == GetFGObjects(CBofString(PDAWLD))) {
			CBofRect r(0, 0, 300, 200);
			m_pPDABmp->SetAssociateWnd(this);
			// To fix pda not updating problem
			InsertFGObjects(m_pPDABmp);

		}

		m_pPDABmp->AttachActiveObjects();

		// If this world file contains an evt_wld
		if ((pSDev = SDEVMNGR->GetStorageDevice("EVT_WLD")) != nullptr) {

			// Have we allocated one yet ?
			//
			if (m_pEvtSDev == nullptr) {
				m_pEvtSDev = (CBagEventSDev *)pSDev;
				m_pEvtSDev->SetAssociateWnd(this);
				if (!m_pEvtSDev->IsAttached())
					m_pEvtSDev->Attach();

#if BOF_MAC
				SetTimer(EVAL_EXPR, 1000);  // nullptr for now, will not play music.
#else
				SetTimer(EVAL_EXPR, 1000);
#endif
				g_bPauseTimer = FALSE;

			} else {
				KillTimer(EVAL_EXPR);

				// We already allocated one
				// we just need to re-associate the parent
				// window and reset the timer
				m_pEvtSDev->SetAssociateWnd(this);

#if BOF_MAC
				SetTimer(EVAL_EXPR, 1000);  // nullptr for now, will not play music.
#else
				SetTimer(EVAL_EXPR, 1000);
#endif
				g_bPauseTimer = FALSE;
			}
		}

		// If anyone can tell me why we redo a InsertFG here I'd
		// be interested in knowing since we already did one above...
		//
		// only do it if we're coming from somewhere other than the zoom
		if (bForegroundObj == TRUE) {
			if ((CBagObject *)nullptr == GetFGObjects(CBofString(WIELDWLD))) {
				m_pWieldBmp->SetAssociateWnd(this);
				InsertFGObjects(m_pWieldBmp);
			}
		}
		//#endif  // WIELD

	}

#if !PDALOC
	OnSize(0, tmpRect.Width() - 1, tmpRect.Height() - 1);

	/*if (GetExitOnEdge()) {
	    SetExitOnEdge(GetViewPortPos().x);
	}*/

	//CBagPanWindow::Attach();

	// Set the first paint to true so the objects
	// won't run until the window is ready
	m_bFirstPaint = TRUE;

	AttachActiveObjects();
#endif

	CBofSound::AudioTask();

	Show();

	OnRender(m_pBackdrop, nullptr);

	// Perform fade
	if ((m_pBackdrop != nullptr) && GetFadeId()) {
		int nFade = GetFadeId() & 0x0F;
//		int nSize = GetFadeId() & 0xF0;
		switch (nFade) {

		case 1:
			m_pBackdrop->FadeIn(this);
			break;

		case 2:
			m_pBackdrop->Curtain(this);
			break;

		case 3:
			m_pBackdrop->FadeLines(this);
			break;
		}
	}

	// Preload the DC for speed boost
#if BOF_WINDOWS
	m_hDC = GetDC();
#endif

	m_pBackdrop->Paint(this, 0, 0);

	/*    TimerStart();
	    for (INT ii = 0; ii < 1000; ii++) {
	        OnTimer(EVAL_EXPR);
	    }
	    LogInfo(BuildString("OnTimer * 1000: %ld ms", TimerStop()));*/


	return m_errCode;
}


ERROR_CODE CMainWindow::Detach() {
#if BOF_WINDOWS
	if (m_hDC != nullptr) {
		ReleaseDC(m_hDC);
	}
#endif

	// If this was a closup then save the leaving position
	//if (GetViewPortSize().cx < 1000)

	m_cLastLoc = GetViewPort().TopLeft();

	CBagPanWindow::Detach();

	UnSetSlidebitmap();

	Destroy();

	// when we move from room to room, we should delete
	// all our foreground objects (remove from memory).

	DeleteFGObjects();

	return m_errCode;
}


VOID CMainWindow::OnSize(UINT nType, int cx, int cy) {
	if (m_pPDABmp) {
		if (GetStretchToScreen()) {
			//CBofSize s = GetViewPortSize();
			//m_pPDABmp->SetPosInWindow(s.cx,s.cy);
			m_pPDABmp->SetPosInWindow(500, 370, g_nPDAIncrement);
		} else
			m_pPDABmp->SetPosInWindow(cx, cy, g_nPDAIncrement);
	}
	CBagPanWindow::OnSize(nType, cx, cy);

}


ERROR_CODE CMainWindow::OnCursorUpdate(INT nCurrObj) {
	ERROR_CODE errCode = ERR_NONE;

	/*
	    if (m_pWieldBmp)
	    if (nCurrObj>=0) {
	        CBagObject *pObj = GetObjectByPos(nCurrObj);
	        if (pObj) {
	            int nCur  = (int)pObj->GetType();
	            m_pCursors[nCur]->Set();
	            return errCode;
	        }
	    }
	*/
	errCode = CBagPanWindow::OnCursorUpdate(nCurrObj);

	return errCode;
}


VOID CMainWindow::OnKeyHit(ULONG lKey, ULONG lRepCount) {
	// terminate app on ALT_Q
	//
	if ((lKey == BKEY_ALT_Q) || (lKey == BKEY_ALT_q) || (lKey == BKEY_ALT_F4)) {

		Close();

	}
	if (lKey == BKEY_SCRL_LOCK) {               // get a scroll lock hit
		if (GetFilterId() == 0x08) {            // if we're in zzazzl filter
			m_bZzazzlVision = !m_bZzazzlVision; // toggle the paint zzazzl flag
			CBagVar *pVar;

			if ((pVar = VARMNGR->GetVariable("ZZAZZLVISION")) != nullptr) {
				pVar->SetValue(m_bZzazzlVision ? 1 : 0);
			}
		}
	} else {

		// default action
		CBagPanWindow::OnKeyHit(lKey, lRepCount);
	}
}


VOID CMainWindow::CorrectZzazzlePoint(CBofPoint *p) {
	// don't correct this boy if he's inside the PDA.
	CBagPDA *pPDA = (CBagPDA *)SDEVMNGR->GetStorageDevice("BPDA_WLD");
	if (pPDA && pPDA->IsInside(*p)) {
		return;
	}

#ifdef ALL_WINDOWS_ACTIVE
	if (GetMovementRect().PtInRect(*p)) {
		int dx = m_xFilterRect.Width();
		int dy = m_xFilterRect.Height();

		int x = (p->x - GetMovementRect().left);
		int y = (p->y - GetMovementRect().top);
		int x = p->x;
		int y = p->y;
		int i = 0, j = 0;

		while (y > dy) {
			j++;
			y -= dy;
		}
		if (j == 1)
			x -= (dx >> 2);

		while (x > dx) {
			//i++;
			x -= dy;
		}

		p->x = x * 3 + GetMovementRect().left;
		p->y = y * 3 + GetMovementRect().top;
	}
#endif

#ifdef TRY2
	if (!GetMovementRect().PtInRect(*p))
		return;

	int dx = m_xFilterRect.Width();
	int dy = m_xFilterRect.Height();
	int x = m_xFilterRect.left;
	int y = m_xFilterRect.top;
	CBofRect r(x + dx, y + dy, x + dx + dx, y + dy + dy);

	if (r.PtInRect(*p)) {
		p->x = m_xFilterRect.left + (p->x - r.left) * 3;
		p->y = m_xFilterRect.top + (p->y - r.top) * 3;
	} else {
		if (p->y < r.top)
			p->y = m_xFilterRect.top/*+2*/;
		else // if (*p->x >r.left)
			p->y = m_xFilterRect.bottom/*-5*/;
		if (p->x < r.left)
			p->x = m_xFilterRect.left/*+2*/;
		else // if (*p->x >r.right)
			p->x = m_xFilterRect.right/*-2*/;
	}

#endif
	//#ifdef TRY2
	if (!GetMovementRect().PtInRect(*p))
		return;

	int dx = m_xFilterRect.Width();
	int dy = m_xFilterRect.Height();
	int x = m_xFilterRect.left;
	int y = m_xFilterRect.top;
	CBofRect r(x + dx, y + dy, x + dx + dx, y + dy + dy);

	// Only the center box is active:
	//  if (r.PtInRect(*p)) {
	//      p->x = m_xFilterRect.left + (p->x-r.left)*3;
	//      p->y = m_xFilterRect.top  + (p->y-r.top)*3;
	//      break;
	//  }
	// attempt to make all squares active in zzazzlvision
	int i, j;
	for (i = 0; i < 3; ++i) {
		if (i == 1) {
			// center row
			x = m_xFilterRect.left;
			j = 0;
		} else {
			// only two squares in top & bottom rows, start 1/2 a square to the left
			x = m_xFilterRect.left + (dx >> 1);
			j = 1;
		}
		for (; j < 3; ++j) {
			r.SetRect(x, y, x + dx, y + dy);
			if (r.PtInRect(*p)) {
				p->x = m_xFilterRect.left + (p->x - r.left) * 3;
				p->y = m_xFilterRect.top + (p->y - r.top) * 3;
				return;
			}
			x += dx;
		}
		y += dy;
	}

	//#endif
}


VOID CMainWindow::OnMouseMove(UINT nFlags, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	if (m_cLastPos != *pPoint) {
		m_cLastPos = *pPoint;

		if (GetFilterId() & 0x08) {
			if (m_bZzazzlVision)                // if zzazzl paint is toggled on
				CorrectZzazzlePoint(pPoint);
		}

		CBagPanWindow::OnMouseMove(nFlags, pPoint);
	}
}



VOID CMainWindow::OnLButtonUp(UINT nFlags, CBofPoint *xPoint, void *) {
	if (GetFilterId() & 0x08) {
		if (m_bZzazzlVision)                // if zzazzl paint is toggled on
			CorrectZzazzlePoint(xPoint);
	}

	CBagPanWindow::OnLButtonUp(nFlags, xPoint);
}


VOID CMainWindow::OnLButtonDown(UINT nFlags, CBofPoint *xPoint, void *) {
	if (GetFilterId() & 0x08) {
		if (m_bZzazzlVision)                // if zzazzl paint is toggled on
			CorrectZzazzlePoint(xPoint);
	}

	CBagPanWindow::OnLButtonDown(nFlags, xPoint);
}

VOID CMainWindow::OnClose() {
	CBagPanWindow::OnClose();       // Destruct the main window
}


CBagObject *CMainWindow::OnNewLinkObject(const CBofString &) {
	return new CMainLinkObject();
}


ERROR_CODE CMainWindow::SetLoadFilePos(const CBofPoint dstLoc) {
	if (dstLoc.x != 0 || dstLoc.y != 0) {
		m_cLastLoc = dstLoc;
		//g_cInitLoc = dstLoc;
	}

	return m_errCode;
}


// Get the setting of the ZzazzlVision variable.
BOOL CMainWindow::GetZzazzlVision() {
	return m_bZzazzlVision;
}

// Set the ZzazzlVision variable.  Return the old setting to the caller.
BOOL CMainWindow::SetZzazzlVision(BOOL newValue) {
	BOOL previousValue = m_bZzazzlVision;

	m_bZzazzlVision = newValue;
	return previousValue;
}

// Handles objects menus
void *fCObjectHandler(INT /*nRefId*/, void *pvInfo) {
	return nullptr;
}

} // namespace SpaceBar
} // namespace Bagel
