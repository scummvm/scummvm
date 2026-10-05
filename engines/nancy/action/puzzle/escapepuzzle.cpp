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

#include "common/algorithm.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/escapepuzzle.h"

namespace Nancy {
namespace Action {

// The board moves on in ticks of this many milliseconds
static const uint32 kTickLength = 20;

void EscapePuzzle::DirectionalAnimation::readData(Common::SeekableReadStream &stream) {
	for (Common::Array<Common::Rect> &directionFrames : frames) {
		directionFrames.resize(stream.readUint16LE());
		for (Common::Rect &frame : directionFrames) {
			readRect(stream, frame);
		}
	}
}

EscapePuzzle::Direction EscapePuzzle::readDirection(Common::SeekableReadStream &stream) {
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

	error("EscapePuzzle: %s is not a direction", name.c_str());
}

Common::Point EscapePuzzle::getNeighbor(const Common::Point &pos, Direction direction) {
	switch (direction) {
	case kNorth:
		return Common::Point(pos.x, pos.y - 1);
	case kSouth:
		return Common::Point(pos.x, pos.y + 1);
	case kEast:
		return Common::Point(pos.x + 1, pos.y);
	case kWest:
		return Common::Point(pos.x - 1, pos.y);
	default:
		return pos;
	}
}

EscapePuzzle::Direction EscapePuzzle::getOpposite(Direction direction) {
	switch (direction) {
	case kNorth:
		return kSouth;
	case kSouth:
		return kNorth;
	case kEast:
		return kWest;
	default:
		return kEast;
	}
}

bool EscapePuzzle::compareObjectNames(const Object &a, const Object &b) {
	return a.name.compareToIgnoreCase(b.name) < 0;
}

void EscapePuzzle::readData(Common::SeekableReadStream &stream) {
	_buttonPressTime = (uint32)(stream.readDoubleLE() * 1000);

	const Direction buttonOrder[] = { kNorth, kSouth, kWest, kEast };
	for (Direction direction : buttonOrder) {
		Button &button = _buttons[direction];
		button.direction = direction;
		readRect(stream, button.pressedSrc);
		readRect(stream, button.dest);
	}

	_buttonCursor = stream.readUint16LE();

	// The top left cell, the one to its right and the one below it
	Common::Rect cellRight, cellBelow;
	readRect(stream, _firstCell);
	readRect(stream, cellRight);
	readRect(stream, cellBelow);
	_cellSpacing = Common::Point(cellRight.left - _firstCell.left, cellBelow.top - _firstCell.top);

	readFilename(stream, _imageName);

	// A star is a wall
	_numRows = stream.readUint16LE();
	Common::Array<Common::String> rows(_numRows);
	for (Common::String &row : rows) {
		char line[50];
		stream.read(line, sizeof(line));
		line[sizeof(line) - 1] = '\0';
		row = line;
		_numColumns = MAX<int>(_numColumns, row.size());
	}

	_walls.resize(_numColumns * _numRows);
	for (int y = 0; y < _numRows; ++y) {
		for (int x = 0; x < (int)rows[y].size(); ++x) {
			_walls[y * _numColumns + x] = rows[y][x] == '*';
		}
	}

	ObjectKind &player = _kinds[kPlayer];
	player.moveTime = (uint32)(stream.readDoubleLE() * 1000);
	player.animation.readData(stream);
	player.secondAnimation.readData(stream);
	player.moveSound.readData(stream);
	player.pushSound.readData(stream);
	player.bumpSound.readData(stream);

	ObjectKind &ball = _kinds[kBall];
	ball.moveTime = (uint32)(stream.readDoubleLE() * 1000);
	ball.animation.readData(stream);
	ball.moveSound.readData(stream);

	ObjectKind &box = _kinds[kBox];
	box.moveTime = (uint32)(stream.readDoubleLE() * 1000);
	box.animation.readData(stream);
	box.moveSound.readData(stream);

	ObjectKind &laser = _kinds[kLaser];
	laser.animation.readData(stream);
	laser.secondAnimation.readData(stream);

	_objects.resize(stream.readUint16LE());
	for (Object &object : _objects) {
		readFilename(stream, object.name);

		Common::String typeName;
		readFilename(stream, typeName);
		if (typeName.equalsIgnoreCase("Player")) {
			if (!_playerName.empty()) {
				error("EscapePuzzle: there can only be one player on the board");
			}

			object.type = kPlayer;
			_playerName = object.name;
		} else if (typeName.equalsIgnoreCase("Ball")) {
			object.type = kBall;
		} else if (typeName.equalsIgnoreCase("Box")) {
			object.type = kBox;
		} else if (typeName.equalsIgnoreCase("Laser")) {
			object.type = kLaser;
		} else {
			error("EscapePuzzle: unknown object type %s", typeName.c_str());
		}

		object.pos.x = stream.readSint32LE();
		object.pos.y = stream.readSint32LE();
		object.facing = readDirection(stream);
	}

	// The objects take their turns in the order of their names
	Common::sort(_objects.begin(), _objects.end(), compareObjectNames);

	_goals.resize(stream.readUint16LE());
	for (Goal &goal : _goals) {
		readFilename(stream, goal.objectName);
		goal.pos.x = stream.readSint32LE();
		goal.pos.y = stream.readSint32LE();
		goal.scene._sceneChange.sceneID = stream.readUint16LE();
		goal.scene._sceneChange.frameID = stream.readUint16LE();
		goal.scene._sceneChange.continueSceneSound = kContinueSceneSound;
		goal.scene._flag.label = stream.readSint16LE();
		goal.scene._flag.flag = stream.readByte();
		goal.sound.readData(stream);
	}

	readExitHotspot(stream);
}

void EscapePuzzle::init() {
	initViewportSurface();
	loadImage();

	_lastUpdateTime = g_nancy->getTotalPlayTime();
	redraw();
}

Common::Rect EscapePuzzle::getCellRect(const Common::Point &pos) const {
	Common::Rect rect = _firstCell;
	rect.translate(pos.x * _cellSpacing.x, pos.y * _cellSpacing.y);
	return rect;
}

EscapePuzzle::CellContents EscapePuzzle::getContents(const Common::Point &pos) const {
	if (pos.x < 0 || pos.y < 0 || pos.x >= _numColumns || pos.y >= _numRows || _walls[pos.y * _numColumns + pos.x]) {
		return kWall;
	}

	for (const Object &object : _objects) {
		if (object.pos == pos) {
			return kObject;
		}

		if (object.type == kLaser) {
			for (const BeamCell &cell : object.beam) {
				if (cell.pos == pos) {
					return kLaserBeam;
				}
			}
		}
	}

	return kEmpty;
}

EscapePuzzle::Object *EscapePuzzle::findObject(const Common::String &name) {
	for (Object &object : _objects) {
		if (object.name.equalsIgnoreCase(name)) {
			return &object;
		}
	}

	return nullptr;
}

void EscapePuzzle::push(const Common::Point &pos, Direction direction) {
	for (Object &object : _objects) {
		if (object.pos != pos) {
			continue;
		}

		// Only balls and boxes give way
		if (object.type == kBall || object.type == kBox) {
			object.movingTo = direction;
		}

		return;
	}
}

void EscapePuzzle::updateBeam(Object &laser) {
	laser.beam.clear();

	Common::Point pos = getNeighbor(laser.pos, laser.facing);
	CellContents contents = getContents(pos);
	while (contents != kWall && contents != kObject) {
		bool isLoop = false;
		for (const BeamCell &cell : laser.beam) {
			if (cell.pos == pos && cell.direction == laser.facing) {
				isLoop = true;
				break;
			}
		}

		if (isLoop) {
			break;
		}

		BeamCell cell;
		cell.pos = pos;
		cell.direction = laser.facing;
		laser.beam.push_back(cell);

		pos = getNeighbor(pos, laser.facing);
		contents = getContents(pos);
	}
}

bool EscapePuzzle::updateObject(Object &object, uint32 time) {
	object.time += time;
	const ObjectKind &kind = _kinds[object.type];

	if (object.type == kLaser) {
		updateBeam(object);
		return false;
	}

	if (object.movingTo == kNoDirection || object.time < kind.moveTime) {
		return false;
	}

	const Direction direction = object.movingTo;
	const Common::Point next = getNeighbor(object.pos, direction);
	const CellContents contents = getContents(next);
	object.facing = direction;
	object.motion = kStill;

	switch (object.type) {
	case kPlayer:
		object.movingTo = kNoDirection;

		if (contents == kEmpty) {
			object.pos = next;
			object.motion = kMoving;
			object.time = 0;
			playSoundBlock(kind.moveSound);
			return true;
		} else if (contents == kObject) {
			playSoundBlock(kind.pushSound);
			object.motion = kPushing;
			push(next, direction);
			object.time = 0;
			return true;
		}

		playSoundBlock(kind.bumpSound);
		return false;
	case kBall:
		// A ball keeps rolling until something stops it
		if (contents == kEmpty || contents == kLaserBeam) {
			object.pos = next;
			object.motion = kMoving;
			object.time = 0;
			playSoundBlock(kind.moveSound);
			return true;
		}

		object.movingTo = kNoDirection;
		object.time = 0;
		return false;
	default:
		// A box moves one cell
		object.movingTo = kNoDirection;

		if (contents == kEmpty || contents == kLaserBeam) {
			object.pos = next;
			object.motion = kMoving;
			object.time = 0;
			playSoundBlock(kind.moveSound);
			return true;
		}

		return false;
	}
}

void EscapePuzzle::pressButton(Button &button) {
	if (button.isPressed) {
		return;
	}

	Object *player = findObject(_playerName);
	if (player) {
		player->movingTo = button.direction;
	}

	button.isPressed = true;
	button.pressTime = 0;
}

void EscapePuzzle::drawObject(const Object &object) {
	const ObjectKind &kind = _kinds[object.type];
	const DirectionalAnimation &animation = object.motion == kPushing ? kind.secondAnimation : kind.animation;
	const Common::Array<Common::Rect> &frames = animation.frames[object.facing];
	if (frames.empty()) {
		return;
	}

	Common::Point dest = getCellRect(object.pos).origin();
	uint frame = 0;

	if (object.motion != kStill) {
		double progress = kind.moveTime > 0 ? object.time / (double)kind.moveTime : 1.0;
		if (object.motion == kMoving) {
			// On the way over from the cell before
			progress = CLIP(progress, 0.0, 1.0);
			Common::Point from = getCellRect(getNeighbor(object.pos, getOpposite(object.facing))).origin();
			dest.x = from.x + (int)((dest.x - from.x) * progress);
			dest.y = from.y + (int)((dest.y - from.y) * progress);
		}

		frame = MIN<uint>(MAX<int>((int)(progress * frames.size()), 0), frames.size() - 1);
	}

	_drawSurface.blitFrom(_image, frames[frame], dest);

	if (object.type == kLaser) {
		for (const BeamCell &cell : object.beam) {
			const Common::Array<Common::Rect> &beamFrames = kind.secondAnimation.frames[cell.direction];
			if (!beamFrames.empty()) {
				_drawSurface.blitFrom(_image, beamFrames[0], getCellRect(cell.pos).origin());
			}
		}
	}
}

void EscapePuzzle::redraw() {
	_drawSurface.clear(_drawSurface.getTransparentColor());

	for (const Object &object : _objects) {
		drawObject(object);
	}

	for (const Button &button : _buttons) {
		if (button.isPressed) {
			_drawSurface.blitFrom(_image, button.pressedSrc, button.dest.origin());
		}
	}

	_needsRedraw = true;
}

void EscapePuzzle::execute() {
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

		uint32 now = g_nancy->getTotalPlayTime();
		uint32 numTicks = (now - _lastUpdateTime) / kTickLength;
		_lastUpdateTime += numTicks * kTickLength;
		uint32 time = numTicks * kTickLength;

		for (Button &button : _buttons) {
			if (button.isPressed) {
				button.pressTime += time;
				if (button.pressTime > _buttonPressTime) {
					button.isPressed = false;
				}
			}
		}

		// Everything that can move does, until nothing moves any more
		bool hasMoved;
		do {
			hasMoved = false;
			for (Object &object : _objects) {
				hasMoved |= updateObject(object, time);
			}

			time = 0;
		} while (hasMoved);

		for (uint i = 0; i < _goals.size() && _reachedGoal == -1; ++i) {
			Object *object = findObject(_goals[i].objectName);
			if (object && object->pos == _goals[i].pos && object->movingTo == kNoDirection &&
					object->time >= _kinds[object->type].moveTime) {
				_reachedGoal = i;
				_state = kActionTrigger;
			}
		}

		redraw();
		break;
	}
	case kActionTrigger:
		if (_reachedGoal != -1) {
			playSoundBlock(_goals[_reachedGoal].sound);
			_goals[_reachedGoal].scene.execute();
		} else {
			_exitScene.execute();
		}

