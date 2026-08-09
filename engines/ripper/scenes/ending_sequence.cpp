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

#include "ripper/scenes/ending_sequence.h"

#include "common/debug.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/system.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/diagnostics/screen_presenter.h"
#include "ripper/input.h"
#include "ripper/media.h"
#include "ripper/milestones.h"
#include "ripper/resources.h"
#include "ripper/ripper.h"

namespace Ripper {

namespace {

static const uint kEndingCursorCount = 20;
static const uint32 kEndingCursorFrameDurationMillis = 100;
static const uint kEndingSelectionSequenceId = 3;
static const int kEndingSelectionCursorX = 100;
static const int kEndingSelectionCursorY = 160;
static const int kEndingSelectionDisplayScale = 2;
static const uint kThrowTargetCount = 8;
static const uint16 kCancelCommand = 0xfffe;
static const char *const kCursorLibraryName = "end_curs.pl";

struct ThrowTarget {
	uint firstFrame;
	uint lastFrame;
	int left;
	int top;
	int right;
	int bottom;
};

// g_endingSelectionThrowTargetRegions at 0x84fe4 stores frame-gated
// coordinates in RIPMID.AVI's logical 320x200 space. The packetized movie is
// presented at 2:1 in the retail 640x400 display mode.
static const ThrowTarget kThrowTargets[kThrowTargetCount] = {
	{ 42, 79, 53, 139, 136, 179 },
	{ 84, 122, 53, 124, 136, 173 },
	{ 126, 186, 40, 140, 142, 186 },
	{ 189, 227, 45, 132, 142, 186 },
	{ 231, 287, 57, 147, 128, 176 },
	{ 294, 351, 54, 126, 138, 170 },
	{ 357, 416, 46, 144, 132, 188 },
	{ 420, 491, 47, 133, 139, 188 }
};

static const char *const kChosenEndingMedia[4] = {
	"end_db.avi", "end_df.avi", "end_dc.avi", "end_dm.avi"
};

static const char *const kCorrectEpilogueMedia[4] = {
	"q4_v4.avi", "q4_v3.avi", "q4_v6.avi", "q4_v5.avi"
};

// RunEndingSelectionEpiloguesAndCredits at 0x43adb maps the story flags to
// these ending indices. The names preserve the retail diagnostic strings.
static const char *const kEndingNames[4] = {
	"Burton", "Eddie", "Powel", "Magnotta"
};

static const uint kEndingMilestoneFlags[4] = { 8, 7, 9, 6 };

static const char *const kCorrectOutcomeRoutes[4] = {
	"end_her.avi -> quin_win.avi -> q4_v4.avi",
	"quin_win.avi -> q4_v3.avi",
	"end_her.avi -> quin_win.avi -> q4_v6.avi",
	"end_him.avi -> quin_win.avi -> q4_v5.avi"
};

class EndingSelectionCallback : public MediaSequenceCallback {
public:
	EndingSelectionCallback(RipperEngine *engine, Audio::SoundHandle &spinHandle,
			Audio::SoundHandle &throwHandle, uint storyEnding) :
		_engine(engine), _spinHandle(spinHandle), _throwHandle(throwHandle),
		_spinStarted(false), _cursorFrame(0), _nextCursorFrameMillis(0),
		_cursorActive(false), _selectionActive(false), _reportedHeldMiss(false),
		_sequenceId(0), _storyEnding(storyEnding) {
	}

	bool loadCursors() {
		Common::ScopedPtr<Common::SeekableReadStream> stream(
			_engine->getResources()->createReadStreamForPath(kCursorLibraryName));
		if (!stream || !_cursorLibrary.open(*stream,
				Common::Path(kCursorLibraryName))) {
			warning("Ripper: could not open ending cursor library '%s'",
				kCursorLibraryName);
			return false;
		}
		for (uint frame = 0; frame < kEndingCursorCount; ++frame) {
			Common::ScopedPtr<Common::SeekableReadStream> frameStream(
				_cursorLibrary.createReadStreamForMember(
					Common::String::format("%u", frame + 1)));
			if (!frameStream || !decodeBitmapAsset(*frameStream,
					_cursorFrames[frame])) {
				warning("Ripper: could not decode ending cursor frame=%u",
					frame + 1);
				return false;
			}
		}
		debugC(1, kDebugScene,
			"Ripper: loaded ending-selection cursors library='%s' frames=%u size=%ux%u",
			kCursorLibraryName, kEndingCursorCount,
			_cursorFrames[0].width, _cursorFrames[0].height);
		return true;
	}

