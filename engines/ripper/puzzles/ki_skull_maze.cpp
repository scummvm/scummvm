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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/puzzles/ki_skull_maze.h"

#include "common/debug.h"
#include "common/file.h"
#include "common/formats/ini-file.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/util.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"
#include "video/smk_decoder.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/media.h"
#include "ripper/milestones.h"
#include "ripper/modal_dialog.h"
#include "ripper/ripper.h"
#include "ripper/settings.h"
#include "ripper/toolbar.h"

namespace Ripper {

namespace {

static const uint kDefaultCursor = 14;
static const uint kSelectionCursor = 16;
static const uint kHelpSelectionTable = 0x1b9;
static const uint16 kEscapeCommand = 0x1b;
static const uint16 kHelpCommand = 0x3b00;
static const uint kDosTickMillis = 55;
static const uint kTerminalDelayTicks = 0x36;
static const uint kFadeSteps = 9;
static const byte kSkullTransparentColor = 0xff;
static const byte kManTransparentColor = 0x2f;
static const char kCompletionKeyword[] = "pretzel";

// RunKiSkullMazePuzzleScene at 0x2ff55 creates its 64 controls from
// g_kiPuzzleTileYPositions at 0x844f4 and g_kiPuzzleTileXPositions at
// 0x84574 after cancelling the scene origin. These are physical points.
static const Common::Point kCellPositions[KiSkullMazeModel::kCellCount] = {
	Common::Point(47, 180), Common::Point(76, 207),
	Common::Point(107, 234), Common::Point(137, 261),
	Common::Point(167, 288), Common::Point(198, 316),
	Common::Point(231, 344), Common::Point(261, 372),
	Common::Point(88, 160), Common::Point(117, 187),
	Common::Point(147, 214), Common::Point(177, 241),
	Common::Point(208, 268), Common::Point(239, 296),
	Common::Point(270, 323), Common::Point(301, 351),
	Common::Point(128, 140), Common::Point(157, 167),
	Common::Point(187, 194), Common::Point(218, 221),
	Common::Point(249, 248), Common::Point(280, 275),
	Common::Point(312, 303), Common::Point(344, 331),
	Common::Point(169, 121), Common::Point(198, 147),
	Common::Point(229, 174), Common::Point(259, 201),
	Common::Point(290, 228), Common::Point(321, 255),
	Common::Point(353, 282), Common::Point(384, 310),
	Common::Point(209, 101), Common::Point(240, 128),
	Common::Point(270, 154), Common::Point(300, 181),
	Common::Point(331, 208), Common::Point(362, 235),
	Common::Point(393, 262), Common::Point(425, 290),
	Common::Point(250, 82), Common::Point(280, 108),
	Common::Point(310, 134), Common::Point(340, 161),
	Common::Point(371, 187), Common::Point(402, 215),
	Common::Point(433, 242), Common::Point(465, 269),
	Common::Point(289, 62), Common::Point(319, 88),
	Common::Point(350, 114), Common::Point(380, 141),
	Common::Point(411, 168), Common::Point(442, 195),
	Common::Point(474, 222), Common::Point(505, 249),
	Common::Point(328, 43), Common::Point(358, 69),
	Common::Point(389, 95), Common::Point(420, 121),
	Common::Point(451, 148), Common::Point(482, 175),
	Common::Point(514, 202), Common::Point(546, 229)
};

// The first two five-short tables at 0x2ea0e and 0x2ea18 are adjusted by
// the tile and idle-player widths during retail setup.
static const int kManSourceXOffsets[5] = {0, -36, -5, -35, -6};
static const int kManYOffsets[5] = {-6, -7, -32, -21, 3};

// Cells redrawn in front of the 212-by-236 skull animation. The exact list is
// g_kiPuzzleTrackedCellRedrawIndexes at 0x2ea22.
static const uint kSkullForegroundCells[10] = {
	45, 52, 53, 54, 58, 59, 60, 61, 62, 63
};

static Common::String stripIniComment(const Common::String &source) {
	const size_t comment = source.find(';');
	Common::String value = comment == Common::String::npos ?
		source : source.substr(0, comment);
	value.trim();
	return value;
}

static bool readIniUint(const Common::INIFile &ini, const char *key,
		uint &value) {
	Common::String text;
	if (!ini.getKey(key, "setting", text))
		return false;
	text = stripIniComment(text);
	if (text.empty())
		return false;
	value = (uint)text.asUint64();
	return true;
}

} // End of anonymous namespace

KiSkullMazePuzzle::Config::Config() :
		randomizer(4), frameRate(15), doorDelayTicks(9), startPosition(3),
		initialHazardDelayTicks(40), hazardsPerDecrease(2),
		hazardDelayDecrementTicks(1), minimumHazardDelayTicks(5) {
}

KiSkullMazePuzzle::KiSkullMazePuzzle(RipperEngine *engine) :
		_engine(engine), _keywordIndex(0), _hoveredCell(-1),
		_movementDirection(0), _movementOriginCell(0), _targetCell(0),
		_movementFrame(0), _skullFrame(0), _toggleStep(0),
		_terminalState(0), _hazardsSinceDecrease(0), _hazardDelayTicks(0),
		_movementCuePlayed(false), _nextFrameMillis(0),
		_nextToggleMillis(0), _nextHazardMillis(0),
		_terminalDeadlineMillis(0) {
}

bool KiSkullMazePuzzle::loadConfiguration() {
	const uint puzzleLevel = CLIP<uint>(
		_engine->getSettings()->getPuzzleLevel(), 1, 3);
	const uint retailIndex = puzzleLevel - 1;
	Common::String name = Common::String::format("ki%u.ini", retailIndex);
	Common::File file;
	if (!file.open(Common::Path(name))) {
		const Common::String fallbackName("ki1.ini");
		if (retailIndex == 1 || !file.open(Common::Path(fallbackName))) {
			warning("Ripper: could not open KI skull-maze configuration '%s'",
				name.c_str());
			return false;
		}
		debugC(1, kDebugPuzzles,
			"Ripper: KI skull-maze configuration '%s' unavailable; using retail fallback '%s'",
			name.c_str(), fallbackName.c_str());
		name = fallbackName;
	}

	Common::INIFile ini;
	ini.requireKeyValueDelimiter();
	if (!ini.loadFromStream(file))
		return false;
	_config = Config();
	readIniUint(ini, "randomiser", _config.randomizer);
	readIniUint(ini, "frame rate", _config.frameRate);
	readIniUint(ini, "door delay", _config.doorDelayTicks);
	readIniUint(ini, "start pos", _config.startPosition);
	readIniUint(ini, "init delay", _config.initialHazardDelayTicks);
	readIniUint(ini, "pieces 2 dec", _config.hazardsPerDecrease);
	readIniUint(ini, "decrement", _config.hazardDelayDecrementTicks);
	readIniUint(ini, "min delay", _config.minimumHazardDelayTicks);
	if (_config.frameRate == 0 || _config.doorDelayTicks == 0 ||
			_config.startPosition >= KiSkullMazeModel::kCellCount ||
			_config.initialHazardDelayTicks == 0 ||
			_config.hazardsPerDecrease == 0 ||
			_config.hazardDelayDecrementTicks == 0 ||
			_config.minimumHazardDelayTicks == 0) {
		warning("Ripper: KI skull-maze configuration '%s' contains invalid values",
			name.c_str());
		return false;
	}

	debugC(2, kDebugPuzzles,
		"Ripper: loaded KI skull-maze configuration path='%s' puzzleLevel=%u "
		"retailIndex=%u randomizer=%u frameRate=%u doorDelay=%u start=%u "
		"hazardDelay=%u decreaseEvery=%u decrement=%u minimum=%u",
		name.c_str(), puzzleLevel, retailIndex, _config.randomizer,
		_config.frameRate, _config.doorDelayTicks, _config.startPosition,
		_config.initialHazardDelayTicks, _config.hazardsPerDecrease,
		_config.hazardDelayDecrementTicks, _config.minimumHazardDelayTicks);
	return true;
}

bool KiSkullMazePuzzle::loadBitmap(const Common::String &path,
		BitmapAssetFrame &frame, bool pcx) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_engine->getResources()->createReadStreamForPath(path));
	if (!stream || !(pcx ? decodePcxAsset(*stream, frame) :
			decodeBitmapAsset(*stream, frame))) {
		warning("Ripper: could not decode KI skull-maze %s '%s'",
			pcx ? "PCX" : "bitmap", path.c_str());
		return false;
	}
	return true;
}

