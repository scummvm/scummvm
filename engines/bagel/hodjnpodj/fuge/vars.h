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

#ifndef HODJNPODJ_DIALOGS_VARS_H
#define HODJNPODJ_DIALOGS_VARS_H

#include "common/rect.h"
#include "graphics/managed_surface.h"
#include "graphics/palette.h"
#include "bagel/boflib/sound.h"
#include "bagel/hodjnpodj/boflib/vector.h"

namespace Bagel {
namespace HodjNPodj {
namespace Fuge {

#define N_ROWS           6
#define BRICKS_PER_ROW  16
#define N_BRICKS        (N_ROWS * BRICKS_PER_ROW)

struct Vars {
	bool _bBrickVisible[N_BRICKS] = { false };
	Common::Point _ptOrigin;
	Common::Rect _rNewGameButton;
	Common::Rect _rBlackHole;
	CVector _vBallVector;
	CVector _ptBallLocation;
	Graphics::Palette *m_pGamePalette = nullptr;
	//CBmpButton *m_pScrollButton;
	Graphics::ManagedSurface *m_pBall = nullptr;
	Graphics::ManagedSurface *m_pPaddle = nullptr;
	CSound *m_pSoundTrack = nullptr;

	Common::String _pBrickSound;
	Common::String _pWallSound;
	Common::String _pPaddleSound;
	Common::String _pExtraLifeSound;
	void *_hBrickRes = nullptr;
	void *_hWallRes = nullptr;
	void *_hPaddleRes = nullptr;
	void *_hExtraLifeRes = nullptr;

	int _nInitNumBalls = 0;
	int _nInitStartLevel = 0;
	int _nInitBallSpeed = 0;
	int _nInitPaddleSize = 0;
	int _nGForceFactor = 0;

	double _fTurboBoost = 0;
	long _lScore = 0;
	long _lExtraLifeScore = 0;
	long _nJoyOrgX = 0;
	long _nJoyOrgY = 0;
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
	bool _bOutterWall = false;
	bool _bJoyActive = false;
	bool _bBallOnPaddle = false;
};

} // namespace Fuge
} // namespace HodjNPodj
} // namespace Bagel

#endif
