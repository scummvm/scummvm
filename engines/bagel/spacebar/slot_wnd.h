
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

#ifndef BAGEL_SPACEBAR_SLOT_WND_H
#define BAGEL_SPACEBAR_SLOT_WND_H

#include "bagel/spacebar/baglib/storage_dev_win.h"
#include "bagel/spacebar/boflib/gui/text_box.h"
#include "bagel/spacebar/boflib/gui/button.h"
#include "bagel/boflib/sound.h"

namespace Bagel {
namespace SpaceBar {

#define NUM_SLOTBUTT 17

// ID's and INDEX of buttons
#define ONE         0
#define FOUR        1
#define SEVEN       2
#define TEN         3
#define FORTY       4
#define SEVENTY     5
#define OHNDRD      6
#define RHNDRD      7
#define SHNDRD      8
#define OTHSND      9
#define RTHSND      10
#define STHSND      11
#define CLRBET      12
#define BETALL      13
#define GO          14
#define SLOTHELP    15
#define SLOTQUIT    16

#define SLOT_NUM        4
#define SLOT_BMP_NUM    9


#define SHOWGO          100



class SBarSlot {
public:
	int _nIdx;
	CBofRect    _cSlotRect;
	CBofBitmap *_pSlotBmp[SLOT_BMP_NUM];
};

class SBarSlotWnd : public CBagStorageDevWnd {
private:
	CBofRect Slot0Rect = CBofRect(220, 46, 464, 98);
	CBofRect Slot1Rect = CBofRect(469, 14, 522, 187);
	CBofRect Slot2Rect = CBofRect(378, 192, 623, 243);
	CBofRect Slot3Rect = CBofRect(321, 102, 374, 274);

	CBofRect CreditRect = CBofRect(24, 334, 135, 360);
	CBofRect BetRect = CBofRect(15, 410, 131, 432);
	CBofRect OddRect = CBofRect(401, 125, 443, 166);
	CBofRect FixRect = CBofRect(150, 306, 150 + 109 - 1, 306 + 64 - 1);

protected:
	SBarSlot _cSlots[SLOT_NUM];
	CBofBmpButton *_pSlotButs[NUM_SLOTBUTT];
	int _nBet;
	int _nCredit;
	int _nPayOff1;
	int _nPayOff2;
	CBofText *_pCredText;
	CBofText *_pBetText;
	CBofText *_pOddsText;
	bool _bFixBet;
	bool _bAutoDecrement;
	CBofBitmap *_bFixBmp;
	CBofBitmap *_pLoseBmp;
	CBofSound *_pBkgSnd;            // Casino background sounds
	CBofSound *_pSlotSound;
	CBofSound *_pWinSound;          // Allow all ambient noise to continue playing
	bool _bLose;

public:
	SBarSlotWnd();

	virtual void onBofButton(CBofObject *pButton, int nState);
	virtual void onMainLoop();

	virtual ErrorCode attach(); // This function attaches the background and necessary bitmaps
	virtual ErrorCode detach(); // This function detaches the background and necessary bitmaps

	void addBet(int nBetVal);
	void betAll();
	void clrBet();
	void fixBet();
	void go();

	void updateText();
	void calcOutcome();

	void quadPays(int nSlotIdx);
	void tripPays(int nSlotIdx);
	void pairPays(int nSlotIdx);

	void setPayOff(int nPay1, int nPay2);

	void slideSlots();

	void onPaint(CBofRect *pRect);
	void onTimer(uint32 nTimerId);
	void onLButtonDown(uint32 nFlags, CBofPoint *pPoint, void * = nullptr);
	void onLButtonUp(uint32 /*nFlags*/, CBofPoint */*pPoint*/, void * = nullptr) { }

};

} // namespace SpaceBar
} // namespace Bagel

#endif
