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
	void        PlayGame(void);
	void        PlayNextWave(void);
	void        PaintScreen(void);
	ERROR_CODE  RepaintSpriteList(CDC *);
	void        LoadIniSettings(void);
	BOOL        MainLoop(void);

protected:
	void        FlushInputEvents(void);
	BOOL        CheckMessages(void);
	ERROR_CODE  LoadMasterSprites(void);
	void        ReleaseMasterSprites(void);
	ERROR_CODE  LoadMasterSounds(void);
	void        ReleaseMasterSounds(void);
	void        InitializeJoystick(void);
	void        GameReset(void);
	void        WaveReset(void);
	CSprite    *NewLife(int);
	ERROR_CODE  CreateLives(void);
	ERROR_CODE  CreateGoodGuy(void);
	ERROR_CODE  CreateBadGuys(void);
	ERROR_CODE  CreateHay(void);
	ERROR_CODE  CreateBurningHay(CPoint);
	ERROR_CODE  CreateGoodArrow(void);
	ERROR_CODE  CreateBadArrow(CSprite *);
	void        GamePause(void);
	void        GameResume(void);

	void        DestroyGoodArrow(CLList *);
	void        DestroyBadArrow(CLList *);
	void        DestroyBadGuy(CLList *, CDC *);
	void        PruneDeadBadGuys(void);
	void        DestroyHay(CLList *, CRect, CDC *, BOOL);
	void        DeleteSprite(CSprite *);
	BOOL        MoveArrows(CDC *);
	BOOL        MoveBadGuys(CDC *);
	void        MoveHodj(int);
	void        LoseLife(CDC *, BOOL);
	void        HandleError(ERROR_CODE);
	void        KillAnimation(void);
	void        OnSoundNotify(CSound *pSound);
	POINT       GetLeftMostBadGuy(void);

	virtual BOOL OnCommand(WPARAM, LPARAM) override;
	void OnPaint(void);
	void OnSysKeyDown(UINT, UINT, UINT);
	void OnKeyDown(UINT, UINT, UINT);
	void OnSysChar(UINT, UINT, UINT);
	void OnTimer(UINT_PTR);
	void OnClose(void);
	long OnJoyStick(UINT, long);
	void OnActivate(UINT, CWnd *, BOOL) override;
	void OnLButtonDown(UINT, CPoint);
	void OnRButtonUp(UINT, CPoint);
	void OnMouseMove(UINT, CPoint);
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

	LPSTR        m_pBadDieSound;
	LPSTR        m_pBoltSound;
	LPSTR        m_pArrowSound;
	LPSTR        m_pBurnSound;
	LPSTR        m_pExtraLifeSound;
	HANDLE       m_hBadDieRes;
	HANDLE       m_hBoltRes;
	HANDLE       m_hArrowRes;
	HANDLE       m_hBurnRes;
	HANDLE       m_hExtraLifeRes;

	POINT        m_ptAnchor;
	ULONG        m_lScore;
	ULONG        m_lNewLifeScore;
	int          m_nGoodArrows, m_nBadArrows;
	int          m_nGameSpeed;
	UINT         m_nBadGuySpeed;
	UINT         m_nArrowSpeed;
	UINT         m_nState;
	int          m_nMoveArrows;
	int          m_nBadGuys;
	int          m_nLives;
	int          m_nLevel;
	int          m_nWave;
	UINT         m_nJoyLast;
	BOOL         m_bPause;
	BOOL         m_bGameActive;
	BOOL         m_bJoyActive;
	BOOL         m_bMoveMode;
	BOOL         m_bInMenu;
	BOOL         m_bNewGame;
	BOOL         m_bAnimationsOn;
	BOOL         m_bTimerActive;


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
	BOOL InitInstance();
	virtual int ExitInstance();
};

} // namespace Archeroids
} // namespace HodjNPodj
} // namespace Bagel

#endif
