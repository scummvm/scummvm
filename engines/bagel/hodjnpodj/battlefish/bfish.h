/*****************************************************************
*
*  Copyright (c) 1994 by Boffo Games, All Rights Reserved
*
*
*  bfish.h    -  Main header for Battle Fish
*
*  HISTORY
*
*       1.00        05/26/94    BCW     Created this file
*
*  MODULE DESCRIPTION:
*
*       Public header for the main module for BattleFish
*
*  RELEVANT DOCUMENTATION:
*
*
*
****************************************************************/
#ifndef BFISH_H
#define BFISH_H

#include "bagel/afxwin.h"

#include "bagel/hodjnpodj/hnplibs/stdinc.h"
#include "bagel/hodjnpodj/hnplibs/sprite.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/boflib/error.h"
#include "bagel/boflib/sound.h"

#define GRID_ROWS       8
#define GRID_COLS       8
#define MAX_FISH        4
#define MAX_FISH_SIZE   2*3

#define MAX_TURNS       MAX_FISH

typedef struct {
    POINT nLoc[MAX_FISH_SIZE + 1];
    UBYTE life;
} FISH;

class CBFishWindow : public CFrameWnd
{
    public:
        CBFishWindow(VOID);
        VOID        PlayGame(VOID);
        VOID        PaintScreen(VOID);
        VOID        LoadIniSettings(VOID);
        VOID        SaveIniSettings(VOID);

    protected:
        VOID        FlushInputEvents(VOID);
        VOID        GameReset(VOID);
        VOID        GamePause(VOID);
        VOID        GameResume(VOID);
        VOID        HandleError(ERROR_CODE);
        VOID        DeleteSprite(CSprite *);
        ERROR_CODE  RepaintSpriteList(CDC *);
        ERROR_CODE  LoadMasterSprites(VOID);
        VOID        ReleaseMasterSprites(VOID);
        VOID        PlaceUserFish(VOID);
        VOID        PlaceEnemyFish(VOID);
        VOID        RotateFish(INT);
        VOID        AssignFishToGrid(INT);
        INT         GetEnemyGridIndex(CPoint);
        INT         GetUserGridIndex(CPoint);
        INT         GetFishIndex(CSprite *);
        BOOL        OkToPlaceFish(INT, CPoint, BOOL);
        VOID        PlaceFish(INT, CPoint);
        INT         IndexToId(INT);
        INT         IdToIndex(INT);
        CPoint      SnapToGrid(CPoint);
        VOID        UsersTurn(INT);
        VOID        ComputersTurn(VOID);
        INT         SelectRandomTarget(VOID);
        INT         SelectBurningTarget(VOID);
        INT         SelectBestFitTarget(VOID);
        INT         FindNeighborTarget(INT, INT);
        INT         FindTarget(INT, INT);
        INT         FindMatch(INT, INT);
        BOOL        FishFits(INT, INT, INT);
        INT         GetNeighbors(INT, INT);
        VOID        CreatePlume(CPoint);
        VOID        CreateHarpoon(CPoint);
        VOID        SinkUserFish(INT);
        VOID        SinkEnemyFish(INT);
        VOID        PlaceTurnHarpoons(VOID);
        VOID        RemoveTurnHarpoon(VOID);

        virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
        LONG        OnMCINotify(WPARAM, LPARAM);
        LONG        OnMMIONotify(WPARAM, LPARAM);
        VOID        OnSoundNotify(CSound *);
        VOID        OnPaint(VOID);
        VOID        OnMouseMove(UINT, CPoint);
        VOID        OnRButtonDown(UINT, CPoint);
        VOID        OnLButtonDown(UINT, CPoint);
        VOID        OnLButtonUp(UINT, CPoint);
        VOID        OnSysChar(UINT, UINT, UINT);
        VOID        OnSysKeyDown(UINT, UINT, UINT);
        VOID        OnKeyDown(UINT, UINT, UINT);
        VOID        OnActivate(UINT, CWnd *, BOOL);
        VOID        OnClose(VOID);

        DECLARE_MESSAGE_MAP()

        // Data Members
        //
        UBYTE       m_nUserGrid[GRID_ROWS][GRID_COLS];
        UBYTE       m_nEnemyGrid[GRID_ROWS][GRID_COLS];
        FISH        m_aEnemyFishInfo[MAX_FISH];
        FISH        m_aUserFishInfo[MAX_FISH];
        CRect       m_rNewGameButton;
        CRect       m_rEndPlacement;
        CPoint      m_cLastPoint;
        CSprite    *m_pHarpoons[MAX_TURNS];
        CSprite    *m_pFish[MAX_FISH];
        CSprite    *m_pEnemyFish[MAX_FISH];
        CSprite    *m_pMasterHit;
        CSprite    *m_pMasterMiss;
        CSprite    *m_pMasterHarpoon;
        CSprite    *m_pDragFish;
        CSprite    *m_pScrollSprite;
        CSprite	   *m_pOctopus;
        CPalette   *m_pGamePalette;
        CSound     *m_pSoundTrack;
        CText      *m_pTxtClickHere;
        INT         m_nEnemyFish;
        INT         m_nUserFish;
        INT         m_nTurns;
        INT         m_nDifficultyLevel;
        BOOL        m_bGameActive;
        BOOL        m_bPause;
        BOOL        m_bUserEditMode;
        BOOL        m_bMovingFish;
        BOOL        m_bLastRotated;
        BOOL        m_bStillCheck;
        BOOL        m_bUsersTurn;
        BOOL        m_bInMenu;
};

#endif // !BFISH_H
