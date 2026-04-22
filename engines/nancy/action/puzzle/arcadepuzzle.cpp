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

#include "common/system.h"
#include "common/random.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/arcadepuzzle.h"

namespace Nancy {
namespace Action {

// Wall normals for collision types 0..16.
// Each entry is {dx, dy, spin} matching original offsets at this+0x188 + type*0xc.
// Type 0 = paddle center (handled specially by FUN_0044d03d — normal not used here).
// Positive dy = up in "math" coords, i.e. y -= dy*speed on screen.
const float ArcadePuzzle::_wallNormals[17][3] = {
	{  0.0f,  1.0f, 0.0f }, // 0  paddle center (placeholder — handled separately)
	{ -1.0f,  0.0f, 0.0f }, // 1  paddle left edge
	{  1.0f,  0.0f, 0.0f }, // 2  paddle right edge
	{  1.0f,  0.0f, 0.0f }, // 3  left wall
	{  0.0f, -1.0f, 0.0f }, // 4  top wall
	{ -1.0f,  0.0f, 0.0f }, // 5  right wall
	{  0.0f,  1.0f, 0.0f }, // 6  bottom wall (wallBounceMode)
	{ -1.0f,  0.0f, 0.0f }, // 7  brick left face
	{  0.0f,  1.0f, 0.0f }, // 8  brick top face
	{  1.0f,  0.0f, 0.0f }, // 9  brick right face
	{  0.0f, -1.0f, 0.0f }, // 10 brick bottom face
	{  0.7071068f, -0.7071068f, 0.0f }, // 0xb top-left corner
	{ -0.7071068f, -0.7071068f, 0.0f }, // 0xc top-right corner
	{  0.7071068f,  0.7071068f, 0.0f }, // 0xd bottom-left corner
	{ -0.7071068f,  0.7071068f, 0.0f }, // 0xe bottom-right corner
	{  0.0f,  1.0f, 0.0f }, // 0xf stuck/bounce-back
	{  0.0f,  0.0f, 0.0f }, // 0x10 out-of-bounds catch-all (negate)
};

ArcadePuzzle::~ArcadePuzzle() {
	_explosionList.clear();
}

// ---- readData ---------------------------------------------------------------

void ArcadePuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);               // 33 bytes, +0x00

	_numLevelsToWin = stream.readUint32LE();              // 4 bytes,  +0x21

	// Per-level grid dimensions: [cols0,rows0, cols1,rows1, ...] (48 bytes, 0x25)
	for (int i = 0; i < 6; ++i) {
		_levelCols[i] = stream.readUint32LE();
		_levelRows[i] = stream.readUint32LE();
	}

	// Per-level offsets within field (48 bytes, +0x55)
	for (int i = 0; i < 6; ++i) {
		_levelXOff[i] = stream.readSint32LE();
		_levelYOff[i] = stream.readSint32LE();
	}

	readRect(stream, _ballSrc);                    // 16 bytes, +0x85
	readRect(stream, _paddleSrc);                  // 16 bytes, +0x95
	for (int i = 0; i < 8; ++i)                          // 128 bytes, +0xa5
		readRect(stream, _brickTypeSrc[i]);

	// Score digit sprites (10 × 16 = 160 bytes, +0x125)
	for (int i = 0; i < 10; ++i)
		readRect(stream, _scoreDigitSrc[i]);

	// Timer digit sprites (10 × 16 = 160 bytes, +0x1c5)
	for (int i = 0; i < 10; ++i)
		readRect(stream, _timerDigitSrc[i]);

	readRect(stream, _timerDisplayDest);           // 16 bytes, +0x265
	readRect(stream, _lifeSrc[0]);                 // 16 bytes, +0x275
	readRect(stream, _lifeSrc[1]);                 // 16 bytes, +0x285
	readRect(stream, _lifeSrc[2]);                 // 16 bytes, +0x295

	_stateDelayMs = stream.readUint32LE();               // 4 bytes,  +0x2a5

	stream.skip(4 * 4);                            // viewport rect, 4x4 bytes - we get it from NancySceneState

	_fieldOffX = stream.readSint32LE();					 // +0x2b9
	_fieldOffY = stream.readSint32LE();					 // +0x2bd

	_scoreDisplayX = stream.readSint32LE();              // +0x2c1
	_scoreDisplayY = stream.readSint32LE();              // +0x2c5
	_timerDisplayX = stream.readSint32LE();              // +0x2c9
	_timerDisplayY = stream.readSint32LE();              // +0x2cd

	stream.skip(4);                                // +0x2d1 (unused render field)
	stream.skip(4);                                // +0x2d5 (unused render field)
	_deathYDist = stream.readSint32LE();                 // +0x2d9

	stream.skip(4);                                // +0x2dd left VK key code (unused)
	stream.skip(4);                                // +0x2e1 right VK key code (unused)
	stream.skip(4);                                // +0x2e5 launch VK key code (unused)

	_paddleSteps      = stream.readUint32LE();           // +0x2e9 (integer)
	_ballSteps        = stream.readUint32LE();           // +0x2ed (integer)
	_paddlePixPerStep = stream.readFloatLE();            // +0x2f1 (IEEE 754 float)
	_ballPixPerStep   = stream.readFloatLE();            // +0x2f5 (IEEE 754 float)
	_angleTableStart  = stream.readSint32LE();           // +0x2f9
	_angleTableEnd    = stream.readSint32LE();           // +0x2fd

	_randomBallStart  = stream.readByte() != 0;          // +0x301
	_wallBounceMode   = stream.readByte() != 0;          // +0x302
	_cumulativeScore  = stream.readByte() != 0;          // +0x303
	stream.skip(1);                                // +0x304 (random seed flag, unused)

	_scoreStepSize    = stream.readSint32LE();           // +0x305
	_timeBonusMax     = stream.readSint32LE();           // +0x309
	_timeLimitSec     = stream.readSint32LE();           // +0x30d
	stream.skip(4);                                      // +0x311 (unused score param)

	for (int i = 0; i < 6; ++i)                          // 6×49 = 294 bytes, +0x315..+0x43a
		_sounds[i].readNormal(stream);

	_levelClearSound.readNormal(stream);              // 49 bytes, +0x43b..+0x46b
	_winScene.readData(stream);                       // 25 bytes, +0x46c..+0x484
	stream.skip(1);                                // 1-byte gap, +0x485

	_gameOverSound.readNormal(stream);                // 49 bytes, +0x486..+0x4b6
	_lifeLostSound.readNormal(stream);                // 49 bytes, +0x4b7..+0x4e7
}

// ---- init -------------------------------------------------------------------

