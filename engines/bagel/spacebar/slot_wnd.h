
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

#include "bagel/baglib/storage_dev_win.h"
#include "bagel/boflib/gui/text_box.h"
#include "bagel/boflib/gui/window.h"
#include "bagel/boflib/gui/button.h"
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
	int m_nIdx;
	CBofRect    m_cSlotRect;
	CBofBitmap *m_pSlotBmp[SLOT_BMP_NUM];
};

class SBarSlotWnd : public CBagStorageDevWnd {
protected:
	SBarSlot        m_cSlots[SLOT_NUM];
	CBofBmpButton *m_pSlotButs[NUM_SLOTBUTT];
	int             m_nBet;
	int             m_nCredit;
	int             m_nPayOff1;
	int             m_nPayOff2;
	CBofText *m_pCredText;
	CBofText *m_pBetText;
	CBofText *m_pOddsText;
	BOOL            m_bFixBet;
	BOOL            m_bAutoDecrement;
	CBofBitmap *m_bFixBmp;
	CBofBitmap *m_pLoseBmp;
	CBofSound *m_pBkgSnd;               // Added for casino background sounds BFW 12/24/96
	CBofSound *m_pSlotSound;
	CBofSound *m_pWinSound;         // Added to allow all ambient noise to continue playing BFW 01/02/97
	BOOL            m_bPaused;
	BOOL            m_bLose;

public:
	SBarSlotWnd();

	virtual VOID        OnBofButton(CBofObject *pButton, INT nState);
	virtual VOID        OnMainLoop();

	virtual ERROR_CODE  Attach();   // This function attachs the background and nessasary bitmaps
	virtual ERROR_CODE  Detach();   // This function detachs the background and nessasary bitmaps

	VOID                AddBet(int nBetVal);
	VOID                BetAll();
	VOID                ClrBet();
	VOID                FixBet();
	VOID                Go();

	VOID                UpdateText();
	VOID                CalcOutcome();

	VOID                QuadPays(int nSlotIdx);
	VOID                TripPays(int nSlotIdx);
	VOID                PairPays(int nSlotIdx);

	VOID                SetPayOff(int nPay1, int nPay2);

	VOID                SlideSlots();

	VOID                OnPaint(CBofRect *pRect);
	VOID                EraseBackdrop();
	VOID                OnTimer(UINT nTimerId);
	VOID                OnLButtonDown(UINT nFlags, CBofPoint *pPoint, void * = nullptr);
	VOID                OnLButtonUp(UINT /*nFlags*/, CBofPoint * /*pPoint*/, void * = nullptr) {
	}
};

} // namespace SpaceBar
} // namespace Bagel

#endif