bool KiSkullMazePuzzle::loadAnimation(const Common::String &path,
		Animation &animation) {
	Common::SeekableReadStream *stream =
		_engine->getResources()->createReadStreamForPath(path);
	Video::SmackerDecoder decoder;
	if (!stream || !decoder.loadStream(stream) ||
			decoder.getPixelFormat().bytesPerPixel != 1) {
		warning("Ripper: could not decode KI skull-maze animation '%s'",
			path.c_str());
		return false;
	}

	animation.frames.clear();
	animation.frames.reserve(decoder.getFrameCount());
	while (!decoder.endOfVideo()) {
		const Graphics::Surface *surface = decoder.decodeNextFrame();
		if (!surface)
			break;
		BitmapAssetFrame frame;
		frame.width = surface->w;
		frame.height = surface->h;
		frame.pixels.resize((uint32)frame.width * frame.height);
		for (uint y = 0; y < frame.height; ++y) {
			memcpy(frame.pixels.data() + y * frame.width,
				surface->getBasePtr(0, y), frame.width);
		}
		animation.frames.push_back(Common::move(frame));
	}
	if (animation.frames.empty())
		return false;
	animation.width = animation.frames[0].width;
	animation.height = animation.frames[0].height;
	debugC(2, kDebugPuzzles,
		"Ripper: decoded KI skull-maze animation media='%s' frames=%u size=%ux%u",
		path.c_str(), animation.frames.size(), animation.width,
		animation.height);
	return true;
}

