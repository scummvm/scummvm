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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/random.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/sequencelockpuzzle.h"

namespace Nancy {
namespace Action {

void SequenceLockPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);
	_hoverCursor = stream.readUint16LE();

	_switches.resize(stream.readUint16LE());
	for (Switch &lockSwitch : _switches) {
		readExclusiveRect(stream, lockSwitch.dest);
	}

	const uint16 numHotspots = stream.readUint16LE();
	if (numHotspots != _switches.size()) {
		error("SequenceLockPuzzle: %u hotspots for %u switches", numHotspots, _switches.size());
	}

	for (Switch &lockSwitch : _switches) {
		readExclusiveRect(stream, lockSwitch.hotspot);
	}

	_resetFrame = stream.readSint32LE();
	_framesPerSecond = stream.readSint32LE();

	_frameSrcs.resize(stream.readUint16LE());
	for (Common::Rect &src : _frameSrcs) {
		readExclusiveRect(stream, src);
	}

	_pressSound.readData(stream);
	_resetSound.readData(stream);

	_solveScene._sceneChange.sceneID = stream.readUint16LE();
	_solveScene._sceneChange.frameID = stream.readUint16LE();
	_solveScene._sceneChange.continueSceneSound = kContinueSceneSound;
	_solveScene._flag.label = stream.readSint16LE();
	_solveScene._flag.flag = stream.readByte();
	_solveSoundBlock.readData(stream);

	readExitHotspot(stream);
}

void SequenceLockPuzzle::init() {
	initViewportSurface();
	loadImage();

	// The order the switches have to be thrown in
	Common::Array<uint> remaining(_switches.size());
	for (uint i = 0; i < remaining.size(); ++i) {
		remaining[i] = i;
	}

	_order.clear();
	while (!remaining.empty()) {
		uint pick = g_nancy->_randomSource->getRandomNumber(remaining.size() - 1);
		_order.push_back(remaining[pick]);
		remaining.remove_at(pick);
	}

	_lastFrameTime = g_nancy->getTotalPlayTime();
	redraw();
}

void SequenceLockPuzzle::redraw() {
	_drawSurface.clear(_drawSurface.getTransparentColor());

	for (const Switch &lockSwitch : _switches) {
		if (lockSwitch.frame < (int)_frameSrcs.size()) {
			_drawSurface.blitFrom(_image, _frameSrcs[lockSwitch.frame], lockSwitch.dest.origin());
		}
	}

	_needsRedraw = true;
}

void SequenceLockPuzzle::pressSwitch(uint switchID) {
	++_numPressed;
	_switches[switchID].direction = kForward;
	playSoundBlock(_pressSound);

	if (_order[_numPressed - 1] != switchID) {
		_wrongSwitch = switchID;
	}
}

void SequenceLockPuzzle::resetSwitches() {
	for (Switch &lockSwitch : _switches) {
		lockSwitch.direction = kBackward;
	}

	playSoundBlock(_resetSound);
	_wrongSwitch = -1;
	_numPressed = 0;
}

// Returns true when a switch has changed
bool SequenceLockPuzzle::animateSwitches(uint numFrames) {
	bool hasChanged = false;
	const int lastFrame = (int)_frameSrcs.size() - 1;

	for (Switch &lockSwitch : _switches) {
		if (lockSwitch.direction == kStopped) {
			continue;
		}

		int frame = CLIP<int>(lockSwitch.frame + lockSwitch.direction * (int)numFrames, 0, MAX(lastFrame, 0));
		if (frame != lockSwitch.frame) {
			lockSwitch.frame = frame;
			hasChanged = true;
		}

		if ((lockSwitch.direction == kForward && frame >= lastFrame) || (lockSwitch.direction == kBackward && frame == 0)) {
			lockSwitch.direction = kStopped;
		}
	}

	return hasChanged;
}

void SequenceLockPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun: {
		if (_exitRequested) {
			_state = kActionTrigger;
			break;
		}

		const uint32 frameTime = 1000 / MAX<int32>(_framesPerSecond, 1);
		const uint32 now = g_nancy->getTotalPlayTime();
		const uint numFrames = (now - _lastFrameTime) / frameTime;
		_lastFrameTime += numFrames * frameTime;

		if (numFrames > 0 && animateSwitches(numFrames)) {
			redraw();
		}

		if (_wrongSwitch != -1) {
			if (_switches[_wrongSwitch].frame > _resetFrame) {
				resetSwitches();
			}

			break;
		}

		bool isMoving = false;
		for (const Switch &lockSwitch : _switches) {
			if (lockSwitch.direction != kStopped) {
				isMoving = true;
			}
		}

		if (_numPressed == _switches.size() && !isMoving) {
			_isSolved = true;
			playSoundBlock(_solveSoundBlock);
			_state = kActionTrigger;
		}

		break;
	}
	case kActionTrigger:
		if (_isSolved) {
			// The solve sound gets to finish first
			if (isSoundBlockPlaying(_solveSoundBlock)) {
				break;
			}

			_solveScene.execute();
		} else {
			_exitScene.execute();
		}

		finishExecution();
		break;
	}
}

void SequenceLockPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _exitRequested) {
		return;
	}

	const bool clicked = input.input & NancyInput::kLeftMouseButtonUp;

	if (hoverExitHotspot(input)) {
		if (clicked) {
			_exitRequested = true;
			input.eatMouseInput();
		}

		return;
	}

	// Nothing can be thrown while a wrong switch is on its way
	if (_wrongSwitch != -1) {
		return;
	}

	Common::Rect mouseRect = NancySceneState.getViewport().convertScreenToViewport(
		Common::Rect(input.mousePos.x, input.mousePos.y, input.mousePos.x + 1, input.mousePos.y + 1));
	Common::Point pos = mouseRect.origin();

	for (uint i = 0; i < _switches.size(); ++i) {
		// Only a switch that is all the way back can be thrown
		if (_switches[i].hotspot.contains(pos) && _switches[i].frame == 0) {
			setDataCursor(_hoverCursor);

			if (clicked && _numPressed < _order.size()) {
				pressSwitch(i);
				input.eatMouseInput();
			}

			return;
		}
	}
}

} // End of namespace Action
} // End of namespace Nancy
