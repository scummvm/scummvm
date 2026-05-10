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

#include "common/random.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/whalesurvivorpuzzle.h"

namespace Nancy {
namespace Action {

void WhaleSurvivorPuzzle::readData(Common::SeekableReadStream &stream) {
	// Image names (4 × 33 bytes each)
	readFilename(stream, _imageNameMain);      // 0x000
	readFilename(stream, _imageNameStart);     // 0x021
	readFilename(stream, _imageNameTryAgain);  // 0x042
	readFilename(stream, _imageNameWin);       // 0x063

	// Porpoise sprite source rects in the main overlay (3 × 16 bytes)
	readRect(stream, _porpoiseSwimmingRect);   // 0x084 - swim sprite
	readRect(stream, _porpoiseBellyUpRect);    // 0x094 - belly up surfacing sprite
	readRect(stream, _livesSrcRect);           // 0x0a4 - small life-indicator sprite

	// Fish entity types: main src rects (6 × 16 bytes)
	for (int i = 0; i < kNumEntityTypes; ++i)
		readRect(stream, _fishTypes[i].mainSrc); // 0x0b4

	// Fish entity types: sub-sprite data (6 × 4 sub-sprites × 20 bytes each)
	for (int i = 0; i < kNumEntityTypes; ++i) {
		for (int j = 0; j < kNumSubSprites; ++j) {
			_fishTypes[i].subSprites[j].active = stream.readUint32LE() != 0;
			readRect(stream, _fishTypes[i].subSprites[j].src);
		}
	} // ends at 0x2f4

	// Hazard entity types: main src rects (6 × 16 bytes)
	for (int i = 0; i < kNumEntityTypes; ++i)
		readRect(stream, _hazardTypes[i].mainSrc); // 0x2f4

	// Hazard entity types: sub-sprite data (6 × 4 sub-sprites × 20 bytes each)
	for (int i = 0; i < kNumEntityTypes; ++i) {
		for (int j = 0; j < kNumSubSprites; ++j) {
			_hazardTypes[i].subSprites[j].active = stream.readUint32LE() != 0;
			readRect(stream, _hazardTypes[i].subSprites[j].src);
		}
	} // ends at 0x534

	// Start button source rects - unpressed/pressed state (2 × 16 bytes)
	for (int i = 0; i < 2; ++i)
		readRect(stream, _startButtonSrcRects[i]); // 0x534

	// Quit button source rects - unpressed/pressed state (2 × 16 bytes)
	for (int i = 0; i < 2; ++i)
		readRect(stream, _quitButtonSrcRects[i]); // 0x554

	// Score digit source rects in main overlay (10 × 16 bytes, digits 0-9)
	for (int i = 0; i < kNumScoreDigits; ++i)
		readRect(stream, _scoreDigitSrcRects[i]); // 0x574

	// Ocean floor background source rects (7 × 16 bytes)
	for (int i = 0; i < kNumFloorSprites; ++i)
		readRect(stream, _floorSrcRects[i]); // 0x614

	// Oxygen bar stage source rects (8 × 16 bytes)
	for (int i = 0; i < kOxygenStages; ++i)
		readRect(stream, _oxygenSrcRects[i]); // 0x684

	// Bubble/breath animation source rects (8 × 16 bytes, one per breathFrame)
	for (int i = 0; i < kOxygenStages; ++i)
		readRect(stream, _bubbleSrcRects[i]); // 0x704..0x783

	// Positional parameters (all viewport-relative, int32)
	_surfaceY  = stream.readSint32LE(); // 0x784
	_maxY      = stream.readSint32LE(); // 0x788
	_spawnYMin = stream.readSint32LE(); // 0x78c
	_spawnYMax = stream.readSint32LE(); // 0x790
	stream.skip(4);                     // 0x794 (unknown)
	stream.skip(4);                     // 0x798 (unknown)
	_scoreX    = stream.readSint32LE(); // 0x79c
	_scoreY    = stream.readSint32LE(); // 0x7a0
	stream.skip(8);                     // 0x7a4..0x7ab (unused)
	_oxygenBarX = stream.readSint32LE(); // 0x7ac
	_oxygenBarY = stream.readSint32LE(); // 0x7b0
	stream.skip(8);                     // 0x7b4..0x7bb (oxygen bar right/bottom, unused)

	// Lives indicator destination rects (3 × 16 bytes)
	for (int i = 0; i < kMaxLives; ++i)
		readRect(stream, _livesDestRects[i]); // 0x7bc

	// Button screen positions (2 × 16 bytes):
	readRect(stream, _startButtonDestRect); // 0x7ec
	readRect(stream, _quitButtonDestRect); // 0x7fc

	// Playfield bounds (viewport-relative, 16 bytes)
	readRect(stream, _playfieldRect); // 0x80c

	// Outer viewport rect – the region covered by overlay (start/try-again/win) images
	readRect(stream, _overlayRect); // 0x81c

	// Ocean floor background dest rects (7 × 16 bytes)
	for (int i = 0; i < kNumFloorSprites; ++i)
		readRect(stream, _floorDestRects[i]); // 0x82c

	// Key codes (4 × int32, skipped – we use NancyInput flags instead)
	stream.skip(4 * 4); // 0x89c..0x8ab

	// Timing parameters
	_bubbleIntervalMs = stream.readUint32LE(); // 0x8ac
	_tryAgainDelayMs  = stream.readUint32LE(); // 0x8b0

	// Speed: stored as numerator / denominator pairs -> px/ms
	uint32 denX      = stream.readUint32LE(); // 0x8b4
	uint32 denY      = stream.readUint32LE(); // 0x8b8
	uint32 denBreath  = stream.readUint32LE(); // 0x8bc
	float  numX      = stream.readFloatLE();  // 0x8c0
	float  numY      = stream.readFloatLE();  // 0x8c4
	float  numBreath  = stream.readFloatLE();  // 0x8c8
	_speedX      = (denX     > 0) ? numX     / (float)denX     : 0.0f;
	_speedY      = (denY     > 0) ? numY     / (float)denY     : 0.0f;
	_breathSpeed = (denBreath > 0) ? numBreath / (float)denBreath : 0.0f;

	// Fish movement parameters
	_fishAnimFPS    = stream.readUint32LE(); // 0x8cc
	_fishSpeedRange = stream.readFloatLE();  // 0x8d0
	_fishSpeedBase  = stream.readFloatLE();  // 0x8d4

	// Hazard movement parameters
	_hazardAnimFPS    = stream.readUint32LE(); // 0x8d8
	_hazardSpeedRange = stream.readFloatLE();  // 0x8dc
	_hazardSpeedBase  = stream.readFloatLE();  // 0x8e0

	stream.skip(4);                           // 0x8e4 (unknown timer)
	_oxygenTickMs = stream.readUint32LE();    // 0x8e8
	stream.skip(4);                           // 0x8ec (unknown)

	_initialFish    = stream.readUint32LE();  // 0x8f0
	_initialHazards = stream.readUint32LE();  // 0x8f4
	stream.skip(4);                           // 0x8f8 (unknown)

	_isHardMode    = stream.readByte() != 0;  // 0x8fc
	_randomSpawn   = stream.readByte() != 0;  // 0x8fd
	_scoreDivisor  = stream.readUint32LE();   // 0x8fe

	stream.skip(6 * 4);                       // 0x902..0x919 (score thresholds)

	_oxygenDeplSpeed = stream.readFloatLE();  // 0x91a
	_maxScore        = stream.readUint32LE(); // 0x91e
	_breathFreq      = stream.readUint32LE(); // 0x922

	// Sounds
	_eatSound.readNormal(stream);     // 0x926
	_hurtSound.readNormal(stream);    // 0x957
	_breatheSound.readNormal(stream); // 0x988
	_sound4.readNormal(stream);       // 0x9b9 (unused/silence)
	_sound5.readNormal(stream);       // 0x9ea (unused/silence)

	// Scenes and remaining sounds
	_winScene.readData(stream);       // 0xa1b  (25 bytes)
	stream.skip(1);                // 0xa34  padding byte
	_winSound.readNormal(stream);     // 0xa35
	_lossScene.readData(stream);      // 0xa66  (25 bytes)
	stream.skip(1);                // 0xa7f  padding byte
	_gameOverSound.readNormal(stream); // 0xa80
	_tryAgainSound.readNormal(stream); // 0xab1
}

void WhaleSurvivorPuzzle::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(vpBounds.width(), vpBounds.height(),
	                    g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(vpBounds);

	g_nancy->_resource->loadImage(_imageNameMain,     _imageMain);
	g_nancy->_resource->loadImage(_imageNameStart,    _imageStart);
	g_nancy->_resource->loadImage(_imageNameTryAgain, _imageTryAgain);
	g_nancy->_resource->loadImage(_imageNameWin,      _imageWin);

	_imageMain.setTransparentColor(g_nancy->_graphics->getTransColor());

	// Compute porpoise bounding box from sprite rect
	_porpWidth  = _porpoiseSwimmingRect.width();
	_porpHeight = _porpoiseSwimmingRect.height();

	// Load sounds that need to be ready during gameplay
	if (_eatSound.name != "NO SOUND")
		g_nancy->_sound->loadSound(_eatSound);
	if (_hurtSound.name != "NO SOUND")
		g_nancy->_sound->loadSound(_hurtSound);
	if (_breatheSound.name != "NO SOUND")
		g_nancy->_sound->loadSound(_breatheSound);

	if (_breathFreq < 1)
		_breathFreq = 1;

	_lastMs    = g_nancy->getTotalPlayTime();
	_gameState = kStartScreen;
	_score     = 0;
	_lives     = kMaxLives;
	_prevScore = -1;

	initRound();
	redraw();
}

/**
 * initRound – reset per-round state and scatter initial entities in playfield
 */
void WhaleSurvivorPuzzle::initRound() {
	// Clear all entities
	for (int i = 0; i < kMaxEntities; ++i)
		_entities[i].active = false;

	_fishCount   = 0;
	_hazardCount = 0;

	_oxygenDepleting  = false;
	_oxygenStage      = 0;
	_oxygenNextTickMs = 0;

	_porpoiseAnim      = kPorpoiseSwim;
	_breathFrame       = 0;
	_nextBubbleMs      = 0;
	_firstHitFrame     = false;
	_inputFlags        = 0;
	_startButtonHeld   = false;
	_quitButtonHeld    = false;

	Common::RandomSource &rnd = *g_nancy->_randomSource;

	if (!_isHardMode) {
		_porpX = (float)(_playfieldRect.left + (_playfieldRect.width() - _porpWidth) / 2);
	} else {
		// Hard mode: random X start
		int range = MAX(1, _playfieldRect.width() - _porpWidth);
		_porpX = (float)(_playfieldRect.left + (int)rnd.getRandomNumber(range - 1));
	}
	_porpY    = (float)_surfaceY;
	_porpLeft = (int)_porpX;
	_porpTop  = (int)_porpY;

	// Scatter initial fish randomly within the playfield
	int spawnRange = (_spawnYMax - _spawnYMin) + 1;
	for (uint32 f = 0; f < _initialFish; ++f) {
		int slot = findFreeEntity();
		if (slot < 0) break;

		int typeIdx = (int)rnd.getRandomNumber(kNumEntityTypes - 1);
		EntityType &fish = _fishTypes[typeIdx];

		Entity &e  = _entities[slot];
		e.active   = true;
		e.kind     = kEntityFish;
		e.typeIdx  = typeIdx;
		e.width    = fish.mainSrc.width();
		e.height   = fish.mainSrc.height();
		e.animFPS  = _fishAnimFPS;

		int xRange = MAX(1, _playfieldRect.width() - e.width);
		e.x = _playfieldRect.left + (int)rnd.getRandomNumber(xRange - 1);

		int yRange = MAX(1, spawnRange - e.height);
		e.y = _spawnYMin + (int)rnd.getRandomNumber(yRange - 1);

		e.floatX = (float)e.x;

		int baseRange = MAX(1, (int)_fishSpeedBase);
		float speed = _fishSpeedRange + (float)rnd.getRandomNumber(baseRange - 1)
		              + (float)rnd.getRandomNumber(9) / 10.0f;
		e.speedX = (_fishAnimFPS > 0) ? speed / (float)_fishAnimFPS : 0.1f;
		e.removed = false;

		++_fishCount;
	}

	// Scatter initial hazards
	for (uint32 hz = 0; hz < _initialHazards; ++hz) {
		int slot = findFreeEntity();
		if (slot < 0)
			break;

		int typeIdx = (int)rnd.getRandomNumber(kNumEntityTypes - 1);
		EntityType &hazard = _hazardTypes[typeIdx];

		Entity &e  = _entities[slot];
		e.active   = true;
		e.kind     = kEntityHazard;
		e.typeIdx  = typeIdx;
		e.width    = hazard.mainSrc.width();
		e.height   = hazard.mainSrc.height();
		e.animFPS  = _hazardAnimFPS;

		int xRange = MAX(1, _playfieldRect.width() - e.width);
		e.x = _playfieldRect.left + (int)rnd.getRandomNumber(xRange - 1);

		int yRange = MAX(1, spawnRange - e.height);
		e.y = _spawnYMin + (int)rnd.getRandomNumber(yRange - 1);

		e.floatX = (float)e.x;

		int baseRange = MAX(1, (int)_hazardSpeedBase);
		float speed = _hazardSpeedRange + (float)rnd.getRandomNumber(baseRange - 1)
		              + (float)rnd.getRandomNumber(9) / 10.0f;
		e.speedX = (_hazardAnimFPS > 0) ? speed / (float)_hazardAnimFPS : 0.05f;
		e.removed = false;

		++_hazardCount;
	}
}

/**
 * spawnEntity – spawn a new fish or hazard at the right edge
 */
void WhaleSurvivorPuzzle::spawnEntity(EntityKind kind) {
	int slot = findFreeEntity();
	if (slot < 0)
		return;

	Common::RandomSource &rnd = *g_nancy->_randomSource;
	int spawnRange = (_spawnYMax - _spawnYMin) + 1;

	Entity &e = _entities[slot];
	e.active  = true;
	e.kind    = kind;
	e.removed = false;
	e.typeIdx = (int)rnd.getRandomNumber(kNumEntityTypes - 1);

	if (kind == kEntityFish) {
		EntityType &fish = _fishTypes[e.typeIdx];
		e.width   = fish.mainSrc.width();
		e.height  = fish.mainSrc.height();
		e.animFPS = _fishAnimFPS;
		int baseRange = MAX(1, (int)_fishSpeedBase);
		float speed = _fishSpeedRange + (float)rnd.getRandomNumber(baseRange - 1)
		              + (float)rnd.getRandomNumber(9) / 10.0f;
		e.speedX = (_fishAnimFPS > 0) ? speed / (float)_fishAnimFPS : 0.1f;
	} else {
		EntityType &hazard = _hazardTypes[e.typeIdx];
		e.width   = hazard.mainSrc.width();
		e.height  = hazard.mainSrc.height();
		e.animFPS = _hazardAnimFPS;
		int baseRange = MAX(1, (int)_hazardSpeedBase);
		float speed = _hazardSpeedRange + (float)rnd.getRandomNumber(baseRange - 1)
		              + (float)rnd.getRandomNumber(9) / 10.0f;
		e.speedX = (_hazardAnimFPS > 0) ? speed / (float)_hazardAnimFPS : 0.05f;
	}

	// Spawn just off the right edge so the entity slides in gradually
	e.x = _playfieldRect.right;

	int yRange = MAX(1, spawnRange - e.height);
	e.y = _spawnYMin + (int)rnd.getRandomNumber(yRange - 1);

	e.floatX = (float)e.x;
}

void WhaleSurvivorPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through

