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
#include "engines/nancy/cursor.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/input.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/cardgamepuzzle.h"

namespace Nancy {
namespace Action {

void CardGamePuzzle::readData(Common::SeekableReadStream &stream) {
	// The whole record is 0x1346 bytes. The loader copies it verbatim and the engine reads fields
	// at fixed offsets, so the layout is identical across scenes; the comments give those offsets.
	readFilename(stream, _imageName);  // 0x000

	_unknown21      = stream.readByte();      // 0x021
	_switchTurnRule = stream.readByte();      // 0x022
	_startPlayer    = stream.readByte();      // 0x023
	_dealMode       = stream.readByte();      // 0x024
	_numCols        = stream.readUint16LE();  // 0x025
	_numRows        = stream.readUint16LE();  // 0x027
	_dealRounds     = stream.readUint16LE();  // 0x029

	readRect(stream, _turnHighlightSrc[0]);   // 0x02b
	readRect(stream, _turnHighlightSrc[1]);
	readRect(stream, _turnHighlightDest[0]);  // 0x04b
	readRect(stream, _turnHighlightDest[1]);

	_faceUpSrc.resize(4 * kMaxCols);          // 0x06b: 4 rows of 13 card src rects
	for (uint i = 0; i < _faceUpSrc.size(); ++i)
		readRect(stream, _faceUpSrc[i]);

	stream.skip(0x47b - 0x3ab);               // 0x3ab: score-digit src rects (10) + padding
	readRect(stream, _suitScoreSrc);          // 0x47b

	_scoreDest.resize(2 * kMaxCols);          // 0x48b: 2 sides of 13 score-tile positions
	for (uint i = 0; i < _scoreDest.size(); ++i)
		readRect(stream, _scoreDest[i]);

	_moveAnimSteps = stream.readUint16LE();   // 0x62b
	_moveAnimDelta = stream.readUint16LE();   // 0x62d
	_moveAnimDelay = stream.readUint32LE();   // 0x62f

	readRect(stream, _cardDisplayDest[0]);    // 0x633
	readRect(stream, _cardDisplayDest[1]);

	_columnButtons.resize(kMaxCols);          // 0x653: one play button under each column
	for (uint i = 0; i < _columnButtons.size(); ++i)
		readRect(stream, _columnButtons[i]);

	_faceDownSrc.resize(3 * kMaxCols);        // 0x723: 3 rows of 13 card-back src rects
	for (uint i = 0; i < _faceDownSrc.size(); ++i)
		readRect(stream, _faceDownSrc[i]);

	// 0x993: deal animation params + frames, then the voiced-line block (0xba3..0x1304, 33-byte
	// name slots, per-side blocks 0x33d apart). Pull out the few lines we play; skip the rest.
	stream.skip(0xbc4 - 0x993);                    // deal animation params + frames
	readFilename(stream, _moveVoiceName);          // 0xbc4
	readFilename(stream, _dealVoiceName);          // 0xbe5
	stream.skip(0xc2b - 0xc06);                    // 0xc06 deal SFX (alt)
	for (int i = 0; i < kMaxCols; ++i)             // 0xc2b AI per-column ask table (-> 0xdd8)
		readFilename(stream, _matchVoice[0][i]);
	readFilename(stream, _noMoveVoice[1]);         // 0xdd8 player "go fish"
	stream.skip(0xe7d - 0xdf9);
	readFilename(stream, _madeMoveVoice[1]);       // 0xe7d player "here you go"
	stream.skip(0xee0 - 0xe9e);
	readFilename(stream, _enemyScoredVoiceName);   // 0xee0
	stream.skip(0xf22 - 0xf01);                    // alt variant
	readFilename(stream, _endVoiceName[0]);        // 0xf22 (AI wins)
	stream.skip(0xf68 - 0xf43);                    // remaining side-0 lines
	for (int i = 0; i < kMaxCols; ++i)             // 0xf68 player per-column ask table (-> 0x1115)
		readFilename(stream, _matchVoice[1][i]);
	readFilename(stream, _noMoveVoice[0]);         // 0x1115 AI "go fish"
	stream.skip(0x11ba - 0x1136);
	readFilename(stream, _madeMoveVoice[0]);       // 0x11ba AI "here you go"
	stream.skip(0x121d - 0x11db);
	readFilename(stream, _playerScoredVoiceName);  // 0x121d (= 0xee0 + 0x33d)
	stream.skip(0x125f - 0x123e);                  // alt variant
	readFilename(stream, _endVoiceName[1]);        // 0x125f (player wins, = 0xf22 + 0x33d)
	stream.skip(0x1304 - 0x1280);                  // remaining voice block

	_winSceneStartPlayer = stream.readUint16LE();        // 0x1304
	_winSceneStartEnemy  = stream.readUint16LE();        // 0x1306
	_winScene.frameID          = stream.readUint16LE();  // 0x1308
	_winScene.verticalOffset   = stream.readUint16LE();  // 0x130a
	_winScene.continueSceneSound = stream.readUint16LE();// 0x130c
	stream.skip(0x131c - 0x130e);                        // listener vector + frame id
	_winFlagPlayer = stream.readSint16LE();              // 0x131c
	_winFlagEnemy  = stream.readSint16LE();              // 0x131e
	_exitScene     = stream.readUint16LE();              // 0x1320
	_exitSceneChange.frameID          = stream.readUint16LE(); // 0x1322
	_exitSceneChange.verticalOffset   = stream.readUint16LE(); // 0x1324
	_exitSceneChange.continueSceneSound = stream.readUint16LE();// 0x1326
	stream.skip(0x1336 - 0x1328);                        // listener vector + flag

	readRect(stream, _exitHotspot);                      // 0x1336 (ends at 0x1346)
}

// Deal a single card to the given side: pick a random still-available cell from the shared deck
// and place it in that side's column, capping the column. While more than a row's worth of cards
// remain on the table, columns are kept below 3 so the opening deal stays spread out.
int CardGamePuzzle::dealOne(int player) {
	if (_deckRemaining < 1) {
		return -1;
	}

	bool allowFull = _deckRemaining <= _numCols;

	for (int attempts = 0; attempts < 10000; ++attempts) {
		int row = g_nancy->_randomSource->getRandomNumber(_numRows - 1);
		int col = g_nancy->_randomSource->getRandomNumber(_numCols - 1);

		if (_availMap[row][col] && (allowFull || _board[player].colCount[col] + 1 < 3)) {
			_board[player].grid[row][col] = 1;
			_board[player].colCount[col]++;
			if (_board[player].colCount[col] >= 3 && !_board[player].colComplete[col]) {
				_board[player].colComplete[col] = 1;
				_board[player].score++;
				playVoice(player == 1 ? _playerScoredVoiceName : _enemyScoredVoiceName);
			}
			_availMap[row][col] = 0;
			--_deckRemaining;
			return col;
		}
	}

	return -1;
}

void CardGamePuzzle::drawBoard() {
	_drawSurface.clear(_drawSurface.getTransparentColor());

	// The visible tableau is side 1's grid; a present card is drawn face up, an empty cell shows
	// the scene background (which carries the card backs). Cards mid-slide are drawn offset.
	for (int row = 0; row < _numRows; ++row) {
		for (int col = 0; col < _numCols; ++col) {
			if (_board[1].grid[row][col] == 1) {
				const Common::Rect &src = _faceUpSrc[row * kMaxCols + col];
				const Common::Rect &dest = _faceDownSrc[row * kMaxCols + col];
				int yOff = (_animating && _appearing[row][col]) ? _animStep * kSlidePerStep : 0;
				_drawSurface.blitFrom(_image, src, Common::Point(dest.left, dest.top + yOff));
			}
		}
	}

	// Cards that just left side 1's grid slide downward out of their cell.
	if (_animating) {
		for (int row = 0; row < _numRows; ++row) {
			for (int col = 0; col < _numCols; ++col) {
				if (_leaving[row][col]) {
					const Common::Rect &src = _faceUpSrc[row * kMaxCols + col];
					const Common::Rect &dest = _faceDownSrc[row * kMaxCols + col];
					int yOff = (_moveAnimSteps - _animStep) * kSlidePerStep;
					_drawSurface.blitFrom(_image, src, Common::Point(dest.left, dest.top + yOff));
				}
			}
		}
	}

	// A completed column shows a suit-score tile on the owning side.
	for (int side = 0; side < 2; ++side) {
		for (int col = 0; col < _numCols; ++col) {
			if (_board[side].colComplete[col]) {
				const Common::Rect &dest = _scoreDest[side * kMaxCols + col];
				_drawSurface.blitFrom(_image, _suitScoreSrc, Common::Point(dest.left, dest.top));
			}
		}
	}

	// Current-turn indicator
	if (_currentTurn == 0 || _currentTurn == 1) {
		_drawSurface.blitFrom(_image, _turnHighlightSrc[_currentTurn],
			Common::Point(_turnHighlightDest[_currentTurn].left, _turnHighlightDest[_currentTurn].top));
	}

	_needsRedraw = true;
}

bool CardGamePuzzle::hasPlayableColumn(int side) const {
	for (int col = 0; col < (int)_numCols; ++col) {
		if (_board[side].colCount[col] > 0 && _board[side].colCount[col] < 3) {
			return true;
		}
	}

	return false;
}

// Transfer every card the opponent holds of the asked rank to the asking side.
bool CardGamePuzzle::takeCards(int side, int col) {
	int opponent = side ^ 1;
	bool moved = false;

	for (int row = 0; row < _numRows; ++row) {
		if (_board[opponent].grid[row][col] == 1) {
			_board[opponent].grid[row][col] = 0;
			_board[opponent].colCount[col]--;
			_board[side].grid[row][col] = 1;
			_board[side].colCount[col]++;
			moved = true;
		}
	}

	return moved;
}

// Start an ask: the mover asks the opponent for a rank. Play its "do you have any X?" line; the
// resolution waits until that line finishes (see updateGraphics/resolveAsk).
void CardGamePuzzle::beginAsk(int side, int col) {
	_mover = side;
	_currentTurn = side;
	_askedCol = col;
	playVoice(_matchVoice[side][col]);
	_phase = kAskSound;
	drawBoard();
}

// The ask voice has finished: hand the cards over (or go fish), play the answer, and wait for it and
// the card-slide to finish before the turn advances.
void CardGamePuzzle::resolveAsk() {
	bool before[kMaxRows][kMaxCols];
	for (int r = 0; r < kMaxRows; ++r)
		for (int c = 0; c < kMaxCols; ++c)
			before[r][c] = (_board[1].grid[r][c] == 1);

	if (takeCards(_mover, _askedCol)) {
		if (_board[_mover].colCount[_askedCol] >= 3 && !_board[_mover].colComplete[_askedCol]) {
			_board[_mover].colComplete[_askedCol] = 1;
			_board[_mover].score++;
			playVoice(_mover == 1 ? _playerScoredVoiceName : _enemyScoredVoiceName);
		} else {
			playVoice(_madeMoveVoice[_mover]); // "here you go"
		}

		_goAgain = true; // the opponent had the rank: ask again
	} else {
		playVoice(_noMoveVoice[_mover]); // "go fish"
		int drawnCol = dealOne(_mover);
		_goAgain = (drawnCol != -1 && _switchTurnRule != 0 && drawnCol == _askedCol);
		if (drawnCol == -1) {
			endGame();
		}
	}

	startMoveAnimation(before);
	_phase = kAnswerSound;
}

// The answer has finished: the mover asks again (a take, or a draw-again), or the turn passes.
void CardGamePuzzle::advanceTurn() {
	int next = _goAgain ? _mover : (_mover ^ 1);
	if (next == 1) {
		startPlayerTurn();
	} else {
		_aiDelayUntil = g_nancy->getTotalPlayTime() + 700;
		_phase = kAiDelay;
	}
}

// Begin the player's turn. If the player holds no rank they can ask for, they are forced to go fish
// and the turn passes to the AI.
void CardGamePuzzle::startPlayerTurn() {
	_mover = 1;
	_currentTurn = 1;

	if (!hasPlayableColumn(1)) {
		if (dealOne(1) == -1) {
			endGame();
			return;
		}

		_aiDelayUntil = g_nancy->getTotalPlayTime() + 700;
		_phase = kAiDelay;
	} else {
		_phase = kWaitInput;
	}

	drawBoard();
}

// The AI picks a rank it is building and asks for it; with nothing to ask, it goes fish and passes.
void CardGamePuzzle::startAiAsk() {
	int col = aiPickColumn();
	if (col == -1) {
		if (dealOne(0) == -1) {
			endGame();
		} else {
			startPlayerTurn();
		}
		return;
	}

	_lastAiColumn = col;
	beginAsk(0, col);
}

// The AI asks for a rank it already holds one or two of, preferring ranks where it holds two (so a
// take completes the set) and avoiding immediately repeating its previous rank. Returns -1 if it
// holds no incomplete rank to ask for.
int CardGamePuzzle::aiPickColumn() {
	Common::Array<int> strong, any;
	for (int col = 0; col < (int)_numCols; ++col) {
		int count = _board[0].colCount[col];
		if (count <= 0 || count >= 3 || col == _lastAiColumn) {
			continue;
		}

		any.push_back(col);
		if (count >= 2) {
			strong.push_back(col);
		}
	}

	const Common::Array<int> &pool = !strong.empty() ? strong : any;
	if (!pool.empty()) {
		return pool[g_nancy->_randomSource->getRandomNumber(pool.size() - 1)];
	}

	// The last-asked rank is the only one left: allow repeating it.
	for (int col = 0; col < (int)_numCols; ++col) {
		if (_board[0].colCount[col] > 0 && _board[0].colCount[col] < 3) {
			return col;
		}
	}

	return -1;
}

void CardGamePuzzle::endGame() {
	_gameOver = true;

	// Play the winner's end-of-game line, then hold briefly before the result scene (updateGraphics
	// fires the transition once the wait elapses).
	int winner = (_board[1].score >= _board[0].score) ? 1 : 0;
	playVoice(_endVoiceName[winner]);
	bool hasVoice = !_endVoiceName[winner].empty() && _endVoiceName[winner] != "NO SOUND";
	_awaitingEnd = true;
	_endWaitUntil = g_nancy->getTotalPlayTime() + (hasVoice ? 3000 : 500);

	drawBoard();
}

void CardGamePuzzle::startMoveAnimation(const bool beforeGrid[kMaxRows][kMaxCols]) {
	_animating = false;
	for (int row = 0; row < kMaxRows; ++row) {
		for (int col = 0; col < kMaxCols; ++col) {
			bool now = (_board[1].grid[row][col] == 1);
			_appearing[row][col] = (!beforeGrid[row][col] && now);
			_leaving[row][col] = (beforeGrid[row][col] && !now);
			if (_appearing[row][col] || _leaving[row][col]) {
				_animating = true;
			}
		}
	}

	if (_animating) {
		_animStep = MAX<int>(_moveAnimSteps, 1);
		_animNextStep = g_nancy->getTotalPlayTime() + _moveAnimDelay;
	}

	drawBoard();
}

void CardGamePuzzle::updateGraphics() {
	// Step any in-progress card slide first; the turn stays parked until it finishes.
	if (_animating) {
		if (g_nancy->getTotalPlayTime() < _animNextStep) {
			return;
		}

		--_animStep;
		_animNextStep = g_nancy->getTotalPlayTime() + _moveAnimDelay;

		if (_animStep <= 0) {
			_animating = false;
		}

		drawBoard();
		return;
	}

	// Once the final move has settled and the winner's line has finished, transition out.
	if (_gameOver) {
		if (_awaitingEnd && g_nancy->getTotalPlayTime() >= _endWaitUntil &&
				!g_nancy->_sound->isSoundPlaying(_voiceSound)) {
			_awaitingEnd = false;
			_state = kActionTrigger;
		}
		return;
	}

	switch (_phase) {
	case kAskSound:
		// The "do you have any X?" line has played; hand the cards over (or go fish).
		if (!g_nancy->_sound->isSoundPlaying(_voiceSound)) {
			resolveAsk();
		}
		break;
	case kAnswerSound:
		// The answer line has played (and the cards have slid); take the next turn.
		if (!g_nancy->_sound->isSoundPlaying(_voiceSound)) {
			advanceTurn();
		}
		break;
	case kAiDelay:
		if (g_nancy->getTotalPlayTime() >= _aiDelayUntil) {
			startAiAsk();
		}
		break;
	default:
		break;
	}
}

void CardGamePuzzle::playVoice(const Common::String &name) {
	if (name.empty() || name == "NO SOUND") {
		return;
	}

	g_nancy->_sound->stopSound(_voiceSound);
	_voiceSound.name = name;
	_voiceSound.channelID = 8;
	_voiceSound.numLoops = 1;
	_voiceSound.volume = 85;
	g_nancy->_sound->loadSound(_voiceSound);
	g_nancy->_sound->playSound(_voiceSound);

	showSubtitle(name);
}

// The card-game lines carry no inline caption; the original looks the subtitle up by sound name in
// the Autotext table. Mirror that and push it to the textbox (as QuizPuzzle does).
void CardGamePuzzle::showSubtitle(const Common::String &soundName) {
	const CVTX *autotext = (const CVTX *)g_nancy->getEngineData("AUTOTEXT");
	if (!autotext) {
		return;
	}

	Common::String text = autotext->texts.getValOrDefault(soundName, "");
	if (!text.empty()) {
		NancySceneState.getTextbox().clear();
		NancySceneState.getTextbox().addTextLine(text);
	}
}

void CardGamePuzzle::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(vpBounds.width(), vpBounds.height(), g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(vpBounds);

	g_nancy->_resource->loadImage(_imageName, _image);
	_image.setTransparentColor(_drawSurface.getTransparentColor());

	// Reset board state and the shared deck (every table cell starts available)
	for (int side = 0; side < 2; ++side) {
		_board[side] = PlayerBoard();
	}
	for (int row = 0; row < kMaxRows; ++row)
		for (int col = 0; col < kMaxCols; ++col)
			_availMap[row][col] = (row < _numRows && col < _numCols) ? 1 : 0;

	_deckRemaining = _numCols * _numRows;
	_mover = _startPlayer;
	_currentTurn = _startPlayer;
	_askedCol = -1;
	_goAgain = false;
	_phase = kWaitInput;
	_lastAiColumn = -1;
	_gameOver = false;
	_gaveUp = false;
	_animating = false;
	_animStep = 0;
	_awaitingEnd = false;

	// Opening deal: _dealRounds cards to each side, alternating
	for (int round = 0; round < _dealRounds; ++round) {
		dealOne(0);
		dealOne(1);
	}

	RenderObject::init();
}

void CardGamePuzzle::execute() {
	if (_state == kBegin) {
		init();
		registerGraphics();

		// Kick off the first turn. When the AI is dealt the opening move, it asks first.
		if (_startPlayer == 0) {
			_mover = 0;
			_currentTurn = 0;
			_aiDelayUntil = g_nancy->getTotalPlayTime() + 700;
			_phase = kAiDelay;
		} else {
			startPlayerTurn();
		}

		drawBoard();
		_state = kRun;
	} else if (_state == kActionTrigger) {
		g_nancy->_sound->stopSound(_voiceSound);

		SceneChangeDescription sceneChange;

		if (_gaveUp) {
			sceneChange = _exitSceneChange;
			sceneChange.sceneID = _exitScene;
		} else {
			sceneChange = _winScene;

			const int playerScore = _board[1].score;
			const int aiScore = _board[0].score;
			const bool tie = (playerScore == aiScore);

			// The result normally uses the first outcome scene; the second is the tie/alternate
			// path the 0xff deal mode takes. 9999 marks an absent scene.
			if (tie && _dealMode == 0xff && _winSceneStartEnemy != 9999) {
				sceneChange.sceneID = _winSceneStartEnemy;
			} else {
				sceneChange.sceneID = (_winSceneStartPlayer != 9999) ? _winSceneStartPlayer : _winSceneStartEnemy;
			}

			// Record the outcome: the player-win flag when ahead, else the alternate flag on a tie.
			if (playerScore > aiScore && _winFlagPlayer != -1) {
				FlagDescription flag;
				flag.label = _winFlagPlayer;
				flag.flag = g_nancy->_true;
				NancySceneState.setEventFlag(flag);
			} else if (tie && _winFlagEnemy != -1) {
				FlagDescription flag;
				flag.label = _winFlagEnemy;
				flag.flag = g_nancy->_true;
				NancySceneState.setEventFlag(flag);
			}
		}

		NancySceneState.changeScene(sceneChange);
		finishExecution();
	}

	// The deal/match slide animations and the voiced lines come in a later stage.
}

// A column is playable while the player (side 1) owns one or two cards in it - a column they are
// building but have not yet completed. Depending on the scene the click target is either the bottom
// button row or the player's own cards in the tableau.
int CardGamePuzzle::columnUnderMouse(const Common::Point &mousePos) const {
	if (usesColumnButtons()) {
		for (int col = 0; col < (int)_numCols; ++col) {
			if (_board[1].colCount[col] <= 0 || _board[1].colCount[col] >= 3) {
				continue;
			}

			Common::Rect button = NancySceneState.getViewport().convertViewportToScreen(_columnButtons[col]);
			if (button.contains(mousePos)) {
				return col;
			}
		}
	} else {
		for (int row = 0; row < _numRows; ++row) {
			for (int col = 0; col < (int)_numCols; ++col) {
				if (_board[1].grid[row][col] != 1 || _board[1].colCount[col] >= 3) {
					continue;
				}

				Common::Rect card = NancySceneState.getViewport().convertViewportToScreen(_faceDownSrc[row * kMaxCols + col]);
				if (card.contains(mousePos)) {
					return col;
				}
			}
		}
	}

	return -1;
}

void CardGamePuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _awaitingEnd) {
		return;
	}

	// Exit hotspot is always available; leaving this way is "giving up" (goes to the exit scene)
	if (NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos)) {
		g_nancy->_cursor->setCursorType(g_nancy->_cursor->_puzzleExitCursor);
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_gaveUp = true;
			_state = kActionTrigger;
		}
		return;
	}

	// Clicks are only accepted while waiting for the player to ask (not mid-ask, mid-answer, or during
	// the AI's turn).
	if (_gameOver || _phase != kWaitInput) {
		return;
	}

	int col = columnUnderMouse(input.mousePos);
	if (col == -1) {
		return;
	}

	g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

	if (input.input & NancyInput::kLeftMouseButtonUp) {
		beginAsk(1, col);
	}
}

} // End of namespace Action
} // End of namespace Nancy
