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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_ACTION_ARCADEPUZZLE_H
#define NANCY_ACTION_ARCADEPUZZLE_H

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/commontypes.h"

#include "graphics/managed_surface.h"
#include "common/path.h"
#include "common/rect.h"

namespace Nancy {
namespace Action {

// Barnacle Blast (Arkanoid clone) mini-game in Nancy 8.
// The player controls a paddle at the bottom of the screen, bouncing a ball
// to destroy bricks. The player needs to beat 2 levels to get the full prize.
// Data file is 0x4e8 (1256) bytes.
class ArcadePuzzle : public RenderActionRecord {
public:
	ArcadePuzzle() : RenderActionRecord(7) {}
	virtual ~ArcadePuzzle();

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "ArcadePuzzle"; }

	// ---------- Inner types ----------

	struct Brick {
		Common::Rect srcRect;           // source in _image (brick type sprite)
		Common::Rect vpRect;            // viewport-relative position (for collision and rendering)
		int type = -1;                  // -1=empty, 0-3=brick types
		int neighborLeft  = -1;         // indices of live neighbors (-1=none)
		int neighborUp    = -1;
		int neighborRight = -1;
		int neighborDown  = -1;
		uint32 explosionTimer = 0;      // absolute ms when to remove sprite
		bool alive = false;             // false after explosion completes
		bool pendingExplosion = false;  // true while queued in explosion list (not yet erased)
	};

	// ---------- Subfunction translations ----------
	void paddleMovement();              // FUN_0044b99b
	void ballAndCollision();            // FUN_0044bca3
	void processExplosions();           // FUN_0044f9dd
	void updateTimer();                 // FUN_00450276
	void updateScore();                 // FUN_00450354
	int  getNextLevel() const;          // FUN_0044e1a8
	void initSublevel();                // FUN_0044d3c7
	void resetRound();                  // FUN_0044b78e
	void wallAndPaddleCollision(int &ballLeft, int &ballTop, int &ballRight, int &ballBottom,
	                             int &ballCenterX, int &ballCenterY);  // FUN_0044c378
	bool brickCollision(int &ballLeft, int &ballTop, int &ballRight, int &ballBottom,
	                     int &ballCenterX, int &ballCenterY);            // FUN_0044cad5
	void applyCollision();              // FUN_0044ced2
	bool ballExited(int ballLeft, int ballTop, int ballRight, int ballBottom,
	                int &ballCenterX, int &ballCenterY);                 // FUN_0044d20e
	void playBrickHitSound();           // FUN_0044fff5
	void addToExplosionList(int brickIdx, uint32 delay);
	void generateBricks();              // FUN_0044e0b5
	void buildAngleTable();				// FUN_0044e1a8
	void drawBrick(int idx);
	void eraseBrick(int idx);
	void drawPaddle();
	void erasePaddle();
	void drawBall();
	void eraseBall();
	void drawDigit(const Common::Rect &destRect, int digit, bool useTimerRects);
	void drawTimer();
	void drawScore();

	// ---------- Data (read from stream) ----------
	Common::Path _imageName;

	uint32 _numLevelsToWin = 0;

	// Per-level grid dimensions (0x25): [cols0, rows0, cols1, rows1, ...]
	uint32 _levelCols[6] = {};
	uint32 _levelRows[6] = {};

	// Per-level grid offsets within viewport (0x55): [xOff0, yOff0, ...]
	int32 _levelXOff[6] = {};
	int32 _levelYOff[6] = {};

	Common::Rect _ballSrc;          // source rect in image for ball
	Common::Rect _paddleSrc;        // source rect in image for paddle
	Common::Rect _brickTypeSrc[8];  // source rects for brick types 0..7 (only 0-3 used)

	Common::Rect _scoreDigitSrc[10]; // 10-frame sprite sheet for score digits (0x125)
	Common::Rect _timerDigitSrc[10]; // 10-frame sprite sheet for timer digits (0x1c5)

	Common::Rect _timerDisplayDest; // destination on screen for timer (0x265)

	Common::Rect _lifeSrc[3];       // life indicator source rects (0x275, 0x285, 0x295)

	uint32 _stateDelayMs = 0;       // wait-state duration in ms (0x2a5)

	// Viewport rect as written in data (0x2a9..0x2b8)
	// We validate against actual viewport but don't store all fields

