/*****************************************************************
*
*  Copyright (c) 1994 by Boffo Games, All Rights Reserved
*
*
*  main.h     -
*
*  HISTORY
*
*       1.00        05/23/94    BCW     Created this file
*
*  MODULE DESCRIPTION:
*
*       Public header for the main module for
*
*  RELEVANT DOCUMENTATION:
*
*
*
****************************************************************/
#ifndef MAIN_H
#define MAIN_H

#include "bagel/afxwin.h"

#include "bagel/hodjnpodj/hnplibs/stdinc.h"
#include "sprite.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/boflib/error.h"
#include "bagel/boflib/sound.h"
#include "barb.h"
#include "animate.h"

class CMainWindow : public CFrameWnd
{
	private:
        CBarber     *m_pBarb;
		CSound		*m_pMIDISound;
		CSound		*m_pWavSound;
		CAnimate	*m_pAnim;
        BOOL         m_bMIDIPlaying;

    public:
        CMainWindow(VOID);
        VOID        PlayGame(VOID);
        VOID        PaintScreen(VOID);
        VOID        LoadIniSettings(VOID);
        VOID        SaveIniSettings(VOID);
		void		FlushInputEvents();
		long		OnMCINotify(WPARAM, LPARAM);
		long		OnMMIONotify(WPARAM, LPARAM);

    protected:
        VOID        GameReset(VOID);
        VOID        GamePause(VOID);
        VOID        GameResume(VOID);
        virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
        VOID        HandleError(ERROR_CODE);
        VOID        DeleteSprite(CSprite *);

        VOID        OnPaint(VOID);
        VOID        OnTimer(UINT);
        VOID        OnMouseMove(UINT, CPoint);
        VOID        OnLButtonDown(UINT, CPoint);
		VOID		OnLButtonDblClk(UINT, CPoint);
        VOID        OnRButtonDown(UINT, CPoint);
        VOID        OnSysChar(UINT, UINT, UINT);
        VOID        OnSysKeyDown(UINT, UINT, UINT);
        VOID        OnKeyDown(UINT, UINT, UINT);
        VOID        OnActivate(UINT, CWnd *, BOOL);
        VOID        OnClose(VOID);
		VOID		OnLButtonUp(UINT,CPoint);

        DECLARE_MESSAGE_MAP()

        CRect       m_rNewGameButton;
        CPalette   *m_pGamePalette;
        CSprite    *m_pScrollSprite;
        BOOLEAN     m_bGameActive;
        BOOLEAN     m_bPause;
        BOOLEAN     m_bInMenu;
};

////
//
// CTheApp:
//
//class CTheApp : public CWinApp
//{
//    public:
//        BOOL InitInstance();
//};

#endif // !MAIN_H
