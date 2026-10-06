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
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/movieplayer.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/escapegridpuzzle.h"

namespace Nancy {
namespace Action {

static const uint32 kTickLength = 20;
static const int kTileRefTolerance = 8;

static const Common::Point kNoCell(-1, -1);

// The jump animation of each hex direction.
static const uint kCompassDirection[EscapeGridPuzzle::kNumDirections] = { 0, 1, 3, 4, 5, 7 };

EscapeGridPuzzle::~EscapeGridPuzzle() {
	for (uint i = 0; i < _jumpMovies.size(); ++i) {
		delete _jumpMovies[i];
	}
}

void EscapeGridPuzzle::readAnimation(Common::SeekableReadStream &stream, Animation &dst) {
	readFilename(stream, dst.name);
	int16 num = stream.readSint16LE();
	dst.srcs.resize(num > 0 ? num : 0);
	for (uint i = 0; i < dst.srcs.size(); ++i) {
		readRect(stream, dst.srcs[i]);
	}
}

void EscapeGridPuzzle::readData(Common::SeekableReadStream &stream) {
	_busyCursorType = stream.readUint16LE();
	_hoverCursorType = stream.readUint16LE();
	readFilename(stream, _tileRefName);
	_defaultSinkTime = stream.readSint32LE();
	_defaultRiseTime = stream.readSint32LE();

	int16 numTypes = stream.readSint16LE();
	_tileTypes.resize(numTypes > 0 ? numTypes : 0);
	for (uint i = 0; i < _tileTypes.size(); ++i) {
		TileType &type = _tileTypes[i];
		type.id = stream.readSint32LE();
		readAnimation(stream, type.sink);
		type.sinkSound.readData(stream);
		readAnimation(stream, type.rise);
		type.riseSound.readData(stream);
		readFilename(stream, type.imageName);
		readRect(stream, type.src);
	}

	readRect(stream, _originCell);
	readRect(stream, _rowStepCell);
	readRect(stream, _colStepCell);

	_unknownGridFlag = stream.readByte();
	_numCols = stream.readSint32LE();
	_numRows = stream.readSint32LE();
	_hexDelay = (int32)(stream.readFloatLE() * (1000 / kTickLength) + 0.5f); // in seconds

	uint numCells = (uint)MAX<int32>(0, _numCols) * (uint)MAX<int32>(0, _numRows);
	_grid.resize(numCells);
	for (uint i = 0; i < numCells; ++i) {
		_grid[i] = stream.readSint32LE();
	}

	// Characters are counted in animations, eight each.
	int16 numBinks = stream.readSint16LE();
	if (numBinks % 8 != 0) {
		error("EscapeGridPuzzle: improper number of animations per character type (should be 8)");
	}

	_characters.resize(numBinks / 8);
	for (uint i = 0; i < _characters.size(); ++i) {
		Character &character = _characters[i];
		readFilename(stream, character.imageName);
		readRect(stream, character.src);
		character.offsetX = stream.readSint32LE();
		character.offsetY = stream.readSint32LE();

		// Counted in animations too, three per tile type.
		int16 numTileAnims = stream.readSint16LE();
		if (numTileAnims % 3 != 0) {
			error("EscapeGridPuzzle: unclear amount of character animations on tiles");
		}

		character.tileAnims.resize(numTileAnims / 3);
		for (uint j = 0; j < character.tileAnims.size(); ++j) {
			CharacterTileAnims &tileAnims = character.tileAnims[j];
			tileAnims.tileTypeID = stream.readSint32LE();
			for (uint k = 0; k < kNumCharTileAnims; ++k) {
				readAnimation(stream, tileAnims.anims[k]);
			}
		}

		for (uint j = 0; j < kNumCompassDirections; ++j) {
			readFilename(stream, character.jumpNames[j]);
			character.jumpOffsetX[j] = stream.readSint32LE();
			character.jumpOffsetY[j] = stream.readSint32LE();
		}
	}

	uint16 numActors = stream.readUint16LE();
	_actors.resize(numActors);
	for (uint i = 0; i < _actors.size(); ++i) {
		Actor &actor = _actors[i];
		actor.characterID = stream.readSint32LE();
		actor.start.x = (int16)stream.readSint32LE();
		actor.start.y = (int16)stream.readSint32LE();

		int16 numGoals = stream.readSint16LE();
		actor.goals.resize(numGoals > 0 ? numGoals : 0);
		for (uint j = 0; j < actor.goals.size(); ++j) {
			actor.goals[j].x = (int16)stream.readSint32LE();
			actor.goals[j].y = (int16)stream.readSint32LE();
		}

		actor.isPlayer = (stream.readByte() == 1);
	}

	_solveScene._sceneChange.sceneID = stream.readUint16LE();
	_solveScene._sceneChange.frameID = stream.readUint16LE();
	_solveScene._sceneChange.continueSceneSound = kContinueSceneSound;
	_solveScene._flag.label = stream.readSint16LE();
	_solveScene._flag.flag = stream.readByte();
	_solveSoundBlock.readData(stream);

	_failScene.sceneID = stream.readUint16LE();
	_failScene.frameID = stream.readUint16LE();
	_failScene.continueSceneSound = kContinueSceneSound;
	_failFlag.label = stream.readSint16LE();
	_failFlag.flag = stream.readByte();
	_failSound.readData(stream);

	readExitHotspot(stream);
	_exitScene._sceneChange.continueSceneSound = kContinueSceneSound;
}

static void loadPuzzleImage(const Common::Path &name, Graphics::ManagedSurface &surface, uint32 transColor) {
	if (name.empty()) {
		return;
	}

	g_nancy->_resource->loadImage(name, surface);
	surface.setTransparentColor(transColor);
}

void EscapeGridPuzzle::init() {
	initViewportSurface();

	uint32 transColor = _drawSurface.getTransparentColor();

	if (!_tileRefName.empty()) {
		g_nancy->_resource->loadImage(_tileRefName, _tileRef);
	}

	_tileImages.clear();
	_tileImages.resize(_tileTypes.size());
	for (uint i = 0; i < _tileTypes.size(); ++i) {
		loadPuzzleImage(_tileTypes[i].imageName, _tileImages[i].staticImage, transColor);
		loadPuzzleImage(_tileTypes[i].sink.name, _tileImages[i].sink, transColor);
		loadPuzzleImage(_tileTypes[i].rise.name, _tileImages[i].rise, transColor);
	}

	for (uint i = 0; i < _jumpMovies.size(); ++i) {
		delete _jumpMovies[i];
	}
	_jumpMovies.clear();
	_jumpMovies.resize(_characters.size() * kNumCompassDirections, nullptr);

	_characterImages.clear();
	_characterImages.resize(_characters.size());
	for (uint i = 0; i < _characters.size(); ++i) {
		const Character &character = _characters[i];
		CharacterImages &images = _characterImages[i];
		loadPuzzleImage(character.imageName, images.root, transColor);

		images.tileAnims.resize(character.tileAnims.size() * kNumCharTileAnims);
		for (uint j = 0; j < character.tileAnims.size(); ++j) {
			for (uint k = 0; k < kNumCharTileAnims; ++k) {
				loadPuzzleImage(character.tileAnims[j].anims[k].name, images.tileAnims[j * kNumCharTileAnims + k], transColor);
			}
		}

		for (uint j = 0; j < kNumCompassDirections; ++j) {
			if (character.jumpNames[j].empty()) {
				continue;
			}

			MoviePlayer *movie = new MoviePlayer();
			if (movie->loadFile(character.jumpNames[j])) {
				_jumpMovies[i * kNumCompassDirections + j] = movie;
			} else {
				delete movie;
			}
		}
	}

	_actorCells.resize(_actors.size());
	for (uint i = 0; i < _actors.size(); ++i) {
		_actorCells[i] = _actors[i].start;
	}
	_lastTargets.clear();
	_lastTargets.resize(_actors.size(), kNoCell);
	_previousTargets.clear();
	_previousTargets.resize(_actors.size(), kNoCell);

	_cellAnims.clear();
	_cellAnims.resize(_grid.size());
	_actorAnims.clear();
	_actorAnims.resize(_actors.size());
	_landedJumps.clear();

	_turn = 0;
	_moveIssued = false;
	_slidingPhase = false;
	_clickedCell = kNoCell;
	_jumpFrameMovie = nullptr;
	_jumpFrameIndex = -1;
	_lastTickTime = g_nancy->getTotalPlayTime();
	_outcome = -1;
	_exitRequested = false;

	NancySceneState.setNoHeldItem();

	redraw();
	registerGraphics();
}

// Odd columns hang half a row lower than even ones.
Common::Point EscapeGridPuzzle::step(const Common::Point &cell, Direction dir) {
	bool odd = (cell.x & 1) != 0;

	switch (dir) {
	case kNorth:
		return Common::Point(cell.x, cell.y - 1);
	case kNorthEast:
		return Common::Point(cell.x + 1, odd ? cell.y : cell.y - 1);
	case kSouthEast:
		return Common::Point(cell.x + 1, odd ? cell.y + 1 : cell.y);
	case kSouth:
		return Common::Point(cell.x, cell.y + 1);
	case kSouthWest:
		return Common::Point(cell.x - 1, odd ? cell.y + 1 : cell.y);
	default:
		return Common::Point(cell.x - 1, odd ? cell.y : cell.y - 1);
	}
}

bool EscapeGridPuzzle::directionTo(const Common::Point &from, const Common::Point &to, Direction &dir) {
	for (int i = 0; i < kNumDirections; ++i) {
		if (step(from, (Direction)i) == to) {
			dir = (Direction)i;
			return true;
		}
	}
	return false;
}

// Animations run at 15 frames a second, rounded to whole ticks.
int32 EscapeGridPuzzle::framesToTicks(uint numFrames) {
	return (int32)(numFrames * 10 + 1) / 3;
}

uint EscapeGridPuzzle::frameAt(uint numFrames, int32 elapsed) {
	int32 duration = framesToTicks(numFrames);
	if (duration <= 0) {
		return 0;
	}
	return MIN<uint>(numFrames - 1, (uint)(numFrames * elapsed / duration));
}

Common::Rect EscapeGridPuzzle::cellRect(const Common::Point &cell) const {
	int stepX = _colStepCell.left - _originCell.left;
	int stepY = _rowStepCell.top - _originCell.top;
	int stagger = _colStepCell.top - _originCell.top;

	int left = _originCell.left + cell.x * stepX;
	int top = _originCell.top + cell.y * stepY + ((cell.x & 1) ? stagger : 0);
	return Common::Rect((int16)left, (int16)top,
		(int16)(left + _originCell.width()), (int16)(top + _originCell.height()));
}

bool EscapeGridPuzzle::validCell(const Common::Point &cell) const {
	return cell.x >= 0 && cell.y >= 0 && cell.x < _numCols && cell.y < _numRows;
}

int EscapeGridPuzzle::tileTypeIndex(int32 id) const {
	for (uint i = 0; i < _tileTypes.size(); ++i) {
		if (_tileTypes[i].id == id) {
			return i;
		}
	}
	return -1;
}

const EscapeGridPuzzle::TileType *EscapeGridPuzzle::tileType(int32 id) const {
	int index = tileTypeIndex(id);
	return index == -1 ? nullptr : &_tileTypes[index];
}

const EscapeGridPuzzle::CharacterTileAnims *EscapeGridPuzzle::characterTileAnims(uint actor, int32 tileID, int &index) const {
	int32 character = _actors[actor].characterID;
	if (character < 0 || (uint)character >= _characters.size()) {
		return nullptr;
	}

	const Common::Array<CharacterTileAnims> &tileAnims = _characters[character].tileAnims;
	for (uint i = 0; i < tileAnims.size(); ++i) {
		if (tileAnims[i].tileTypeID == tileID) {
			index = i;
			return &tileAnims[i];
		}
	}
	return nullptr;
}

int EscapeGridPuzzle::actorAt(const Common::Point &cell) const {
	for (uint i = 0; i < _actorCells.size(); ++i) {
		if (_actorCells[i] == cell) {
			return i;
		}
	}
	return -1;
}

bool EscapeGridPuzzle::canStepOnto(const Common::Array<int32> &grid, const Common::Array<Common::Point> &cells,
		const Common::Point &cell) const {
	if (!validCell(cell)) {
		return false;
	}

	int32 id = grid[cell.y * _numCols + cell.x];
	if (!(id & kTileWalkable) || (id & kTileMissing)) {
		return false;
	}

	for (uint i = 0; i < cells.size(); ++i) {
		if (cells[i] == cell) {
			return false;
		}
	}
	return true;
}

// The run of movable hexes ahead of the landing hex, then the run behind it farthest
// first, so the line closes back on the landing hex. Nothing slides, and the line is left
// empty, when there is no movable hex ahead of the one landed on.
void EscapeGridPuzzle::collectLine(const Common::Array<int32> &grid, const Common::Point &from, const Common::Point &to,
		Common::Array<Common::Point> &line) const {
	line.clear();

	Direction dir = kNorth;
	if (!validCell(to) || !(grid[to.y * _numCols + to.x] & kTileMovable) || !directionTo(from, to, dir)) {
		return;
	}

	for (Common::Point p = to; validCell(p) && (grid[p.y * _numCols + p.x] & kTileMovable); p = step(p, dir)) {
		line.push_back(p);
	}

	if (line.size() == 1) {
		line.clear();
		return;
	}

	Direction back = (Direction)((dir + kNumDirections / 2) % kNumDirections);
	Common::Array<Common::Point> behind;
	for (Common::Point p = to; validCell(p) && (grid[p.y * _numCols + p.x] & kTileMovable); p = step(p, back)) {
		behind.push_back(p);
	}
	for (uint i = behind.size(); i-- > 0;) {
		line.push_back(behind[i]);
	}
}

// Every hex takes the place of the one ahead of it, the front one wrapping around to the
// back, and anyone standing on the line rides along.
void EscapeGridPuzzle::rotateLine(Common::Array<int32> &grid, Common::Array<Common::Point> &cells,
		const Common::Array<Common::Point> &line) const {
	int32 carried = grid[line[0].y * _numCols + line[0].x];
	for (uint i = 1; i < line.size(); ++i) {
		SWAP(grid[line[i].y * _numCols + line[i].x], carried);
	}

	for (uint a = 0; a < cells.size(); ++a) {
		for (uint i = 0; i + 1 < line.size(); ++i) {
			if (cells[a] == line[i]) {
				cells[a] = line[i + 1];
				break;
			}
		}
	}
}

// Slides the line on the real board. The hexes sink one after the other along the line;
// each then rises holding the hex that was behind it, once that one has sunk. Characters
// on the line sink and rise with their hex, and the hex itself isn't drawn meanwhile.
void EscapeGridPuzzle::slideBoard(const Common::Point &from, const Common::Point &to) {
	Common::Array<Common::Point> line;
	collectLine(_grid, from, to, line);
	if (line.empty()) {
		return;
	}

	const uint count = line.size();

	for (uint i = 0; i + 1 < count; ++i) {
		const Common::Point &cell = line[i];
		int32 id = tileAt(cell);
		int32 delay = _hexDelay * i;

		if (i > 0) {
			pushTileAnim(cell, kTileStay, id, delay);
		}

		int actor = actorAt(cell);
		if (actor != -1) {
			if (delay > 0) {
				pushActorAnim(actor, kActorStay, cell, 0, delay);
			}
			int32 time = characterSinkTime(actor, id);
			pushActorAnim(actor, i > 0 ? kActorSinkRider : kActorSink, cell, id, time);
			pushTileAnim(cell, kTileBlank, 0, time);
		} else {
			pushTileAnim(cell, kTileSink, id, tileSinkTime(id));
		}
	}

	Common::Array<int32> queued;
	queued.resize(count, 0);
	for (uint i = 1; i < count; ++i) {
		queued[i] = cellQueueTime(line[i]);
	}

	rotateLine(_grid, _actorCells, line);

	for (uint i = 1; i < count; ++i) {
		const Common::Point &cell = line[i];
		int32 id = tileAt(cell);
		int32 delay = MAX<int32>(0, _hexDelay - queued[i] + queued[i - 1]);

		int actor = actorAt(cell);
		if (actor != -1) {
			if (delay > 0) {
				pushActorAnim(actor, kActorStay, cell, 0, delay);
			}
			pushActorAnim(actor, kActorRise, cell, id, characterRiseTime(actor, id));
			pushTileAnim(cell, kTileBlank, 0, MAX<int32>(0, actorQueueTime(actor) - cellQueueTime(cell) - 1));
		} else {
			if (delay > 0) {
				pushTileAnim(cell, kTileBlank, 0, delay);
			}
			pushTileAnim(cell, kTileRise, id, tileRiseTime(id));
		}
	}
}

int32 EscapeGridPuzzle::tileSinkTime(int32 tileID) const {
	const TileType *type = tileType(tileID);
	return (type && !type->sink.srcs.empty()) ? framesToTicks(type->sink.srcs.size()) : _defaultSinkTime;
}

int32 EscapeGridPuzzle::tileRiseTime(int32 tileID) const {
	const TileType *type = tileType(tileID);
	return (type && !type->rise.srcs.empty()) ? framesToTicks(type->rise.srcs.size()) : _defaultRiseTime;
}

// A character without animations for the tile takes the default sink time either way.
int32 EscapeGridPuzzle::characterSinkTime(uint actor, int32 tileID) const {
	int index;
	const CharacterTileAnims *anims = characterTileAnims(actor, tileID, index);
	if (!anims || anims->anims[kCharSink].srcs.empty()) {
		return _defaultSinkTime;
	}
	return framesToTicks(anims->anims[kCharSink].srcs.size());
}

int32 EscapeGridPuzzle::characterRiseTime(uint actor, int32 tileID) const {
	int index;
	const CharacterTileAnims *anims = characterTileAnims(actor, tileID, index);
	if (!anims) {
		return _defaultSinkTime;
	}
	if (anims->anims[kCharRise].srcs.empty()) {
		return _defaultRiseTime;
	}
	return framesToTicks(anims->anims[kCharRise].srcs.size());
}

void EscapeGridPuzzle::pushTileAnim(const Common::Point &cell, TileAnimKind kind, int32 tileID, int32 time) {
	TileAnim anim;
	anim.kind = kind;
	anim.tileID = tileID;
	anim.total = anim.remaining = time;
	_cellAnims[cell.y * _numCols + cell.x].push_back(anim);
}

void EscapeGridPuzzle::pushActorAnim(uint actor, ActorAnimKind kind, const Common::Point &cell, int32 tileID, int32 time) {
	ActorAnim anim;
	anim.kind = kind;
	anim.from = anim.to = cell;
	anim.tileID = tileID;
	anim.total = anim.remaining = time;
	_actorAnims[actor].push_back(anim);
}

int32 EscapeGridPuzzle::cellQueueTime(const Common::Point &cell) const {
	const Common::Array<TileAnim> &queue = _cellAnims[cell.y * _numCols + cell.x];
	int32 time = 0;
	for (uint i = 0; i < queue.size(); ++i) {
		time += queue[i].remaining;
	}
	return time;
}

int32 EscapeGridPuzzle::actorQueueTime(uint actor) const {
	const Common::Array<ActorAnim> &queue = _actorAnims[actor];
	int32 time = 0;
	for (uint i = 0; i < queue.size(); ++i) {
		time += queue[i].remaining;
	}
	return time;
}

bool EscapeGridPuzzle::actorsAnimating() const {
	for (uint i = 0; i < _actorAnims.size(); ++i) {
		if (!_actorAnims[i].empty()) {
			return true;
		}
	}
	return false;
}

// Only the animation at the front of each queue advances. A finished jump is kept, so
// that its line gets slid once everyone has come to rest.
void EscapeGridPuzzle::updateActorAnims(int32 ticks) {
	for (uint i = 0; i < _actorAnims.size(); ++i) {
		Common::Array<ActorAnim> &queue = _actorAnims[i];
		if (queue.empty()) {
			continue;
		}

		ActorAnim &anim = queue[0];
		if (anim.remaining == anim.total && anim.kind != kActorStay && anim.kind != kActorJump) {
			const TileType *type = tileType(anim.tileID);
			if (type) {
				playSoundBlock(anim.kind == kActorRise ? type->riseSound : type->sinkSound);
			}
		}

		anim.remaining = MAX<int32>(0, anim.remaining - ticks);
		if (anim.remaining == 0) {
			if (anim.from != anim.to) {
				_landedJumps.push_back(anim);
			}
			queue.remove_at(0);
		}
	}
}

// Returns whether any hex was still moving.
bool EscapeGridPuzzle::updateTileAnims(int32 ticks) {
	bool moving = false;

	for (uint i = 0; i < _cellAnims.size(); ++i) {
		Common::Array<TileAnim> &queue = _cellAnims[i];
		if (queue.empty()) {
			continue;
		}

		moving = true;
		TileAnim &anim = queue[0];
		if (anim.remaining == anim.total && (anim.kind == kTileSink || anim.kind == kTileRise)) {
			const TileType *type = tileType(anim.tileID);
			if (type) {
				playSoundBlock(anim.kind == kTileRise ? type->riseSound : type->sinkSound);
			}
		}

		anim.remaining = MAX<int32>(0, anim.remaining - ticks);
		if (anim.remaining == 0) {
			queue.remove_at(0);
		}
	}

	return moving;
}

bool EscapeGridPuzzle::hasAnyMove(uint actor) const {
	for (int i = 0; i < kNumDirections; ++i) {
		if (canStepOnto(_grid, _actorCells, step(_actorCells[actor], (Direction)i))) {
			return true;
		}
	}
	return false;
}

// Looks up to two jumps ahead for whatever gets closest to a goal; the first jump of the
// best line is taken. Failing that, a random neighboring hex is picked.
Common::Point EscapeGridPuzzle::chooseOpponentMove(uint actor) {
	Common::Array<int32> grid = _grid;
	Common::Array<Common::Point> cells = _actorCells;
	Common::Array<Common::Point> path;
	Common::Array<Common::Point> bestPath;
	int32 bestDist = 0x7FFFFFFF;

	searchPath(actor, 0, grid, cells, path, bestDist, bestPath);

	Common::Point target = kNoCell;
	if (!bestPath.empty() && bestPath[0] != _previousTargets[actor]) {
		target = bestPath[0];
	} else {
		Common::Array<Common::Point> candidates;
		for (int i = 0; i < kNumDirections; ++i) {
			candidates.push_back(step(_actorCells[actor], (Direction)i));
		}

		while (!candidates.empty()) {
			uint i = g_nancy->_randomSource->getRandomNumber(candidates.size() - 1);
			if (candidates[i] == _previousTargets[actor] && candidates.size() > 1) {
				continue;
			}
			if (canStepOnto(_grid, _actorCells, candidates[i])) {
				target = candidates[i];
				break;
			}
			candidates.remove_at(i);
		}
	}

	_previousTargets[actor] = _lastTargets[actor];
	_lastTargets[actor] = target;
	return target;
}

// Distance is counted in board columns plus rows. A tie goes to the shorter line.
void EscapeGridPuzzle::searchPath(uint actor, int depth, Common::Array<int32> &grid, Common::Array<Common::Point> &cells,
		Common::Array<Common::Point> &path, int32 &bestDist, Common::Array<Common::Point> &bestPath) const {
	const Common::Point pos = cells[actor];

	const Common::Array<Common::Point> &goals = _actors[actor].goals;
	for (uint i = 0; i < goals.size(); ++i) {
		int32 dist = ABS(goals[i].y - pos.y) + ABS(goals[i].x - pos.x);
		if ((dist < bestDist && !path.empty()) || (dist == bestDist && path.size() < bestPath.size())) {
			bestDist = dist;
			bestPath = path;
		}
	}

	if (depth == 2 || bestDist == 0) {
		return;
	}

	for (int i = 0; i < kNumDirections; ++i) {
		Common::Point next = step(pos, (Direction)i);
		if (!canStepOnto(grid, cells, next)) {
			continue;
		}

		Common::Array<int32> nextGrid = grid;
		Common::Array<Common::Point> nextCells = cells;
		nextCells[actor] = next;

		Common::Array<Common::Point> line;
		collectLine(nextGrid, pos, next, line);
		if (!line.empty()) {
			rotateLine(nextGrid, nextCells, line);
		}

		path.push_back(next);
		searchPath(actor, depth + 1, nextGrid, nextCells, path, bestDist, bestPath);
		path.pop_back();
	}
}

// A target off the board means the actor has nowhere to go and sits the turn out.
void EscapeGridPuzzle::startMove(const Common::Point &target) {
	Direction dir = kNorth;
	const Common::Point from = _actorCells[_turn];
	if (!validCell(target) || !directionTo(from, target, dir)) {
		return;
	}

	uint compassDir = kCompassDirection[dir];
	const MoviePlayer *movie = _jumpMovies[_actors[_turn].characterID * kNumCompassDirections + compassDir];

	ActorAnim anim;
	anim.kind = kActorJump;
	anim.from = from;
	anim.to = target;
	anim.compassDir = compassDir;
	anim.total = anim.remaining = framesToTicks(movie ? movie->getFrameCount() : 0);
	_actorAnims[_turn].push_back(anim);

	_actorCells[_turn] = target;
}

// One step of the turn cycle: the actor to move jumps, the line it landed on slides once
// everyone has come to rest, and the turn passes on once the hexes have settled.
void EscapeGridPuzzle::updateTurn(int32 ticks) {
	updateActorAnims(ticks);
	bool animating = actorsAnimating();

	if (_moveIssued) {
		if (animating) {
			return;
		}

		int outcome = checkOutcome();
		if (outcome != -1) {
			endPuzzle(outcome);
			return;
		}

		_moveIssued = false;
		_slidingPhase = true;
		for (uint i = 0; i < _landedJumps.size(); ++i) {
			slideBoard(_landedJumps[i].from, _landedJumps[i].to);
		}
		_landedJumps.clear();
		return;
	}

	if (_slidingPhase) {
		_slidingPhase = updateTileAnims(ticks);
		if (!_slidingPhase) {
			_turn = (_turn + 1) % _actors.size();

			int outcome = checkOutcome();
			if (outcome != -1) {
				endPuzzle(outcome);
			}
		}
		return;
	}

	if (animating) {
		return;
	}

	if (!_actors[_turn].isPlayer) {
		startMove(chooseOpponentMove(_turn));
	} else if (_clickedCell != kNoCell) {
		startMove(_clickedCell);
		_clickedCell = kNoCell;
	} else if (!hasAnyMove(_turn)) {
		startMove(kNoCell);
	} else {
		return;
	}

	_moveIssued = true;
}

// An opponent reaching its goal beats the player reaching theirs at the same time.
int EscapeGridPuzzle::checkOutcome() const {
	bool playerArrived = false;
	bool opponentArrived = false;

	for (uint i = 0; i < _actors.size(); ++i) {
		for (uint j = 0; j < _actors[i].goals.size(); ++j) {
			if (_actorCells[i] == _actors[i].goals[j]) {
				if (_actors[i].isPlayer) {
					playerArrived = true;
				} else {
					opponentArrived = true;
				}
				break;
			}
		}
	}

	if (opponentArrived) {
		return 1;
	}
	return playerArrived ? 0 : -1;
}

void EscapeGridPuzzle::endPuzzle(int outcome) {
	_outcome = outcome;
	_endSound = playSoundBlock(outcome == 0 ? _solveSoundBlock : _failSound);
}

bool EscapeGridPuzzle::tileRefColor(const Common::Point &cell, const Common::Point &pos, byte &r, byte &g, byte &b) const {
	if (_tileRef.empty()) {
		return false;
	}

	Common::Rect rect = cellRect(cell);
	int x = pos.x - rect.left;
	int y = pos.y - rect.top;
	if (x < 0 || y < 0 || x >= _tileRef.w || y >= _tileRef.h) {
		return false;
	}

	_tileRef.format.colorToRGB(_tileRef.getPixel(x, y), r, g, b);
	return true;
}

static bool colorNear(byte r, byte g, byte b, byte refR, byte refG, byte refB) {
	return ABS(r - refR) < kTileRefTolerance && ABS(g - refG) < kTileRefTolerance && ABS(b - refB) < kTileRefTolerance;
}

bool EscapeGridPuzzle::isCellPixel(const Common::Point &cell, const Common::Point &pos) const {
	byte r, g, b;
	return validCell(cell) && tileRefColor(cell, pos, r, g, b) && colorNear(r, g, b, 0xff, 0xff, 0xff);
}

// The cursor is first placed in a cell by its bounding box alone. The hit mask then tells
// whether it is really over that hex (white) or over one of the four diagonal neighbors
// overlapping its corners: red for north-east, black for south-east, green for south-west
// and blue for north-west. The target has to be a hex the player can jump onto.
bool EscapeGridPuzzle::findHoveredCell(const Common::Point &mousePos, Common::Point &outCell) const {
	Common::Rect mouseRect = NancySceneState.getViewport().convertScreenToViewport(
		Common::Rect(mousePos.x, mousePos.y, mousePos.x + 1, mousePos.y + 1));
	Common::Point pos(mouseRect.left, mouseRect.top);

	int stepX = _colStepCell.left - _originCell.left;
	int stepY = _rowStepCell.top - _originCell.top;
	int stagger = _colStepCell.top - _originCell.top;

	int dx = pos.x - _originCell.left;
	if (dx < 0) {
		dx -= stepX;
	}
	int col = dx / stepX;

	int dy = pos.y - _originCell.top - ((col & 1) ? stagger : 0);
	if (dy < 0) {
		dy -= stepY;
	}
	int row = dy / stepY;

	Common::Point cell(col, row);
	if (!isCellPixel(cell, pos)) {
		byte r, g, b;
		if (!tileRefColor(cell, pos, r, g, b)) {
			return false;
		}

		if (colorNear(r, g, b, 0xff, 0, 0)) {
			cell = step(cell, kNorthEast);
		} else if (colorNear(r, g, b, 0, 0, 0)) {
			cell = step(cell, kSouthEast);
		} else if (colorNear(r, g, b, 0, 0xff, 0)) {
			cell = step(cell, kSouthWest);
		} else if (colorNear(r, g, b, 0, 0, 0xff)) {
			cell = step(cell, kNorthWest);
		} else {
			return false;
		}

		if (!isCellPixel(cell, pos)) {
			return false;
		}
	}

	Direction dir;
	if (!directionTo(_actorCells[_turn], cell, dir) || !canStepOnto(_grid, _actorCells, cell)) {
		return false;
	}

	outCell = cell;
	return true;
}

void EscapeGridPuzzle::drawAnimFrame(const Graphics::ManagedSurface &image, const Animation &anim, int32 elapsed,
		const Common::Point &pos) {
	if (image.empty() || anim.srcs.empty()) {
		return;
	}

	_drawSurface.blitFrom(image, anim.srcs[frameAt(anim.srcs.size(), elapsed)], pos);
}

void EscapeGridPuzzle::drawJump(uint actor, const ActorAnim &anim) {
	const Character &character = _characters[_actors[actor].characterID];
	MoviePlayer *movie = _jumpMovies[_actors[actor].characterID * kNumCompassDirections + anim.compassDir];
	if (!movie || movie->getFrameCount() <= 0) {
		return;
	}

	int frame = frameAt(movie->getFrameCount(), anim.total - anim.remaining);
	if (movie != _jumpFrameMovie || frame != _jumpFrameIndex) {
		const Graphics::Surface *decoded = movie->decodeNextFrame(frame);
		if (!decoded) {
			return;
		}

		GraphicsManager::copyToManaged(*decoded, _jumpFrame);
		_jumpFrame.setTransparentColor(_drawSurface.getTransparentColor());
		_jumpFrameMovie = movie;
		_jumpFrameIndex = frame;
	}

	Common::Rect from = cellRect(anim.from);
	_drawSurface.blitFrom(_jumpFrame, Common::Point(from.left + character.jumpOffsetX[anim.compassDir],
		from.top + character.jumpOffsetY[anim.compassDir]));
}

void EscapeGridPuzzle::drawActor(uint actor) {
	int32 id = _actors[actor].characterID;
	if (id < 0 || (uint)id >= _characters.size()) {
		return;
	}

	const Character &character = _characters[id];
	const CharacterImages &images = _characterImages[id];
	const Common::Array<ActorAnim> &queue = _actorAnims[actor];

	if (!queue.empty() && queue[0].kind == kActorJump) {
		drawJump(actor, queue[0]);
		return;
	}

	Common::Rect cell = cellRect(queue.empty() ? _actorCells[actor] : queue[0].from);
	Common::Point pos(cell.left + character.offsetX, cell.top + character.offsetY);

	if (queue.empty() || queue[0].kind == kActorStay) {
		if (!images.root.empty()) {
			_drawSurface.blitFrom(images.root, character.src, pos);
		}
		return;
	}

	const ActorAnim &anim = queue[0];
	int index;
	const CharacterTileAnims *anims = characterTileAnims(actor, anim.tileID, index);
	if (!anims) {
		return;
	}

	uint which = anim.kind == kActorRise ? kCharRise : (anim.kind == kActorSinkRider ? kCharSinkRider : kCharSink);
	drawAnimFrame(images.tileAnims[index * kNumCharTileAnims + which], anims->anims[which], anim.total - anim.remaining, pos);
}

void EscapeGridPuzzle::drawCell(const Common::Point &cell) {
	const Common::Array<TileAnim> &queue = _cellAnims[cell.y * _numCols + cell.x];
	Common::Rect dest = cellRect(cell);
	Common::Point pos(dest.left, dest.top);

	int32 id = queue.empty() ? tileAt(cell) : queue[0].tileID;
	int typeIndex = tileTypeIndex(id);
	if (typeIndex == -1) {
		return;
	}

	const TileType &type = _tileTypes[typeIndex];
	const TileImages &images = _tileImages[typeIndex];
	TileAnimKind kind = queue.empty() ? kTileStay : queue[0].kind;

	switch (kind) {
	case kTileStay:
		if (!images.staticImage.empty() && !type.src.isEmpty()) {
			_drawSurface.blitFrom(images.staticImage, type.src, pos);
		}
		break;
	case kTileSink:
		drawAnimFrame(images.sink, type.sink, queue[0].total - queue[0].remaining, pos);
		break;
	case kTileRise:
		drawAnimFrame(images.rise, type.rise, queue[0].total - queue[0].remaining, pos);
		break;
	default:
		break;
	}
}

void EscapeGridPuzzle::redraw() {
	_drawSurface.clear(g_nancy->_graphics->getTransColor());

	for (int row = 0; row < _numRows; ++row) {
		for (int col = 0; col < _numCols; ++col) {
			drawCell(Common::Point(col, row));
		}
	}

	for (uint i = 0; i < _actors.size(); ++i) {
		drawActor(i);
	}

	_needsRedraw = true;
}

void EscapeGridPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		_state = kRun;
		// fall through
	case kRun: {
		if (_exitRequested) {
			_state = kActionTrigger;
			break;
		}

		if (_outcome != -1) {
			if (_endSound.name.empty() || !g_nancy->_sound->isSoundPlaying(_endSound)) {
				_state = kActionTrigger;
			}
			break;
		}

		uint32 ticks = (g_nancy->getTotalPlayTime() - _lastTickTime) / kTickLength;
		if (ticks == 0) {
			break;
		}

		_lastTickTime += ticks * kTickLength;
		updateTurn(ticks);
		redraw();
		break;
	}
	case kActionTrigger:
		if (_exitRequested) {
			_exitScene.execute();
		} else if (_outcome == 0) {
			_solveScene.execute();
		} else {
			NancySceneState.setEventFlag(_failFlag);
			NancySceneState.changeScene(_failScene);
		}

		finishExecution();
		break;
	}
}

void EscapeGridPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _outcome != -1 || _exitRequested) {
		return;
	}

	const bool click = (input.input & NancyInput::kLeftMouseButtonUp) != 0;
	const bool playerToMove = _actors[_turn].isPlayer && !_moveIssued && !_slidingPhase && !actorsAnimating();

	if (playerToMove) {
		Common::Point cell;
		if (findHoveredCell(input.mousePos, cell)) {
			g_nancy->_cursor->setCursorType((CursorManager::CursorType)_hoverCursorType, true, true);
			if (click) {
				_clickedCell = cell;
			}
			input.eatMouseInput();
			return;
		}
	} else {
		g_nancy->_cursor->setCursorType((CursorManager::CursorType)_busyCursorType, true, false);
	}

	if (isExitHotspotHovered(input)) {
		if (playerToMove) {
			setExitCursor();
		}
		if (click) {
			_exitRequested = true;
		}
	}
}

} // End of namespace Action
} // End of namespace Nancy
