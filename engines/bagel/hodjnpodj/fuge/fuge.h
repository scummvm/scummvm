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
	CFugeWindow();
	void        PlayGame();
	void        PaintScreen();
	void        LoadIniSettings();

protected:
	void initMembers();
	void initStatics();
	void        GameReset();
	void        GamePause();
	void        GameResume();
	void        HandleError(ERROR_CODE);
	void        RealignVectors();
	ERROR_CODE  LoadMasterSprites();
	void        ReleaseMasterSprites();
	ERROR_CODE  LoadMasterSounds();
	void        ReleaseMasterSounds();
	ERROR_CODE  LoadNewPaddle(int);
	void        InitializeJoystick();
	void        RepaintSpriteList(CDC *);
	void        PaintBricks(CDC *);
	void        StartBall();
	void        EndBall();
	void        PaintBall();
	void        LaunchBall();
	void        StartPaddle();
	void        EndPaddle();
	void        PaintPaddle(bool);
	CVector     BallOnPaddle();
	void        StartBricks();
	void        EndBricks();
	void        EraseBrick(CDC *, int);
	void        LoseBall();
	void        BallvsPaddle();
	void        BallvsBrick(double);

	virtual bool OnCommand(WPARAM wParam, LPARAM lParam) override;
	long        OnJoyStick(unsigned int, long);
	void        OnPaint();
	void        OnTimer(uintptr);
	void        OnMouseMove(unsigned int, CPoint);
	void        OnRButtonUp(unsigned int, CPoint);
	void        OnLButtonDown(unsigned int, CPoint);
	void        OnSysChar(unsigned int, unsigned int, unsigned int);
	void        OnSysKeyDown(unsigned int, unsigned int, unsigned int);
	void        OnKeyDown(unsigned int, unsigned int, unsigned int);
	void        OnActivate(unsigned int, CWnd *, bool) override;
	LRESULT     OnMCINotify(WPARAM, LPARAM);
	LRESULT     OnMMIONotify(WPARAM, LPARAM);
	void        OnSoundNotify(CSound *);
	void        OnClose();

	DECLARE_MESSAGE_MAP()

	bool        m_bBrickVisible[N_BRICKS] = {};
	CPoint      m_ptOrigin;
	CRect       m_rNewGameButton;
	CRect       m_rBlackHole;
	CVector     m_vBallVector;
	CVector     m_ptBallLocation;
	CPalette   *m_pGamePalette = nullptr;
	CBmpButton *m_pScrollButton = nullptr;
	CSprite    *m_pBall = nullptr;
	CSprite    *m_pPaddle = nullptr;
	CSound     *m_pSoundTrack = nullptr;

	char *      m_pBrickSound = nullptr;
	char *      m_pWallSound = nullptr;
	char *      m_pPaddleSound = nullptr;
	char *      m_pExtraLifeSound = nullptr;
	HANDLE      m_hBrickRes = nullptr;
	HANDLE      m_hWallRes = nullptr;
	HANDLE      m_hPaddleRes = nullptr;
	HANDLE      m_hExtraLifeRes = nullptr;

	int         m_nInitNumBalls = 0;
	int         m_nInitStartLevel = 0;
	int         m_nInitBallSpeed = 0;
	int         m_nInitPaddleSize = 0;
	int         m_nGForceFactor = 0;

	double      m_fTurboBoost = 0;
	long        m_lScore = 0;
	long        m_lExtraLifeScore = 0;
	long        m_nJoyOrgX = 0;
	long        m_nJoyOrgY = 0;
	int         m_nBricks = 0;
	int         m_nBalls = 0;
	int         m_nBallSpeed = 0;
	int         m_nNumRows = 0;
	int         m_nPaddleCelIndex = 0;
	int			m_nOldSize = -1;

	bool        m_bMovingPaddle = false;
	bool        m_bGameActive = false;
	bool        m_bPause = false;
	bool        m_bIgnoreScrollClick = false;
	bool        m_bPaddleHit = false;
	bool        m_bOutterWall = false;
	bool        m_bJoyActive = false;
	bool        m_bBallOnPaddle = false;

	CVector gvCenter;
};

} // namespace Fuge
} // namespace HodjNPodj
} // namespace Bagel

#endif
