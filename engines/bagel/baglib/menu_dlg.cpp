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

#include "bagel/baglib/menu_dlg.h"
#include "bagel/baglib/pan_window.h"
#include "bagel/baglib/text_object.h"
#include "bagel/baglib/bagel.h"
#include "bagel/baglib/master_win.h"
#include "bagel/baglib/zoom_pda.h"
#include "bagel/boflib/log.h"

namespace Bagel {

#define MENU_DFLT_HEIGHT 20

#define TIMER_CLOSE_ID   108
#define TIMER_HACK_ID    109
#define DELAY_DEFAULT      0

CBagObject *CBagMenuDlg::_pSelectedObject;
CBofList<CBagObject *> *CBagMenu::_pUniversalObjectList;   // Objects used in every menu
int CBagMenu::_nDefaultDelay;

bool g_pauseTimerFl = false;
extern bool g_bAAOk;

#define EXAMINEBMP          "$SBARDIR\\GENERAL\\MENUS\\EXAMINE.BMP"

void CBagMenuDlg::initialize() {
	_pSelectedObject = nullptr;
}

void CBagMenu::initialize() {
	_pUniversalObjectList = nullptr;
	_nDefaultDelay = DELAY_DEFAULT;
}

CBagMenu::CBagMenu() {
	_nY = 0;

	static bool bFirstTime = true;

	// Get option for delay for caption boxes
	if (bFirstTime) {
		bFirstTime = false;

		_nDefaultDelay = DELAY_DEFAULT;

		CBagel *pBagel = (CBagel *)CBagel::getApp();
		if (pBagel != nullptr) {
			pBagel->getOption("UserOptions", "TextDelay", &_nDefaultDelay, DELAY_DEFAULT);

			if (_nDefaultDelay < 0) {
				_nDefaultDelay = -1;
			} else if (_nDefaultDelay > 0x7FFF) {
				_nDefaultDelay = 0x7FFF;
			}
		}
	}

	_nDelay = (int16)_nDefaultDelay;
}

CBagObject *CBagMenu::onNewSpriteObject(const CBofString &) {
	CBagSpriteObject *pObj = new CBagSpriteObject();

	const CBofPoint pt(0, _nY);
	pObj->setPosition(pt);
	pObj->setTransparent(false);

	return pObj;
}

bool CBagMenu::trackPopupMenu(uint32 /*nFlags*/, int x, int y, CBofWindow *pWnd, CBofPalette * /*pPal*/, CBofRect * /*lpRect*/) {
	static int nNumCalls = 0;
	CBagMenuDlg dlg;
	CBagObject *pObj;
	int nObjectPal = -1;
	int nMenuCount = 0, nRunItems = 0, nBaseMenuLocX = 3;
	CBofList<CBagObject *>  xObjList;
	int nNumItems = 0;
	CBofRect wndRect(80, 10, 80 + 480 - 1, 10 + 360 - 1);
	CBofRect objSize;
	CBofSize cWieldMenuSize;
	CBofSize menuSize(1, 1);
	CBofPoint menuLoc(4, 1);
	bool bTextOnly = true;
	int tmpVal = 0;
	CBofPoint cMouseDown(x, y);
	bool bZoomed = false;

	nNumCalls++;

	bool bReturn = true;
	CBofWindow *pParent = pWnd;
	int nNumWieldChoices = 0;

	if ((getObjectList()->getCount() == 1) && (getObjectList()->getTail()->getNodeItem()->getType() == TEXT_OBJ) && (((CBagTextObject *)getObjectList()->getTail()->getNodeItem())->isCaption())) {
		nBaseMenuLocX = 0;

	} else if (nNumCalls == 1 && _pUniversalObjectList && _pUniversalObjectList != getObjectList()) {
		for (int i = 0; i < _pUniversalObjectList->getCount(); ++i) {

			pObj = _pUniversalObjectList->getNodeItem(i);

			if (pObj->isLocal() && (!pObj->getExpression() || pObj->getExpression()->evaluate(pObj->isNegative()))) {
				// Only attach if not attached
				if (pObj->isAttached() == false) {
					pObj->attach();

					// Otherwise, we need to re-calculate the size of the text object,
					// since we are gonna trash is with our own values soon.

				} else if (pObj->getType() == TEXT_OBJ) {
					((CBagTextObject *)pObj)->recalcTextRect(((CBagTextObject *)pObj)->isCaption());
				}

				if (!pObj->isImmediateRun()) {
					// Get the next menu items pos
					objSize = pObj->getRect();

					if (menuSize.cx < (objSize.width() + menuLoc.x))
						menuSize.cx = (objSize.width() + menuLoc.x);
					if (menuSize.cy < (objSize.height() + menuLoc.y))
						menuSize.cy = (objSize.height() + menuLoc.y);

					pObj->setPosition(menuLoc);
					pObj->setHighlight(false);

					if (!nMenuCount && (pObj->getType() == TEXT_OBJ)) {
						menuLoc.y += objSize.height();
					} else {
						menuLoc.x += objSize.width();
					}

					xObjList.addToTail(pObj);

					nMenuCount++;
				} else {
					nRunItems++;
					pObj->runObject();

					// This detach may cause problems in the future, if it does delete it
					// Some object may not work if detached for example midi sound
					pObj->detach();
				}
			}
		}

		// Start non-wield menu on next row
		menuLoc.y += objSize.height();

		nNumWieldChoices = xObjList.getCount();

		if (nNumWieldChoices != 0) {
			cWieldMenuSize = menuSize;
		}
	}

	bool bNoMenu = false;

	for (int i = 0; i < getObjectList()->getCount(); ++i) {

		pObj = getObjectList()->getNodeItem(i);

		if (pObj->isLocal() && (!pObj->getExpression() || pObj->getExpression()->evaluate(pObj->isNegative()))) {
			// Only attach if not attached
			if (pObj->isAttached() == false) {
				pObj->attach();

				// Otherwise, we need to re-calculate the size of the text object,
				// since we are gonna trash is with our own values soon.

			} else if (pObj->getType() == TEXT_OBJ) {
				((CBagTextObject *)pObj)->recalcTextRect(((CBagTextObject *)pObj)->isCaption());
			}

			if (!pObj->isImmediateRun()) {
				// Get the next menu items pos
				objSize = pObj->getRect();

				// If it is a text object increment next position by its height
				if (pObj->getType() == TEXT_OBJ) {

					if (tmpVal)
						// If we have a value move text to next line
						menuLoc.y += tmpVal;
					tmpVal = 0; // Text objects set the next line to be at the very beginning

					// If we're zoomed, then do things differently
					CBagTextObject *pTXObj = (CBagTextObject *)pObj;
					if (pTXObj->isCaption()) {
						SBZoomPda *pZPDA = (SBZoomPda *)g_SDevManager->getStorageDevice("BPDAZ_WLD");
						if (pZPDA && pZPDA->getZoomed()) {
							bZoomed = true;

							wndRect = pZPDA->getViewRect();
						}
					}

					if (bTextOnly) {
						if (wndRect.height() <= (objSize.height() + menuLoc.y)) {
							menuLoc.y = 1;
							nBaseMenuLocX += (menuSize.cx + 2);
							menuLoc.x = nBaseMenuLocX + 1;
						}
					} else if (wndRect.height() <= ((objSize.height() + menuLoc.y) + 41)) {
							menuLoc.y = 1;
							nBaseMenuLocX += (menuSize.cx + 2);
							menuLoc.x = nBaseMenuLocX;
					}

					// CHECKME: the previous assignment seems to indicate that the following line should be removed in order to keep this slightly different value
					menuLoc.x = (1 + nBaseMenuLocX);
					if (menuSize.cx < (objSize.width() + menuLoc.x))
						menuSize.cx = (objSize.width() + menuLoc.x);
					if (menuSize.cy < (objSize.height() + menuLoc.y))
						menuSize.cy = (objSize.height() + menuLoc.y);
					pObj->setPosition(menuLoc);
					pObj->setHighlight(false);

					menuLoc.x = (1 + nBaseMenuLocX);
					menuLoc.y += (objSize.height() + 1);
				} else {
					// Increment next position by its width
					if (wndRect.height() <= (objSize.height() + menuLoc.y)) {
						menuLoc.y = 1;
						nBaseMenuLocX += (menuSize.cx + 2);
						menuLoc.x = nBaseMenuLocX;
					}

					bTextOnly = false;

					// Continue to grow menu size to max required
					if (menuSize.cx < (objSize.width() + menuLoc.x))
						menuSize.cx = (objSize.width() + menuLoc.x);
					if (menuSize.cy < (objSize.height() + menuLoc.y))
						menuSize.cy = (objSize.height() + menuLoc.y);

					pObj->setPosition(menuLoc);
					pObj->setHighlight(false);

					if ((nObjectPal < 0) && ((pObj->getType() == BMP_OBJ) || (pObj->getType() == SPRITE_OBJ)))
						nObjectPal = i;
					menuLoc.x += objSize.width();
					tmpVal = objSize.height();//save the obj height for use later if we get a text obj
				}

				if (pObj->isNoMenu()) {
					bNoMenu = true;
				}

				xObjList.addToTail(pObj);
				nNumItems++;

				nMenuCount++;

			} else {
				nRunItems++;
				pObj->runObject();
				if (pObj->getType() == LINK_OBJ) {
					g_bAAOk = false;
				}
			}
		} // If in local area and activated by expression
	}

	// If we ran something and there are no other menu items just return
	if (!(nRunItems && !nMenuCount)) {
		if (nMenuCount) {
			menuLoc.y += objSize.height();
			menuLoc.x = 1;
			nMenuCount = 0;
		}

		menuSize.cx++;
		menuSize.cy++;

		CBofRect tmpRect(CBofPoint(x, y - menuSize.cy / 2), menuSize);
		menuSize.cy += 2;

		// If the menu contains only one object and it is a caption style text object
		// position the dialog box at the bottom of the Game window screen
		bool bMoved = false;
		bool bCaption = false;
		int nNumChars = 0;
		if ((nNumItems == 1) && (xObjList.getTail()->getNodeItem()->getType() == TEXT_OBJ) && (((CBagTextObject *)xObjList.getTail()->getNodeItem())->isCaption())) {
			while (nNumWieldChoices-- != 0) {
				pObj = xObjList.removeHead();
				pObj->detach();
			}

			// If we are not using the wield menu with this menu, then
			// the menu is smaller
			menuSize.cy -= cWieldMenuSize.cy;
			if (cWieldMenuSize.cy != 0)
				cWieldMenuSize.cy--;

			bMoved = true;

			tmpRect = wndRect;

			tmpRect.top = tmpRect.bottom - menuSize.cy;
			bCaption = true;

			nNumChars = ((CBagTextObject *)xObjList.getHead()->getNodeItem())->getText().getLength();

			// Bring caption back 3 pixels
			CBofPoint cPoint;
			cPoint = xObjList.getHead()->getNodeItem()->getPosition();
			cPoint.x = 1;

			xObjList.getHead()->getNodeItem()->setPosition(cPoint);

		} else {
			if (bTextOnly) {
				while (nNumWieldChoices-- != 0) {
					pObj = xObjList.removeHead();
					pObj->detach();
				}

				// If we are not using the wield menu with this menu, then
				// the menu is smaller
				menuSize.cy -= cWieldMenuSize.cy;
				bMoved = true;
			}

			// Use the mouse pos (x,y), and add the menuSize calculated above
			tmpRect.left = x;
			tmpRect.top = y;
			tmpRect.bottom = y + menuSize.cy;
			tmpRect.right = x + menuSize.cx + 1;

			// If the menu would go off the screen, adjust it
			while (tmpRect.top < wndRect.top) {
				tmpRect.offsetRect(0, (wndRect.top - tmpRect.top));
			}
			while (tmpRect.bottom > wndRect.bottom) {
				tmpRect.offsetRect(0, (wndRect.bottom - tmpRect.bottom));
			}
			while (tmpRect.left < wndRect.left) {
				tmpRect.right += (wndRect.left - tmpRect.left);
				tmpRect.left += (wndRect.left - tmpRect.left);
			}
			while (tmpRect.right > wndRect.right) {
				tmpRect.left -= (tmpRect.right - wndRect.right);
				tmpRect.right -= (tmpRect.right - wndRect.right);
			}
		}

		bReturn = false;
		if (xObjList.getCount() && !bNoMenu) {
			bReturn = true;

			dlg.setObjectList(&xObjList);

			for (int i = 0; i < xObjList.getCount(); i++) {
				pObj = xObjList[i];

				if (pObj->getType() == TEXT_OBJ) {
					int cx = tmpRect.size().cx - 1;
					int cy = tmpRect.size().cy - 1;
					if (!bCaption) {
						cy = ((CBagTextObject *)pObj)->getSize().cy + 2;
					}

					pObj->setSize(CBofSize(cx, cy));

					// Need to move the menus up when have a wielded item, but
					// not displaying the wield menu
					if (bMoved) {
						CBofPoint cPos;
						cPos = pObj->getPosition();
						cPos.y -= cWieldMenuSize.cy;
						pObj->setPosition(cPos);
					}
				}
			}

			CBagPanWindow::flushInputEvents();

			// If we were requested to put a dialog over the PDA, then shift it upward
			// a bit... unless of course the mousedown occurred in the PDA itself.
			CBagPDA *pPDA = (CBagPDA *)g_SDevManager->getStorageDevice("BPDA_WLD");

			if (pPDA != nullptr && (pPDA->isActivated() && bZoomed == false)) {
				if (!pPDA->isInside(cMouseDown)) {
					CBofRect cPDARect = pPDA->getRect();

					tmpRect.offsetRect(0, (tmpRect.bottom > cPDARect.top ? cPDARect.top - tmpRect.bottom : 0));

					// Make sure we didn't go too far
					if (tmpRect.top < 0) {
						tmpRect.offsetRect(0, -tmpRect.top);
					}
				}
			}

			// Force all menus to be created using a specific palette
			// that we know contains the correct colors for our menu.

			char szBuf[256];
			Common::strcpy_s(szBuf, EXAMINEBMP);
			CBofString cString(szBuf, 256);
			fixPathName(cString);
			CBofPalette      xPal;

			xPal.loadPalette(cString);
			dlg.createDlg(pParent, &xPal, &tmpRect);

			if (bCaption) {
				// We need to move this back to the correct position
				// because the Boflibs center dialog ALWAYS !!!!!!!!

				// If the PDA is currently active, then we have to put the caption elsewhere,
				// or deactivate the PDA (if PDA zoomed, just place at bottom.
				if (CBagPanWindow::_pPDABmp != nullptr && CBagPanWindow::_pPDABmp->isActivated() && bZoomed == false) {
					dlg.move(wndRect.left, wndRect.top, true);
				} else {
					dlg.move(tmpRect.left, tmpRect.top, true);
				}

				// Need to re-save the background because there is a bug with moving
				// the dialog box.
				dlg.saveBackground();

				// Set this caption to automatically go away after specified delay
				if (_nDelay > 0) {
					dlg.setTimer(TIMER_CLOSE_ID, _nDelay + (80 * nNumChars));
				}
			} else {
				dlg.move(tmpRect.topLeft().x, tmpRect.topLeft().y, true);

				// Need to re-save the background because there is a bug with moving
				// the dialog box.
				dlg.saveBackground();
			}

			CBagPanWindow::flushInputEvents();

			g_pauseTimerFl = true;
			dlg.doModal();
			g_pauseTimerFl = false;

			pObj = dlg._pSelectedObject;
			dlg.destroy();

			for (int i = 0; i < getObjectCount(); ++i) {
				getObjectByPos(i)->detach();
			}

			// If our current storage device is "AS CUSTOM" then don't allow
			// the timer to get incremented.
			CBagStorageDev *pCurSDEV = CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev();
			if (pObj != nullptr) {
				pObj->runCallBack();

				// Selecting this menu item causes a turn to go by
				if (nNumCalls == 1 && pCurSDEV->isCustom() == false) {
					g_VarManager->incrementTimers();
				}
			} else if (bCaption && (nNumCalls == 2)) {
				// Selecting this menu item causes a turn to go by
				dlg._pSelectedObject = nullptr;
				if (pCurSDEV->isCustom() == false) {
					g_VarManager->incrementTimers();
				}
			}
		}
	}

	nNumCalls--;

	return bReturn;
}

bool CBagMenu::addItem(CBagObject *pObj, void *( * /*func*/)(int, void *), void * /*info*/) {
	pObj->setPosition(CBofPoint(0, _nY));

	_nY = (int16)(_nY + (int16)(pObj->getRect().height() + 1));
	addObject(pObj);

	return true;
}

bool CBagMenu::deleteItem(const CBofString & /*sLabel*/) {
	return false;
}

bool CBagMenu::isChecked(const CBofString & /*sLabel*/, const CBofString & /*sSubLabel*/) {
	const int nRow = 0;
	const int nCol = 0;

	return isCheckedPos(nRow, nCol);
}

bool CBagMenu::isCheckedPos(int /*nRow*/, int /*nCol*/) {
	return false;
}

bool CBagMenu::isChecked(int /*nRefId*/) {
	return false;
}

bool CBagMenu::check(const CBofString & /*sLabel*/, const CBofString & /*sSubLabel*/) {
	return false;
}

bool CBagMenu::unCheck(const CBofString & /*sLabel*/, const CBofString & /*sSubLabel*/) {
	return false;
}

bool CBagMenu::check(int /*nRefId*/) {
	return true;
}

bool CBagMenu::unCheck(int /*nRefId*/) {
	return true;
}

bool CBagMenu::setUniversalObjectList(CBofList<CBagObject *> *pObjList) {
	if (_pUniversalObjectList != nullptr) {
		removeUniversalObjectList();
	}

	_pUniversalObjectList = pObjList;

	return true;
}

bool CBagMenu::removeUniversalObjectList() {
	if (_pUniversalObjectList == nullptr) {
		return true;
	}

	_pUniversalObjectList = nullptr;

	return true;
}

//
//
//

CBagMenuDlg::CBagMenuDlg() {
	// Remove this SDEV from the storage device list so that it is not deleted
	// when we switch .WLD files, and there may still be a Dialog open.
	g_SDevManager->unregisterStorageDev(this);

	_bAcceptInput = false;
	_bMultipleDialogs = false;
}

CBagMenuDlg::~CBagMenuDlg() {
	assert(isValidObject(this));
}

ErrorCode CBagMenuDlg::createDlg(CBofWindow *pWnd, CBofPalette *pPal, CBofRect *pRect) {
	CBofRect r;

	_bMultipleDialogs = false;
	_bAcceptInput = true;
	_nReturnValue = 0;

	if (!pRect) {
		r = pWnd->getWindowRect();
		r.offsetRect(-r.left, -r.top);
		r.bottom = r.top + MENU_DFLT_HEIGHT;
	} else {
		r = *pRect;
	}

	CBagStorageDevDlg::create("Menu", &r, pWnd, 0);

	CBofBitmap *pBmp = new CBofBitmap(r.width(), r.height(), pPal);

	r.offsetRect(-r.left, -r.top);
	assert(pPal != nullptr);
	pBmp->fillRect(&r, pPal->getNearestIndex(RGB(82, 82, 82)));
	pBmp->drawRect(&r, pPal->getNearestIndex(RGB(0, 0, 0)));
	setBackdrop(pBmp);

	return _errCode;
}


void CBagMenuDlg::onLButtonUp(uint32 nFlags, CBofPoint *pPoint, void *) {
	// We are ignoring all input until the dialog is actually visible
	if (_bAcceptInput) {
		_pSelectedObject = nullptr;

		onClose();

		if (_bMultipleDialogs) {
			CBofRect r = getWindowRect();
			r.offsetRect(-r.left, -r.top);
			if (r.ptInRect(*pPoint)) {
				CBagStorageDevDlg::onLButtonUp(nFlags, pPoint);
				_pSelectedObject = getLActiveObject();
			}

		} else {
			const CBofPoint pt = devPtToViewPort(*pPoint);
			_pSelectedObject = getObject(pt);
			if (_pSelectedObject != nullptr) {
				_pSelectedObject->onLButtonUp(nFlags, pPoint);
			}
		}

		_nReturnValue = (_pSelectedObject != nullptr);
	}
}

void CBagMenuDlg::onMouseMove(uint32 /*nFlags*/, CBofPoint *pPoint, void *) {
	CBagMasterWin::setActiveCursor(0);
	CBagObject *pObj = getObject(*pPoint);
	if (pObj != nullptr) {

		// Switch to that cursor
		CBagMasterWin::setActiveCursor(pObj->getOverCursor());

		if (pObj != getLActiveObject()) {
			if (pObj->getCallBack() || pObj->getMenuPtr()) {

				pObj->setHighlight();
				if (getLActiveObject())
					getLActiveObject()->setHighlight(false);
				setLActiveObject(pObj);
			}
		}
	}
}

void CBagMenuDlg::onTimer(uint32 nID) {
	assert(isValidObject(this));

	switch (nID) {
	// Auto close for text-Captions
	case TIMER_CLOSE_ID:
		killTimer(nID);
		close();
		break;

	// Can now allow user input
	case TIMER_HACK_ID:
		killTimer(nID);
		_bAcceptInput = true;
		break;

	default:
		logWarning("Invalid Timer ID");
		break;
	}
}

void CBagMenuDlg::onPaint(CBofRect *pRect) {
	assert(isValidObject(this));

	CBagStorageDevDlg::onPaint(pRect);

	// Don't allow user input until this menu is visible
	CBagPanWindow::flushInputEvents();
}

void CBagMenuDlg::onDeActivate() {
	assert(isValidObject(this));

	close();
}

} // namespace Bagel
