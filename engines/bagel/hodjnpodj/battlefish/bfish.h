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
	CBFishWindow();
	void        PlayGame();
	void        PaintScreen();
	void        LoadIniSettings();
	void        SaveIniSettings();

protected:
	void        FlushInputEvents();
	void        GameReset();
	void        GamePause();
	void        GameResume();
	void        HandleError(ERROR_CODE);
	void        DeleteSprite(CSprite *);
	ERROR_CODE  RepaintSpriteList(CDC *);
	ERROR_CODE  LoadMasterSprites();
	void        ReleaseMasterSprites();
	void        PlaceUserFish();
	void        PlaceEnemyFish();
	void        RotateFish(int);
	void        AssignFishToGrid(int);
	int         GetEnemyGridIndex(CPoint);
	int         GetUserGridIndex(CPoint);
	int         GetFishIndex(CSprite *);
	bool        OkToPlaceFish(int, CPoint, bool);
	void        PlaceFish(int, CPoint);
	int         IndexToId(int);
	int         IdToIndex(int);
	CPoint      SnapToGrid(CPoint);
	void        UsersTurn(int);
	void        ComputersTurn();
	int         SelectRandomTarget();
	int         SelectBurningTarget();
	int         SelectBestFitTarget();
	int         FindNeighborTarget(int, int);
	int         FindTarget(int, int);
	int         FindMatch(int, int);
	bool        FishFits(int, int, int);
	int         GetNeighbors(int, int);
	void        CreatePlume(CPoint);
	void        CreateHarpoon(CPoint);
	void        SinkUserFish(int);
	void        SinkEnemyFish(int);
	void        PlaceTurnHarpoons();
	void        RemoveTurnHarpoon();

	virtual bool OnCommand(WPARAM wParam, LPARAM lParam) override;
	LRESULT     OnMCINotify(WPARAM, LPARAM);
	LRESULT     OnMMIONotify(WPARAM, LPARAM);
	void        OnSoundNotify(CSound *);
	void        OnPaint();
	void        OnMouseMove(unsigned int, CPoint);
	void        OnRButtonDown(unsigned int, CPoint);
	void        OnLButtonDown(unsigned int, CPoint);
	void        OnLButtonUp(unsigned int, CPoint);
	void        OnSysChar(unsigned int, unsigned int, unsigned int);
	void        OnSysKeyDown(unsigned int, unsigned int, unsigned int);
	void        OnKeyDown(unsigned int, unsigned int, unsigned int);
	void        OnActivate(unsigned int, CWnd *, bool) override;
	void        OnClose();

	DECLARE_MESSAGE_MAP()

	// Data Members
	//
	byte       m_nUserGrid[GRID_ROWS][GRID_COLS] = {};
	byte       m_nEnemyGrid[GRID_ROWS][GRID_COLS] = {};
	FISH        m_aEnemyFishInfo[MAX_FISH] = {};
	FISH        m_aUserFishInfo[MAX_FISH] = {};
	CRect       m_rNewGameButton;
	CRect       m_rEndPlacement;
	CPoint      m_cLastPoint;
	CSprite    *m_pHarpoons[MAX_TURNS] = {};
	CSprite    *m_pFish[MAX_FISH] = {};
	CSprite    *m_pEnemyFish[MAX_FISH] = {};
	CSprite    *m_pMasterHit = nullptr;
	CSprite    *m_pMasterMiss = nullptr;
	CSprite    *m_pMasterHarpoon = nullptr;
	CSprite    *m_pDragFish = nullptr;
	CSprite    *m_pScrollSprite = nullptr;
	CSprite    *m_pOctopus = nullptr;
	CPalette   *m_pGamePalette = nullptr;
	CSound     *m_pSoundTrack = nullptr;
	CText      *m_pTxtClickHere = nullptr;
	int         m_nEnemyFish = 0;
	int         m_nUserFish = 0;
	int         m_nTurns = 0;
	int         m_nDifficultyLevel = 0;
	bool        m_bGameActive = false;
	bool        m_bPause = false;
	bool        m_bUserEditMode = false;
	bool        m_bMovingFish = false;
	bool        m_bLastRotated = false;
	bool        m_bStillCheck = false;
	bool        m_bUsersTurn = false;
	bool        m_bInMenu = false;
};

} // namespace Battlefish
} // namespace HodjNPodj
} // namespace Bagel

#endif
