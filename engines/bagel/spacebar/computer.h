
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
#include "bagel/spacebar/boflib/gui/list_box.h"
#include "bagel/spacebar/boflib/gui/text_box.h"
#include "bagel/spacebar/boflib/gui/button.h"
#include "bagel/spacebar/boflib/list.h"

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
	char *_pItem;
	char *_pList;
	char *_pDrink;
};

class SBarComputer : public CBagStorageDevWnd {
private:
	CBofRect _compDisplay;
	CBofRect _compTextWindow;

protected:
	CBofListBox *_pDrinkBox;   // Used for list of Drinks
	CBofListBox *_pIngBox;     // Used for list of Ingredients
	CBofTextBox *_pTBox;       // Used to display the list
	char *_pDrinkBuff;
	char *_pIngBuff;
	CBofList<SBarCompItem> *_pDrinkList;
	CBofList<SBarCompItem> *_pIngList;
	int _nDrinkSelect;
	int _nIngSelect;
	CBofBmpButton *_pButtons[NUM_COMPBUTT];
	COMPMODE _eMode;
	COMPMODE _ePrevMode;

public:
	SBarComputer();
	virtual ~SBarComputer();

	void onBofListBox(CBofObject *pListBox, int nItemIndex);

	virtual void onBofButton(CBofObject *pButton, int nState);
	virtual void onKeyHit(uint32 lKey, uint32 nRepCount);

	/**
	 * This function attaches the background and necessary bitmaps
	 */
	virtual ErrorCode attach();

	/**
	 * This function detaches the background and necessary bitmaps
	 */
	virtual ErrorCode detach();

	ErrorCode createDrinksListBox();
	ErrorCode createIngListBox();

	void createTextBox(CBofString &newText);
	void deleteTextBox();
	void deleteListBox();

	void setOn();
	void setOff();
	void setDrink();
	void setIng();
	void setList();

	/**
	 * Read in the ingredient file
	 */
	ErrorCode readDrnkFile();

	/**
	 * Read in the ingredient file
	 */
	ErrorCode readIngFile();

	/**
	 * Order Button
	 */
	void order();

	void pageUp();
	void pageDown();
	void back();

	void onMainLoop();

	virtual void onTimer(uint32 /*nTimerID*/) {
	}

	void onPaint(CBofRect *pRect);
	void eraseBackdrop();

	void onLButtonDown(uint32 /*nFlags*/, CBofPoint */*pPoint*/, void * = nullptr) {}
	void onLButtonUp(uint32 /*nFlags*/, CBofPoint */*pPoint*/, void * = nullptr) {}

	void onMouseMove(uint32 nFlags, CBofPoint *, void * = nullptr);
};

} // namespace SpaceBar
} // namespace Bagel

#endif
