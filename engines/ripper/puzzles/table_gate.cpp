/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this program.
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

#include "ripper/puzzles/table_gate.h"

#include "common/debug.h"
#include "common/file.h"
#include "common/formats/ini-file.h"
#include "common/system.h"
#include "common/util.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/ini.h"
#include "ripper/media.h"
#include "ripper/milestones.h"
#include "ripper/modal_dialog.h"
#include "ripper/ripper.h"
#include "ripper/settings.h"

namespace Ripper {

namespace {

struct PathNode {
	int16 y;
	int16 x;
	int16 next;
};

struct GateDefinition {
	int16 y;
	int16 x;
	int16 sourceNode;
	int16 route0;
	int16 route1;
};

static const uint kPathNodeCount = 123;
static const uint kGateCount = 13;
static const uint kLeverCount = 5;
static const uint kGatesPerLever = 5;
static const uint kAudioCueCount = 6;
static const uint kDefaultCursor = 14;
static const uint kLeverCursor = 16;
static const uint kExitCursor = 7;
static const int kSceneOriginY = 50;
static const uint kDosTimerTickMillis = 55;
static const uint kLeverFeedbackTicks = 9;
static const uint kMarkerAudioDelayTicks = 3;
static const uint kMaximumMarkerSpeed = 6;
static const uint kSpeedRampFrames = 3;
static const uint16 kHelpAction = 0x3b00;
static const uint kHelpSelectionTable = 0x1a1;
static const uint kNoLaunchFrame = 0xffffffff;

// RunTableGateLeverPuzzleScene at 0x38eb8 normalizes the original 1-based
// records at 0x84720 once. These are the resulting zero-based links.
static const PathNode kPathNodes[kPathNodeCount] = {
	{22, 486, 1}, {72, 486, 6}, {185, 487, 3}, {263, 485, -2},
	{37, 446, 16}, {66, 446, 4}, {71, 455, 7}, {76, 446, 8},
	{99, 446, 9}, {99, 474, 10}, {170, 474, 57}, {185, 432, 2},
	{230, 432, 11}, {234, 425, 14}, {236, 432, 15}, {264, 432, 112},
	{37, 399, 17}, {62, 399, 19}, {73, 397, 37}, {73, 406, 20},
	{73, 432, 25}, {88, 399, 38}, {113, 399, 21}, {120, 403, 24},
	{122, 400, 26}, {120, 432, 23}, {154, 400, 40}, {185, 385, 28},
	{228, 385, 30}, {234, 382, 42}, {234, 388, 13}, {22, 355, 113},
	{22, 370, 113}, {54, 338, 78}, {54, 346, 36}, {63, 340, 34},
	{54, 355, 31}, {73, 370, 32}, {88, 340, 35}, {105, 353, 47},
	{154, 353, 39}, {185, 335, 27}, {234, 353, 44}, {249, 335, 41},
	{264, 353, 112}, {71, 323, 50}, {102, 323, 45}, {105, 331, 48},
	{110, 323, 49}, {137, 323, 53}, {71, 302, 51}, {91, 302, 55},
	{105, 302, 65}, {137, 302, 52}, {71, 273, 87}, {91, 273, 54},
	{122, 286, 68}, {170, 286, 56}, {205, 301, 59}, {246, 301, 61},
	{249, 296, 74}, {249, 304, 43}, {22, 262, 113}, {37, 262, 62},
	{88, 256, 88}, {105, 256, 64}, {106, 240, 79}, {120, 240, 66},
	{122, 248, 69}, {126, 241, 70}, {142, 241, 80}, {176, 271, 73},
	{190, 253, 82}, {205, 271, 58}, {249, 253, 72}, {37, 216, 85},
	{37, 223, 63}, {39, 218, 75}, {54, 218, 77}, {106, 206, 90},
	{142, 223, 81}, {176, 223, 71}, {190, 223, 83}, {264, 223, 112},
	{22, 188, 113}, {37, 188, 84}, {55, 188, 95}, {71, 188, 86},
	{88, 188, 89}, {117, 188, 109}, {146, 206, 100}, {210, 208, 92},
	{235, 208, 105}, {22, 171, 113}, {52, 171, 93}, {55, 176, 94},
	{58, 171, 97}, {75, 171, 108}, {131, 172, 110}, {142, 172, 98},
	{146, 177, 101}, {148, 172, 102}, {203, 172, 104}, {210, 167, 111},
	{210, 175, 91}, {235, 172, 106}, {264, 172, 112}, {22, 130, 113},
	{75, 130, 107}, {117, 130, 107}, {131, 130, 107}, {210, 130, 107},
	{264, 130, 107}, {22, 419, -1}, {68, 399, 18}, {120, 410, 22},
	{58, 340, 33}, {105, 329, 46}, {243, 301, 60}, {122, 243, 67},
	{44, 218, 76}, {55, 177, 96}, {146, 179, 99}
};

// The source node and two routes are the normalized records at 0x84a02.
// Gate state zero selects route0 and state one selects route1.
static const GateDefinition kGateDefinitions[kGateCount] = {
	{68, 445, 6, 7, 5}, {226, 427, 13, 14, 12},
	{66, 394, 16, 17, 114}, {111, 397, 25, 23, 115},
	{227, 378, 28, 30, 29}, {48, 335, 38, 35, 116},
	{99, 320, 39, 47, 117}, {242, 294, 58, 59, 118},
	{116, 234, 56, 68, 119}, {32, 211, 78, 77, 120},
	{49, 166, 86, 95, 121}, {136, 166, 90, 100, 122},
	{200, 165, 102, 104, 103}
};

static const Common::Point kLeverPositions[kLeverCount] = {
	Common::Point(533, 132 + kSceneOriginY),
	Common::Point(533, 144 + kSceneOriginY),
	Common::Point(533, 155 + kSceneOriginY),
	Common::Point(533, 167 + kSceneOriginY),
	Common::Point(533, 179 + kSceneOriginY)
};

// Dirty-region coordinates at 0x84ae4. The original reveals one indicator
// after each lever activation and restores all five after a failed traversal.
static const Common::Point kActivationPositions[kLeverCount] = {
	Common::Point(537, 97 + kSceneOriginY),
	Common::Point(531, 102 + kSceneOriginY),
	Common::Point(544, 102 + kSceneOriginY),
	Common::Point(537, 108 + kSceneOriginY),
	Common::Point(537, 118 + kSceneOriginY)
};

} // End of anonymous namespace

TableGatePuzzle::TableGatePuzzle(RipperEngine *engine) : Puzzle(engine),
		_random("ripper-table-gate-puzzle"), _allowMultipleLeverUse(false),
		_markerActive(false), _segmentAudioPending(false),
		_leverActivationsBeforeStart(0), _leverActivationCount(0),
		_visibleActivationCount(0), _currentPathNode(0),
		_launchFrame(kNoLaunchFrame), _speedRampFrameCounter(0),
		_lastMarkerAdvanceMillis(0), _segmentStartMillis(0),
		_pressedLever(-1), _hoveredLever(-1) {
	for (uint gate = 0; gate < kGateCount; ++gate)
		_gateStates[gate] = false;
	for (uint lever = 0; lever < kLeverCount; ++lever) {
		_leverEnabled[lever] = true;
		for (uint target = 0; target < kGatesPerLever; ++target)
			_leverGates[lever][target] = -1;
	}
}

bool TableGatePuzzle::captureBackground() {
	return _backgroundDisplay.capture();
}

void TableGatePuzzle::restoreBackground() const {
	_backgroundDisplay.restore();
}

bool TableGatePuzzle::loadConfiguration() {
	const uint puzzleLevel = CLIP<uint>(_engine->getSettings()->getPuzzleLevel(), 1, 3);
	const Common::String filename = Common::String::format("tblini%u.ini", puzzleLevel);
	Common::File file;
	Common::INIFile ini;
	if (!file.open(Common::Path(filename)) || !ini.loadFromStream(file)) {
		warning("Ripper: could not load table gate puzzle configuration '%s'", filename.c_str());
		return false;
	}

	if (!readIniUint(ini, "game", "num 2 start", _leverActivationsBeforeStart) ||
			_leverActivationsBeforeStart == 0 ||
			_leverActivationsBeforeStart > kLeverCount) {
		warning("Ripper: table gate puzzle configuration '%s' has an invalid activation threshold",
			filename.c_str());
		return false;
	}

	uint multiple = 0;
	if (readIniUint(ini, "game", "multiple", multiple))
		_allowMultipleLeverUse = multiple != 0;

	for (uint gate = 0; gate < kGateCount; ++gate) {
		uint value = 0;
		if (!readIniUint(ini, "gates", Common::String::format("gate%u", gate + 1), value) ||
				value > 1) {
			warning("Ripper: table gate puzzle configuration '%s' has an invalid gate%u",
				filename.c_str(), gate + 1);
			return false;
		}
		_gateStates[gate] = value != 0;
	}

	for (uint lever = 0; lever < kLeverCount; ++lever) {
		const Common::String section = Common::String::format("switch%u", lever + 1);
		for (uint target = 0; target < kGatesPerLever; ++target) {
			uint value = 0;
			if (!readIniUint(ini, section,
					Common::String::format("lever%u", target + 1), value) ||
					value == 0 || value > kGateCount) {
				warning("Ripper: table gate puzzle configuration '%s' has an invalid %s lever%u",
					filename.c_str(), section.c_str(), target + 1);
				return false;
			}
			_leverGates[lever][target] = value - 1;
		}
	}

	debugC(2, kDebugPuzzles,
		"Ripper: loaded table gate puzzle configuration path='%s' difficulty=%u "
		"activations=%u multiple=%d",
		filename.c_str(), puzzleLevel, _leverActivationsBeforeStart,
		_allowMultipleLeverUse);
	return true;
}

bool TableGatePuzzle::loadFrame(const Common::String &name, BitmapAssetFrame &frame) {
	BitmapAssetSequence sequence;
	if (!_engine->getResources()->loadBitmapSequence(name, sequence) || sequence.frames.empty()) {
		warning("Ripper: could not load table gate puzzle bitmap '%s'", name.c_str());
		return false;
	}
	frame = Common::move(sequence.frames.front());
	return true;
}

bool TableGatePuzzle::loadAssets() {
	if (!loadFrame("gb_tbl0.bbm", _markerFrame))
		return false;

	for (uint frame = 1; frame <= 6; ++frame) {
		BitmapAssetFrame asset;
		if (!loadFrame(Common::String::format("gb_tbl%u.bbm", frame), asset))
			return false;
		_launchFrames.push_back(Common::move(asset));
	}
	for (uint gate = 0; gate < kGateCount; ++gate) {
		BitmapAssetFrame asset;
		if (!loadFrame(Common::String::format("gb_tbl%u.bbm", gate + 10), asset))
			return false;
		_gateFrames.push_back(Common::move(asset));
	}
	for (uint indicator = 0; indicator < kLeverCount; ++indicator) {
		BitmapAssetFrame asset;
		if (!loadFrame(Common::String::format("gb_tbl%u.bbm", indicator + 25), asset))
			return false;
		_activationFrames.push_back(Common::move(asset));
	}
	for (uint lever = 0; lever < kLeverCount; ++lever) {
		BitmapAssetFrame asset;
		if (!loadFrame(Common::String::format("gb_tbl%u.bbm", lever + 30), asset))
			return false;
		_leverFrames.push_back(Common::move(asset));
	}

	debugC(1, kDebugPuzzles,
		"Ripper: loaded table gate puzzle assets marker=1 launch=%u gates=%u "
		"indicators=%u levers=%u",
		_launchFrames.size(), _gateFrames.size(), _activationFrames.size(),
		_leverFrames.size());
	return true;
}

void TableGatePuzzle::drawFrame(byte *screen, uint pitch,
		const BitmapAssetFrame &frame, int x, int y) const {
	for (uint sourceY = 0; sourceY < frame.height; ++sourceY) {
		const int destinationY = y + sourceY;
		if (destinationY < 0 || destinationY >= 400)
			continue;
		for (uint sourceX = 0; sourceX < frame.width; ++sourceX) {
			const int destinationX = x + sourceX;
			if (destinationX < 0 || destinationX >= 640)
				continue;
			const byte pixel = frame.pixels[sourceY * frame.width + sourceX];
			if (pixel != frame.transparentColor)
				screen[destinationY * pitch + destinationX] = pixel;
		}
	}
}

void TableGatePuzzle::render() {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}

