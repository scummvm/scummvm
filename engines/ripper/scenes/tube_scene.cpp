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

#include "ripper/scenes/tube_scene.h"

#include "common/debug.h"
#include "common/system.h"
#include "graphics/surface.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/inventory.h"
#include "ripper/milestones.h"
#include "ripper/modal_dialog.h"
#include "ripper/ripper.h"
#include "ripper/toolbar.h"

namespace Ripper {

namespace {

static const char *const kTubeAudio[4] = {
	"tube0.wav", "tube1.wav", "tube2.wav", "tube3.wav"
};

static const uint kDefaultCursor = 14;
static const uint kSwitchCursor = 16;
static const uint kExitCursor = 7;
static const uint kToolbarMask = 0x84;
static const uint kInventoryAction = 2;
static const uint kHelpAction = 7;
static const uint kSceneHelpResource = 400;
static const uint kFirstTubeFlag = 52;
static const uint kLastTubeFlag = 54;
static const uint kConsumedFlagOffset = 50;
static const uint kSceneMediaX = 18;
static const uint kSceneMediaY = 52;
static const uint kSwitchAnimationDelay = 55;
static const int kSwitchLeft = 554;
static const int kSwitchTop = 223;
static const int kSwitchWidth = 43;
static const int kSwitchHeight = 54;

static const uint16 kEscapeCommand = 0x1b;
static const uint16 kHelpCommand = 0x3b00;
static const uint16 kToggleCommand = 0x672;
static const uint16 kReloadCommand = 0x7ff0;
static const uint16 kFailureCommand = 0x7ffd;

static bool isSwitchPoint(const Common::Point &point) {
	return point.x >= kSwitchLeft && point.x < kSwitchLeft + kSwitchWidth &&
		point.y >= kSwitchTop && point.y < kSwitchTop + kSwitchHeight;
}

static bool isExitPoint(const Common::Point &point) {
	return point.x >= 115 && point.x < 515 && point.y >= 115 && point.y < 325;
}

} // End of anonymous namespace

TubeScene::TubeScene(RipperEngine *engine) : Scene(engine), _tubeCount(0),
		_hoveredControl(-1), _switchOn(false), _secondaryCuePending(false) {
}

bool TubeScene::initialize() {
	if (!_engine->getResources()->loadBitmapLibrary("tubesw.pl", _switchFrames) ||
			_switchFrames.size() != 7) {
		warning("Ripper: tube scene expected seven TUBESW.PL switch frames");
		return false;
	}
	for (uint frame = 0; frame < _switchFrames.size(); ++frame) {
		if (_switchFrames[frame].width != kSwitchWidth ||
				_switchFrames[frame].height != kSwitchHeight) {
			warning("Ripper: tube scene switch frame=%u has invalid size=%ux%u",
				frame, _switchFrames[frame].width, _switchFrames[frame].height);
			return false;
		}
	}
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return false;
	}
	_switchBacking.resize(kSwitchWidth * kSwitchHeight);
	for (int y = 0; y < kSwitchHeight; ++y) {
		memcpy(_switchBacking.data() + y * kSwitchWidth,
			screen->getBasePtr(kSwitchLeft, kSwitchTop + y), kSwitchWidth);
	}
	g_system->unlockScreen();
	debugC(1, kDebugScene,
		"Ripper: initialized tube switch scene switchFrames=%u tubeFlags=%u..%u",
		_switchFrames.size(), kFirstTubeFlag, kLastTubeFlag);
	return true;
}

void TubeScene::drawSwitchFrame(uint frameIndex) const {
	if (frameIndex >= _switchFrames.size() ||
			_switchBacking.size() != kSwitchWidth * kSwitchHeight)
		return;
	const BitmapAssetFrame &frame = _switchFrames[frameIndex];
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}
	for (uint y = 0; y < frame.height; ++y) {
		byte *destination = (byte *)screen->getBasePtr(kSwitchLeft,
			kSwitchTop + y);
		memcpy(destination, _switchBacking.data() + y * kSwitchWidth, kSwitchWidth);
		const byte *source = frame.pixels.data() + y * frame.width;
		for (uint x = 0; x < frame.width; ++x) {
			if (source[x] != frame.transparentColor)
				destination[x] = source[x];
		}
	}
	g_system->unlockScreen();
}

void TubeScene::drawSwitch() const {
	drawSwitchFrame(_switchOn ? _switchFrames.size() - 1 : 0);
}

