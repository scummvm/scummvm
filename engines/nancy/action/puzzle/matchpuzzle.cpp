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

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/matchpuzzle.h"

#include "graphics/font.h"

#include "common/system.h"
#include "common/random.h"

namespace Nancy {
namespace Action {

static const uint kNumHighScores = 5;

// How long a match stays highlighted, and how long a button stays pressed
static const uint32 kMatchAnimTime = 800;
static const uint32 kButtonDownTime = 250;
// Pause on the WIN!!/TIME! message before the high score screen comes up
static const uint32 kEndDelayTime = 2000;

// The board has to sit above the viewport ornaments, which draw the panels the
// score, target and timer are written into
static const uint16 kNancy14ZOrder = 10;

void MatchPuzzle::readDataNancy14(Common::SeekableReadStream &stream) {
	readFilename(stream, _overlayName);
	readFilename(stream, _buttonsImageName);

	_rows = stream.readSint16LE();
	_cols = stream.readSint16LE();

	_startInactive = stream.readByte() != 0;
	_inProgressFlag = stream.readSint16LE();
	stream.skip(2);

	readRect(stream, _doneButtonSrcRect);
	readRect(stream, _doneButtonDestRect);
	readRect(stream, _shuffleButtonSrcRect);
	readRect(stream, _shuffleButtonDestRect);

	_gridOffX = stream.readSint16LE();
	_gridOffY = stream.readSint16LE();
	_rowSpacing = stream.readSint16LE();
	_colSpacing = stream.readSint16LE();

	_fontID = stream.readUint16LE();
	_fontColor = stream.readUint16LE();

	readFilename(stream, _timerSuffix);
	readFilename(stream, _winString);
	readFilename(stream, _timeUpString);

	readRect(stream, _scoreValueRect);
	readRect(stream, _goalValueRect);
	readRect(stream, _timerValueRect);
	readRect(stream, _highScoreButtonRect);

	_timeLimitSecs = stream.readSint16LE();
	_scorePerTile = stream.readSint16LE();
	_timeBonusFor3 = stream.readSint16LE();
	_scoreBonusFor4 = stream.readSint16LE();
	_timeBonusFor4 = stream.readSint16LE();
	_scoreBonusFor5 = stream.readSint16LE();
	_timeBonusFor5 = stream.readSint16LE();
	_defaultScoreTarget = stream.readSint32LE();

	readRect(stream, _matchedTileSrcRect);

	_numTileTypes = stream.readSint16LE();
	readRectArray(stream, _tileSrcRects, _numTileTypes);

	readFilename(stream, _highScoreImageName);
	readFilename(stream, _playerName);

	readRectArray(stream, _highScoreRects, kNumHighScores);

	_highScores.resize(kNumHighScores);
	for (uint i = 0; i < kNumHighScores; ++i) {
		readFilename(stream, _highScores[i].name);
		_highScores[i].score = stream.readSint32LE();
	}

	_matchSound.readData(stream);
	_selectSound.readData(stream);
	_swapSound.readData(stream);
	_winSound.readData(stream);
	_timeUpSound.readData(stream);
	_goButtonSound.readData(stream);

	_solveScene._sceneChange.sceneID = stream.readUint16LE();
	_solveScene._sceneChange.frameID = stream.readUint16LE();
	_solveScene._flag.label = stream.readSint16LE();
	_solveScene._flag.flag = stream.readByte();

	_exitCursorType = stream.readUint16LE();
	_exitCursorFromData = true;
	_exitScene._sceneChange.sceneID = stream.readUint16LE();
	_exitScene._sceneChange.frameID = stream.readUint16LE();

	readRect(stream, _exitHotspot);

	_doneSceneChange._sceneChange.sceneID = stream.readUint16LE();
	_doneSceneChange._sceneChange.frameID = stream.readUint16LE();
}

void MatchPuzzle::readData(Common::SeekableReadStream &stream) {
	if (g_nancy->getGameType() >= kGameTypeNancy14) {
		readDataNancy14(stream);
		return;
	}

	// data+0x00..0x20  main sprite sheet name
	readFilename(stream, _overlayName);
	// data+0x21..0x41  score-panel background name
	readFilename(stream, _flagPointBgName);

	_rows         = stream.readSint16LE();  // data+0x42
	_cols         = stream.readSint16LE();  // data+0x44
	_numTileTypes = stream.readSint16LE();  // data+0x46

	readRect(stream, _shuffleButtonSrcRect);   // data+0x48..0x57 (source rect in sprite sheet)

	_tileSrcRects.resize(26);

	for (int i = 0; i < 26; ++i)
		readRect(stream, _tileSrcRects[i]); // data+0x58..0x1F7 (source rects in sprite sheet)

	// data+0x1F8..0x237 — 64 bytes unused (all zeros)
	stream.skip(0x40);

	// data+0x238  script execution / flag-info display enable (byte)
	_execScript = stream.readByte() != 0;
	stream.skip(1);                         // data+0x239 padding
	_scriptID   = stream.readSint16LE();    // data+0x23A script ID

	// data+0x23C  score-panel font ID (uint16)
	/*_scorePanelFontID = */stream.readUint16LE();

	// data+0x23E..0x25E  33-byte label string drawn in score panel
	readFilename(stream, _displayLabelString);

	// data+0x25F..0x5B8 — 26 per-flag-type names (33 bytes each)
	readFilenameArray(stream, _flagSoundNames, 26);

	// data+0x5B9..0x63C — 132 bytes (4 × 33-byte strings, e.g. score-panel labels)
	stream.skip(132);

	// data+0x63D  score-panel display enable (byte)
	_showScoreDisplay = stream.readByte() != 0;
	_timeLimitSecs  = stream.readSint16LE();          // data+0x63E
	_scoreTarget    = stream.readSint32LE();          // data+0x640
	_scorePerTile   = stream.readSint16LE();          // data+0x644

	readRect(stream, _matchedTileSrcRect);            // data+0x646..0x655 matched/highlight src rect

	_timeBonusFor3  = stream.readSint16LE();          // data+0x656 (seconds)
	_scoreBonusFor4 = stream.readSint16LE();          // data+0x658
	_timeBonusFor4  = stream.readSint16LE();          // data+0x65A (seconds)
	_scoreBonusFor5 = stream.readSint16LE();          // data+0x65C
	_timeBonusFor5  = stream.readSint16LE();          // data+0x65E (seconds)
	_gridOffX       = stream.readSint16LE();          // data+0x660
	_gridOffY       = stream.readSint16LE();          // data+0x662
	_rowSpacing     = stream.readSint16LE();          // data+0x664
	_colSpacing     = stream.readSint16LE();          // data+0x666

	// data+0x668..0x6E7 — 8 × 16-byte viewport-local destination rects
	readRect(stream, _labelStringRect);       // data+0x668 — label string display rect
	readRect(stream, _shuffleButtonDestRect); // data+0x678 — shuffle button on-screen rect (hotspot)
	readRect(stream, _goalValueRect);         // data+0x688 — goal value text rect
	readRect(stream, _scoreValueRect);        // data+0x698 — score value text rect
	readRect(stream, _timerValueRect);        // data+0x6A8 — timer text rect
	readRect(stream, _flagNameRect);          // data+0x6B8 — matched flag name text rect
	readRect(stream, _flagImageRect);         // data+0x6C8 — matched flag image rect
	readRect(stream, _hsDisplayRect);         // data+0x6D8 — high-score display positions

	_scoreDisplayDelay = stream.readSint16LE();       // data+0x6E8

	_slotWinSound.readNormal(stream);           // data+0x6EA..0x71A  match animation sound
	_shuffleSound.readNormal(stream);           // data+0x71B..0x74B
	_cardPlaceSound.readNormal(stream);         // data+0x74C..0x77C

	_solveScene.readData(stream);         // data+0x77D..0x795  win scene
	stream.skip(2);                             // data+0x796..0x797 pre-result delay (unused)

	_matchSuccessSound.readNormal(stream);      // data+0x798..0x7C8  win/time-up sound

	_exitScene.readData(stream);          // data+0x7C9..0x7E1  quit/exit scene

	readRect(stream, _exitHotspot);             // data+0x7E2..0x7F1 exit hotspot
}

void MatchPuzzle::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();

