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
#include "engines/nancy/action/puzzle/shovelmeasurepuzzle.h"

namespace Nancy {
namespace Action {

ShovelMeasurePuzzle::Direction ShovelMeasurePuzzle::readDirection(Common::SeekableReadStream &stream) {
	Common::String name;
	readFilename(stream, name);

	if (name.equalsIgnoreCase("N")) {
		return kNorth;
	} else if (name.equalsIgnoreCase("S")) {
		return kSouth;
	} else if (name.equalsIgnoreCase("E")) {
		return kEast;
	} else if (name.equalsIgnoreCase("W")) {
		return kWest;
	}

	error("ShovelMeasurePuzzle: %s is not a direction", name.c_str());
}

Common::Point ShovelMeasurePuzzle::getNeighbor(const Common::Point &pos, Direction direction) {
	switch (direction) {
	case kNorth:
		return Common::Point(pos.x, pos.y - 1);
	case kSouth:
		return Common::Point(pos.x, pos.y + 1);
	case kEast:
		return Common::Point(pos.x + 1, pos.y);
	default:
		return Common::Point(pos.x - 1, pos.y);
	}
}

double ShovelMeasurePuzzle::getAngle(Direction direction) {
	switch (direction) {
	case kSouth:
		return M_PI;
	case kEast:
		return M_PI / 2;
	case kWest:
		return 3 * M_PI / 2;
	default:
		return 0;
	}
}

void ShovelMeasurePuzzle::readData(Common::SeekableReadStream &stream) {
	_buttonCursor = stream.readUint16LE();
	_maxWrongSteps = stream.readSint32LE();
	_moveTime = (uint32)(stream.readDoubleLE() * 1000);
	readFilename(stream, _imageName);

	readRect(stream, _directionButtons[kNorth]);
	readRect(stream, _directionButtons[kSouth]);
	readRect(stream, _directionButtons[kEast]);
	readRect(stream, _directionButtons[kWest]);

	readRect(stream, _dialSrc);
	readRect(stream, _dialDest);
	_dialPivot.x = (int16)(stream.readDoubleLE() - _dialDest.left);
	_dialPivot.y = (int16)(stream.readDoubleLE() - _dialDest.top);

	readRect(stream, _stepPressedSrc);
	readRect(stream, _stepButton);

	_startPos.x = stream.readSint32LE();
	_startPos.y = stream.readSint32LE();
	_startDirection = readDirection(stream);

	readRect(stream, _shovelSrc);
	readRect(stream, _tapeDest);

	uint16 numLengths = stream.readUint16LE();
	_maxMeasure = numLengths;
	for (uint i = 0; i < numLengths; ++i) {
		uint16 length = stream.readUint16LE();
		if (length >= _tapeSrcs.size()) {
			_tapeSrcs.resize(length + 1);
		}

		_tapeSrcs[length].resize(stream.readUint16LE());
		for (Common::Rect &src : _tapeSrcs[length]) {
			readRect(stream, src);
		}
	}

	// The top left cell, the one to its right and the one below it
	Common::Rect cellRight, cellBelow;
	readRect(stream, _firstCell);
	readRect(stream, cellRight);
	readRect(stream, cellBelow);
	_cellSpacing = Common::Point(cellRight.left - _firstCell.left, cellBelow.top - _firstCell.top);

	// A space is a cell the shovel can be on
	_numRows = stream.readUint16LE();
	for (int y = 0; y < _numRows; ++y) {
		Common::String row;
		readFilename(stream, row);

		if (y == 0) {
			_numColumns = row.size();
			_cells.resize(_numColumns * _numRows);
		}

		for (int x = 0; x < _numColumns && x < (int)row.size(); ++x) {
			_cells[y * _numColumns + x].isValid = row[x] == ' ';
		}
	}

	if (!isValidCell(_startPos)) {
		error("ShovelMeasurePuzzle: the shovel does not start on the grid");
	}

	// The path is given as directions and numbers of steps
	Common::Point pos = _startPos;
	_path.resize(stream.readUint16LE());
	for (Stretch &stretch : _path) {
		stretch.direction = readDirection(stream);
		stretch.from = pos;

		int32 numSteps = stream.readSint32LE();
		for (int32 i = 0; i < numSteps; ++i) {
			pos = getNeighbor(pos, stretch.direction);
		}

		if (!isValidCell(pos)) {
			error("ShovelMeasurePuzzle: the path leaves the grid");
		}

		stretch.to = pos;
		getCell(pos).endsStretch[stretch.direction] = true;
	}

	_turnSound.readData(stream);
	_stepSound.readData(stream);

	_solveScene._sceneChange.sceneID = stream.readUint16LE();
	_solveScene._sceneChange.frameID = stream.readUint16LE();
	_solveScene._sceneChange.continueSceneSound = kContinueSceneSound;
	_solveScene._flag.label = stream.readSint16LE();
	_solveScene._flag.flag = stream.readByte();
	_winSound.readData(stream);

	_loseScene._sceneChange.sceneID = stream.readUint16LE();
	_loseScene._sceneChange.frameID = stream.readUint16LE();
	_loseScene._sceneChange.continueSceneSound = kContinueSceneSound;
	_loseScene._flag.label = stream.readSint16LE();
	_loseScene._flag.flag = stream.readByte();
	_loseSound.readData(stream);

	readExitHotspot(stream);
}

void ShovelMeasurePuzzle::init() {
	initViewportSurface();
	loadImage();

	_pos = _targetPos = _startPos;
	_direction = _targetDirection = _startDirection;
	_lastGuessPos = Common::Point(-1, -1);

	updateMeasure();
	_shownMeasure = _measure;
	redraw();
}

bool ShovelMeasurePuzzle::isValidCell(const Common::Point &pos) const {
	return pos.x >= 0 && pos.y >= 0 && pos.x < _numColumns && pos.y < _numRows && getCell(pos).isValid;
}

Common::Point ShovelMeasurePuzzle::getCellCenter(const Common::Point &pos) const {
	return Common::Point(_firstCell.left + pos.x * _cellSpacing.x + _firstCell.width() / 2,
						_firstCell.top + pos.y * _cellSpacing.y + _firstCell.height() / 2);
}

void ShovelMeasurePuzzle::updateMeasure() {
	const Common::Point start = _move == kStep ? _targetPos : _pos;
	const Direction direction = _move == kFace ? _targetDirection : _direction;

	Common::Point pos = start;
	_measure = 0;

	while (true) {
		pos = getNeighbor(pos, direction);
		if (!isValidCell(pos) || _measure >= _maxMeasure) {
			break;
		}

		++_measure;

		// The tape is right when it gets to the end of a stretch going this way
		if (pos == _lastGuessPos || getCell(pos).endsStretch[direction]) {
			return;
		}
	}

	if (_measure > 0) {
		// Nothing to measure to, so the tape shows whatever
		_lastGuessPos = start;
		_measure = MAX<uint>(g_nancy->_randomSource->getRandomNumber(MIN(_maxMeasure, _measure)), 1);
	}
}

void ShovelMeasurePuzzle::face(Direction direction) {
	if (direction == _direction) {
		return;
	}

	_targetDirection = direction;
	_move = kFace;
	_moveStartTime = g_nancy->getTotalPlayTime();
	playSoundBlock(_turnSound);
	updateMeasure();
}

void ShovelMeasurePuzzle::step() {
	Common::Point next = getNeighbor(_pos, _direction);
	if (!isValidCell(next)) {
		return;
	}

	_targetPos = next;
	_move = kStep;
	_moveStartTime = g_nancy->getTotalPlayTime();
	playSoundBlock(_stepSound);
	updateMeasure();
}

void ShovelMeasurePuzzle::finishMove() {
	if (_move == kFace) {
		_direction = _targetDirection;
	} else {
		_pos = _targetPos;

		// A step that is not on the stretch being followed counts against the player
		const Stretch &stretch = _path[_currentStretch];
		bool isOnStretch = false;
		for (Common::Point pos = stretch.from; ; pos = getNeighbor(pos, stretch.direction)) {
			if (pos == _pos) {
				isOnStretch = true;
				break;
			}

			if (pos == stretch.to) {
				break;
			}
		}

		if (isOnStretch) {
			_numWrongSteps = 0;
		} else {
			++_numWrongSteps;
		}

		if (_pos == stretch.to) {
			++_currentStretch;
		}
	}

	_move = kNoCommand;
	_shownMeasure = _measure;

	if (_numWrongSteps > _maxWrongSteps) {
		_outcome = kLost;
	} else if (_currentStretch == _path.size()) {
		_outcome = kWon;
	}
}

void ShovelMeasurePuzzle::drawRotated(const Common::Rect &src, const Common::Point &srcPivot, const Common::Point &destPivot, double angle) {
	const double cosA = cos(angle);
	const double sinA = sin(angle);
	const uint32 transColor = _image.getTransparentColor();

	// Everything the picture can cover, however it is turned
	const int radius = (int)ceil(sqrt((double)(src.width() * src.width() + src.height() * src.height())));
	const int startX = MAX<int>(0, destPivot.x - radius);
	const int startY = MAX<int>(0, destPivot.y - radius);
	const int endX = MIN<int>(_drawSurface.w, destPivot.x + radius + 1);
	const int endY = MIN<int>(_drawSurface.h, destPivot.y + radius + 1);

	for (int y = startY; y < endY; ++y) {
		for (int x = startX; x < endX; ++x) {
			double dx = x + 0.5 - destPivot.x;
			double dy = y + 0.5 - destPivot.y;
			int srcX = (int)floor(dx * cosA + dy * sinA + srcPivot.x);
			int srcY = (int)floor(-dx * sinA + dy * cosA + srcPivot.y);

			if (srcX < 0 || srcX >= src.width() || srcY < 0 || srcY >= src.height()) {
				continue;
			}

			uint32 color = _image.getPixel(src.left + srcX, src.top + srcY);
			if (color != transColor) {
				_drawSurface.setPixel(x, y, color);
			}
		}
	}
}

void ShovelMeasurePuzzle::redraw() {
	_drawSurface.clear(_drawSurface.getTransparentColor());

	// How far along the turn or the step is
	double progress = 1.0;
	if (_move != kNoCommand && _moveTime > 0) {
		progress = CLIP((g_nancy->getTotalPlayTime() - _moveStartTime) / (double)_moveTime, 0.0, 1.0);
	}

	double angle = getAngle(_direction);
	if (_move == kFace) {
		// The short way around
		double delta = getAngle(_targetDirection) - angle;
		if (delta > M_PI) {
			delta -= 2 * M_PI;
		} else if (delta < -M_PI) {
			delta += 2 * M_PI;
		}

		angle += delta * progress;
	}

	Common::Point shovelPos = getCellCenter(_pos);
	if (_move == kStep) {
		Common::Point target = getCellCenter(_targetPos);
		shovelPos.x += (int16)((target.x - shovelPos.x) * progress);
		shovelPos.y += (int16)((target.y - shovelPos.y) * progress);
	}

	drawRotated(_shovelSrc, Common::Point(_shovelSrc.width() / 2, _shovelSrc.height() / 2), shovelPos, angle);
	drawRotated(_dialSrc, _dialPivot, Common::Point(_dialDest.left + _dialPivot.x, _dialDest.top + _dialPivot.y), angle);

	if (_move == kStep) {
		_drawSurface.blitFrom(_image, _stepPressedSrc, _stepButton.origin());
	}

	// The tape goes back in during the first half of a move, and out to the new
	// length during the second one
	uint measure = _shownMeasure;
	double tapeProgress = 1.0;
	if (_move != kNoCommand) {
		if (progress < 0.5) {
			tapeProgress = 1.0 - 2 * progress;
		} else {
			measure = _measure;
			tapeProgress = 2 * (progress - 0.5);
		}
	}

	if (measure == 0) {
		// Nothing to show is the shortest tape, all the way in
		measure = 1;
		tapeProgress = 0.0;
	}

	if (measure < _tapeSrcs.size() && !_tapeSrcs[measure].empty()) {
		const Common::Array<Common::Rect> &srcs = _tapeSrcs[measure];
		int frame = CLIP<int>((int)(srcs.size() * tapeProgress), 0, srcs.size() - 1);
		_drawSurface.blitFrom(_image, srcs[frame], _tapeDest.origin());
	}

	_needsRedraw = true;
}

void ShovelMeasurePuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun:
		if (_move != kNoCommand) {
			if (g_nancy->getTotalPlayTime() - _moveStartTime >= _moveTime) {
				finishMove();
			}

			redraw();
		}

