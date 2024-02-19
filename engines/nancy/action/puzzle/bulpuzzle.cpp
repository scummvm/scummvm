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

#include "engines/nancy/action/puzzle/bulpuzzle.h"

#include "common/random.h"

namespace Nancy {
namespace Action {

void BulPuzzle::init() {
	Common::Rect screenBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(screenBounds.width(), screenBounds.height(), g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(screenBounds);

	g_nancy->_resource->loadImage(_imageName, _image);
	_image.setTransparentColor(_drawSurface.getTransparentColor());

	reset(false);

	for (int i = 0; i < _numPieces - 1; ++i) {
		_drawSurface.blitFrom(_image, _playerBarracksSrc, _playerBarracksDests[i]);
		_drawSurface.blitFrom(_image, _enemyBarracksSrc, _enemyBarracksDests[i]);
	}

	_drawSurface.blitFrom(_image, _playerLightSrc, _playerLightDest);
}

void BulPuzzle::updateGraphics() {
	bool isPlayer = _turn / _numRolls == 0;

	if (_currentAction == kCapture && g_nancy->getTotalPlayTime() > _nextMoveTime) {
		if (g_nancy->_sound->isSoundPlaying(_playerCapturedSound) || g_nancy->_sound->isSoundPlaying(_enemyCapturedSound)) {
			return;
		} else {
			if (isPlayer) {
				--_enemyPieces;
			} else {
				--_playerPieces;
			}

			if (_playerPieces && _enemyPieces) {
				reset(true);
			}
		}
	}

	if (_changeLight && !g_nancy->_sound->isSoundPlaying(_moveSound)) {
		if (_turn == 0) {
			_drawSurface.fillRect(_enemyLightDest, _drawSurface.getTransparentColor());
			_drawSurface.blitFrom(_image, _playerLightSrc, _playerLightDest);
		} else if (_turn == _numRolls) {
			_drawSurface.fillRect(_playerLightDest, _drawSurface.getTransparentColor());
			_drawSurface.blitFrom(_image, _enemyLightSrc, _enemyLightDest);
		}

		if (_turn == 0 || _turn == _numRolls) {
			_drawSurface.blitFrom(_image, _passButtonDisabledSrc, _passButtonDest);
		} else {
			_drawSurface.fillRect(_passButtonDest, _drawSurface.getTransparentColor());
		}

		_changeLight = false;
		_needsRedraw = true;
	}

	if (_nextMoveTime && g_nancy->getTotalPlayTime() > _nextMoveTime) {
		// First, handle buttons
		if (_pushedButton) {
			switch (_currentAction) {
			case kRoll:
				_drawSurface.fillRect(_rollButtonDest, _drawSurface.getTransparentColor());

				// Do the roll logic here since it's more convenient
				for (uint i = 0; i < _diceDestsPlayer.size(); ++i) {
					Common::Rect *dest = isPlayer ? &_diceDestsPlayer[i] : &_diceDestsEnemy[i];
					_drawSurface.fillRect(*dest, _drawSurface.getTransparentColor());
					bool black = g_nancy->_randomSource->getRandomBit();
					if (black) {
						// Black, add one movement
						_drawSurface.blitFrom(_image, _diceBlackSrcs[g_nancy->_randomSource->getRandomNumber(_diceBlackSrcs.size() - 1)], *dest);
						++_moveDiff;
					} else {
						// Non-black, no movement
						_drawSurface.blitFrom(_image, _diceCleanSrcs[g_nancy->_randomSource->getRandomNumber(_diceCleanSrcs.size() - 1)], *dest);
					}
				}

				if (_moveDiff == 0) {
					_moveDiff = 5;
				}

				_nextMoveTime = g_nancy->getTotalPlayTime() + 200;
				break;
			case kPass:
				_drawSurface.fillRect(_passButtonDest, _drawSurface.getTransparentColor());

				if (isPlayer) {
					_drawSurface.fillRect(_playerLightDest, _drawSurface.getTransparentColor());
					_drawSurface.blitFrom(_image, _enemyLightSrc, _enemyLightDest);
					_turn = _numRolls;
				} else {
					_drawSurface.fillRect(_enemyLightDest, _drawSurface.getTransparentColor());
					_drawSurface.blitFrom(_image, _playerLightSrc, _playerLightDest);
					_turn = 0;
				}

				_currentAction = kNone;
				_nextMoveTime = 0;
				break;
			case kReset:
				_drawSurface.fillRect(_resetButtonDest, _drawSurface.getTransparentColor());
				_drawSurface.fillRect(_cellDests[_playerPos], _drawSurface.getTransparentColor());
				_drawSurface.fillRect(_cellDests[_enemyPos], _drawSurface.getTransparentColor());

				for (uint i = 0; i < _playerJailDests.size(); ++i) {
					_drawSurface.fillRect(_playerJailDests[i], _drawSurface.getTransparentColor());
					_drawSurface.fillRect(_enemyJailDests[i], _drawSurface.getTransparentColor());
				}

				break;
			default:
				break;
			}

			_pushedButton = false;
			_needsRedraw = true;
		}

		if (g_nancy->_sound->isSoundPlaying(_rollSound) ||
			g_nancy->_sound->isSoundPlaying(_passSound) ||
			g_nancy->_sound->isSoundPlaying(_resetSound)) {
				return;
		}

		// Now, handle the movement logic
		switch (_currentAction) {
		case kRoll:
			if (_moveDiff) {
				// Moving
				movePiece(isPlayer);
				--_moveDiff;

				if (_moveDiff || _playerPos == _enemyPos) {
					_nextMoveTime = g_nancy->getTotalPlayTime() + 200; // hardcoded
				} else {
					// This was the last move, go to next turn
					g_nancy->_sound->playSound(_moveSound);
					_currentAction = kNone;
					_turn = _turn + 1 > 3 ? 0 : _turn + 1;
					_changeLight = true;
				}
			} else {
				// Capturing
				SoundDescription &sound = isPlayer ? _enemyCapturedSound : _playerCapturedSound;
				g_nancy->_sound->loadSound(sound);
				g_nancy->_sound->playSound(sound);
				_drawSurface.fillRect(_cellDests[_playerPos], _drawSurface.getTransparentColor());
				_drawSurface.blitFrom(_image, isPlayer ? _enemyCapturedSrc : _playerCapturedSrc, _cellDests[_playerPos]);
				_currentAction = kCapture;
				_nextMoveTime = g_nancy->getTotalPlayTime() + 1000;
				_needsRedraw = true;
			}

			return;
		case kPass:
			_currentAction = kNone;
			_turn = (_turn + 1 > _numRolls * 2) ? 0 : _turn + 1;

			return;
		case kReset:
			reset(false);
			return;
		default:
			break;
		}
	}
}

void BulPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);