	_drawSurface.create(vpBounds.width(), vpBounds.height(),
	                    g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(vpBounds);

	g_nancy->_resource->loadImage(_overlayName, _image);
	_image.setTransparentColor(_drawSurface.getTransparentColor());

	if (!_flagPointBgName.empty()) {
		g_nancy->_resource->loadImage(_flagPointBgName, _scorePanelImage);
		_scorePanelImage.setTransparentColor(_drawSurface.getTransparentColor());
	}

	if (!_buttonsImageName.empty()) {
		g_nancy->_resource->loadImage(_buttonsImageName, _buttonsImage);
		_buttonsImage.setTransparentColor(_drawSurface.getTransparentColor());
	}

	if (!_highScoreImageName.empty()) {
		g_nancy->_resource->loadImage(_highScoreImageName, _highScoreImage);
		_highScoreImage.setTransparentColor(_drawSurface.getTransparentColor());
	}

	// Build grid — compute dest rects; cells will be filled by shuffleGrid()
	_grid.resize(_cols);
	for (int col = 0; col < _cols; ++col) {
		_grid[col].resize(_rows);
		for (int row = 0; row < _rows; ++row)
			computeDestRect(col, row);
	}

	if (_highScores.empty())
		_highScores.resize(kNumHighScores);

	sortHighScores();

	if (g_nancy->getGameType() >= kGameTypeNancy14) {
		setZOrder(kNancy14ZOrder);

		// The target is whatever tops the high score list, and it has to be beaten outright
		_scoreTarget = _highScores[0].score > 0 ? _highScores[0].score : _defaultScoreTarget;
		_goalStr = Common::String::format("%d", _scoreTarget);
		startRound();

		if (_startInactive) {
			_showHighScores = true;
			_canResumeGame = true;
			_gameSubState = kHighScores;
		}

		NancySceneState.setEventFlag(_inProgressFlag, _startInactive ? g_nancy->_true : g_nancy->_false);
		redrawAllCells();
		return;
	}

	// Initialise display strings
	_goalStr  = Common::String::format("%d", _scoreTarget);
	_scoreStr = Common::String::format("%d", (int32)0);
	if (_timeLimitSecs > 0)
		_timerStr = Common::String::format("%2dm %2ds", _timeLimitSecs / 60, _timeLimitSecs % 60);

	shuffleGrid(true);
	redrawAllCells();
}

void MatchPuzzle::playMatchSound() {
	if (g_nancy->getGameType() >= kGameTypeNancy14) {
		playSoundBlock(_matchSound);
	} else if (_slotWinSound.name != "NO SOUND") {
		g_nancy->_sound->playSound(_slotWinSound);
	}
}

bool MatchPuzzle::isMatchSoundPlaying() const {
	if (g_nancy->getGameType() >= kGameTypeNancy14)
		return isSoundBlockPlaying(_matchSound);

	return g_nancy->_sound->isSoundPlaying(_slotWinSound);
}

// Reset score and timer and deal a fresh board
void MatchPuzzle::startRound() {
	_score = 0;
	_scoreStr = Common::String::format("%d", _score);
	_timerStr.clear();
	_prevTimerSecs = -1;
	_hasPiece1 = _hasPiece2 = false;
	_hasSelection = false;
	_timerDeadline = g_system->getMillis() + (uint32)_timeLimitSecs * 1000;
	shuffleGrid(true);
}

void MatchPuzzle::sortHighScores() {
	for (uint i = 0; i + 1 < _highScores.size(); ++i)
		for (uint j = 0; j + 1 < _highScores.size() - i; ++j)
			if (_highScores[j].score < _highScores[j + 1].score)
				SWAP(_highScores[j], _highScores[j + 1]);
}

// Insert the score just achieved into the list, pushing the rest down
void MatchPuzzle::insertHighScore() {
	for (uint i = 0; i < _highScores.size(); ++i) {
		if (_highScores[i].score >= _score)
			continue;

		for (uint j = _highScores.size() - 1; j > i; --j)
			_highScores[j] = _highScores[j - 1];

		_highScores[i].name = _playerName;
		_highScores[i].score = _score;
		return;
	}
}

void MatchPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		NancySceneState.setNoHeldItem();