		finishExecution();
		break;
	}
}

void EscapePuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _exitRequested || _reachedGoal != -1) {
		return;
	}

	Direction heldKey = kNoDirection;
	if (input.input & NancyInput::kMoveUp) {
		heldKey = kNorth;
	} else if (input.input & NancyInput::kMoveDown) {
		heldKey = kSouth;
	} else if (input.input & NancyInput::kMoveRight) {
		heldKey = kEast;
	} else if (input.input & NancyInput::kMoveLeft) {
		heldKey = kWest;
	}

	// A key moves the player once, when it is let go
	if (heldKey != _heldKey) {
		if (_heldKey != kNoDirection) {
			pressButton(_buttons[_heldKey]);
		}

		_heldKey = heldKey;
	}

	Common::Rect viewportBounds = NancySceneState.getViewport().getBounds();
	if (!NancySceneState.getViewport().convertViewportToScreen(viewportBounds).contains(input.mousePos)) {
		return;
	}

	const bool clicked = input.input & NancyInput::kLeftMouseButtonUp;

	Common::Rect mouseRect = NancySceneState.getViewport().convertScreenToViewport(
		Common::Rect(input.mousePos.x, input.mousePos.y, input.mousePos.x + 1, input.mousePos.y + 1));
	Common::Point pos = mouseRect.origin();

	for (Button &button : _buttons) {
		if (!button.dest.contains(pos) || button.isPressed) {
			continue;
		}

		setDataCursor(_buttonCursor);
		if (clicked) {
			pressButton(button);
			input.eatMouseInput();
		}

		return;
	}

	if (hoverExitHotspot(input) && clicked) {
		_exitRequested = true;
		input.eatMouseInput();
	}
}

} // End of namespace Action
} // End of namespace Nancy