bool KiSkullMazePuzzle::loadAssets() {
	if (!loadBitmap("ki_1.pcx", _background, true) ||
			_background.width != kRipperScreenWidth ||
			_background.height != kRipperScreenHeight ||
			_background.palette.size() != kRipperPaletteByteCount)
		return false;

	for (uint state = 0; state < ARRAYSIZE(_lids); ++state) {
		if (!loadBitmap(Common::String::format("ki_lid%u.bbm", state),
				_lids[state], false) || _lids[state].width != 49 ||
				_lids[state].height != 34)
			return false;
	}
	if (!loadAnimation("ki_skull.smk", _skullAnimation))
		return false;
	for (uint direction = 0; direction < ARRAYSIZE(_manAnimations);
			direction++) {
		if (!loadAnimation(Common::String::format("ki_man%u.smk", direction),
				_manAnimations[direction]))
			return false;
	}
	for (uint cue = 0; cue < ARRAYSIZE(_audioHandles); ++cue) {
		const Common::String path = Common::String::format("ki_%u.wav", cue);
		Common::ScopedPtr<Common::SeekableReadStream> stream(
			_engine->getResources()->createReadStreamForPath(path));
		if (!stream) {
			warning("Ripper: KI skull-maze audio '%s' is missing", path.c_str());
			return false;
		}
	}

	debugC(1, kDebugPuzzles,
		"Ripper: loaded KI skull-maze assets background=%ux%u lids=%u "
		"skull=%ux%u/%u man=%ux%u audio=%u",
		_background.width, _background.height, ARRAYSIZE(_lids),
		_skullAnimation.width, _skullAnimation.height,
		_skullAnimation.frames.size(), _manAnimations[0].width,
		_manAnimations[0].height, ARRAYSIZE(_audioHandles));
	return true;
}

void KiSkullMazePuzzle::applyPalette() const {
	Common::Array<byte> palette = _background.palette;
	_engine->getToolbar()->applySharedPalettePatch(palette.data(),
		kRipperPaletteColorCount);
	_engine->getSettings()->applyVideoPalette(palette.data(),
		kRipperPaletteColorCount, true);
	g_system->getPaletteManager()->setPalette(palette.data(), 0,
		kRipperPaletteColorCount);
}

void KiSkullMazePuzzle::drawFrame(byte *screen, uint pitch,
		const BitmapAssetFrame &frame, int x, int y,
		byte transparentColor) const {
	for (uint sourceY = 0; sourceY < frame.height; ++sourceY) {
		const int destinationY = y + sourceY;
		if (destinationY < 0 || destinationY >= kRipperScreenHeight)
			continue;
		for (uint sourceX = 0; sourceX < frame.width; ++sourceX) {
			const int destinationX = x + sourceX;
			if (destinationX < 0 || destinationX >= kRipperScreenWidth)
				continue;
			const byte pixel = frame.pixels[sourceY * frame.width + sourceX];
			if (pixel != transparentColor)
				screen[destinationY * pitch + destinationX] = pixel;
		}
	}
}