		if (_slotWinSound.name != "NO SOUND")
			g_nancy->_sound->loadSound(_slotWinSound);
		if (_shuffleSound.name != "NO SOUND")
			g_nancy->_sound->loadSound(_shuffleSound);
		if (_cardPlaceSound.name != "NO SOUND")
			g_nancy->_sound->loadSound(_cardPlaceSound);

		_score        = 0;
		_wonGame      = false;
		_hasPiece1 = _hasPiece2 = false;
		_hasSelection = false;
		_showFlagName = false;
		_prevTimerSecs = -1;

		// init() already put Nancy14 into its starting substate
		if (g_nancy->getGameType() < kGameTypeNancy14) {
			_gameSubState = kPlaying;

			if (_timeLimitSecs > 0)
				_timerDeadline = g_system->getMillis() + (uint32)_timeLimitSecs * 1000;
		}

		_state = kRun;
		// fall through

	case kRun:
		switch (_gameSubState) {

		case kPlaying: { // Playing — update timer display, process pending clicks, check win/lose
			uint32 now = g_system->getMillis();

			// Update countdown timer display once per second
			if (_timeLimitSecs > 0) {
				int32 remainMs = (int32)(_timerDeadline - now);
				if (remainMs < 0) remainMs = 0;
				int secs = remainMs / 1000;
				if (secs != _prevTimerSecs) {
					_prevTimerSecs = secs;
					if (g_nancy->getGameType() >= kGameTypeNancy14)
						_timerStr = Common::String::format("%2d%s", secs, _timerSuffix.c_str());
					else
						_timerStr = Common::String::format("%2dm %2ds", secs / 60, secs % 60);
					redrawAllCells();
				}
			}

			// Process piece 1 first
			if (_hasPiece1) {
				_hasPiece1 = false;
				checkForMatch(_piece1Col, _piece1Row);
				if (_hasVMatch || _hasHMatch) {
					_hasSelection = false;
					if (_execScript && (uint16)_matchedFlagType < _flagSoundNames.size()) {
						_flagNameStr  = _flagSoundNames[_matchedFlagType];
						_showFlagName = true;
					}
					_scoreStr = Common::String::format("%d", _score);
					playMatchSound();
					_stateTimer   = now + kMatchAnimTime;
					_gameSubState = kMatchAnim;
					redrawAllCells();
				}
				break;
			}

			// Process piece 2 on the next frame after piece 1 is cleared
			if (_hasPiece2) {
				_hasPiece2 = false;
				checkForMatch(_piece2Col, _piece2Row);
				if (_hasVMatch || _hasHMatch) {
					_hasSelection = false;
					if (_execScript && (uint16)_matchedFlagType < _flagSoundNames.size()) {
						_flagNameStr  = _flagSoundNames[_matchedFlagType];
						_showFlagName = true;
					}
					_scoreStr = Common::String::format("%d", _score);
					playMatchSound();
					_stateTimer   = now + kMatchAnimTime;
					_gameSubState = kMatchAnim;
					redrawAllCells();
				}
				break;
			}

			// Neither pending: check win/lose conditions
			bool timerExpired  = (_timeLimitSecs > 0) && ((int32)(_timerDeadline - now) < 500);

			if (g_nancy->getGameType() >= kGameTypeNancy14) {
				// The score is only judged when the clock runs out, and the
				// high score at the top of the list has to be beaten outright
				if (timerExpired) {
					_wonGame = _score > _scoreTarget;
					_timerStr = _wonGame ? _winString : _timeUpString;
					_hasSelection = false;

					playSoundBlock(_wonGame ? _winSound : _timeUpSound);

					_stateTimer = now + kEndDelayTime;
					_gameSubState = kEndDelay;
					redrawAllCells();
				}
				break;
			}

			bool reachedTarget = (_score >= _scoreTarget);

			if (timerExpired || reachedTarget) {
				_wonGame      = reachedTarget;
				_timerStr     = _wonGame ? "WIN!!!" : "TIME!";
				_hasSelection = false;
				_gameSubState = kStartEndSeq;
				redrawAllCells();
			}
			break;
		}

		case kStartEndSeq: {
			if (_matchSuccessSound.name != "NO SOUND") {
				g_nancy->_sound->loadSound(_matchSuccessSound);
				g_nancy->_sound->playSound(_matchSuccessSound);
			}
			_stateTimer   = g_system->getMillis() + (uint32)_scoreDisplayDelay * 1000;
			_gameSubState = kWaitDelay;
			break;
		}

		case kMatchAnim: { // Match animation: wait for 800ms timer and sound to finish.
			// While the 800ms window is still open, the slot-win "boop"
			// sound is *re-triggered* every time the previous play
			// finishes. With a short sample, that fits roughly three plays
			// inside the window — which is why a match makes three boops.
			uint32 now = g_system->getMillis();
			bool timerDone = (now >= _stateTimer);
			bool soundDone = !isMatchSoundPlaying();

			if (timerDone && soundDone) {
				// Reshuffle only the cells that were part of the match
				for (int c = 0; c < _cols; ++c)
					for (int r = 0; r < _rows; ++r)
						if (_grid[c][r].matched)
							shuffleGrid(false, c, r);

				_hasVMatch = _hasHMatch = false;
				_showFlagName = false;
				redrawAllCells();
				_gameSubState = kPlaying;
			} else if (soundDone) {
				// Sound has finished but the match-anim window hasn't
				// closed yet — replay it for the next "boop".
				playMatchSound();
			}
			break;
		}

		case kShuffleDelay: { // Shuffle-button delay
			if (g_system->getMillis() >= _shuffleTimer) {
				shuffleGrid(true);
				redrawAllCells();
				_gameSubState = kPlaying;
			}
			break;
		}

		case kWaitSound: { // Wait for win/time-up sound to finish, then enter score display
			if (!g_nancy->_sound->isSoundPlaying(_matchSuccessSound)) {
				_stateTimer   = g_system->getMillis() + (uint32)_scoreDisplayDelay * 1000;
				_gameSubState = kScoreDisplay;
				redrawAllCells(); // show the final-score / high-score screen
			}
			break;
		}

		case kWaitDelay: { // Wait for delay timer, then go to sound-wait
			if (g_system->getMillis() >= _stateTimer)
				_gameSubState = kWaitSound;
			break;
		}

		case kScoreDisplay: { // Score display — show until timer expires, then insert score and exit/reset
			if (g_system->getMillis() < _stateTimer)
				break;

			// Insert current score into the top-5 high score list (descending)
			int32 toInsert = _score;
			for (int i = 0; i < 5; ++i) {
				if (_highScores[i].score < toInsert)
					SWAP(_highScores[i].score, toInsert);
			}

			if (_wonGame) {
				_state = kActionTrigger;
			} else {
				// Time ran out — fresh round
				_score         = 0;
				_scoreStr      = Common::String::format("%d", (int32)0);
				_hasPiece1     = _hasPiece2 = false;
				_hasSelection  = false;
				_showFlagName  = false;
				_prevTimerSecs = -1;
				if (_timeLimitSecs > 0)
					_timerDeadline = g_system->getMillis() + (uint32)_timeLimitSecs * 1000;
				shuffleGrid(true);
				_gameSubState  = kPlaying;
				redrawAllCells();
			}
			break;
		}

		case kButtonDown: { // Nancy14: a button is held down for a moment before it acts
			if (g_system->getMillis() < _stateTimer)
				break;

			if (_shuffleButtonDown) {
				_shuffleButtonDown = false;
				_hasSelection = false;
				shuffleGrid(true);
			} else {
				_doneButtonDown = false;

				playSoundBlock(_goButtonSound);

				_showHighScores = false;
				NancySceneState.setEventFlag(_inProgressFlag, g_nancy->_false);

				if (_canResumeGame) {
					_canResumeGame = false;
					startRound();
				}
			}

			_gameSubState = kPlaying;
			redrawAllCells();
			break;
		}

		case kEndDelay: { // Nancy14: hold the win/time-up message, then show the high scores
			if (g_system->getMillis() < _stateTimer)
				break;

			insertHighScore();
			_showHighScores = true;
			_canResumeGame = true;
			_gameSubState = kHighScores;
			NancySceneState.setEventFlag(_inProgressFlag, g_nancy->_true);
			redrawAllCells();
			break;
		}

		case kHighScores: // Nancy14: waiting for the GO button
			break;

		default:
			break;
		}
		break;

