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
 */

#include "ripper/puzzles/clock.h"

#include "common/archive.h"
#include "common/debug.h"
#include "common/system.h"
#include "common/util.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"

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

struct ClockControlLayout {
	int x;
	int y;
	int width;
	int height;
	uint16 command;
};

static const uint kDialCount = 12;
static const uint kArmyFrameCount = 11;
static const uint kEgyptFrameCount = 12;
static const uint kAudioCueCount = 6;
static const int kSceneOriginY = 50;
static const uint kDefaultCursor = 14;
static const uint kSelectionCursor = 16;
static const uint kExitCursor = 7;
static const uint16 kExitCommand = 0x1b;
static const uint16 kHelpCommand = 0x3b00;
static const uint kHelpSelectionTable = 0x1a2;

// g_clockPuzzleUiControlRecords at 0x846a6 stores these seven
// x/y/width/height/control-id records. Y is relative to the scene viewport.
static const ClockControlLayout kControls[] = {
	{290, 82, 94, 91, 0x672},
	{490, 236, 18, 18, 0x673},
	{511, 239, 18, 18, 0x674},
	{534, 244, 21, 17, 0x675},
	{556, 246, 22, 18, 0x676},
	{65, 246, 35, 30, 0x677},
	{144, 246, 35, 30, 0x678}
};

// g_clockPuzzleOverlayFrameTable at 0x8463a. Entries 0..11 anchor the
// two analog hands; entries 12..17 anchor Egyptian digits and markers.
static const Common::Point kDialPositions[kDialCount] = {
	Common::Point(330, 91), Common::Point(331, 97),
	Common::Point(330, 110), Common::Point(331, 123),
	Common::Point(331, 124), Common::Point(331, 124),
	Common::Point(330, 125), Common::Point(314, 125),
	Common::Point(302, 125), Common::Point(299, 124),
	Common::Point(304, 106), Common::Point(317, 94)
};

static const Common::Point kArmyPositions[5] = {
	Common::Point(494, 207), Common::Point(510, 209),
	Common::Point(538, 213), Common::Point(557, 216),
	Common::Point(530, 214)
};

static const Common::Point kEgyptPositions[6] = {
	Common::Point(105, 259), Common::Point(114, 259),
	Common::Point(127, 259), Common::Point(136, 259),
	Common::Point(121, 259), Common::Point(101, 266)
};

static const int kTargetDials[2] = {7, 8};
static const int kTargetFirstClock[4] = {1, 4, 3, 5};
static const int kTargetSecondClock[4] = {0, 9, 3, 5};
static const bool kTargetHalfDay = true;

static RipperEngine *sClockStateOwner = nullptr;
static int sDialIndices[2] = {0, 0};
static int sFirstClockDigits[4] = {0, 0, 0, 0};
static int sSecondClockDigits[4] = {0, 0, 0, 0};
static bool sHalfDay = false;

} // End of anonymous namespace

ClockPuzzle::ClockPuzzle(RipperEngine *engine) : Puzzle(engine),
		_hoveredControl(-1), _dragDial(-1), _lastDragIndex(-1),
		_halfDay(false) {
	memset(_dialIndices, 0, sizeof(_dialIndices));
	memset(_firstClockDigits, 0, sizeof(_firstClockDigits));
	memset(_secondClockDigits, 0, sizeof(_secondClockDigits));
}

bool ClockPuzzle::captureBackground() {
	return _backgroundDisplay.capture();
}

void ClockPuzzle::restoreBackground() const {
	_backgroundDisplay.restore();
}

bool ClockPuzzle::loadFrame(const Common::String &name, BitmapAssetFrame &frame) {
	BitmapAssetSequence sequence;
	if (!_engine->getResources()->loadBitmapSequence(name, sequence) || sequence.frames.empty()) {
		warning("Ripper: could not load clock puzzle bitmap '%s'", name.c_str());
		return false;
	}
	frame = Common::move(sequence.frames.front());
	return true;
}

