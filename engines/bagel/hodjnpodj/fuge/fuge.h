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
#include "bagel/hodjnpodj/gfx/button.h"
#include "bagel/hodjnpodj/libs/vector.h"
#include "bagel/hodjnpodj/fuge/defines.h"
#include "bagel/boflib/error.h"
#include "bagel/boflib/sound.h"

namespace Bagel {
namespace HodjNPodj {
namespace Fuge {

class Fuge : public MinigameView {
private:
	BmpButton _scrollButton;
	Graphics::ManagedSurface _background;
	const CVector _gvCenter;
	int _timerCtr = 0;
	int _backgroundNumRows = -1;
	int _paddleOldSize = -1;

	bool _brickVisible[N_BRICKS] = { false };
	const Common::Point _ptOrigin;
	const Common::Rect _rNewGameButton;
	CVector _vBallVector;
	CVector _ptBallLocation;
	Sprite _sprBall;
	Sprite _sprPaddle;
	CBofSound *_soundTrack;

	const char *m_pBrickSound;
	const char *m_pWallSound;
	const char *m_pPaddleSound;
	const char *m_pExtraLifeSound;
	Common::SeekableReadStream *_brickSound = nullptr;
	Common::SeekableReadStream *_wallSound = nullptr;
	Common::SeekableReadStream *_paddleSound = nullptr;
	Common::SeekableReadStream *_extraLifeSound = nullptr;

	int _nInitNumBalls = 0;
	int _nInitStartLevel = 0;
	int _nInitBallSpeed = 0;
	int _nInitPaddleSize = 0;
	int _nGForceFactor = 0;

	double _fTurboBoost = 0;
	int _lScore = 0;
	int _lExtraLifeScore = 0;
	int _nBricks = 0;
	int _nBalls = 0;
	int _nBallSpeed = 0;
	int _nNumRows = 0;
	int _nPaddleCelIndex = 0;
	bool _bMovingPaddle = false;
	bool _bGameActive = false;
	bool _bPause = false;
	bool _bIgnoreScrollClick = false;
	bool _bPaddleHit = false;
	bool _bOuterWall = false;
	bool _bJoyActive = false;
	bool _bBallOnPaddle = false;

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
	void playGame();
	void gameReset();
	void loadIniSettings();
	void releaseMasterSprites();
	void releaseMasterSounds();
	void initializeJoystick();
	void startBall();
	void endBall();
	void moveBall();
	void launchBall();
	void startPaddle();
	void endPaddle();
	void movePaddle(bool);
	CVector ballOnPaddle();
	void startBricks();
	void endBricks();
	void loseBall();
	void ballvsPaddle();
	void ballvsBrick(double);

public:
	Fuge();
	virtual ~Fuge() {
	}

	void gameOverClosed();
	void newLifeClosed();
	void roundCompleteClosed();
	void optionsClosed();
	void gameResume();

	bool msgOpen(const OpenMessage &msg) override;
	bool msgClose(const CloseMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;
	bool msgMouseMove(const MouseMoveMessage &msg) override;
	bool msgMouseDown(const MouseDownMessage &msg) override;
	bool msgMouseUp(const MouseUpMessage &msg) override;
	void draw() override;
	bool tick() override;
};

} // namespace Fuge
} // namespace HodjNPodj
} // namespace Bagel

#endif
