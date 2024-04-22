
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

struct ST_PAYOFFS {
	int m_nPay1;
	int m_nPay2;
};

struct CBetAreaDef {
	int left;
	int top;
	int right;
	int bottom;
	int m_nBet;
	int m_nPayOff1;
	int m_nPayOff2;
	const char *m_cAudioFile;
	const char *m_cPayFile;
};

class CBetArea : public CBofObject {
public:
	CBetArea() {}
	CBetArea(const CBetAreaDef &def);

	// Data members
	CBofRect m_cRect;
	int m_nBet = 0;
	int m_nPayOff1 = 0;
	int m_nPayOff2 = 0;
	bool m_bWon = false;
	CBofString m_cAudioFile;
	CBofString m_cPayFile;
};

extern CBetAreaDef BET_AREAS[BIBBLE_NUM_BET_AREAS];
extern const ST_PAYOFFS PAY_OFFS[BIBBLE_NUM_PAYOFFS];


class CBibbleWindow : public CBagStorageDevWnd {
public:
	CBibbleWindow();

	virtual ErrorCode Attach();
	virtual ErrorCode Detach();

	virtual void OnBofButton(CBofObject *pButton, int nState);
	virtual void OnClose();

protected:
	ErrorCode PlayGame();
	ErrorCode BonkBibble(int nBibble, int nShout);

	void CalcOutcome();
	ErrorCode DisplayCredits();

	ErrorCode Highlight(CBetArea *pArea, byte nColor);
	ErrorCode UnHighlight(CBetArea *pArea);

	virtual void OnLButtonDown(uint32 nFlags, CBofPoint *pPoint, void * = nullptr);
	virtual void OnLButtonUp(uint32 nFlags, CBofPoint *pPoint, void * = nullptr);
	virtual void OnLButtonDblClk(uint32 nFlags, CBofPoint *pPoint);
	virtual void OnKeyHit(uint32 lKey, uint32 lRepCount);

	virtual void OnPaint(CBofRect *pRect);
	virtual void OnMainLoop();
	virtual void OnTimer(uint32 /*nTimerID*/) {
		// Do nothing
	}

	// Data
	CBofSound *m_pBkgSnd;
	CBofBmpButton *m_pButtons[BIBBLE_NUM_BUTTONS];
	CBofText *m_pCreditsText;
	CBofSprite *m_pMasterBibble;
	CBofSprite *m_pBibble[BIBBLE_NUM_BIBBLES];
	CBofSprite *m_pBall;
	CBofSprite *m_pArch1;
	CBofSprite *m_pArch2;
	CBofSprite *m_pArch3;
	CBofSound *m_pShouts[BIBBLE_NUM_SHOUTS];
	uint32 m_nNumCredits;

	int m_nBall1;           // Which Bibble hit by ball 1
	int m_nBall2;           // Which Bibble hit by ball 2
	int m_nBall3;           // Which Bibble hit by ball 3

	int m_nBall1Said;
	int m_nBall2Said;
	int m_nBall3Said;

	int m_nNumShout1;
	int m_nNumShout2;
	int m_nNumShout3;
	int m_nNumShout4;

	int m_nNumTopBonks;
	int m_nNumMidBonks;
	int m_nNumBotBonks;
	CBetArea *m_pSelected;
};

} // namespace SpaceBar
} // namespace Bagel

#endif