	case kRun: {
		uint32 nowMs = g_nancy->getTotalPlayTime();

		switch (_gameState) {
		case kStartScreen:
			// Waiting for the player to click – keep start screen visible.
			redraw();
			break;

		case kPlaying:
			updateGame(nowMs);
			redraw();
			break;

		case kRoundWon:
			// Play win sound once, then start the countdown
			if (_winSound.name != "NO SOUND") {
				g_nancy->_sound->loadSound(_winSound);
				g_nancy->_sound->playSound(_winSound);
			}
			_countdownEndMs = nowMs + _tryAgainDelayMs;
			_executeWin     = true;
			_gameState      = kCountdown;
			redraw();
			break;

		case kHitAnimation:
			// Waiting for player to click – handled in handleInput.
			// If no lives remain, proceed directly to game over.
			if (_lives <= 0) {
				_gameState = kWinScreen;
			}
			redraw();
			break;

		case kTryAgain:
			// Reset entities; entities and score are preserved between lives.
			initRound();
			_gameState = kPlaying;
			_lastMs    = nowMs;
			redraw();
			break;

		case kWinScreen:
			// Fire the scene change on the very next tick
			_gameState      = kCountdown;
			_countdownEndMs = nowMs;
			redraw();
			break;

		case kCountdown:
			redraw();
			if (nowMs >= _countdownEndMs)
				_state = kActionTrigger;
			break;
		}
		break;
	}

