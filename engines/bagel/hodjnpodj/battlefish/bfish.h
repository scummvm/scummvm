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
	byte life;
} FISH;

class CBFishWindow : public CFrameWnd {
public:
	CBFishWindow(void);
	void        PlayGame(void);
	void        PaintScreen(void);
	void        LoadIniSettings(void);
	void        SaveIniSettings(void);

protected:
	void        FlushInputEvents(void);
	void        GameReset(void);
	void        GamePause(void);
	void        GameResume(void);
	void        HandleError(ERROR_CODE);
	void        DeleteSprite(CSprite *);
	ERROR_CODE  RepaintSpriteList(CDC *);
	ERROR_CODE  LoadMasterSprites(void);
	void        ReleaseMasterSprites(void);
	void        PlaceUserFish(void);
	void        PlaceEnemyFish(void);
	void        RotateFish(int);
	void        AssignFishToGrid(int);
	int         GetEnemyGridIndex(CPoint);
	int         GetUserGridIndex(CPoint);
	int         GetFishIndex(CSprite *);
	BOOL        OkToPlaceFish(int, CPoint, BOOL);
	void        PlaceFish(int, CPoint);
	int         IndexToId(int);
	int         IdToIndex(int);
	CPoint      SnapToGrid(CPoint);
	void        UsersTurn(int);
	void        ComputersTurn(void);
	int         SelectRandomTarget(void);
	int         SelectBurningTarget(void);
	int         SelectBestFitTarget(void);
	int         FindNeighborTarget(int, int);
	int         FindTarget(int, int);
	int         FindMatch(int, int);
	BOOL        FishFits(int, int, int);
	int         GetNeighbors(int, int);
	void        CreatePlume(CPoint);
	void        CreateHarpoon(CPoint);
	void        SinkUserFish(int);
	void        SinkEnemyFish(int);
	void        PlaceTurnHarpoons(void);
	void        RemoveTurnHarpoon(void);

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
	LRESULT     OnMCINotify(WPARAM, LPARAM);
	LRESULT     OnMMIONotify(WPARAM, LPARAM);
	void        OnSoundNotify(CSound *);
	void        OnPaint(void);
	void        OnMouseMove(UINT, CPoint);
	void        OnRButtonDown(UINT, CPoint);
	void        OnLButtonDown(UINT, CPoint);
	void        OnLButtonUp(UINT, CPoint);
	void        OnSysChar(UINT, UINT, UINT);
	void        OnSysKeyDown(UINT, UINT, UINT);
	void        OnKeyDown(UINT, UINT, UINT);
	void        OnActivate(UINT, CWnd *, BOOL) override;
	void        OnClose(void);

	DECLARE_MESSAGE_MAP()

	// Data Members
	//
	byte       m_nUserGrid[GRID_ROWS][GRID_COLS];
	byte       m_nEnemyGrid[GRID_ROWS][GRID_COLS];
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
	int         m_nEnemyFish;
	int         m_nUserFish;
	int         m_nTurns;
	int         m_nDifficultyLevel;
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