	// Display positions
	int32 _timerDisplayX = 0, _timerDisplayY = 0;   // 0x2c9, 0x2cd (+ viewport offset)
	int32 _scoreDisplayX = 0, _scoreDisplayY = 0;   // 0x2c1, 0x2c5 (+ viewport offset)

	int32 _deathYDist = 0;   // pixels from playfield bottom to death line (0x2d9)

	// Speed parameters: steps = ms interval (int); pixPerStep = pixels per interval (float in data)
	uint32 _paddleSteps = 1;         // 0x2e9 (integer)
	uint32 _ballSteps   = 1;         // 0x2ed (integer)
	float  _paddlePixPerStep = 1.0f; // 0x2f1 (IEEE 754 float stored in binary)
	float  _ballPixPerStep   = 1.0f; // 0x2f5 (IEEE 754 float stored in binary)

	int32 _angleTableStart = 45; // starting angle in degrees (0x2f9)
	int32 _angleTableEnd   = 90; // ending angle (0x2fd)

	bool _randomBallStart = false; // random ball starting offset (0x301)
	bool _wallBounceMode  = false; // true=ball bounces off bottom, false=dies (0x302)
	bool _cumulativeScore = false; // true=score accumulates across levels (0x303)

	int32 _scoreStepSize  = 1;    // score per brick tier (0x305)
	int32 _timeBonusMax   = 100;  // max time bonus (0x309)
	int32 _timeLimitSec   = 60;   // time limit in seconds (0x30d)

	// Sounds: 6 permanent + 3 dynamic
	SoundDescription _sounds[6];         // 0x315..0x43a (bounce, launch, brick hit A/B/C, wall)
	SoundDescription _levelClearSound;   // 0x43b
	SoundDescription _gameOverSound;     // 0x486
	SoundDescription _lifeLostSound;     // 0x4b7

	// Win scene (0x46c, 25 bytes)
	SceneChangeWithFlag _winScene;

	// ---------- Runtime state ----------

	enum GameSubState {
		kPlaying      = 0, // normal gameplay loop
		kLevelClear   = 1, // play level-clear sound then wait
		kLifeLost     = 2, // decrement lives, play sound, then wait
		kResetBoard   = 3, // init new sublevel
		kGameOverWin  = 4, // play game-over sound then wait
		kFinish       = 5, // trigger win/lose scene
		kWaitTimer    = 6, // wait for _stateWaitUntil
		kGameOverLose = 7  // wait then finish (no lives left)
	};

	enum BallState {
		kOnPaddle  = 0, // resting on paddle before launch
		kInFlight  = 1, // moving freely
		kDying     = 2  // ball has exited the playfield
	};

	GameSubState _gameSubState = kPlaying;

	// Flags set by gameplay events
	bool _winGame     = false;   // 0x18
	bool _levelClear  = false;   // 0x1c
	bool _lifeLost    = false;   // 0x20
	bool _lifeLostBall = false;  // 0x24 (set by ball exit, used to stop paddle)
	bool _gameHalted  = false;   // 0x28

	int _livesLeft = 3;          // remaining lives (starts at 3)

	uint32 _stateWaitUntil = 0;  // absolute ms for wait-state expiry

	// Level tracking
	int  _currentLevel = 0;      // 0xb0
	uint32 _winFlags[6] = {};    // 0x270..0x284
	int32 _levelScore[6] = {};   // 0x254..0x268 (accumulated score per level)
	int32 _totalLevelScore = 0;  // 0x26c
	int32 _score    = 0;         // 0xb4 (displayed score)
	int32 _prevScore = -1;       // 0x298 (to detect changes)

	// Brick grid for current sublevel
	int _brickCols  = 0; // 0x2ac[level]
	int _brickRows  = 0; // 0x2b0[level]
	int _totalBricks = 0; // 0xac

	// Brick area screen coords (playfield-relative = viewport-relative)
	int _brickAreaLeft = 0;    // 0x31c
	int _brickAreaTop  = 0;    // 0x320
	int _brickAreaRight = 0;   // 0x324
	int _brickAreaBottom = 0;  // 0x328
	int _brickWidth  = 0;      // 0xa4
	int _brickHeight = 0;      // 0xa8

	Common::Array<Brick> _bricks;