	case kActionTrigger:
		g_nancy->_sound->stopSound(_slotWinSound);
		g_nancy->_sound->stopSound(_shuffleSound);
		g_nancy->_sound->stopSound(_cardPlaceSound);
		g_nancy->_sound->stopSound(_matchSuccessSound);

		if (_wonGame)
			_solveScene.execute();
		else if (_leftThroughButton)
			_doneSceneChange.execute();
		else
			_exitScene.execute();

		finishExecution();
		break;
	}
}

void MatchPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun)
		return;
	// Convert mouse to viewport-local coordinates.
	Common::Rect vpPos = NancySceneState.getViewport().getScreenPosition();
	Common::Point localMouse = input.mousePos;
	localMouse -= Common::Point(vpPos.left, vpPos.top);

	if (isExitHotspotHovered(input)) {
		if (g_nancy->getGameType() >= kGameTypeNancy14)
			setExitCursor();
		else
			g_nancy->_cursor->setCursorType(CursorManager::kMoveBackward);

		if (input.input & NancyInput::kLeftMouseButtonUp)
			_state = kActionTrigger;
	}

	if (g_nancy->getGameType() >= kGameTypeNancy14) {
		handleInputNancy14(input, localMouse);
		return;
	}

	if (_gameSubState != kPlaying)
		return;

	// Shuffle button — use the on-screen destination rect for hit-testing
	if (_shuffleButtonDestRect.contains(localMouse)) {
		g_nancy->_cursor->setCursorType(CursorManager::kHotspot);
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_hasSelection = false;
			if (_shuffleSound.name != "NO SOUND")
				g_nancy->_sound->playSound(_shuffleSound);
			_shuffleTimer = g_system->getMillis() + 500;
			_gameSubState = kShuffleDelay;
		}
		return;
	}

	// Grid cell click
	for (int col = 0; col < _cols; ++col) {
		for (int row = 0; row < _rows; ++row) {
			if (!_grid[col][row].visible)
				continue;
			if (_grid[col][row].destRect.contains(localMouse)) {
				g_nancy->_cursor->setCursorType(CursorManager::kHotspot);
				if (input.input & NancyInput::kLeftMouseButtonUp) {
					if (!_hasSelection) {
						// First click: remember this cell as the selection
						_selCol = col;
						_selRow = row;
						_hasSelection = true;
						if (_cardPlaceSound.name != "NO SOUND")
							g_nancy->_sound->playSound(_cardPlaceSound);
						redrawAllCells();
					} else if (col == _selCol && row == _selRow) {
						// Clicked same cell again: deselect
						_hasSelection = false;
						redrawAllCells();
					} else {
						// Second click: swap the two flags and queue both for match-check
						SWAP(_grid[_selCol][_selRow].tileType,
						     _grid[col][row].tileType);

						_piece1Col = _selCol;
						_piece1Row = _selRow;
						_piece2Col = col;
						_piece2Row = row;
						_hasPiece1 = true;
						_hasPiece2 = true;
						_hasSelection = false;

						if (_cardPlaceSound.name != "NO SOUND")
							g_nancy->_sound->playSound(_cardPlaceSound);
						redrawAllCells();
					}
				}
				return;
			}
		}
	}
}