Common::Point KiSkullMazePuzzle::playerPosition(uint direction,
		uint originCell) const {
	if (originCell >= KiSkullMazeModel::kCellCount || direction >= 5)
		return Common::Point();
	const int centeredX = _lids[0].width / 2 -
		_manAnimations[0].width + 6;
	return Common::Point(
		kCellPositions[originCell].x + centeredX + kManSourceXOffsets[direction],
		kCellPositions[originCell].y + kManYOffsets[direction]);
}

void KiSkullMazePuzzle::render() const {
	Graphics::Surface *surface = g_system->lockScreen();
	if (!surface || surface->format.bytesPerPixel != 1 ||
			surface->w < kRipperScreenWidth || surface->h < kRipperScreenHeight) {
		if (surface)
			g_system->unlockScreen();
		return;
	}

	byte *screen = (byte *)surface->getPixels();
	for (uint y = 0; y < _background.height; ++y) {
		memcpy(screen + y * surface->pitch,
			_background.pixels.data() + y * _background.width,
			_background.width);
	}
	for (uint cell = 0; cell < KiSkullMazeModel::kCellCount; ++cell) {
		const uint state = _model.cellState(cell);
		drawFrame(screen, surface->pitch, _lids[state],
			kCellPositions[cell].x, kCellPositions[cell].y,
			_lids[state].transparentColor);
	}

	const BitmapAssetFrame &skull =
		_skullAnimation.frames[_skullFrame % _skullAnimation.frames.size()];
	drawFrame(screen, surface->pitch, skull,
		kRipperScreenWidth - _skullAnimation.width, 0,
		kSkullTransparentColor);
	for (uint index = 0; index < ARRAYSIZE(kSkullForegroundCells); ++index) {
		const uint cell = kSkullForegroundCells[index];
		const uint state = _model.cellState(cell);
		drawFrame(screen, surface->pitch, _lids[state],
			kCellPositions[cell].x, kCellPositions[cell].y,
			_lids[state].transparentColor);
	}

	const uint direction = _movementDirection;
	const Animation &man = _manAnimations[direction];
	const uint frame = direction == 0 ?
		_movementFrame % man.frames.size() :
		MIN<uint>(_movementFrame, man.frames.size() - 1);
	const Common::Point position = playerPosition(direction,
		direction == 0 ? _model.currentCell() : _movementOriginCell);
	drawFrame(screen, surface->pitch, man.frames[frame], position.x,
		position.y, kManTransparentColor);
	g_system->unlockScreen();
	presentScreen();
}

int KiSkullMazePuzzle::findCell(const Common::Point &point) const {
	int bestCell = -1;
	uint bestDistance = 0xffffffff;
	for (uint cell = 0; cell < KiSkullMazeModel::kCellCount; ++cell) {
		const Common::Point &position = kCellPositions[cell];
		const Common::Rect bounds(position.x, position.y,
			position.x + _lids[0].width, position.y + _lids[0].height);
		if (!bounds.contains(point))
			continue;
		const int dx = point.x - (position.x + _lids[0].width / 2);
		const int dy = point.y - (position.y + _lids[0].height / 2);
		const uint distance = dx * dx + dy * dy;
		if (distance < bestDistance) {
			bestDistance = distance;
			bestCell = cell;
		}
	}
	return bestCell;
}

void KiSkullMazePuzzle::updateCursor(const Common::Point &point) {
	const int cell = findCell(point);
	if (cell != _hoveredCell) {
		debugC(2, kDebugInput,
			"Ripper: KI skull-maze hover cell=%d previous=%d state=%d "
			"point=%d,%d current=%u busy=%d",
			cell, _hoveredCell,
			cell >= 0 ? _model.cellState(cell) : -1,
			point.x, point.y, _model.currentCell(),
			_movementDirection != 0 || _toggleStep != 0 || _terminalState != 0);
		_hoveredCell = cell;
	}
	_engine->getCursor()->update(cell >= 0 && _model.isOpen(cell) ?
		kSelectionCursor : kDefaultCursor);
	_engine->getCursor()->setVisible(true);
}

