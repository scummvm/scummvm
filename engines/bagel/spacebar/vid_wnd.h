
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

#ifndef BAGEL_SPACEBAR_VID_WND_H
#define BAGEL_SPACEBAR_VID_WND_H

#include "bagel/spacebar/main_window.h"
#include "bagel/spacebar/baglib/button_object.h"
#include "bagel/spacebar/baglib/character_object.h"

namespace Bagel {
namespace SpaceBar {

// ID's and INDEX of buttons
#define VID_PLAY_BUT        0
#define VID_STOP_BUT        100
#define VID_REW_BUT         2
#define VID_FF_BUT          3

class SBarVidBut : public CBagButtonObject {
public:
	SBarVidBut() : CBagButtonObject() {
	}

	void onLButtonUp(uint32 nFlags, CBofPoint *xPoint, void *pInfo = nullptr) override;
};


class SBarVidWnd : public CMainWindow {
protected:
	CBagCharacterObject *_pMovie;

	CBagVar *_pTimerVar;
	CBagVar *_pPlayingVar;
	CBagVar *_pDiscVar;

	double   _fTimerDiff;
	double   _fTimer;
	int      _nStartTime;

public:
	SBarVidWnd();
	~SBarVidWnd();

	void setPlayMode(int nMode);
	void setInc(double fDiff) {
		_fTimerDiff = fDiff;
	}

	bool hasDisc();
	int getFrame(double fTime, int nUseDisc);

	ErrorCode attach() override;
	ErrorCode detach() override;

	ErrorCode onRender(CBofBitmap *pBmp, CBofRect *pRect) override;

	CBagObject *onNewButtonObject(const CBofString &sInit) override;
};

} // namespace SpaceBar
} // namespace Bagel

#endif