	case kActionTrigger:
		// Stop all sounds
		g_nancy->_sound->stopSound(_eatSound);
		g_nancy->_sound->stopSound(_hurtSound);
		g_nancy->_sound->stopSound(_breatheSound);
		g_nancy->_sound->stopSound(_winSound);
		g_nancy->_sound->stopSound(_tryAgainSound);
		g_nancy->_sound->stopSound(_gameOverSound);

		if (_executeWin)
			_winScene.execute();
		else
			_lossScene.execute();

		finishExecution();
		break;
	}
}

/**
 * handleInput – mouse click starts the game; arrow keys move the porpoise
 */
void WhaleSurvivorPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun)
		return;

	// Cache movement flags for use in updateGame
	_inputFlags = input.input;

	// Convert screen-space mouse position to viewport-relative coordinates
	Common::Point localMousePos = input.mousePos;
	Common::Rect vpPos = NancySceneState.getViewport().getScreenPosition();
	localMousePos -= Common::Point(vpPos.left, vpPos.top);

	bool overStart = (_gameState == kStartScreen || _gameState == kHitAnimation) &&
	                 _startButtonDestRect.contains(localMousePos);
	bool overQuit  = (_gameState == kStartScreen || _gameState == kPlaying || _gameState == kHitAnimation) &&
	                 _quitButtonDestRect.contains(localMousePos);

	// Highlight cursor over clickable buttons
	if (overStart || overQuit)
		g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

	// Track held state for pressed-sprite rendering
	bool mouseHeld = (input.input & (NancyInput::kLeftMouseButtonDown | NancyInput::kLeftMouseButtonHeld)) != 0;
	bool wasStartHeld = _startButtonHeld;
	bool wasQuitHeld  = _quitButtonHeld;
	_startButtonHeld = mouseHeld && overStart;
	_quitButtonHeld  = mouseHeld && overQuit;
	if (_startButtonHeld != wasStartHeld || _quitButtonHeld != wasQuitHeld)
		_needsRedraw = true;

	if (!(input.input & NancyInput::kLeftMouseButtonUp))
		return;

	// Quit button: active on start screen, try-again screen, and during gameplay.
	// Clicking it immediately exits to the loss scene.
	if (_gameState == kStartScreen || _gameState == kPlaying || _gameState == kHitAnimation) {
		if (_quitButtonDestRect.contains(localMousePos)) {
			if (_gameOverSound.name != "NO SOUND") {
				g_nancy->_sound->loadSound(_gameOverSound);
				g_nancy->_sound->playSound(_gameOverSound);
			}
			_executeWin     = false;
			_gameState      = kCountdown;
			_countdownEndMs = g_nancy->getTotalPlayTime();
			_needsRedraw    = true;
			return;
		}
	}

	// Start button: starts the game from the start screen or restarts after a hit.
	if (_gameState == kStartScreen && _startButtonDestRect.contains(localMousePos)) {
		_gameState   = kPlaying;
		_lastMs      = g_nancy->getTotalPlayTime();
		_needsRedraw = true;
	} else if (_gameState == kHitAnimation && _startButtonDestRect.contains(localMousePos)) {
		// Player chose to try again (full entity reset, keeps score & lives)
		_gameState   = kTryAgain;
		_needsRedraw = true;
	}
}

