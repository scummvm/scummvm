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
#include "bagel/spacebar/baglib/event_sdev.h"
#include "bagel/bagel.h"
#include "bagel/spacebar/boflib/std_keys.h"

namespace Bagel {
namespace SpaceBar {

#define PLAY_MODE_ONLY    1

SBarThud *CMainWindow::_pThudBmp = nullptr;    // Pointer to the WIELD object
int CMainWindow::_nInstances = 0;      // Number of space bar windows
bool CMainWindow::_bZzazzlVision = false;
CBofRect *CMainWindow::_xFilterRect;
bool CMainWindow::chipdisp;
int CMainWindow::pause;

static int g_nPDAIncrement = 13;

void CMainWindow::initialize() {
	_xFilterRect = new CBofRect();
}

void CMainWindow::shutdown() {
	delete _xFilterRect;
}

CMainWindow::CMainWindow() {
	_cLastLoc.x = 1195;
	_cLastLoc.y = 105;

	_pMenu = nullptr;
	_pGamePalette = nullptr;

	_nInstances++;

	_nGameMode = VRPLAYMODE;
}

CMainWindow::~CMainWindow() {
	if (_pMenu != nullptr) {
		_pMenu = nullptr;
	}

	// Delete the filter bitmaps.
	destroyFilters();

	if (--_nInstances) {
		_pPDABmp = nullptr;
		_pWieldBmp = nullptr;
		_pThudBmp = nullptr;
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

	CBagStorageDev *pSDev = g_SDevManager->getStorageDevice(getPrevSDev());
	if (pSDev && pSDev->getDeviceType() == SDEV_ZOOMPDA) {
		bForegroundObj = false;
	}

	// Get rid of any extra mouse button clicks
	flushInputEvents();

	// Create the window
	CBofString s = getName();
	create(s.getBuffer(), &tmpRect, CBagel::getBagApp()->getMasterWnd());

	// Associate this window with callbacks so that any public member function can
	// be accessed by objects inserted into this class.
	setAssociateWnd(this);

	// Assume we will use the view we had last time.
	CBofRect rView;
	rView.setRect(_cLastLoc.x, _cLastLoc.y, _cLastLoc.x - 1, _cLastLoc.y - 1);
	if (g_engine->g_bUseInitLoc) {
		rView.setRect(g_engine->g_cInitLoc.x, g_engine->g_cInitLoc.y,
		              g_engine->g_cInitLoc.x - 1, g_engine->g_cInitLoc.y - 1);
		g_engine->g_bUseInitLoc = false;
	}

	CBofSound::audioTask();

	CBofPalette *bofpal = setSlideBitmap(getBackgroundName(), rView);
	setPalPtr(bofpal);

	CBagel::getBagApp()->getMasterWnd()->selectPalette(bofpal);
	CBofApp::getApp()->setPalette(bofpal);

	activateView();
	CBofSound::audioTask();

	g_lastWindow = this;

	if (_nGameMode == VRPLAYMODE && bForegroundObj == true) {
		if (!_pThudBmp) {
			pSDev = g_SDevManager->getStorageDevice(THUD_WLD);
			if (pSDev != nullptr) {
				_pThudBmp = (SBarThud *)pSDev;
				_pThudBmp->setAssociateWnd(this);
				if (!_pThudBmp->isAttached())
					_pThudBmp->attach();
				insertFGObjects(_pThudBmp);
				_pThudBmp->setVisible(true);
			}
		}

		if (_pThudBmp && (CBagObject *)nullptr == getFGObjects(CBofString(THUD_WLD))) {
			_pThudBmp->setAssociateWnd(this);
			insertFGObjects(_pThudBmp);
		}

		if (!_pWieldBmp) {
			pSDev = g_SDevManager->getStorageDevice(WIELD_WLD);
			if (pSDev == nullptr)
				fatalError(ERR_UNKNOWN, "No Wield found");

			_pWieldBmp = (CBagWield *)pSDev;
			_pWieldBmp->setAssociateWnd(this);
			if (!_pWieldBmp->isAttached())
				_pWieldBmp->attach();

			if (_pWieldBmp->getRect().isRectEmpty()) {
				CBofRect r(0, 380, 0 + 100 - 1, 380 + 100 - 1);
				_pWieldBmp->setRect(r);
				r = getClientRect();
			}

			insertFGObjects(_pWieldBmp);
			_pWieldBmp->setVisible(true);
		}

		if ((CBagObject *)nullptr == getFGObjects(CBofString(WIELD_WLD))) {
			_pWieldBmp->setAssociateWnd(this);
			insertFGObjects(_pWieldBmp);
		}

		// Create the PDA for the game
		if (!_pPDABmp) {
			pSDev = g_SDevManager->getStorageDevice(PDA_WLD);
			if (pSDev == nullptr)
				fatalError(ERR_UNKNOWN, "No PDA found");

			_pPDABmp = (CBagPDA *)pSDev;
			CBofRect r(0, 0, 300, 200);
			_pPDABmp->setAssociateWnd(this);
			_pPDABmp->setRect(r);
			r = getClientRect();
			if (!_pPDABmp->isAttached())
				_pPDABmp->attach();

			// Allow the script to specify the increment height.
			CBagVar *pVar = g_VarManager->getVariable("PDAINCREMENT");
			if (pVar) {
				g_nPDAIncrement = pVar->getNumValue();
				_pPDABmp->setPosInWindow(r.width(), r.height(), g_nPDAIncrement);
			} else {
				g_nPDAIncrement = PDA_INCREMENT;
				_pPDABmp->setPosInWindow(r.width(), r.height(), g_nPDAIncrement);
			}
			insertFGObjects(_pPDABmp);
			deactivatePDA();
			_pPDABmp->setVisible(true);
		}

		if ((CBagObject *)nullptr == getFGObjects(CBofString(PDA_WLD))) {
			_pPDABmp->setAssociateWnd(this);

			// To fix pda not updating problem
			insertFGObjects(_pPDABmp);
		}

		_pPDABmp->attachActiveObjects();

		// If this world file contains an evt_wld
		pSDev = g_SDevManager->getStorageDevice("EVT_WLD");
		if (pSDev != nullptr) {
			// Have we allocated one yet ?
			if (_pEvtSDev == nullptr) {
				_pEvtSDev = (CBagEventSDev *)pSDev;
				_pEvtSDev->setAssociateWnd(this);
				if (!_pEvtSDev->isAttached())
					_pEvtSDev->attach();

				setTimer(EVAL_EXPR, 1000);
				g_pauseTimerFl = false;

			} else {
				killTimer(EVAL_EXPR);

				// We already allocated one
				// we just need to re-associate the parent
				// window and reset the timer
				_pEvtSDev->setAssociateWnd(this);
				setTimer(EVAL_EXPR, 1000);

				g_pauseTimerFl = false;
			}
		}

		// If anyone can tell me why we redo a InsertFG here I'd
		// be interested in knowing since we already did one above...
		//
		// Only do it if we're coming from somewhere other than the zoom
		if ((CBagObject *)nullptr == getFGObjects(CBofString(WIELD_WLD))) {
			_pWieldBmp->setAssociateWnd(this);
			insertFGObjects(_pWieldBmp);
		}
	}

	onSize(0, tmpRect.width() - 1, tmpRect.height() - 1);

	// Set the first paint to true so the objects
	// won't run until the window is ready
	_bFirstPaint = true;

	attachActiveObjects();
	CBofSound::audioTask();

	show();

	onRender(_pBackdrop, nullptr);

	// Perform fade
	if ((_pBackdrop != nullptr) && getFadeId()) {
		int nFade = getFadeId() & 0x0F;

		switch (nFade) {
		case 1:
			_pBackdrop->fadeIn(this);
			break;

		case 2:
			_pBackdrop->curtain(this);
			break;

		case 3:
			_pBackdrop->fadeLines(this);
			break;

		default:
			break;
		}
	}

	if (_pBackdrop != nullptr)
		_pBackdrop->paint(this, 0, 0);


	return _errCode;
}


ErrorCode CMainWindow::detach() {
	// If this was a closeup then save the leaving position
	_cLastLoc = getViewPort().topLeft();

	CBagPanWindow::detach();

	unSetSlideBitmap();

	destroy();

	// When we move from room to room, we should delete
	// all our foreground objects (remove from memory).
	deleteFGObjects();

	return _errCode;
}


void CMainWindow::onSize(uint32 nType, int cx, int cy) {
	if (_pPDABmp) {
		if (getStretchToScreen()) {
			_pPDABmp->setPosInWindow(500, 370, g_nPDAIncrement);
		} else
			_pPDABmp->setPosInWindow(cx, cy, g_nPDAIncrement);
	}

	CBagPanWindow::onSize(nType, cx, cy);
}


ErrorCode CMainWindow::onCursorUpdate(int nCurrObj) {
	return CBagPanWindow::onCursorUpdate(nCurrObj);
}


void CMainWindow::onKeyHit(uint32 lKey, uint32 lRepCount) {
	// Terminate app on ALT_Q
	if ((lKey == BKEY_ALT_q) || (lKey == BKEY_ALT_F4)) {
		close();
		g_engine->quitGame();
	}

	if (lKey == BKEY_SCRL_LOCK) {               // Get a scroll lock hit
		if (getFilterId() == 0x08) {            // If we're in zzazzl filter
			_bZzazzlVision = !_bZzazzlVision; // toggle the paint zzazzl flag
			CBagVar *pVar = g_VarManager->getVariable("ZZAZZLVISION");

			if (pVar != nullptr) {
				pVar->setValue(_bZzazzlVision ? 1 : 0);
			}
		}
	} else {
		// Default action
		CBagPanWindow::onKeyHit(lKey, lRepCount);
	}
}


void CMainWindow::correctZzazzlePoint(CBofPoint *p) {
	// Don't correct this boy if he's inside the PDA.
	CBagPDA *pPDA = (CBagPDA *)g_SDevManager->getStorageDevice("BPDA_WLD");
	if (pPDA && pPDA->isInside(*p)) {
		return;
	}

	if (!getMovementRect().ptInRect(*p))
		return;

	int dx = _xFilterRect->width();
	int dy = _xFilterRect->height();
	int x = _xFilterRect->left;
	int y = _xFilterRect->top;
	CBofRect r(x + dx, y + dy, x + dx + dx, y + dy + dy);

	// Attempt to make all squares active in zzazzlvision
	int j;
	for (int i = 0; i < 3; ++i) {
		if (i == 1) {
			// Center row
			x = _xFilterRect->left;
			j = 0;
		} else {
			// Only two squares in top & bottom rows, start 1/2 a square to the left
			x = _xFilterRect->left + (dx >> 1);
			j = 1;
		}
		for (; j < 3; ++j) {
			r.setRect(x, y, x + dx, y + dy);
			if (r.ptInRect(*p)) {
				p->x = _xFilterRect->left + (p->x - r.left) * 3;
				p->y = _xFilterRect->top + (p->y - r.top) * 3;
				return;
			}
			x += dx;
		}
		y += dy;
	}
}


void CMainWindow::onMouseMove(uint32 nFlags, CBofPoint *pPoint, void *) {
	assert(isValidObject(this));
	assert(pPoint != nullptr);

	if (_cLastPos != *pPoint) {
		_cLastPos = *pPoint;

		if (getFilterId() & 0x08) {
			if (_bZzazzlVision)                // If zzazzl paint is toggled on
				correctZzazzlePoint(pPoint);
		}

		CBagPanWindow::onMouseMove(nFlags, pPoint);
	}
}


void CMainWindow::onLButtonUp(uint32 nFlags, CBofPoint *xPoint, void *) {
	if (getFilterId() & 0x08) {
		if (_bZzazzlVision)                // If zzazzl paint is toggled on
			correctZzazzlePoint(xPoint);
	}

	CBagPanWindow::onLButtonUp(nFlags, xPoint);
}


void CMainWindow::onLButtonDown(uint32 nFlags, CBofPoint *xPoint, void *) {
	if (getFilterId() & 0x08) {
		if (_bZzazzlVision)                // If zzazzl paint is toggled on
			correctZzazzlePoint(xPoint);
	}

	CBagPanWindow::onLButtonDown(nFlags, xPoint);
}

void CMainWindow::onClose() {
	CBagPanWindow::onClose();       // Destruct the main window
}


CBagObject *CMainWindow::onNewLinkObject(const CBofString &) {
	return new CMainLinkObject();
}


ErrorCode CMainWindow::setloadFilePos(const CBofPoint dstLoc) {
	if (dstLoc.x != 0 || dstLoc.y != 0) {
		_cLastLoc = dstLoc;
	}

	return _errCode;
}


// Set the ZzazzlVision variable.
void CMainWindow::setZzazzlVision(bool newValue) {
	_bZzazzlVision = newValue;
}

} // namespace SpaceBar
} // namespace Bagel
