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
	"end_df.avi", "end_dc.avi", "end_dm.avi", "end_him.avi"
};

static const char *const kCorrectEpilogueMedia[4] = {
	"q4_v3.avi", "q4_v6.avi", "q4_v5.avi", "ripfinal.avi"
};

class EndingSelectionCallback : public MediaSequenceCallback {
public:
	EndingSelectionCallback(RipperEngine *engine, Audio::SoundHandle &spinHandle,
			Audio::SoundHandle &throwHandle) :
		_engine(engine), _spinHandle(spinHandle), _throwHandle(throwHandle),
		_spinStarted(false), _cursorFrame(0), _nextCursorFrameMillis(0),
		_cursorActive(false) {
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

	uint16 service(uint frame) override {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			return kCancelCommand;
		}
		while (_engine->getInput()->hasPendingKey()) {
			if (_engine->getInput()->consumeKey() == 0x1b) {
				debugC(1, kDebugScene,
					"Ripper: ending selection cancelled by Escape frame=%u",
					frame);
				return kCancelCommand;
			}
		}

		if (!_spinStarted && frame >= 3) {
			_spinStarted = _engine->getMedia()->playSoundEffect(
				"spin_1.wav", _spinHandle, 100, true);
			debugC(_spinStarted ? 2 : 1, kDebugScene,
				"Ripper: ending-selection spin audio started frame=%u success=%d",
				frame, _spinStarted);
		}

		serviceCursor();

		const MouseState mouse = _engine->getInput()->publishMouseState();
		if (frame < 3 || (mouse.pressed & kMouseButtonLeft) == 0)
			return 0;

		const Common::Point logicalPoint(mouse.position.x / 2,
			mouse.position.y / 2);
		for (uint target = 0; target < kThrowTargetCount; ++target) {
			const ThrowTarget &region = kThrowTargets[target];
			if (frame < region.firstFrame || frame > region.lastFrame ||
					logicalPoint.x <= region.left ||
					logicalPoint.x >= region.right ||
					logicalPoint.y <= region.top ||
					logicalPoint.y >= region.bottom)
				continue;
			_engine->getMedia()->playSoundEffect(
				"ballthro.wav", _throwHandle, 100, false);
			debugC(1, kDebugScene,
				"Ripper: ending selection hit target=%u frame=%u point=%d,%d logical=%d,%d result=%u",
				target, frame, mouse.position.x, mouse.position.y,
				logicalPoint.x, logicalPoint.y, target + 1);
			return target + 1;
		}
		debugC(3, kDebugScene,
			"Ripper: ending selection missed frame=%u point=%d,%d logical=%d,%d",
			frame, mouse.position.x, mouse.position.y,
			logicalPoint.x, logicalPoint.y);
		return 0;
	}

	bool ownsInput() const override { return true; }
	bool keepsCursorVisible() const override { return true; }

private:
	void applyCursorFrame() {
		const BitmapAssetFrame &cursor = _cursorFrames[_cursorFrame];
		if (!cursor.pixels.empty()) {
			_engine->getCursor()->applyCustomCursor(cursor,
				cursor.width / 2, cursor.height / 2, 2);
		}
	}

	void serviceCursor() {
		if (!_cursorActive)
			activateCursor();
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
	EndingSelectionCallback callback(_engine, spinHandle, throwHandle);
	if (!callback.loadCursors())
		return false;

	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	callback.activateCursor();
	const bool windStarted = _engine->getMedia()->playSoundEffect(
		"windfin.wav", windHandle, 100, true);
	uint16 command = 0;
	debugC(1, kDebugScene,
		"Ripper: entered ending selection function=RunEndingSelectionEpiloguesAndCredits@0x43adb storyEnding=%d wind=%d",
		storyEnding, windStarted);
	bool result = _engine->getMedia()->playInteractiveIavf(
		"ripmid.avi", &callback, &command);

	if (command < 1 || command > kThrowTargetCount) {
		_engine->getMedia()->stopSoundEffect(windHandle);
		_engine->getMedia()->stopSoundEffect(spinHandle);
		if (!_engine->shouldQuit())
			result = _engine->getMedia()->play("end_db.avi", true) && result;
		debugC(1, kDebugScene,
			"Ripper: ending selection produced no target command=0x%04x",
			command);
	} else {
		const uint selectedEnding = (command - 1) % 4;
		result = _engine->getMedia()->play(
			kChosenEndingMedia[selectedEnding], true) && result;
		_engine->getMedia()->stopSoundEffect(spinHandle);
		if (selectedEnding == (uint)storyEnding) {
			if (storyEnding != 1) {
				result = _engine->getMedia()->play(
					storyEnding == 3 ? "end_her.avi" : "quin_win.avi",
					true) && result;
			}
			_engine->getMedia()->stopSoundEffect(windHandle);
			result = _engine->getMedia()->play("q4_v4.avi", true) && result;
			result = _engine->getMedia()->play(
				kCorrectEpilogueMedia[selectedEnding], true) && result;
		} else {
			_engine->getMedia()->stopSoundEffect(windHandle);
			result = _engine->getMedia()->play("ripfinal.avi", true) && result;
		}
		debugC(1, kDebugScene,
			"Ripper: ending selection completed target=%u selectedEnding=%u storyEnding=%d correct=%d",
			command - 1, selectedEnding, storyEnding,
			selectedEnding == (uint)storyEnding);
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
