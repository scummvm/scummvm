
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
	int _nPay1;
	int _nPay2;
};

struct CBetAreaDef {
	int _nBet;
	int _left;
	int _top;
	int _right;
	int _bottom;
	int _nPayOff1;
	int _nPayOff2;
	const char *_cAudioFile;
	const char *_cPayFile;
};

class CBetArea : public CBofObject {
public:
	CBetArea() {}
	CBetArea(const CBetAreaDef &def);

	// Data members
	CBofRect _cRect;
	int _nBet = 0;
	int _nPayOff1 = 0;
	int _nPayOff2 = 0;
	bool _bWon = false;
	CBofString _cAudioFile;
	CBofString _cPayFile;
};

extern CBetAreaDef BET_AREAS[BIBBLE_NUM_BET_AREAS];
extern const ST_PAYOFFS PAY_OFFS[BIBBLE_NUM_PAYOFFS];


class CBibbleWindow : public CBagStorageDevWnd {
public:
	CBibbleWindow();

	virtual ErrorCode attach();
	virtual ErrorCode detach();

	virtual void onBofButton(CBofObject *pButton, int nState);
	virtual void onClose();

protected:
	ErrorCode playGame();
	ErrorCode bonkBibble(int nBibble, int nShout);

	void calcOutcome();
	ErrorCode displayCredits();

	ErrorCode highlight(CBetArea *pArea, byte nColor);
	ErrorCode unHighlight(CBetArea *pArea);

	virtual void onLButtonDown(uint32 nFlags, CBofPoint *pPoint, void * = nullptr);
	virtual void onLButtonUp(uint32 nFlags, CBofPoint *pPoint, void * = nullptr);
	virtual void onLButtonDblClk(uint32 nFlags, CBofPoint *pPoint);
	virtual void onKeyHit(uint32 lKey, uint32 lRepCount);

	virtual void onPaint(CBofRect *pRect);
	virtual void onMainLoop();
	virtual void onTimer(uint32 /*nTimerID*/) {
		// Do nothing
	}

	// Data
	CBofSound *_pBkgSnd;
	CBofBmpButton *_pButtons[BIBBLE_NUM_BUTTONS];
	CBofText *_pCreditsText;
	CBofSprite *_pMasterBibble;
	CBofSprite *_pBibble[BIBBLE_NUM_BIBBLES];
	CBofSprite *_pBall;
	CBofSprite *_pArch1;
	CBofSprite *_pArch2;
	CBofSprite *_pArch3;
	CBofSound *_pShouts[BIBBLE_NUM_SHOUTS];
	uint32 _nNumCredits;

	int _nBall1;           // Which Bibble hit by ball 1
	int _nBall2;           // Which Bibble hit by ball 2
	int _nBall3;           // Which Bibble hit by ball 3

	int _nBall1Said;
	int _nBall2Said;
	int _nBall3Said;

	int _nNumShout1;
	int _nNumShout2;
	int _nNumShout3;
	int _nNumShout4;

	int _nNumTopBonks;
	int _nNumMidBonks;
	int _nNumBotBonks;
	CBetArea *_pSelected;
};

} // namespace SpaceBar
} // namespace Bagel

#endif
