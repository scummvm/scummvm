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

#include "ripper/puzzles/rolodex.h"

#include "common/debug.h"
#include "common/rect.h"
#include "common/system.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/milestones.h"
#include "ripper/modal_dialog.h"
#include "ripper/ripper.h"

namespace Ripper {

namespace {

static const int kSceneOriginY = 50;
static const int kAdvanceLeft = 0x79;
static const int kAdvanceTop = kSceneOriginY + 0x33;
static const int kAdvanceWidth = 0x16d;
static const int kAdvanceHeight = 0xc0;
static const int kExitLeft = 0x21b;
static const int kExitTop = kSceneOriginY + 0x16;
static const int kExitWidth = 0x65;
static const int kExitHeight = 0xff;
static const uint kDefaultCursor = 0;
static const uint kAdvanceCursor = 0x10;
static const uint kExitCursor = 7;
static const uint16 kAdvanceCommand = 0x0d;
static const uint16 kExitCommand = 0x1b;
static const uint16 kHelpCommand = 0x3b00;
static const uint kHelpSelectionTable = 0x19f;
static const uint kNormalLoopStartFrame = 0x14;
static const uint kSuccessLoopStartFrame = 0x16;

static const uint kSequenceAudioStopFrames[] = {
	0, 10, 13, 17, 16, 16, 22
};

static bool isInsideAdvanceControl(const Common::Point &point) {
	return Common::Rect(kAdvanceLeft, kAdvanceTop,
		kAdvanceLeft + kAdvanceWidth, kAdvanceTop + kAdvanceHeight).contains(point);
}

static bool isInsideExitControl(const Common::Point &point) {
	return Common::Rect(kExitLeft, kExitTop,
		kExitLeft + kExitWidth, kExitTop + kExitHeight).contains(point);
}

} // End of anonymous namespace

RolodexPuzzle::RolodexPuzzle(RipperEngine *engine) : Puzzle(engine),
		_activeSequence(0), _acceptInput(false) {
}

void RolodexPuzzle::startAudio(uint index, uint volumePercent, bool loop) {
	if (index >= ARRAYSIZE(_audioHandles))
		return;
	const Common::String path = Common::String::format("rolo%u.wav", index);
	if (!_engine->getMedia()->playSoundEffect(path, _audioHandles[index], volumePercent, loop)) {
		warning("Ripper: rolodex puzzle could not start audio '%s'", path.c_str());
		return;
	}
	debugC(3, kDebugPuzzles,
		"Ripper: rolodex audio started index=%u frameSequence=%u volume=%u loop=%d",
		index, _activeSequence, volumePercent, loop);
}

void RolodexPuzzle::stopAudio(uint index) {
	if (index >= ARRAYSIZE(_audioHandles))
		return;
	if (_engine->getMedia()->stopSoundEffect(_audioHandles[index])) {
		debugC(3, kDebugPuzzles,
			"Ripper: rolodex audio stopped index=%u frameSequence=%u",
			index, _activeSequence);
	}
}

void RolodexPuzzle::stopAllAudio() {
	for (uint i = 0; i < ARRAYSIZE(_audioHandles); ++i)
		stopAudio(i);
}

void RolodexPuzzle::serviceAudio(uint frame) {
	// ServiceRolodexMediaSequenceCallback at 0x27e38 starts the four
	// rolo%d.wav descriptors at these exact media-sequence frame counters.
	if (_activeSequence == 0) {
		if (frame == 1)
			startAudio(2);
		else if (frame == 12)
			startAudio(0, 20, true);
		return;
	}

	if (frame == 1)
		startAudio(3);
	else if (frame == 15)
		startAudio(1);
	else if (_activeSequence < ARRAYSIZE(kSequenceAudioStopFrames) &&
			frame == kSequenceAudioStopFrames[_activeSequence])
		stopAudio(3);
}

void RolodexPuzzle::updateCursor(const Common::Point &point) {
	uint cursor = kDefaultCursor;
	if (isInsideAdvanceControl(point))
		cursor = kAdvanceCursor;
	else if (isInsideExitControl(point))
		cursor = kExitCursor;
	_engine->getCursor()->update(cursor);
}

uint16 RolodexPuzzle::serviceInput() {
	while (_engine->getInput()->hasPendingKey()) {
		const uint16 command = _engine->getInput()->consumeKey();
		if (command == kHelpCommand) {
			debugC(1, kDebugPuzzles,
				"Ripper: rolodex puzzle opening modal help table=0x%x sequence=%u",
				kHelpSelectionTable, _activeSequence);
			if (!_engine->getModalDialog()->run(kHelpSelectionTable))
				warning("Ripper: rolodex puzzle modal help failed");
			updateCursor(_engine->getInput()->peekMouseState().position);
			continue;
		}
		if (command == kAdvanceCommand || command == kExitCommand)
			return command;
	}

	const MouseState mouse = _engine->getInput()->publishMouseState();
	updateCursor(mouse.position);
	if ((mouse.pressed & kMouseButtonLeft) == 0)
		return 0;
	if (isInsideAdvanceControl(mouse.position))
		return kAdvanceCommand;
	if (isInsideExitControl(mouse.position))
		return kExitCommand;
	return 0;
}

uint16 RolodexPuzzle::service(uint frame) {
	serviceAudio(frame);
	if (!_acceptInput)
		return 0;
	const uint16 command = serviceInput();
	if (command != 0) {
		debugC(2, kDebugPuzzles,
			"Ripper: rolodex input command=0x%04x sequence=%u frame=%u",
			command, _activeSequence, frame);
	}
	return command;
}

RolodexPuzzle::Result RolodexPuzzle::run(uint completionFlag) {
	debugC(1, kDebugPuzzles,
		"Ripper: entered rolodex puzzle completionFlag=%u helpTable=0x%x "
		"advanceRect=%d,%d,%d,%d exitRect=%d,%d,%d,%d",
		completionFlag, kHelpSelectionTable,
		kAdvanceLeft, kAdvanceTop, kAdvanceWidth, kAdvanceHeight,
		kExitLeft, kExitTop, kExitWidth, kExitHeight);
	_engine->getInput()->discardMouseTransitions();
	_engine->getCursor()->setVisible(false);

	_activeSequence = 0;
	_acceptInput = false;
	if (!_engine->getMedia()->playPuzzleSequence("rolodex8.smk", 0, this)) {
		stopAllAudio();
		return kLoadFailed;
	}
	_engine->getInput()->discardMouseTransitions();

	Result result = kExited;
	uint16 command = kAdvanceCommand;
	_acceptInput = true;
	while (command != kExitCommand && !_engine->shouldQuit()) {
		if (command != kAdvanceCommand)
			continue;
		stopAudio(3);
		++_activeSequence;
		if (_activeSequence > 6)
			_activeSequence = 1;
		if (_activeSequence == 6) {
			// RunRolodexSequencePuzzleScene at 0x28328 sets the caller's
			// named flag before starting the sixth sequence.
			if (!markSolved(completionFlag, "rolodex-puzzle")) {
				result = kLoadFailed;
				break;
			}
			result = kSolved;
			debugC(1, kDebugPuzzles,
				"Ripper: rolodex puzzle completed milestone=%u sequence=%u",
				completionFlag, _activeSequence);
		}

		const Common::String path = Common::String::format("rolodex%u.smk", _activeSequence);
		const uint loopStartFrame = _activeSequence == 6 ?
			kSuccessLoopStartFrame : kNormalLoopStartFrame;
		command = 0;
		if (!_engine->getMedia()->playPuzzleSequence(path, loopStartFrame, this, &command)) {
			result = kLoadFailed;
			break;
		}
	}

	_acceptInput = false;
	stopAudio(3);
	stopAudio(0);
	if (!_engine->shouldQuit() && result != kLoadFailed) {
		startAudio(2);
		if (!_engine->getMedia()->playPuzzleSequence("rolodex7.smk", 0, nullptr))
			result = kLoadFailed;
	}
	stopAllAudio();
	_engine->getCursor()->update(kDefaultCursor);
	debugC(1, kDebugPuzzles,
		"Ripper: exited rolodex puzzle result=%d milestone=%u activeSequence=%u",
		result, completionFlag, _activeSequence);
	return result;
}

} // End of namespace Ripper