	void beginIavfSegment(uint sequenceId) override {
		_sequenceId = sequenceId;
		_selectionActive = sequenceId == kEndingSelectionSequenceId;
		if (_selectionActive) {
			// InitializeEndingSelectionPlaybackCallback at 0x43840 installs
			// the 2:1 display descriptor and moves the retail selection point
			// to logical (100,160) as branch sequence 3 begins.
			_engine->getInput()->warpMousePosition(Common::Point(
				kEndingSelectionCursorX * kEndingSelectionDisplayScale,
				kEndingSelectionCursorY * kEndingSelectionDisplayScale));
			activateCursor();
			debugC(1, kDebugScene,
				"Ripper: ending selection armed expectedEnding=%u expected='%s' milestone=%u cursorLogical=%d,%d cursorPhysical=%d,%d",
				_storyEnding, kEndingNames[_storyEnding],
				kEndingMilestoneFlags[_storyEnding], kEndingSelectionCursorX,
				kEndingSelectionCursorY,
				kEndingSelectionCursorX * kEndingSelectionDisplayScale,
				kEndingSelectionCursorY * kEndingSelectionDisplayScale);
		}
		debugC(2, kDebugScene,
			"Ripper: ending-selection packetized branch sequence=%u active=%d cursor=%d position=%d,%d",
			_sequenceId, _selectionActive, _cursorActive,
			_engine->getInput()->peekMouseState().position.x,
			_engine->getInput()->peekMouseState().position.y);
	}

	uint16 service(uint frame) override {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			return kCancelCommand;
		}
		while (_engine->getInput()->hasPendingKey()) {
			if (_engine->getInput()->consumeKey() == 0x1b) {
				debugC(1, kDebugScene,
					"Ripper: ending selection cancelled by Escape sequence=%u frame=%u",
					_sequenceId, frame);
				return kCancelCommand;
			}
		}

		// HandleEndingSelectionThrowTargetCallback at 0x438cf gates spin audio
		// and input on packetized playback state 3, not Smacker frame 3.
		const MouseState mouse = _engine->getInput()->publishMouseState();
		if (!_selectionActive)
			return 0;
		if (!_spinStarted) {
			_spinStarted = _engine->getMedia()->playSoundEffect(
				"spin_1.wav", _spinHandle, 100, true);
			debugC(_spinStarted ? 2 : 1, kDebugScene,
				"Ripper: ending-selection spin audio started sequence=%u frame=%u success=%d",
				_sequenceId, frame, _spinStarted);
		}

		serviceCursor();

		// HandleEndingSelectionThrowTargetCallback tests the published current
		// button flags, so a held primary button remains eligible until a target
		// window accepts it.
		if ((mouse.buttons & kMouseButtonLeft) == 0) {
			_reportedHeldMiss = false;
			return 0;
		}

		const Common::Point logicalPoint(
			mouse.position.x / kEndingSelectionDisplayScale,
			mouse.position.y / kEndingSelectionDisplayScale);
		int activeTarget = -1;
		for (uint target = 0; target < kThrowTargetCount; ++target) {
			const ThrowTarget &region = kThrowTargets[target];
			if (frame < region.firstFrame || frame > region.lastFrame)
				continue;
			activeTarget = target;
			if (logicalPoint.x <= region.left ||
					logicalPoint.x >= region.right ||
					logicalPoint.y <= region.top ||
					logicalPoint.y >= region.bottom)
				break;

			const uint selectedEnding = target % 4;
			const bool correct = selectedEnding == _storyEnding;
			_engine->getMedia()->playSoundEffect(
				"ballthro.wav", _throwHandle, 100, false);
			debugC(1, kDebugScene,
				"Ripper: ending selection queued throw command=0x%04x target=%u candidateEnding=%u candidate='%s' expectedEnding=%u expected='%s' correct=%d sound='ballthro.wav' initialMedia='%s' followupRoute='%s' sequence=%u frame=%u point=%d,%d logical=%d,%d",
				target + 1, target, selectedEnding,
				kEndingNames[selectedEnding], _storyEnding,
				kEndingNames[_storyEnding], correct,
				kChosenEndingMedia[selectedEnding],
				correct ? kCorrectOutcomeRoutes[selectedEnding] : "ripfinal.avi",
				_sequenceId, frame, mouse.position.x, mouse.position.y,
				logicalPoint.x, logicalPoint.y);
			return target + 1;
		}