/**
 * updateGame – per-frame physics, spawning, collision, oxygen
 */
void WhaleSurvivorPuzzle::updateGame(uint32 nowMs) {
	uint32 deltaMs = nowMs - _lastMs;
	_lastMs = nowMs;
	if (deltaMs == 0)
		return;

	// ---- Porpoise movement ----
	bool moved = false;

	if (_porpoiseAnim == kPorpoiseSwim) {
		// Horizontal movement
		if (_inputFlags & NancyInput::kMoveRight) {
			_porpX += (float)deltaMs * _speedX;
			int maxLeft = _playfieldRect.right - _porpWidth;
			if ((int)_porpX > maxLeft)
				_porpX = (float)maxLeft;
			moved = true;
		} else if (_inputFlags & NancyInput::kMoveLeft) {
			_porpX -= (float)deltaMs * _speedX;
			if ((int)_porpX < _playfieldRect.left)
				_porpX = (float)_playfieldRect.left;
			moved = true;
		}

		// Vertical movement
		if (_inputFlags & NancyInput::kMoveUp) {
			// Manual surface – move up faster
			_porpY -= (float)deltaMs * _speedY;
			if ((int)_porpY <= _surfaceY) {
				_porpY = (float)_surfaceY;
				_oxygenDepleting  = false;
				_oxygenStage      = 0;
				_oxygenNextTickMs = 0;
			}
			moved = true;
		} else if (_inputFlags & NancyInput::kMoveDown) {
			// Always sink, regardless of current Y
			_porpY += (float)deltaMs * _speedY;
			int maxTop = _maxY - _porpHeight + 1;
			if ((int)_porpY > maxTop)
				_porpY = (float)maxTop;
			// Start oxygen depletion on first move below surface
			if (!_oxygenDepleting && (int)_porpY > _surfaceY) {
				_oxygenDepleting  = true;
				_oxygenStage      = 0;
				_oxygenNextTickMs = nowMs + _oxygenTickMs;
			}
			moved = true;
		} else {
			// Natural upward drift
			_porpY -= (float)deltaMs * _breathSpeed;
			if ((int)_porpY <= _surfaceY) {
				// Just surfaced – trigger breath if we were diving
				bool wasBelow = _oxygenDepleting;
				_porpY            = (float)_surfaceY;
				_oxygenDepleting  = false;
				_oxygenStage      = 0;
				_oxygenNextTickMs = 0;
				if (wasBelow) {
					Common::RandomSource &rnd = *g_nancy->_randomSource;
					bool doBreath = (_breathFreq <= 1) ||
					                (rnd.getRandomNumber(_breathFreq - 1) == _breathFreq - 1);
					if (doBreath) {
						_porpoiseAnim = kPorpoiseSurface;
						_breathFrame  = 0;
						_nextBubbleMs = nowMs + _bubbleIntervalMs;
						if (_breatheSound.name != "NO SOUND")
							g_nancy->_sound->playSound(_breatheSound);
					}
				}
			}
			moved = true;
		}

	} else if (_porpoiseAnim == kPorpoiseSurface) {
		// Breath animation – advance frames at bubble interval
		if (nowMs >= _nextBubbleMs) {
			++_breathFrame;
			if (_breathFrame > 7) {
				_breathFrame  = 0;
				_porpoiseAnim = kPorpoiseSwim;
				_oxygenDepleting  = false;
				_oxygenStage      = 0;
			}
			_nextBubbleMs = nowMs + _bubbleIntervalMs;
			moved = true;
		}

	} else if (_porpoiseAnim == kPorpoiseEat) {
		// Momentary eat state – immediately return to swimming
		_porpoiseAnim = kPorpoiseSwim;
		moved = true;

	} else if (_porpoiseAnim == kPorpoiseHit) {
		// Float upward after hit until reaching surface, then lose a life
		if (_firstHitFrame) {
			// Push porpoise down a bit on first hit frame (matches original)
			if ((int)_porpY < _surfaceY + 10)
				_porpY = (float)(_surfaceY + 10);
			_firstHitFrame = false;
		}
		_porpY -= (float)deltaMs * _breathSpeed;
		if ((int)_porpY <= _surfaceY) {
			_porpY        = (float)_surfaceY;
			_porpoiseAnim = kPorpoiseSwim;
			loseLife(nowMs);
		}
		moved = true;
	}

	if (moved) {
		int newLeft = (int)_porpX;
		int newTop  = (int)_porpY;
		if (newLeft != _porpLeft || newTop != _porpTop) {
			_porpLeft    = newLeft;
			_porpTop     = newTop;
			_needsRedraw = true;
		}
	}

	// Entity movement
	for (int i = 0; i < kMaxEntities; ++i) {
		Entity &e = _entities[i];
		if (!e.active || e.removed)
			continue;

		e.floatX -= (float)deltaMs * e.speedX;
		int newX = (int)e.floatX;
		if (newX != e.x) {
			e.x          = newX;
			_needsRedraw = true;
		}

		// Mark as removed when fully past the left edge of the playfield
		if (e.x + e.width <= _playfieldRect.left) {
			e.removed = true;
			if (e.kind == kEntityFish)
				--_fishCount;
			else
				--_hazardCount;
		}
	}

	// Collision (only when swimming normally)
	if (_porpoiseAnim == kPorpoiseSwim)
		checkCollisions();

	updateOxygen(nowMs);

	// Respawn entities to maintain target counts
	while (_fishCount < (int)_initialFish) {
		spawnEntity(kEntityFish);
		++_fishCount;
	}
	while (_hazardCount < (int)_initialHazards) {
		spawnEntity(kEntityHazard);
		++_hazardCount;
	}

	// Win / loss transitions
	if (_score >= (int)_maxScore && _gameState == kPlaying) {
		_gameState   = kRoundWon;
		_needsRedraw = true;
	}
}

