/**
*   MAIN.H      -
*
*
*   Description -
*
*
*
*
*   (c) Copyright 1994 - Boffo Games
*   All rights reserved.
*
*
*   Revision History:
*
*   Version     Date        Author      Comments
*   -------     --------    ------      --------------------------------
*   1.00a       03-16-94     BCW        Created this file
*
**/
#ifndef MAIN_H
#define MAIN_H
#include "bagel/afxwin.h"

#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/hodjnpodj/hnplibs/sprite.h"
#include "bagel/boflib/error.h"
#include "bagel/hodjnpodj/hnplibs/stdinc.h"
#include "bagel/boflib/sound.h"
#include "guess.h"

#define N_CATEGORIES    4

class CMainWindow : public CFrameWnd
{
    public:
        CMainWindow();
        VOID        PlayGame(void);
        VOID        PaintScreen(VOID);
        VOID        PaintCategory(INT);
        VOID        EraseCategory(VOID);

        int         m_nTurnCount;

    protected:
        ERROR_CODE  LoadCategoryNames(VOID);
        VOID        ReleaseCategoryNames(VOID);

        virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
        VOID HandleError(ERROR_CODE);
		VOID OnSoundNotify(CSound *pSound);

        void OnPaint(void);
        void OnChar(UINT, UINT, UINT);
        void OnSysChar(UINT, UINT, UINT);
        void OnKeyDown(UINT, UINT, UINT);
        void OnSysKeyDown(UINT, UINT, UINT);
        void OnMouseMove(UINT, CPoint);
        void OnLButtonDown(UINT, CPoint);
        void CMainWindow::OnActivate(UINT, CWnd *, BOOL);
        void OnSetFocus(CWnd *);
        void OnClose(void);
	   	long OnMCINotify( WPARAM, LPARAM);
	   	long OnMMIONotify( WPARAM, LPARAM);

        DECLARE_MESSAGE_MAP()

        CRect        m_rNewGameButton;
        CSprite     *m_pCategories[N_CATEGORIES];
        CSprite     *m_pScrollSprite;
        CGuessDlg   *m_pDlgGuess;
        CSound      *m_pSoundTrack;
        INT          m_iLastType;
        BOOL         m_bInMenu;
        BOOL         m_bInGuess;
};

#endif // !MAIN_H
