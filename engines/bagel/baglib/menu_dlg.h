
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

#ifndef BAGEL_BAGLIB_MENU_DLG_H
#define BAGEL_BAGLIB_MENU_DLG_H

#include "bagel/baglib/storage_dev_win.h"
#include "bagel/baglib/sprite_object.h"

namespace Bagel {

class CBagMenu : public CBagStorageDev {
private:
	static CBofList<CBagObject *> *m_pUniversalObjectList; // Objects used in every menu
	static INT m_nDefaultDelay;

	SHORT m_nDelay;
	SHORT m_nY; // Replaces m_nNextPos

public:
	CBagMenu();

	ERROR_CODE SetBackground(CBofBitmap * /*pBmp*/) {
		Assert(FALSE);
		return ERR_NONE;
	}
	CBofBitmap *GetBackground() {
		Assert(FALSE);
		return nullptr;
	}

	BOOL AddItem(CBagObject *pObj, void *(*func)(int, void *), void *info);
	BOOL DeleteItem(const CBofString &sLabel);

	BOOL IsChecked(const CBofString &sLabel, const CBofString &sSubLabel = BofEmptyString);
	BOOL IsChecked(int nRefId);
	BOOL IsCheckedPos(int nRow, int nCol = -1);
	BOOL Check(const CBofString &sLabel, const CBofString &sSubLabel = BofEmptyString);
	BOOL Check(int nRefId);
	BOOL UnCheck(const CBofString &sLabel, const CBofString &sSubLabel = BofEmptyString);
	BOOL UnCheck(int nRefId);

	BOOL TrackPopupMenu(UINT nFlags, int x, int y, CBofWindow *pWnd, CBofPalette *pPal, CBofRect *lpRect = 0);

	// BOOL AddUniversalObject(CBagObject *pObj);
	static BOOL RemoveUniversalObjectList();
	static BOOL SetUniversalObjectList(CBofList<CBagObject *> *pObjList);
	static CBofList<CBagObject *> *GetUniversalObjectList() { return m_pUniversalObjectList; }

	CBagObject *OnNewSpriteObject(const CBofString &);
};

class CBagMenuDlg : public CBagStorageDevDlg {
	friend class CBagMenu;

private:
	BOOL m_bMultipleDialogs;
	BOOL m_bAcceptInput;

public:
	static CBagObject *m_pSelectedObject;

	// static INT 		m_nActiveMenus;

	CBagMenuDlg();
	virtual ~CBagMenuDlg();

	ERROR_CODE Create(CBofWindow *pWnd, CBofPalette *pPal, const CBofRect *cRect = nullptr, UINT nStyle = 0);

	// virtual VOID OnClose();
	virtual VOID OnMouseMove(UINT nFlags, CBofPoint *pPoint);
	virtual VOID OnLButtonUp(UINT nFlags, CBofPoint *pPoint);

	virtual VOID OnDeActivate();

	virtual VOID OnTimer(UINT nID);
	virtual VOID OnPaint(CBofRect *pRect);
};

} // namespace Bagel

#endif