	_numCells = stream.readUint16LE();
	_numPieces = stream.readUint16LE();
	_numRolls = stream.readUint16LE();
	_playerStart = stream.readUint16LE();
	_enemyStart = stream.readUint16LE();

	readRectArray(stream, _diceDestsPlayer, 4);
	readRectArray(stream, _diceDestsEnemy, 4);

	readRectArray(stream, _cellDests, _numCells, 15);

	readRectArray(stream, _playerBarracksDests, 6);
	readRectArray(stream, _playerJailDests, 6);
	readRectArray(stream, _enemyBarracksDests, 6);
	readRectArray(stream, _enemyJailDests, 6);

	readRect(stream, _rollButtonDest);
	readRect(stream, _passButtonDest);
	readRect(stream, _resetButtonDest);
	readRect(stream, _playerLightDest);
	readRect(stream, _enemyLightDest);

	_diceBlackSrcs.resize(4);
	_diceCleanSrcs.resize(4);
	for (uint i = 0; i < 4; ++i) {
		readRect(stream, _diceCleanSrcs[i]);
		readRect(stream, _diceBlackSrcs[i]);
	}

	readRect(stream, _playerSrc);
	readRect(stream, _enemySrc);
	readRect(stream, _enemyCapturedSrc);
	readRect(stream, _playerCapturedSrc);