/**
 * checkCollisions – AABB test of each entity's sub-sprite rects vs porpoise
 */
void WhaleSurvivorPuzzle::checkCollisions() {
	int pL = _porpLeft;
	int pT = _porpTop;
	int pR = pL + _porpWidth  - 1;
	int pB = pT + _porpHeight - 1;

	for (int i = 0; i < kMaxEntities; ++i) {
		Entity &e = _entities[i];
		if (!e.active || e.removed)
			continue;

		const EntityType &et = (e.kind == kEntityFish) ? _fishTypes[e.typeIdx] : _hazardTypes[e.typeIdx];
		bool hit = false;

		for (int j = 0; j < kNumSubSprites && !hit; ++j) {
			const SubSprite &ss = et.subSprites[j];
			if (!ss.active)
				continue;

			// Compute screen rect of this sub-sprite
			int offX = ss.src.left - et.mainSrc.left;
			int offY = ss.src.top  - et.mainSrc.top;
			int sL   = e.x + offX;
			int sT   = e.y + offY;
			int sR   = sL + ss.src.width();
			int sB   = sT + ss.src.height();

			// Standard AABB overlap
			if (sL <= pR && sR >= pL && sT <= pB && sB >= pT)
				hit = true;
		}

		if (!hit)
			continue;

		e.removed = true;
		_needsRedraw = true;

		if (e.kind == kEntityFish) {
			// Fish – eaten!
			--_fishCount;
			_score += _scoreDivisor > 0 ? (int)_scoreDivisor : 500;
			if (_prevScore != _score)
				_needsRedraw = true;

			_porpoiseAnim = kPorpoiseEat;

			if (_eatSound.name != "NO SOUND")
				g_nancy->_sound->playSound(_eatSound);
		} else {
			// Hazard – hit!
			--_hazardCount;
			_porpoiseAnim  = kPorpoiseHit;
			_firstHitFrame = true;

			if (_hurtSound.name != "NO SOUND")
				g_nancy->_sound->playSound(_hurtSound);
		}
	}
}

