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

#include "graphics/surface.h"

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
static const uint kPuzzleHelpBorderWidth = 3;
static const uint kPuzzleHelpDitherMask = 3;
static const uint16 kCancelCommand = 0xfffe;
static const char *const kCursorLibraryName = "end_curs.pl";

struct ThrowTarget {
	uint firstFrame;
	uint lastFrame;
	int cursorYMin;
	int cursorXMin;
	int cursorYMax;
	int cursorXMax;
};

// RIPPER.LE table 0x84fe4 stores Y/X bounds at +0x08/+0x0c/+0x10/+0x14 in
// RIPMID.AVI's 320x200 space. Playback scales them 2:1.
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
// these ending indices. The names preserve RIPPER.LE's diagnostic strings.
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

static int findActiveThrowTarget(uint frame) {
	for (uint target = 0; target < kThrowTargetCount; ++target) {
		const ThrowTarget &region = kThrowTargets[target];
		if (frame >= region.firstFrame && frame <= region.lastFrame)
			return target;
	}
	return -1;
}

class EndingSelectionDebugHelper {
public:
	EndingSelectionDebugHelper(RipperEngine *engine, uint storyEnding) :
		_engine(engine), _storyEnding(storyEnding), _lastTarget(-1),
		_overlayPaletteIndex(0), _enabled(false), _overlayActive(false),
		_overlayCorrect(false), _warningIssued(false) {
	}

	bool update(bool selectionActive, uint frame) {
		const bool hadOverlay = _overlayActive;
		_overlayActive = false;
		const bool enabled = _engine->isPuzzleHelpEnabled();
		if (enabled != _enabled) {
			_enabled = enabled;
			_lastTarget = -1;
			debugC(2, kDebugPuzzles,
				"Ripper: ending-selection puzzle-help overlay enabled=%d expectedEnding=%u expected='%s' correctColor=green incorrectColor=red command=PUZZLE_HELP",
				_enabled, _storyEnding, kEndingNames[_storyEnding]);
		}
		if (!_enabled || !selectionActive) {
			_lastTarget = -1;
			return hadOverlay;
		}

		const int target = findActiveThrowTarget(frame);
		if (target < 0) {
			_lastTarget = -1;
			return hadOverlay;
		}

		const uint selectedEnding = target % 4;
		_overlayCorrect = selectedEnding == _storyEnding;
		if (!draw(kThrowTargets[target]))
			return hadOverlay;
		_overlayActive = true;
		if (target != _lastTarget) {
			const ThrowTarget &region = kThrowTargets[target];
			debugC(2, kDebugPuzzles,
				"Ripper: ending-selection puzzle-help target=%d candidateEnding=%u candidate='%s' expectedEnding=%u expected='%s' correct=%d color=%s paletteIndex=%u frames=%u..%u logical=%d,%d..%d,%d physical=%d,%d..%d,%d",
				target, selectedEnding, kEndingNames[selectedEnding],
				_storyEnding, kEndingNames[_storyEnding], _overlayCorrect,
				_overlayCorrect ? "green" : "red", _overlayPaletteIndex,
				region.firstFrame, region.lastFrame, region.cursorXMin + 1,
				region.cursorYMin + 1, region.cursorXMax - 1,
				region.cursorYMax - 1,
				(region.cursorXMin + 1) * kEndingSelectionDisplayScale,
				(region.cursorYMin + 1) * kEndingSelectionDisplayScale,
				region.cursorXMax * kEndingSelectionDisplayScale - 1,
				region.cursorYMax * kEndingSelectionDisplayScale - 1);
		}
		_lastTarget = target;
		return false;
	}

	bool managesPalette() const { return _overlayActive; }

	void transformPalette(byte *palette, uint colorCount) const {
		if (!_overlayActive || _overlayPaletteIndex >= colorCount)
			return;
		byte *color = palette + _overlayPaletteIndex * 3;
		if (_overlayCorrect) {
			color[0] = 0;
			color[1] = 255;
			color[2] = 0;
		} else {
			color[0] = 255;
			color[1] = 0;
			color[2] = 0;
		}
	}

private:
	bool draw(const ThrowTarget &region) {
		Graphics::Surface *screen = g_system->lockScreen();
		if (!screen || screen->format.bytesPerPixel != 1) {
			if (screen)
				g_system->unlockScreen();
			if (!_warningIssued) {
				warning("Ripper: ending-selection puzzle-help overlay requires the indexed display");
				_warningIssued = true;
			}
			return false;
		}

		uint32 paletteUse[256] = { 0 };
		for (int y = 0; y < screen->h; ++y) {
			const byte *row = (const byte *)screen->getBasePtr(0, y);
			for (int x = 0; x < screen->w; ++x)
				++paletteUse[row[x]];
		}
		_overlayPaletteIndex = 1;
		// Keep indices 0, 254, and 255 available to the engine's other
		// diagnostic overlays, which use those established interface colors.
		for (uint index = 2; index < 254; ++index) {
			if (paletteUse[index] < paletteUse[_overlayPaletteIndex])
				_overlayPaletteIndex = index;
		}

		// RIPPER.LE uses strict edge comparisons; exclude pixels no click can hit.
		const Common::Rect bounds(
			(region.cursorXMin + 1) * kEndingSelectionDisplayScale,
			(region.cursorYMin + 1) * kEndingSelectionDisplayScale,
			region.cursorXMax * kEndingSelectionDisplayScale,
			region.cursorYMax * kEndingSelectionDisplayScale);
		for (int y = bounds.top; y < bounds.bottom; ++y) {
			byte *row = (byte *)screen->getBasePtr(0, y);
			for (int x = bounds.left; x < bounds.right; ++x) {
				const bool border =
					x - bounds.left < (int)kPuzzleHelpBorderWidth ||
					bounds.right - x <= (int)kPuzzleHelpBorderWidth ||
					y - bounds.top < (int)kPuzzleHelpBorderWidth ||
					bounds.bottom - y <= (int)kPuzzleHelpBorderWidth;
				if (border || ((x + y) & kPuzzleHelpDitherMask) == 0)
					row[x] = _overlayPaletteIndex;
			}
		}
		g_system->unlockScreen();
		return true;
	}