	for (int y = 0; y < screen->h; ++y)
		memcpy(screen->getBasePtr(0, y),
			_backgroundDisplay.pixels().data() + y * screen->w, screen->w);
	byte *pixels = (byte *)screen->getPixels();

	for (uint gate = 0; gate < kGateCount; ++gate) {
		if (_gateStates[gate]) {
			drawFrame(pixels, screen->pitch, _gateFrames[gate],
				kGateDefinitions[gate].x, kGateDefinitions[gate].y + kSceneOriginY);
		}
	}
	for (uint indicator = 0; indicator < _visibleActivationCount; ++indicator) {
		drawFrame(pixels, screen->pitch, _activationFrames[indicator],
			kActivationPositions[indicator].x, kActivationPositions[indicator].y);
	}
	if (_launchFrame < _launchFrames.size()) {
		drawFrame(pixels, screen->pitch, _launchFrames[_launchFrame],
			416, 20 + kSceneOriginY);
	}
	if (_markerActive) {
		drawFrame(pixels, screen->pitch, _markerFrame,
			_markerLine.x, _markerLine.y);
	}
	if (_pressedLever >= 0) {
		drawFrame(pixels, screen->pitch, _leverFrames[_pressedLever],
			kLeverPositions[_pressedLever].x, kLeverPositions[_pressedLever].y);
	}