void ArcadePuzzle::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(vpBounds.width(), vpBounds.height(),
	                    g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(vpBounds);

	g_nancy->_resource->loadImage(_imageName, _image);
	_image.setTransparentColor(_drawSurface.getTransparentColor());

	// Determine game field bounds within viewport
	int vpDataWidth  = (vpBounds.right  - vpBounds.left);
	int vpDataHeight = (vpBounds.bottom - vpBounds.top);

	// Field may be offset within the viewport
	_fieldLeft   = _fieldOffX;
	_fieldTop    = _fieldOffY;
	_fieldRight  = _fieldOffX + vpDataWidth  - 1;
	_fieldBottom = _fieldOffY + vpDataHeight - 1;
	_fieldWidth  = vpDataWidth;
	_fieldHeight = vpDataHeight;

	// Death line: ball dies when it passes this y coord
	_deathY = _fieldBottom - _deathYDist;

	// Paddle dimensions from paddle src rect
	_paddleWidth  = _paddleSrc.width();
	_paddleHeight = _paddleSrc.height();
	_paddleHalfW  = _paddleWidth / 2;

	// Ball dimensions from ball src rect
	_ballWidth  = _ballSrc.width();
	_ballHeight = _ballSrc.height();
	_ballHalfW  = _ballWidth / 2;
	_ballHalfH  = _ballHeight / 2;

	// Brick dimensions from first brick type src
	_brickWidth  = _brickTypeSrc[0].width();
	_brickHeight = _brickTypeSrc[0].height();

	// Derived speeds (pixels per ms) — pixPerStep is float from data, steps is integer
	_paddleSpeedPerMs = (_paddleSteps > 0)
	    ? _paddlePixPerStep / (float)_paddleSteps : 1.0f;
	_ballSpeedPerMs   = (_ballSteps > 0)
	    ? _ballPixPerStep   / (float)_ballSteps   : 1.0f;

	// Build angle table
	buildAngleTable();

	// Score/timer display positions adjusted for field offset
	// (stored coords are relative to field, so add _fieldLeft/_fieldTop)
	// (stored as-is; drawing uses them with +field offset)

	// Initialize sublevel (finds first unbeaten level, sets up bricks, paddle, ball)
	initSublevel();
}

// ---- buildAngleTable --------------------------------------------------------
// Populates _angleTable with (dx,dy,spin) pairs.
// Each paddle position has 2 sub-entries: direct (random_bit=0) and mirrored (random_bit=1).
// Layout: _angleTable[pos*6 + sub*3 + {0,1,2}] = {dx, dy, spin}.

void ArcadePuzzle::buildAngleTable() {
	_angleTable.resize(_paddleWidth * 6, 0.0f);

	if (_paddleWidth == 0)
		return;

	int halfW = _paddleWidth / 2;
	float angleStep = (_paddleWidth > 1)
	    ? (float)(_angleTableEnd - _angleTableStart) / (float)halfW : 0.0f;
	float curAngleDeg = (float)_angleTableStart;

	// Mirror normal: (0, 1) — reflects across y-axis
	float mirrorNx = 0.0f, mirrorNy = 1.0f;

	int left  = 0;
	int right = _paddleWidth - 1;

	while (left <= halfW) {
		// Rotate unit vector (1,0) by curAngleDeg
		float angleRad = curAngleDeg * (float)M_PI / 180.0f;
		float dx = (float)cos(angleRad);
		float dy = (float)sin(angleRad);
		// Normalize (already unit length from cos/sin, but in case of precision)
		float len = (float)sqrt(dx * dx + dy * dy);
		if (len > 0.0f) { dx /= len; dy /= len; }

		// sub-entry 0: direct
		_angleTable[left * 6 + 0] = dx;
		_angleTable[left * 6 + 1] = dy;
		_angleTable[left * 6 + 2] = 0.0f; // spin always 0

		// Copy to right position (symmetric)
		_angleTable[right * 6 + 0] = dx;
		_angleTable[right * 6 + 1] = dy;
		_angleTable[right * 6 + 2] = 0.0f;

		// sub-entry 1: reflect across y-axis: (-dx, dy)
		// FUN_00447eb9 with no-negate: result = 2*(v·n)*n - v
		// n=(0,1), v=(dx,dy): result = (-dx, dy)
		float dot  = dx * mirrorNx + dy * mirrorNy; // = dy
		float rdx  = 2.0f * dot * mirrorNx - dx;    // = -dx
		float rdy  = 2.0f * dot * mirrorNy - dy;    // = dy
		// Normalize
		float rlen = (float)sqrt(rdx * rdx + rdy * rdy);
		if (rlen > 0.0f) { rdx /= rlen; rdy /= rlen; }

		_angleTable[left * 6 + 3] = rdx;
		_angleTable[left * 6 + 4] = rdy;
		_angleTable[left * 6 + 5] = 0.0f;

		_angleTable[right * 6 + 3] = rdx;
		_angleTable[right * 6 + 4] = rdy;
		_angleTable[right * 6 + 5] = 0.0f;

		curAngleDeg += angleStep;

		if (left + 1 == right || right <= left)
			break;
		++left;
		--right;
	}
}

// ---- resetRound ------------------------------------------------------------
// Clears bricks, resets flags and timers. Called at start of each sublevel.

void ArcadePuzzle::resetRound() {
	// Free bricks array
	_bricks.clear();

	// Free explosion list
	_explosionList.clear();

	// Reset state flags
	_levelClear   = false;
	_lifeLost     = false;
	_lifeLostBall = false;
	_gameHalted   = false;
	_score        = 0;
	_launchBall   = false;

	// Reset timers to now
	_paddleLastMs = _ballLastMs = g_system->getMillis();
}

// ---- getNextLevel ----------------------------------------------------------
// Returns index of first unbeaten level, or -1 if enough levels are won.

int ArcadePuzzle::getNextLevel() const {
	int won = 0;
	for (int i = 0; i < 6; ++i)
		if (_winFlags[i]) ++won;

	if (won < (int)_numLevelsToWin) {
		for (int i = 0; i < 6; ++i)
			if (!_winFlags[i])
				return i;
	}
	return -1;
}

// ---- generateBricks --------------------------------------------------------
// Randomly assigns brick types, retrying until >=50% are non-empty.

void ArcadePuzzle::generateBricks() {
	bool ok = false;
	while (!ok) {
		int filled = 0;
		for (int i = 0; i < _totalBricks; ++i) {
			int r = g_nancy->_randomSource->getRandomNumber(4); // 0..4
			if (r == 4) {
				_bricks[i].type = -1; // empty
			} else {
				_bricks[i].type = r;  // type 0-3
				++filled;
			}
		}
		if (filled * 2 >= _totalBricks) // >= 50%
			ok = true;
	}
}

// ---- initSublevel ----------------------------------------------------------