bool ClockPuzzle::loadFrames(const char *pattern, uint count,
		Common::Array<BitmapAssetFrame> &frames) {
	frames.clear();
	for (uint i = 0; i < count; ++i) {
		BitmapAssetFrame frame;
		if (!loadFrame(Common::String::format(pattern, i), frame))
			return false;
		frames.push_back(Common::move(frame));
	}
	return true;
}

bool ClockPuzzle::loadAssets() {
	if (!loadFrames("c_hour%u.bbm", kDialCount, _hourFrames) ||
			!loadFrames("c_min%u.bbm", kDialCount, _minuteFrames) ||
			!loadFrames("c_army%u.bbm", kArmyFrameCount, _armyFrames) ||
			!loadFrames("c_egyp%u.bbm", kEgyptFrameCount, _egyptFrames) ||
			!loadFrame("clock.bbm", _clockFace) ||
			!loadFrame("c_armybk.bbm", _armyBackdrop) ||
			!loadFrame("clock_b5.bbm", _markerButtons[0]) ||
			!loadFrame("clock_b6.bbm", _markerButtons[1]))
		return false;

	const Common::String maskName = SearchMan.hasFile(Common::Path("clock_tmp.mcg")) ?
		"clock_tmp.mcg" : "clock_tm.mcg";
	// RunClockPuzzleScene at 0x374c5 loads CLOCK_TMP.MCG through
	// DecodeCustomBitmapAsset at 0x53fdf. It is one compressed 640x300 bitmap,
	// not a bitmap-set container like the numbered hand and digit resources.
	if (!_engine->getResources()->loadBitmap(maskName, _dialHitMask) ||
			_dialHitMask.width != 640 ||
			_dialHitMask.height != 300) {
		warning("Ripper: invalid clock puzzle hit mask '%s' size=%ux%u",
			maskName.c_str(), _dialHitMask.width, _dialHitMask.height);
		return false;
	}

	debugC(1, kDebugPuzzles,
		"Ripper: loaded clock puzzle assets hands=%u/%u army=%u egypt=%u hitMask=%ux%u",
		_hourFrames.size(), _minuteFrames.size(), _armyFrames.size(),
		_egyptFrames.size(), _dialHitMask.width, _dialHitMask.height);
	return true;
}

void ClockPuzzle::render() const {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1 ||
			_backgroundDisplay.pixels().size() != (uint)screen->w * screen->h) {
		if (screen)
			g_system->unlockScreen();
		return;
	}
	for (int y = 0; y < screen->h; ++y)
		memcpy(screen->getBasePtr(0, y),
			_backgroundDisplay.pixels().data() + y * screen->w, screen->w);
	byte *pixels = (byte *)screen->getPixels();
	const Common::Rect screenBounds(0, 0, screen->w, screen->h);
	IndexedBitmapRenderer::drawBitmap(pixels, screen->pitch, _clockFace,
		296, 88 + kSceneOriginY, screenBounds);
	if (_dialIndices[0] >= 0 && _dialIndices[0] < (int)_minuteFrames.size()) {
		const Common::Point &position = kDialPositions[_dialIndices[0]];
		IndexedBitmapRenderer::drawBitmap(pixels, screen->pitch,
			_minuteFrames[_dialIndices[0]], position.x,
			position.y + kSceneOriginY, screenBounds);
	}
	if (_dialIndices[1] >= 0 && _dialIndices[1] < (int)_hourFrames.size()) {
		const Common::Point &position = kDialPositions[_dialIndices[1]];
		IndexedBitmapRenderer::drawBitmap(pixels, screen->pitch,
			_hourFrames[_dialIndices[1]], position.x,
			position.y + kSceneOriginY, screenBounds);
	}

	IndexedBitmapRenderer::drawBitmap(pixels, screen->pitch, _armyBackdrop,
		492, 206 + kSceneOriginY, screenBounds);
	for (uint i = 0; i < 4; ++i) {
		const int digit = _firstClockDigits[i];
		if (digit >= 0 && digit < (int)_armyFrames.size())
			IndexedBitmapRenderer::drawBitmap(pixels, screen->pitch,
				_armyFrames[digit], kArmyPositions[i].x,
				kArmyPositions[i].y + kSceneOriginY, screenBounds);
	}
	IndexedBitmapRenderer::drawBitmap(pixels, screen->pitch, _armyFrames[10],
		kArmyPositions[4].x, kArmyPositions[4].y + kSceneOriginY, screenBounds);

	for (uint i = 0; i < 4; ++i) {
		const int digit = _secondClockDigits[i];
		if (digit >= 0 && digit < 10)
			IndexedBitmapRenderer::drawBitmap(pixels, screen->pitch,
				_egyptFrames[digit], kEgyptPositions[i].x,
				kEgyptPositions[i].y + kSceneOriginY, screenBounds);
	}
	IndexedBitmapRenderer::drawBitmap(pixels, screen->pitch, _egyptFrames[10],
		kEgyptPositions[4].x, kEgyptPositions[4].y + kSceneOriginY, screenBounds);
	if (_halfDay)
		IndexedBitmapRenderer::drawBitmap(pixels, screen->pitch, _egyptFrames[11],
			kEgyptPositions[5].x, kEgyptPositions[5].y + kSceneOriginY, screenBounds);

	for (uint i = 0; i < 2; ++i)
		IndexedBitmapRenderer::drawBitmap(pixels, screen->pitch, _markerButtons[i],
			kControls[i + 5].x, kControls[i + 5].y + kSceneOriginY, screenBounds);
	g_system->unlockScreen();
	_backgroundDisplay.restorePalette();
	presentScreen();
}

