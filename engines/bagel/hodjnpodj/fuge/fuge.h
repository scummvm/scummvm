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
 * aint32 with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef HODJNPODJ_DIALOGS_Fuge_H
#define HODJNPODJ_DIALOGS_Fuge_H

#include "bagel/hodjnpodj/view.h"
#include "bagel/hodjnpodj/boflib/vector.h"
#include "bagel/hodjnpodj/fuge/defines.h"
#include "bagel/boflib/error.h"
#include "bagel/boflib/sound.h"

namespace Bagel {
namespace HodjNPodj {
namespace Fuge {

class Fuge : public View {
private:
	//CBmpButton *m_pScrollButton;
	bool        m_bBrickVisible[N_BRICKS];
	Common::Point      m_ptOrigin;
	Common::Rect       m_rNewGameButton;
	Common::Rect       m_rBlackHole;
	CVector     m_vBallVector;
	CVector     m_ptBallLocation;
	Graphics::Palette *m_pGamePalette;
	Graphics::ManagedSurface *m_pBall;
	Graphics::ManagedSurface *m_pPaddle;
	CBofSound *m_pSoundTrack;

	const char *m_pBrickSound;
	const char *m_pWallSound;
	const char *m_pPaddleSound;
	const char *m_pExtraLifeSound;
	uint m_hBrickRes = 0;
	uint m_hWallRes = 0;
	uint m_hPaddleRes = 0;
	uint m_hExtraLifeRes = 0;

	int m_nInitNumBalls = 0;
	int m_nInitStartLevel = 0;
	int m_nInitBallSpeed = 0;
	int m_nInitPaddleSize = 0;
	int m_nGForceFactor = 0;

	double m_fTurboBoost = 0;
	int m_lScore = 0;
	int m_lExtraLifeScore = 0;
	int m_nJoyOrgX = 0;
	int m_nJoyOrgY = 0;
	int m_nBricks = 0;
	int m_nBalls = 0;
	int m_nBallSpeed = 0;
	int m_nNumRows = 0;
	int m_nPaddleCelIndex = 0;
	bool m_bMovingPaddle = false;
	bool m_bGameActive = false;
	bool m_bPause = false;
	bool m_bIgnoreScrollClick = false;
	bool m_bPaddleHit = false;
	bool m_bOutterWall = false;
	bool m_bJoyActive = false;
	bool m_bBallOnPaddle = false;

private:
	void clear();
	void GameReset();
	void GamePause();
	void GameResume();
	void HandleError(ErrorCode);
	void RealignVectors();
	ErrorCode LoadMasterSprites();
	void ReleaseMasterSprites();
	ErrorCode LoadMasterSounds();
	void ReleaseMasterSounds();
	ErrorCode LoadNewPaddle(int);
	void InitializeJoystick();
	void RepaintSpriteList(void *);
	void PaintBricks(void *);
	void StartBall();
	void EndBall();
	void PaintBall();
	void LaunchBall();
	void StartPaddle();
	void EndPaddle();
	void PaintPaddle(bool);
	CVector BallOnPaddle();
	void StartBricks();
	void EndBricks();
	void EraseBrick(void *, int);
	void LoseBall();
	void BallvsPaddle();
	void BallvsBrick(double);

	//virtual bool OnCommand(uint16 uint16, int32 int32);
	int32 OnJoyStick(uint, int32);
	void OnPaint();
	void OnTimer(uint);
	void OnMouseMove(uint, const Common::Point &);
	void OnRButtonUp(uint, const Common::Point &);
	void OnLButtonDown(uint, const Common::Point &);
	void OnSysChar(uint, uint, uint);
	void OnSysKeyDown(uint, uint, uint);
	void OnKeyDown(uint, uint, uint);
	void OnActivate(uint, void *, bool);
	int32 OnMCINotify(uint16, int32);
	int32 OnMMIONotify(uint16, int32);
	void OnSoundNotify(CBofSound *);
	void OnClose();

public:
	Fuge() : View("Fuge") {
		clear();
	}
	virtual ~Fuge() {
	}

	bool msgFocus(const FocusMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;
	void draw() override;
	bool tick() override;

	void        PlayGame();
	void        PaintScreen();
	void        LoadIniSettings();
};

} // namespace Fuge
} // namespace HodjNPodj
} // namespace Bagel

#endif