void ArcadePuzzle::initSublevel() {
	_currentLevel = getNextLevel();
	if (_currentLevel < 0)
		return; // shouldn't happen

	resetRound();

	// Grid dimensions for this level
	_brickCols  = (int)_levelCols[_currentLevel];
	_brickRows  = (int)_levelRows[_currentLevel];
	_totalBricks = _brickCols * _brickRows;

	// Brick area position (viewport-relative)
	_brickAreaLeft   = _fieldLeft + _levelXOff[_currentLevel];
	_brickAreaTop    = _fieldTop  + _levelYOff[_currentLevel];
	_brickAreaRight  = _brickAreaLeft + _brickCols * _brickWidth  - 1;
	_brickAreaBottom = _brickAreaTop  + _brickRows * _brickHeight - 1;

	// Paddle initial position: centered in field, bottom at deathY
	_paddleLeft   = (_fieldLeft + _fieldWidth / 2) - _paddleHalfW;
	_paddleRight  = _paddleLeft + _paddleWidth - 1;
	_paddleBottom = _deathY;
	_paddleTop    = _paddleBottom - _paddleHeight + 1;
	_paddlePrevLeft  = _paddleLeft;  _paddlePrevTop   = _paddleTop;
	_paddlePrevRight = _paddleRight; _paddlePrevBottom= _paddleBottom;
	_paddleX     = (float)_paddleLeft;
	_paddleSrcCur= _paddleSrc;

	// Ball initial position: centered above paddle
	if (!_randomBallStart) {
		_ballInitOffset = _paddleHalfW - _ballHalfW;
	} else {
		_ballInitOffset = g_nancy->_randomSource->getRandomNumber(
		    MAX(1, _paddleWidth - _ballWidth));
	}
	_ballLeft    = _paddleLeft + _ballInitOffset;
	_ballRight   = _ballLeft + _ballWidth - 1;
	_ballBottom  = _paddleTop;
	_ballTop     = _paddleTop - _ballHeight + 1;
	_ballCenterX = _ballLeft + _ballHalfW;
	_ballCenterY = _ballTop  + _ballHalfH;
	_ballX = (float)_ballLeft;
	_ballY = (float)_ballTop;
	_ballPrevLeft = _ballLeft; _ballPrevTop = _ballTop;
	_ballPrevRight= _ballRight;_ballPrevBottom=_ballBottom;
	_ballState    = kOnPaddle;
	_ballNeedsRedraw = false;
	_collisionType   = -2;

	// Default ball direction (straight up)
	_ballDX = 0.0f;
	_ballDY = 1.0f;
	_ballSpin = 0.0f;

	// Allocate bricks
	_bricks.resize(_totalBricks);
	generateBricks();

	// Set up brick rects
	int py = _brickAreaTop;
	for (int row = 0; row < _brickRows; ++row) {
		int px = _brickAreaLeft;
		for (int col = 0; col < _brickCols; ++col) {
			int idx = row * _brickCols + col;
			Brick &b = _bricks[idx];

			// Source rect in image (from brick type sprite)
			if (b.type >= 0 && b.type < 8) {
				b.srcRect = _brickTypeSrc[b.type];
			} else {
				b.srcRect = Common::Rect();
			}

			// Viewport-relative rect for this brick cell
			b.vpRect = Common::Rect(px, py, px + _brickWidth, py + _brickHeight);

			b.alive = (b.type >= 0);

			b.neighborLeft  = -1;
			b.neighborUp    = -1;
			b.neighborRight = -1;
			b.neighborDown  = -1;

			b.explosionTimer = 0;

			px += _brickWidth;
		}
		py += _brickHeight;
	}

	// Link neighbors (only live bricks)
	for (int row = 0; row < _brickRows; ++row) {
		for (int col = 0; col < _brickCols; ++col) {
			int idx = row * _brickCols + col;
			if (!_bricks[idx].alive) continue;

			if (row > 0 && _bricks[idx - _brickCols].alive)
				_bricks[idx].neighborUp    = idx - _brickCols;
			if (row < _brickRows - 1 && _bricks[idx + _brickCols].alive)
				_bricks[idx].neighborDown  = idx + _brickCols;
			if (col > 0 && _bricks[idx - 1].alive)
				_bricks[idx].neighborLeft  = idx - 1;
			if (col < _brickCols - 1 && _bricks[idx + 1].alive)
				_bricks[idx].neighborRight = idx + 1;
		}
	}

	// Reset timer and score for this level
	_timerStartMs   = g_system->getMillis();
	_timerElapsedMs = 0;
	_timerMins = 0; _timerSecs = 0;
	_levelScore[_currentLevel] = 0;
	_prevScore = -1;

	if (!_cumulativeScore)
		_score = 0;
	else {
		// Cumulative: sum of all level scores
		_score = _totalLevelScore;
	}

	// Draw initial state
	_drawSurface.clear(g_nancy->_graphics->getTransColor());

	for (int i = 0; i < _totalBricks; ++i)
		if (_bricks[i].alive)
			drawBrick(i);

	drawPaddle();
	drawBall();
	drawScore();
	drawTimer();

	_needsRedraw = true;
}

// ---- execute ----------------------------------------------------------------

void ArcadePuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();

		// Load sounds
		for (int i = 0; i < 6; ++i)
			g_nancy->_sound->loadSound(_sounds[i]);
		// Dynamic sounds are loaded when needed

		_livesLeft    = 3;
		_state = kRun;
		break;

	case kRun: {
		switch (_gameSubState) {
		case kPlaying:
			paddleMovement();
			ballAndCollision();
			processExplosions();
			updateTimer();
			updateScore();

			if (_levelClear) {
				// Mark current level as won
				_winFlags[_currentLevel] = 1;

				// Recalculate total score
				_totalLevelScore = 0;
				for (int i = 0; i < 6; ++i)
					_totalLevelScore += _levelScore[i];

				if (getNextLevel() == -1) {
					// All required levels beaten
					_winGame = true;
					_levelClear = false;
					_gameSubState = kGameOverWin;
				} else {
					_gameSubState = kLevelClear;
				}
			}

			if (_lifeLost) {
				// Reset current level flags and score
				_winFlags[_currentLevel] = 0;
				_levelScore[_currentLevel] = 0;
				_gameSubState = kLifeLost;
			}
			break;

		case kLevelClear:
			if (_levelClearSound.name != "NO SOUND") {
				g_nancy->_sound->loadSound(_levelClearSound);
				g_nancy->_sound->playSound(_levelClearSound);
			}
			_stateWaitUntil = g_system->getMillis() + _stateDelayMs;
			_gameSubState = kWaitTimer;
			break;

		case kLifeLost:
			--_livesLeft;
			if (_livesLeft <= 0) {
				// No lives left: game over lose
				_lifeLost     = false;
				_winGame      = false;
				if (_gameOverSound.name != "NO SOUND") {
					g_nancy->_sound->loadSound(_gameOverSound);
					g_nancy->_sound->playSound(_gameOverSound);
				}
				_stateWaitUntil = g_system->getMillis() + _stateDelayMs;
				_gameSubState = kGameOverLose;
				break;
			}
			if (_lifeLostSound.name != "NO SOUND") {
				g_nancy->_sound->loadSound(_lifeLostSound);
				g_nancy->_sound->playSound(_lifeLostSound);
			}
			_stateWaitUntil = g_system->getMillis() + _stateDelayMs;
			_gameSubState = kWaitTimer;
			break;

		case kResetBoard:
			initSublevel();
			drawScore();
			drawTimer();
			_gameSubState = kPlaying;
			break;

		case kGameOverWin:
			if (_gameOverSound.name != "NO SOUND") {
				g_nancy->_sound->loadSound(_gameOverSound);
				g_nancy->_sound->playSound(_gameOverSound);
			}
			_stateWaitUntil = g_system->getMillis() + _stateDelayMs;
			_gameSubState = kWaitTimer;
			break;

		case kFinish:
			_state = kActionTrigger;
			break;

		case kWaitTimer:
			if (g_system->getMillis() > _stateWaitUntil) {
				if (_levelClear || _lifeLost) {
					_gameSubState = kResetBoard;
				} else if (_winGame) {
					_gameSubState = kFinish;
				}
				// else nothing (shouldn't happen)
			}
			break;

		case kGameOverLose:
			if (g_system->getMillis() > _stateWaitUntil)
				_gameSubState = kFinish;
			break;

		default:
			break;
		}
		break;
	}

	case kActionTrigger:
		// Stop all sounds
		for (int i = 0; i < 6; ++i)
			g_nancy->_sound->stopSound(_sounds[i]);
		g_nancy->_sound->stopSound(_levelClearSound);
		g_nancy->_sound->stopSound(_gameOverSound);
		g_nancy->_sound->stopSound(_lifeLostSound);

		if (_livesLeft > 0) {
			_winScene.execute();
		} else {
			// Change to the same destination scene as winning, but without setting
			// the win flag. The game script checks that flag to determine win/lose
			// and will show the "you lost" message when it is absent.
			NancySceneState.changeScene(_winScene._sceneChange);
		}

		finishExecution();
		break;
	}
}

// ---- handleInput ------------------------------------------------------------

void ArcadePuzzle::handleInput(NancyInput &input) {
	if (_state != kRun)
		return;

	// Update movement flags (these are "held" flags)
	_moveLeft  = (input.input & NancyInput::kMoveLeft)  != 0;
	_moveRight = (input.input & NancyInput::kMoveRight) != 0;

	// Launch ball: check for space or up-arrow
	if (_ballState == kOnPaddle) {
		for (const Common::KeyState &ks : input.otherKbdInput) {
			if (ks.keycode == Common::KEYCODE_SPACE ||
			    ks.keycode == Common::KEYCODE_RETURN ||
			    ks.keycode == Common::KEYCODE_UP) {
				_launchBall = true;
				break;
			}
		}
		if (input.input & NancyInput::kMoveUp)
			_launchBall = true;
	}
}

// ---- paddleMovement ---------------------------------------------------------

void ArcadePuzzle::paddleMovement() {
	// Paddle stops when game is halted or ball is dying
	if (_gameHalted || _lifeLostBall)
		return;

	uint32 now = g_system->getMillis();
	uint32 dt  = now - _paddleLastMs;
	_paddleLastMs = now;

	if (!_moveLeft && !_moveRight)
		return;

	// Save previous position
	_paddlePrevLeft  = _paddleLeft;
	_paddlePrevTop   = _paddleTop;
	_paddlePrevRight = _paddleRight;
	_paddlePrevBottom= _paddleBottom;

	if (_moveLeft) {
		_paddleX -= (float)dt * _paddleSpeedPerMs;
		if (_paddleX < (float)_fieldLeft) {
			_paddleX = (float)_fieldLeft;
		}
		_paddleLeft  = (int)_paddleX;
		_paddleRight = _paddleLeft + _paddleWidth - 1;
	} else { // _moveRight
		_paddleX += (float)dt * _paddleSpeedPerMs;
		if (_paddleX + (float)_paddleWidth - 1.0f > (float)_fieldRight) {
			_paddleX = (float)(_fieldRight - _paddleWidth + 1);
		}
		_paddleLeft  = (int)_paddleX;
		_paddleRight = _paddleLeft + _paddleWidth - 1;
	}

	if (_paddleLeft  != _paddlePrevLeft  || _paddleRight != _paddlePrevRight ||
	    _paddleTop   != _paddlePrevTop   || _paddleBottom!= _paddlePrevBottom) {
		_paddleNeedsRedraw = true;
	}
}

// ---- ballAndCollision -------------------------------------------------------