void MatchPuzzle::handleInputNancy14(NancyInput &input, const Common::Point &localMouse) {
	if (_state != kRun)
		return;

	if (_gameSubState != kPlaying && _gameSubState != kHighScores)
		return;

	// The button next to the board doubles as GO on the high score screen: it
	// leaves the puzzle while a round is running or once the target was beaten,
	// and dismisses the high scores otherwise
	if (_doneButtonDestRect.contains(localMouse)) {
		g_nancy->_cursor->setCursorType(CursorManager::kHotspot);
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_leftThroughButton = true;

			if (!_showHighScores || _wonGame) {
				_state = kActionTrigger;
			} else {
				_doneButtonDown = true;
				_stateTimer = g_system->getMillis() + kButtonDownTime;
				_gameSubState = kButtonDown;
				redrawAllCells();
			}
		}
		input.eatMouseInput();
		return;
	}

	// Brings up the high score list mid-game; GO puts the board back
	if (_gameSubState != kHighScores && _highScoreButtonRect.contains(localMouse)) {
		g_nancy->_cursor->setCursorType(CursorManager::kHotspot);
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_showHighScores = true;
			_gameSubState = kHighScores;
			NancySceneState.setEventFlag(_inProgressFlag, g_nancy->_true);
			redrawAllCells();
		}
		input.eatMouseInput();
		return;
	}

	if (_gameSubState != kPlaying)
		return;

	if (_shuffleButtonDestRect.contains(localMouse)) {
		g_nancy->_cursor->setCursorType(CursorManager::kHotspot);
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			playSoundBlock(_swapSound);

			_shuffleButtonDown = true;
			_stateTimer = g_system->getMillis() + kButtonDownTime;
			_gameSubState = kButtonDown;
			redrawAllCells();
		}
		input.eatMouseInput();
		return;
	}

	for (int col = 0; col < _cols; ++col) {
		for (int row = 0; row < _rows; ++row) {
			GridCell &cell = _grid[col][row];
			if (!cell.visible)
				continue;

			// The clickable area is inset slightly from the tile
			Common::Rect hotspot = cell.destRect;
			hotspot.grow(-3);
			if (!hotspot.contains(localMouse))
				continue;

			g_nancy->_cursor->setCursorType(CursorManager::kHotspot);
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				if (!_hasSelection) {
					_selCol = col;
					_selRow = row;
					_hasSelection = true;

					playSoundBlock(_selectSound);
				} else {
					playSoundBlock(_swapSound);

					SWAP(_grid[_selCol][_selRow].tileType, cell.tileType);

					_piece1Col = _selCol;
					_piece1Row = _selRow;
					_piece2Col = col;
					_piece2Row = row;
					_hasPiece1 = _hasPiece2 = true;
					_hasSelection = false;
					redrawAllCells();
				}
			}

			input.eatMouseInput();
			return;
		}
	}
}