bool KiSkullMazePuzzle::startMove(uint targetCell, uint32 now) {
	if (!_model.canMoveTo(targetCell))
		return false;
	const uint current = _model.currentCell();
	const uint currentRow = current / KiSkullMazeModel::kBoardSize;
	const uint currentColumn = current % KiSkullMazeModel::kBoardSize;
	const uint targetRow = targetCell / KiSkullMazeModel::kBoardSize;
	const uint targetColumn = targetCell % KiSkullMazeModel::kBoardSize;
	if (targetRow > currentRow)
		_movementDirection = 1;
	else if (targetRow < currentRow)
		_movementDirection = 2;
	else if (targetColumn < currentColumn)
		_movementDirection = 3;
	else
		_movementDirection = 4;
	_movementOriginCell = current;
	_targetCell = targetCell;
	_movementFrame = 0;
	_movementCuePlayed = false;
	_nextFrameMillis = now + MAX<uint>(1, 1000 / _config.frameRate);
	debugC(2, kDebugPuzzles,
		"Ripper: KI skull-maze started move from=%u[%u,%u] to=%u[%u,%u] "
		"direction=%u targetState=%u",
		current, currentRow, currentColumn, targetCell, targetRow,
		targetColumn, _movementDirection, _model.cellState(targetCell));
	return true;
}

bool KiSkullMazePuzzle::advanceAnimation(uint32 now) {
	if ((int32)(now - _nextFrameMillis) < 0)
		return false;
	const uint frameDelay = MAX<uint>(1, 1000 / _config.frameRate);
	_nextFrameMillis = now + frameDelay;
	_skullFrame = (_skullFrame + 1) % _skullAnimation.frames.size();

	Animation &man = _manAnimations[_movementDirection];
	if (_movementDirection == 0) {
		_movementFrame = (_movementFrame + 1) % man.frames.size();
		return true;
	}

	if (_movementFrame + 1 < man.frames.size())
		++_movementFrame;
	if (!_movementCuePlayed && man.frames.size() > 5 &&
			_movementFrame >= man.frames.size() - 5) {
		playCue(2);
		_movementCuePlayed = true;
	}
	if (_movementFrame + 1 < man.frames.size())
		return true;

	_model.setCurrentCell(_targetCell);
	debugC(2, kDebugPuzzles,
		"Ripper: KI skull-maze completed move cell=%u row=%u column=%u "
		"state=%u frames=%u",
		_model.currentCell(), _model.currentCell() / KiSkullMazeModel::kBoardSize,
		_model.currentCell() % KiSkullMazeModel::kBoardSize,
		_model.cellState(_model.currentCell()), man.frames.size());
	_movementDirection = 0;
	_movementFrame = 0;
	_toggleStep = 1;
	const int changedCell = _model.toggleNeighbor(_toggleStep);
	if (changedCell >= 0)
		playCue(0);
	debugC(3, kDebugPuzzles,
		"Ripper: KI skull-maze toggled neighbor step=%u cell=%d state=%d",
		_toggleStep, changedCell,
		changedCell >= 0 ? _model.cellState(changedCell) : -1);
	_nextToggleMillis = now + _config.doorDelayTicks * kDosTickMillis;
	if (_model.reachedExit())
		beginTerminalState(1, now);
	return true;
}

bool KiSkullMazePuzzle::advanceToggle(uint32 now) {
	if (_toggleStep == 0 || (int32)(now - _nextToggleMillis) < 0)
		return false;
	++_toggleStep;
	const uint step = _toggleStep;
	const int changedCell = _model.toggleNeighbor(step);
	if (changedCell >= 0)
		playCue(0);
	debugC(3, kDebugPuzzles,
		"Ripper: KI skull-maze toggled neighbor step=%u cell=%d state=%d",
		step, changedCell,
		changedCell >= 0 ? _model.cellState(changedCell) : -1);
	if (step == 4)
		_toggleStep = 0;
	else
		_nextToggleMillis = now + _config.doorDelayTicks * kDosTickMillis;
	return true;
}