	readRect(stream, _playerBarracksSrc);
	readRect(stream, _enemyBarracksSrc);
	readRect(stream, _playerJailSrc);
	readRect(stream, _enemyJailSrc);

	readRect(stream, _rollButtonSrc);
	readRect(stream, _passButtonSrc);
	readRect(stream, _resetButtonSrc);
	readRect(stream, _playerLightSrc);
	readRect(stream, _enemyLightSrc);
	readRect(stream, _passButtonDisabledSrc);

	_moveSound.readNormal(stream);
	_enemyCapturedSound.readNormal(stream);
	_playerCapturedSound.readNormal(stream);
	_rollSound.readNormal(stream);
	_passSound.readNormal(stream);
	_resetSound.readNormal(stream);

	_solveScene.readData(stream);
	_solveSoundDelay = stream.readUint16LE();
	_solveSound.readNormal(stream);

	_exitScene.readData(stream);
	_loseSoundDelay = stream.readUint16LE();
	_loseSound.readNormal(stream);
	readRect(stream, _exitHotspot);
}

void BulPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();

		g_nancy->_sound->loadSound(_rollSound);
		g_nancy->_sound->loadSound(_resetSound);
		g_nancy->_sound->loadSound(_passSound);
		g_nancy->_sound->loadSound(_moveSound);

		_state = kRun;
		// fall through
	case kRun:
		if (_playerPieces == 0) {
			_state = kActionTrigger;
			_nextMoveTime = g_nancy->getTotalPlayTime() + _loseSoundDelay * 1000;
		}

		if (_enemyPieces == 0) {
			_playerWon = true;
			_state = kActionTrigger;
			_nextMoveTime = g_nancy->getTotalPlayTime() + _solveSoundDelay * 1000;
		}

		if (_state == kRun) {
			break;
		}

		// fall through
	case kActionTrigger:
		SoundDescription &sound = _playerWon ? _solveSound : _loseSound;

		if (g_nancy->getTotalPlayTime() >= _nextMoveTime) {
			_nextMoveTime = 0;
			g_nancy->_sound->loadSound(sound);
			g_nancy->_sound->playSound(sound);
		}

		if (_nextMoveTime == 0 && !g_nancy->_sound->isSoundPlaying(sound)) {
			if (_playerWon) {
				_solveScene.execute();
			} else {
				_exitScene.execute();
			}
		}

		break;
	}
}

void BulPuzzle::handleInput(NancyInput &input) {
	if (NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos)) {
		g_nancy->_cursor->setCursorType(g_nancy->_cursor->_puzzleExitCursor);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_state = kActionTrigger;
			_nextMoveTime = 0;
		}

		return;
	}

	if (_pushedButton) {
		return;
	}

	bool canClick = _currentAction == kNone && !g_nancy->_sound->isSoundPlaying(_moveSound);

	if (NancySceneState.getViewport().convertViewportToScreen(_rollButtonDest).contains(input.mousePos)) {
		g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

		if (canClick && input.input & NancyInput::kLeftMouseButtonUp) {
			_drawSurface.blitFrom(_image, _rollButtonSrc, _rollButtonDest);
			_needsRedraw = true;
			g_nancy->_sound->playSound(_rollSound);
			_currentAction = kRoll;
			_pushedButton = true;
			_nextMoveTime = g_nancy->getTotalPlayTime() + 250;
		}

		return;
	}

	if ((_turn % _numRolls) && NancySceneState.getViewport().convertViewportToScreen(_passButtonDest).contains(input.mousePos)) {
		g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

		if (canClick && input.input & NancyInput::kLeftMouseButtonUp) {
			_drawSurface.blitFrom(_image, _passButtonSrc, _passButtonDest);
			_needsRedraw = true;
			g_nancy->_sound->playSound(_passSound);
			_currentAction = kPass;
			_pushedButton = true;
			_nextMoveTime = g_nancy->getTotalPlayTime() + 250;
		}

		return;
	}

	if (NancySceneState.getViewport().convertViewportToScreen(_resetButtonDest).contains(input.mousePos)) {
		g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

		if (canClick && input.input & NancyInput::kLeftMouseButtonUp) {
			_drawSurface.blitFrom(_image, _resetButtonSrc, _resetButtonDest);
			_needsRedraw = true;
			g_nancy->_sound->playSound(_resetSound);
			_currentAction = kReset;
			_pushedButton = true;
			_nextMoveTime = g_nancy->getTotalPlayTime() + 250;
		}

		return;
	}
}

