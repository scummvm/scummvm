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

CBagObject *CBagMenuDlg::m_pSelectedObject;
CBofList<CBagObject *> *CBagMenu::m_pUniversalObjectList;   // Objects used in every menu
int CBagMenu::m_nDefaultDelay;

bool g_bPauseTimer = false;
extern bool g_bAAOk;

#define EXAMINEBMP          "$SBARDIR\\GENERAL\\MENUS\\EXAMINE.BMP"

void CBagMenuDlg::initialize() {
	m_pSelectedObject = nullptr;
}

void CBagMenu::initialize() {
	m_pUniversalObjectList = nullptr;
	m_nDefaultDelay = DELAY_DEFAULT;
}

CBagMenu::CBagMenu() {
	m_nY = 0;

	static bool bFirstTime = true;

	// Get option for delay for caption boxes
	if (bFirstTime) {
		bFirstTime = false;

		m_nDefaultDelay = DELAY_DEFAULT;

		CBagel *pBagel = (CBagel *)CBagel::getApp();
		if (pBagel != nullptr) {
			pBagel->getOption("UserOptions", "TextDelay", &m_nDefaultDelay, DELAY_DEFAULT);

			if (m_nDefaultDelay < 0) {
				m_nDefaultDelay = -1;
			} else if (m_nDefaultDelay > 0x7FFF) {
				m_nDefaultDelay = 0x7FFF;
			}
		}
	}

	m_nDelay = (int16)m_nDefaultDelay;
}

CBagObject *CBagMenu::OnNewSpriteObject(const CBofString &) {
	CBagSpriteObject *pObj = new CBagSpriteObject();

	CBofPoint pt(0, m_nY);
	pObj->setPosition(pt);
	pObj->SetTransparent(false);

	return pObj;
}