void MatchPuzzle::shuffleGrid(bool allCells, int targetCol, int targetRow) {
	// Valid flag indices are 0 .. (_numTileTypes - 2) inclusive
	int numTypes = (_numTileTypes > 1) ? (_numTileTypes - 1) : 1;

	for (int row = 0; row < _rows; ++row) {
		for (int col = 0; col < _cols; ++col) {
			if (!allCells && (col != targetCol || row != targetRow))
				continue;

			// Pick a random type that doesn't match its above or left neighbour.
			// A single tile dealt back into a finished match also has to avoid
			// the tiles below and to the right of it, so it can't match again
			// on the spot.
			bool checkAllNeighbors = !allCells && g_nancy->getGameType() >= kGameTypeNancy14;
			int16 chosen = 0;
			for (int attempt = 0; attempt < 100; ++attempt) {
				chosen = (int16)(g_nancy->_randomSource->getRandomNumber(numTypes - 1));
				bool sameAbove = (row > 0) && (chosen == _grid[col][row - 1].tileType);
				bool sameLeft  = (col > 0) && (chosen == _grid[col - 1][row].tileType);
				bool sameBelow = checkAllNeighbors && (row < _rows - 1) && (chosen == _grid[col][row + 1].tileType);
				bool sameRight = checkAllNeighbors && (col < _cols - 1) && (chosen == _grid[col + 1][row].tileType);
				if (!sameAbove && !sameLeft && !sameBelow && !sameRight)
					break;
			}

			_grid[col][row].tileType = chosen;
			_grid[col][row].visible  = true;
			_grid[col][row].matched  = false;
		}
	}
}

void MatchPuzzle::checkForMatch(int col, int row) {
	_hasVMatch = _hasHMatch = false;

	if (col < 0 || col >= _cols || row < 0 || row >= _rows)
		return;
	if (!_grid[col][row].visible)
		return;

	int16 type = _grid[col][row].tileType;
	_matchedFlagType = type;

	// --- Vertical run (fixed column, walk along rows) ---
	int rStart = row, rEnd = row;
	while (rStart > 0        && _grid[col][rStart - 1].tileType == type) --rStart;
	while (rEnd   < _rows - 1 && _grid[col][rEnd   + 1].tileType == type) ++rEnd;
	_matchRowStart = rStart;
	_matchRowEnd   = rEnd;

	// --- Horizontal run (fixed row, walk along cols) ---
	int cStart = col, cEnd = col;
	while (cStart > 0        && _grid[cStart - 1][row].tileType == type) --cStart;
	while (cEnd   < _cols - 1 && _grid[cEnd   + 1][row].tileType == type) ++cEnd;
	_matchColStart = cStart;
	_matchColEnd   = cEnd;

	int vLen = rEnd - rStart; // 2 -> 3-match, 3 -> 4-match, 4 -> 5-match
	int hLen = cEnd - cStart;

	// --- Score and mark: vertical match (>=3 flags) ---
	if (vLen >= 2) {
		_hasVMatch = true;
		for (int r = rStart; r <= rEnd; ++r) {
			_grid[col][r].matched = true;
			_score += _scorePerTile;
		}
		if (vLen == 2)
			_timerDeadline += (uint32)_timeBonusFor3 * 1000;
		else if (vLen == 3) {
			_score         += _scoreBonusFor4;
			_timerDeadline += (uint32)_timeBonusFor4 * 1000;
		} else if (vLen >= 4) {
			_score         += _scoreBonusFor5;
			_timerDeadline += (uint32)_timeBonusFor5 * 1000;
		}
	}

	// --- Score and mark: horizontal match (>=3 flags) ---
	if (hLen >= 2) {
		_hasHMatch = true;
		for (int c = cStart; c <= cEnd; ++c) {
			_grid[c][row].matched = true;
			_score += _scorePerTile;
		}
		if (hLen == 2)
			_timerDeadline += (uint32)_timeBonusFor3 * 1000;
		else if (hLen == 3) {
			_score         += _scoreBonusFor4;
			_timerDeadline += (uint32)_timeBonusFor4 * 1000;
		} else if (hLen >= 4) {
			_score         += _scoreBonusFor5;
			_timerDeadline += (uint32)_timeBonusFor5 * 1000;
		}
	}

	// Nancy14 has to be able to overshoot the target to beat it
	if (g_nancy->getGameType() < kGameTypeNancy14 && _score > _scoreTarget)
		_score = _scoreTarget;
}