	g_system->unlockScreen();
	_backgroundDisplay.restorePalette();
	presentScreen();
}

int TableGatePuzzle::findLever(const Common::Point &point) const {
	if (_markerActive)
		return -1;
	for (uint lever = 0; lever < kLeverCount; ++lever) {
		if (!_leverEnabled[lever])
			continue;
		const BitmapAssetFrame &frame = _leverFrames[lever];
		const Common::Point &position = kLeverPositions[lever];
		if (Common::Rect(position.x, position.y,
				position.x + frame.width, position.y + frame.height).contains(point))
			return lever;
	}
	return -1;
}

bool TableGatePuzzle::isExitRegion(const Common::Point &point) const {
	return point.y >= kSceneOriginY && point.y < kSceneOriginY + 274 &&
		((point.x >= 4 && point.x < 119) || (point.x >= 570 && point.x < 640));
}

void TableGatePuzzle::updateCursor(const Common::Point &point) {
	const int hoveredLever = findLever(point);
	uint cursor = kDefaultCursor;
	if (hoveredLever >= 0)
		cursor = kLeverCursor;
	else if (isExitRegion(point))
		cursor = kExitCursor;

	if (hoveredLever != _hoveredLever) {
		debugC(3, kDebugPuzzles,
			"Ripper: table gate puzzle hover lever=%d previous=%d point=%d,%d cursor=%u",
			hoveredLever, _hoveredLever, point.x, point.y, cursor);
		_hoveredLever = hoveredLever;
	}
	_engine->getCursor()->update(cursor);
}

