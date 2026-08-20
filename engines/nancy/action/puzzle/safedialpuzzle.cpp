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
#include "engines/nancy/resource.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/action/puzzle/safedialpuzzle.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Action {

void SafeDialPuzzle::init() {
	g_nancy->_resource->loadImage(_imageName1, _image1);
	g_nancy->_resource->loadImage(_imageName2, _image2);
	g_nancy->_resource->loadImage(_resetImageName, _resetImage);

	Common::Rect screenBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(screenBounds.width(), screenBounds.height(), g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(screenBounds);

	registerGraphics();
}

void SafeDialPuzzle::updateGraphics() {
	if (_animState == kSelect && (_state == kActionTrigger ? _nextAnim - 500 : _nextAnim) < g_nancy->getTotalPlayTime()) {
		_drawSurface.fillRect(_arrowDest, _drawSurface.getTransparentColor());
		_animState = kNone;
		_needsRedraw = true;
	}

	if (_animState == kSpin && _nextAnim < g_nancy->getTotalPlayTime()) {
		drawDialFrame(_current * (1 + _numInbetweens));
		_animState = kNone;
	}

	if (_animState == kReset && _nextAnim < g_nancy->getTotalPlayTime()) {
		if (!_resetImageName.empty()) {
			_animState = kResetAnim;
		} else {
			_animState = kNone;
			_current = 0;
			drawDialFrame(_current);
		}

		g_nancy->_sound->playSound(_resetSound);
	}

	if (_animState == kResetAnim && _nextAnim < g_nancy->getTotalPlayTime()) {
		_drawSurface.blitFrom(_resetImage, _resetDialSrcs[_current % _resetDialSrcs.size()], _dialDest);
		++_current;
		if (_current >= _resetDialSrcs.size() * _resetTurns) {
			_animState = kNone;
			_current = 0;
			drawDialFrame(_current);
		} else {
			// Nancy 12 and up step the animation on a timer, earlier games do so every frame
			_nextAnim = g_nancy->getTotalPlayTime() + (g_nancy->getGameType() >= kGameTypeNancy12 ? 100 : 0); // hardcoded
		}

		_needsRedraw = true;
	}
}

void SafeDialPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName1);
	readFilename(stream, _imageName2);
	if (g_nancy->getGameType() < kGameTypeNancy12) {
		// Nancy 12 dropped the separate reset image from the header
		readFilename(stream, _resetImageName);
	}

	_numInbetweens = (!_imageName2.empty() ? 1 : 0);

	if (g_nancy->getGameType() >= kGameTypeNancy12) {
		// The second image doubles as the one holding the reset spinning frames
		_resetImageName = _imageName2;
	}

	uint16 num = 10;
	if (g_nancy->getGameType() >= kGameTypeNancy4) {
		num = stream.readUint16LE();
		_enableWraparound = stream.readByte();
	}

	readRect(stream, _dialDest);
	readRectArray(stream, _dialSrcs, num * (1 + _numInbetweens), g_nancy->getGameType() >= kGameTypeNancy12 ? 24 : 20);

	readRect(stream, _resetDest);
	readRect(stream, _resetSrc);
	readRect(stream, _arrowDest);
	readRect(stream, _arrowSrc);

	readRectArray(stream, _resetDialSrcs, g_nancy->getGameType() >= kGameTypeNancy12 ? 12 : 10);

	// Trailing empty rects are unused slots, and are skipped while animating
	while (_resetDialSrcs.size() && _resetDialSrcs.back().isEmpty()) {
		_resetDialSrcs.pop_back();
	}

	_resetTurns = stream.readUint16LE();

	uint16 solveSize = stream.readUint16LE();
	_correctSequence.resize(solveSize);
	for (uint i = 0; i < solveSize; ++i) {
		_correctSequence[i] = stream.readUint16LE();
	}
	stream.skip((10 - solveSize) * 2);

	readRect(stream, _ccwHotspot);
	readRect(stream, _cwHotspot);

	if (g_nancy->getGameType() >= kGameTypeNancy4) {
		_useMoveArrows = stream.readByte();
	}

	if (_useMoveArrows) {
		// Swap the two hotspots
		Common::Rect temp = _cwHotspot;
		_cwHotspot = _ccwHotspot;
		_ccwHotspot = temp;
	} else if (g_nancy->getGameType() >= kGameTypeNancy12) {
		// Nancy 12's dial frames are laid out counter-clockwise, so the counter-clockwise
		// hotspot is the one that steps forwards through them
		_ccwTurnsForwards = true;
	}

	_spinSound.readNormal(stream);
	_selectSound.readNormal(stream);
	_resetSound.readNormal(stream);

	_solveScene.readData(stream);
	_solveSoundDelay = stream.readUint16LE();
	_solveSound.readNormal(stream);

	_exitScene.readData(stream);
	readRect(stream, _exitHotspot);
}

