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

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/dialsequencepuzzle.h"

namespace Nancy {
namespace Action {

uint DialSequencePuzzle::countPositions(const Common::String &positions) {
	uint count = 1;
	for (uint i = 0; i < positions.size(); ++i) {
		if (positions[i] == ',') {
			++count;
		}
	}

	return count;
}

void DialSequencePuzzle::readData(Common::SeekableReadStream &stream) {
	_setCursor = stream.readUint16LE();
	_forwardCursor = stream.readUint16LE();
	_backCursor = stream.readUint16LE();

	readFilename(stream, _imageName);

	_dials.resize(stream.readUint16LE());
	for (Dial &dial : _dials) {
		readFilename(stream, dial.name);
		dial.canWrapAround = stream.readByte();
		readRect(stream, dial.forwardHotspot);
		readRect(stream, dial.backHotspot);

		dial.srcs.resize(stream.readUint16LE());
		for (Common::Rect &src : dial.srcs) {
			readRect(stream, src);
		}
	}

	readRect(stream, _dialDest);
	readFilename(stream, _setMovieName);
	readRect(stream, _setDest);

	_setSound.readData(stream);
	_turnSound.readData(stream);

	_solutions.resize(stream.readUint16LE());
	for (Solution &solution : _solutions) {
		solution.condition.label = stream.readSint16LE();
		solution.condition.flag = stream.readByte();

		// A dial name and its positions each count as one
		solution.sequences.resize(stream.readUint16LE() / 2);
		for (Sequence &sequence : solution.sequences) {
			readFilename(stream, sequence.dialName);
			readFilename(stream, sequence.positions);
			_maxPositions = MAX(_maxPositions, countPositions(sequence.positions));
		}

		solution.scene._sceneChange.sceneID = stream.readUint16LE();
		solution.scene._sceneChange.frameID = stream.readUint16LE();
		solution.scene._sceneChange.continueSceneSound = kContinueSceneSound;
		solution.scene._flag.label = stream.readSint16LE();
		solution.scene._flag.flag = stream.readByte();
		solution.sound.readData(stream);
	}

	readExitHotspot(stream);
}

void DialSequencePuzzle::init() {
	initViewportSurface();
	loadImage();

	if (!_setMovieName.empty()) {
		_setMovie.loadFile(_setMovieName);
	}

	redraw();
}

void DialSequencePuzzle::redraw() {
	_drawSurface.clear(_drawSurface.getTransparentColor());

	if (_isSetMovieShown) {
		_setMovie.drawFrame(_drawSurface, _setDest.origin());
	}

	for (const Dial &dial : _dials) {
		if (dial.position < dial.srcs.size()) {
			_drawSurface.blitFrom(_image, dial.srcs[dial.position], _dialDest.origin());
		}
	}

	_needsRedraw = true;
}

bool DialSequencePuzzle::turnDial(Dial &dial, const Common::Point &mousePos) {
	const uint numPositions = dial.srcs.size();
	if (numPositions == 0) {
		return false;
	}

	if (dial.forwardHotspot.contains(mousePos) && (dial.canWrapAround || dial.position + 1 < numPositions)) {
		dial.position = (dial.position + 1) % numPositions;
		return true;
	}

	if (dial.backHotspot.contains(mousePos) && (dial.canWrapAround || dial.position > 0)) {
		dial.position = (dial.position + numPositions - 1) % numPositions;
		return true;
	}

	return false;
}

bool DialSequencePuzzle::isSolution(const Solution &solution) const {
	if (solution.condition.label != -1 &&
			!NancySceneState.getEventFlag(solution.condition.label, solution.condition.flag)) {
		return false;
	}

	for (const Sequence &sequence : solution.sequences) {
		bool matches = false;

		for (const Dial &dial : _dials) {
			if (dial.name.equalsIgnoreCase(sequence.dialName)) {
				matches = dial.entered == sequence.positions;
				break;
			}
		}

		if (!matches) {
			return false;
		}
	}

	return true;
}

void DialSequencePuzzle::enterPositions() {
	for (Dial &dial : _dials) {
		if (!dial.entered.empty()) {
			dial.entered += ',';
		}

		dial.entered += Common::String::format("%u", dial.position);

		// Only as many positions as the longest solution has are kept
		if (countPositions(dial.entered) > _maxPositions) {
			dial.entered = dial.entered.substr(dial.entered.findFirstOf(',') + 1);
		}
	}

	for (uint i = 0; i < _solutions.size() && _solvedSolution == -1; ++i) {
		if (isSolution(_solutions[i])) {
			_solvedSolution = i;
			_state = kActionTrigger;
		}
	}

	if (_setMovie.isVideoLoaded()) {
		_setMovie.playRange(0, _setMovie.getFrameCount() - 1);
		_isSetMovieShown = true;
	}

	playSoundBlock(_setSound);
	redraw();
}

void DialSequencePuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun:
		if (_exitRequested) {
			_state = kActionTrigger;
			break;
		}

		if (_setMovie.isRangePlaying() && _setMovie.update()) {
			redraw();
		}

		break;
	case kActionTrigger:
		if (_solvedSolution != -1) {
			playSoundBlock(_solutions[_solvedSolution].sound);
			_solutions[_solvedSolution].scene.execute();
		} else {
			_exitScene.execute();
		}

		finishExecution();
		break;
	}
}

void DialSequencePuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _exitRequested) {
		return;
	}

	Common::Rect viewportBounds = NancySceneState.getViewport().getBounds();
	if (!NancySceneState.getViewport().convertViewportToScreen(viewportBounds).contains(input.mousePos)) {
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

	Common::Rect mouseRect = NancySceneState.getViewport().convertScreenToViewport(
		Common::Rect(input.mousePos.x, input.mousePos.y, input.mousePos.x + 1, input.mousePos.y + 1));
	Common::Point pos = mouseRect.origin();

	for (Dial &dial : _dials) {
		if (dial.forwardHotspot.contains(pos)) {
			setDataCursor(_forwardCursor);
		} else if (dial.backHotspot.contains(pos)) {
			setDataCursor(_backCursor);
		} else {
			continue;
		}

		if (clicked && turnDial(dial, pos)) {
			playSoundBlock(_turnSound);
			redraw();
			input.eatMouseInput();
		}

		return;
	}

	// The button can't be pressed again while it is moving
	if (_setDest.contains(pos) && !_setMovie.isRangePlaying()) {
		setDataCursor(_setCursor);

		if (clicked) {
			enterPositions();
			input.eatMouseInput();
		}
	}
}

} // End of namespace Action
} // End of namespace Nancy