void ClockPuzzle::playCue(uint cue) {
	if (cue >= kAudioCueCount)
		return;
	const Common::String path = Common::String::format("gb_clok%u.wav", cue);
	_engine->getMedia()->stopSoundEffect(_audioHandles[cue]);
	if (!_engine->getMedia()->playSoundEffect(path, _audioHandles[cue]))
		warning("Ripper: could not play clock puzzle audio cue '%s'", path.c_str());
}

void ClockPuzzle::stopAudio() {
	for (uint cue = 0; cue < kAudioCueCount; ++cue)
		_engine->getMedia()->stopSoundEffect(_audioHandles[cue]);
}

bool ClockPuzzle::playOverlay(uint index) {
	playCue(index == 0 ? 5 : 3);
	const Common::String path = Common::String::format("clock%u.smk", index);
	const bool played = _engine->getMedia()->playTransparentSmackerOverlay(path, 282, 77);
	debugC(2, kDebugPuzzles,
		"Ripper: clock puzzle overlay media='%s' index=%u success=%d",
		path.c_str(), index, played);
	return played;
}

uint16 ClockPuzzle::serviceKeyboard() {
	while (_engine->getInput()->hasPendingKey()) {
		const uint16 command = _engine->getInput()->consumeKey();
		if (command == kExitCommand || command == kHelpCommand)
			return command;
	}
	return 0;
}

int ClockPuzzle::findControl(const Common::Point &point) const {
	for (uint i = 0; i < ARRAYSIZE(kControls); ++i) {
		const ClockControlLayout &control = kControls[i];
		if (Common::Rect(control.x, control.y + kSceneOriginY,
				control.x + control.width,
				control.y + kSceneOriginY + control.height).contains(point))
			return i;
	}
	return -1;
}

bool ClockPuzzle::isExitRegion(const Common::Point &point) const {
	return Common::Rect(0, kSceneOriginY, 177, kSceneOriginY + 223).contains(point) ||
		Common::Rect(449, kSceneOriginY, 623, kSceneOriginY + 191).contains(point);
}

int ClockPuzzle::dialIndexAt(const Common::Point &point) const {
	const int maskY = point.y - kSceneOriginY;
	if (point.x < 0 || point.x >= (int)_dialHitMask.width ||
			maskY < 0 || maskY >= (int)_dialHitMask.height)
		return -1;
	const byte index = _dialHitMask.pixels[maskY * _dialHitMask.width + point.x];
	return index < kDialCount ? index : -1;
}

void ClockPuzzle::updateCursor(const Common::Point &point) {
	const int control = findControl(point);
	uint cursor = kDefaultCursor;
	if (control >= 0)
		cursor = kSelectionCursor;
	else if (isExitRegion(point))
		cursor = kExitCursor;
	if (control != _hoveredControl) {
		debugC(3, kDebugPuzzles,
			"Ripper: clock puzzle hover control=%d command=0x%04x point=%d,%d cursor=%u",
			control, control >= 0 ? kControls[control].command : 0,
			point.x, point.y, cursor);
		_hoveredControl = control;
	}
	_engine->getCursor()->update(cursor);
}

