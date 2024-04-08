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
#include "bagel/baglib/zoom_pda.h"

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
	//
	if (bFirstTime) {
		bFirstTime = false;

		CBagel *pBagel;

		m_nDefaultDelay = DELAY_DEFAULT;
		if ((pBagel = (CBagel *)CBagel::GetApp()) != nullptr) {
			pBagel->GetOption("UserOptions", "TextDelay", &m_nDefaultDelay, DELAY_DEFAULT);

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
	pObj->SetPosition(pt);
	pObj->SetTransparent(false);

	return pObj;
}

bool CBagMenu::TrackPopupMenu(uint32 /*nFlags*/, int x, int y, CBofWindow *pWnd, CBofPalette * /*pPal*/, CBofRect * /*lpRect*/) {
	static int nNumCalls = 0;
	CBofWindow *pParent;
	CBagMenuDlg dlg;
	CBagObject *pObj;
	int nObjectPal = -1;
	int i, nMenuCount = 0, nRunItems = 0, nBaseMenuLocX = 3;
	CBofList<CBagObject *>  xObjList;
	int nNumItems = 0;
	CBofRect wndRect(80, 10, 80 + 480 - 1, 10 + 360 - 1);
	CBofRect objSize;
	CBofSize cWieldMenuSize;
	CBofSize menuSize(1, 1);
	CBofPoint menuLoc(4, 1);
	bool bCaption = false;
	bool bTextOnly = true;
	bool bReturn;
	int tmpVal = 0;
	int nNumChars;
	int nNumWieldChoices;
	CBofPoint cMouseDown(x, y);
	bool bZoomed = false;

	nNumCalls++;

	bReturn = true;

	pParent = pWnd;

	nNumChars = 0;

	nNumWieldChoices = 0;

	if ((GetObjectList()->GetCount() == 1) && (GetObjectList()->GetTail()->GetNodeItem()->GetType() == TEXTOBJ) && (((CBagTextObject *)GetObjectList()->GetTail()->GetNodeItem())->IsCaption())) {
		nBaseMenuLocX = 0;

	} else {
		if (nNumCalls == 1 && m_pUniversalObjectList && m_pUniversalObjectList != GetObjectList()) {
			for (i = 0; i < m_pUniversalObjectList->GetCount(); ++i) {

				pObj = m_pUniversalObjectList->GetNodeItem(i);

				if (pObj->IsLocal() && (!pObj->GetExpression() || pObj->GetExpression()->Evaluate(pObj->IsNegative()))) {
					// Only attach if not attached
					if (pObj->IsAttached() == false) {
						pObj->Attach();

						// Otherwise, we need to re-calculate the size of the text object,
						// since we are gonna trash is with our own values soon.

					} else {
						if (pObj->GetType() == TEXTOBJ) {
							((CBagTextObject *)pObj)->RecalcTextRect(((CBagTextObject *)pObj)->IsCaption());
						}
					}

					if (!pObj->IsImmediateRun()) {
						// Get the next menu items pos
						objSize = pObj->GetRect();

						if (menuSize.cx < (objSize.Width() + menuLoc.x))
							menuSize.cx = (objSize.Width() + menuLoc.x);
						if (menuSize.cy < (objSize.Height() + menuLoc.y))
							menuSize.cy = (objSize.Height() + menuLoc.y);

						pObj->SetPosition(menuLoc);
						pObj->SetHighlight(false);

						if (!nMenuCount && (pObj->GetType() == TEXTOBJ)) {
							menuLoc.y += objSize.Height();
						} else {
							menuLoc.x += objSize.Width();
						}

						xObjList.AddToTail(pObj);

						nMenuCount++;
					} else {
						nRunItems++;
						pObj->RunObject();

						// This detach may cause problems in the future, if it does delete it
						// Some object may not work if detached for example midi sound
						pObj->Detach();
					}
				}
			}

			// Start non-wield menu on next row
			menuLoc.y += objSize.Height();

			nNumWieldChoices = xObjList.GetCount();

			if (nNumWieldChoices != 0) {
				cWieldMenuSize = menuSize;
			}
		}
	}

	bool bNoMenu;
	bNoMenu = false;

	for (i = 0; i < GetObjectList()->GetCount(); ++i) {

		pObj = GetObjectList()->GetNodeItem(i);

		if (pObj->IsLocal() && (!pObj->GetExpression() || pObj->GetExpression()->Evaluate(pObj->IsNegative()))) {
			// Only attach if not attached
			if (pObj->IsAttached() == false) {
				pObj->Attach();

				// Otherwise, we need to re-calculate the size of the text object,
				// since we are gonna trash is with our own values soon.

			} else {
				if (pObj->GetType() == TEXTOBJ) {
					((CBagTextObject *)pObj)->RecalcTextRect(((CBagTextObject *)pObj)->IsCaption());
				}
			}

			if (!pObj->IsImmediateRun()) {
				// Get the next menu items pos
				objSize = pObj->GetRect();

				// If it is a text object increment next position by its height
				if (pObj->GetType() == TEXTOBJ) {

					if (tmpVal)//if we have a value move text to next line
						menuLoc.y += tmpVal;
					tmpVal = 0; //text objects set the next line to be at the very begining

					// If we're zoomed, then do things differently
					CBagTextObject *pTXObj = (CBagTextObject *)pObj;
					if (pTXObj->IsCaption()) {
						SBZoomPda *pZPDA = (SBZoomPda *)SDEVMNGR->GetStorageDevice("BPDAZ_WLD");
						if (pZPDA && pZPDA->GetZoomed()) {
							bZoomed = true;

							wndRect = pZPDA->GetViewRect();
						}
					}

					if (bTextOnly) {
						if (wndRect.Height() <= (objSize.Height() + menuLoc.y)) {
							menuLoc.y = 1;
							nBaseMenuLocX += (menuSize.cx + 2);
							menuLoc.x = nBaseMenuLocX + 1;
						}
					} else {
						if (wndRect.Height() <= ((objSize.Height() + menuLoc.y) + 41)) {
							menuLoc.y = 1;
							nBaseMenuLocX += (menuSize.cx + 2);
							menuLoc.x = nBaseMenuLocX;
						}
					}
					menuLoc.x = (1 + nBaseMenuLocX);
					if (menuSize.cx < (objSize.Width() + menuLoc.x))
						menuSize.cx = (objSize.Width() + menuLoc.x);
					if (menuSize.cy < (objSize.Height() + menuLoc.y))
						menuSize.cy = (objSize.Height() + menuLoc.y);
					pObj->SetPosition(menuLoc);
					pObj->SetHighlight(false);

					menuLoc.x = (1 + nBaseMenuLocX);
					menuLoc.y += (objSize.Height() + 1);
				} else { // increment next position by its width

					if (wndRect.Height() <= (objSize.Height() + menuLoc.y)) {
						menuLoc.y = 1;
						nBaseMenuLocX += (menuSize.cx + 2);
						menuLoc.x = nBaseMenuLocX;
					}

					bTextOnly = false;

					// continue to grow menu size to max required
					if (menuSize.cx < (objSize.Width() + menuLoc.x))
						menuSize.cx = (objSize.Width() + menuLoc.x);
					if (menuSize.cy < (objSize.Height() + menuLoc.y))
						menuSize.cy = (objSize.Height() + menuLoc.y);

					pObj->SetPosition(menuLoc);
					pObj->SetHighlight(false);

					if ((nObjectPal < 0) && ((pObj->GetType() == BMPOBJ) || (pObj->GetType() == SPRITEOBJ)))
						nObjectPal = i;
					menuLoc.x += objSize.Width();
					tmpVal = objSize.Height();//save the obj height for use later if we get a text obj
				}

				if (pObj->IsNoMenu()) {
					bNoMenu = true;
				}

				xObjList.AddToTail(pObj);
				nNumItems++;

				nMenuCount++;

			} else {
				nRunItems++;
				pObj->RunObject();
				if (pObj->GetType() == LINKOBJ) {
					g_bAAOk = false;
				}
			}
		} // if in local area and activated by expression
	}

	// If we ran something and there are no other menu items just return
	if (!(nRunItems && !nMenuCount)) {

		if (nMenuCount) {
			menuLoc.y += objSize.Height();
			menuLoc.x = 1;
			nMenuCount = 0;
		}

		menuSize.cx++;
		menuSize.cy++;

		CBofRect tmpRect(CBofPoint(x, y - menuSize.cy / 2), menuSize);
		menuSize.cy += 2;
		bool bMoved;

		// If the menu contains only one object and it is a caption style text object
		// position the dialog box at the bottom of the Game window screen
		bMoved = false;
		if ((nNumItems == 1) && (xObjList.GetTail()->GetNodeItem()->GetType() == TEXTOBJ) && (((CBagTextObject *)xObjList.GetTail()->GetNodeItem())->IsCaption())) {
			while (nNumWieldChoices-- != 0) {
				pObj = xObjList.RemoveHead();
				pObj->Detach();
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

			nNumChars = ((CBagTextObject *)xObjList.GetHead()->GetNodeItem())->GetText().GetLength();

			// Bring caption back 3 pixels
			CBofPoint cPoint;
			cPoint = xObjList.GetHead()->GetNodeItem()->GetPosition();
			cPoint.x = 1;

			xObjList.GetHead()->GetNodeItem()->SetPosition(cPoint);

		} else {
			if (bTextOnly) {
				while (nNumWieldChoices-- != 0) {
					pObj = xObjList.RemoveHead();
					pObj->Detach();
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

			// if the menu would go off the screen, adjust it
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

			for (i = 0; i < xObjList.GetCount(); i++) {
				pObj = xObjList[i];

				if (pObj->GetType() == TEXTOBJ) {
					int cx, cy;
					cx = tmpRect.Size().cx - 1;
					cy = tmpRect.Size().cy - 1;
					if (!bCaption) {
						cy = ((CBagTextObject *)pObj)->GetSize().cy + 2;
					}

					pObj->SetSize(CBofSize(cx, cy));

					// Need to move the menus up when have a wielded item, but
					// not displaying the wield menu
					//
					if (bMoved) {
						CBofPoint cPos;
						cPos = pObj->GetPosition();
						cPos.y -= cWieldMenuSize.cy;
						pObj->SetPosition(cPos);
					}
				}
			}

			CBagPanWindow::FlushInputEvents();

			// If we were requested to put a dialog over the PDA, then shift it upward
			// a bit... unless of course the mousedown occurred in the PDA itself.
			CBagPDA *pPDA = nullptr;
			pPDA = (CBagPDA *)SDEVMNGR->GetStorageDevice("BPDA_WLD");

			if (pPDA != nullptr && (pPDA->IsActivated() && bZoomed == false)) {
				if (!pPDA->IsInside(cMouseDown)) {
					CBofRect cPDARect = pPDA->GetRect();

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

			xPal.LoadPalette(cString);
			dlg.Create(pParent, &xPal, &tmpRect);

			if (bCaption) {
				// We need to move this back to the correct position
				// because the Boflibs center dialog ALWAYS !!!!!!!!

				// If the PDA is currently active, then we have to put the caption elsewhere,
				// or deactivate the PDA (if PDA zoomed, just place at bottom.
				if (CBagPanWindow::m_pPDABmp != nullptr && CBagPanWindow::m_pPDABmp->IsActivated() && bZoomed == false) {
					dlg.Move(wndRect.left, wndRect.top, true);
				} else {
					dlg.Move(tmpRect.left, tmpRect.top, true);
				}

				// Need to re-save the background because there is a bug with moving
				// the dialog box.
				dlg.SaveBackground();

				// Set this caption to automatically go away after specified delay
				if (m_nDelay > 0) {
					dlg.SetTimer(TIMER_CLOSE_ID, m_nDelay + (80 * nNumChars));
				}
			} else {
				dlg.Move(tmpRect.TopLeft().x, tmpRect.TopLeft().y, true);

				// Need to re-save the background because there is a bug with moving
				// the dialog box.
				dlg.SaveBackground();
			}

			CBagPanWindow::FlushInputEvents();
			int nUseTurn;

			g_bPauseTimer = true;

			nUseTurn = dlg.DoModal();
			g_bPauseTimer = false;

			pObj = dlg.m_pSelectedObject;

			dlg.Destroy();

			for (i = 0; i < GetObjectCount(); ++i) {
				GetObjectByPos(i)->Detach();
			}

			// If our current storage device is "AS CUSTOM" then don't allow
			// the timer to get incremented.
			CBagStorageDev *pCurSDEV = CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev();
			if (pObj != nullptr) {
				pObj->RunCallBack();

				// Selecting this menu item causes a turn to go by
				if (nNumCalls == 1 && pCurSDEV->IsCustom() == false) {
					VARMNGR->IncrementTimers();
				}
			} else if (bCaption) {
				// Selecting this menu item causes a turn to go by
				if (nNumCalls == 2) {
					dlg.m_pSelectedObject = nullptr;
					if (pCurSDEV->IsCustom() == false) {
						VARMNGR->IncrementTimers();
					}
				}
			}
		}
	}

	nNumCalls--;

	return bReturn;
}

bool CBagMenu::AddItem(CBagObject *pObj, void *( * /*func*/)(int, void *), void * /*info*/) {
	pObj->SetPosition(CBofPoint(0, m_nY));
	//pObj->Attach();
	m_nY = (int16)(m_nY + (int16)(pObj->GetRect().Height() + 1));
	AddObject(pObj);

	return true;
}

bool CBagMenu::DeleteItem(const CBofString & /*sLabel*/) {
	return false;
}

bool CBagMenu::IsChecked(const CBofString & /*sLabel*/, const CBofString & /*sSubLabel*/) {
	int nRow, nCol;
	nRow = nCol = 0;

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
//
//
//
//
//
//
//
//

CBagMenuDlg::CBagMenuDlg() {
	// Remove this SDEV from the storage device list so that it is not deleted
	// when we switch .WLD files, and there may still be a Dialog open.
	SDEVMNGR->UnRegisterStorageDev(this);
}

CBagMenuDlg::~CBagMenuDlg() {
	Assert(IsValidObject(this));
}

ErrorCode CBagMenuDlg::Create(CBofWindow *pWnd, CBofPalette *pPal, const CBofRect *pRect, uint32 /*nStyle*/) {
	CBofRect r;

	m_bMultipleDialogs = false;
	m_bAcceptInput = true;
	_nReturnValue = 0;

	if (!pRect) {
		r = pWnd->GetWindowRect();
		r.OffsetRect(-r.left, -r.top);
		r.bottom = r.top + MENU_DFLT_HEIGHT;
	} else {
		r = *pRect;
	}

	CBagStorageDevDlg::Create("Menu", &r, pWnd, 0);

	CBofBitmap *pBmp;
	if ((pBmp = new CBofBitmap(r.Width(), r.Height(), pPal)) != nullptr) {

		r.OffsetRect(-r.left, -r.top);
		Assert(pPal != nullptr);
		pBmp->FillRect(&r, pPal->GetNearestIndex(RGB(82, 82, 82)) /*RGB(0,0,0)*/);
		pBmp->DrawRect(&r, pPal->GetNearestIndex(RGB(0, 0, 0)) /*RGB(0,0,0)*/);
		SetBackdrop(pBmp);
	}

	return m_errCode;
}


void CBagMenuDlg::OnLButtonUp(uint32 nFlags, CBofPoint *pPoint, void *) {
	// We are ignoring all input until the dialog is actually visible
	if (m_bAcceptInput) {
		m_pSelectedObject = nullptr;

		OnClose();

		if (m_bMultipleDialogs) {
			CBofRect r = GetWindowRect();
			r.OffsetRect(-r.left, -r.top);
			if (r.PtInRect(*pPoint)) {
				CBagStorageDevDlg::OnLButtonUp(nFlags, pPoint);
				if ((m_pSelectedObject = GetLActiveObject()) != nullptr) {
				}
			}

		} else {
			CBofPoint pt;

			pt = DevPtToViewPort(*pPoint);

			if ((m_pSelectedObject = GetObject(pt)) != nullptr) {
				m_pSelectedObject->OnLButtonUp(nFlags, pPoint);
			}
		}

		_nReturnValue = (m_pSelectedObject != nullptr);
	}
}

void CBagMenuDlg::OnMouseMove(uint32 /*nFlags*/, CBofPoint *pPoint, void *) {
	CBagObject *pObj;

	CBagMasterWin::SetActiveCursor(0);

	if ((pObj = GetObject(*pPoint)) != nullptr) {

		// Switch to that cursor
		//
		CBagMasterWin::SetActiveCursor(pObj->GetOverCursor());

		if (pObj != GetLActiveObject()) {
			if (pObj->GetCallBack() || pObj->GetMenuPtr()) {

				pObj->SetHighlight();
				if (GetLActiveObject())
					GetLActiveObject()->SetHighlight(false);
				SetLActiveObject(pObj);
			}
		}
	}
}

void CBagMenuDlg::OnTimer(uint32 nID) {
	Assert(IsValidObject(this));

	switch (nID) {
	// Auto close for text-Captions
	//
	case TIMER_CLOSE_ID:
		KillTimer(nID);
		Close();
		break;

	// Can now allow user input
	//
	case TIMER_HACK_ID:
		KillTimer(nID);
		m_bAcceptInput = true;
		break;

	default:
		LogWarning("Invalid Timer ID");
		break;
	}
}

void CBagMenuDlg::OnPaint(CBofRect *pRect) {
	Assert(IsValidObject(this));

	CBagStorageDevDlg::OnPaint(pRect);

	// Don't allow user input until this menu is visible
	CBagPanWindow::FlushInputEvents();
}

void CBagMenuDlg::OnDeActivate() {
	Assert(IsValidObject(this));

	Close();
}

} // namespace Bagel
