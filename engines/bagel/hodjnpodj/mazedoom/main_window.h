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

#ifndef HODJNPODJ_MAZEDOOM_MAIN_WINDOW_H
#define HODJNPODJ_MAZEDOOM_MAIN_WINDOW_H

#include "bagel/mfc/frame_wnd.h"

namespace Bagel {
namespace HodjNPodj {
namespace MazeDoom {

class CMainWindow : public CFrameWnd {
public:
	CMainWindow();
	void MovePlayer(CPoint point);
	void GetNewCursor();				// Load up the new directional cursor
	void NewGame();						// Set up a new game & start the timer

	//added data members:

	virtual void SplashScreen();

protected:
	/**
	 * This function is called when a WM_COMMAND message is issued,
	 * typically in order to process control related activities.
	 */
	bool OnCommand(uint16 wParam, int32 lParam) override;

	//{{AFX_MSG( CMainWindow )
	void OnPaint() override;
	void OnChar(uint nChar, uint nRepCnt, uint nFlags) override;
	void OnSysChar(uint nChar, uint nRepCnt, uint nFlags) override;
	void OnSysKeyDown(uint nChar, uint nRepCnt, uint nFlags) override;
	void OnKeyDown(uint nChar, uint nRepCnt, uint nFlags) override;
	void OnTimer(uint nIDEvent) override;
	void OnLButtonDown(uint nFlags, CPoint point) override;
	void OnRButtonDown(uint nFlags, CPoint point) override;
	void OnLButtonUp(uint nFlags, CPoint point) override;
	void OnMouseMove(uint nFlags, CPoint point) override;
	void OnClose() override;
	long OnMCINotify(uint16, int32) override;
	long OnMMIONotify(uint16, int32) override;
	//}}AFX_MSG

	//DECLARE_MESSAGE_MAP()
};

} // namespace MazeDoom
} // namespace HodjNPodj
} // namespace Bagel

#endif
