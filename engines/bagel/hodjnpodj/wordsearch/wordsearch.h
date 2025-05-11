/*****************************************************************
 * Copyright (c) 1994 by Boffo Games, All Rights Reserved
 *
 *
 * wordsrch.h
 *
 * HISTORY
 *
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

#ifndef __wordsrch_H__
#define __wordsrch_H__

#include <time.h>
#include <sound.h>
#include "globals.h"
#include "resource.h"
#include "dibdoc.h"
#include "gamedll.h"
#include "cbofdlg.h"            // header for the options library
#include "mainmenu.h"

#include "button.h"
#include "rules.h"
#include "text.h"
#include "bitmaps.h"
#include "sprite.h"

#include "copyrite.h"						// mandatory internal copyright notice

// Button Identifier codes (BIDs)
#define	IDC_OPTION			100  
#define IDC_LETTERBUTTON	200	// to 480
#define	IDC_JUNK			999

// Art File Identifiers
#define 	SPLASHSPEC		".\\ART\\WORDSRCH.BMP"
#define 	ALLLETTERS		".\\ART\\LETTERS.BMP"
#define		TIMERSPRITE		".\\ART\\DASYTIMR.BMP"

// Timer info
#define		GAMETIMER		99              
#define		TIMERSPRITECELS	13

// Rules File Identifiers
#define RULESFILE					"WORDSRCH.TXT"
#define	RULES_WAV					".\\SOUND\\WORDSRCH.WAV"

// Sound files:
#define	GAME_THEME			".\\SOUND\\WORDSRCH.MID"
#define	COW_WAV				".\\SOUND\\SLURP2.WAV"
#define	CHICKEN_WAV			".\\SOUND\\BRAWK.WAV"
#define	PIG_WAV				".\\SOUND\\OINK.WAV"
#define	FLOWER_WAV			".\\SOUND\\LOVESME.WAV"
#define	FIND_WAV			".\\SOUND\\TINYBELL.WAV"
#define	NOPE_WAV			".\\SOUND\\NOPE.WAV"
#define	TRYAGAIN_WAV		".\\SOUND\\TRYAGAIN.WAV"
#define	TICK_WAV			".\\SOUND\\TICK.WAV"
#define	TIMEOUT_WAV			".\\SOUND\\BUZZER.WAV"
#define	ALLFOUND_WAV		".\\SOUND\\APPLAUSE.WAV"

// Anim files:
#define	COW_ANIM			".\\ART\\COW.BMP"
#define	CHICKEN_ANIM		".\\ART\\CHICKEN.BMP"
#define	PIG_ANIM			".\\ART\\PIG.BMP"

// Easter Egg info
#define	COW_X 				40	//38
#define	COW_Y				47	//43
#define	COW_DX				132
#define	COW_DY				182
#define	NUM_COW_CELS		13
#define	COW_SLEEP			100	//100     150 for slurp.wav

#define	CHICKEN_X 			16	//14
#define	CHICKEN_Y			165	//156
#define	CHICKEN_DX			62	//82
#define	CHICKEN_DY			53	//71
#define	NUM_CHICKEN_CELS	11	//22
#define	CHICKEN_SLEEP		175

#define	PIG_X 				43
#define	PIG_Y				343
#define	PIG_DX				153
#define	PIG_DY				122
#define	NUM_PIG_CELS		28
#define	PIG_SLEEP			100

#define	FLOWER_X 			513
#define	FLOWER_Y			388
#define	FLOWER_DX			110
#define	FLOWER_DY			 75

// Bitmap Identifiers
#define OPTSCROLL					"ART\\OPTSCRL.BMP"

// Button positioning constants
#define OPTION_WIDTH				146
#define OPTION_HEIGHT				23
#define OPTION_LEFT					246
#define OPTION_TOP					0

#define		NUMBEROFCOLS	20
#define		NUMBEROFROWS	14

/////////////////////////////////////////////////////////////////////////////

// CMainPackRatWindow:
// See game.cpp for the code to the member functions and the message map.
//              

class CMainWSWindow : public CFrameWnd
{
public:
UINT			m_nTimeForGame;
BOOL			m_bShowWordList;
BOOL			m_bWordsForwardOnly;

private:
BOOL			m_bPlaySounds;            // bool for am I playing a certain # of rounds
HWND			m_hCallAppWnd;
LPGAMESTRUCT	m_lpGameStruct;
BOOL			m_bMouseCaptured;
long			m_lScore;
CString			m_alpszCurrectGrid[NUMBEROFROWS*NUMBEROFCOLS];
BOOL			m_bNoGrid;

public:
	CMainWSWindow( HWND, LPGAMESTRUCT );

	void SplashScreen();

static	void ReleaseResources(void);
static	void FlushInputEvents(void);
		void MainLoop();
		void ResetGame();
		void ClearGrid();
		void LoadNewGrid( int =1 );
		void CreateNewGrid();

private:
		void OnSoundNotify(CSound *pSound);

protected:
virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	//{{AFX_MSG( CMainPackRatWindow )
	afx_msg void OnPaint();
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnTimer( UINT );
  	afx_msg void OnLButtonDown( UINT, CPoint );
  	afx_msg void OnLButtonUp( UINT, CPoint );
  	afx_msg void OnRButtonDown( UINT, CPoint );
	afx_msg void OnMouseMove( UINT, CPoint );
	afx_msg void OnKeyDown( UINT, UINT, UINT);
	afx_msg void OnSysKeyDown( UINT, UINT, UINT);
	afx_msg BOOL OnEraseBkgnd( CDC * );
	afx_msg void OnActivate(UINT nState, CWnd	*pWndOther, BOOL bMinimized);
	afx_msg	long OnMCINotify( WPARAM, LPARAM);
	afx_msg	long OnMMIONotify( WPARAM, LPARAM);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};


#endif // __wordsrch_H__

