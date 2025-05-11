// game.h : Declares the class interfaces for the application.
//         game is a simple program which consists of a main window
//         and an "About" dialog which can be invoked by a menu choice.
//         It is intended to serve as a starting-point for new
//         applications.
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#ifndef __game_H__
#define __game_H__

#include <sound.h>
#include "gamedll.h"

#define SPLASHSPEC			".\\art\\backgrnd.BMP"

static const char *curly[5] = {
	".\\art\\Colony1.BMP",
	".\\art\\Colony2.BMP",
	".\\art\\Colony3.BMP",
	".\\art\\Colony4.BMP",
	".\\art\\Colony5.BMP" };

#define CURLY_X			25		// Colony dimensions
#define CURLY_Y			24

#define	VILLAGES_PLACED	10		// Number of colonies pre-placed in meta-game mode
#define	PLACE_COLS		10
#define	PLACE_ROWS		10
#define	OFFSET_X		 6
#define	OFFSET_Y		 4

static const char *months[12] = {
 	".\\art\\jan.BMP",
	".\\art\\feb.BMP",
	".\\art\\mar.BMP",
	".\\art\\apr.BMP",
	".\\art\\may.BMP",
	".\\art\\jun.BMP",
	".\\art\\jul.BMP",
	".\\art\\aug.BMP",
	".\\art\\sep.BMP",
	".\\art\\oct.BMP",
	".\\art\\nov.BMP",
	".\\art\\dec.BMP" };

#define	SCROLL_BUTTON 	".\\art\\SCROLBTN.BMP"	// scroll commands button bmp
#define CALENDAR_BMP	".\\ART\\CALENDAR.BMP"	// the blank calendar artwork

// stat info
#define	STATS_COLOR				RGB(255, 255, 0) 	// color of visual game stats
#define	MONTH_COL_POS			17 					// sprite calendar pos
#define	MONTH_ROW_POS			140

#define	SCORE_LEFT_COL			459			// score pos
#define SCORE_LEFT_ROW			30
#define SCORE_RIGHT_COL			574
#define SCORE_RIGHT_ROW			52

#define	CURRENT_LEFT_COL		44			// current villages pos
#define	CURRENT_LEFT_ROW		30
#define CURRENT_RIGHT_COL		220
#define CURRENT_RIGHT_ROW		52

#define	ROUND_LEFT_COL			15			// years past pos
#define	ROUND_LEFT_ROW			315
#define	ROUND_RIGHT_COL			62
#define	ROUND_RIGHT_ROW     	329

#define	ROUND_TEXT1_LEFT_COL	15
#define	ROUND_TEXT1_LEFT_ROW	330
#define	ROUND_TEXT1_RIGHT_COL	62
#define	ROUND_TEXT1_RIGHT_ROW	350

#define	ROUND_TEXT2_LEFT_COL	15
#define	ROUND_TEXT2_LEFT_ROW	351
#define	ROUND_TEXT2_RIGHT_COL	62
#define	ROUND_TEXT2_RIGHT_ROW	371

#define VILLAGE_LEFT_COL		15			// villages left pos
#define VILLAGE_LEFT_ROW    	423
#define VILLAGE_RIGHT_COL   	62
#define VILLAGE_RIGHT_ROW		437

#define	VILLAGE_TEXT1_LEFT_COL		15
#define	VILLAGE_TEXT1_LEFT_ROW		438
#define	VILLAGE_TEXT1_RIGHT_COL		62
#define	VILLAGE_TEXT1_RIGHT_ROW		459

// Button Identifier codes (BIDs)
#define IDC_EVOLVE	101
#define IDC_COMMAND	102

// Button positioning constants
#define QUIT_BUTTON_WIDTH			50
#define QUIT_BUTTON_HEIGHT			20
#define QUIT_BUTTON_OFFSET_X		10
#define QUIT_BUTTON_OFFSET_Y		10


#define COMMAND_BUTTON_WIDTH		80
#define COMMAND_BUTTON_HEIGHT		18
#define COMMAND_OFFSET_X			0
#define COMMAND_OFFSET_Y			0

#define EVOLVE_BUTTON_WIDTH			94
#define EVOLVE_BUTTON_HEIGHT		20
#define EVOLVE_BUTTON_OFFSET_X 		0
#define EVOLVE_BUTTON_OFFSET_Y		30

// Timer ID's
#define	EVOLVE_TIMER_ID				10001
#define	EVOLVE_INTERVAL				100 // decaseconds

/////////////////////////////////////////////////////////////////////////////

// CMainWindow:
// See game.cpp for the code to the member functions and the message map.

class CLife;

class CMainWindow : public CFrameWnd
{
private:
	CLife			*m_cLife;
//	CButton			*pEvolveButton,
//					*pCommandButton;
	LPGAMESTRUCT    m_lpGameStruct;
	HWND			m_hCallAppWnd;
	BOOL			m_bGameActive;
	CSound			*m_pSound;
    CRect			m_rNewGameButton;

public:
	CMainWindow( HWND, LPGAMESTRUCT );
	~CMainWindow();

	void SplashScreen();
	void DisplayStats();
	void RefreshStats();
	void NewGame();
	void GamePause();
	void GameResume();

protected:
virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	//{{AFX_MSG( CMainWindow )
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT,CPoint);
	afx_msg void OnLButtonDblClk(UINT,CPoint);
    afx_msg void OnSysKeyDown(UINT, UINT, UINT);
	afx_msg void OnSysChar(UINT, UINT, UINT);
	afx_msg void OnKeyDown(UINT, UINT, UINT);
	afx_msg void OnTimer(UINT);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg long OnMCINotify(WPARAM, LPARAM);
	afx_msg long OnMMIONotify(WPARAM, LPARAM);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

// CTheApp:
// See game.cpp for the code to the InitInstance member function.
//
class CTheApp : public CWinApp
{
public:
	BOOL InitInstance();
};

/////////////////////////////////////////////////////////////////////////////

#endif // __game_H__

/////////////////////////////////////////////////////////////////////////////
// CWindowMain frame

class CWindowMain : public CFrameWnd
{
	DECLARE_DYNCREATE(CWindowMain)
protected:
	CWindowMain();			// protected constructor used by dynamic creation

// Attributes
public:

// Operations
public:

// Implementation
protected:
	virtual ~CWindowMain();

	// Generated message map functions
	//{{AFX_MSG(CWindowMain)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