void BulPuzzle::movePiece(bool player) {
	int16 &piecePos = player ? _playerPos : _enemyPos;
	_drawSurface.fillRect(_cellDests[piecePos], _drawSurface.getTransparentColor());
	piecePos += player ? 1 : -1;

	if (ABS<int16>(_playerPos - _enemyPos) == 1) {
		// Redraw other piece in case one piece goes behind the other's back
		_drawSurface.blitFrom(_image, player ? _enemySrc : _playerSrc, _cellDests[player ? _enemyPos : _playerPos]);
	}

	if (piecePos < 0) {
		piecePos = _cellDests.size() - 1;
	} else if (piecePos > (int)_cellDests.size() - 1) {
		piecePos = 0;
	}

	_drawSurface.blitFrom(_image, player ? _playerSrc : _enemySrc, _cellDests[piecePos]);
	_needsRedraw = true;
}

void BulPuzzle::reset(bool capture) {
	_drawSurface.clear(_drawSurface.getTransparentColor());

	// Reset dice
	for (uint i = 0; i < _diceDestsPlayer.size(); ++i) {
		_drawSurface.blitFrom(_image, _diceCleanSrcs[i], _diceDestsPlayer[i]);
		_drawSurface.blitFrom(_image, _diceCleanSrcs[i], _diceDestsEnemy[i]);
	}

	if (!capture) {
		_playerPieces = _enemyPieces = _numPieces;
	}

	// Reset player/enemy
	_playerPos = _playerStart - 1;
	_enemyPos = _enemyStart - 1;
	_drawSurface.blitFrom(_image, _playerSrc, _cellDests[_playerPos]);
	_drawSurface.blitFrom(_image, _enemySrc, _cellDests[_enemyPos]);

	// Reset to player turn
	_turn = 0;
	_drawSurface.blitFrom(_image, _playerLightSrc, _playerLightDest);

	// Draw jail and barracks
	for (int i = 0; i < _numPieces - 1; ++i) {
		if (i < _playerPieces - 1) {
			// Draw piece in barracks
			_drawSurface.blitFrom(_image, _playerBarracksSrc, _playerBarracksDests[i]);
		} else {
			// Draw piece in jail
			_drawSurface.blitFrom(_image, _playerJailSrc, _enemyJailDests[i - _playerPieces + 1]);
		}

		if (i < _enemyPieces - 1) {
			// Draw piece in barracks
			_drawSurface.blitFrom(_image, _enemyBarracksSrc, _enemyBarracksDests[i]);
		} else {
			// Draw piece in jail
			_drawSurface.blitFrom(_image, _enemyJailSrc, _playerJailDests[i - _enemyPieces + 1]);
		}
	}

	// Draw disabled pass button
	_drawSurface.blitFrom(_image, _passButtonDisabledSrc, _passButtonDest);

	_currentAction = kNone;
	_nextMoveTime = 0;
	_pushedButton = false;
	_needsRedraw = true;
}

} // End of namespace Action
} // End of namespace Nancy
