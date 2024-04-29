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
#include "bagel/bagel.h"
#include "bagel/boflib/std_keys.h"

namespace Bagel {
namespace SpaceBar {

#define PLAYMODEONLY    1
#define PDAWLD          "BPDA_WLD"
#define WIELDWLD        "BWIELD_WLD"
#define THUDWLD         "THUD_WLD"

SBarThud *CMainWindow::m_pThudBmp = nullptr;    // Pointer to the WEILD object
int CMainWindow::m_nInstances = 0;      // Number of space bar windows
bool CMainWindow::m_bZzazzlVision = false;
CBofRect *CMainWindow::m_xFilterRect;
bool CMainWindow::chipdisp;
int CMainWindow::pause;

static int g_nPDAIncrement = 13;

void CMainWindow::initialize() {
	m_xFilterRect = new CBofRect();
}

void CMainWindow::shutdown() {
	delete m_xFilterRect;
}

CMainWindow::CMainWindow() {
	m_cLastLoc.x = 1195;
	m_cLastLoc.y = 105;

	m_pMenu = nullptr;
	m_pGamePalette = nullptr;

	m_nInstances++;

	m_nGameMode = VRPLAYMODE;
}

CMainWindow::~CMainWindow() {
	if (m_pMenu != nullptr) {
		m_pMenu = nullptr;
	}

	// Delete the filter bitmaps.
	DestroyFilters();

	if (--m_nInstances) {
		m_pPDABmp = nullptr;
		m_pWieldBmp = nullptr;
		m_pThudBmp = nullptr;
	}
}

#define PAN_WIDTH 640
#define PAN_HEIGHT 480

ErrorCode CMainWindow::attach() {
	CBofRect tmpRect(0, 0, PAN_WIDTH - 1, PAN_HEIGHT - 1);

	chipdisp = false;
	pause = 0;

	// Have to know if we're being activated from a zoom... if so, then
	// we don't want any foreground objects.
	bool bForegroundObj = true;

	g_allowPaintFl = true;

	CBagStorageDev *pSDev = SDEVMNGR->GetStorageDevice(GetPrevSDev());
	if (pSDev && pSDev->GetDeviceType() == SDEV_ZOOMPDA) {
		bForegroundObj = false;
	}

	// Get rid of any extra mouse button clicks
	FlushInputEvents();

	// Create the window
	CBofString s = GetName();
	create(s.GetBuffer(), &tmpRect, CBagel::getBagApp()->getMasterWnd());

	// Associate this window with callbacks so that any public member function can
	// be accessed by objects inserted into this class.
	SetAssociateWnd(this);

	// Assume we will use the view we had last time.
	CBofRect rView;
	rView.SetRect(m_cLastLoc.x, m_cLastLoc.y, m_cLastLoc.x - 1, m_cLastLoc.y - 1);
	if (g_engine->g_bUseInitLoc) {
		rView.SetRect(g_engine->g_cInitLoc.x, g_engine->g_cInitLoc.y,
			g_engine->g_cInitLoc.x - 1, g_engine->g_cInitLoc.y - 1);
		g_engine->g_bUseInitLoc = false;
	}

	CBofSound::AudioTask();

	CBofPalette *bofpal = SetSlidebitmap(GetBackgroundName(), rView);
	SetPalPtr(bofpal);

	CBagel::getBagApp()->getMasterWnd()->SelectPalette(bofpal);
	CBofApp::GetApp()->SetPalette(bofpal);

	ActivateView();
	CBofSound::AudioTask();

	g_pLastWindow = this;

	if (m_nGameMode == VRPLAYMODE && bForegroundObj == true) {
		if (!m_pThudBmp) {
			pSDev = SDEVMNGR->GetStorageDevice(THUDWLD);
			if (pSDev != nullptr) {
				m_pThudBmp = (SBarThud *)pSDev;
				m_pThudBmp->SetAssociateWnd(this);
				if (!m_pThudBmp->isAttached())
					m_pThudBmp->attach();
				InsertFGObjects(m_pThudBmp);
				m_pThudBmp->SetVisible(true);
			}
		}

		if (m_pThudBmp && (CBagObject *)nullptr == GetFGObjects(CBofString(THUDWLD))) {
			CBofRect r(1, tmpRect.Height() - 101, 101, tmpRect.Height() - 1);
			m_pThudBmp->SetAssociateWnd(this);
			InsertFGObjects(m_pThudBmp);
		}

		if (!m_pWieldBmp) {
			pSDev = SDEVMNGR->GetStorageDevice(WIELDWLD);
			if (pSDev != nullptr) {
				m_pWieldBmp = (CBagWield *)pSDev;
				m_pWieldBmp->SetAssociateWnd(this);
				if (!m_pWieldBmp->isAttached())
					m_pWieldBmp->attach();

				if (m_pWieldBmp->getRect().IsRectEmpty()) {
					CBofRect r(0, 380, 0 + 100 - 1, 380 + 100 - 1);
					m_pWieldBmp->SetRect(r);
					r = GetClientRect();
				}

				InsertFGObjects(m_pWieldBmp);
				m_pWieldBmp->SetVisible(true);

			} else {
				ReportError(ERR_UNKNOWN, "No Wield found");
			}
		}

		if ((CBagObject *)nullptr == GetFGObjects(CBofString(WIELDWLD))) {
			m_pWieldBmp->SetAssociateWnd(this);
			InsertFGObjects(m_pWieldBmp);
		}

		// Create the PDA for the game
		if (!m_pPDABmp) {
			pSDev = SDEVMNGR->GetStorageDevice(PDAWLD);
			if (pSDev != nullptr) {
				m_pPDABmp = (CBagPDA *)pSDev;
				CBofRect r(0, 0, 300, 200);
				m_pPDABmp->SetAssociateWnd(this);
				m_pPDABmp->SetRect(r);
				r = GetClientRect();
				if (!m_pPDABmp->isAttached())
					m_pPDABmp->attach();

				// Allow the script to specify the increment height.
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
				m_pPDABmp->SetVisible(true);

			} else {
				ReportError(ERR_UNKNOWN, "No PDA found");
			}
		}

		if ((CBagObject *)nullptr == GetFGObjects(CBofString(PDAWLD))) {
			CBofRect r(0, 0, 300, 200);
			m_pPDABmp->SetAssociateWnd(this);

			// To fix pda not updating problem
			InsertFGObjects(m_pPDABmp);
		}

		m_pPDABmp->attachActiveObjects();

		// If this world file contains an evt_wld
		pSDev = SDEVMNGR->GetStorageDevice("EVT_WLD");
		if (pSDev != nullptr) {
			// Have we allocated one yet ?
			if (m_pEvtSDev == nullptr) {
				m_pEvtSDev = (CBagEventSDev *)pSDev;
				m_pEvtSDev->SetAssociateWnd(this);
				if (!m_pEvtSDev->isAttached())
					m_pEvtSDev->attach();

				SetTimer(EVAL_EXPR, 1000);
				g_bPauseTimer = false;

			} else {
				KillTimer(EVAL_EXPR);

				// We already allocated one
				// we just need to re-associate the parent
				// window and reset the timer
				m_pEvtSDev->SetAssociateWnd(this);
				SetTimer(EVAL_EXPR, 1000);

				g_bPauseTimer = false;
			}
		}

		// If anyone can tell me why we redo a InsertFG here I'd
		// be interested in knowing since we already did one above...
		//
		// Only do it if we're coming from somewhere other than the zoom
		if (bForegroundObj == true) {
			if ((CBagObject *)nullptr == GetFGObjects(CBofString(WIELDWLD))) {
				m_pWieldBmp->SetAssociateWnd(this);
				InsertFGObjects(m_pWieldBmp);
			}
		}
	}

	OnSize(0, tmpRect.Width() - 1, tmpRect.Height() - 1);

	// Set the first paint to true so the objects
	// won't run until the window is ready
	m_bFirstPaint = true;

	AttachActiveObjects();
	CBofSound::AudioTask();

	Show();

	OnRender(m_pBackdrop, nullptr);

	// Perform fade
	if ((m_pBackdrop != nullptr) && GetFadeId()) {
		int nFade = GetFadeId() & 0x0F;

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

	m_pBackdrop->paint(this, 0, 0);

	return _errCode;
}


ErrorCode CMainWindow::detach() {
	// If this was a closeup then save the leaving position
	m_cLastLoc = GetViewPort().TopLeft();

	CBagPanWindow::detach();

	UnSetSlidebitmap();

	Destroy();

	// When we move from room to room, we should delete
	// all our foreground objects (remove from memory).
	DeleteFGObjects();

	return _errCode;
}


void CMainWindow::OnSize(uint32 nType, int cx, int cy) {
	if (m_pPDABmp) {
		if (GetStretchToScreen()) {
			m_pPDABmp->SetPosInWindow(500, 370, g_nPDAIncrement);
		} else
			m_pPDABmp->SetPosInWindow(cx, cy, g_nPDAIncrement);
	}

	CBagPanWindow::OnSize(nType, cx, cy);
}


ErrorCode CMainWindow::OnCursorUpdate(int nCurrObj) {
	return CBagPanWindow::OnCursorUpdate(nCurrObj);
}


void CMainWindow::onKeyHit(uint32 lKey, uint32 lRepCount) {
	// Terminate app on ALT_Q
	if ((lKey == BKEY_ALT_q) || (lKey == BKEY_ALT_F4)) {
		Close();
		g_engine->quitGame();
	}

	if (lKey == BKEY_SCRL_LOCK) {               // Get a scroll lock hit
		if (GetFilterId() == 0x08) {            // If we're in zzazzl filter
			m_bZzazzlVision = !m_bZzazzlVision; // toggle the paint zzazzl flag
			CBagVar *pVar = VARMNGR->GetVariable("ZZAZZLVISION");

			if (pVar != nullptr) {
				pVar->SetValue(m_bZzazzlVision ? 1 : 0);
			}
		}
	} else {
		// Default action
		CBagPanWindow::onKeyHit(lKey, lRepCount);
	}
}


void CMainWindow::CorrectZzazzlePoint(CBofPoint *p) {
	// Don't correct this boy if he's inside the PDA.
	CBagPDA *pPDA = (CBagPDA *)SDEVMNGR->GetStorageDevice("BPDA_WLD");
	if (pPDA && pPDA->isInside(*p)) {
		return;
	}

	if (!GetMovementRect().PtInRect(*p))
		return;

	int dx = m_xFilterRect->Width();
	int dy = m_xFilterRect->Height();
	int x = m_xFilterRect->left;
	int y = m_xFilterRect->top;
	CBofRect r(x + dx, y + dy, x + dx + dx, y + dy + dy);

	// Attempt to make all squares active in zzazzlvision
	int j;
	for (int i = 0; i < 3; ++i) {
		if (i == 1) {
			// Center row
			x = m_xFilterRect->left;
			j = 0;
		} else {
			// Only two squares in top & bottom rows, start 1/2 a square to the left
			x = m_xFilterRect->left + (dx >> 1);
			j = 1;
		}
		for (; j < 3; ++j) {
			r.SetRect(x, y, x + dx, y + dy);
			if (r.PtInRect(*p)) {
				p->x = m_xFilterRect->left + (p->x - r.left) * 3;
				p->y = m_xFilterRect->top + (p->y - r.top) * 3;
				return;
			}
			x += dx;
		}
		y += dy;
	}
}


void CMainWindow::onMouseMove(uint32 nFlags, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	if (m_cLastPos != *pPoint) {
		m_cLastPos = *pPoint;

		if (GetFilterId() & 0x08) {
			if (m_bZzazzlVision)                // If zzazzl paint is toggled on
				CorrectZzazzlePoint(pPoint);
		}

		CBagPanWindow::onMouseMove(nFlags, pPoint);
	}
}


void CMainWindow::onLButtonUp(uint32 nFlags, CBofPoint *xPoint, void *) {
	if (GetFilterId() & 0x08) {
		if (m_bZzazzlVision)                // If zzazzl paint is toggled on
			CorrectZzazzlePoint(xPoint);
	}

	CBagPanWindow::onLButtonUp(nFlags, xPoint);
}


void CMainWindow::onLButtonDown(uint32 nFlags, CBofPoint *xPoint, void *) {
	if (GetFilterId() & 0x08) {
		if (m_bZzazzlVision)                // If zzazzl paint is toggled on
			CorrectZzazzlePoint(xPoint);
	}

	CBagPanWindow::onLButtonDown(nFlags, xPoint);
}

void CMainWindow::onClose() {
	CBagPanWindow::onClose();       // Destruct the main window
}


CBagObject *CMainWindow::OnNewLinkObject(const CBofString &) {
	return new CMainLinkObject();
}


ErrorCode CMainWindow::SetLoadFilePos(const CBofPoint dstLoc) {
	if (dstLoc.x != 0 || dstLoc.y != 0) {
		m_cLastLoc = dstLoc;
	}

	return _errCode;
}


// Set the ZzazzlVision variable.
void CMainWindow::SetZzazzlVision(bool newValue) {
	m_bZzazzlVision = newValue;
}

} // namespace SpaceBar
} // namespace Bagel
