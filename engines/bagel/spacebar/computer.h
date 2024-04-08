
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

#ifndef BAGEL_SPACEBAR_COMPUTER_H
#define BAGEL_SPACEBAR_COMPUTER_H

#include "bagel/spacebar/spacebar.h"
#include "bagel/boflib/gui/list_box.h"
#include "bagel/boflib/gui/text_box.h"
#include "bagel/boflib/gui/button.h"
#include "bagel/boflib/list.h"

namespace Bagel {
namespace SpaceBar {

#define NUM_COMPBUTT 12

// ID's and INDEX of buttons
#define OFFBUT  0
#define ONBUT   1
#define DRINKS  2
#define INGRED  3
#define LISTD   4
#define LISTI   5
#define ORDER   6
#define PGUP    7
#define PGDOWN  8
#define BCBACK  9
#define BCHELP  10
#define BCQUIT  11

enum COMPMODE {
	OFFMODE, DRINKMODE, INGMODE, LISTMODE
};

class SBarCompItem {
public:
	char *m_pItem;
	char *m_pList;
	char *m_pDrink;
};

class SBarComputer : public CBagStorageDevWnd {
protected:
	CBofListBox *m_pDrinkBox;   // Used for list of Drinks
	CBofListBox *m_pIngBox;     // Used for list of Ingredients
	CBofTextBox *m_pTBox;       // Used to display the list
	char *m_pDrinkBuff;
	char *m_pIngBuff;
	CBofList<SBarCompItem> *m_pDrinkList;
	CBofList<SBarCompItem> *m_pIngList;
	int m_nDrinkSelect;
	int m_nIngSelect;
	CBofBmpButton *m_pButtons[NUM_COMPBUTT];
	COMPMODE m_eMode;
	COMPMODE m_ePrevMode;

public:
	SBarComputer();
	virtual ~SBarComputer();

	VOID OnBofListBox(CBofObject *pListBox, INT nItemIndex);

	virtual VOID OnBofButton(CBofObject *pButton, INT nState);
	virtual VOID OnKeyHit(uint32 lKey, uint32 nRepCount);

	/**
	 * This function attachs the background and nessasary bitmaps
	 */
	virtual ERROR_CODE Attach();

	/**
	 * This function detachs the background and nessasary bitmaps
	 */
	virtual ERROR_CODE Detach();

	ERROR_CODE CreateDrinksListBox();
	ERROR_CODE CreateIngListBox();

	VOID CreateTextBox(CBofString &newText);
	VOID DeleteTextBox();
	VOID DeleteListBox();

	VOID SetOn();
	VOID SetOff();
	VOID SetDrink();
	VOID SetIng();
	VOID SetList();

	/**
	 * Read in the ingredient file
	 */
	ERROR_CODE ReadDrnkFile();

	/**
	 * Read in the ingredient file
	 */
	ERROR_CODE ReadIngFile();

	/**
	 * Order Button
	 */
	VOID Order();

	VOID PageUp();
	VOID PageDown();
	VOID Back();

	VOID OnMainLoop();

	virtual VOID OnTimer(uint32 /*nTimerID*/) {
	}

	VOID OnPaint(CBofRect *pRect);
	VOID EraseBackdrop();

	VOID OnLButtonDown(uint32 /*nFlags*/, CBofPoint * /*pPoint*/, void * = nullptr) {}
	VOID OnLButtonUp(uint32 /*nFlags*/, CBofPoint * /*pPoint*/, void * = nullptr) {}

	VOID OnMouseMove(uint32 nFlags, CBofPoint *, void * = nullptr);
};

} // namespace SpaceBar
} // namespace Bagel

#endif
