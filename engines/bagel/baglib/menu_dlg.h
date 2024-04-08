
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
	static int m_nDefaultDelay;

	int16 m_nDelay;
	int16 m_nY; // Replaces m_nNextPos

public:
	CBagMenu();
	static void initialize();

	ERROR_CODE SetBackground(CBofBitmap * /*pBmp*/) {
		Assert(FALSE);
		return ERR_NONE;
	}
	CBofBitmap *GetBackground() {
		Assert(FALSE);
		return nullptr;
	}

	bool AddItem(CBagObject *pObj, void *(*func)(int, void *), void *info);
	bool DeleteItem(const CBofString &sLabel);

	bool IsChecked(const CBofString &sLabel, const CBofString &sSubLabel = BofEmptyString);
	bool IsChecked(int nRefId);
	bool IsCheckedPos(int nRow, int nCol = -1);
	bool Check(const CBofString &sLabel, const CBofString &sSubLabel = BofEmptyString);
	bool Check(int nRefId);
	bool UnCheck(const CBofString &sLabel, const CBofString &sSubLabel = BofEmptyString);
	bool UnCheck(int nRefId);

	bool TrackPopupMenu(uint32 nFlags, int x, int y, CBofWindow *pWnd, CBofPalette *pPal, CBofRect *lpRect = 0);

	// bool AddUniversalObject(CBagObject *pObj);
	static bool RemoveUniversalObjectList();
	static bool SetUniversalObjectList(CBofList<CBagObject *> *pObjList);
	static CBofList<CBagObject *> *GetUniversalObjectList() {
		return m_pUniversalObjectList;
	}

	CBagObject *OnNewSpriteObject(const CBofString &);
};

class CBagMenuDlg : public CBagStorageDevDlg {
	friend class CBagMenu;

private:
	bool m_bMultipleDialogs;
	bool m_bAcceptInput;

public:
	static CBagObject *m_pSelectedObject;
	static void initialize();

	CBagMenuDlg();
	virtual ~CBagMenuDlg();

	ERROR_CODE Create(CBofWindow *pWnd, CBofPalette *pPal, const CBofRect *cRect = nullptr, uint32 nStyle = 0);

	// virtual void OnClose();
	virtual void OnMouseMove(uint32 nFlags, CBofPoint *pPoint, void * = nullptr);
	virtual void OnLButtonUp(uint32 nFlags, CBofPoint *pPoint, void * = nullptr);

	virtual void OnDeActivate();

	virtual void OnTimer(uint32 nID);
	virtual void OnPaint(CBofRect *pRect);
};

extern bool g_bPauseTimer;

} // namespace Bagel

#endif