		if (_outcome != kNone) {
			_state = kActionTrigger;
		}

		break;
	case kActionTrigger:
		switch (_outcome) {
		case kWon:
			playSoundBlock(_winSound);
			_solveScene.execute();
			break;
		case kLost:
			playSoundBlock(_loseSound);
			_loseScene.execute();
			break;
		default:
			_exitScene.execute();
			break;
		}

		finishExecution();
		break;
	}
}

void ShovelMeasurePuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _outcome != kNone) {
		return;
	}

	// A turn or a step has to be over before the next one
	const bool canMove = _move == kNoCommand;

	Direction direction = kNoDirection;
	bool wantsStep = false;

	if (input.input & NancyInput::kMoveUp) {
		direction = kNorth;
	} else if (input.input & NancyInput::kMoveDown) {
		direction = kSouth;
	} else if (input.input & NancyInput::kMoveRight) {
		direction = kEast;
	} else if (input.input & NancyInput::kMoveLeft) {
		direction = kWest;
	}

	for (const Common::KeyState &key : input.otherKbdInput) {
		if (key.keycode == Common::KEYCODE_RETURN || key.keycode == Common::KEYCODE_KP_ENTER) {
			wantsStep = true;
		}
	}

	Common::Rect viewportBounds = NancySceneState.getViewport().getBounds();
	if (NancySceneState.getViewport().convertViewportToScreen(viewportBounds).contains(input.mousePos)) {
		const bool clicked = input.input & NancyInput::kLeftMouseButtonUp;

		Common::Rect mouseRect = NancySceneState.getViewport().convertScreenToViewport(
			Common::Rect(input.mousePos.x, input.mousePos.y, input.mousePos.x + 1, input.mousePos.y + 1));
		Common::Point pos = mouseRect.origin();

		bool isOverButton = _stepButton.contains(pos);
		if (isOverButton && clicked) {
			wantsStep = true;
		}

		for (uint i = 0; i < kNumDirections && !isOverButton; ++i) {
			if (_directionButtons[i].contains(pos)) {
				isOverButton = true;
				if (clicked) {
					direction = (Direction)i;
				}
			}
		}

		if (isOverButton) {
			setDataCursor(_buttonCursor, canMove);
			if (clicked) {
				input.eatMouseInput();
			}
		} else if (hoverExitHotspot(input) && clicked) {
			_outcome = kExited;
			input.eatMouseInput();
			return;
		}
	}

	if (!canMove) {
		return;
	}

	if (wantsStep) {
		step();
	} else if (direction != kNoDirection) {
		face(direction);
	}
}

} // End of namespace Action
} // End of namespace Nancy