bool KiSkullMazePuzzle::spawnHazard(uint32 now) {
	if (_model.blockedCellCount() >= KiSkullMazeModel::kCellCount - 1 ||
			(int32)(now - _nextHazardMillis) < 0)
		return false;
	const uint protectedCell = _movementDirection != 0 ?
		_targetCell : _model.currentCell();
	const int cell = _model.spawnSkull(protectedCell);
	if (cell < 0)
		return false;
	playCue(1);
	++_hazardsSinceDecrease;
	if (_hazardsSinceDecrease >= _config.hazardsPerDecrease) {
		if (_hazardDelayTicks > _config.minimumHazardDelayTicks) {
			_hazardDelayTicks = MAX<uint>(_config.minimumHazardDelayTicks,
				_hazardDelayTicks - MIN<uint>(_hazardDelayTicks,
					_config.hazardDelayDecrementTicks));
		}
		_hazardsSinceDecrease = 0;
	}
	_nextHazardMillis = now + _hazardDelayTicks * kDosTickMillis;
	debugC(2, kDebugPuzzles,
		"Ripper: KI skull-maze spawned skull cell=%d row=%u column=%u "
		"blocked=%u protected=%u nextDelayTicks=%u",
		cell, cell / KiSkullMazeModel::kBoardSize,
		cell % KiSkullMazeModel::kBoardSize, _model.blockedCellCount(),
		protectedCell, _hazardDelayTicks);
	return true;
}

void KiSkullMazePuzzle::beginTerminalState(uint state, uint32 now) {
	if (_terminalState != 0)
		return;
	_terminalState = state;
	_terminalDeadlineMillis = now + kTerminalDelayTicks * kDosTickMillis;
	debugC(1, kDebugPuzzles,
		"Ripper: KI skull-maze terminal state=%s current=%u blocked=%u "
		"delayTicks=%u",
		state == 1 ? "exit-reached" : "trapped", _model.currentCell(),
		_model.blockedCellCount(), kTerminalDelayTicks);
}

bool KiSkullMazePuzzle::complete(uint completionFlag, const char *source) {
	if (!_engine->getMilestones()->set(completionFlag, true, source))
		return false;
	debugC(1, kDebugPuzzles,
		"Ripper: solved KI skull-maze puzzle milestone=%u source=%s "
		"current=%u blocked=%u",
		completionFlag, source, _model.currentCell(),
		_model.blockedCellCount());
	return true;
}

bool KiSkullMazePuzzle::handleKeyword(uint16 command, uint completionFlag,
		Result &result) {
	if (command == 0)
		return false;
	if (command > 0xff) {
		_keywordIndex = 0;
		return false;
	}
	char character = command & 0xff;
	if (character >= 'A' && character <= 'Z')
		character += 'a' - 'A';
	if (character == kCompletionKeyword[_keywordIndex])
		++_keywordIndex;
	else
		_keywordIndex = 0;
	if (kCompletionKeyword[_keywordIndex] != '\0')
		return false;

	debugC(1, kDebugPuzzles,
		"Ripper: KI skull-maze solved by hidden keyword '%s'",
		kCompletionKeyword);
	result = complete(completionFlag, "ki-skull-maze-keyword") ?
		kSolved : kLoadFailed;
	return true;
}

void KiSkullMazePuzzle::playCue(uint cue) {
	if (cue >= ARRAYSIZE(_audioHandles))
		return;
	const Common::String path = Common::String::format("ki_%u.wav", cue);
	const bool success = _engine->getMedia()->playSoundEffect(path,
		_audioHandles[cue]);
	debugC(success ? 3 : 2, kDebugPuzzles,
		"Ripper: KI skull-maze audio cue=%u path='%s' success=%d",
		cue, path.c_str(), success);
}

void KiSkullMazePuzzle::stopAudio() {
	for (uint cue = 0; cue < ARRAYSIZE(_audioHandles); ++cue)
		_engine->getMedia()->stopSoundEffect(_audioHandles[cue]);
}

void KiSkullMazePuzzle::cleanup(uint savedSelectionIndex,
		bool savedCursorVisible) {
	stopAudio();
	CursorManager *cursor = _engine->getCursor();
	cursor->setVisible(false);
	if (!_engine->shouldQuit()) {
		_engine->getMedia()->fadePalette(false, kFadeSteps);
		g_system->fillScreen(0);
		presentScreen();
	}
	cursor->setSelectionIndex(savedSelectionIndex);
	cursor->dispatchSelectionIndexChange(savedSelectionIndex);
	cursor->refresh();
	cursor->setVisible(savedCursorVisible);
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
}

