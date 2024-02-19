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

#include "engines/nancy/util.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/puzzledata.h"
#include "engines/nancy/state/scene.h"

#include "engines/nancy/action/puzzle/spigotpuzzle.h"

namespace Nancy {
namespace Action {

void SpigotPuzzle::init() {
	Common::Rect screenBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(screenBounds.width(), screenBounds.height(), g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(screenBounds);

	g_nancy->_resource->loadImage(_imageName, _image);
	registerGraphics();
}

void SpigotPuzzle::updateGraphics() {
	if (_pushedButtonID != -1) {
		if (g_nancy->getTotalPlayTime() >= _nextAnimTime) {
			_animatingLetterID = _pushedButtonID;
			_drawSurface.fillRect(_buttonDests[_pushedButtonID], _drawSurface.getTransparentColor());
			_pushedButtonID = -1;
			_nextAnimTime = 0;

			uint numSpins = _numSpins[_numPulls[_animatingLetterID] - 1];
			_frameID = numSpins * _numInbetweens;
		}
	}

	if (_animatingSpigotID != -1) {
		uint32 curTime = g_nancy->getTotalPlayTime();
		if (curTime >= _nextAnimTime) {
			if (_nextAnimTime == 0) {
				_nextAnimTime = curTime + 100;
			} else {
				_nextAnimTime += 100;
			}
		} else {
			return;
		}

		if (_frameID != _spigotAnimSrcs[_animatingSpigotID].size()) {
			_drawSurface.blitFrom(_image, _spigotAnimSrcs[_animatingSpigotID][_frameID], _spigotDests[_animatingSpigotID]);
			_needsRedraw = true;

			++_frameID;
		} else {
			// Increment the number and end the animation
			_numPulls[_animatingSpigotID] = MIN<uint16>(_numPulls[_animatingSpigotID] + 1, 6);
			_drawSurface.blitFrom(_image, _digitSrcs[_animatingSpigotID][_numPulls[_animatingSpigotID]], _digitDests[_animatingSpigotID]);

			// Also, clear the last drawn spigot frame
			_drawSurface.fillRect(_spigotDests[_animatingSpigotID], _drawSurface.getTransparentColor());

			_needsRedraw = true;
			_animatingSpigotID = -1;
			_frameID = 0;
			_nextAnimTime = 0;
		}
	} else if (_animatingLetterID != -1) {
		uint32 curTime = g_nancy->getTotalPlayTime();
		if (curTime >= _nextAnimTime) {
			if (_nextAnimTime == 0) {
				_nextAnimTime = curTime + _letterTime * 200;
			} else {
				_nextAnimTime += _letterTime * 200;
			}
		} else {
			return;
		}

		if (_frameID != 0) {
			if (++_currentAnimOrder[_animatingLetterID] >= _numLetters * _numInbetweens) {
				_currentAnimOrder[_animatingLetterID] = 0;
			}

			g_nancy->_sound->playSound(_letterSound);
			_drawSurface.blitFrom(_image, _letterSrcs[_animatingLetterID][_currentAnimOrder[_animatingLetterID]], _letterDests[_animatingLetterID]);
			_needsRedraw = true;

			--_frameID;
		} else {
			// Clear the number
			_numPulls[_animatingLetterID] = 0;
			_drawSurface.fillRect(_digitDests[_animatingLetterID], _drawSurface.getTransparentColor());
			_needsRedraw = true;

			// Set the order at the end of the animation to avoid "solving" while animating
			_currentOrder[_animatingLetterID] = _currentAnimOrder[_animatingLetterID] / _numInbetweens;
			_animatingLetterID = -1;
			_nextAnimTime = 0;
		}
	}
}

void SpigotPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);

	_numSpigots = stream.readUint16LE();
	_numLetters = stream.readUint16LE();
	_numInbetweens = stream.readUint16LE();

	_startOrder.resize(_numSpigots);
	for (uint i = 0; i < _numSpigots; ++i) {
		_startOrder[i] = stream.readUint16LE();
	}
	stream.skip((6 - _numSpigots) * 2);

	_numSpins.resize(6);
	for (uint i = 0; i < 6; ++i) {
		_numSpins[i] = stream.readUint16LE();
	}

	readRectArray(stream, _spigotDests, _numSpigots, 6);
	readRectArray(stream, _spigotHotspots, _numSpigots, 6);
	readRectArray(stream, _letterDests, _numSpigots, 6);
	readRectArray(stream, _digitDests, _numSpigots, 6);
	readRectArray(stream, _buttonDests, _numSpigots, 6);

