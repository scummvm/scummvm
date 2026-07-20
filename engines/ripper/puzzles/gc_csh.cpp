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

#include "ripper/puzzles/gc_csh.h"

#include "common/debug.h"
#include "common/system.h"
#include "graphics/surface.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/media.h"
#include "ripper/milestones.h"
#include "ripper/modal_dialog.h"
#include "ripper/ripper.h"

namespace Ripper {

namespace {

struct ChoiceLayout {
	int y;
	int x;
	int width;
	int height;
};

static const uint kChoiceCount = 4;
static const uint kSequenceLength = 4;
static const uint kAnimationFrameCount = 6;
static const uint kAudioCueCount = 2;
static const int kSceneOriginY = 50;
static const uint kDosTickDurationMillis = 55;
static const uint kDefaultCursor = 14;
static const uint kChoiceCursor = 16;
static const uint kExitCursor = 7;
static const uint16 kFirstChoiceCommand = 0x672;
static const uint16 kExitCommand = 0x1b;
static const uint16 kHelpCommand = 0x3b00;
static const uint kHelpSelectionTable = 0x1a3;

// g_gcCshPuzzleChoiceControlRecords at 0x84700 stores y, x, width,
// and height in the original display's vertical/horizontal order.
static const ChoiceLayout kChoiceLayouts[kChoiceCount] = {
	{52,  91, 10, 10},
	{54, 111, 10, 10},
	{56, 127,  9, 10},
	{56, 139,  9,  8}
};

// g_gcCshPuzzleSequenceSlotDirtyRegions at 0x846ec stores y/x pairs.
static const Common::Point kSequenceSlotPositions[kSequenceLength] = {
	Common::Point(424, 42 + kSceneOriginY),
	Common::Point(440, 42 + kSceneOriginY),
	Common::Point(455, 42 + kSceneOriginY),
	Common::Point(470, 42 + kSceneOriginY)
};

// The display command at 0x38959 captures this exact backing strip before
// input begins; the reset path at 0x38d3a restores it after the reverse pass.
static const int kSequenceBackingX = 421;
static const int kSequenceBackingY = 51 + kSceneOriginY;
static const int kSequenceBackingWidth = 98;
static const int kSequenceBackingHeight = 5;

// The two Escape controls created by RunGcCshFourChoiceSequencePuzzleScene
// at 0x38871 use these original y/x/width/height values.
static const int kLeftExitY = 0x9a + kSceneOriginY;
static const int kLeftExitWidth = 0x9f;
static const int kLeftExitHeight = 0x92;
static const int kRightExitY = 0xf9 + kSceneOriginY;
static const int kRightExitX = 0xa0;
static const int kRightExitWidth = 0x1e0;
static const int kRightExitHeight = 0x33;

static const int kTargetSequence[kSequenceLength] = {1, 2, 3, 1};

} // End of anonymous namespace

GcCshPuzzle::GcCshPuzzle(RipperEngine *engine) :
		_engine(engine), _hoveredChoice(-1) {
}

bool GcCshPuzzle::captureBackground() {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1 || screen->w != 640 || screen->h != 400) {
		if (screen)
			g_system->unlockScreen();
		return false;
	}

	_backgroundPixels.resize(screen->w * screen->h);
	for (int y = 0; y < screen->h; ++y)
		memcpy(_backgroundPixels.data() + y * screen->w,
			screen->getBasePtr(0, y), screen->w);
	g_system->unlockScreen();
	return true;
}

bool GcCshPuzzle::loadAssets() {
	for (uint choice = 0; choice < kChoiceCount; ++choice) {
		_choiceFrames[choice].clear();
		for (uint frameIndex = 0; frameIndex < kAnimationFrameCount; ++frameIndex) {
			BitmapAssetSequence sequence;
			const Common::String name = Common::String::format(
				"gc_csh%u%u.bbm", choice, frameIndex);
			if (!_engine->getResources()->loadBitmapSequence(name, sequence) ||
					sequence.frames.empty()) {
				warning("Ripper: could not load GC/CSH choice bitmap '%s'", name.c_str());
				return false;
			}
			_choiceFrames[choice].push_back(Common::move(sequence.frames.front()));
		}
	}

	debugC(1, kDebugPuzzles,
		"Ripper: loaded GC/CSH puzzle assets choices=%u framesPerChoice=%u audioCues=%u",
		kChoiceCount, kAnimationFrameCount, kAudioCueCount);
	return true;
}

void GcCshPuzzle::drawFrame(byte *screen, uint pitch,
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

void GcCshPuzzle::drawAnimationFrame(uint choice, uint sequenceSlot,
		uint frameIndex) {
	if (choice >= kChoiceCount || sequenceSlot >= kSequenceLength ||
			frameIndex >= _choiceFrames[choice].size())
		return;

	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}
	const Common::Point &position = kSequenceSlotPositions[sequenceSlot];
	drawFrame((byte *)screen->getPixels(), screen->pitch,
		_choiceFrames[choice][frameIndex], position.x, position.y);
	g_system->unlockScreen();
	g_system->updateScreen();
}