void ClockPuzzle::applyStepper(uint16 command) {
	int *digits = nullptr;
	int maximum = 0;
	bool increment = true;
	switch (command) {
	case 0x673:
		digits = _firstClockDigits;
		maximum = 23;
		break;
	case 0x674:
		digits = _firstClockDigits;
		maximum = 23;
		increment = false;
		break;
	case 0x675:
		digits = _firstClockDigits + 2;
		maximum = 59;
		break;
	case 0x676:
		digits = _firstClockDigits + 2;
		maximum = 59;
		increment = false;
		break;
	case 0x677: {
		int value = _secondClockDigits[0] * 10 + _secondClockDigits[1];
		if (value == 12) {
			value = 1;
			_halfDay = !_halfDay;
		} else {
			++value;
		}
		_secondClockDigits[0] = value / 10;
		_secondClockDigits[1] = value % 10;
		break;
	}
	case 0x678: {
		int value = _secondClockDigits[2] * 10 + _secondClockDigits[3];
		value = (value + 1) % 60;
		_secondClockDigits[2] = value / 10;
		_secondClockDigits[3] = value % 10;
		break;
	}
	default:
		return;
	}
	if (digits) {
		int value = digits[0] * 10 + digits[1];
		value = increment ? (value + 1) % (maximum + 1) :
			(value == 0 ? maximum : value - 1);
		digits[0] = value / 10;
		digits[1] = value % 10;
	}
	storePersistentState();
	debugC(2, kDebugPuzzles,
		"Ripper: clock puzzle stepper command=0x%04x first=%d%d:%d%d second=%d%d:%d%d halfDay=%d",
		command, _firstClockDigits[0], _firstClockDigits[1],
		_firstClockDigits[2], _firstClockDigits[3], _secondClockDigits[0],
		_secondClockDigits[1], _secondClockDigits[2], _secondClockDigits[3],
		_halfDay);
}

bool ClockPuzzle::isSolved() const {
	for (uint i = 0; i < 2; ++i) {
		if (_dialIndices[i] != kTargetDials[i])
			return false;
	}
	for (uint i = 0; i < 4; ++i) {
		if (_firstClockDigits[i] != kTargetFirstClock[i] ||
				_secondClockDigits[i] != kTargetSecondClock[i])
			return false;
	}
	return _halfDay == kTargetHalfDay;
}

void ClockPuzzle::loadPersistentState() {
	if (sClockStateOwner != _engine) {
		sClockStateOwner = _engine;
		memset(sDialIndices, 0, sizeof(sDialIndices));
		memset(sFirstClockDigits, 0, sizeof(sFirstClockDigits));
		memset(sSecondClockDigits, 0, sizeof(sSecondClockDigits));
		sHalfDay = false;
	}
	memcpy(_dialIndices, sDialIndices, sizeof(_dialIndices));
	memcpy(_firstClockDigits, sFirstClockDigits, sizeof(_firstClockDigits));
	memcpy(_secondClockDigits, sSecondClockDigits, sizeof(_secondClockDigits));
	_halfDay = sHalfDay;
}

void ClockPuzzle::storePersistentState() const {
	memcpy(sDialIndices, _dialIndices, sizeof(_dialIndices));
	memcpy(sFirstClockDigits, _firstClockDigits, sizeof(_firstClockDigits));
	memcpy(sSecondClockDigits, _secondClockDigits, sizeof(_secondClockDigits));
	sHalfDay = _halfDay;
}

