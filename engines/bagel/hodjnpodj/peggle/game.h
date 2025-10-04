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

#ifndef HODJNPODJ_PEGGLE_GAME_H
#define HODJNPODJ_PEGGLE_GAME_H

#include "bagel/hodjnpodj/hnplibs/sprite.h"
#include "bagel/boflib/sound.h"

namespace Bagel {
namespace HodjNPodj {
namespace Peggle {

#define NARRATIVESPEC   ".\\SOUND\\PEGG.WAV"
#define RULESSPEC       "PEGGLE.TXT"

// Button positioning constants
#define QUIT_BUTTON_WIDTH       50
#define QUIT_BUTTON_HEIGHT      20
#define QUIT_BUTTON_OFFSET_X    10
#define QUIT_BUTTON_OFFSET_Y    10

#define RESTART_BUTTON_WIDTH    63
#define RESTART_BUTTON_HEIGHT   39
#define RESTART_BUTTON_OFFSET_X 10
#define RESTART_BUTTON_OFFSET_Y 150

#define INIT_BUTTON_WIDTH       80
#define INIT_BUTTON_HEIGHT      30
#define INIT_BUTTON_OFFSET_X    280
#define INIT_BUTTON_OFFSET_Y    20

void SetUpBoard(CDC *pDC);
void UndoMove(CDC *pDC);

/////////////////////////////////////////////////////////////////////////////

// CMainWindow:
// See game.cpp for the code to the member functions and the message map.
//
class CMainWindow : public CFrameWnd {
public:
	CMainWindow(HWND);

	void initStatics();
	void SplashScreen();

	static  void UpdatePegPosition(CDC *pDC, CSprite *pSprite, int x, int y);
	static  CPoint GridToPoint(int i, int j);
	static  CPoint PointToGrid(CPoint point);
	static  void UndoMove(CDC *pDC);
	void UndoTurn();

private:
	bool    m_bProgramActive = false;
	HWND    m_hCallAppWnd = nullptr;
	CRect   ScrollRect;

	void    OnSoundNotify(CSound *pSound);

protected:
	virtual bool OnCommand(WPARAM wParam, LPARAM lParam) override;

	//{{AFX_MSG( CMainWindow )
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(unsigned int, CPoint);
	afx_msg void OnLButtonUp(unsigned int, CPoint);
	afx_msg void OnLButtonDblClk(unsigned int, CPoint);
	afx_msg void OnMButtonDown(unsigned int, CPoint);
	afx_msg void OnMButtonUp(unsigned int, CPoint);
	afx_msg void OnMButtonDblClk(unsigned int, CPoint);
	afx_msg void OnRButtonDown(unsigned int, CPoint);
	afx_msg void OnRButtonUp(unsigned int, CPoint);
	afx_msg void OnRButtonDblClk(unsigned int, CPoint);
	afx_msg void OnMouseMove(unsigned int nFlags, CPoint point);
	afx_msg void OnChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags);
	afx_msg void OnSysChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags);
	afx_msg void OnActivateApp(bool bActive, HTASK hTask);
	afx_msg LRESULT OnMCINotify(WPARAM, LPARAM);
	afx_msg LRESULT OnMMIONotify(WPARAM, LPARAM);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

} // namespace Peggle
} // namespace HodjNPodj
} // namespace Bagel

#endif