void GcCshPuzzle::drawResetFrame(const int enteredChoices[4],
		uint enteredChoiceCount, uint frameIndex) {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}
	for (uint slot = 0; slot < enteredChoiceCount; ++slot) {
		const uint choice = enteredChoices[slot];
		if (choice >= kChoiceCount || frameIndex >= _choiceFrames[choice].size())
			continue;
		const Common::Point &position = kSequenceSlotPositions[slot];
		drawFrame((byte *)screen->getPixels(), screen->pitch,
			_choiceFrames[choice][frameIndex], position.x, position.y);
	}
	g_system->unlockScreen();
	g_system->updateScreen();
}

void GcCshPuzzle::restoreSequenceBacking() {
	if (_backgroundPixels.size() != 640 * 400)
		return;

	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1 || screen->w != 640 || screen->h != 400) {
		if (screen)
			g_system->unlockScreen();
		return;
	}
	for (int y = 0; y < kSequenceBackingHeight; ++y) {
		memcpy(screen->getBasePtr(kSequenceBackingX, kSequenceBackingY + y),
			_backgroundPixels.data() + (kSequenceBackingY + y) * 640 +
				kSequenceBackingX,
			kSequenceBackingWidth);
	}
	g_system->unlockScreen();
	g_system->updateScreen();
}

void GcCshPuzzle::playCue(uint cue) {
	if (cue >= kAudioCueCount)
		return;
	const Common::String path = Common::String::format("csh_snd%u.wav", cue);
	if (!_engine->getMedia()->playSoundEffect(path, _audioHandles[cue]))
		warning("Ripper: could not play GC/CSH audio cue '%s'", path.c_str());
}

void GcCshPuzzle::stopAudio() {
	for (uint cue = 0; cue < kAudioCueCount; ++cue)
		_engine->getMedia()->stopSoundEffect(_audioHandles[cue]);
}

bool GcCshPuzzle::waitForCue(uint cue) {
	if (cue >= kAudioCueCount)
		return false;
	while (!_engine->shouldQuit() &&
			_engine->getMedia()->isSoundEffectActive(_audioHandles[cue])) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			return false;
		}
		g_system->delayMillis(10);
	}
	return !_engine->shouldQuit();
}

bool GcCshPuzzle::playChoice(uint choice, uint sequenceSlot) {
	if (choice >= kChoiceCount || sequenceSlot >= kSequenceLength)
		return false;

	_engine->getCursor()->setVisible(false);
	playCue(0);
	const Common::String media = Common::String::format("gc_csh%u.avi", choice);
	// RunGcCshFourChoiceSequencePuzzleScene at 0x38b88 uses the generic
	// uncontrolled RunMediaPresentation path, leaving its final IAVF display.
	if (!_engine->getMedia()->play(media, false, 0, 64, true)) {
		warning("Ripper: could not play GC/CSH choice media '%s'", media.c_str());
		_engine->getCursor()->setVisible(true);
		return false;
	}
	playCue(1);
	for (uint frameIndex = 0; frameIndex < _choiceFrames[choice].size(); ++frameIndex) {
		drawAnimationFrame(choice, sequenceSlot, frameIndex);
		g_system->delayMillis(kDosTickDurationMillis);
	}
	_engine->getCursor()->setVisible(true);
	return true;
}

bool GcCshPuzzle::resetSequence(const int enteredChoices[4],
		uint enteredChoiceCount) {
	if (enteredChoiceCount == 0)
		return true;

	_engine->getCursor()->setVisible(false);
	playCue(1);
	for (uint frameIndex = kAnimationFrameCount; frameIndex > 0; --frameIndex) {
		drawResetFrame(enteredChoices, enteredChoiceCount, frameIndex - 1);
		g_system->delayMillis(kDosTickDurationMillis);
	}
	restoreSequenceBacking();
	const bool completed = waitForCue(1);
	_engine->getCursor()->setVisible(true);
	debugC(2, kDebugPuzzles,
		"Ripper: reset GC/CSH sequence choices=%u audioComplete=%d",
		enteredChoiceCount, completed);
	return completed;
}

int GcCshPuzzle::findChoice(const Common::Point &point) const {
	for (uint choice = 0; choice < kChoiceCount; ++choice) {
		const ChoiceLayout &layout = kChoiceLayouts[choice];
		if (Common::Rect(layout.x, layout.y + kSceneOriginY,
				layout.x + layout.width,
				layout.y + kSceneOriginY + layout.height).contains(point))
			return choice;
	}
	return -1;
}

bool GcCshPuzzle::isExitRegion(const Common::Point &point) const {
	return Common::Rect(0, kLeftExitY, kLeftExitWidth,
		kLeftExitY + kLeftExitHeight).contains(point) ||
		Common::Rect(kRightExitX, kRightExitY,
			kRightExitX + kRightExitWidth,
			kRightExitY + kRightExitHeight).contains(point);
}

