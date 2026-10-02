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

#include "ripper/puzzles/date_selection.h"

#include "common/debug.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/system.h"
#include "graphics/surface.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/display.h"
#include "ripper/input.h"
#include "ripper/milestones.h"
#include "ripper/modal_dialog.h"
#include "ripper/ripper.h"
#include "ripper/toolbar.h"

namespace Ripper {

namespace {

static const uint kAudioCueCount = 5;
static const uint kDefaultCursor = 14;
static const uint kHelpSelectionTable = 0x1b6;
static const uint kMaximumDateLength = 0x16;
static const uint kMediaLoopStartFrame = 10;
static const uint kDosTickMillis = 55;
static const uint kFeedbackStepCount = 17;
static const uint kFirstDateFlag = 339;
static const uint kSecondDateFlag = 340;
static const uint kRemainingDatesFlag = 341;
static const uint16 kExitCommand = 4;
static const uint16 kSubmitCommand = 5;
static const uint16 kLoadFailedCommand = 2;

static const char *const kWaitBitmap = "eh_wait.bbm";
static const Common::Point kWaitPosition(186, 275);
static const int kFeedbackStartX = 195;
static const int kFeedbackY = 294;
static const int kFeedbackWidth = 18;
static const int kFeedbackHeight = 8;
static const int kFeedbackAdvance = 8;
static const byte kFeedbackColor = 0x5e;

static Common::String mediaName(uint index) {
	return Common::String::format("eh_eeg%u.smk", index);
}

static Common::String cueName(uint index) {
	return Common::String::format("eh_eeg%u.wav", index);
}

static Common::Rect textEntryBounds() {
	return Common::Rect(190, 280, 336, 310);
}

} // End of anonymous namespace

DateSelectionPuzzle::DateSelectionPuzzle(RipperEngine *engine) :
		Puzzle(engine), _mediaIndex(0),
		_textEntryActive(false), _result(kExited) {
}

bool DateSelectionPuzzle::loadAssets() {
	Common::ScopedPtr<Common::SeekableReadStream> waitStream(
		_engine->getResources()->createReadStreamForPath(kWaitBitmap));
	if (!waitStream || !decodeBitmapAsset(*waitStream, _waitFrame)) {
		warning("Ripper: could not decode date-selection feedback bitmap '%s'",
			kWaitBitmap);
		return false;
	}

	for (uint index = 0; index < 2; ++index) {
		const Common::String name = mediaName(index);
		Common::ScopedPtr<Common::SeekableReadStream> stream(
			_engine->getResources()->createReadStreamForPath(name));
		if (!stream) {
			warning("Ripper: date-selection media '%s' is missing", name.c_str());
			return false;
		}
	}
	for (uint index = 0; index < kAudioCueCount; ++index) {
		const Common::String name = cueName(index);
		Common::ScopedPtr<Common::SeekableReadStream> stream(
			_engine->getResources()->createReadStreamForPath(name));
		if (!stream) {
			warning("Ripper: date-selection audio '%s' is missing", name.c_str());
			return false;
		}
	}

	debugC(2, kDebugPuzzles,
		"Ripper: loaded date-selection assets feedback='%s' size=%ux%u media=2 audio=%u",
		kWaitBitmap, _waitFrame.width, _waitFrame.height, kAudioCueCount);
	return true;
}

bool DateSelectionPuzzle::playCue(uint cue, uint volumePercent, bool loop) {
	if (cue >= kAudioCueCount)
		return false;
	const Common::String name = cueName(cue);
	Common::SeekableReadStream *stream =
		_engine->getResources()->createReadStreamForPath(name);
	if (!stream || !_engine->getMedia()->playSoundEffectStream(
			stream, name, _audioHandles[cue], volumePercent, loop)) {
		warning("Ripper: could not play date-selection audio '%s'", name.c_str());
		return false;
	}
	debugC(2, kDebugPuzzles,
		"Ripper: started date-selection audio cue=%u name='%s' volume=%u loop=%d",
		cue, name.c_str(), volumePercent, loop);
	return true;
}

void DateSelectionPuzzle::stopAudio() {
	stopAudioHandles(_audioHandles, ARRAYSIZE(_audioHandles));
}

DateSelectionPuzzle::DateGroup DateSelectionPuzzle::matchDate(
		const Common::String &text, uint &day) const {
	for (day = 18; day <= 23; ++day) {
		const Common::String forms[] = {
			Common::String::format("11-%u", day),
			Common::String::format("11/%u", day),
			Common::String::format("nov %u", day),
			Common::String::format("nov. %u", day),
			Common::String::format("november %u", day)
		};
		for (uint form = 0; form < ARRAYSIZE(forms); ++form) {
			if (text.equalsIgnoreCase(forms[form])) {
				if (day == 19)
					return kNovember19;
				if (day == 20)
					return kNovember20;
				return kOtherAcceptedDate;
			}
		}
	}
	day = 0;
	return kInvalidDate;
}

bool DateSelectionPuzzle::waitTicks(uint ticks) {
	const uint32 target = g_system->getMillis() + ticks * kDosTickMillis;
	while (!_engine->shouldQuit() &&
			(int32)(target - g_system->getMillis()) > 0) {
		if (!serviceEngineEvents())
			return false;
		g_system->delayMillis(10);
	}
	return !_engine->shouldQuit();
}

bool DateSelectionPuzzle::drawFeedback() {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		warning("Ripper: could not draw date-selection feedback on active display");
		return false;
	}
	IndexedBitmapRenderer::drawBitmap((byte *)screen->getPixels(), screen->pitch,
		_waitFrame, kWaitPosition.x, kWaitPosition.y,
		Common::Rect(0, 0, screen->w, screen->h));
	g_system->unlockScreen();
	presentScreen();

