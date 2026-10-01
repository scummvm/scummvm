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
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/wolfmazepuzzle.h"

namespace Nancy {
namespace Action {

static const uint kRowLength = 50;

Common::Point WolfMazePuzzle::step(const Common::Point &cell, int direction) {
	static const int8 kSteps[kNumDirections][2] = {
		{ 0, -1 }, { 1, -1 }, { 1, 0 }, { 1, 1 }, { 0, 1 }, { -1, 1 }, { -1, 0 }, { -1, -1 }
	};

	return Common::Point(cell.x + kSteps[direction][0], cell.y + kSteps[direction][1]);
}

WolfMazePuzzle::Command WolfMazePuzzle::commandFromName(const Common::String &name) {
	static const char *const kNames[] = { "Right", "Left", "Forward", "Backward", "Around", "Bark", "Jump", "Paw" };

	for (uint i = 0; i < ARRAYSIZE(kNames); ++i) {
		if (name.equalsIgnoreCase(kNames[i])) {
			return (Command)(kRight + i);
		}
	}

	return kCommandNone;
}

void WolfMazePuzzle::readEnding(Common::SeekableReadStream &stream, Ending &ending) {
	ending.scene._sceneChange.sceneID = stream.readUint16LE();
	ending.scene._sceneChange.frameID = stream.readUint16LE();
	ending.scene._sceneChange.continueSceneSound = kContinueSceneSound;
	ending.scene._flag.label = stream.readSint16LE();
	ending.scene._flag.flag = stream.readByte();
	ending.sound.readData(stream);
}

bool WolfMazePuzzle::isValid(const Common::Point &cell) const {
	return cell.x >= 0 && cell.y >= 0 && cell.x < (int)_width && cell.y < (int)_height;
}

bool WolfMazePuzzle::hasNeighbor(const Common::Point &cell, int direction) const {
	return typeAt(step(cell, direction)) != kEmpty;
}

int WolfMazePuzzle::countNeighbors(const Common::Point &cell, bool orthogonalOnly) const {
	int count = 0;
	for (int direction = 0; direction < kNumDirections; direction += orthogonalOnly ? 2 : 1) {
		if (hasNeighbor(cell, direction)) {
			++count;
		}
	}

	return count;
}

void WolfMazePuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _playlistName);
	_cursor = stream.readUint16LE();

	// The map is drawn with characters: ' ' is empty, '*' a path, 'L' a path that shows
	// tracks, 'P' a pit, 'S' a switch, 'B' a barricade and 'E' the exit
	Common::Array<Common::String> rows(MAX<int16>(stream.readSint16LE(), 0));
	for (Common::String &row : rows) {
		char buf[kRowLength + 1];
		stream.read(buf, kRowLength);
		buf[kRowLength] = '\0';
		row = buf;
	}

	_height = rows.size();
	_width = _height ? rows[0].size() : 0;

	Common::Rect firstCell, nextColumn, nextRow;
	readRect(stream, firstCell);
	readRect(stream, nextColumn);
	readRect(stream, nextRow);
	readFilename(stream, _imageName);
	readRect(stream, _trackSrc);

	_cells.resize(_width * _height);
	for (uint y = 0; y < _height; ++y) {
		if (rows[y].size() != _width) {
			error("WolfMazePuzzle: grid rows are of different length");
		}

		for (uint x = 0; x < _width; ++x) {
			Cell &cell = _cells[y * _width + x];
			cell.dest = firstCell;
			cell.dest.translate(x * (nextColumn.left - firstCell.left), y * (nextRow.top - firstCell.top));

			switch (rows[y][x]) {
			case ' ':
				cell.type = kEmpty;
				break;
			case '*':
				cell.type = kPath;
				break;
			case 'L':
			case 'l':
				cell.type = kTrack;
				break;
			case 'P':
			case 'p':
				cell.type = kPit;
				break;
			case 'S':
			case 's':
				cell.type = kSwitch;
				break;
			case 'B':
			case 'b':
				cell.type = kBarricadeAcross;
				break;
			case 'E':
			case 'e':
				cell.type = kExit;
				break;
			default:
				error("WolfMazePuzzle: unknown tile '%c'", rows[y][x]);
			}
		}
	}