/**
 * updateOxygen – deplete oxygen stage when submerged
 */
void WhaleSurvivorPuzzle::updateOxygen(uint32 nowMs) {
	if (!_oxygenDepleting || _porpoiseAnim != kPorpoiseSwim)
		return;
	if (_oxygenNextTickMs == 0 || nowMs < _oxygenNextTickMs)
		return;

	++_oxygenStage;
	_oxygenNextTickMs = nowMs + _oxygenTickMs;
	_needsRedraw = true;

	if (_oxygenStage > kOxygenStages) {
		// Drowned – treat same as hazard hit
		_oxygenStage   = kOxygenStages;
		_oxygenDepleting = false;
		_porpoiseAnim  = kPorpoiseHit;
		_firstHitFrame = true;

		if (_hurtSound.name != "NO SOUND")
			g_nancy->_sound->playSound(_hurtSound);
	}
}

void WhaleSurvivorPuzzle::loseLife(uint32 nowMs) {
	--_lives;
	_needsRedraw = true;

	if (_lives <= 0) {
		_lives = 0;
		// No lives left -> go to loss scene after countdown
		_gameState      = kWinScreen;
		_executeWin     = false;
		_countdownEndMs = nowMs;
	} else {
		// Still have lives -> show "try again" screen
		_gameState = kHitAnimation;
		if (_tryAgainSound.name != "NO SOUND") {
			g_nancy->_sound->loadSound(_tryAgainSound);
			g_nancy->_sound->playSound(_tryAgainSound);
		}
	}
}

