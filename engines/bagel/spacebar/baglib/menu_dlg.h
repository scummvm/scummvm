
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

#include "bagel/spacebar/baglib/storage_dev_win.h"
#include "bagel/spacebar/baglib/sprite_object.h"

namespace Bagel {
namespace SpaceBar {

class CBagMenu : public CBagStorageDev {
private:
	static CBofList<CBagObject *> *_pUniversalObjectList; // Objects used in every menu
	static int _nDefaultDelay;

	int16 _nDelay;
	int16 _nY; // Replaces _nNextPos

public:
	CBagMenu();
	static void initialize();

	ErrorCode setBackground(CBofBitmap * /*pBmp*/) {
		assert(false);
		return ERR_NONE;
	}
	CBofBitmap *getBackground() {
		assert(false);
		return nullptr;
	}

	bool addItem(CBagObject *pObj, void *(*func)(int, void *), void *info);
	bool deleteItem(const CBofString &sLabel);

	bool isChecked(const CBofString &sLabel, const CBofString &sSubLabel = CBofString());
	bool isChecked(int nRefId);
	bool isCheckedPos(int nRow, int nCol = -1);
	bool check(const CBofString &sLabel, const CBofString &sSubLabel = CBofString());
	bool check(int nRefId);
	bool unCheck(const CBofString &sLabel, const CBofString &sSubLabel = CBofString());
	bool unCheck(int nRefId);

	bool trackPopupMenu(uint32 nFlags, int x, int y, CBofWindow *pWnd, CBofPalette *pPal, CBofRect *lpRect = nullptr);

	static bool removeUniversalObjectList();
	static bool setUniversalObjectList(CBofList<CBagObject *> *pObjList);
	static CBofList<CBagObject *> *getUniversalObjectList() {
		return _pUniversalObjectList;
	}

	CBagObject *onNewSpriteObject(const CBofString &);
};

class CBagMenuDlg : public CBagStorageDevDlg {
	friend class CBagMenu;

private:
	bool _bMultipleDialogs;
	bool _bAcceptInput;

public:
	static CBagObject *_pSelectedObject;
	static void initialize();

	CBagMenuDlg();
	virtual ~CBagMenuDlg();

	ErrorCode createDlg(CBofWindow *pWnd, CBofPalette *pPal, CBofRect *cRect);

	virtual void onMouseMove(uint32 nFlags, CBofPoint *pPoint, void * = nullptr);
	virtual void onLButtonUp(uint32 nFlags, CBofPoint *pPoint, void * = nullptr);

	virtual void onDeActivate();

	virtual void onTimer(uint32 nID);
	virtual void onPaint(CBofRect *pRect);
};

extern bool g_pauseTimerFl;

} // namespace SpaceBar
} // namespace Bagel

#endif