	for (uint i = 0; i < 2; ++i) {
		int16 count = stream.readSint16LE();
		for (int16 j = 0; j < count; ++j) {
			Common::Point pos;
			pos.x = stream.readSint32LE();
			pos.y = stream.readSint32LE();
			FlagDescription flag;
			flag.label = stream.readSint16LE();
			flag.flag = stream.readByte();
			if (isValid(pos)) {
				cellAt(pos).flag = flag;
			}
		}
	}

	// Paths that branch off stop the wolf, and barricades face across their path
	for (uint y = 0; y < _height; ++y) {
		for (uint x = 0; x < _width; ++x) {
			Common::Point pos(x, y);
			Cell &cell = cellAt(pos);
			if (cell.type == kPath && countNeighbors(pos, true) > 2) {
				cell.type = kJunction;
			} else if (cell.type == kBarricadeAcross && hasNeighbor(pos, kNorth) && hasNeighbor(pos, kSouth)) {
				cell.type = kBarricadeUpDown;
			}
		}
	}

	_wolfStart.x = stream.readSint32LE();
	_wolfStart.y = stream.readSint32LE();
	Common::Point facing;
	facing.x = stream.readSint32LE();
	facing.y = stream.readSint32LE();
	for (int direction = 0; direction < kNumDirections; ++direction) {
		if (step(_wolfStart, direction) == facing) {
			_wolfStartDirection = direction;
		}
	}

	_moveTime = (uint32)(stream.readFloatLE() * 1000);
	_returnTime = (uint32)(stream.readFloatLE() * 1000);

	int16 numMovies = stream.readSint16LE();
	for (int16 i = 0; i < numMovies; ++i) {
		Common::String movie, command;
		readFilename(stream, movie);
		readFilename(stream, command);
		_movieCommands.setVal(movie, commandFromName(command));
	}

	_commandSound.readData(stream);
	_blockedSound.readData(stream);
	_invalidSound.readData(stream);
	_runSound.readData(stream);
	readEnding(stream, _failEnding);
	readEnding(stream, _solveEnding);

	readExitHotspot(stream);
}

void WolfMazePuzzle::init() {
	initViewportSurface();
	loadImage();

	// The commands WolfCommands left for the wolf
	Common::Array<Common::String> movies;
	NancySceneState.takeMoviePlaylist(_playlistName, movies);
	for (const Common::String &movie : movies) {
		if (_movieCommands.contains(movie)) {
			_commands.push_back(_movieCommands[movie]);
		} else {
			warning("WolfMazePuzzle: movie %s isn't mapped to a command", movie.c_str());
		}
	}

	_direction = _wolfStartDirection;
	enterCell(_wolfStart);
	_lastTime = g_nancy->getTotalPlayTime();
	redraw();
}

void WolfMazePuzzle::enterCell(const Common::Point &cell) {
	_wolf = cell;
	++cellAt(cell).visits;
	_track.push_back(cell);
}

// Where the path ahead ends, the wolf turns to the first other way she didn't come from
void WolfMazePuzzle::followBend() {
	if (typeAt(step(_wolf, _direction)) != kEmpty) {
		return;
	}

	int back = (_direction + kNumDirections / 2) % kNumDirections;
	for (int direction = 0; direction < kNumDirections; ++direction) {
		if (direction != back && hasNeighbor(_wolf, direction)) {
			_direction = direction;
			return;
		}
	}
}

WolfMazePuzzle::WolfStatus WolfMazePuzzle::updateWolf(uint32 elapsed) {
	if (_isMoving) {
		_timer += elapsed;
		if (_timer < _moveTime) {
			return kRunning;
		}

		_timer = 0;
		followBend();

		Common::Point ahead = step(_wolf, _direction);
		switch (typeAt(ahead)) {
		case kPit:
			if (!_isJumping) {
				break;
			}

			// fall through
		case kPath:
		case kTrack:
			_isJumping = false;
			enterCell(ahead);
			return kRunning;
		case kJunction:
			enterCell(ahead);
			_isMoving = false;
			return kDone;
		case kExit:
			enterCell(ahead);
			_isMoving = false;
			return kExited;
		case kBarricadeAcross:
		case kBarricadeUpDown: {
			bool isInTheWay = typeAt(ahead) == kBarricadeAcross ?
				(_direction == kEast || _direction == kWest) : (_direction == kNorth || _direction == kSouth);
			const FlagDescription &flag = cellAt(ahead).flag;
			if (isInTheWay && !NancySceneState.getEventFlag(flag.label, flag.flag)) {
				_isMoving = false;
				return kBlocked;
			}

			enterCell(ahead);
			return kRunning;
		}
		default:
			break;
		}

		_isMoving = false;
		return kDone;
	}

	if (_isReturning) {
		_timer += elapsed;
		if (_timer >= _returnTime) {
			_timer = 0;
			if (_track.empty()) {
				_isReturning = false;
				return kReturned;
			}

			_wolf = _track.back();
			--cellAt(_wolf).visits;
			_track.pop_back();
		}

		return kRunning;
	}

	return _track.empty() ? kReturned : kDone;
}

