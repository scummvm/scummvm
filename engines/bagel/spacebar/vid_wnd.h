
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
#include "bagel/baglib/button_object.h"
#include "bagel/baglib/character_object.h"

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

	void OnLButtonUp(uint32 nFlags, CBofPoint *xPoint, void *pInfo = nullptr) override;
};


class SBarVidWnd : public CMainWindow {
protected:
	CBagCharacterObject *m_pMovie;

	CBagVar *m_pTimerVar;
	CBagVar *m_pPlayingVar;
	CBagVar *m_pDiscVar;

	double              m_fTimerDiff;
	double              m_fTimer;
	INT                 m_nStartTime;

public:
	SBarVidWnd();
	~SBarVidWnd();

	void                SetPlayMode(INT nMode);
	void                SetInc(double fDiff) {
		m_fTimerDiff = fDiff;
	}

	BOOL                HasDisc();
	INT                 GetFrame(double fTime, INT nUseDisc);

	virtual ERROR_CODE  Attach();
	virtual ERROR_CODE  Detach();

	ERROR_CODE          OnRender(CBofBitmap *pBmp, CBofRect *pRect);

	CBagObject *OnNewButtonObject(const CBofString &sInit);
};

} // namespace SpaceBar
} // namespace Bagel

#endif
