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

#ifndef HODJNPODJ_ARCHEROIDS_MAIN_H
#define HODJNPODJ_ARCHEROIDS_MAIN_H

#include "bagel/afxwin.h"

#include "bagel/boflib/error.h"
#include "bagel/hodjnpodj/hnplibs/stdinc.h"
#include "bagel/boflib/sound.h"
#include "bagel/hodjnpodj/hnplibs/sprite.h"
#include "bagel/boflib/llist.h"
#include "bagel/hodjnpodj/archeroids/usercfg.h"

namespace Bagel {
namespace HodjNPodj {
namespace Archeroids {

class CMainWindow : public CFrameWnd {
public:
	CMainWindow();
	void        PlayGame();
	void        PlayNextWave();
	void        PaintScreen();
	ERROR_CODE  RepaintSpriteList(CDC *);
	void        LoadIniSettings();
	bool        MainLoop();

protected:
	void        FlushInputEvents();
	bool        CheckMessages();
	ERROR_CODE  LoadMasterSprites();
	void        ReleaseMasterSprites();
	ERROR_CODE  LoadMasterSounds();
	void        ReleaseMasterSounds();
	void        InitializeJoystick();
	void        GameReset();
	void        WaveReset();
	CSprite    *NewLife(int);
	ERROR_CODE  CreateLives();
	ERROR_CODE  CreateGoodGuy();
	ERROR_CODE  CreateBadGuys();
	ERROR_CODE  CreateHay();
	ERROR_CODE  CreateBurningHay(CPoint);
	ERROR_CODE  CreateGoodArrow();
	ERROR_CODE  CreateBadArrow(CSprite *);
	void        GamePause();
	void        GameResume();

	void        DestroyGoodArrow(CLList *);
	void        DestroyBadArrow(CLList *);
	void        DestroyBadGuy(CLList *, CDC *);
	void        PruneDeadBadGuys();
	void        DestroyHay(CLList *, CRect, CDC *, bool);
	void        DeleteSprite(CSprite *);
	bool        MoveArrows(CDC *);
	bool        MoveBadGuys(CDC *);
	void        MoveHodj(int);
	void        LoseLife(CDC *, bool);
	void        HandleError(ERROR_CODE);
	void        KillAnimation();
	void        OnSoundNotify(CSound *pSound);
	POINT       GetLeftMostBadGuy();

	virtual bool OnCommand(WPARAM, LPARAM) override;
	void OnPaint();
	void OnSysKeyDown(unsigned int, unsigned int, unsigned int);
	void OnKeyDown(unsigned int, unsigned int, unsigned int);
	void OnSysChar(unsigned int, unsigned int, unsigned int);
	void OnTimer(uintptr);
	void OnClose();
	long OnJoyStick(unsigned int, long);
	void OnActivate(unsigned int, CWnd *, bool) override;
	void OnLButtonDown(unsigned int, CPoint);
	void OnRButtonUp(unsigned int, CPoint);
	void OnMouseMove(unsigned int, CPoint);
	LRESULT OnMCINotify(WPARAM, LPARAM);
	LRESULT OnMMIONotify(WPARAM, LPARAM);

	DECLARE_MESSAGE_MAP()

	//
	// protected data members
	//
	CRect        m_rNewGameButton = nullptr;
	CPalette    *m_pGamePalette = nullptr;
	CSprite     *m_pMasterBadArrow = nullptr;
	CSprite     *m_pMasterGoodArrow = nullptr;
	CSprite     *m_pMasterBurn = nullptr;
	CSprite     *m_pMasterBadWalk = nullptr;
	CSprite     *m_pMasterBadShoot = nullptr;
	CSprite     *m_pMasterBadDie = nullptr;
	CSprite     *m_pMasterGoodWalk = nullptr;
	CSprite     *m_pMasterGoodShoot = nullptr;
	CSprite     *m_pMasterGoodDie = nullptr;
	CSprite     *m_pMasterHeart = nullptr;
	CSprite     *m_pHodj = nullptr;
	CSprite     *m_pScrollSprite = nullptr;
	CSprite     *m_pLives[LIVES_MAX] = {};
	CLList      *m_pBadGuyList = nullptr;
	CLList      *m_pBadArrowList = nullptr;
	CLList      *m_pGoodArrowList = nullptr;
	CLList      *m_pHayList = nullptr;
	CLList      *m_pFXList = nullptr;
	CSound      *m_pSoundTrack = nullptr;

	char *       m_pBadDieSound = nullptr;
	char *       m_pBoltSound = nullptr;
	char *       m_pArrowSound = nullptr;
	char *       m_pBurnSound = nullptr;
	char *       m_pExtraLifeSound = nullptr;
	HANDLE       m_hBadDieRes = nullptr;
	HANDLE       m_hBoltRes = nullptr;
	HANDLE       m_hArrowRes = nullptr;
	HANDLE       m_hBurnRes = nullptr;
	HANDLE       m_hExtraLifeRes = nullptr;

	POINT        m_ptAnchor;
	unsigned long        m_lScore = 0;
	unsigned long        m_lNewLifeScore = 0;
	int          m_nGoodArrows, m_nBadArrows = 0;
	int          m_nGameSpeed = 0;
	unsigned int         m_nBadGuySpeed = 0;
	unsigned int         m_nArrowSpeed = 0;
	unsigned int         m_nState = 0;
	int          m_nMoveArrows = 0;
	int          m_nBadGuys = 0;
	int          m_nLives = 0;
	int          m_nLevel = 0;
	int          m_nWave = 0;
	unsigned int         m_nJoyLast = 0;
	bool         m_bPause = false;
	bool         m_bGameActive = false;
	bool         m_bJoyActive = false;
	bool         m_bMoveMode = false;
	bool         m_bInMenu = false;
	bool         m_bNewGame = false;
	bool         m_bAnimationsOn = false;
	bool         m_bTimerActive = false;


	// User Setup variables
	//
	int          m_nInitGameSpeed = 0;
	int          m_nInitArcherLevel = 0;
	int          m_nInitNumLives = 0;
	int          m_nInitNumBadGuys = 0;
};

////
//
// CTheApp:
//
class CTheApp : public CWinApp {
public:
	bool InitInstance();
	virtual int ExitInstance();
};

} // namespace Archeroids
} // namespace HodjNPodj
} // namespace Bagel

#endif