void ArcadePuzzle::ballAndCollision() {
	uint32 now = g_system->getMillis();

	// Update float position while in flight or dying
	if (_ballState == kInFlight || _ballState == kDying) {
		uint32 dt = now - _ballLastMs;
		_ballLastMs = now;

		_ballX += _ballDX * _ballSpeedPerMs * (float)dt;
		_ballY -= _ballDY * _ballSpeedPerMs * (float)dt;
	}

	if (_ballState == kOnPaddle) {
		// Ball sitting on paddle: follow paddle
		_ballPrevLeft  = _ballLeft;
		_ballPrevTop   = _ballTop;
		_ballPrevRight = _ballRight;
		_ballPrevBottom= _ballBottom;

		_ballLeft    = _paddleLeft + _ballInitOffset;
		_ballRight   = _ballLeft + _ballWidth - 1;
		_ballBottom  = _paddleTop;
		_ballTop     = _paddleTop - _ballHeight + 1;
		_ballCenterX = _ballLeft + _ballHalfW;
		_ballCenterY = _ballTop  + _ballHalfH;
		_ballX = (float)_ballLeft;
		_ballY = (float)_ballTop;

		if (_ballLeft  != _ballPrevLeft  || _ballRight != _ballPrevRight ||
		    _ballTop   != _ballPrevTop   || _ballBottom!= _ballPrevBottom)
			_ballNeedsRedraw = true;

		// Launch?
		if (_launchBall) {
			_launchBall = false;
			// Pick sub-entry based on random bit
			int hitPos  = _ballInitOffset + _ballHalfW; // offset of ball center from paddle left
			hitPos = CLIP(hitPos, 0, _paddleWidth - 1);
			uint32 rbit = g_nancy->_randomSource->getRandomBit();
			int tableIdx = hitPos * 6 + rbit * 3;
			_ballDX   = _angleTable[tableIdx + 0];
			_ballDY   = _angleTable[tableIdx + 1];
			_ballSpin = _angleTable[tableIdx + 2];
			_ballState = kInFlight;
			_ballLastMs = g_system->getMillis();
			g_nancy->_sound->playSound(_sounds[1]); // launch sound
		}
	} else if (_ballState == kInFlight) {
		// Ball in flight: save previous, compute new integer position
		_ballPrevLeft  = _ballLeft;
		_ballPrevTop   = _ballTop;
		_ballPrevRight = _ballRight;
		_ballPrevBottom= _ballBottom;

		int ballLeft = (int)_ballX;
		int ballTop = (int)_ballY;
		int ballRight = ballLeft + _ballWidth  - 1;
		int ballBottom = ballTop + _ballHeight - 1;
		int ballCenterX = ballLeft + _ballHalfW;
		int ballCenterY = ballTop + _ballHalfH;

		// Detect and resolve collision
		wallAndPaddleCollision(ballLeft, ballTop, ballRight, ballBottom, ballCenterX, ballCenterY);

		// Snap float position for certain collision types
		switch (_collisionType) {
		case 0: case 4: case 6: case 8: case 10:
			_ballY = (float)ballTop; break;
		case 1: case 2: case 3: case 5: case 7: case 9:
			_ballX = (float)ballLeft; break;
		case 0xb: case 0xc: case 0xd: case 0xe:
			_ballX = (float)ballLeft;
			_ballY = (float)ballTop;
			break;
		case 0x10:
			_ballX = (float)_ballLeft;
			_ballY = (float)_ballTop;
			break;
		default:
			break;
		}

		// Apply position (unless no-move types)
		if (_collisionType != 0xf && _collisionType != -1 && _collisionType != 0x10) {
			_ballLeft   = ballLeft;
			_ballTop    = ballTop;
			_ballRight  = ballRight;
			_ballBottom = ballBottom;
			_ballCenterX = ballCenterX;
			_ballCenterY = ballCenterY;
		}

		// Apply velocity change
		applyCollision();

		if (_ballLeft  != _ballPrevLeft  || _ballRight != _ballPrevRight ||
		    _ballTop   != _ballPrevTop   || _ballBottom!= _ballPrevBottom)
			_ballNeedsRedraw = true;

		if (_collisionType == -1) {
			// Ball died
			_ballState    = kDying;
			_lifeLostBall = true;
			_deadBallLeft   = _ballLeft;  _deadBallTop    = _ballTop;
			_deadBallRight  = _ballRight; _deadBallBottom = _ballBottom;
			_deadBallSrcLeft   = _ballSrc.left;  _deadBallSrcTop    = _ballSrc.top;
			_deadBallSrcRight  = _ballSrc.right; _deadBallSrcBottom = _ballSrc.bottom;
			_ballX = (float)_ballLeft;
			_ballY = (float)_ballTop;
		}
	} else if (_ballState == kDying) {
		// Dying ball: keep moving until it exits field
		_ballPrevLeft  = _deadBallLeft;
		_ballPrevTop   = _deadBallTop;
		_ballPrevRight = _deadBallRight;
		_ballPrevBottom= _deadBallBottom;

		int ballLeft = (int)_ballX;
		int ballTop = (int)_ballY;
		int ballRight = ballLeft + _ballWidth  - 1;
		int ballBottom = ballTop + _ballHeight - 1;

		if (!ballExited(ballLeft, ballTop, ballRight, ballBottom, ballLeft, ballTop)) {
			// Still visible — update dead ball display rect
			_ballX = (float)_deadBallLeft;
			_ballY = (float)_deadBallTop;
		} else {
			// Fully exited
			_lifeLost   = true;
			_gameHalted = true;
		}

		if (_deadBallLeft  != _ballPrevLeft  || _deadBallRight != _ballPrevRight ||
		    _deadBallTop   != _ballPrevTop   || _deadBallBottom!= _ballPrevBottom)
			_ballNeedsRedraw = true;
	}

	// Redraw if needed
	if (_paddleNeedsRedraw) {
		erasePaddle();
		drawPaddle();
		_paddleNeedsRedraw = false;
	}
	if (_ballNeedsRedraw) {
		Common::Rect prevBallRect(_ballPrevLeft, _ballPrevTop, _ballPrevRight + 1, _ballPrevBottom + 1);
		eraseBall();
		// Restore any brick pixels the ball erase may have cleared
		for (int i = 0; i < _totalBricks; ++i) {
			if (_bricks[i].alive && _bricks[i].vpRect.intersects(prevBallRect))
				drawBrick(i);
		}
		drawBall();
		// Restore HUD in case ball erase clipped into the score/timer area
		drawScore();
		drawTimer();
		_ballNeedsRedraw = false;
	}
}

// ---- wallAndPaddleCollision -------------------------------------------------
// Detects wall and paddle collisions, adjusts ball rect, sets _collisionType.