void MatchPuzzle::computeDestRect(int col, int row) {
	if (_tileSrcRects.empty())
		return;

	// Cell size taken from the first flag rect (all flags are the same size)
	int cellW = _tileSrcRects[0].width() - 1;
	int cellH = _tileSrcRects[0].height() - 1;

	// Column position: extra spacing per col + cell width
	int left = col * (_colSpacing + cellW) + _gridOffX;
	// Row position: extra spacing per row + cell height
	int top  = row * (_rowSpacing + cellH) + _gridOffY;

	_grid[col][row].destRect = Common::Rect(left, top, left + cellW, top + cellH);
}

// ---- rendering helpers ------------------------------------------------------

void MatchPuzzle::drawCell(int col, int row) {
	const GridCell &cell = _grid[col][row];
	if (!cell.visible)
		return;

	int type = cell.tileType;
	if (type < 0 || type >= (int)_tileSrcRects.size())
		return;

	// Draw matched cells with the highlight source rect ("50" graphic); others with their normal rect
	const Common::Rect &srcRect = cell.matched ? _matchedTileSrcRect : _tileSrcRects[type];
	_drawSurface.blitFrom(_image, srcRect,
	                      Common::Point(cell.destRect.left, cell.destRect.top));
	_needsRedraw = true;
}

void MatchPuzzle::eraseCell(int col, int row) {
	_drawSurface.fillRect(_grid[col][row].destRect,
	                      _drawSurface.getTransparentColor());
	_needsRedraw = true;
}

// The original anchors text at the bottom row of the glyphs, ScummVM at the top of the line
void MatchPuzzle::drawText(const Common::String &str, const Common::Point &pos) {
	if (str.empty())
		return;

	const Graphics::Font *font = g_nancy->_graphics->getFont(_fontID);
	if (!font)
		font = g_nancy->_graphics->getFont(0);

	if (!font)
		return;

	int y = pos.y - font->getFontHeight() + 1;
	font->drawString(&_drawSurface, str, pos.x, y, _drawSurface.w - pos.x, _fontColor);
	_needsRedraw = true;
}

void MatchPuzzle::drawHighScoreScreen() {
	if (!_highScoreImage.empty())
		_drawSurface.blitFrom(_highScoreImage, Common::Point(0, 0));

	for (uint i = 0; i < _highScoreRects.size() && i < _highScores.size(); ++i) {
		const Common::Rect &rect = _highScoreRects[i];
		drawText(_highScores[i].name, Common::Point(rect.left, rect.top));
		drawText(Common::String::format("%d", _highScores[i].score),
		         Common::Point(rect.right, rect.bottom));
	}

	_needsRedraw = true;
}

void MatchPuzzle::drawBoardNancy14() {
	for (int col = 0; col < _cols; ++col)
		for (int row = 0; row < _rows; ++row)
			drawCell(col, row);

	// An empty score rect turns the whole score/target readout off
	if (!_scoreValueRect.isEmpty()) {
		drawText(_scoreStr, Common::Point(_scoreValueRect.left, _scoreValueRect.bottom));
		drawText(_goalStr, Common::Point(_goalValueRect.left, _goalValueRect.bottom));
	}

	if (_timeLimitSecs > 0)
		drawText(_timerStr, Common::Point(_timerValueRect.left, _timerValueRect.bottom));

	// The buttons live in the scene background; only their pressed state is drawn
	if (_shuffleButtonDown)
		_drawSurface.blitFrom(_buttonsImage, _shuffleButtonSrcRect,
		                      Common::Point(_shuffleButtonDestRect.left, _shuffleButtonDestRect.top));
	else if (_doneButtonDown)
		_drawSurface.blitFrom(_buttonsImage, _doneButtonSrcRect,
		                      Common::Point(_doneButtonDestRect.left, _doneButtonDestRect.top));

	_needsRedraw = true;
}