	RipperEngine *_engine;
	uint _storyEnding;
	int _lastTarget;
	byte _overlayPaletteIndex;
	bool _enabled;
	bool _overlayActive;
	bool _overlayCorrect;
	bool _warningIssued;
};

class EndingSelectionCallback : public MediaSequenceCallback {
public:
	EndingSelectionCallback(RipperEngine *engine, Audio::SoundHandle &spinHandle,
			Audio::SoundHandle &throwHandle, uint storyEnding) :
		_engine(engine), _spinHandle(spinHandle), _throwHandle(throwHandle),
		_debugHelper(engine, storyEnding),
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
			// Sequence 3 starts at logical (100,160) under the 2:1 descriptor
			// installed by RIPPER.LE at 0x43840.
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
		const bool refreshPalette =
			_debugHelper.update(_selectionActive, frame);

		// HandleEndingSelectionThrowTargetCallback tests the published current
		// button flags, so a held primary button remains eligible until a target
		// window accepts it.
		if ((mouse.buttons & kMouseButtonLeft) == 0) {
			_reportedHeldMiss = false;
			return refreshPalette ? kContinueRefreshPalette : 0;
		}

		const Common::Point logicalPoint(
			mouse.position.x / kEndingSelectionDisplayScale,
			mouse.position.y / kEndingSelectionDisplayScale);
		const int activeTarget = findActiveThrowTarget(frame);
		if (activeTarget >= 0) {
			const ThrowTarget &region = kThrowTargets[activeTarget];
			const bool inside = logicalPoint.x > region.cursorXMin &&
				logicalPoint.x < region.cursorXMax &&
				logicalPoint.y > region.cursorYMin &&
				logicalPoint.y < region.cursorYMax;
			if (inside) {
				const uint selectedEnding = activeTarget % 4;
				const bool correct = selectedEnding == _storyEnding;
				_engine->getMedia()->playSoundEffect(
					"ballthro.wav", _throwHandle, 100, false);
				debugC(1, kDebugScene,
					"Ripper: ending selection queued throw command=0x%04x target=%u candidateEnding=%u candidate='%s' expectedEnding=%u expected='%s' correct=%d sound='ballthro.wav' initialMedia='%s' followupRoute='%s' sequence=%u frame=%u point=%d,%d logical=%d,%d",
					activeTarget + 1, (uint)activeTarget, selectedEnding,
					kEndingNames[selectedEnding], _storyEnding,
					kEndingNames[_storyEnding], correct,
					kChosenEndingMedia[selectedEnding], correct ?
						kCorrectOutcomeRoutes[selectedEnding] : "ripfinal.avi",
					_sequenceId, frame, mouse.position.x, mouse.position.y,
					logicalPoint.x, logicalPoint.y);
				return activeTarget + 1;
			}
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
					logicalPoint.y, region.cursorXMin, region.cursorYMin,
					region.cursorXMax, region.cursorYMax);
			} else {
				debugC(1, kDebugScene,
					"Ripper: ending selection rejected press reason='no active target at frame' queuedCommand=0x0000 expectedEnding=%u expected='%s' sequence=%u frame=%u point=%d,%d logical=%d,%d",
					_storyEnding, kEndingNames[_storyEnding], _sequenceId,
					frame, mouse.position.x, mouse.position.y,
					logicalPoint.x, logicalPoint.y);
			}
			_reportedHeldMiss = true;
		}
		return refreshPalette ? kContinueRefreshPalette : 0;
	}

	bool ownsInput() const override { return true; }
	bool keepsCursorVisible() const override { return _selectionActive; }
	bool managesPalette() const override { return _debugHelper.managesPalette(); }
	void transformPalette(byte *palette, uint colorCount) const override {
		_debugHelper.transformPalette(palette, colorCount);
	}

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
	EndingSelectionDebugHelper _debugHelper;
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