		if (!_reportedHeldMiss) {
			if (activeTarget >= 0) {
				const ThrowTarget &region = kThrowTargets[activeTarget];
				const uint selectedEnding = activeTarget % 4;
				debugC(1, kDebugScene,
					"Ripper: ending selection rejected press reason='outside active target' queuedCommand=0x0000 activeTarget=%d candidateEnding=%u candidate='%s' expectedEnding=%u expected='%s' wouldBeCorrect=%d sequence=%u frame=%u point=%d,%d logical=%d,%d targetLogical=%d,%d..%d,%d exclusive=1",
					activeTarget, selectedEnding, kEndingNames[selectedEnding],
					_storyEnding, kEndingNames[_storyEnding],
					selectedEnding == _storyEnding, _sequenceId, frame,
					mouse.position.x, mouse.position.y, logicalPoint.x,
					logicalPoint.y, region.left, region.top, region.right,
					region.bottom);
			} else {
				debugC(1, kDebugScene,
					"Ripper: ending selection rejected press reason='no active target at frame' queuedCommand=0x0000 expectedEnding=%u expected='%s' sequence=%u frame=%u point=%d,%d logical=%d,%d",
					_storyEnding, kEndingNames[_storyEnding], _sequenceId,
					frame, mouse.position.x, mouse.position.y,
					logicalPoint.x, logicalPoint.y);
			}
			_reportedHeldMiss = true;
		}
		return 0;
	}

	bool ownsInput() const override { return true; }
	bool keepsCursorVisible() const override { return _selectionActive; }

private:
	void activateCursor() {
		if (_cursorActive)
			return;
		_cursorActive = true;
		_cursorFrame = 0;
		applyCursorFrame();
		_nextCursorFrameMillis =
			g_system->getMillis(true) + kEndingCursorFrameDurationMillis;
		_engine->getCursor()->setVisible(true);
		debugC(2, kDebugCursor,
			"Ripper: activated ending-selection cursor frames=%u durationMs=%u",
			kEndingCursorCount, kEndingCursorFrameDurationMillis);
	}

	void applyCursorFrame() {
		const BitmapAssetFrame &cursor = _cursorFrames[_cursorFrame];
		if (!cursor.pixels.empty()) {
			_engine->getCursor()->applyCustomCursor(cursor,
				cursor.width / 2, cursor.height / 2,
				kEndingSelectionDisplayScale);
		}
	}

	void serviceCursor() {
		const uint32 now = g_system->getMillis(true);
		if (now >= _nextCursorFrameMillis) {
			const uint elapsedFrames = 1 +
				(now - _nextCursorFrameMillis) /
					kEndingCursorFrameDurationMillis;
			_cursorFrame = (_cursorFrame + elapsedFrames) % kEndingCursorCount;
			_nextCursorFrameMillis +=
				elapsedFrames * kEndingCursorFrameDurationMillis;
			applyCursorFrame();
		}
		_engine->getCursor()->setVisible(true);
	}

	RipperEngine *_engine;
	Audio::SoundHandle &_spinHandle;
	Audio::SoundHandle &_throwHandle;
	AssetLibrary _cursorLibrary;
	BitmapAssetFrame _cursorFrames[kEndingCursorCount];
	bool _spinStarted;
	uint _cursorFrame;
	uint32 _nextCursorFrameMillis;
	bool _cursorActive;
	bool _selectionActive;
	bool _reportedHeldMiss;
	uint _sequenceId;
	uint _storyEnding;
};

} // End of anonymous namespace

EndingSequence::EndingSequence(RipperEngine *engine) : _engine(engine) {
}