void TableGatePuzzle::playCue(uint cue) {
	if (cue >= kAudioCueCount)
		return;
	_engine->getMedia()->playSoundEffect(Common::String::format("table%u.wav", cue),
		_audioHandles[cue]);
}

void TableGatePuzzle::activateLever(uint lever) {
	if (lever >= kLeverCount || !_leverEnabled[lever] || _markerActive)
		return;

	_pressedLever = lever;
	playCue(5);
	render();
	g_system->delayMillis(kLeverFeedbackTicks * kDosTimerTickMillis);
	_pressedLever = -1;

	++_leverActivationCount;
	_visibleActivationCount = MIN<uint>(_leverActivationCount, kLeverCount);
	if (!_allowMultipleLeverUse)
		_leverEnabled[lever] = false;
	playCue(1);

	for (uint target = 0; target < kGatesPerLever; ++target) {
		const uint gate = _leverGates[lever][target];
		_gateStates[gate] = !_gateStates[gate];
		const GateDefinition &definition = kGateDefinitions[gate];
		debugC(3, kDebugPuzzles,
			"Ripper: table gate puzzle toggled gate=%u state=%d sourceNode=%d route=%d",
			gate + 1, _gateStates[gate], definition.sourceNode,
			_gateStates[gate] ? definition.route1 : definition.route0);
	}
	debugC(2, kDebugPuzzles,
		"Ripper: table gate puzzle activated lever=%u count=%u/%u multiple=%d",
		lever + 1, _leverActivationCount, _leverActivationsBeforeStart,
		_allowMultipleLeverUse);

	if (_leverActivationCount >= _leverActivationsBeforeStart)
		startMarkerRun();
	else
		render();
}

int TableGatePuzzle::nextPathNode(uint node) const {
	for (uint gate = 0; gate < kGateCount; ++gate) {
		if (kGateDefinitions[gate].sourceNode == (int)node) {
			return _gateStates[gate] ? kGateDefinitions[gate].route1 :
				kGateDefinitions[gate].route0;
		}
	}
	return kPathNodes[node].next;
}