void ArcadePuzzle::wallAndPaddleCollision(int &ballLeft, int &ballTop, int &ballRight, int &ballBottom,
                                           int &ballCenterX, int &ballCenterY) {
	_collisionType = -2; // "not yet determined"

	// --- Paddle collision (ball coming from above the paddle top) ---
	if (_paddleLeft < ballLeft && ballRight < _paddleRight && _paddleTop < ballBottom) {
		// Ball rect crosses paddle top
		ballBottom = _paddleTop;
		ballTop = (ballBottom - _ballHeight) + 1;
		ballCenterY = ballTop + _ballHalfH;
		_collisionType = 0; // paddle center (direction-based)
	}
	// Left edge of paddle
	else if (ballLeft < _paddleLeft && _fieldBottom < ballBottom) {
		ballLeft  = _fieldLeft;
		ballRight  = ballLeft + _ballWidth - 1;
		ballBottom  = _fieldBottom;
		ballTop  = (ballBottom - _ballHeight) + 1;
		ballCenterX  = ballLeft + _ballHalfW;
		ballCenterY  = ballTop + _ballHalfH;
		_collisionType = _wallBounceMode ? 0xd : -1;
	}
	// Right edge of paddle
	else if (_paddleRight < ballRight && _fieldBottom < ballBottom) {
		ballRight  = _fieldRight;
		ballLeft  = (ballRight - _ballWidth) + 1;
		ballBottom  = _fieldBottom;
		ballTop  = (ballBottom - _ballHeight) + 1;
		ballCenterX  = ballLeft + _ballHalfW;
		ballCenterY  = ballTop + _ballHalfH;
		_collisionType = _wallBounceMode ? 0xe : -1;
	}
	// Top-left corner
	else if (ballLeft < _fieldLeft && ballTop < _fieldTop) {
		ballLeft  = _fieldLeft;
		ballRight  = ballLeft + _ballWidth - 1;
		ballTop  = _fieldTop;
		ballBottom  = ballTop + _ballHeight - 1;
		ballCenterX  = ballLeft + _ballHalfW;
		ballCenterY  = ballTop + _ballHalfH;
		_collisionType = 0xb;
	}
	// Top-right corner
	else if (_fieldRight < ballRight && ballTop < _fieldTop) {
		ballRight  = _fieldRight;
		ballLeft  = (ballRight - _ballWidth) + 1;
		ballTop  = _fieldTop;
		ballBottom  = ballTop + _ballHeight - 1;
		ballCenterX  = ballLeft + _ballHalfW;
		ballCenterY  = ballTop + _ballHalfH;
		_collisionType = 0xc;
	}
	// Bottom wall (past deathY or field bottom)
	else if (_fieldBottom < ballBottom) {
		ballBottom  = _fieldBottom;
		ballTop  = (ballBottom - _ballHeight) + 1;
		ballCenterY  = ballTop + _ballHalfH;
		_collisionType = _wallBounceMode ? 6 : -1;
	}
	// Paddle left-edge collision
	else if (ballLeft < _paddleLeft && _paddleLeft < ballRight && _paddleTop < ballTop) {
		ballRight  = _paddleLeft;
		ballLeft  = (ballRight - _ballWidth) + 1;
		ballCenterX  = ballLeft + _ballHalfW;
		_collisionType = 1;
	}
	// Paddle right-edge collision
	else if (ballLeft < _paddleRight && _paddleRight < ballRight && _paddleTop < ballTop) {
		ballLeft  = _paddleRight;
		ballRight  = ballLeft + _ballWidth - 1;
		ballCenterX  = ballLeft + _ballHalfW;
		_collisionType = 2;
	}
	// Left wall
	else if (ballLeft < _fieldLeft) {
		ballLeft  = _fieldLeft;
		ballRight  = ballLeft + _ballWidth - 1;
		ballCenterX  = ballLeft + _ballHalfW;
		_collisionType = 3;
	}
	// Right wall
	else if (_fieldRight < ballRight) {
		ballRight  = _fieldRight;
		ballLeft  = (ballRight - _ballWidth) + 1;
		ballCenterX  = ballLeft + _ballHalfW;
		_collisionType = 5;
	}
	// Top wall
	else if (ballTop < _fieldTop) {
		ballTop  = _fieldTop;
		ballBottom  = ballTop + _ballHeight - 1;
		ballCenterY  = ballTop + _ballHalfH;
		_collisionType = 4;
	}
	// Brick area
	else if (_brickAreaLeft <= ballCenterX && ballCenterX <= _brickAreaRight &&
	         _brickAreaTop  <= ballCenterY && ballCenterY <= _brickAreaBottom) {
		brickCollision(ballLeft, ballTop, ballRight, ballBottom, ballCenterX, ballCenterY);
	}

	// Sanity: if ball is still outside field after all corrections → stuck
	if (ballTop < _fieldTop || _fieldBottom < ballBottom || ballLeft < _fieldLeft || _fieldRight < ballRight) {
		_collisionType = 0x10;
	}
}

// ---- segmentsCross (FUN_00450120 / FUN_0045022b) ----------------------------
// Returns true if segment P1->P2 crosses segment P3->P4 (including touching).
// Uses the standard 2D cross-product orientation test.

static int crossSign(int ax, int ay, int bx, int by, int px, int py) {
	return (bx - ax) * (py - ay) - (px - ax) * (by - ay);
}

static bool segmentsCross(int p1x, int p1y, int p2x, int p2y,
                           int p3x, int p3y, int p4x, int p4y) {
	int d1 = crossSign(p3x, p3y, p4x, p4y, p1x, p1y);
	int d2 = crossSign(p3x, p3y, p4x, p4y, p2x, p2y);
	int d3 = crossSign(p1x, p1y, p2x, p2y, p3x, p3y);
	int d4 = crossSign(p1x, p1y, p2x, p2y, p4x, p4y);
	if (d1 == 0 || d2 == 0 || d3 == 0 || d4 == 0)
		return true;
	if ((d1 > 0) == (d2 > 0) || (d3 > 0) == (d4 > 0))
		return false;
	return true;
}

// ---- brickCollision (FUN_0044cad5) ------------------------------------------
// Ball center is inside brick area. Find which brick is hit and set collision type.
// Direct grid-index lookup (O(1)), neighbor-based face detection with line-segment
// intersection test (FUN_00450120).

bool ArcadePuzzle::brickCollision(int &ballLeft, int &ballTop, int &ballRight, int &ballBottom,
                                   int &ballCenterX, int &ballCenterY) {
	// Direct index calculation
	int row = (ballCenterY - _brickAreaTop)  / _brickHeight;
	int col = (ballCenterX - _brickAreaLeft) / _brickWidth;
	int i   = row * _brickCols + col;

	// Out of bounds or dead/pending → ball passes through freely (leave collisionType = -2)
	if (i < 0 || i >= _totalBricks)
		return false;
	Brick &b = _bricks[i];
	if (!b.alive || b.pendingExplosion)
		return false;

	// Previous ball center (this+0x148 in original = previous center point)
	int prevCX = _ballPrevLeft + _ballHalfW;
	int prevCY = _ballPrevTop  + _ballHalfH;

	// Check each exposed face using trajectory line-segment intersection.
	// Priority order matches original: top(8) → bottom(10) → left(7) → right(9).
	if (b.neighborUp == -1 &&
	    segmentsCross(prevCX, prevCY, ballCenterX, ballCenterY,
	                  b.vpRect.left, b.vpRect.top, b.vpRect.right, b.vpRect.top)) {
		ballBottom = b.vpRect.top;
		ballTop = (ballBottom - _ballHeight) + 1;
		ballCenterY = ballTop + _ballHalfH;
		_collisionType = 8;
	} else if (b.neighborDown == -1 &&
	           segmentsCross(prevCX, prevCY, ballCenterX, ballCenterY,
	                         b.vpRect.left, b.vpRect.bottom, b.vpRect.right, b.vpRect.bottom)) {
		ballTop = b.vpRect.bottom;
		ballBottom = ballTop + _ballHeight - 1;
		ballCenterY = ballTop + _ballHalfH;
		_collisionType = 10;
	} else if (b.neighborLeft == -1 &&
	           segmentsCross(prevCX, prevCY, ballCenterX, ballCenterY,
	                         b.vpRect.left, b.vpRect.top, b.vpRect.left, b.vpRect.bottom)) {
		ballRight = b.vpRect.left;
		ballLeft = (ballRight - _ballWidth) + 1;
		ballCenterX = ballLeft + _ballHalfW;
		_collisionType = 7;
	} else if (b.neighborRight == -1 &&
	           segmentsCross(prevCX, prevCY, ballCenterX, ballCenterY,
	                         b.vpRect.right, b.vpRect.top, b.vpRect.right, b.vpRect.bottom)) {
		ballLeft = b.vpRect.right;
		ballRight = ballLeft + _ballWidth - 1;
		ballCenterX = ballLeft + _ballHalfW;
		_collisionType = 9;
	} else {
		// Alive brick but all faces blocked by neighbors (or trajectory didn't cross any face)
		_collisionType = 0xf;
	}

	// Queue brick for removal and accumulate score
	addToExplosionList(i, 125);

	_levelScore[_currentLevel] += (int32)_timeBonusMultiplier;
	if (_cumulativeScore)
		_score = _totalLevelScore + _levelScore[_currentLevel];
	else
		_score = _levelScore[_currentLevel];

	_needsRedraw = true;
	return true;
}