void WhaleSurvivorPuzzle::redraw() {
	_needsRedraw = true;
	_drawSurface.clear(g_nancy->_graphics->getTransColor());

	// ---- State-specific overlay ----
	switch (_gameState) {
	case kStartScreen:
		// Show the start screen overlay with the clickable Start button sprite on top
		_drawSurface.blitFrom(_imageStart, Common::Point(_overlayRect.left, _overlayRect.top));
		_drawSurface.blitFrom(_imageMain, _startButtonHeld ? _startButtonSrcRects[1] : _startButtonSrcRects[0],
		                      Common::Point(_startButtonDestRect.left, _startButtonDestRect.top));
		return;

	case kHitAnimation:
		// Show "try again" overlay with the clickable Start button sprite on top
		_drawSurface.blitFrom(_imageTryAgain, Common::Point(_overlayRect.left, _overlayRect.top));
		_drawSurface.blitFrom(_imageMain, _startButtonHeld ? _startButtonSrcRects[1] : _startButtonSrcRects[0],
		                      Common::Point(_startButtonDestRect.left, _startButtonDestRect.top));
		return;

	case kWinScreen:
	case kCountdown:
		if (_executeWin) {
			_drawSurface.blitFrom(_imageWin, Common::Point(_overlayRect.left, _overlayRect.top));
			return;
		}
		// Loss – just show the try-again overlay; actual scene change pending
		_drawSurface.blitFrom(_imageTryAgain, Common::Point(_overlayRect.left, _overlayRect.top));
		return;

	default:
		break;
	}

	// ---- Active gameplay ----

	// Draw ocean floor background sprites (behind everything else)
	for (int i = 0; i < kNumFloorSprites; ++i) {
		_drawSurface.blitFrom(_imageMain, _floorSrcRects[i],
		                      Common::Point(_floorDestRects[i].left, _floorDestRects[i].top));
	}

	// Draw oxygen bar (left side): stage 0 = full oxygen (no bar shown); stages 1-8 show depletion
	if (_oxygenStage > 0 && _oxygenStage <= kOxygenStages) {
		_drawSurface.blitFrom(_imageMain, _oxygenSrcRects[_oxygenStage - 1],
		                      Common::Point(_oxygenBarX, _oxygenBarY));
	}

	// Draw quit button (pressed variant while mouse held)
	_drawSurface.blitFrom(_imageMain, _quitButtonHeld ? _quitButtonSrcRects[1] : _quitButtonSrcRects[0],
	                      Common::Point(_quitButtonDestRect.left, _quitButtonDestRect.top));

	// Draw entities (fish and hazards)
	for (int i = 0; i < kMaxEntities; ++i) {
		const Entity &e = _entities[i];
		if (!e.active || e.removed)
			continue;
		// Check entity is within playfield
		if (e.x + e.width <= _playfieldRect.left || e.x >= _playfieldRect.right)
			continue;

		const EntityType &et = (e.kind == kEntityFish) ? _fishTypes[e.typeIdx] : _hazardTypes[e.typeIdx];

		// Blit mainSrc as base layer first (fills any gaps between sub-sprites)
		int mainScreenX = e.x;
		int mainScreenY = e.y;
		Common::Rect mainDrawSrc = et.mainSrc;
		if (mainScreenX < _playfieldRect.left) {
			mainDrawSrc.left += (_playfieldRect.left - mainScreenX);
			mainScreenX = _playfieldRect.left;
		}
		if (mainScreenX + mainDrawSrc.width() > _playfieldRect.right)
			mainDrawSrc.right = mainDrawSrc.left + (_playfieldRect.right - mainScreenX);
		if (!mainDrawSrc.isEmpty())
			_drawSurface.blitFrom(_imageMain, mainDrawSrc, Common::Point(mainScreenX, mainScreenY));

		for (int j = 0; j < kNumSubSprites; ++j) {
			const SubSprite &ss = et.subSprites[j];
			if (!ss.active)
				continue;

			int offX    = ss.src.left - et.mainSrc.left;
			int offY    = ss.src.top  - et.mainSrc.top;
			int screenX = e.x + offX;
			int screenY = e.y + offY;

			// Skip if entirely outside playfield horizontally
			if (screenX + ss.src.width() <= _playfieldRect.left)
				continue;
			if (screenX >= _playfieldRect.right)
				continue;

			Common::Rect drawSrc = ss.src;
			// Clip left edge to playfield boundary
			if (screenX < _playfieldRect.left) {
				drawSrc.left += (_playfieldRect.left - screenX);
				screenX = _playfieldRect.left;
			}
			// Clip right edge to playfield boundary
			if (screenX + drawSrc.width() > _playfieldRect.right)
				drawSrc.right = drawSrc.left + (_playfieldRect.right - screenX);

			if (drawSrc.isEmpty())
				continue;

			_drawSurface.blitFrom(_imageMain, drawSrc,
			                      Common::Point(screenX, screenY));
		}
	}

	// Draw porpoise
	const Common::Rect *porpSrc = nullptr;
	switch (_porpoiseAnim) {
	case kPorpoiseSwim:
	case kPorpoiseSurface:
	case kPorpoiseEat:
		porpSrc = &_porpoiseSwimmingRect;
		break;
	case kPorpoiseHit:
		porpSrc = &_porpoiseBellyUpRect;
		break;
	}

	if (porpSrc)
		_drawSurface.blitFrom(_imageMain, *porpSrc,
		                      Common::Point(_porpLeft, _porpTop));

	// Draw bubble animation frame on top of porpoise when surfacing.
	if (_porpoiseAnim == kPorpoiseSurface && _breathFrame < kOxygenStages) {
		const Common::Rect &bubbleSrc = _bubbleSrcRects[_breathFrame];
		int bubbleX = _porpLeft - 5 + (_porpWidth - bubbleSrc.width()) / 2;
		int bubbleY = _porpTop + 9 - bubbleSrc.height();
		_drawSurface.transBlitFrom(_imageMain, bubbleSrc,
		                           Common::Point(bubbleX, bubbleY),
		                           g_nancy->_graphics->getTransColor());
	}

	// Draw lives indicator (small porpoise icons)
	for (int i = 0; i < _lives && i < kMaxLives; ++i) {
		_drawSurface.blitFrom(_imageMain, _livesSrcRect,
		                      Common::Point(_livesDestRects[i].left,
		                                    _livesDestRects[i].top));
	}

	// Draw score – display as 5 digits
	_prevScore = _score;
	// Score is raw points; display with zero-padding to 5 digits
	int displayScore = _score;
	const int kDigits = 5;
	int digits[kDigits];
	for (int d = kDigits - 1; d >= 0; --d) {
		digits[d] = displayScore % 10;
		displayScore /= 10;
	}
	// Digit sprite width (including gap) = 13 pixels
	const int digitW = 13;
	for (int d = 0; d < kDigits; ++d) {
		int idx = digits[d]; // 0..9
		_drawSurface.blitFrom(_imageMain, _scoreDigitSrcRects[idx],
		                      Common::Point(_scoreX + d * digitW, _scoreY));
	}
}

int WhaleSurvivorPuzzle::findFreeEntity() const {
	for (int i = 0; i < kMaxEntities; ++i)
		if (!_entities[i].active || _entities[i].removed)
			return i;
	return -1;
}

} // End of namespace Action
} // End of namespace Nancy
