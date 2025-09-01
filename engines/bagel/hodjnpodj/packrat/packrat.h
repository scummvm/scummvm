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

#ifndef HODJNPODJ_PACKRAT_PACKRAT_H
#define HODJNPODJ_PACKRAT_PACKRAT_H

#define OINK

#include "bagel/boflib/sound.h"
#include "bagel/hodjnpodj/packrat/globals.h"
#include "bagel/hodjnpodj/packrat/resource.h"
#include "bagel/hodjnpodj/hnplibs/dibdoc.h"
#include "bagel/hodjnpodj/hnplibs/gamedll.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"            // header for the options library
#include "bagel/hodjnpodj/hnplibs/mainmenu.h"
#include "bagel/hodjnpodj/hnplibs/sprite.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/hnplibs/rules.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/hnplibs/bitmaps.h"

namespace Bagel {
namespace HodjNPodj {
namespace Packrat {

// Button Identifier codes (BIDs)
#define IDC_OPTION          119
#define IDC_JUNK            999

// Sound File Identifiers

// Rules File Identifiers
#define RULESFILE                   "PACKRAT.TXT"
#define RULES_WAV                   ".\\SOUND\\PACKRAT.WAV"

// Sound files:
#define GAME_THEME          ".\\SOUND\\PACRAT.MID"
#define DEAD_WAV            ".\\SOUND\\TAPS.WAV"
#define DOODAD_WAV          ".\\SOUND\\DOODAD.WAV"
#define LEVEL_WAV           ".\\SOUND\\APPLAUSE.WAV"
#define LASTLEV_WAV         ".\\SOUND\\WIN45.WAV"
// Bitmap Identifiers
#define OPTSCROLL                   "ART\\OPTSCRL.BMP"

// Button positioning constants
#define OPTION_WIDTH                146
#define OPTION_HEIGHT               23
#define OPTION_LEFT                 246
#define OPTION_TOP                  0

/////////////////////////////////////////////////////////////////////////////

// CMainPackRatWindow:
// See game.cpp for the code to the member functions and the message map.
//

class CMainPackRatWindow : public CFrameWnd {
public:
	int             m_nBadGuySpeed = 0;
	int             m_nPlayerSpeed = 0;
	int             m_nGameLevel = 0;
	int             m_nLives = 0;

private:
	HWND            m_hCallAppWnd = nullptr;
	LPGAMESTRUCT    m_lpGameStruct = nullptr;
	BOOL            m_bMouseCaptured = FALSE;
	int             *m_anMazeArray = 0;
	int             m_nPDirection = 0;
	POINT           m_ptCurrPLocInGrid = { 0, 0 };
	POINT           m_ptCurrentPPos = { 0, 0 };
	int             m_anBDirection[4] = { 0 };
	int             m_nNumberOfMoves = 0;
	int             m_nBDirection = 0;
	POINT           m_aptCurrBLocInGrid[4] = {
		{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
	};
	POINT           m_aptCurrentBPos[4] = {
		{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }
	};
	int             m_nNumberOfObjectsLeft = 0;
	long            m_lScore = 0;
	int             m_nMaze = 0;
	int             m_nNextDir = 0;
	BOOL            m_bSuspend = FALSE;
	CRect   MainRect;                           // screen area spanned by the game window
	CRect   OptionRect;                         // screen area spanned by the game window
	CPoint  ptBaloon;
	CPoint  ptLastMouseCoord;
	CPoint  ptCurrMouseCoord;
	CRect   rNewGame;
	CRect   ptScore;

public:
	CMainPackRatWindow(HWND, LPGAMESTRUCT);

	void initStatics();
	void SplashScreen();

	static  void ReleaseResources(void);
	static  void FlushInputEvents(void);
	void MainLoop();
	void ResetGame();
	int Power(int, int);

private:
	void    PickUpObject();
	void    PickUpSuperObject();
	void    SetNewPlayerPos();
	BOOL    SetNewBadGuyPos(int, BOOL);
	void    SetMaze();
	void    SetLevel();
	BOOL    PlayerKilled(int);
	void    KillPlayer(BOOL = FALSE);
	void    KillBadGuy(int, BOOL = FALSE);
	void    PaintObjects(int  = 1);
	void    MoveBadGuys();
	void    MovePlayer();
	BOOL    CheckMessages();
	void    OnSoundNotify(CSound *pSound);

protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;

	//{{AFX_MSG( CMainPackRatWindow )
	afx_msg void OnPaint();
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR);
	afx_msg void OnLButtonDown(UINT, CPoint);
	afx_msg void OnLButtonUp(UINT, CPoint);
	afx_msg void OnRButtonDown(UINT, CPoint);
	afx_msg void OnRButtonUp(UINT, CPoint);
	afx_msg void OnMouseMove(UINT, CPoint);
	afx_msg void OnKeyDown(UINT, UINT, UINT);
	afx_msg void OnSysKeyDown(UINT, UINT, UINT);
//	afx_msg void OnDrawItem( int, LPDRAWITEMSTRUCT );
	afx_msg BOOL OnEraseBkgnd(CDC *);
	afx_msg void OnActivate(UINT nState, CWnd   *pWndOther, BOOL bMinimized) override;
	afx_msg LRESULT OnMCINotify(WPARAM, LPARAM);
	afx_msg LRESULT OnMMIONotify(WPARAM, LPARAM);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

} // namespace Packrat
} // namespace HodjNPodj
} // namespace Bagel

#endif
