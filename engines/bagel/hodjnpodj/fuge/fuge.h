/*****************************************************************
*
*  Copyright (c) 1994 by Boffo Games, All Rights Reserved
*
*
*  fuge.h     -
*
*  HISTORY
*
*       1.00        06/21/94    BCW     Created this file
*
*  MODULE DESCRIPTION:
*
*       Public header for the main module for Fuge
*
*  RELEVANT DOCUMENTATION:
*
*
*
****************************************************************/
#ifndef FUGE_H
#define FUGE_H

#include <afxwin.h>
#include <afxext.h>
#include <stdinc.h>
#include <sprite.h>
#include <text.h>
#include <errors.h>
#include <button.h>
#include <sound.h>
#include <vector.h>

#define N_ROWS           6
#define BRICKS_PER_ROW  16
#define N_BRICKS        (N_ROWS*BRICKS_PER_ROW)

class CFugeWindow : public CFrameWnd
{
    public:
        CFugeWindow(VOID);
        VOID        PlayGame(VOID);
        VOID        PaintScreen(VOID);
        VOID        LoadIniSettings(VOID);

    protected:
        VOID        GameReset(VOID);
        VOID        GamePause(VOID);
        VOID        GameResume(VOID);
        VOID        HandleError(ERROR_CODE);
        VOID        RealignVectors(VOID);
        ERROR_CODE  LoadMasterSprites(VOID);
        VOID        ReleaseMasterSprites(VOID);
        ERROR_CODE  LoadMasterSounds(VOID);
        VOID        ReleaseMasterSounds(VOID);
        ERROR_CODE  LoadNewPaddle(INT);
        VOID        InitializeJoystick(VOID);
        VOID        RepaintSpriteList(CDC *);
        VOID        PaintBricks(CDC *);
        VOID        StartBall(VOID);
        VOID        EndBall(VOID);
        VOID        PaintBall(VOID);
        VOID        LaunchBall(VOID);
        VOID        StartPaddle(VOID);
        VOID        EndPaddle(VOID);
        VOID        PaintPaddle(BOOL);
        CVector     BallOnPaddle(VOID);
        VOID        StartBricks(VOID);
        VOID        EndBricks(VOID);
        VOID        EraseBrick(CDC *, INT);
        VOID        LoseBall(VOID);
        VOID        BallvsPaddle(VOID);
        VOID        BallvsBrick(DOUBLE);

        virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
        LONG        OnJoyStick(UINT, LONG);
        VOID        OnPaint(VOID);
        VOID        OnTimer(UINT);
        VOID        OnMouseMove(UINT, CPoint);
        VOID        OnRButtonUp(UINT, CPoint);
        VOID        OnLButtonDown(UINT, CPoint);
        VOID        OnSysChar(UINT, UINT, UINT);
        VOID        OnSysKeyDown(UINT, UINT, UINT);
        VOID        OnKeyDown(UINT, UINT, UINT);
        VOID        OnActivate(UINT, CWnd *, BOOL);
        LONG        OnMCINotify(WPARAM, LPARAM);
        LONG        OnMMIONotify(WPARAM, LPARAM);
        VOID        OnSoundNotify(CSound *);
        VOID        OnClose(VOID);

        DECLARE_MESSAGE_MAP()

        BOOL        m_bBrickVisible[N_BRICKS];
        CPoint      m_ptOrigin;
        CRect       m_rNewGameButton;
        CRect       m_rBlackHole;
        CVector     m_vBallVector;
        CVector     m_ptBallLocation;
        CPalette   *m_pGamePalette;
        CBmpButton *m_pScrollButton;
        CSprite    *m_pBall;
        CSprite    *m_pPaddle;
        CSound     *m_pSoundTrack;

        LPSTR       m_pBrickSound;
        LPSTR       m_pWallSound;
        LPSTR       m_pPaddleSound;
        LPSTR       m_pExtraLifeSound;
        HANDLE      m_hBrickRes;
        HANDLE      m_hWallRes;
        HANDLE      m_hPaddleRes;
        HANDLE      m_hExtraLifeRes;

        INT         m_nInitNumBalls;
        INT         m_nInitStartLevel;
        INT         m_nInitBallSpeed;
        INT         m_nInitPaddleSize;
        INT         m_nGForceFactor;

        DOUBLE      m_fTurboBoost;
        LONG        m_lScore;
        LONG        m_lExtraLifeScore;
        LONG        m_nJoyOrgX;
        LONG        m_nJoyOrgY;
        INT         m_nBricks;
        INT         m_nBalls;
        INT         m_nBallSpeed;
        INT         m_nNumRows;
        INT         m_nPaddleCelIndex;
        BOOL        m_bMovingPaddle;
        BOOL        m_bGameActive;
        BOOL        m_bPause;
        BOOL        m_bIgnoreScrollClick;
        BOOL        m_bPaddleHit;
        BOOL        m_bOutterWall;
        BOOL        m_bJoyActive;
        BOOL        m_bBallOnPaddle;
};

#endif // !FUGE_H
