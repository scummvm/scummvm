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
	CRect        m_rNewGameButton;
	CPalette    *m_pGamePalette;
	CSprite     *m_pMasterBadArrow;
	CSprite     *m_pMasterGoodArrow;
	CSprite     *m_pMasterBurn;
	CSprite     *m_pMasterBadWalk;
	CSprite     *m_pMasterBadShoot;
	CSprite     *m_pMasterBadDie;
	CSprite     *m_pMasterGoodWalk;
	CSprite     *m_pMasterGoodShoot;
	CSprite     *m_pMasterGoodDie;
	CSprite     *m_pMasterHeart;
	CSprite     *m_pHodj;
	CSprite     *m_pScrollSprite;
	CSprite     *m_pLives[LIVES_MAX];
	CLList      *m_pBadGuyList;
	CLList      *m_pBadArrowList;
	CLList      *m_pGoodArrowList;
	CLList      *m_pHayList;
	CLList      *m_pFXList;
	CSound      *m_pSoundTrack;

	char *       m_pBadDieSound;
	char *       m_pBoltSound;
	char *       m_pArrowSound;
	char *       m_pBurnSound;
	char *       m_pExtraLifeSound;
	HANDLE       m_hBadDieRes;
	HANDLE       m_hBoltRes;
	HANDLE       m_hArrowRes;
	HANDLE       m_hBurnRes;
	HANDLE       m_hExtraLifeRes;

	POINT        m_ptAnchor;
	unsigned long        m_lScore;
	unsigned long        m_lNewLifeScore;
	int          m_nGoodArrows, m_nBadArrows;
	int          m_nGameSpeed;
	unsigned int         m_nBadGuySpeed;
	unsigned int         m_nArrowSpeed;
	unsigned int         m_nState;
	int          m_nMoveArrows;
	int          m_nBadGuys;
	int          m_nLives;
	int          m_nLevel;
	int          m_nWave;
	unsigned int         m_nJoyLast;
	bool         m_bPause;
	bool         m_bGameActive;
	bool         m_bJoyActive;
	bool         m_bMoveMode;
	bool         m_bInMenu;
	bool         m_bNewGame;
	bool         m_bAnimationsOn;
	bool         m_bTimerActive;


	// User Setup variables
	//
	int          m_nInitGameSpeed;
	int          m_nInitArcherLevel;
	int          m_nInitNumLives;
	int          m_nInitNumBadGuys;
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
