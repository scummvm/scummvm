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

#ifndef HODJNPODJ_WORDSEARCH_WORDSEARCH_H
#define HODJNPODJ_WORDSEARCH_WORDSEARCH_H

#include "bagel/boflib/sound.h"
#include "bagel/hodjnpodj/hnplibs/dibdoc.h"
#include "bagel/hodjnpodj/hnplibs/gamedll.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"            // header for the options library
#include "bagel/hodjnpodj/hnplibs/mainmenu.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/hnplibs/rules.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/hnplibs/bitmaps.h"
#include "bagel/hodjnpodj/hnplibs/sprite.h"
#include "bagel/hodjnpodj/wordsearch/globals.h"
#include "bagel/hodjnpodj/wordsearch/resource.h"
#include "bagel/hodjnpodj/wordsearch/wordlist.h"

namespace Bagel {
namespace HodjNPodj {
namespace WordSearch {

// Button Identifier codes (BIDs)
#define IDC_OPTION          100
#define IDC_LETTERBUTTON    200 // to 480
#define IDC_JUNK            999

// Art File Identifiers
#define     SPLASHSPEC      ".\\ART\\WORDSRCH.BMP"
#define     ALLLETTERS      ".\\ART\\LETTERS.BMP"
#define     TIMERSPRITE     ".\\ART\\DASYTIMR.BMP"

// Timer info
#define     GAMETIMER       99
#define     TIMERSPRITECELS 13

// Rules File Identifiers
#define RULESFILE                   "WORDSRCH.TXT"
#define RULES_WAV                   ".\\SOUND\\WORDSRCH.WAV"

// Sound files:
#define GAME_THEME          ".\\SOUND\\WORDSRCH.MID"
#define COW_WAV             ".\\SOUND\\SLURP2.WAV"
#define CHICKEN_WAV         ".\\SOUND\\BRAWK.WAV"
#define PIG_WAV             ".\\SOUND\\OINK.WAV"
#define FLOWER_WAV          ".\\SOUND\\LOVESME.WAV"
#define FIND_WAV            ".\\SOUND\\TINYBELL.WAV"
#define NOPE_WAV            ".\\SOUND\\NOPE.WAV"
#define TRYAGAIN_WAV        ".\\SOUND\\TRYAGAIN.WAV"
#define TICK_WAV            ".\\SOUND\\TICK.WAV"
#define TIMEOUT_WAV         ".\\SOUND\\BUZZER.WAV"
#define ALLFOUND_WAV        ".\\SOUND\\APPLAUSE.WAV"

// Anim files:
#define COW_ANIM            ".\\ART\\COW.BMP"
#define CHICKEN_ANIM        ".\\ART\\CHICKEN.BMP"
#define PIG_ANIM            ".\\ART\\PIG.BMP"

// Easter Egg info
#define COW_X               40  //38
#define COW_Y               47  //43
#define COW_DX              132
#define COW_DY              182
#define NUM_COW_CELS        13
#define COW_SLEEP           100 //100     150 for slurp.wav

#define CHICKEN_X           16  //14
#define CHICKEN_Y           165 //156
#define CHICKEN_DX          62  //82
#define CHICKEN_DY          53  //71
#define NUM_CHICKEN_CELS    11  //22
#define CHICKEN_SLEEP       175

#define PIG_X               43
#define PIG_Y               343
#define PIG_DX              153
#define PIG_DY              122
#define NUM_PIG_CELS        28
#define PIG_SLEEP           100

#define FLOWER_X            513
#define FLOWER_Y            388
#define FLOWER_DX           110
#define FLOWER_DY            75

// Bitmap Identifiers
#define OPTSCROLL                   "ART\\OPTSCRL.BMP"

// Button positioning constants
#define OPTION_WIDTH                146
#define OPTION_HEIGHT               23
#define OPTION_LEFT                 246
#define OPTION_TOP                  0

#define     NUMBEROFCOLS    20
#define     NUMBEROFROWS    14

/////////////////////////////////////////////////////////////////////////////

// CMainPackRatWindow:
// See game.cpp for the code to the member functions and the message map.
//

class CMainWSWindow : public CFrameWnd {
public:
	unsigned int            m_nTimeForGame;
	bool            m_bShowWordList;
	bool            m_bWordsForwardOnly;

private:
	bool            m_bPlaySounds;            // bool for am I playing a certain # of rounds
	HWND            m_hCallAppWnd;
	LPGAMESTRUCT    m_lpGameStruct;
	bool            m_bMouseCaptured;
	long            m_lScore;
	CString         m_alpszCurrectGrid[NUMBEROFROWS * NUMBEROFCOLS];
	bool            m_bNoGrid;
	CRect   MainRect;                           // screen area spanned by the game window
	CRect   OptionRect;                         // screen area spanned by the option button
	CRect   rNewGame;
	CRect   rRefreshRect;
	CRect   arLetterButt[NUMBEROFROWS * NUMBEROFCOLS];
	CRect   arScreenGrid[NUMBEROFROWS][NUMBEROFCOLS];
	CRect       rScore;
	CString     astrCurrentDisplay[NUMBEROFROWS];

	char    acGameGrid[NUMBEROFROWS][NUMBEROFCOLS];
	CRect   arWordDisplay[WORDSPERLIST];

	CString _strGameList[WORDSPERLIST];
	CString _strGameListDisplay[WORDSPERLIST];
	CText _txtDisplayWord[WORDSPERLIST];

public:
	CMainWSWindow(HWND, LPGAMESTRUCT);

	void SplashScreen();

	static  void ReleaseResources();
	static  void FlushInputEvents();
	void MainLoop();
	void ResetGame();
	void ClearGrid();
	void LoadNewGrid(int = 1);
	void CreateNewGrid();

private:
	void OnSoundNotify(CSound *pSound);

protected:
	virtual bool OnCommand(WPARAM wParam, LPARAM lParam) override;

	//{{AFX_MSG( CMainPackRatWindow )
	afx_msg void OnPaint();
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(uintptr);
	afx_msg void OnLButtonDown(unsigned int, CPoint);
	afx_msg void OnLButtonUp(unsigned int, CPoint);
	afx_msg void OnRButtonDown(unsigned int, CPoint);
	afx_msg void OnMouseMove(unsigned int, CPoint);
	afx_msg void OnKeyDown(unsigned int, unsigned int, unsigned int);
	afx_msg void OnSysKeyDown(unsigned int, unsigned int, unsigned int);
	afx_msg bool OnEraseBkgnd(CDC *);
	afx_msg void OnActivate(unsigned int nState, CWnd   *pWndOther, bool bMinimized) override;
	afx_msg LRESULT OnMCINotify(WPARAM, LPARAM);
	afx_msg LRESULT OnMMIONotify(WPARAM, LPARAM);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

} // namespace Poker
} // namespace HodjNPodj
} // namespace Bagel

#endif