void TubeScene::animateSwitch(bool enabled) {
	playCue(enabled ? 2 : 3);
	for (uint step = 0; step < _switchFrames.size() && !_engine->shouldQuit(); ++step) {
		const uint frameIndex = enabled ? step : _switchFrames.size() - step - 1;
		drawSwitchFrame(frameIndex);
		presentScreen();
		g_system->delayMillis(kSwitchAnimationDelay);
	}
}

uint TubeScene::countInstalledTubes() const {
	uint count = 0;
	for (uint flag = kFirstTubeFlag + kConsumedFlagOffset;
			flag <= kLastTubeFlag + kConsumedFlagOffset; ++flag) {
		if (_engine->getMilestones()->isSet(flag))
			++count;
	}
	return count;
}

bool TubeScene::playCue(uint cue) {
	if (cue >= ARRAYSIZE(kTubeAudio))
		return false;
	return _engine->getMedia()->playSoundEffect(kTubeAudio[cue], _audioHandles[cue]);
}

void TubeScene::updateCursor(const Common::Point &point) {
	int hovered = -1;
	uint cursor = kDefaultCursor;
	if (isSwitchPoint(point)) {
		hovered = 0;
		cursor = kSwitchCursor;
	} else if (isExitPoint(point)) {
		hovered = 1;
		cursor = kExitCursor;
	}
	if (hovered != _hoveredControl) {
		debugC(2, kDebugInput,
			"Ripper: tube scene hover control=%d previous=%d point=%d,%d cursor=%u",
			hovered, _hoveredControl, point.x, point.y, cursor);
		_hoveredControl = hovered;
	}
	_engine->getCursor()->update(cursor);
	_engine->getCursor()->setVisible(true);
}

uint16 TubeScene::serviceInventory() {
	uint usedUnlockFlag = 0;
	// ExecuteUnlockSelectionChoice compares items 2..4 with the GAZ2 frame
	// label. Those are the three tubes consumed by RunTubeSwitchScene.
	const Inventory::Result inventoryResult =
		_engine->getInventory()->run("GAZ2", -1, &usedUnlockFlag);
	if (inventoryResult == Inventory::kLoadFailed)
		return kFailureCommand;
	if (inventoryResult == Inventory::kCancelled)
		return 0;
	if (usedUnlockFlag < kFirstTubeFlag || usedUnlockFlag > kLastTubeFlag) {
		warning("Ripper: tube scene inventory returned incompatible flag %u",
			usedUnlockFlag);
		return kFailureCommand;
	}
	debugC(1, kDebugScene,
		"Ripper: installed tube inventoryFlag=%u consumedFlag=%u",
		usedUnlockFlag, usedUnlockFlag + kConsumedFlagOffset);
	return kReloadCommand;
}

uint16 TubeScene::serviceInput() {
	while (_engine->getInput()->hasPendingKey()) {
		const uint16 command = _engine->getInput()->consumeKey();
		if (command == kEscapeCommand)
			return kEscapeCommand;
		if (command == kHelpCommand) {
			debugC(1, kDebugScene,
				"Ripper: tube scene opening modal help resource=%u", kSceneHelpResource);
			if (!_engine->getModalDialog()->run(kSceneHelpResource))
				return kFailureCommand;
		}
	}

	const MouseState mouse = _engine->getInput()->publishMouseState();
	int toolbarAction = -1;
	if (_engine->getToolbar()->service(mouse, kToolbarMask, &toolbarAction)) {
		_engine->getCursor()->update(kDefaultCursor);
		_engine->getCursor()->setVisible(true);
		if (toolbarAction == kInventoryAction)
			return serviceInventory();
		if (toolbarAction == kHelpAction &&
				!_engine->getModalDialog()->run(kSceneHelpResource))
			return kFailureCommand;
		return 0;
	}

	updateCursor(mouse.position);
	if ((mouse.pressed & kMouseButtonLeft) == 0)
		return 0;
	if (_hoveredControl == 0)
		return kToggleCommand;
	if (_hoveredControl == 1)
		return kEscapeCommand;
	return 0;
}

uint16 TubeScene::service(uint frame) {
	if (_secondaryCuePending && frame == 16) {
		playCue(1);
		_secondaryCuePending = false;
		debugC(3, kDebugScene,
			"Ripper: tube scene queued running cue at mediaFrame=15");
	}
	const uint16 command = serviceInput();
	drawSwitch();
	presentScreen();
	return command;
}

