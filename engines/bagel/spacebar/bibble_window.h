
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

#ifndef BAGEL_SPACEBAR_BIBBLE_WINDOW_H
#define BAGEL_SPACEBAR_BIBBLE_WINDOW_H

#include "bagel/baglib/storage_dev_win.h"
#include "bagel/boflib/gui/text_box.h"
#include "bagel/boflib/gui/window.h"
#include "bagel/boflib/app.h"
#include "bagel/boflib/gui/button.h"
#include "bagel/boflib/gfx/sprite.h"
#include "bagel/boflib/sound.h"

namespace Bagel {
namespace SpaceBar {

#define BIBBLE_NUM_BUTTONS 12
#define BIBBLE_NUM_BIBBLES  3
#define BIBBLE_NUM_SHOUTS   4
#define BIBBLE_NUM_PAYOFFS 16
#define BIBBLE_NUM_BET_AREAS 24

typedef struct {
	INT m_nPay1;
	INT m_nPay2;
} ST_PAYOFFS;

class CBetArea : public CBofObject {
public:
	CBetArea(UINT nBet, INT nLeft, INT nTop, INT nRight, INT nBot, INT nPay1, INT nPay2, const CHAR *pszAudioFile, const CHAR *pszPayFile);

	// Data members
	//
	CBofRect m_cRect;
	INT m_nBet;
	INT m_nPayOff1;
	INT m_nPayOff2;
	BOOL m_bWon;
	CBofString m_cAudioFile;
	CBofString m_cPayFile;
};

extern CBetArea g_cBetAreas[BIBBLE_NUM_BET_AREAS];
extern const ST_PAYOFFS g_stPayOffs[BIBBLE_NUM_PAYOFFS];


class CBibbleWindow : public CBagStorageDevWnd {
public:
	CBibbleWindow();

#if BOF_DEBUG
	virtual ~CBibbleWindow();
#endif

	virtual	ERROR_CODE Attach();
	virtual ERROR_CODE Detach();

	virtual VOID OnBofButton(CBofObject *pButton, INT nState);
	virtual VOID OnClose();

protected:
	ERROR_CODE PlayGame();
	ERROR_CODE BonkBibble(INT nBibble, INT nShout);

	VOID CalcOutcome();
	ERROR_CODE DisplayCredits();

	ERROR_CODE Highlight(CBetArea *pArea, UBYTE nColor);
	ERROR_CODE UnHighlight(CBetArea *pArea);

	virtual VOID OnLButtonDown(UINT nFlags, CBofPoint *pPoint);
	virtual VOID OnLButtonUp(UINT nFlags, CBofPoint *pPoint);
	virtual VOID OnLButtonDblClk(UINT nFlags, CBofPoint *pPoint);
	virtual VOID OnKeyHit(ULONG lKey, ULONG lRepCount);

	virtual VOID OnPaint(CBofRect *pRect);
	virtual VOID OnMainLoop();
	virtual VOID OnTimer(UINT /*nTimerID*/) {
	};   // Do nothing

	// Data
	//
	CBofSound *m_pBkgSnd;				// Added for casino background sounds BFW 12/24/96
	CBofBmpButton *m_pButtons[BIBBLE_NUM_BUTTONS];
	CBofText *m_pCreditsText;
	CBofSprite *m_pMasterBibble;
	CBofSprite *m_pBibble[BIBBLE_NUM_BIBBLES];
	CBofSprite *m_pBall;
	CBofSprite *m_pArch1;
	CBofSprite *m_pArch2;
	CBofSprite *m_pArch3;
	CBofSound *m_pShouts[BIBBLE_NUM_SHOUTS];
	ULONG m_nNumCredits;

	INT m_nBall1;           // Which Bibble hit by ball 1
	INT m_nBall2;           // Which Bibble hit by ball 2
	INT m_nBall3;           // Which Bibble hit by ball 3

	INT m_nBall1Said;
	INT m_nBall2Said;
	INT m_nBall3Said;

	INT m_nNumShout1;
	INT m_nNumShout2;
	INT m_nNumShout3;
	INT m_nNumShout4;

	INT m_nNumTopBonks;
	INT m_nNumMidBonks;
	INT m_nNumBotBonks;
	CBetArea *m_pSelected;
};

} // namespace SpaceBar
} // namespace Bagel

#endif