	for (uint step = 0; step < kFeedbackStepCount; ++step) {
		screen = g_system->lockScreen();
		if (!screen || screen->format.bytesPerPixel != 1) {
			if (screen)
				g_system->unlockScreen();
			return false;
		}
		const int x = kFeedbackStartX + step * kFeedbackAdvance;
		screen->fillRect(Common::Rect(x, kFeedbackY,
			x + kFeedbackWidth, kFeedbackY + kFeedbackHeight),
			kFeedbackColor);
		g_system->unlockScreen();
		presentScreen();
		debugC(3, kDebugPuzzles,
			"Ripper: date-selection feedback step=%u/%u x=%d y=%d",
			step + 1, kFeedbackStepCount, x, kFeedbackY);
		if (!waitTicks(1))
			return false;
	}
	return true;
}

bool DateSelectionPuzzle::processSubmission() {
	uint day = 0;
	const DateGroup group = matchDate(_submittedText, day);
	uint flag = 0;
	_mediaIndex = 0;
	_engine->getMedia()->stopSoundEffect(_audioHandles[0]);
	_engine->getMedia()->stopSoundEffect(_audioHandles[1]);

	if (group == kNovember19) {
		flag = kFirstDateFlag;
		_mediaIndex = 1;
	} else if (group == kNovember20) {
		flag = kSecondDateFlag;
		_mediaIndex = 1;
	} else if (group == kOtherAcceptedDate) {
		flag = kRemainingDatesFlag;
	}

	if (flag != 0 && !_engine->getMilestones()->set(
			flag, true, "date-selection-puzzle")) {
		return false;
	}

	debugC(group == kInvalidDate ? 2 : 1, kDebugPuzzles,
		"Ripper: date-selection submission text='%s' day=%u group=%d flag=%u nextMedia=%u",
		_submittedText.c_str(), day, group, flag, _mediaIndex);
	if (!drawFeedback())
		return false;

	if (group == kInvalidDate) {
		if (!playCue(4) || !playCue(0))
			return false;
	} else if (_mediaIndex == 1) {
		if (!playCue(1))
			return false;
	} else if (!playCue(0)) {
		return false;
	}
	return true;
}

