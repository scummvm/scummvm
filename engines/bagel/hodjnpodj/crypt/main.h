/*****************************************************************
 * Copyright (c) 1994 by Boffo Games, All Rights Reserved
 *
 *
 * main.h
 *
 * HISTORY
 *
 *      1.00      03/28/94     JSC     Initial release
 *
 * MODULE DESCRIPTION:
 *
 *      [Describe the function of the module]
 *
 * RELEVANT DOCUMENTATION:
 *
 *      [Specifications, documents, test plans, etc.]
 *
 ****************************************************************/

#ifndef __game_H__
#define __game_H__

/////////////////////////////////////////////////////////////////////////////

// CMainWindow:
// See game.cpp for the code to the member functions and the message map.
//
#include "bagel/boflib/sound.h"
#include "gamedll.h"
#include "crypt.h"

#define NEW_GAME_LEFT	15
#define NEW_GAME_TOP	2
#define NEW_GAME_RIGHT	233
#define NEW_GAME_BOTTOM	21     

#define PAUSE_TIME		15000				// Pause for 15 seconds -- allows user to read quote

// Game theme song
#define	GAME_THEME		".\\sound\\crypt.mid"
// Rules files
#define	RULES_TEXT		"CRYPTRUL.TXT"
#define RULES_WAV		".\\SOUND\\CRYPT.WAV"

///////////////////////////////////////////
// EASTER EGG AND SOUND HOTSPOT DATA:
//
#define	WAV_TIMEOUT	".\\SOUND\\BUZZER.WAV"
#define	WAV_WON		".\\SOUND\\SOUNDWON.WAV"
#define	WAV_URN1	".\\SOUND\\OUTTHERE.WAV"
#define	WAV_URN2	".\\SOUND\\SPOOKMUS.WAV"
#define	WAV_URN3	".\\SOUND\\BUTFLIES.WAV"
#define	WAV_JOKE1	".\\SOUND\\JOKE1.WAV"
#define	WAV_JOKE2	".\\SOUND\\JOKE2.WAV"
#define	WAV_JOKE3	".\\SOUND\\JOKE3.WAV"
#define	WAV_JOKE4	".\\SOUND\\JOKE4.WAV"
#define	WAV_JOKE5	".\\SOUND\\JOKE5.WAV"
#define	WAV_JOKE6	".\\SOUND\\JOKE6.WAV"
#define	WAV_TORCH	".\\SOUND\\FIREENG.WAV"
#define	WAV_GRYPH	".\\SOUND\\GRYPHON.WAV"
#define	WAV_STEPS	".\\SOUND\\STAIRS.WAV"
#define	WAV_HOUR	".\\SOUND\\SANDTIME.WAV" 

#define NUM_SKULL_SOUNDS		6

// Skull anim offsets
#define SKULL_X				21	//15
#define SKULL_Y				257	//245
#define	SKULL_DX			97
#define SKULL_DY			135

// URN1 anim offsets
#define URN1_X				98	//95
#define URN1_Y				28	//22
#define	URN1_DX				42	//54
#define URN1_DY				133	//48

// URN2 anim offsets
#define URN2_X				204	//186
#define URN2_Y				25	//24
#define	URN2_DX				94	//120
#define URN2_DY				104	//106

// URN3 anim offsets
#define URN3_X				361	//357
#define URN3_Y				26	//23
#define	URN3_DX				72	//84
#define URN3_DY				103	//108

// Anim cel count
#define NUM_SKULL_CELS		20
#define NUM_URN1_CELS		25
#define NUM_URN2_CELS		22
#define NUM_URN3_CELS		20

// Anim sleep times in millisecs
#define	SKULL_SLEEP			100
#define JOKE1_SLEEP			130			// funny thing
#define JOKE2_SLEEP			130			// pj's part A
#define JOKE2B_SLEEP		120			// pj's part B
#define JOKE3_SLEEP			120			// audience
#define JOKE4_SLEEP			130			// brooklyn
#define JOKE5_SLEEP			150			// philly
#define JOKE6_SLEEP			40			// sock it to me
#define	URN1_SLEEP			150
#define	URN2_SLEEP			200
#define	URN3_SLEEP			125

// TORCH audio hotspot offsets

#define TORCH1_X			45
#define TORCH1_Y			25
#define TORCH2_X			170
#define TORCH2_Y			40
#define TORCH3_X			307
#define TORCH3_Y			35
#define TORCH4_X			450
#define TORCH4_Y			40
#define	TORCH_DX			50
#define TORCH_DY			80

// GRYPH audio hotspot offsets

#define GRYPH_X				208
#define GRYPH_Y				130
#define	GRYPH_DX			240
#define GRYPH_DY			45

// STEPS audio hotspot offsets

#define STEPS_X				568
#define STEPS_Y				198
#define	STEPS_DX			42
#define STEPS_DY			183

// HOUR audio hotspot offsets
#define HOUR_X			473
#define HOUR_Y			123
#define	HOUR_DX			40
#define HOUR_DY			61

#define	MAX_HOURS		10


class CMainWindow : public CFrameWnd
{
public:
	CMainWindow( HWND, LPGAMESTRUCT);
	~CMainWindow();

	void 		SplashScreen();
	static void FlushInputEvents(void);
	void		DisplayStats(CDC*);
	void		RefreshStats();
	void		GameWin();
	void		GameLose();

private:
	HWND			m_hCallAppWnd;
	LPGAMESTRUCT    m_lpGameStruct;
	CRect		MainRect;				// screen area spanned by the game window
	CRect		m_cNewGame;				// area spanned by new game rect
//	CCryptogram	*m_cCryptograms;		// cryptogram game object
	CBmpButton	*m_pScrollButton;		// scroll button
	BOOL		m_bIgnoreScrollClick;	// scroll button
	BOOL		m_bIsFirstTimeHack;
	CSprite 	*m_pHourGlass;
//	int			m_nTimer;
//	BOOL		m_bPause;

private:
  	VOID 		OnSoundNotify(CSound *pSound);

protected:
virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	//{{AFX_MSG( CMainWindow )
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT,CPoint);
	afx_msg void OnLButtonUp(UINT,CPoint);
	afx_msg void OnLButtonDblClk(UINT,CPoint);
	afx_msg void OnMButtonDown(UINT,CPoint);
	afx_msg void OnMButtonUp(UINT,CPoint);
	afx_msg void OnMButtonDblClk(UINT,CPoint);
	afx_msg void OnRButtonDown(UINT,CPoint);
	afx_msg void OnRButtonUp(UINT,CPoint);
	afx_msg void OnRButtonDblClk(UINT,CPoint);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
	afx_msg void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg long OnMCINotify( WPARAM, LPARAM);
    afx_msg long OnMMIONotify( WPARAM, LPARAM);
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
	int ExitInstance();
};

/////////////////////////////////////////////////////////////////////////////

#endif // __game_H__