void GcCshPuzzle::updateCursor(const Common::Point &point) {
	const int hoveredChoice = findChoice(point);
	uint cursor = kDefaultCursor;
	if (hoveredChoice >= 0)
		cursor = kChoiceCursor;
	else if (isExitRegion(point))
		cursor = kExitCursor;
	if (hoveredChoice != _hoveredChoice) {
		debugC(3, kDebugPuzzles,
			"Ripper: GC/CSH hover choice=%d previous=%d point=%d,%d cursor=%u",
			hoveredChoice, _hoveredChoice, point.x, point.y, cursor);
		_hoveredChoice = hoveredChoice;
	}
	_engine->getCursor()->update(cursor);
}

uint16 GcCshPuzzle::serviceInput() {
	while (_engine->getInput()->hasPendingKey()) {
		const uint16 command = _engine->getInput()->consumeKey();
		if (command == kHelpCommand || command == kExitCommand)
			return command;
	}

	const MouseState mouse = _engine->getInput()->publishMouseState();
	updateCursor(mouse.position);
	if ((mouse.pressed & kMouseButtonLeft) == 0)
		return 0;
	const int choice = findChoice(mouse.position);
	if (choice >= 0)
		return kFirstChoiceCommand + choice;
	if (isExitRegion(mouse.position))
		return kExitCommand;
	return 0;
}

GcCshPuzzle::Result GcCshPuzzle::run(uint completionFlag) {
	debugC(1, kDebugPuzzles,
		"Ripper: entered GC/CSH puzzle completionFlag=%u helpTable=0x%x target=2,3,4,2",
		completionFlag, kHelpSelectionTable);
	if (!loadAssets() || !captureBackground())
		return kLoadFailed;

	int enteredChoices[kSequenceLength] = {-1, -1, -1, -1};
	uint enteredChoiceCount = 0;
	Result result = kExited;
	_engine->getInput()->discardMouseTransitions();
	_engine->getCursor()->setVisible(true);
	_engine->getCursor()->update(kDefaultCursor);

	while (!_engine->shouldQuit() && result == kExited) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			break;
		}
		const uint16 command = serviceInput();
		_engine->getInput()->drainKeys();
		if (command == kHelpCommand) {
			debugC(1, kDebugPuzzles,
				"Ripper: GC/CSH puzzle opening modal help table=0x%x entered=%u",
				kHelpSelectionTable, enteredChoiceCount);
			if (!_engine->getModalDialog()->run(kHelpSelectionTable))
				warning("Ripper: GC/CSH puzzle modal help failed");
			updateCursor(_engine->getInput()->peekMouseState().position);
			continue;
		}
		if (command == kExitCommand) {
			if (!resetSequence(enteredChoices, enteredChoiceCount))
				result = kLoadFailed;
			break;
		}
		if (command >= kFirstChoiceCommand &&
				command < kFirstChoiceCommand + kChoiceCount) {
			const uint choice = command - kFirstChoiceCommand;
			debugC(2, kDebugPuzzles,
				"Ripper: GC/CSH choice=%u slot=%u control=0x%04x",
				choice + 1, enteredChoiceCount, command);
			if (!playChoice(choice, enteredChoiceCount)) {
				result = kLoadFailed;
				break;
			}
			enteredChoices[enteredChoiceCount++] = choice;
			bool solved = true;
			for (uint slot = 0; slot < kSequenceLength; ++slot) {
				if (enteredChoices[slot] != kTargetSequence[slot])
					solved = false;
			}
			if (solved) {
				// The original takes the shared four-entry reset path even after
				// a match, then observes the retained solved flag at the loop head.
				if (!resetSequence(enteredChoices, enteredChoiceCount)) {
					result = kLoadFailed;
					break;
				}
				if (!_engine->getMilestones()->set(completionFlag, true,
						"gc-csh-puzzle")) {
					result = kLoadFailed;
					break;
				}
				result = kSolved;
				debugC(1, kDebugPuzzles,
					"Ripper: completed GC/CSH puzzle milestone=%u sequence=2,3,4,2",
					completionFlag);
				break;
			}
			if (enteredChoiceCount == kSequenceLength) {
				if (!resetSequence(enteredChoices, enteredChoiceCount)) {
					result = kLoadFailed;
					break;
				}
				for (uint slot = 0; slot < kSequenceLength; ++slot)
					enteredChoices[slot] = -1;
				enteredChoiceCount = 0;
			}
		}
		g_system->delayMillis(10);
	}

	stopAudio();
	_engine->getCursor()->setVisible(true);
	_engine->getCursor()->update(0);
	debugC(1, kDebugPuzzles,
		"Ripper: exited GC/CSH puzzle result=%d milestone=%u entered=%u",
		result, completionFlag, enteredChoiceCount);
	return result;
}

} // End of namespace Ripper