bool TubeScene::playSegment(uint firstFrame, uint lastFrame, uint16 &command,
		uint loopStartFrame) {
	command = 0;
	_secondaryCuePending = firstFrame <= 15 && lastFrame >= 15;
	const Common::String media = Common::String::format("ga_tube%u.smk", _tubeCount + 1);
	const bool result = _engine->getMedia()->playPuzzleSequenceSegment(media,
		firstFrame, lastFrame, kSceneMediaX, kSceneMediaY, this, &command,
		loopStartFrame);
	debugC(result ? 2 : 1, kDebugScene,
		"Ripper: tube scene media segment='%s' frames=%u..%u loopStart=%d command=0x%04x result=%d",
		media.c_str(), firstFrame, lastFrame,
		loopStartFrame == 0xffffffff ? -1 : (int)loopStartFrame, command, result);
	return result;
}

void TubeScene::stopAudio() {
	for (uint cue = 0; cue < ARRAYSIZE(_audioHandles); ++cue)
		Scene::stopAudio(_audioHandles[cue]);
}

TubeScene::Result TubeScene::run(uint completionFlag) {
	if (!initialize())
		return kLoadFailed;
	prepare("tube-entry", kDefaultCursor, true);
	_tubeCount = countInstalledTubes();
	debugC(1, kDebugScene,
		"Ripper: entered tube switch scene completionFlag=%u installed=%u toolbarMask=0x%03x",
		completionFlag, _tubeCount, kToolbarMask);

	Result result = kExited;
	uint16 command = 0;
	if (!playSegment(0, 0, command))
		result = kLoadFailed;
	bool active = result != kLoadFailed;
	while (active && !_engine->shouldQuit()) {
		if (command == kFailureCommand) {
			result = kLoadFailed;
			break;
		}
		if (command == kEscapeCommand)
			break;
		if (command == kReloadCommand) {
			const uint previousCount = _tubeCount;
			_tubeCount = countInstalledTubes();
			if (_tubeCount != previousCount) {
				if (!_engine->getMedia()->play("tubehand.avi", true)) {
					result = kLoadFailed;
					break;
				}
				debugC(1, kDebugScene,
					"Ripper: tube scene reloaded installed tubes previous=%u current=%u",
					previousCount, _tubeCount);
			}
			command = 0;
			if (!playSegment(_switchOn ? 15 : 0, _switchOn ? 45 : 0, command,
					_switchOn && _tubeCount <= 2 ? 15 : 0xffffffff)) {
				result = kLoadFailed;
				break;
			}
			continue;
		}
		if (command == kToggleCommand) {
			if (_switchOn) {
				_switchOn = false;
				_engine->getMedia()->stopSoundEffect(_audioHandles[1]);
				playCue(0);
				animateSwitch(false);
				if (!playSegment(46, 59, command)) {
					result = kLoadFailed;
					break;
				}
			} else {
				_switchOn = true;
				playCue(0);
				animateSwitch(true);
				if (!playSegment(0, 45, command,
						_tubeCount <= 2 ? 15 : 0xffffffff)) {
					result = kLoadFailed;
					break;
				}
				if (command == 0 && _tubeCount > 2) {
					if (!_engine->getMedia()->play("tube_win.avi", true) ||
							!_engine->getMilestones()->set(completionFlag, true,
								"tube-switch-scene")) {
						result = kLoadFailed;
					} else {
						result = kSolved;
					}
					break;
				}
			}
			continue;
		}
		if (_switchOn) {
			if (!playSegment(15, 45, command,
					_tubeCount <= 2 ? 15 : 0xffffffff)) {
				result = kLoadFailed;
				break;
			}
			if (command == 0 && _tubeCount > 2) {
				if (!_engine->getMedia()->play("tube_win.avi", true) ||
						!_engine->getMilestones()->set(completionFlag, true,
							"tube-switch-scene")) {
					result = kLoadFailed;
				} else {
					result = kSolved;
				}
				break;
			}
		} else {
			if (_engine->getInput()->pollEvents()) {
				_engine->quitGame();
				break;
			}
			command = serviceInput();
			drawSwitch();
			presentScreen();
			g_system->delayMillis(10);
		}
	}

	stopAudio();
	finish("tube-exit", 0, false);
	debugC(result == kLoadFailed ? 2 : 1, kDebugScene,
		"Ripper: left tube switch scene result=%d installed=%u milestone=%u quit=%d",
		result, _tubeCount, completionFlag, _engine->shouldQuit());
	return result;
}

} // End of namespace Ripper
