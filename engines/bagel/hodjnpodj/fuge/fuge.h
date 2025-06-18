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

#ifndef HODJNPODJ_FUGE_FUGE_H
#define HODJNPODJ_FUGE_FUGE_H

#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/hnplibs/stdinc.h"
#include "bagel/hodjnpodj/hnplibs/sprite.h"
#include "bagel/hodjnpodj/hnplibs/text.h"
#include "bagel/boflib/error.h"
#include "bagel/hodjnpodj/libs/vector.h"
#include "bagel/hodjnpodj/hnplibs/button.h"
#include "bagel/boflib/sound.h"

namespace Bagel {
namespace HodjNPodj {
namespace Fuge {

#define N_ROWS           6
#define BRICKS_PER_ROW  16
#define N_BRICKS        (N_ROWS*BRICKS_PER_ROW)

class CFugeWindow : public CFrameWnd {
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

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
	LONG        OnJoyStick(UINT, LONG);
	VOID        OnPaint(VOID);
	VOID        OnTimer(UINT_PTR);
	VOID        OnMouseMove(UINT, CPoint);
	VOID        OnRButtonUp(UINT, CPoint);
	VOID        OnLButtonDown(UINT, CPoint);
	VOID        OnSysChar(UINT, UINT, UINT);
	VOID        OnSysKeyDown(UINT, UINT, UINT);
	VOID        OnKeyDown(UINT, UINT, UINT);
	VOID        OnActivate(UINT, CWnd *, BOOL);
	LRESULT     OnMCINotify(WPARAM, LPARAM);
	LRESULT     OnMMIONotify(WPARAM, LPARAM);
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

} // namespace Fuge
} // namespace HodjNPodj
} // namespace Bagel

#endif