	// Explosion list (0x6a7)
	Common::List<int> _explosionList;
	int _brickSoundRotator = 0;  // 0x6af: cycles 0,1,2 for brick hit sounds A/B/C

	// Playfield screen rect (where the game is drawn)
	int _fieldLeft = 0, _fieldTop = 0, _fieldRight = 0, _fieldBottom = 0; // 0x3c..0x48
	int _fieldWidth = 0, _fieldHeight = 0; // 0x35c, 0x360
	int _fieldOffX = 0, _fieldOffY = 0;    // 0x4c, 0x50

	// Death line: ball dies if it passes _deathY (y coord on viewport)
	int _deathY = 0;  // 0xa0

	// Paddle state
	Common::Rect _paddleSrcCur; // current paddle source (may differ for damaged paddle)
	int _paddleLeft = 0, _paddleTop = 0, _paddleRight = 0, _paddleBottom = 0; // 0xc8..0xd4
	int _paddlePrevLeft = 0, _paddlePrevTop = 0, _paddlePrevRight = 0, _paddlePrevBottom = 0;
	int _paddleWidth = 0, _paddleHeight = 0, _paddleHalfW = 0; // 0x94, 0x98, 0x9c
	float _paddleX = 0.0f;   // 0x50c (floating-point left edge)
	float _paddleSpeedPerMs = 1.0f; // pixels/ms (derived from _paddleSteps/_paddlePixPerStep)
	uint32 _paddleLastMs = 0;  // 0x4e8
	bool _paddleNeedsRedraw = false; // 0xc0

	// Ball state
	BallState _ballState = kOnPaddle;  // 0x180
	float _ballX = 0.0f, _ballY = 0.0f; // floating-point position; 0x100, 0x104
	float _ballDX = 0.0f, _ballDY = 1.0f; // velocity direction; 0xe8, 0xec
	float _ballSpin = 0.0f; // 0xf0
	int _ballLeft = 0, _ballTop = 0, _ballRight = 0, _ballBottom = 0;   // 0x118..0x124
	int _ballPrevLeft = 0, _ballPrevTop = 0, _ballPrevRight = 0, _ballPrevBottom = 0; // 0x150..0x15c
	int _ballCenterX = 0, _ballCenterY = 0; // 0x148, 0x14c
	int _ballInitOffset = 0;  // offset from paddle left for initial position; 0x174
	int _ballWidth = 0, _ballHeight = 0, _ballHalfW = 0, _ballHalfH = 0; // 0x164..0x170
	float _ballSpeedPerMs = 1.0f; // pixels/ms
	uint32 _ballLastMs = 0;  // 0x178
	bool _ballNeedsRedraw = false; // 0x160
	int _collisionType = -2;  // 0x184 (-2 = no collision yet)

	// Dead-ball state (when ball is dying, this tracks the ball's last valid rect)
	int _deadBallSrcLeft = 0, _deadBallSrcTop = 0, _deadBallSrcRight = 0, _deadBallSrcBottom = 0; // 0x128..0x134
	int _deadBallLeft = 0, _deadBallTop = 0, _deadBallRight = 0, _deadBallBottom = 0; // 0x138..0x144

	// Wall normals (0x188 + type*12, 3 floats each)
	// Only dx, dy used (index 0 and 1 per entry)
	static const float _wallNormals[17][3]; // for collision types 0..16 (0xf = 15, 0x10 = 16)

	// Angle table (dynamically allocated, size = _paddleWidth * 2 * 3 floats)
	// table[hitPos * 2 + random_bit] = {dx, dy, spin}
	Common::Array<float> _angleTable; // 0x51c

	// Timer tracking
	uint32 _timerStartMs = 0;   // 0x344
	uint32 _timerElapsedMs = 0; // 0x348
	uint32 _timerMins = 0, _timerSecs = 0; // 0x33c, 0x340 (for display)

	// Time bonus
	float _timeBonusMultiplier = 1.0f; // 0x29c

	// Input state (updated each handleInput call, consumed in execute/paddleMovement)
	bool _moveLeft  = false;
	bool _moveRight = false;
	bool _launchBall = false;

	// Surfaces
	Graphics::ManagedSurface _image;
	Graphics::ManagedSurface _backgroundCache; // copy of initial draw surface for erasing
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_ARCADEPUZZLE_H