void SafeDialPuzzle::execute() {
	switch (_state) {
	case kBegin :
		init();
		g_nancy->_sound->loadSound(_spinSound);
		g_nancy->_sound->loadSound(_selectSound);
		g_nancy->_sound->loadSound(_resetSound);
		_current = 0;
		drawDialFrame(_current);

		NancySceneState.setNoHeldItem();

		_state = kRun;
		// fall through
	case kRun :
		if (!g_nancy->_sound->isSoundPlaying(_selectSound) && g_nancy->getTotalPlayTime() > _nextAnim) {
			if (_playerSequence == _correctSequence) {
				_solved = true;
				_state = kActionTrigger;
				_nextAnim = g_nancy->getTotalPlayTime() + (g_nancy->getGameType() >= kGameTypeNancy12 ? 300 : 1000) * _solveSoundDelay;
			}
		}

		break;
	case kActionTrigger :
		if (_solved) {
			if (_nextAnim == 0) {
				if (g_nancy->_sound->isSoundPlaying(_solveSound)) {
					break;
				}
			} else {
				if (_nextAnim < g_nancy->getTotalPlayTime()) {
					g_nancy->_sound->loadSound(_solveSound);
					g_nancy->_sound->playSound(_solveSound);
					_nextAnim = 0;
				}
				break;
			}

			_solveScene.execute();
		} else {
			_exitScene.execute();
		}

		g_nancy->_sound->stopSound(_solveSound);
		g_nancy->_sound->stopSound(_spinSound);
		g_nancy->_sound->stopSound(_selectSound);
		g_nancy->_sound->stopSound(_resetSound);

		finishExecution();

		break;
	}
}

void SafeDialPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _playerSequence == _correctSequence) {
		return;
	}

	if (NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos)) {
		g_nancy->_cursor->setCursorType(g_nancy->_cursor->_puzzleExitCursor);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_state = kActionTrigger;
		}

		return;
	} else if (NancySceneState.getViewport().convertViewportToScreen(_ccwHotspot).contains(input.mousePos)) {
		if (!canTurnDial(_ccwTurnsForwards)) {
			return;
		}

		g_nancy->_cursor->setCursorType(_useMoveArrows ? CursorManager::kMoveLeft : CursorManager::kDialCCW);

		if (!g_nancy->_sound->isSoundPlaying(_spinSound) && input.input & NancyInput::kLeftMouseButtonUp && _nextAnim < g_nancy->getTotalPlayTime() &&
				_animState != kReset && _animState != kResetAnim) {
			turnDial(_ccwTurnsForwards);
		}

		return;
	} else if (NancySceneState.getViewport().convertViewportToScreen(_cwHotspot).contains(input.mousePos)) {
		if (!canTurnDial(!_ccwTurnsForwards)) {
			return;
		}

		g_nancy->_cursor->setCursorType(_useMoveArrows ? CursorManager::kMoveRight : CursorManager::kDialCW);

		if (!g_nancy->_sound->isSoundPlaying(_spinSound) && input.input & NancyInput::kLeftMouseButtonUp && _nextAnim < g_nancy->getTotalPlayTime() &&
				_animState != kReset && _animState != kResetAnim) {
			turnDial(!_ccwTurnsForwards);
		}

		return;
	}

	if (g_nancy->_sound->isSoundPlaying(_selectSound) || _animState == kReset || _animState == kResetAnim || _nextAnim > g_nancy->getTotalPlayTime()) {
		return;
	}

	// The puzzle arrow cursor only exists in Nancy 10 and up
	CursorManager::CursorType buttonCursor = g_nancy->getGameType() >= kGameTypeNancy10 ?
		CursorManager::kPuzzleArrow : CursorManager::kHotspot;

	if (NancySceneState.getViewport().convertViewportToScreen(_arrowDest).contains(input.mousePos)) {
		g_nancy->_cursor->setCursorType(buttonCursor);

		if (!g_nancy->_sound->isSoundPlaying(_selectSound) && input.input & NancyInput::kLeftMouseButtonUp) {
			g_nancy->_sound->playSound(_selectSound);
			pushSequence(_current);
			_drawSurface.blitFrom(_image1, _arrowSrc, _arrowDest);
			_animState = kSelect;
			_nextAnim = g_nancy->getTotalPlayTime() + 500; // hardcoded
			_needsRedraw = true;
		}

		return;
	} else if (NancySceneState.getViewport().convertViewportToScreen(_resetDest).contains(input.mousePos)) {
		g_nancy->_cursor->setCursorType(buttonCursor);

		if (!g_nancy->_sound->isSoundPlaying(_resetSound) && input.input & NancyInput::kLeftMouseButtonUp) {
			if (!_resetSrc.isEmpty()) {
				_drawSurface.blitFrom(_image1, _resetSrc, _resetDest);
			}

			g_nancy->_sound->playSound(_selectSound);
			_animState = kReset;
			_nextAnim = g_nancy->getTotalPlayTime() + 500; // hardcoded
			_current = 0;
			_playerSequence.clear();
			_needsRedraw = true;
		}

		return;
	}
}

bool SafeDialPuzzle::canTurnDial(bool forwards) const {
	if (_enableWraparound) {
		return true;
	}

	return forwards ? _current != numDialPositions() - 1 : _current != 0;
}

void SafeDialPuzzle::turnDial(bool forwards) {
	// The in-between frame of a step always belongs to the lower of the two positions
	if (forwards) {
		drawDialFrame(_current * (1 + _numInbetweens) + 1);
		_current = (_current == numDialPositions() - 1) ? 0 : _current + 1;
	} else {
		_current = (_current == 0) ? numDialPositions() - 1 : _current - 1;
		drawDialFrame(_current * (1 + _numInbetweens) + (_numInbetweens ? 1 : 0));
	}

	_nextAnim = g_nancy->getTotalPlayTime() + (g_nancy->getGameType() == kGameTypeNancy3 ? 250 : 500); // hardcoded

	g_nancy->_sound->playSound(_spinSound);
	_animState = kSpin;
}

void SafeDialPuzzle::drawDialFrame(uint frame) {
	if (frame >= _dialSrcs.size() / 2 && !_imageName2.empty()) {
		_drawSurface.blitFrom(_image2, _dialSrcs[frame], _dialDest);
	} else {
		_drawSurface.blitFrom(_image1, _dialSrcs[frame], _dialDest);
	}

	_needsRedraw = true;
}

void SafeDialPuzzle::pushSequence(uint id) {
	if (!_useMoveArrows && id != 0 && g_nancy->getGameType() < kGameTypeNancy12) {
		// When the puzzle is set to use rotation cursors, the ids in the correct sequence are in reverse order.
		// Nancy 12 stores the dial positions themselves.
		id = numDialPositions() - id;
	}

	_playerSequence.push_back(id);
	if (_playerSequence.size() > _correctSequence.size()) {
		_playerSequence.erase(_playerSequence.begin());
	}
}

} // End of namespace Action
} // End of namespace Nancy