void MatchPuzzle::drawScorePanel() {
	// ---- State 6: final score / high-score screen ----
	if (_gameSubState == kScoreDisplay) {
		if (!_scorePanelImage.empty())
			_drawSurface.blitFrom(_scorePanelImage, Common::Point(0, 0));

		if (_showScoreDisplay) {
			const Graphics::Font *font = g_nancy->_graphics->getFont(_scriptID);
			if (!font) font = g_nancy->_graphics->getFont(0);
			if (font) {
				const int fh = font->getFontHeight();
				const int lineSpacing = fh + 12;

				// Final score
				Common::Rect rect = _hsDisplayRect;
				rect.moveTo(196, 37);
				rect = NancySceneState.getViewport().convertViewportToScreen(rect);
				rect = convertToLocal(rect);
				
				int scoreX = rect.left + 60;
				int hsX = rect.right - 1;
				int scoreY = (rect.bottom - 1) - 16 + lineSpacing;
				font->drawString(&_drawSurface, _scoreStr, scoreX, scoreY, 100, 0);

				// High-score list: entries start one lineSpacing below the final score
				int hsY = scoreY + lineSpacing;
				for (int i = 0; i < 5; ++i) {
					Common::String hs = Common::String::format("%d", _highScores[i].score);
					font->drawString(&_drawSurface, hs, hsX, hsY, 80, 0);
					hsY += lineSpacing;
				}
			}
		}
		return; // don't draw the normal gameplay overlay
	}

	// ---- Normal gameplay ----

	// Draw the shuffle button sprite
	if (!_shuffleButtonSrcRect.isEmpty() && !_shuffleButtonDestRect.isEmpty())
		_drawSurface.blitFrom(_image, _shuffleButtonSrcRect,
		                      Common::Point(_shuffleButtonDestRect.left, _shuffleButtonDestRect.top));

	if (!_showScoreDisplay)
		return;

	// The score-panel font is determined by _scriptID, instead of _scorePanelFontID.
	const Graphics::Font *font = g_nancy->_graphics->getFont(_scriptID);
	if (!font)
		font = g_nancy->_graphics->getFont(0);

	// Helper: vertically centre font within rect (font may be taller than rect)
	const int fh = font->getFontHeight();
	auto textY = [&](const Common::Rect &r) {
		return r.top + (r.height() - fh) / 2 - 1;
	};

	// Static label string (empty in practice — labels are baked into the background image)
	if (!_displayLabelString.empty() && !_labelStringRect.isEmpty())
		font->drawString(&_drawSurface, _displayLabelString,
		                 _labelStringRect.left, textY(_labelStringRect),
		                 _labelStringRect.width(), 0);

	// Goal value (fixed for the lifetime of the puzzle)
	if (!_goalValueRect.isEmpty())
		font->drawString(&_drawSurface, _goalStr,
		                 _goalValueRect.left, textY(_goalValueRect),
		                 _goalValueRect.width(), 0);

	// Current score value
	if (!_scoreValueRect.isEmpty())
		font->drawString(&_drawSurface, _scoreStr,
		                 _scoreValueRect.left, textY(_scoreValueRect),
		                 _scoreValueRect.width(), 0);

	// Countdown timer
	if (_timeLimitSecs > 0 && !_timerValueRect.isEmpty())
		font->drawString(&_drawSurface, _timerStr,
		                 _timerValueRect.left, textY(_timerValueRect),
		                 _timerValueRect.width(), 0);

	// Matched flag info — shown briefly after a match (gated by _execScript)
	if (_execScript && _showFlagName) {
		if (!_flagNameStr.empty() && !_flagNameRect.isEmpty())
			font->drawString(&_drawSurface, _flagNameStr,
			                 _flagNameRect.left, textY(_flagNameRect),
			                 _flagNameRect.width(), 0);

		int16 ft = _matchedFlagType;
		if (ft >= 0 && ft < (int16)_tileSrcRects.size() && !_flagImageRect.isEmpty())
			_drawSurface.blitFrom(_image, _tileSrcRects[ft],
			                      Common::Point(_flagImageRect.left, _flagImageRect.top));
	}
}

void MatchPuzzle::redrawAllCells() {
	_drawSurface.clear(_drawSurface.getTransparentColor());

	if (g_nancy->getGameType() >= kGameTypeNancy14) {
		if (_showHighScores)
			drawHighScoreScreen();
		else
			drawBoardNancy14();

		return;
	}

	drawScorePanel();
	// During state 6 the score-screen covers everything; skip cell drawing
	if (_gameSubState != kScoreDisplay) {
		for (int col = 0; col < _cols; ++col)
			for (int row = 0; row < _rows; ++row)
				drawCell(col, row);
	}
	_needsRedraw = true;
}

} // End of namespace Action
} // End of namespace Nancy
