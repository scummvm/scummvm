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
	CFugeWindow(void);
	void        PlayGame(void);
	void        PaintScreen(void);
	void        LoadIniSettings(void);

protected:
	void initMembers();
	void initStatics();
	void        GameReset(void);
	void        GamePause(void);
	void        GameResume(void);
	void        HandleError(ERROR_CODE);
	void        RealignVectors(void);
	ERROR_CODE  LoadMasterSprites(void);
	void        ReleaseMasterSprites(void);
	ERROR_CODE  LoadMasterSounds(void);
	void        ReleaseMasterSounds(void);
	ERROR_CODE  LoadNewPaddle(int);
	void        InitializeJoystick(void);
	void        RepaintSpriteList(CDC *);
	void        PaintBricks(CDC *);
	void        StartBall(void);
	void        EndBall(void);
	void        PaintBall(void);
	void        LaunchBall(void);
	void        StartPaddle(void);
	void        EndPaddle(void);
	void        PaintPaddle(BOOL);
	CVector     BallOnPaddle(void);
	void        StartBricks(void);
	void        EndBricks(void);
	void        EraseBrick(CDC *, int);
	void        LoseBall(void);
	void        BallvsPaddle(void);
	void        BallvsBrick(DOUBLE);

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
	long        OnJoyStick(UINT, long);
	void        OnPaint(void);
	void        OnTimer(UINT_PTR);
	void        OnMouseMove(UINT, CPoint);
	void        OnRButtonUp(UINT, CPoint);
	void        OnLButtonDown(UINT, CPoint);
	void        OnSysChar(UINT, UINT, UINT);
	void        OnSysKeyDown(UINT, UINT, UINT);
	void        OnKeyDown(UINT, UINT, UINT);
	void        OnActivate(UINT, CWnd *, BOOL) override;
	LRESULT     OnMCINotify(WPARAM, LPARAM);
	LRESULT     OnMMIONotify(WPARAM, LPARAM);
	void        OnSoundNotify(CSound *);
	void        OnClose(void);

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

	char *      m_pBrickSound;
	char *      m_pWallSound;
	char *      m_pPaddleSound;
	char *      m_pExtraLifeSound;
	HANDLE      m_hBrickRes;
	HANDLE      m_hWallRes;
	HANDLE      m_hPaddleRes;
	HANDLE      m_hExtraLifeRes;

	int         m_nInitNumBalls;
	int         m_nInitStartLevel;
	int         m_nInitBallSpeed;
	int         m_nInitPaddleSize;
	int         m_nGForceFactor;

	DOUBLE      m_fTurboBoost;
	long        m_lScore;
	long        m_lExtraLifeScore;
	long        m_nJoyOrgX;
	long        m_nJoyOrgY;
	int         m_nBricks;
	int         m_nBalls;
	int         m_nBallSpeed;
	int         m_nNumRows;
	int         m_nPaddleCelIndex;
	int			m_nOldSize = -1;

	BOOL        m_bMovingPaddle;
	BOOL        m_bGameActive;
	BOOL        m_bPause;
	BOOL        m_bIgnoreScrollClick;
	BOOL        m_bPaddleHit;
	BOOL        m_bOutterWall;
	BOOL        m_bJoyActive;
	BOOL        m_bBallOnPaddle;

	CVector gvCenter;
};

} // namespace Fuge
} // namespace HodjNPodj
} // namespace Bagel

#endif