void TableGatePuzzle::initializeMarkerSegment() {
	const PathNode &source = kPathNodes[_currentPathNode];
	const int nextNode = nextPathNode(_currentPathNode);
	const PathNode &target = kPathNodes[nextNode];
	_markerLine.x = source.x;
	_markerLine.y = source.y + kSceneOriginY;
	_markerLine.targetX = target.x;
	_markerLine.targetY = target.y + kSceneOriginY;
	_markerLine.deltaX = ABS(_markerLine.targetX - _markerLine.x);
	_markerLine.deltaY = ABS(_markerLine.targetY - _markerLine.y);
	_markerLine.stepX = _markerLine.x < _markerLine.targetX ? 1 : -1;
	_markerLine.stepY = _markerLine.y < _markerLine.targetY ? 1 : -1;
	_markerLine.error = _markerLine.deltaX - _markerLine.deltaY;
	_markerLine.stepsPerAdvance = 1;
	_speedRampFrameCounter = 0;
	_segmentAudioPending = true;
	_segmentStartMillis = g_system->getMillis();
	_lastMarkerAdvanceMillis = _segmentStartMillis;
	debugC(3, kDebugPuzzles,
		"Ripper: table gate puzzle marker segment source=%u target=%d from=%d,%d to=%d,%d",
		_currentPathNode, nextNode, _markerLine.x, _markerLine.y,
		_markerLine.targetX, _markerLine.targetY);
}

bool TableGatePuzzle::advanceMarkerLine() {
	for (uint step = 0; step < _markerLine.stepsPerAdvance; ++step) {
		if (_markerLine.x == _markerLine.targetX && _markerLine.y == _markerLine.targetY)
			return true;
		const int doubledError = _markerLine.error * 2;
		if (doubledError > -_markerLine.deltaY) {
			_markerLine.error -= _markerLine.deltaY;
			_markerLine.x += _markerLine.stepX;
		}
		if (doubledError < _markerLine.deltaX) {
			_markerLine.error += _markerLine.deltaX;
			_markerLine.y += _markerLine.stepY;
		}
	}
	return _markerLine.x == _markerLine.targetX && _markerLine.y == _markerLine.targetY;
}

void TableGatePuzzle::startMarkerRun() {
	_leverActivationCount = 0;
	for (uint lever = 0; lever < kLeverCount; ++lever)
		_leverEnabled[lever] = false;
	playCue(0);
	debugC(2, kDebugPuzzles,
		"Ripper: table gate puzzle marker run starting node=0 indicators=%u",
		_visibleActivationCount);

	for (uint frame = 0; frame + 1 < _launchFrames.size(); ++frame) {
		_launchFrame = frame;
		render();
		g_system->delayMillis(kLeverFeedbackTicks * kDosTimerTickMillis);
	}
	_launchFrame = _launchFrames.size() - 1;
	_currentPathNode = 0;
	_markerActive = true;
	initializeMarkerSegment();
	playCue(_random.getRandomBit() ? 2 : 4);
	render();
}

TableGatePuzzle::Result TableGatePuzzle::serviceMarker(uint completionFlag) {
	if (!_markerActive)
		return kExited;

	const uint32 now = g_system->getMillis();
	if (_segmentAudioPending &&
			(int32)(now - (_segmentStartMillis + kMarkerAudioDelayTicks * kDosTimerTickMillis)) >= 0) {
		playCue(3);
		_segmentAudioPending = false;
	}
	if ((int32)(now - (_lastMarkerAdvanceMillis + kDosTimerTickMillis)) < 0)
		return kExited;

	const bool reachedNode = advanceMarkerLine();
	if (_markerLine.stepsPerAdvance < kMaximumMarkerSpeed &&
			++_speedRampFrameCounter > kSpeedRampFrames - 1) {
		++_markerLine.stepsPerAdvance;
		_speedRampFrameCounter = 0;
	}
	_lastMarkerAdvanceMillis = now;
	render();
	if (!reachedNode)
		return kExited;

	_engine->getMedia()->stopSoundEffect(_audioHandles[3]);
	playCue(_random.getRandomBit() ? 2 : 4);
	const int nextNode = nextPathNode(_currentPathNode);
	_currentPathNode = nextNode;
	const int terminal = kPathNodes[_currentPathNode].next;
	debugC(3, kDebugPuzzles,
		"Ripper: table gate puzzle marker reached node=%u terminal=%d",
		_currentPathNode, terminal);
	if (terminal == -2) {
		_markerActive = false;
		if (!markSolved(completionFlag, "table-gate-puzzle"))
			return kLoadFailed;
		debugC(1, kDebugPuzzles,
			"Ripper: solved table gate puzzle milestone=%u terminalNode=%u",
			completionFlag, _currentPathNode);
		return kSolved;
	}
	if (terminal < 0) {
		debugC(2, kDebugPuzzles,
			"Ripper: table gate puzzle marker reached failure terminal node=%u; preserving gates",
			_currentPathNode);
		resetAfterFailedRun();
		return kExited;
	}

	initializeMarkerSegment();
	return kExited;
}