uint16 DateSelectionPuzzle::service(uint frame) {
	if (!_textEntryActive) {
		const Common::Rect bounds = textEntryBounds();
		if (!_engine->getModalDialog()->beginTextEntry(Common::String(),
				kMaximumDateLength, kHelpSelectionTable,
				"date-selection-puzzle",
				ModalDialogManager::kSceneEntryPresentation,
				bounds)) {
			_result = kLoadFailed;
			return kLoadFailedCommand;
		}
		_textEntryActive = true;
		debugC(2, kDebugPuzzles,
			"Ripper: date-selection text entry armed frame=%u bounds=%d,%d,%d,%d max=%u help=0x%x",
			frame, bounds.left, bounds.top, bounds.width(), bounds.height(),
			kMaximumDateLength, kHelpSelectionTable);
	}

	Common::String text;
	const ModalDialogManager::TextEntryResult result =
		_engine->getModalDialog()->serviceTextEntry(text);
	if (result == ModalDialogManager::kTextEntryPending)
		return 0;

	_textEntryActive = false;
	if (result == ModalDialogManager::kTextEntryAccepted) {
		_submittedText = text;
		debugC(2, kDebugPuzzles,
			"Ripper: date-selection accepted text length=%u frame=%u",
			_submittedText.size(), frame);
		return kSubmitCommand;
	}
	if (result == ModalDialogManager::kTextEntryCancelled) {
		debugC(1, kDebugPuzzles,
			"Ripper: date-selection text entry cancelled frame=%u", frame);
		return kExitCommand;
	}

	_result = kLoadFailed;
	return kLoadFailedCommand;
}

DateSelectionPuzzle::Result DateSelectionPuzzle::run(uint completionFlag) {
	// DispatchSceneEntryAction at 0x36892 maps action 17 to
	// RunDateSelectionPuzzleScene at 0x3d5ed.
	if (!loadAssets())
		return kLoadFailed;

	_mediaIndex = 0;
	_textEntryActive = false;
	_submittedText.clear();
	_result = kExited;
	_engine->getToolbar()->leave();
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	_engine->getCursor()->setSelectionIndex(kDefaultCursor);
	_engine->getCursor()->dispatchSelectionIndexChange(kDefaultCursor);
	_engine->getCursor()->setVisible(true);
	if (!playCue(0) || !playCue(2, 10))
		_result = kLoadFailed;

	debugC(1, kDebugPuzzles,
		"Ripper: entered date-selection puzzle function="
		"RunDateSelectionPuzzleScene@0x3d5ed milestone=%u help=0x%x "
		"progressFlags=%u,%u,%u maxLength=%u",
		completionFlag, kHelpSelectionTable, kFirstDateFlag,
		kSecondDateFlag, kRemainingDatesFlag, kMaximumDateLength);

	while (_result != kLoadFailed && !_engine->shouldQuit()) {
		const Common::String name = mediaName(_mediaIndex);
		Common::SeekableReadStream *stream =
			_engine->getResources()->createReadStreamForPath(name);
		uint16 command = 0;
		if (!_engine->getMedia()->playPuzzleSequenceStream(stream, name,
				0, 0, kMediaLoopStartFrame, this, &command)) {
			_result = kLoadFailed;
			break;
		}
		debugC(2, kDebugPuzzles,
			"Ripper: date-selection media completed name='%s' command=0x%04x submittedLength=%u",
			name.c_str(), command, _submittedText.size());
		if (command == kExitCommand)
			break;
		if (command == kLoadFailedCommand) {
			_result = kLoadFailed;
			break;
		}
		if (command != kSubmitCommand || !processSubmission()) {
			_result = kLoadFailed;
			break;
		}
	}

	stopAudio();
	if (_result != kLoadFailed &&
			_engine->getMilestones()->isSet(kFirstDateFlag) &&
			_engine->getMilestones()->isSet(kSecondDateFlag) &&
			_engine->getMilestones()->isSet(kRemainingDatesFlag)) {
		if (!markSolved(completionFlag, "date-selection-puzzle")) {
			_result = kLoadFailed;
		} else {
			_result = kSolved;
		}
	}

	_engine->getCursor()->setSelectionIndex(0);
	_engine->getCursor()->dispatchSelectionIndexChange(0);
	_engine->getCursor()->refresh();
	_engine->getCursor()->setVisible(true);
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	debugC(_result == kLoadFailed ? 2 : 1, kDebugPuzzles,
		"Ripper: left date-selection puzzle result=%d milestone=%u milestoneSet=%d "
		"progress=%d,%d,%d quit=%d",
		_result, completionFlag,
		_engine->getMilestones()->isSet(completionFlag),
		_engine->getMilestones()->isSet(kFirstDateFlag),
		_engine->getMilestones()->isSet(kSecondDateFlag),
		_engine->getMilestones()->isSet(kRemainingDatesFlag),
		_engine->shouldQuit());
	return _result;
}

} // End of namespace Ripper