// ---- applyCollision ---------------------------------------------------------
// Changes ball velocity based on current _collisionType.

void ArcadePuzzle::applyCollision() {
	switch (_collisionType) {
	case 0: {
		// Paddle center: look up angle table based on where ball hit paddle
		int hitPos = _ballCenterX - _paddleLeft;
		hitPos = CLIP(hitPos, 0, _paddleWidth - 1);
		uint32 rbit = g_nancy->_randomSource->getRandomBit();
		int idx = hitPos * 6 + rbit * 3;
		_ballDX   = _angleTable[idx + 0];
		_ballDY   = _angleTable[idx + 1];
		_ballSpin = _angleTable[idx + 2];
		g_nancy->_sound->playSound(_sounds[0]); // bounce sound
		break;
	}

	case 1: case 2:
		// Paddle edge: negate both components
		_ballDX = -_ballDX;
		_ballDY = -_ballDY;
		break;

	case 3: case 4: case 5: case 6: {
		// Wall: reflect with negation, then play wall sound
		float dx = -_ballDX, dy = -_ballDY;
		const float *n = _wallNormals[_collisionType];
		float dot = dx * n[0] + dy * n[1];
		_ballDX = 2.0f * dot * n[0] - dx;
		_ballDY = 2.0f * dot * n[1] - dy;
		float len = (float)sqrt(_ballDX * _ballDX + _ballDY * _ballDY);
		if (len > 0.0f) { _ballDX /= len; _ballDY /= len; }
		g_nancy->_sound->playSound(_sounds[0]); // wall bounce sound (same as paddle)
		break;
	}

	case 7: case 8: case 9: case 10: {
		// Brick face: reflect with negation, then play brick hit sound
		float dx = -_ballDX, dy = -_ballDY;
		const float *n = _wallNormals[_collisionType];
		float dot = dx * n[0] + dy * n[1];
		_ballDX = 2.0f * dot * n[0] - dx;
		_ballDY = 2.0f * dot * n[1] - dy;
		float len = (float)sqrt(_ballDX * _ballDX + _ballDY * _ballDY);
		if (len > 0.0f) { _ballDX /= len; _ballDY /= len; }
		playBrickHitSound();
		break;
	}

	case 0xb: case 0xc: case 0xd: case 0xe:
		// Corner: set velocity directly from precomputed normal
		_ballDX = _wallNormals[_collisionType][0];
		_ballDY = _wallNormals[_collisionType][1];
		break;

	case 0xf:
		// Stuck in brick area: negate and play brick hit sound
		_ballDX = -_ballDX;
		_ballDY = -_ballDY;
		playBrickHitSound();
		break;

	case 0x10:
		// Catch-all out-of-bounds: negate
		_ballDX = -_ballDX;
		_ballDY = -_ballDY;
		break;

	case -1:
		// Ball died — handled in ballAndCollision
		break;

	default:
		break;
	}
}

// ---- ballExited -------------------------------------------------------------
// Returns true if the dead ball has completely exited the field.
// Updates _deadBall* rects to the visible clipped portion while still on screen.

bool ArcadePuzzle::ballExited(int ballLeft, int ballTop, int ballRight, int ballBottom, int &, int &) {
	// Check if any part of ball is still within field
	if (ballLeft < _fieldRight && ballRight > _fieldLeft &&
	    ballTop < _fieldBottom && ballBottom > _fieldTop) {
		// Clamp visible portion
		_deadBallTop    = ballTop;
		_deadBallSrcTop = _ballSrc.top;

		if (ballLeft < _fieldLeft) {
			_deadBallLeft    = _fieldLeft;
			_deadBallSrcLeft = _ballSrc.left + (_fieldLeft - ballLeft);
		} else {
			_deadBallLeft    = ballLeft;
			_deadBallSrcLeft = _ballSrc.left;
		}
		if (_fieldBottom < ballBottom) {
			_deadBallBottom    = _fieldBottom;
			_deadBallSrcBottom = _ballSrc.bottom - (ballBottom - _fieldBottom);
		} else {
			_deadBallBottom    = ballBottom;
			_deadBallSrcBottom = _ballSrc.bottom;
		}
		if (_fieldRight < ballRight) {
			_deadBallRight    = _fieldRight;
			_deadBallSrcRight = _ballSrc.right - (ballRight - _fieldRight);
		} else {
			_deadBallRight    = ballRight;
			_deadBallSrcRight = _ballSrc.right;
		}
		return false; // still visible
	}

	// Completely outside
	_deadBallLeft = _deadBallTop = _deadBallRight = _deadBallBottom = 0;
	_deadBallSrcLeft = _deadBallSrcTop = _deadBallSrcRight = _deadBallSrcBottom = 0;
	return true;
}

void ArcadePuzzle::processExplosions() {
	if (_explosionList.empty())
		return;

	uint32 now = g_system->getMillis();
	for (auto it = _explosionList.begin(); it != _explosionList.end(); ++it) {
		int i = *it;
		if (now >= _bricks[i].explosionTimer) {
			// Erase brick from draw surface
			eraseBrick(i);

			// If ball was overlapping this brick, force a ball redraw so it isn't erased
			{
				Common::Rect ballRect(_ballLeft, _ballTop, _ballRight + 1, _ballBottom + 1);
				if (_bricks[i].vpRect.intersects(ballRect))
					_ballNeedsRedraw = true;
			}

			// Mark dead
			_bricks[i].alive            = false;
			_bricks[i].pendingExplosion = false;

			// Unlink from neighbors
			int uIdx = _bricks[i].neighborUp;
			int dIdx = _bricks[i].neighborDown;
			int lIdx = _bricks[i].neighborLeft;
			int rIdx = _bricks[i].neighborRight;

			if (uIdx != -1)
				_bricks[uIdx].neighborDown  = -1;
			if (dIdx != -1)
				_bricks[dIdx].neighborUp    = -1;
			if (lIdx != -1)
				_bricks[lIdx].neighborRight = -1;
			if (rIdx != -1)
				_bricks[rIdx].neighborLeft  = -1;

			_needsRedraw = true;
			it = _explosionList.erase(it);
		}
	}

	// If explosion list is now empty, check whether all bricks are gone
	if (_explosionList.empty()) {
		bool allGone = true;
		for (int i = 0; i < _totalBricks; ++i) {
			if (_bricks[i].alive) {
				allGone = false;
				break;
			}
		}
		if (allGone)
			_levelClear = true;
	}
}

void ArcadePuzzle::addToExplosionList(int brickIdx, uint32 delay) {
	// Don't add the same brick twice
	for (int i : _explosionList)
		if (i == brickIdx)
			return;

	_explosionList.push_back(brickIdx);

	_bricks[brickIdx].explosionTimer    = g_system->getMillis() + delay;
	_bricks[brickIdx].pendingExplosion = true;
}

