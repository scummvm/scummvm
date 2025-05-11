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

#include "sprite.h"
#include "sound.h"

#define NARRATIVESPEC	".\\SOUND\\PEGG.WAV"
#define RULESSPEC		"PEGGLE.TXT"

// Button positioning constants
#define QUIT_BUTTON_WIDTH		50
#define QUIT_BUTTON_HEIGHT		20
#define QUIT_BUTTON_OFFSET_X	10
#define QUIT_BUTTON_OFFSET_Y	10

#define RESTART_BUTTON_WIDTH	63
#define RESTART_BUTTON_HEIGHT	39
#define RESTART_BUTTON_OFFSET_X	10
#define RESTART_BUTTON_OFFSET_Y	150

#define INIT_BUTTON_WIDTH		80
#define INIT_BUTTON_HEIGHT		30
#define INIT_BUTTON_OFFSET_X	280
#define INIT_BUTTON_OFFSET_Y	20

void SetUpBoard(CDC *pDC);
void UndoMove(CDC *pDC);

/////////////////////////////////////////////////////////////////////////////

// CMainWindow:
// See game.cpp for the code to the member functions and the message map.
//
class CMainWindow : public CFrameWnd
{
public:
	CMainWindow( HWND );

void SplashScreen();

static 	void UpdatePegPosition(CDC *pDC, CSprite *pSprite, int x, int y);
static 	CPoint GridToPoint(int i,int j);
static 	CPoint PointToGrid(CPoint point);
static 	void UndoMove(CDC *pDC);
		void UndoTurn(void);

private:
	BOOL	m_bProgramActive;
	HWND	m_hCallAppWnd;
	void 	OnSoundNotify(CSound *pSound);

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
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
	afx_msg void OnActivateApp(BOOL bActive, HTASK hTask);
    afx_msg long OnMCINotify( WPARAM, LPARAM);
    afx_msg long OnMMIONotify( WPARAM, LPARAM);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

// CTheApp:
// See game.cpp for the code to the InitInstance member function.
//
//class CTheApp : public CWinApp
//{
//public:
//	BOOL InitInstance();
//static	BOOL ExitInstance();
//};

/////////////////////////////////////////////////////////////////////////////

#endif // __game_H__