void TableGatePuzzle::resetAfterFailedRun() {
	_markerActive = false;
	_segmentAudioPending = false;
	_launchFrame = kNoLaunchFrame;
	_visibleActivationCount = 0;
	for (uint lever = 0; lever < kLeverCount; ++lever)
		_leverEnabled[lever] = true;
	render();
}

void TableGatePuzzle::stopAudio() {
	stopAudioHandles(_audioHandles, ARRAYSIZE(_audioHandles));
}

TableGatePuzzle::Result TableGatePuzzle::run(uint completionFlag) {
	if (!captureBackground() || !loadConfiguration() || !loadAssets()) {
		restoreBackground();
		return kLoadFailed;
	}

	debugC(1, kDebugPuzzles,
		"Ripper: entered table gate puzzle completionFlag=%u pathNodes=%u gates=%u levers=%u",
		completionFlag, kPathNodeCount, kGateCount, kLeverCount);
	_engine->getInput()->discardMouseTransitions();
	// RunTableGateLeverPuzzleScene at 0x3912a..0x39134 stores and dispatches
	// cursor row 14 before activating the puzzle's UI-selection presentation.
	// GBZ1 has just hidden the cursor, so reinstall the frame after the retained
	// scene and overlay palette have been presented.
	_engine->getCursor()->setSelectionIndex(kDefaultCursor);
	_engine->getCursor()->dispatchSelectionIndexChange(kDefaultCursor);
	playCue(1);
	render();
	_engine->getCursor()->refresh();
	debugC(2, kDebugPuzzles,
		"Ripper: initialized table gate cursor presentation selection=%u visible=%d",
		kDefaultCursor, _engine->getCursor()->isVisible());

	Result result = kExited;
	bool active = true;
	while (active && !_engine->shouldQuit()) {
		if (!serviceEngineEvents())
			break;
		while (_engine->getInput()->hasPendingKey()) {
			const uint16 command = _engine->getInput()->consumeKey();
			if (command == 0x1b) {
				debugC(1, kDebugPuzzles, "Ripper: table gate puzzle exited by Escape");
				active = false;
				break;
			}
			if (command == kHelpAction && !_markerActive) {
				debugC(1, kDebugPuzzles,
					"Ripper: table gate puzzle opening modal help table=0x%x",
					kHelpSelectionTable);
				if (!_engine->getModalDialog()->run(kHelpSelectionTable))
					warning("Ripper: table gate puzzle modal help failed");
				render();
			}
		}
		if (!active)
			break;

		const MouseState mouse = _engine->getInput()->publishMouseState();
		updateCursor(mouse.position);
		// ServiceUiControlStateSelection at 0x393f6 runs under the original
		// active UI-selection presentation. Present each translated cursor tick
		// explicitly for ScummVM's software cursor.
		presentScreen();
		if ((mouse.pressed & kMouseButtonLeft) != 0) {
			if (isExitRegion(mouse.position)) {
				debugC(1, kDebugPuzzles,
					"Ripper: table gate puzzle exited by edge control point=%d,%d",
					mouse.position.x, mouse.position.y);
				active = false;
			} else {
				const int lever = findLever(mouse.position);
				if (lever >= 0)
					activateLever(lever);
			}
		}
		if (!active)
			break;

		const Result markerResult = serviceMarker(completionFlag);
		if (markerResult == kSolved || markerResult == kLoadFailed) {
			result = markerResult;
			active = false;
		}
		g_system->delayMillis(10);
	}

	stopAudio();
	_engine->getCursor()->update(0);
	restoreBackground();
	_engine->getInput()->discardMouseTransitions();
	debugC(1, kDebugPuzzles,
		"Ripper: left table gate puzzle result=%d milestone=%u",
		result, completionFlag);
	return result;
}

} // End of namespace Ripper