KiSkullMazePuzzle::Result KiSkullMazePuzzle::run(uint completionFlag) {
	// DispatchSceneEntryAction at 0x36892 routes action 18 to
	// RunKiSkullMazePuzzleScene at 0x2ff55 with the completion flag in EAX.
	if (!_incomingDisplay.capture() || !loadConfiguration() || !loadAssets()) {
		_incomingDisplay.restore();
		return kLoadFailed;
	}

	_model.reset(_config.randomizer, _config.startPosition);
	CursorManager *cursor = _engine->getCursor();
	const uint savedSelectionIndex = cursor->getSelectionIndex();
	const bool savedCursorVisible = cursor->isVisible();
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	cursor->setSelectionIndex(kDefaultCursor);
	cursor->dispatchSelectionIndexChange(kDefaultCursor);
	cursor->setVisible(false);
	applyPalette();
	render();
	cursor->setVisible(true);

	const uint32 now = g_system->getMillis();
	_nextFrameMillis = now + MAX<uint>(1, 1000 / _config.frameRate);
	_hazardDelayTicks = _config.initialHazardDelayTicks;
	_nextHazardMillis = now + _hazardDelayTicks * kDosTickMillis;
	debugC(1, kDebugPuzzles,
		"Ripper: entered KI skull-maze puzzle function="
		"RunKiSkullMazePuzzleScene@0x2ff55 milestone=%u grid=8x8 "
		"controls=0x672..0x6b1 help=0x%x start=%u seed=%u "
		"frameRate=%u hazardDelayTicks=%u",
		completionFlag, kHelpSelectionTable, _model.currentCell(),
		_config.randomizer, _config.frameRate, _hazardDelayTicks);
	debugC(3, kDebugPuzzles,
		"Ripper: KI skull-maze initial cells=[%s] hiddenKeyword='%s'",
		_model.stateString().c_str(), kCompletionKeyword);

	Result result = kExited;
	while (!_engine->shouldQuit()) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			break;
		}

		uint16 command = 0;
		if (_engine->getInput()->hasPendingKey())
			command = _engine->getInput()->consumeKey();
		if (command == kEscapeCommand) {
			debugC(1, kDebugPuzzles,
				"Ripper: KI skull-maze exited by Escape");
			break;
		}
		if (command == kHelpCommand) {
			_keywordIndex = 0;
			debugC(1, kDebugPuzzles,
				"Ripper: KI skull-maze opening modal help table=0x%x",
				kHelpSelectionTable);
			if (!_engine->getModalDialog()->run(kHelpSelectionTable))
				warning("Ripper: KI skull-maze modal help failed");
			applyPalette();
			render();
		} else if (_movementDirection == 0 && _toggleStep == 0 &&
				_terminalState == 0 &&
				handleKeyword(command, completionFlag, result)) {
			break;
		}

		const MouseState mouse = _engine->getInput()->publishMouseState();
		updateCursor(mouse.position);
		if (_movementDirection == 0 && _toggleStep == 0 &&
				_terminalState == 0 &&
				(mouse.pressed & kMouseButtonLeft) != 0) {
			const int cell = findCell(mouse.position);
			if (cell >= 0 && !startMove(cell, g_system->getMillis())) {
				debugC(3, kDebugPuzzles,
					"Ripper: KI skull-maze rejected cell=%d current=%u "
					"state=%u adjacent=%d",
					cell, _model.currentCell(), _model.cellState(cell),
					_model.isAdjacent(cell));
			}
		}

		const uint32 currentMillis = g_system->getMillis();
		bool redraw = advanceAnimation(currentMillis);
		redraw = advanceToggle(currentMillis) || redraw;
		redraw = spawnHazard(currentMillis) || redraw;
		if (_terminalState == 0 && _movementDirection == 0 &&
				_toggleStep == 0 && _model.isTrapped()) {
			beginTerminalState(2, currentMillis);
			redraw = true;
		}
		if (_terminalState != 0 &&
				(int32)(currentMillis - _terminalDeadlineMillis) >= 0) {
			if (_terminalState == 1) {
				result = complete(completionFlag,
					"ki-skull-maze-puzzle") ? kSolved : kLoadFailed;
			} else {
				result = kTrapped;
			}
			break;
		}
		if (redraw)
			render();
		g_system->delayMillis(10);
	}

	cleanup(savedSelectionIndex, savedCursorVisible);
	debugC(result == kLoadFailed ? 1 : 2, kDebugPuzzles,
		"Ripper: left KI skull-maze puzzle result=%d milestone=%u "
		"milestoneSet=%d current=%u blocked=%u terminal=%u quit=%d",
		result, completionFlag,
		_engine->getMilestones()->isSet(completionFlag),
		_model.currentCell(), _model.blockedCellCount(), _terminalState,
		_engine->shouldQuit());
	return result;
}

} // End of namespace Ripper
