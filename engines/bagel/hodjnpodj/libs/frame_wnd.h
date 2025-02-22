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

#ifndef HODJNPODJ_LIB_FRAME_WND_H
#define HODJNPODJ_LIB_FRAME_WND_H

#include "bagel/bagel.h"
#include "bagel/boflib/point.h"

namespace Bagel {
namespace HodjNPodj {

enum {
	SW_SHOWNORMAL
};

typedef int CDC;

class CFrameWnd {
protected:
	virtual bool OnCommand(uint16 wParam, int32 lParam) {
		return false;
	}

	//{{AFX_MSG( CMainWindow )
	virtual void OnPaint() {}
	virtual void OnChar(uint nChar, uint nRepCnt, uint nFlags) {}
	virtual void OnSysChar(uint nChar, uint nRepCnt, uint nFlags) {}
	virtual void OnSysKeyDown(uint nChar, uint nRepCnt, uint nFlags) {}
	virtual void OnKeyDown(uint nChar, uint nRepCnt, uint nFlags) {}
	virtual void OnTimer(uint nIDEvent) {}
	virtual void OnLButtonDown(uint nFlags, CPoint point) {}
	virtual void OnRButtonDown(uint nFlags, CPoint point) {}
	virtual void OnLButtonUp(uint nFlags, CPoint point) {}
	virtual void OnMouseMove(uint nFlags, CPoint point) {}
	virtual void OnClose() {}
	virtual long OnMCINotify(uint16, int32) { return 0; }
	virtual long OnMMIONotify(uint16, int32) { return 0; }

	void BeginWaitCursor() {}

public:
	virtual ~CFrameWnd() {}
	void ShowWindow(int);
	void UpdateWindow();
	void SetActiveWindow();
};

} // namespace HodjNPodj
} // namespace Bagel

#endif
