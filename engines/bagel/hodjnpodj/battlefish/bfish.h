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

#ifndef HODJNPODJ_BATTLEFISH_BFISH_H
#define HODJNPODJ_BATTLEFISH_BFISH_H

#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/hnplibs/stdinc.h"
#include "bagel/hodjnpodj/hnplibs/sprite.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/boflib/error.h"
#include "bagel/boflib/sound.h"

namespace Bagel {
namespace HodjNPodj {
namespace Battlefish {

#define GRID_ROWS       8
#define GRID_COLS       8
#define MAX_FISH        4
#define MAX_FISH_SIZE   2*3

#define MAX_TURNS       MAX_FISH

typedef struct {
	POINT nLoc[MAX_FISH_SIZE + 1];
	UBYTE life;
} FISH;

class CBFishWindow : public CFrameWnd {
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

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
	LRESULT     OnMCINotify(WPARAM, LPARAM);
	LRESULT     OnMMIONotify(WPARAM, LPARAM);
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
	CSprite    *m_pOctopus;
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

} // namespace Battlefish
} // namespace HodjNPodj
} // namespace Bagel

#endif
