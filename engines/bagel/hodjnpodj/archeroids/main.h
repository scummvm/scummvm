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
	VOID        PlayGame(VOID);
	VOID        PlayNextWave(VOID);
	VOID        PaintScreen(VOID);
	ERROR_CODE  RepaintSpriteList(CDC *);
	VOID        LoadIniSettings(VOID);
	BOOL        MainLoop(VOID);

protected:
	VOID        FlushInputEvents(VOID);
	BOOL        CheckMessages(VOID);
	ERROR_CODE  LoadMasterSprites(VOID);
	VOID        ReleaseMasterSprites(VOID);
	ERROR_CODE  LoadMasterSounds(VOID);
	VOID        ReleaseMasterSounds(VOID);
	VOID        InitializeJoystick(VOID);
	VOID        GameReset(VOID);
	VOID        WaveReset(VOID);
	CSprite    *NewLife(INT);
	ERROR_CODE  CreateLives(VOID);
	ERROR_CODE  CreateGoodGuy(VOID);
	ERROR_CODE  CreateBadGuys(VOID);
	ERROR_CODE  CreateHay(VOID);
	ERROR_CODE  CreateBurningHay(CPoint);
	ERROR_CODE  CreateGoodArrow(VOID);
	ERROR_CODE  CreateBadArrow(CSprite *);
	VOID        GamePause(VOID);
	VOID        GameResume(VOID);

	VOID        DestroyGoodArrow(CLList *);
	VOID        DestroyBadArrow(CLList *);
	VOID        DestroyBadGuy(CLList *, CDC *);
	VOID        PruneDeadBadGuys(VOID);
	VOID        DestroyHay(CLList *, CRect, CDC *, BOOL);
	VOID        DeleteSprite(CSprite *);
	BOOL        MoveArrows(CDC *);
	BOOL        MoveBadGuys(CDC *);
	VOID        MoveHodj(INT);
	VOID        LoseLife(CDC *, BOOL);
	VOID        HandleError(ERROR_CODE);
	VOID        KillAnimation(VOID);
	VOID        OnSoundNotify(CSound *pSound);
	POINT       GetLeftMostBadGuy(VOID);

	virtual BOOL OnCommand(WPARAM, LPARAM) override;
	VOID OnPaint(VOID);
	VOID OnSysKeyDown(UINT, UINT, UINT);
	VOID OnKeyDown(UINT, UINT, UINT);
	VOID OnSysChar(UINT, UINT, UINT);
	VOID OnTimer(UINT_PTR);
	VOID OnClose(void);
	LONG OnJoyStick(UINT, LONG);
	VOID OnActivate(UINT, CWnd *, BOOL) override;
	VOID OnLButtonDown(UINT, CPoint);
	VOID OnRButtonUp(UINT, CPoint);
	VOID OnMouseMove(UINT, CPoint);
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
	INT          m_nGoodArrows, m_nBadArrows;
	INT          m_nGameSpeed;
	UINT         m_nBadGuySpeed;
	UINT         m_nArrowSpeed;
	UINT         m_nState;
	INT          m_nMoveArrows;
	INT          m_nBadGuys;
	INT          m_nLives;
	INT          m_nLevel;
	INT          m_nWave;
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
	INT          m_nInitGameSpeed;
	INT          m_nInitArcherLevel;
	INT          m_nInitNumLives;
	INT          m_nInitNumBadGuys;
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
