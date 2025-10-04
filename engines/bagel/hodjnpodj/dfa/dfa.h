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

#ifndef HODJNPODJ_DFA_DFA_H
#define HODJNPODJ_DFA_DFA_H

#include "bagel/boflib/sound.h"
#include "bagel/hodjnpodj/hnplibs/dibdoc.h"
#include "bagel/hodjnpodj/hnplibs/gamedll.h"
#include "bagel/hodjnpodj/dfa/globals.h"
#include "bagel/hodjnpodj/dfa/resource.h"
#include "bagel/hodjnpodj/hnplibs/cbofdlg.h"            // header for the options library
#include "bagel/hodjnpodj/hnplibs/mainmenu.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/hodjnpodj/hnplibs/rules.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/hnplibs/bitmaps.h"
#include "bagel/hodjnpodj/hnplibs/sprite.h"

namespace Bagel {
namespace HodjNPodj {
namespace DFA {

// Button Identifier codes (BIDs)
#define IDC_OPTION          100
#define IDC_LETTERBUTTON    200 // to 480
#define IDC_JUNK            999

// Border width & height
#define SIDE_BORDER         16
#define TOP_BORDER          20

// Rules File Identifiers
#define RULESFILE                   "DFA.TXT"
#define RULES_WAV                   ".\\SOUND\\DAMFUR.WAV"

// Sound files:
#define GAME_THEME          ".\\SOUND\\DAMFUR.MID"
#define HIT_1_WAV           ".\\SOUND\\OW1.WAV"
#define HIT_2_WAV           ".\\SOUND\\OW2.WAV"
#define HIT_3_WAV           ".\\SOUND\\OW3.WAV"
#define HIT_4_WAV           ".\\SOUND\\OW4.WAV"
#define HIT_5_WAV           ".\\SOUND\\OW5.WAV"
#define HIT_6_WAV           ".\\SOUND\\OW6.WAV"
#define HIT_7_WAV           ".\\SOUND\\OW7.WAV"
#define HIT_8_WAV           ".\\SOUND\\OW8.WAV"
#define HIT_9_WAV           ".\\SOUND\\OW9.WAV"
#define HIT_10_WAV          ".\\SOUND\\OW10.WAV"
#define HIT_11_WAV          ".\\SOUND\\OW11.WAV"
#define MISS_1_WAV          ".\\SOUND\\HAHA.WAV"
#define MISS_2_WAV          ".\\SOUND\\MISSED.WAV"
#define MISS_3_WAV          ".\\SOUND\\MISSEDME.WAV"
#define TICK_WAV            ".\\SOUND\\TICK.WAV"
#define TIME_WAV            ".\\SOUND\\BUZZER.WAV"
#define LAKE_WAV            ".\\SOUND\\LAKE.WAV"
#define MOUNT_WAV           ".\\SOUND\\YODEL.WAV"
#define BEE_WAV             ".\\SOUND\\BEE.WAV"
#define WATCH_WAV           ".\\SOUND\\CUCKOO.WAV"

#define NUM_HIT_SOUNDS      11      // Number of sounds available for a hit
#define NUM_MISS_SOUNDS     3       // Number of sounds available for a missed hit
#define NUM_BEAVERS         7       // Number of beavers to hit

// Audio EasterEgg area constants
#define LAKE_X          178
#define LAKE_Y          152
#define LAKE_DX         283
#define LAKE_DY         38

#define MOUNT_X         40
#define MOUNT_Y         40
#define MOUNT_DX        510
#define MOUNT_DY        80

#define FLOWERS_X       540
#define FLOWERS_Y       325
#define FLOWERS_DX      82
#define FLOWERS_DY      136

#define WATCH_X         437         // 436
#define WATCH_Y         401         // 401
#define WATCH_DX        70
#define WATCH_DY        60

// Bitmap Identifiers
#define OPTSCROLL           "ART\\SSCROLL.BMP"
#define BEAVER1             "ART\\BEAVER1.BMP"
#define BEAVER2             "ART\\BEAVER2.BMP"
#define BEAVER3             "ART\\BEAVER3.BMP"
#define BEAVER4             "ART\\BEAVER4.BMP"
#define BEAVER5             "ART\\BEAVER5.BMP"
#define BEAVER6             "ART\\BEAVER6.BMP"
#define BEAVER7             "ART\\BEAVER7.BMP"

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

class CMainDFAWindow : public CFrameWnd {
public:
	unsigned int m_nTimeForGame = 0;
	int m_nBeaverDuration = 0;

private:
	bool m_bPlaySounds = false;				// bool for am I playing a certain # of rounds
	HWND m_hCallAppWnd = nullptr;
	LPGAMESTRUCT m_lpGameStruct = nullptr;
	bool m_bMouseCaptured = false;
	long m_lScore = 0;
	CRect   MainRect;						// screen area spanned by the game window
	CRect   ArtRect;						// screen area inside the border trim
	CRect   OptionRect;						// screen area spanned by the option button
	CRect   rNewGame;						// screen area spanned by the name plate
	CRect   arBeaver[NUM_BEAVERS];

	CString aHitFile[NUM_HIT_SOUNDS];
	CString aMissFile[NUM_MISS_SOUNDS];

public:
	CMainDFAWindow(HWND, LPGAMESTRUCT);

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
	bool LoadBeaverSounds();
	void ReleaseBeaverSounds();
	//
	// Sound as resource dudes:
	//
	char *m_pHitSound[NUM_HIT_SOUNDS] = {};
	HANDLE m_hHitRes[NUM_HIT_SOUNDS] = {};
	char *m_pMissSound[NUM_MISS_SOUNDS] = {};
	HANDLE m_hMissRes[NUM_MISS_SOUNDS] = {};

	//{{AFX_MSG( CMainPackRatWindow )
	afx_msg void OnPaint();
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(uintptr);
	afx_msg void OnRButtonDown(unsigned int, CPoint);
	afx_msg void OnLButtonDown(unsigned int, CPoint);
	afx_msg void OnLButtonUp(unsigned int, CPoint);
	afx_msg void OnMouseMove(unsigned int, CPoint);
	afx_msg void OnSysKeyDown(unsigned int, unsigned int, unsigned int);
	afx_msg void OnKeyDown(unsigned int, unsigned int, unsigned int);
	afx_msg bool OnEraseBkgnd(CDC *);
	afx_msg void OnActivate(unsigned int nState, CWnd   *pWndOther, bool bMinimized) override;
	afx_msg LRESULT OnMCINotify(WPARAM, LPARAM);
	afx_msg LRESULT OnMMIONotify(WPARAM, LPARAM);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

} // namespace DFA
} // namespace HodjNPodj
} // namespace Bagel

#endif