bool EndingSequence::run() {
	// RunEndingSelectionEpiloguesAndCredits at 0x43adb resolves the selected
	// Ripper from flags 6..9 before installing the END_CURS.PL cursor set and
	// playing RIPMID.AVI through HandleEndingSelectionThrowTargetCallback.
	int storyEnding = -1;
	if (_engine->getMilestones()->isSet(6))
		storyEnding = 3;
	else if (_engine->getMilestones()->isSet(7))
		storyEnding = 1;
	else if (_engine->getMilestones()->isSet(8))
		storyEnding = 0;
	else if (_engine->getMilestones()->isSet(9))
		storyEnding = 2;
	if (storyEnding < 0) {
		warning("Ripper: ending sequence has no selected Ripper flag 6..9");
		return false;
	}

	Audio::SoundHandle windHandle;
	Audio::SoundHandle spinHandle;
	Audio::SoundHandle throwHandle;
	EndingSelectionCallback callback(
		_engine, spinHandle, throwHandle, storyEnding);
	if (!callback.loadCursors())
		return false;

	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	const bool windStarted = _engine->getMedia()->playSoundEffect(
		"windfin.wav", windHandle, 100, true);
	uint16 command = 0;
	debugC(1, kDebugScene,
		"Ripper: entered ending selection function=RunEndingSelectionEpiloguesAndCredits@0x43adb storyEnding=%d expected='%s' milestone=%u wind=%d",
		storyEnding, kEndingNames[storyEnding],
		kEndingMilestoneFlags[storyEnding], windStarted);
	bool result = _engine->getMedia()->playInteractiveIavf(
		"ripmid.avi", &callback, &command);

	if (command < 1 || command > kThrowTargetCount) {
		_engine->getMedia()->stopSoundEffect(windHandle);
		_engine->getMedia()->stopSoundEffect(spinHandle);
		if (!_engine->shouldQuit())
			result = _engine->getMedia()->play("ripfinal.avi", true) && result;
		debugC(1, kDebugScene,
			"Ripper: ending selection resolved action='no throw queued' command=0x%04x expectedEnding=%d expected='%s' nextMedia='ripfinal.avi'",
			command, storyEnding, kEndingNames[storyEnding]);
	} else {
		const uint selectedEnding = (command - 1) % 4;
		const bool correct = selectedEnding == (uint)storyEnding;
		debugC(1, kDebugScene,
			"Ripper: ending selection resolving queued throw command=0x%04x target=%u selectedEnding=%u selected='%s' expectedEnding=%d expected='%s' correct=%d initialMedia='%s' followupRoute='%s'",
			command, command - 1, selectedEnding,
			kEndingNames[selectedEnding], storyEnding,
			kEndingNames[storyEnding], correct,
			kChosenEndingMedia[selectedEnding],
			correct ? kCorrectOutcomeRoutes[selectedEnding] : "ripfinal.avi");
		result = _engine->getMedia()->play(
			kChosenEndingMedia[selectedEnding], true) && result;
		_engine->getMedia()->stopSoundEffect(spinHandle);
		if (correct) {
			if (storyEnding != 1) {
				result = _engine->getMedia()->play(
					storyEnding == 3 ? "end_him.avi" : "end_her.avi",
					true) && result;
			}
			_engine->getMedia()->stopSoundEffect(windHandle);
			result = _engine->getMedia()->play("quin_win.avi", true) && result;
			result = _engine->getMedia()->play(
				kCorrectEpilogueMedia[selectedEnding], true) && result;
		} else {
			_engine->getMedia()->stopSoundEffect(windHandle);
			result = _engine->getMedia()->play("ripfinal.avi", true) && result;
		}
		debugC(1, kDebugScene,
			"Ripper: ending selection completed command=0x%04x target=%u selectedEnding=%u selected='%s' storyEnding=%d expected='%s' correct=%d",
			command, command - 1, selectedEnding,
			kEndingNames[selectedEnding], storyEnding,
			kEndingNames[storyEnding], correct);
	}

	_engine->getMedia()->stopSoundEffect(windHandle);
	_engine->getMedia()->stopSoundEffect(spinHandle);
	_engine->getMedia()->stopSoundEffect(throwHandle);
	_engine->getCursor()->setSelectionIndex(0);
	_engine->getCursor()->dispatchSelectionIndexChange(0);
	_engine->getCursor()->setVisible(false);
	if (!_engine->shouldQuit()) {
		result = _engine->getMedia()->play("credits1.avi", true) && result;
		g_system->fillScreen(0);
		result = _engine->getMedia()->play("credits2.avi", true) && result;
		g_system->fillScreen(0);
		presentScreen();
	}
	debugC(result ? 1 : 2, kDebugScene,
		"Ripper: left ending sequence result=%d command=0x%04x quit=%d",
		result, command, _engine->shouldQuit());
	return result;
}

} // End of namespace Ripper
