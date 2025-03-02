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

#ifndef HODJNPODJ_FUGE_FUGE_H
#define HODJNPODJ_FUGE_FUGE_H

#include "bagel/hodjnpodj/views/minigame_view.h"
#include "bagel/hodjnpodj/gfx/bmp_button.h"
#include "bagel/hodjnpodj/libs/vector.h"
#include "bagel/hodjnpodj/fuge/defines.h"
#include "bagel/boflib/error.h"
#include "bagel/boflib/sound.h"

namespace Bagel {
namespace HodjNPodj {
namespace Fuge {

class Fuge : public MinigameView {
private:
	BmpButton m_ScrollButton;
	Graphics::Palette m_GamePalette;
	Graphics::ManagedSurface _background;
	int _backgroundNumRows = -1;
	int _paddleOldSize = -1;
	bool _timerPaused = true;

	bool m_bBrickVisible[N_BRICKS] = { false };
	const Common::Point m_ptOrigin;
	const Common::Rect m_rNewGameButton;
	Common::Rect m_rBlackHole;
	CVector m_vBallVector;
	CVector m_ptBallLocation;
	Sprite m_pBall;
	Sprite m_pPaddle;
	CBofSound *m_pSoundTrack;

	const char *m_pBrickSound;
	const char *m_pWallSound;
	const char *m_pPaddleSound;
	const char *m_pExtraLifeSound;
	Common::SeekableReadStream *m_hBrickRes = nullptr;
	Common::SeekableReadStream *m_hWallRes = nullptr;
	Common::SeekableReadStream *m_hPaddleRes = nullptr;
	Common::SeekableReadStream *m_hExtraLifeRes = nullptr;

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
	CVector _gvCenter;

private:
	void clear();
	void paintBricks();
	void repaintSpriteList();
	void eraseBrick(int brickIndex);
	void realignVectors();
	void loadMasterSprites();
	void loadNewPaddle(int nNewSize);
	void loadMasterSounds();
	void showOptionsMenu();
	void gamePause();
	void gameResume();
	void playGame();
	void gameReset();
	void loadIniSettings();
	void releaseMasterSprites();
	void releaseMasterSounds();
	void initializeJoystick();
	void startBall();
	void endBall();
	void paintBall();
	void launchBall();
	void startPaddle();
	void endPaddle();
	void paintPaddle(bool);
	CVector ballOnPaddle();
	void startBricks();
	void endBricks();
	void loseBall();
	void ballvsPaddle();
	void ballvsBrick(double);
	void HandleError(ErrorCode);

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
	Fuge();
	virtual ~Fuge() {
	}

	void gameOverClosed();
	void newLifeClosed();
	void roundCompleteClosed();

	bool msgOpen(const OpenMessage &msg) override;
	bool msgClose(const CloseMessage &msg) override;
	bool msgFocus(const FocusMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;
	void draw() override;
	bool tick() override;

	void        PaintScreen();
};

} // namespace Fuge
} // namespace HodjNPodj
} // namespace Bagel

#endif
