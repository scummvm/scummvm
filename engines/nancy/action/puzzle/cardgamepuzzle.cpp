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
	for (int i = 0; i < kMaxCols; ++i)             // 0xc2b AI per-column match table (-> 0xdd8)
		readFilename(stream, _matchVoice[0][i]);
	stream.skip(0xee0 - 0xdd8);                    // side-0 no-move / made-move lines
	readFilename(stream, _enemyScoredVoiceName);   // 0xee0
	stream.skip(0xf22 - 0xf01);                    // alt variant
	readFilename(stream, _endVoiceName[0]);        // 0xf22 (AI wins)
	stream.skip(0xf68 - 0xf43);                    // remaining side-0 lines
	for (int i = 0; i < kMaxCols; ++i)             // 0xf68 player per-column match table (-> 0x1115)
		readFilename(stream, _matchVoice[1][i]);
	stream.skip(0x121d - 0x1115);                  // side-1 no-move / made-move lines
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

// One Go Fish "ask": the side asks the opponent for the given rank. If the opponent holds any cards
// of it, they all transfer (owning three scores a set) and the side asks again. Otherwise the side
// draws a card from the deck; the turn then passes, unless the draw-again rule applies and it drew
// the rank it asked for. Ends the game if the deck runs out on a draw.
bool CardGamePuzzle::askForColumn(int side, int askedCol) {
	int opponent = side ^ 1;
	bool took = false;

	for (int row = 0; row < _numRows; ++row) {
		if (_board[opponent].grid[row][askedCol] == 1) {
			_board[opponent].grid[row][askedCol] = 0;
			_board[opponent].colCount[askedCol]--;
			_board[side].grid[row][askedCol] = 1;
			_board[side].colCount[askedCol]++;
			took = true;
		}
	}

	if (took) {
		if (_board[side].colCount[askedCol] >= 3 && !_board[side].colComplete[askedCol]) {
			_board[side].colComplete[askedCol] = 1;
			_board[side].score++;
			playVoice(side == 1 ? _playerScoredVoiceName : _enemyScoredVoiceName);
		} else if (askedCol < kMaxCols) {
			// A rank-specific line for the taken cards (silent in scenes that don't use them)
			playVoice(_matchVoice[side][askedCol]);
		}

		return true; // the opponent had the rank: ask again
	}

	// Go fish
	int drawnCol = dealOne(side);
	if (drawnCol == -1) {
		endGame();
		return false;
	}

	return (_switchTurnRule != 0 && drawnCol == askedCol);
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

// The AI (side 0) takes its whole turn at once: it keeps asking as long as each ask grants another
// turn, then control returns to the player. The game ends once a draw finds the deck exhausted.
void CardGamePuzzle::runAiTurn() {
	_lastAiColumn = -1;

	for (int guard = 0; !_gameOver && guard < 1000; ++guard) {
		int col = aiPickColumn();
		if (col == -1) {
			// Nothing to ask for: go fish and end the turn.
			if (dealOne(0) == -1) {
				endGame();
			}
			break;
		}

		bool goAgain = askForColumn(0, col);
		_lastAiColumn = col;
		if (!goAgain) {
			break;
		}
	}
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

	// Once the final move has settled and the winner's line has had its moment, transition out.
	if (_gameOver && _awaitingEnd && g_nancy->getTotalPlayTime() >= _endWaitUntil) {
		_awaitingEnd = false;
		_state = kActionTrigger;
		return;
	}

	// If it becomes the player's turn but they hold no rank they can ask for, they must go fish and
	// pass, so the AI takes over. This resolves a turn the player could otherwise never act on.
	if (_state == kRun && !_gameOver && !_awaitingEnd && _currentTurn == 1 && !hasPlayableColumn(1)) {
		if (dealOne(1) == -1) {
			endGame();
		} else if (!_gameOver) {
			runAiTurn();
		}

		drawBoard();
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
	_currentTurn = _startPlayer;
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

		// When the AI is dealt the opening move, it plays out its turn before the player's first ask.
		if (_startPlayer == 0 && !_gameOver) {
			runAiTurn();
			_currentTurn = 1;
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

	if (_gameOver || _animating || _currentTurn != 1) {
		return;
	}

	int col = columnUnderMouse(input.mousePos);
	if (col == -1) {
		return;
	}

	g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

	if (input.input & NancyInput::kLeftMouseButtonUp) {
		bool before[kMaxRows][kMaxCols];
		for (int r = 0; r < kMaxRows; ++r)
			for (int c = 0; c < kMaxCols; ++c)
				before[r][c] = (_board[1].grid[r][c] == 1);

		playVoice(_moveVoiceName);
		bool goAgain = askForColumn(1, col);

		// A failed ask (go fish) ends the player's turn, so the AI plays out its whole turn before
		// control returns. A successful ask keeps the turn: the player just asks again.
		if (_state == kRun && !_gameOver && !goAgain) {
			runAiTurn();
		}

		if (_state == kRun && !_gameOver) {
			startMoveAnimation(before);
		} else {
			drawBoard();
		}
	}
}

} // End of namespace Action
} // End of namespace Nancy