void ArcadePuzzle::playBrickHitSound() {
	// Rotates through sounds[2], sounds[3], sounds[4] (brick hit A/B/C)
	int slot = _brickSoundRotator + 2; // maps 0→2, 1→3, 2→4
	g_nancy->_sound->playSound(_sounds[slot]);
	_brickSoundRotator = (_brickSoundRotator + 1) % 3;
}

void ArcadePuzzle::updateTimer() {
	uint32 now = g_system->getMillis();
	_timerElapsedMs = now - _timerStartMs;

	uint32 totalSecs = _timerElapsedMs / 1000;
	uint32 mins = totalSecs / 60;
	uint32 secs = totalSecs % 60;
	if (mins > 59) mins = 59;

	if (mins != _timerMins || secs != _timerSecs) {
		_timerMins = mins;
		_timerSecs = secs;
		drawTimer();
	}
}

void ArcadePuzzle::updateScore() {
	if (_score == _prevScore)
		return;

	// Cap at 12,000,000
	if (_score > 12000000)
		_score = 12000000;

	// Update time bonus multiplier
	uint32 elapsedSecs = _timerElapsedMs / 1000;
	if ((int32)elapsedSecs < _timeLimitSec) {
		_timeBonusMultiplier =
		    ((float)(_timeLimitSec - (int32)elapsedSecs)) *
		    ((float)_timeBonusMax / (float)_timeLimitSec);
	} else {
		_timeBonusMultiplier = 1.0f;
	}

	drawScore();
	_prevScore = _score;
}

// ---- Drawing helpers --------------------------------------------------------

void ArcadePuzzle::drawBrick(int idx) {
	const Brick &b = _bricks[idx];
	if (!b.alive || b.type < 0) return;
	_drawSurface.blitFrom(_image, b.srcRect, Common::Point(b.vpRect.left, b.vpRect.top));
	_needsRedraw = true;
}

void ArcadePuzzle::eraseBrick(int idx) {
	_drawSurface.fillRect(_bricks[idx].vpRect, _drawSurface.getTransparentColor());
	_needsRedraw = true;
}

void ArcadePuzzle::drawPaddle() {
	_drawSurface.blitFrom(_image, _paddleSrcCur,
	                       Common::Point(_paddleLeft, _paddleTop));
	_needsRedraw = true;
}

void ArcadePuzzle::erasePaddle() {
	Common::Rect r(_paddlePrevLeft, _paddlePrevTop,
	               _paddlePrevRight + 1, _paddlePrevBottom + 1);
	_drawSurface.fillRect(r, _drawSurface.getTransparentColor());
	_needsRedraw = true;
}

void ArcadePuzzle::drawBall() {
	if (_ballState == kDying) {
		// Draw dead ball clipped rect
		if (_deadBallRight > _deadBallLeft && _deadBallBottom > _deadBallTop) {
			Common::Rect src(_deadBallSrcLeft, _deadBallSrcTop,
			                 _deadBallSrcRight, _deadBallSrcBottom);
			_drawSurface.blitFrom(_image, src,
			                      Common::Point(_deadBallLeft, _deadBallTop));
			_needsRedraw = true;
		}
	} else {
		_drawSurface.blitFrom(_image, _ballSrc,
		                       Common::Point(_ballLeft, _ballTop));
		_needsRedraw = true;
	}
}

void ArcadePuzzle::eraseBall() {
	Common::Rect r(_ballPrevLeft, _ballPrevTop,
	               _ballPrevRight + 1, _ballPrevBottom + 1);
	if (!r.isEmpty())
		_drawSurface.fillRect(r, _drawSurface.getTransparentColor());
	_needsRedraw = true;
}

void ArcadePuzzle::drawDigit(const Common::Rect &destRect, int digit,
                              bool useTimerRects) {
	if (digit < 0 || digit > 9)
		return;
	const Common::Rect &src = useTimerRects ? _timerDigitSrc[digit]
	                                        : _scoreDigitSrc[digit];
	_drawSurface.blitFrom(_image, src, Common::Point(destRect.left, destRect.top));
	_needsRedraw = true;
}

void ArcadePuzzle::drawScore() {
	if (_scoreDigitSrc[0].isEmpty())
		return;

	int digitW = _scoreDigitSrc[0].width();
	int digitH = _scoreDigitSrc[0].height();
	int baseX  = _scoreDisplayX + _fieldLeft;
	int baseY  = _scoreDisplayY + _fieldTop;

	// Collect digits least-significant first
	int digits[7];
	int nDigits = 0;
	int val = ABS(_score);
	do {
		digits[nDigits++] = val % 10;
		val /= 10;
	} while (val != 0 && nDigits < 7);

	// Erase old digits
	Common::Rect eraseR(baseX, baseY, baseX + 7 * digitW, baseY + digitH);
	_drawSurface.fillRect(eraseR, _drawSurface.getTransparentColor());

	// Draw digits left-to-right (most-significant first)
	for (int i = nDigits - 1; i >= 0; --i) {
		int col = (nDigits - 1 - i);
		Common::Rect dest(baseX + col * digitW, baseY,
		                  baseX + col * digitW + digitW, baseY + digitH);
		drawDigit(dest, digits[i], false);
	}
	_needsRedraw = true;
}

void ArcadePuzzle::drawTimer() {
	if (_timerDigitSrc[0].isEmpty())
		return;

	int digitW = _timerDigitSrc[0].width();
	int digitH = _timerDigitSrc[0].height();
	int baseX  = _timerDisplayX + _fieldLeft;
	int baseY  = _timerDisplayY + _fieldTop;

	// Erase old digits
	Common::Rect eraseR(baseX, baseY, baseX + 4 * digitW, baseY + digitH);
	_drawSurface.fillRect(eraseR, _drawSurface.getTransparentColor());

	// Draw MM:SS left-to-right: [mins_tens][mins_ones][secs_tens][secs_ones]
	uint32 mins = _timerMins;
	uint32 secs = _timerSecs;

	Common::Rect d3(baseX,                baseY, baseX +     digitW, baseY + digitH);
	drawDigit(d3, mins / 10, true);
	Common::Rect d2(baseX +     digitW,   baseY, baseX + 2 * digitW, baseY + digitH);
	drawDigit(d2, mins % 10, true);
	Common::Rect d1(baseX + 2 * digitW,   baseY, baseX + 3 * digitW, baseY + digitH);
	drawDigit(d1, secs / 10, true);
	Common::Rect d0(baseX + 3 * digitW,   baseY, baseX + 4 * digitW, baseY + digitH);
	drawDigit(d0, secs % 10, true);
	_needsRedraw = true;
}

} // End of namespace Action
} // End of namespace Nancy
