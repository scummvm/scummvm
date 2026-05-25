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

#include "engines/nancy/action/puzzle/rotatinglockpuzzle.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Action {

void RotatingLockPuzzle::init() {
	_drawSurface.create(_screenPosition.width(), _screenPosition.height(), g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());

	setTransparent(true);

	g_nancy->_resource->loadImage(_imageName, _image);
}

void RotatingLockPuzzle::readData(Common::SeekableReadStream &stream) {
	const bool isNancy10 = g_nancy->getGameType() >= kGameTypeNancy10;

	readFilename(stream, _imageName);

	uint numDials = stream.readUint16LE();

	_iconsPerDial = isNancy10 ? stream.readUint16LE() : 10;

	const uint numSrcRects = isNancy10 ? 12 : 10;
	_srcRects.reserve(numSrcRects);
	for (uint i = 0; i < numSrcRects; ++i) {
		_srcRects.push_back(Common::Rect());
		readRect(stream, _srcRects.back());
	}

	_destRects.reserve(numDials);
	for (uint i = 0; i < numDials; ++i) {
		_destRects.push_back(Common::Rect());
		readRect(stream, _destRects.back());

		if (i == 0) {
			_screenPosition = _destRects.back();
		} else {
			_screenPosition.extend(_destRects.back());
		}
	}

	stream.skip((8 - numDials) * 16);

	_upHotspots.reserve(numDials);
	for (uint i = 0; i < numDials; ++i) {
		_upHotspots.push_back(Common::Rect());
		readRect(stream, _upHotspots.back());
	}

	_downHotspots.reserve(numDials);
	stream.skip((8 - numDials) * 16);

	for (uint i = 0; i < numDials; ++i) {
		_downHotspots.push_back(Common::Rect());
		readRect(stream, _downHotspots.back());
	}

	stream.skip((8 - numDials) * 16);

	_correctSequence.reserve(numDials);
	for (uint i = 0; i < numDials; ++i) {
		_correctSequence.push_back(stream.readByte());
	}

	const uint padding = isNancy10 ? 12 : 8;
	stream.skip(padding - numDials);

	_clickSound.readNormal(stream);

	if (isNancy10) {
		// Nancy 10 splits the old SceneChangeWithFlag (25 bytes with embedded
		// flag) into a 20-byte SceneChangeDescription + 2-byte pause tail,
		// with the event flag stored as a separate (label, value) pair.
		_solveExitScene._sceneChange.readData(stream);
		stream.skip(2);
		_solveExitScene._flag.label = stream.readSint16LE();
		_solveExitScene._flag.flag  = stream.readByte();

		_solveSoundDelay = stream.readUint16LE();
		_solveSound.readNormal(stream);

		_exitScene._sceneChange.readData(stream);
		stream.skip(2);
		_exitScene._flag.label = stream.readSint16LE();
		_exitScene._flag.flag  = stream.readByte();

		readRect(stream, _exitHotspot);
		// 16 trailing bytes (cursor type + unused) at offset 0x317 are ignored.
	} else {
		_solveExitScene.readData(stream);
		_solveSoundDelay = stream.readUint16LE();
		_solveSound.readNormal(stream);

		_exitScene.readData(stream);
		readRect(stream, _exitHotspot);
	}
}

void RotatingLockPuzzle::execute() {
	const bool isNancy10 = g_nancy->getGameType() >= kGameTypeNancy10;

	switch (_state) {
	case kBegin:
		init();
		registerGraphics();

		NancySceneState.setNoHeldItem();

		for (uint i = 0; i < _correctSequence.size(); ++i) {
			byte v = g_nancy->_randomSource->getRandomNumber(_iconsPerDial - 1);
			// Nancy 10 rerolls until the starting value differs from the
			// solution so the puzzle never appears already-solved.
			while (isNancy10 && v == _correctSequence[i])
				v = g_nancy->_randomSource->getRandomNumber(_iconsPerDial - 1);
			_currentSequence.push_back(v);
			drawDial(i);
		}

		g_nancy->_sound->loadSound(_clickSound);
		g_nancy->_sound->loadSound(_solveSound);
		_state = kRun;
		// fall through
	case kRun:
		switch (_solveState) {
		case kNotSolved:
			for (uint i = 0; i < _correctSequence.size(); ++i) {
				if (_currentSequence[i] != (int16)_correctSequence[i]) {
					return;
				}
			}

			NancySceneState.setEventFlag(_solveExitScene._flag);
			_solveSoundPlayTime = g_nancy->getTotalPlayTime() + _solveSoundDelay * 1000;
			_solveState = kPlaySound;
			// fall through
		case kPlaySound:
			if (g_nancy->getTotalPlayTime() <= _solveSoundPlayTime) {
				break;
			}

			g_nancy->_sound->playSound(_solveSound);
			_solveState = kWaitForSound;
			break;
		case kWaitForSound:
			if (!g_nancy->_sound->isSoundPlaying(_solveSound)) {
				_state = kActionTrigger;
			}

			break;
		}
		break;
	case kActionTrigger:
		g_nancy->_sound->stopSound(_clickSound);
		g_nancy->_sound->stopSound(_solveSound);

		if (_solveState == kNotSolved)
			_exitScene.execute();
		else
			NancySceneState.changeScene(_solveExitScene._sceneChange);

		finishExecution();
	}
}

void RotatingLockPuzzle::handleInput(NancyInput &input) {
	if (_solveState != kNotSolved) {
		return;
	}

	if (NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos)) {
		g_nancy->_cursor->setCursorType(g_nancy->_cursor->_puzzleExitCursor);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_state = kActionTrigger;
		}

		return;
	}

	for (uint i = 0; i < _upHotspots.size(); ++i) {
		if (NancySceneState.getViewport().convertViewportToScreen(_upHotspots[i]).contains(input.mousePos)) {
			g_nancy->_cursor->setCursorType(CursorManager::kMoveUp);

			if (!g_nancy->_sound->isSoundPlaying(_clickSound) && input.input & NancyInput::kLeftMouseButtonUp) {
				g_nancy->_sound->playSound(_clickSound);

				int n = _currentSequence[i] + 1;
				if (n >= (int)_iconsPerDial)
					n = 0;
				_currentSequence[i] = (byte)n;
				drawDial(i);
			}

			return;
		}
	}

	for (uint i = 0; i < _downHotspots.size(); ++i) {
		if (NancySceneState.getViewport().convertViewportToScreen(_downHotspots[i]).contains(input.mousePos)) {
			g_nancy->_cursor->setCursorType(CursorManager::kMoveDown);

			if (!g_nancy->_sound->isSoundPlaying(_clickSound) && input.input & NancyInput::kLeftMouseButtonUp) {
				g_nancy->_sound->playSound(_clickSound);

				int n = (int)_currentSequence[i] - 1;
				if (n < 0)
					n = (int)_iconsPerDial - 1;
				_currentSequence[i] = (byte)n;
				drawDial(i);
			}

			return;
		}
	}
}
void RotatingLockPuzzle::drawDial(uint id) {
	Common::Point destPoint(_destRects[id].left - _screenPosition.left, _destRects[id].top - _screenPosition.top);
	_drawSurface.blitFrom(_image, _srcRects[_currentSequence[id]], destPoint);

	_needsRedraw = true;
}

} // End of namespace Action
} // End of namespace Nancy