bool CBagMenu::TrackPopupMenu(uint32 /*nFlags*/, int x, int y, CBofWindow *pWnd, CBofPalette * /*pPal*/, CBofRect * /*lpRect*/) {
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

	if ((GetObjectList()->GetCount() == 1) && (GetObjectList()->GetTail()->GetNodeItem()->GetType() == TEXTOBJ) && (((CBagTextObject *)GetObjectList()->GetTail()->GetNodeItem())->IsCaption())) {
		nBaseMenuLocX = 0;

	} else if (nNumCalls == 1 && m_pUniversalObjectList && m_pUniversalObjectList != GetObjectList()) {
		for (int i = 0; i < m_pUniversalObjectList->GetCount(); ++i) {

			pObj = m_pUniversalObjectList->GetNodeItem(i);

			if (pObj->IsLocal() && (!pObj->GetExpression() || pObj->GetExpression()->evaluate(pObj->IsNegative()))) {
				// Only attach if not attached
				if (pObj->isAttached() == false) {
					pObj->attach();

					// Otherwise, we need to re-calculate the size of the text object,
					// since we are gonna trash is with our own values soon.

				} else if (pObj->GetType() == TEXTOBJ) {
					((CBagTextObject *)pObj)->RecalcTextRect(((CBagTextObject *)pObj)->IsCaption());
				}

				if (!pObj->IsImmediateRun()) {
					// Get the next menu items pos
					objSize = pObj->getRect();

					if (menuSize.cx < (objSize.width() + menuLoc.x))
						menuSize.cx = (objSize.width() + menuLoc.x);
					if (menuSize.cy < (objSize.height() + menuLoc.y))
						menuSize.cy = (objSize.height() + menuLoc.y);

					pObj->setPosition(menuLoc);
					pObj->setHighlight(false);

					if (!nMenuCount && (pObj->GetType() == TEXTOBJ)) {
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

		nNumWieldChoices = xObjList.GetCount();

		if (nNumWieldChoices != 0) {
			cWieldMenuSize = menuSize;
		}
	}

	bool bNoMenu = false;

	for (int i = 0; i < GetObjectList()->GetCount(); ++i) {

		pObj = GetObjectList()->GetNodeItem(i);

		if (pObj->IsLocal() && (!pObj->GetExpression() || pObj->GetExpression()->evaluate(pObj->IsNegative()))) {
			// Only attach if not attached
			if (pObj->isAttached() == false) {
				pObj->attach();

				// Otherwise, we need to re-calculate the size of the text object,
				// since we are gonna trash is with our own values soon.

			} else if (pObj->GetType() == TEXTOBJ) {
				((CBagTextObject *)pObj)->RecalcTextRect(((CBagTextObject *)pObj)->IsCaption());
			}

			if (!pObj->IsImmediateRun()) {
				// Get the next menu items pos
				objSize = pObj->getRect();

				// If it is a text object increment next position by its height
				if (pObj->GetType() == TEXTOBJ) {

					if (tmpVal)
						// If we have a value move text to next line
						menuLoc.y += tmpVal;
					tmpVal = 0; // Text objects set the next line to be at the very beginning

					// If we're zoomed, then do things differently
					CBagTextObject *pTXObj = (CBagTextObject *)pObj;
					if (pTXObj->IsCaption()) {
						SBZoomPda *pZPDA = (SBZoomPda *)SDEV_MANAGER->GetStorageDevice("BPDAZ_WLD");
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

					if ((nObjectPal < 0) && ((pObj->GetType() == BMPOBJ) || (pObj->GetType() == SPRITEOBJ)))
						nObjectPal = i;
					menuLoc.x += objSize.width();
					tmpVal = objSize.height();//save the obj height for use later if we get a text obj
				}

				if (pObj->IsNoMenu()) {
					bNoMenu = true;
				}

				xObjList.addToTail(pObj);
				nNumItems++;

				nMenuCount++;

			} else {
				nRunItems++;
				pObj->runObject();
				if (pObj->GetType() == LINKOBJ) {
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
		if ((nNumItems == 1) && (xObjList.GetTail()->GetNodeItem()->GetType() == TEXTOBJ) && (((CBagTextObject *)xObjList.GetTail()->GetNodeItem())->IsCaption())) {
			while (nNumWieldChoices-- != 0) {
				pObj = xObjList.RemoveHead();
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

			nNumChars = ((CBagTextObject *)xObjList.GetHead()->GetNodeItem())->getText().GetLength();

			// Bring caption back 3 pixels
			CBofPoint cPoint;
			cPoint = xObjList.GetHead()->GetNodeItem()->getPosition();
			cPoint.x = 1;

			xObjList.GetHead()->GetNodeItem()->setPosition(cPoint);

		} else {
			if (bTextOnly) {
				while (nNumWieldChoices-- != 0) {
					pObj = xObjList.RemoveHead();
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
				tmpRect.OffsetRect(0, (wndRect.top - tmpRect.top));
			}
			while (tmpRect.bottom > wndRect.bottom) {
				tmpRect.OffsetRect(0, (wndRect.bottom - tmpRect.bottom));
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
		if (xObjList.GetCount() && !bNoMenu) {
			bReturn = true;

			dlg.SetObjectList(&xObjList);

			for (int i = 0; i < xObjList.GetCount(); i++) {
				pObj = xObjList[i];

				if (pObj->GetType() == TEXTOBJ) {
					int cx = tmpRect.Size().cx - 1;
					int cy = tmpRect.Size().cy - 1;
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

			CBagPanWindow::FlushInputEvents();

			// If we were requested to put a dialog over the PDA, then shift it upward
			// a bit... unless of course the mousedown occurred in the PDA itself.
			CBagPDA *pPDA = (CBagPDA *)SDEV_MANAGER->GetStorageDevice("BPDA_WLD");

			if (pPDA != nullptr && (pPDA->isActivated() && bZoomed == false)) {
				if (!pPDA->isInside(cMouseDown)) {
					CBofRect cPDARect = pPDA->getRect();

					tmpRect.OffsetRect(0, (tmpRect.bottom > cPDARect.top ? cPDARect.top - tmpRect.bottom : 0));

					// Make sure we didn't go too far
					if (tmpRect.top < 0) {
						tmpRect.OffsetRect(0, -tmpRect.top);
					}
				}
			}

			// Force all menus to be created using a specific palette
			// that we know contains the correct colors for our menu.

			char szBuf[256];
			Common::strcpy_s(szBuf, EXAMINEBMP);
			CBofString cString(szBuf, 256);
			MACROREPLACE(cString);
			CBofPalette      xPal;

			xPal.loadPalette(cString);
			dlg.create(pParent, &xPal, &tmpRect);

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
				if (m_nDelay > 0) {
					dlg.setTimer(TIMER_CLOSE_ID, m_nDelay + (80 * nNumChars));
				}
			} else {
				dlg.move(tmpRect.TopLeft().x, tmpRect.TopLeft().y, true);

				// Need to re-save the background because there is a bug with moving
				// the dialog box.
				dlg.saveBackground();
			}

			CBagPanWindow::FlushInputEvents();

			g_bPauseTimer = true;
			dlg.doModal();
			g_bPauseTimer = false;

			pObj = dlg.m_pSelectedObject;
			dlg.destroy();

			for (int i = 0; i < GetObjectCount(); ++i) {
				GetObjectByPos(i)->detach();
			}

			// If our current storage device is "AS CUSTOM" then don't allow
			// the timer to get incremented.
			CBagStorageDev *pCurSDEV = CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev();
			if (pObj != nullptr) {
				pObj->runCallBack();

				// Selecting this menu item causes a turn to go by
				if (nNumCalls == 1 && pCurSDEV->IsCustom() == false) {
					VAR_MANAGER->IncrementTimers();
				}
			} else if (bCaption && (nNumCalls == 2)) {
				// Selecting this menu item causes a turn to go by
				dlg.m_pSelectedObject = nullptr;
				if (pCurSDEV->IsCustom() == false) {
					VAR_MANAGER->IncrementTimers();
				}
			}
		}
	}

	nNumCalls--;

	return bReturn;
}

bool CBagMenu::AddItem(CBagObject *pObj, void *( * /*func*/)(int, void *), void * /*info*/) {
	pObj->setPosition(CBofPoint(0, m_nY));

	m_nY = (int16)(m_nY + (int16)(pObj->getRect().height() + 1));
	AddObject(pObj);

	return true;
}

bool CBagMenu::DeleteItem(const CBofString & /*sLabel*/) {
	return false;
}

bool CBagMenu::IsChecked(const CBofString & /*sLabel*/, const CBofString & /*sSubLabel*/) {
	int nRow = 0;
	int nCol = 0;

	return IsCheckedPos(nRow, nCol);
}

bool CBagMenu::IsCheckedPos(int /*nRow*/, int /*nCol*/) {
	return false;
}

bool CBagMenu::IsChecked(int /*nRefId*/) {
	return false;
}

bool CBagMenu::Check(const CBofString & /*sLabel*/, const CBofString & /*sSubLabel*/) {
	return false;
}

bool CBagMenu::UnCheck(const CBofString & /*sLabel*/, const CBofString & /*sSubLabel*/) {
	return false;
}

bool CBagMenu::Check(int /*nRefId*/) {
	return true;
}

bool CBagMenu::UnCheck(int /*nRefId*/) {
	return true;
}

bool CBagMenu::SetUniversalObjectList(CBofList<CBagObject *> *pObjList) {
	if (m_pUniversalObjectList != nullptr) {
		RemoveUniversalObjectList();
	}

	m_pUniversalObjectList = pObjList;

	return true;
}

bool CBagMenu::RemoveUniversalObjectList() {
	if (m_pUniversalObjectList == nullptr) {
		return true;
	}

	m_pUniversalObjectList = nullptr;

	return true;
}

//
//
//

CBagMenuDlg::CBagMenuDlg() {
	// Remove this SDEV from the storage device list so that it is not deleted
	// when we switch .WLD files, and there may still be a Dialog open.
	SDEV_MANAGER->UnRegisterStorageDev(this);
}

CBagMenuDlg::~CBagMenuDlg() {
	Assert(IsValidObject(this));
}

ErrorCode CBagMenuDlg::create(CBofWindow *pWnd, CBofPalette *pPal, const CBofRect *pRect, uint32 /*nStyle*/) {
	CBofRect r;

	m_bMultipleDialogs = false;
	m_bAcceptInput = true;
	_nReturnValue = 0;

	if (!pRect) {
		r = pWnd->getWindowRect();
		r.OffsetRect(-r.left, -r.top);
		r.bottom = r.top + MENU_DFLT_HEIGHT;
	} else {
		r = *pRect;
	}

	CBagStorageDevDlg::create("Menu", &r, pWnd, 0);

	CBofBitmap *pBmp = new CBofBitmap(r.width(), r.height(), pPal);
	if (pBmp != nullptr) {
		r.OffsetRect(-r.left, -r.top);
		Assert(pPal != nullptr);
		pBmp->fillRect(&r, pPal->getNearestIndex(RGB(82, 82, 82)) /*RGB(0,0,0)*/);
		pBmp->drawRect(&r, pPal->getNearestIndex(RGB(0, 0, 0)) /*RGB(0,0,0)*/);
		setBackdrop(pBmp);
	}

	return _errCode;
}


void CBagMenuDlg::onLButtonUp(uint32 nFlags, CBofPoint *pPoint, void *) {
	// We are ignoring all input until the dialog is actually visible
	if (m_bAcceptInput) {
		m_pSelectedObject = nullptr;

		onClose();

		if (m_bMultipleDialogs) {
			CBofRect r = getWindowRect();
			r.OffsetRect(-r.left, -r.top);
			if (r.PtInRect(*pPoint)) {
				CBagStorageDevDlg::onLButtonUp(nFlags, pPoint);
				m_pSelectedObject = GetLActiveObject();
			}

		} else {
			CBofPoint pt = DevPtToViewPort(*pPoint);
			m_pSelectedObject = GetObject(pt);
			if (m_pSelectedObject != nullptr) {
				m_pSelectedObject->onLButtonUp(nFlags, pPoint);
			}
		}

		_nReturnValue = (m_pSelectedObject != nullptr);
	}
}

void CBagMenuDlg::onMouseMove(uint32 /*nFlags*/, CBofPoint *pPoint, void *) {
	CBagMasterWin::setActiveCursor(0);
	CBagObject *pObj = GetObject(*pPoint);
	if (pObj != nullptr) {

		// Switch to that cursor
		CBagMasterWin::setActiveCursor(pObj->GetOverCursor());

		if (pObj != GetLActiveObject()) {
			if (pObj->getCallBack() || pObj->getMenuPtr()) {

				pObj->setHighlight();
				if (GetLActiveObject())
					GetLActiveObject()->setHighlight(false);
				SetLActiveObject(pObj);
			}
		}
	}
}

void CBagMenuDlg::onTimer(uint32 nID) {
	Assert(IsValidObject(this));

	switch (nID) {
	// Auto close for text-Captions
	case TIMER_CLOSE_ID:
		killTimer(nID);
		close();
		break;

	// Can now allow user input
	case TIMER_HACK_ID:
		killTimer(nID);
		m_bAcceptInput = true;
		break;

	default:
		LogWarning("Invalid Timer ID");
		break;
	}
}

void CBagMenuDlg::onPaint(CBofRect *pRect) {
	Assert(IsValidObject(this));

	CBagStorageDevDlg::onPaint(pRect);

	// Don't allow user input until this menu is visible
	CBagPanWindow::FlushInputEvents();
}

void CBagMenuDlg::onDeActivate() {
	Assert(IsValidObject(this));

	close();
}

} // namespace Bagel