// Returns false when the wolf can't carry the command out
bool WolfMazePuzzle::runCommand(Command command) {
	Common::Point ahead = step(_wolf, _direction);

	switch (command) {
	case kAround:
	case kBark:
		return true;
	case kJump:
		if (typeAt(ahead) != kPit) {
			return false;
		}

		_isMoving = true;
		_isJumping = true;
		return true;
	case kPaw:
		if (typeAt(ahead) != kSwitch) {
			return false;
		}

		NancySceneState.setEventFlag(cellAt(ahead).flag);
		return true;
	case kReturn:
		_isReturning = true;
		return true;
	case kRight:
	case kLeft:
	case kForward:
	case kBackward: {
		// Moves are relative to where the wolf is facing
		static const int kTurns[] = { 2, 6, 0, 4 };
		int direction = (_direction + kTurns[command - kRight]) % kNumDirections;
		TileType type = typeAt(step(_wolf, direction));
		if (type == kEmpty || type == kPit || type == kSwitch) {
			return false;
		}

		_direction = direction;
		_isMoving = true;
		return true;
	}
	default:
		return false;
	}
}

void WolfMazePuzzle::startEnding(const Ending &ending) {
	_isEnding = true;
	_endScene = ending.scene;
	_endSound = playSoundBlock(ending.sound);
}

void WolfMazePuzzle::redraw() {
	_drawSurface.clear(_drawSurface.getTransparentColor());

	for (const Cell &cell : _cells) {
		if (cell.type == kTrack && cell.visits > 0) {
			_drawSurface.blitFrom(_image, _trackSrc, cell.dest.origin());
		}
	}

	_needsRedraw = true;
}

void WolfMazePuzzle::execute() {
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

		if (_isEnding) {
			if (!g_nancy->_sound->isSoundPlaying(_endSound)) {
				_state = kActionTrigger;
			}

			break;
		}

		uint32 now = g_nancy->getTotalPlayTime();
		uint32 elapsed = now - _lastTime;
		_lastTime = now;

		switch (updateWolf(elapsed)) {
		case kDone:
			if (_commands.empty()) {
				runCommand(kReturn);
			} else {
				Command command = _commands.remove_at(0);
				if (runCommand(command)) {
					playSoundBlock(_commandSound);
				} else {
					playSoundBlock(_invalidSound);
					_commands.clear();
				}
			}

			break;
		case kRunning:
			if (!g_nancy->_sound->isSoundPlaying(_runSoundDesc)) {
				_runSoundDesc = playSoundBlock(_runSound);
			}

			break;
		case kExited:
			startEnding(_solveEnding);
			break;
		case kBlocked:
			playSoundBlock(_blockedSound);
			_commands.clear();
			runCommand(kReturn);
			break;
		case kReturned:
			if (_commands.empty()) {
				startEnding(_failEnding);
			}

			break;
		}

		redraw();
		break;
	}
	case kActionTrigger:
		if (_exitRequested) {
			_exitScene.execute();
		} else {
			_endScene.execute();
		}

		finishExecution();
		break;
	}
}

void WolfMazePuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _isEnding || _exitRequested) {
		return;
	}

	Common::Rect viewportBounds = NancySceneState.getViewport().getBounds();
	if (!NancySceneState.getViewport().convertViewportToScreen(viewportBounds).contains(input.mousePos)) {
		return;
	}

	if (hoverExitHotspot(input)) {
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_exitRequested = true;
			input.eatMouseInput();
		}

		return;
	}

	setDataCursor(_cursor);
}

} // End of namespace Action
} // End of namespace Nancy