ClockPuzzle::Result ClockPuzzle::run(uint completionFlag) {
	const uint puzzleLevel = CLIP<uint>(_engine->getSettings()->getPuzzleLevel(), 1, 3);
	debugC(1, kDebugPuzzles,
		"Ripper: entered clock puzzle completionFlag=%u level=%u helpTable=0x%x target=dials:7,8 first:14:35 second:09:35 halfDay=1",
		completionFlag, puzzleLevel, kHelpSelectionTable);
	if (!captureBackground() || !loadAssets())
		return kLoadFailed;

	loadPersistentState();
	_engine->getToolbar()->leave();
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	_engine->getCursor()->setSelectionIndex(kDefaultCursor);
	_engine->getCursor()->dispatchSelectionIndexChange(kDefaultCursor);
	render();
	if (!playOverlay(1)) {
		restoreBackground();
		stopAudio();
		return kLoadFailed;
	}
	render();
	_engine->getCursor()->refresh();

	Result result = kExited;
	bool finished = false;
	while (!_engine->shouldQuit() && !finished) {
		if (!serviceEngineEvents())
			break;
		const uint16 keyboardCommand = serviceKeyboard();
		if (keyboardCommand == kExitCommand) {
			finished = true;
			continue;
		}
		if (keyboardCommand == kHelpCommand) {
			debugC(1, kDebugPuzzles,
				"Ripper: clock puzzle opening modal help table=0x%x",
				kHelpSelectionTable);
			if (!_engine->getModalDialog()->run(kHelpSelectionTable))
				warning("Ripper: clock puzzle modal help failed");
			render();
		}

		const MouseState mouse = _engine->getInput()->publishMouseState();
		updateCursor(mouse.position);
		const int control = findControl(mouse.position);
		if (control == 0 && (mouse.buttons & (kMouseButtonLeft | kMouseButtonRight)) != 0) {
			const int dial = (mouse.buttons & kMouseButtonLeft) != 0 ? 0 : 1;
			const int dialIndex = dialIndexAt(mouse.position);
			if (_dragDial != dial) {
				_dragDial = dial;
				_lastDragIndex = -1;
			}
			if (dialIndex >= 0 && dialIndex != _lastDragIndex) {
				_dialIndices[dial] = dialIndex;
				_lastDragIndex = dialIndex;
				storePersistentState();
				playCue(0);
				render();
				debugC(2, kDebugPuzzles,
					"Ripper: clock puzzle dial=%d index=%d point=%d,%d buttons=0x%02x",
					dial, dialIndex, mouse.position.x, mouse.position.y, mouse.buttons);
			}
		} else if ((mouse.buttons & (kMouseButtonLeft | kMouseButtonRight)) == 0) {
			_dragDial = -1;
			_lastDragIndex = -1;
		}

		if ((mouse.pressed & kMouseButtonLeft) != 0) {
			if (control >= 1) {
				playCue(control < 5 ? 2 : 1);
				applyStepper(kControls[control].command);
				render();
			} else if (control < 0 && isExitRegion(mouse.position)) {
				finished = true;
			}
		}
		if ((mouse.buttons & kMouseButtonLeft) == 0 && isSolved()) {
			result = kSolved;
			finished = true;
			debugC(1, kDebugPuzzles,
				"Ripper: solved clock puzzle milestone=%u state=7,8/14:35/09:35/1",
				completionFlag);
		}
		presentScreen();
		g_system->delayMillis(10);
	}

	_engine->getCursor()->setVisible(false);
	render();
	if (!playOverlay(0) && !_engine->shouldQuit())
		result = kLoadFailed;
	if (result == kSolved &&
			!_engine->getMilestones()->set(completionFlag, true, "clock-puzzle"))
		result = kLoadFailed;
	stopAudio();
	restoreBackground();
	_engine->getCursor()->setSelectionIndex(0);
	_engine->getCursor()->dispatchSelectionIndexChange(0);
	_engine->getCursor()->refresh();
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	debugC(1, kDebugPuzzles,
		"Ripper: exited clock puzzle result=%d milestone=%u dials=%d,%d first=%d%d:%d%d second=%d%d:%d%d halfDay=%d",
		result, completionFlag, _dialIndices[0], _dialIndices[1],
		_firstClockDigits[0], _firstClockDigits[1], _firstClockDigits[2],
		_firstClockDigits[3], _secondClockDigits[0], _secondClockDigits[1],
		_secondClockDigits[2], _secondClockDigits[3], _halfDay);
	return result;
}

} // End of namespace Ripper
