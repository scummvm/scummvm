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

#include "engines/nancy/action/puzzle/bballpuzzle.h"

namespace Nancy {
namespace Action {

void BBallPuzzle::init() {
	Common::Rect screenBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(screenBounds.width(), screenBounds.height(), g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(screenBounds);

	g_nancy->_resource->loadImage(_imageName, _image);
	_image.setTransparentColor(_drawSurface.getTransparentColor());

	// Set up flags
	if (NancySceneState.getEventFlag(_goodShootFlag, g_nancy->_true)) {
		// Last shot entered the hoop
		for (uint i = 0; i < _playerPositionFlags.size(); ++i) {
			if (NancySceneState.getEventFlag(_playerPositionFlags[i], g_nancy->_true)) {
				_curPosition = i;
				break;
			}
		}

		// Unset last position flag
		NancySceneState.setEventFlag(_playerPositionFlags[_curPosition], g_nancy->_false);

		if ((int)_curPosition == _positions - 1) {
			// Beat the game once, reset to initial
			_curPosition = 0;
		} else {
			// In the middle of the game, move to next position
			++_curPosition;
		}

		NancySceneState.setEventFlag(_playerPositionFlags[_curPosition], g_nancy->_true);
	} else {
		// Last shot did not enter the hoop, reset to initial position
		NancySceneState.setEventFlag(_playerPositionFlags[0], g_nancy->_true);

		for (uint i = 1; i < _playerPositionFlags.size(); ++i) {
			NancySceneState.setEventFlag(_playerPositionFlags[i], g_nancy->_false);
		}
	}

	// Reset shot type flags
	for (uint i = 0; i < _badShootFlags.size(); ++i) {
		NancySceneState.setEventFlag(_badShootFlags[i], g_nancy->_false);
	}

	NancySceneState.setEventFlag(_goodShootFlag, g_nancy->_false);

	// Draw the current player position
	if (_curPosition > 0) {
		_drawSurface.blitFrom(_image, _playerSrcs[_curPosition - 1], _playerDest);
	}
}

void BBallPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);

	_positions = stream.readUint16LE();
	_powers = stream.readUint16LE();
	_angles = stream.readUint16LE();

	_correctVals.resize(_positions);
	for (uint i = 0; i < _positions; ++i) {
		_correctVals[i].x = stream.readUint16LE();
		_correctVals[i].y = stream.readUint16LE();
	}

	readRect(stream, _shootButtonDest);
	readRect(stream, _minusButtonDest);
	readRect(stream, _plusButtonDest);

	readRect(stream, _playerDest);
	readRect(stream, _powerDest);
	readRect(stream, _angleDest);

	readRectArray(stream, _angleSliderHotspots, 3);

	readRect(stream, _shootButtonSrc);
	readRect(stream, _minusButtonSrc);
	readRect(stream, _plusButtonSrc);

	readRectArray(stream, _playerSrcs, 3);
	readRectArray(stream, _powerSrcs, 5);
	readRectArray(stream, _anglesSrcs, 2);

	_shootSound.readNormal(stream);
	_minusSound.readNormal(stream);
	_plusSound.readNormal(stream);

	_shootSceneChange.readData(stream);
	stream.skip(2);

	_badShootFlags.resize(3);
	for (uint i = 0; i < 3; ++i) {
		_badShootFlags[i] = stream.readSint16LE();
	}

	_goodShootFlag = stream.readSint16LE();

	_playerPositionFlags.resize(_positions);
	for (uint i = 0; i < _positions; ++i) {
		_playerPositionFlags[i] = stream.readSint16LE();
	}

	_winFlag = stream.readUint16LE();

	_exitScene.readData(stream);
	readRect(stream, _exitHotspot);
}

void BBallPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();

		g_nancy->_sound->loadSound(_plusSound);
		g_nancy->_sound->loadSound(_minusSound);
		g_nancy->_sound->loadSound(_shootSound);

		_state = kRun;
		// fall through
	case kRun:
		if (_pressedButton) {
			if (g_nancy->_sound->isSoundPlaying(_plusSound) || g_nancy->_sound->isSoundPlaying(_minusSound)) {
				return;
			}

			_pressedButton = false;
			_drawSurface.fillRect(_powerDest, _drawSurface.getTransparentColor());
			_drawSurface.fillRect(_plusButtonDest, _drawSurface.getTransparentColor());
			_drawSurface.fillRect(_minusButtonDest, _drawSurface.getTransparentColor());

			if (_curPower > 0) {
				_drawSurface.blitFrom(_image, _powerSrcs[_curPower - 1], _powerDest);
			}

			_needsRedraw = true;
		}