	uint16 numSpigotAnimationFrames = stream.readUint16LE();
	_spigotAnimSrcs.resize(_numSpigots);
	for (uint i = 0; i < _numSpigots; ++i) {
		_spigotAnimSrcs[i].resize(numSpigotAnimationFrames);

		uint32 x = stream.readUint32LE();
		uint32 y = stream.readUint32LE();
		uint16 deltaX = stream.readUint16LE();
		uint16 height = stream.readUint16LE() + 1;

		for (uint j = 0; j < numSpigotAnimationFrames; ++j) {
			_spigotAnimSrcs[i][j] = Common::Rect(x + j * deltaX, y, x + j * deltaX + deltaX, y + height);
		}
	}
	stream.skip((6 - _numSpigots) * 12);

	_digitSrcs.resize(_numSpigots);
	for (uint i = 0; i < _numSpigots; ++i) {
		readRectArray(stream, _digitSrcs[i], 7);
	}
	stream.skip((6 - _numSpigots) * 7 * 16);

	readRectArray(stream, _buttonSrcs, _numSpigots, 6);

	_letterSrcs.resize(_numSpigots);
	for (uint i = 0; i < _numSpigots; ++i) {
		readRectArray(stream, _letterSrcs[i], _numLetters * _numInbetweens, 7 * 2);
	}
	stream.skip((6 - _numSpigots) * 7 * 2 * 16);

	_letterTime = stream.readUint16LE();

	_correctOrder.resize(_numSpigots);
	for (uint i = 0; i < _numSpigots; ++i) {
		_correctOrder[i] = stream.readUint16LE();
	}
	stream.skip((6 - _numSpigots) * 2);

	_buttonSound.readNormal(stream);
	_letterSound.readNormal(stream);
	_spigotSound.readNormal(stream);

	_solveScene.readData(stream);
	_solveSoundDelay = stream.readUint16LE();
	_solveSound.readNormal(stream);

	_exitScene.readData(stream);
	readRect(stream, _exitHotspot);
}

void SpigotPuzzle::execute() {
	switch(_state) {
	case kBegin:
		init();
		g_nancy->_sound->loadSound(_buttonSound);
		g_nancy->_sound->loadSound(_letterSound);
		g_nancy->_sound->loadSound(_spigotSound);

		_currentOrder = _startOrder;
		_currentAnimOrder.resize(_currentOrder.size());
		for (uint i = 0; i < _currentAnimOrder.size(); ++i) {
			_currentAnimOrder[i] = _currentOrder[i] * _numInbetweens;
		}
		_numPulls.resize(_numSpigots, 0);

		// Draw the start letters, in case the background ones are different
		for (uint i = 0; i < _numSpigots; ++i) {
			_drawSurface.blitFrom(_image, _letterSrcs[i][_currentAnimOrder[i]], _letterDests[i]);
		}
		_needsRedraw = true;

		NancySceneState.setNoHeldItem();

		_state = kRun;
		// fall through
	case kRun:
		if (_currentOrder == _correctOrder) {
			g_nancy->_sound->loadSound(_solveSound);
			g_nancy->_sound->playSound(_solveSound);
			_solved = true;
			_state = kActionTrigger;
		}

		break;
	case kActionTrigger:
		if (_solved) {
			// Sound delay not used
			if (g_nancy->_sound->isSoundPlaying(_solveSound)) {
				return;
			}

			_solveScene.execute();
		} else {
			_exitScene.execute();
		}

		break;
	}
}

void SpigotPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _animatingSpigotID != -1 || _animatingLetterID != -1) {
		return;
	}

	Common::Rect vpScreenPos = NancySceneState.getViewport().convertViewportToScreen(_screenPosition);
	if (!vpScreenPos.contains(input.mousePos)) {
		return;
	}

	Common::Point mousePos = input.mousePos;
	mousePos -= { vpScreenPos.left, vpScreenPos.top };

	if (_exitHotspot.contains(mousePos)) {
		g_nancy->_cursor->setCursorType(g_nancy->_cursor->_puzzleExitCursor);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_state = kActionTrigger;
		}

		return;
	}

	for (uint i = 0; i < _numSpigots; ++i) {
		if (_spigotHotspots[i].contains(mousePos)) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				g_nancy->_sound->playSound(_spigotSound);
				_animatingSpigotID = i;
				return;
			}
		}

		if (_numPulls[i] && _buttonDests[i].contains(mousePos)) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				g_nancy->_sound->playSound(_buttonSound);
				_drawSurface.blitFrom(_image, _buttonSrcs[i], _buttonDests[i]);
				_needsRedraw = true;
				_pushedButtonID = i;
				_nextAnimTime = g_nancy->getTotalPlayTime() + 250;
				return;
			}
		}
	}
}

} // End of namespace Action
} // End of namespace Nancy