		break;
	case kActionTrigger:
		if (_pressedButton) {
			// Pressed the shoot button
			if (g_nancy->_sound->isSoundPlaying(_shootSound)) {
				return;
			}

			int16 flagToSet = -1;

			if ((int)_curPower == _correctVals[_curPosition].x && (int)_curAngle == _correctVals[_curPosition].y) {
				// Selected correct values
				flagToSet = _goodShootFlag;

				if ((int)_curPosition == _positions - 1) {
					// Last throw, mark puzzle as solved
					NancySceneState.setEventFlag(_winFlag, g_nancy->_true);
				}
			} else if (_curPower == 0) {
				// Low throw
				flagToSet = _badShootFlags[2];
			} else if ((int)_curPower >= _correctVals[_curPosition].x && (int)_curAngle <= _correctVals[_curPosition].y) {
				// High throw
				flagToSet = _badShootFlags[0];
			} else {
				// Mid throw
				flagToSet = _badShootFlags[1];
			}

			NancySceneState.setEventFlag(flagToSet, g_nancy->_true);
			NancySceneState.changeScene(_shootSceneChange);
		} else {
			// Exited the puzzle
			_exitScene.execute();
		}

		g_nancy->_sound->stopSound(_plusSound);
		g_nancy->_sound->stopSound(_minusSound);
		g_nancy->_sound->stopSound(_shootSound);

		finishExecution();
	}
}

void BBallPuzzle::handleInput(NancyInput &input) {
	Common::Point localMousePos = input.mousePos;
	Common::Rect vpPos = NancySceneState.getViewport().getScreenPosition();
	localMousePos -= { vpPos.left, vpPos.top };

	if (_exitHotspot.contains(localMousePos)) {
		g_nancy->_cursor->setCursorType(g_nancy->_cursor->_puzzleExitCursor);

		if (!_pressedButton &&input.input & NancyInput::kLeftMouseButtonUp) {
			_state = kActionTrigger;
		}

		return;
	}

	for (uint i = 0; i < _angleSliderHotspots.size(); ++i) {
		if (_curAngle != i && _angleSliderHotspots[i].contains(localMousePos)) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

			if (!_pressedButton && input.input & NancyInput::kLeftMouseButtonUp) {
				_drawSurface.fillRect(_angleDest, _drawSurface.getTransparentColor());

				if (i > 0) {
					_drawSurface.blitFrom(_image, _anglesSrcs[i - 1], _angleDest);
				}

				_curAngle = i;
				_needsRedraw = true;
			}

			return;
		}
	}

	if (_curPower > 0 && _minusButtonDest.contains(localMousePos)) {
		g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

		if (!_pressedButton && input.input & NancyInput::kLeftMouseButtonUp) {
			--_curPower;
			_drawSurface.blitFrom(_image, _minusButtonSrc, _minusButtonDest);
			g_nancy->_sound->playSound(_minusSound);
			_pressedButton = true;
			_needsRedraw = true;
		}

		return;
	}

	if ((int)_curPower < _powers - 1 && _plusButtonDest.contains(localMousePos)) {
		g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

		if (!_pressedButton && input.input & NancyInput::kLeftMouseButtonUp) {
			++_curPower;
			_drawSurface.blitFrom(_image, _plusButtonSrc, _plusButtonDest);
			g_nancy->_sound->playSound(_plusSound);
			_pressedButton = true;
			_needsRedraw = true;
		}

		return;
	}

	if (_shootButtonDest.contains(localMousePos)) {
		g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

		if (!_pressedButton && input.input & NancyInput::kLeftMouseButtonUp) {
			_drawSurface.blitFrom(_image, _shootButtonSrc, _shootButtonDest);
			g_nancy->_sound->playSound(_shootSound);
			_pressedButton = true;
			_needsRedraw = true;
			_state = kActionTrigger;
		}

		return;
	}
}

} // End of namespace Action
} // End of namespace Nancy
