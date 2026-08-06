/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of their respective copyright holders.
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

#include "ripper/puzzles/circuit_chip.h"

#include "common/debug.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/system.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/media.h"
#include "ripper/milestones.h"
#include "ripper/modal_dialog.h"
#include "ripper/ripper.h"
#include "ripper/toolbar.h"

namespace Ripper {

namespace {

static const uint kChipCount = 16;
static const uint kGroupCount = 4;
static const uint kSlotsPerGroup = 4;
static const uint kDefaultCursor = 14;
static const uint kSelectionCursor = 16;
static const uint kHelpSelectionTable = 0x1ae;
static const uint16 kEscapeCommand = 0x1b;
static const uint16 kHelpCommand = 0x3b00;
static const uint kDosTickMillis = 55;
static const uint kMeterTickCount = 2;
static const int kSceneOriginY = 50;
static const int kMeterX = 27;
static const int kMeterY = 196;
static const int kMeterWidth = 48;
static const int kMeterHeight = 80;
static const uint kCircuitManualFlag = 0x55;
static const uint kCircuitManualResource = 0xb6;
static const char *const kCompletionKeyword = "theory";

// RunCircuitChipPlacementPuzzleScene at 0x28aa4 reads both tables as
// scene-space Y followed by physical X. The scene is a 640-by-400
// presentation, so no 50-pixel scene origin is added here.
static const Common::Point kSourcePositions[kChipCount] = {
	Common::Point(460, 22),
	Common::Point(460, 322),
	Common::Point(513, 13),
	Common::Point(513, 62),
	Common::Point(506, 112),
	Common::Point(498, 163),
	Common::Point(506, 219),
	Common::Point(512, 277),
	Common::Point(513, 330),
	Common::Point(568, 20),
	Common::Point(567, 70),
	Common::Point(568, 121),
	Common::Point(567, 172),
	Common::Point(567, 224),
	Common::Point(567, 276),
	Common::Point(565, 330)
};

static const Common::Point kTargetPositions[kChipCount] = {
	Common::Point(193, 14),
	Common::Point(281, 13),
	Common::Point(61, 74),
	Common::Point(139, 75),
	Common::Point(216, 76),
	Common::Point(290, 76),
	Common::Point(364, 77),
	Common::Point(88, 145),
	Common::Point(166, 146),
	Common::Point(242, 147),
	Common::Point(315, 147),
	Common::Point(389, 148),
	Common::Point(232, 214),
	Common::Point(427, 212),
	Common::Point(131, 266),
	Common::Point(324, 269)
};

// g_circuitPuzzleChipConnectorTypes at 0x8437e and
// g_circuitPuzzleSlotConnectorTypes at 0x8439e.
static const uint kChipConnectorTypes[kChipCount] = {
	12, 16, 9, 6, 12, 6, 9, 9, 6, 6, 12, 12, 16, 16, 16, 9
};

static const uint kSlotConnectorTypes[kChipCount] = {
	16, 6, 12, 6, 16, 9, 12, 12, 9, 12, 6, 16, 16, 9, 9, 6
};

// g_circuitPuzzleExpectedChipBySlot at 0x25db1 stores one-based chip
// numbers. This normalized table is zero based.
static const uint kExpectedChipBySlot[kChipCount] = {
	13, 3, 10, 5, 14, 15, 4, 0, 6, 11, 8, 1, 12, 7, 2, 9
};

// g_circuitPuzzleMeterSlotGroups at 0x25dd1.
static const uint kMeterSlotGroups[kChipCount] = {
	0, 14, 7, 15,
	1, 11, 8, 2,
	3, 6, 13, 12,
	4, 9, 5, 10
};

static const uint kProgressStartFrames[kGroupCount] = {1, 17, 32, 45};
static const uint kProgressFrameLimits[kGroupCount] = {16, 31, 44, 59};
static const uint kRegressStartFrames[kGroupCount] = {45, 30, 17, 1};
static const uint kRegressFrameLimits[kGroupCount] = {59, 44, 29, 16};

static const char *const kAudioNames[6] = {
	"circ0.wav", "circ1.wav", "circ2.wav",
	"circ3.wav", "circ4.wav", "circ5.wav"
};

static Common::Rect circuitManualBounds() {
	// RunCircuitChipPlacementPuzzleScene loads X=0x72, Y=0x15e,
	// width=0x113, and height=0x32 for resource 0xb6.
	return Common::Rect(114, 350, 389, 400);
}

static Common::Rect circuitManualMediaBounds() {
	// The RunMediaSequence call at 0x28ed2 presents the 320-by-76
	// ED_WAC.SMK patch at physical X=0x5c, Y=0x144.
	return Common::Rect(92, 324, 412, 400);
}

} // End of anonymous namespace

CircuitChipPuzzle::CircuitChipPuzzle(RipperEngine *engine) :
		_engine(engine), _draggedChip(-1), _hoveredControl(-1),
		_correctGroupCount(0), _idleMeterFrame(0), _keywordProgress(0),
		_lastMeterTick(0), _manualFirstVisible(0),
		_manualMaximumFirstVisible(0), _manualVisibleRows(0),
		_manualHoveredControl(ModalDialogManager::kTextPanelScrollNone),
		_manualAvailable(false) {
	for (uint slot = 0; slot < kChipCount; ++slot) {
		_sourceChips[slot] = slot;
		_targetChips[slot] = -1;
	}
}

bool CircuitChipPuzzle::loadPcx(const Common::String &path,
		BitmapAssetFrame &frame) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_engine->getResources()->createReadStreamForPath(path));
	if (!stream || !decodePcxAsset(*stream, frame)) {
		warning("Ripper: could not decode circuit puzzle PCX '%s'",
			path.c_str());
		return false;
	}
	return true;
}

bool CircuitChipPuzzle::loadAssets() {
	if (!loadPcx("empty.pcx", _background) ||
			_background.width != kRipperScreenWidth ||
			_background.height != kRipperScreenHeight)
		return false;

	_chips.clear();
	for (uint chip = 0; chip < kChipCount; ++chip) {
		BitmapAssetSequence sequence;
		const Common::String path =
			Common::String::format("chip%u.bbm", chip + 1);
		if (!_engine->getResources()->loadBitmapSequence(path, sequence) ||
				sequence.frames.empty()) {
			warning("Ripper: could not load circuit puzzle chip '%s'",
				path.c_str());
			return false;
		}
		_chips.push_back(Common::move(sequence.frames.front()));
	}

	BitmapAssetSequence meterSequence;
	if (!_engine->getResources()->loadBitmapSequence(
			"meter.pl", meterSequence) ||
			meterSequence.frames.empty())
		return false;
	_idleMeterFrames = Common::move(meterSequence.frames);

	for (uint cue = 0; cue < ARRAYSIZE(kAudioNames); ++cue) {
		Common::ScopedPtr<Common::SeekableReadStream> stream(
			_engine->getResources()->createReadStreamForPath(
				kAudioNames[cue]));
		if (!stream) {
			warning("Ripper: circuit puzzle audio '%s' is missing",
				kAudioNames[cue]);
			return false;
		}
	}

	debugC(1, kDebugPuzzles,
		"Ripper: loaded circuit chip puzzle assets chips=%u "
		"idleMeterFrames=%u background=%ux%u "
		"progress='meter2.smk' regress='meter3.smk' audio=6",
		_chips.size(), _idleMeterFrames.size(),
		_background.width, _background.height);
	return true;
}

bool CircuitChipPuzzle::captureManualBacking() {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1 ||
			screen->w != kRipperScreenWidth ||
			screen->h != kRipperScreenHeight ||
			_background.width != kRipperScreenWidth ||
			_background.height != kRipperScreenHeight) {
		if (screen)
			g_system->unlockScreen();
		return false;
	}

	const Common::Rect bounds = circuitManualMediaBounds();
	for (int y = bounds.top; y < bounds.bottom; ++y)
		memcpy(_background.pixels.data() + y * _background.width +
				bounds.left,
			screen->getBasePtr(bounds.left, y), bounds.width());
	g_system->unlockScreen();

	_background.palette.resize(256 * 3);
	g_system->getPaletteManager()->grabPalette(
		_background.palette.data(), 0, 256);
	debugC(2, kDebugPuzzles,
		"Ripper: captured circuit puzzle manual backing "
		"bounds=%d,%d,%dx%d paletteEntries=256",
		bounds.left, bounds.top, bounds.width(), bounds.height());
	return true;
}

void CircuitChipPuzzle::drawFrame(byte *screen, uint pitch,
		const BitmapAssetFrame &frame, int x, int y) const {
	for (uint sourceY = 0; sourceY < frame.height; ++sourceY) {
		const int destinationY = y + sourceY;
		if (destinationY < 0 || destinationY >= kRipperScreenHeight)
			continue;
		for (uint sourceX = 0; sourceX < frame.width; ++sourceX) {
			const int destinationX = x + sourceX;
			if (destinationX < 0 || destinationX >= kRipperScreenWidth)
				continue;
			const byte pixel = frame.pixels[sourceY * frame.width + sourceX];
			if (pixel != frame.transparentColor)
				screen[destinationY * pitch + destinationX] = pixel;
		}
	}
}

void CircuitChipPuzzle::render() {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}

	for (int y = 0; y < screen->h; ++y)
		memcpy(screen->getBasePtr(0, y),
			_background.pixels.data() + y * _background.width,
			MIN<int>(screen->w, _background.width));
	byte *pixels = (byte *)screen->getPixels();

	if (_meterPixels.size() == kMeterWidth * kMeterHeight) {
		for (int y = 0; y < kMeterHeight; ++y)
			memcpy(screen->getBasePtr(kMeterX, kMeterY + y),
				_meterPixels.data() + y * kMeterWidth, kMeterWidth);
	}
	if (!_idleMeterFrames.empty())
		drawFrame(pixels, screen->pitch,
			_idleMeterFrames[_idleMeterFrame], kMeterX, kMeterY);

	for (uint slot = 0; slot < kChipCount; ++slot) {
		if (_sourceChips[slot] >= 0)
			drawFrame(pixels, screen->pitch, _chips[_sourceChips[slot]],
				kSourcePositions[slot].x, kSourcePositions[slot].y);
		if (_targetChips[slot] >= 0)
			drawFrame(pixels, screen->pitch, _chips[_targetChips[slot]],
				kTargetPositions[slot].x, kTargetPositions[slot].y);
	}
	if (_draggedChip >= 0) {
		const BitmapAssetFrame &chip = _chips[_draggedChip];
		drawFrame(pixels, screen->pitch, chip,
			_dragPoint.x - chip.width / 2,
			_dragPoint.y - chip.height / 2);
	}
	g_system->unlockScreen();

	if (_background.palette.size() == 256 * 3)
		g_system->getPaletteManager()->setPalette(
			_background.palette.data(), 0, 256);
	if (_manualAvailable &&
			!_engine->getModalDialog()->drawRetainedTextPanel(
				kCircuitManualResource, circuitManualBounds(),
				_manualFirstVisible, _manualMaximumFirstVisible,
				_manualVisibleRows,
				ModalDialogManager::kPrimaryPresentation,
				static_cast<ModalDialogManager::TextPanelScrollControl>(
					_manualHoveredControl)))
		warning("Ripper: circuit puzzle manual panel failed");
	presentScreen();
}

void CircuitChipPuzzle::captureMeter() {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}
	_meterPixels.resize(kMeterWidth * kMeterHeight);
	for (int y = 0; y < kMeterHeight; ++y)
		memcpy(_meterPixels.data() + y * kMeterWidth,
			screen->getBasePtr(kMeterX, kMeterY + y), kMeterWidth);
	g_system->unlockScreen();
}

void CircuitChipPuzzle::animateMeter(uint previousCount,
		uint correctCount) {
	if (previousCount == correctCount || correctCount > kGroupCount)
		return;

	const bool progress = correctCount > previousCount;
	const uint tableIndex = progress ? correctCount - 1 : correctCount;
	const uint firstFrame = progress ?
		kProgressStartFrames[tableIndex] :
		kRegressStartFrames[tableIndex];
	const uint frameLimit = progress ?
		kProgressFrameLimits[tableIndex] :
		kRegressFrameLimits[tableIndex];
	const char *media = progress ? "meter2.smk" : "meter3.smk";
	const uint cue = progress ? 3 : 2;

	_engine->getMedia()->playSoundEffect(kAudioNames[cue],
		_audioHandles[cue]);
	_engine->getCursor()->setVisible(false);
	debugC(2, kDebugPuzzles,
		"Ripper: circuit puzzle meter transition direction=%s "
		"groups=%u->%u media='%s' frames=%u..%u position=%d,%d",
		progress ? "progress" : "regress", previousCount, correctCount,
		media, firstFrame, frameLimit - 1, kMeterX, kMeterY);
	if (!_engine->getMedia()->playPuzzleSequenceSegment(media,
			firstFrame, frameLimit - 1, kMeterX,
			kMeterY - kSceneOriginY, nullptr))
		warning("Ripper: circuit puzzle meter animation '%s' failed",
			media);
	captureMeter();
	_engine->getInput()->discardMouseTransitions();
	_engine->getCursor()->setVisible(true);
}

bool CircuitChipPuzzle::advanceIdleMeter(uint32 now) {
	if (_idleMeterFrames.empty() ||
			(int32)(now - _lastMeterTick) <
				(int32)(kMeterTickCount * kDosTickMillis))
		return false;
	if (_idleMeterFrame == 0)
		_engine->getMedia()->playSoundEffect(kAudioNames[0],
			_audioHandles[0]);
	_idleMeterFrame = (_idleMeterFrame + 1) % _idleMeterFrames.size();
	_lastMeterTick = now;
	return true;
}

bool CircuitChipPuzzle::handleManualClick(const Common::Point &point) {
	if (!_manualAvailable)
		return false;
	const ModalDialogManager::TextPanelScrollControl control =
		_engine->getModalDialog()->findTextPanelScrollControl(
			circuitManualBounds(), point, _manualFirstVisible,
			_manualMaximumFirstVisible,
			ModalDialogManager::kPrimaryPresentation);
	if (control == ModalDialogManager::kTextPanelScrollUp &&
			_manualFirstVisible > 0) {
		--_manualFirstVisible;
		return true;
	}
	if (control == ModalDialogManager::kTextPanelScrollDown &&
			_manualFirstVisible < _manualMaximumFirstVisible) {
		++_manualFirstVisible;
		return true;
	}
	return control != ModalDialogManager::kTextPanelScrollNone;
}

int CircuitChipPuzzle::findSourceSlot(
		const Common::Point &point) const {
	for (uint slot = 0; slot < kChipCount; ++slot) {
		const BitmapAssetFrame &frame =
			_chips[_sourceChips[slot] >= 0 ?
				_sourceChips[slot] : slot];
		if (Common::Rect(kSourcePositions[slot].x,
				kSourcePositions[slot].y,
				kSourcePositions[slot].x + frame.width,
				kSourcePositions[slot].y + frame.height).contains(point))
			return slot;
	}
	return -1;
}

int CircuitChipPuzzle::findTargetSlot(
		const Common::Point &point) const {
	for (uint slot = 0; slot < kChipCount; ++slot) {
		const BitmapAssetFrame &frame =
			_chips[_targetChips[slot] >= 0 ?
				_targetChips[slot] : slot];
		if (Common::Rect(kTargetPositions[slot].x,
				kTargetPositions[slot].y,
				kTargetPositions[slot].x + frame.width,
				kTargetPositions[slot].y + frame.height).contains(point))
			return slot;
	}
	return -1;
}

void CircuitChipPuzzle::pickSourceChip(uint slot) {
	_draggedChip = _sourceChips[slot];
	_sourceChips[slot] = -1;
	debugC(2, kDebugPuzzles,
		"Ripper: circuit puzzle picked chip=%d sourceSlot=%u",
		_draggedChip, slot);
}

void CircuitChipPuzzle::pickTargetChip(uint slot) {
	_draggedChip = _targetChips[slot];
	_targetChips[slot] = -1;
	debugC(2, kDebugPuzzles,
		"Ripper: circuit puzzle picked chip=%d targetSlot=%u",
		_draggedChip, slot);
}

void CircuitChipPuzzle::placeSourceChip(uint slot) {
	_sourceChips[slot] = _draggedChip;
	debugC(2, kDebugPuzzles,
		"Ripper: circuit puzzle placed chip=%d sourceSlot=%u",
		_draggedChip, slot);
	_draggedChip = -1;
	_engine->getMedia()->playSoundEffect(kAudioNames[4],
		_audioHandles[4]);
}

void CircuitChipPuzzle::placeTargetChip(uint slot) {
	_targetChips[slot] = _draggedChip;
	debugC(2, kDebugPuzzles,
		"Ripper: circuit puzzle placed chip=%d targetSlot=%u "
		"connector=%u expectedChip=%u",
		_draggedChip, slot, kSlotConnectorTypes[slot],
		kExpectedChipBySlot[slot]);
	_draggedChip = -1;
	_engine->getMedia()->playSoundEffect(kAudioNames[4],
		_audioHandles[4]);
}

bool CircuitChipPuzzle::handleClick(const Common::Point &point) {
	const int sourceSlot = findSourceSlot(point);
	const int targetSlot = findTargetSlot(point);
	if (_draggedChip < 0) {
		if (targetSlot >= 0 && _targetChips[targetSlot] >= 0) {
			pickTargetChip(targetSlot);
			return true;
		}
		if (sourceSlot >= 0 && _sourceChips[sourceSlot] >= 0) {
			pickSourceChip(sourceSlot);
			return true;
		}
		return false;
	}

	if (targetSlot >= 0 && _targetChips[targetSlot] < 0) {
		if (kChipConnectorTypes[_draggedChip] ==
				kSlotConnectorTypes[targetSlot]) {
			placeTargetChip(targetSlot);
			return true;
		}
		debugC(2, kDebugPuzzles,
			"Ripper: circuit puzzle rejected chip=%d targetSlot=%d "
			"chipConnector=%u slotConnector=%u",
			_draggedChip, targetSlot,
			kChipConnectorTypes[_draggedChip],
			kSlotConnectorTypes[targetSlot]);
		return false;
	}
	if (sourceSlot >= 0 && _sourceChips[sourceSlot] < 0) {
		placeSourceChip(sourceSlot);
		return true;
	}
	return false;
}

uint CircuitChipPuzzle::countCorrectGroups(bool logGroups) const {
	uint correctCount = 0;
	for (uint group = 0; group < kGroupCount; ++group) {
		bool correct = true;
		for (uint member = 0; member < kSlotsPerGroup; ++member) {
			const uint slot =
				kMeterSlotGroups[group * kSlotsPerGroup + member];
			if (_targetChips[slot] !=
					(int)kExpectedChipBySlot[slot])
				correct = false;
			if (logGroups) {
				debugC(3, kDebugPuzzles,
					"Ripper: circuit puzzle group=%u member=%u "
					"slot=%u placed=%d expected=%u status=%s",
					group, member, slot, _targetChips[slot],
					kExpectedChipBySlot[slot],
					_targetChips[slot] ==
						(int)kExpectedChipBySlot[slot] ?
						"PASS" : "FAIL");
			}
		}
		if (correct)
			++correctCount;
		debugC(logGroups ? 2 : 3, kDebugPuzzles,
			"Ripper: circuit puzzle group=%u status=%s "
			"slots=%u,%u,%u,%u",
			group, correct ? "PASS" : "FAIL",
			kMeterSlotGroups[group * kSlotsPerGroup],
			kMeterSlotGroups[group * kSlotsPerGroup + 1],
			kMeterSlotGroups[group * kSlotsPerGroup + 2],
			kMeterSlotGroups[group * kSlotsPerGroup + 3]);
	}
	return correctCount;
}

bool CircuitChipPuzzle::complete(uint completionFlag) {
	if (!_engine->getMilestones()->set(completionFlag, true,
			"circuit-chip-puzzle"))
		return false;
	debugC(1, kDebugPuzzles,
		"Ripper: solved circuit chip puzzle milestone=%u "
		"groups=4 placements=[%s]",
		completionFlag, placementString().c_str());
	return true;
}

bool CircuitChipPuzzle::handleKeyword(uint16 command,
		uint completionFlag, Result &result) {
	if (command == 0)
		return false;
	if (command > 0xff) {
		_keywordProgress = 0;
		return false;
	}
	char character = command & 0xff;
	if (character >= 'A' && character <= 'Z')
		character += 'a' - 'A';
	if (character == kCompletionKeyword[_keywordProgress])
		++_keywordProgress;
	else
		_keywordProgress = 0;
	if (kCompletionKeyword[_keywordProgress] != '\0')
		return false;

	debugC(1, kDebugPuzzles,
		"Ripper: circuit chip puzzle solved by hidden keyword '%s'",
		kCompletionKeyword);
	result = complete(completionFlag) ? kSolved : kLoadFailed;
	return true;
}

bool CircuitChipPuzzle::updateCursor(const Common::Point &point) {
	ModalDialogManager::TextPanelScrollControl manualControl =
		ModalDialogManager::kTextPanelScrollNone;
	if (_manualAvailable) {
		manualControl =
			_engine->getModalDialog()->findTextPanelScrollControl(
				circuitManualBounds(), point, _manualFirstVisible,
				_manualMaximumFirstVisible,
				ModalDialogManager::kPrimaryPresentation);
	}
	const int sourceSlot = findSourceSlot(point);
	const int targetSlot = findTargetSlot(point);
	int control = -1;
	bool selectable = false;
	if (manualControl != ModalDialogManager::kTextPanelScrollNone) {
		control = kChipCount * 2 + manualControl;
		selectable = true;
	} else if (targetSlot >= 0) {
		control = kChipCount + targetSlot;
		selectable = _targetChips[targetSlot] >= 0 ||
			(_draggedChip >= 0 && _targetChips[targetSlot] < 0 &&
				kChipConnectorTypes[_draggedChip] ==
					kSlotConnectorTypes[targetSlot]);
	} else if (sourceSlot >= 0) {
		control = sourceSlot;
		selectable = _sourceChips[sourceSlot] >= 0 ||
			(_draggedChip >= 0 && _sourceChips[sourceSlot] < 0);
	}
	const bool changed = control != _hoveredControl;
	if (changed) {
		debugC(2, kDebugInput,
			"Ripper: circuit puzzle hover control=%d previous=%d "
			"sourceSlot=%d targetSlot=%d draggedChip=%d "
			"selectable=%d point=%d,%d",
			control, _hoveredControl, sourceSlot, targetSlot,
			_draggedChip, selectable, point.x, point.y);
		_hoveredControl = control;
	}
	_engine->getCursor()->update(
		selectable ? kSelectionCursor : kDefaultCursor);
	_engine->getCursor()->setVisible(true);
	_manualHoveredControl = manualControl;
	return changed;
}

void CircuitChipPuzzle::stopAudio() {
	for (uint cue = 0; cue < ARRAYSIZE(_audioHandles); ++cue)
		_engine->getMedia()->stopSoundEffect(_audioHandles[cue]);
}

Common::String CircuitChipPuzzle::placementString() const {
	Common::String result;
	for (uint slot = 0; slot < kChipCount; ++slot) {
		if (slot != 0)
			result += ',';
		if (_targetChips[slot] < 0)
			result += '-';
		else
			result += Common::String::format("%d",
				_targetChips[slot]);
	}
	return result;
}

CircuitChipPuzzle::Result CircuitChipPuzzle::run(uint completionFlag) {
	// DispatchSceneEntryAction at 0x36892 maps action 1 to
	// RunCircuitChipPlacementPuzzleScene at 0x28aa4.
	if (!_incomingDisplay.capture() || !loadAssets()) {
		_incomingDisplay.restore();
		return kLoadFailed;
	}

	_engine->getToolbar()->leave();
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	CursorManager *cursor = _engine->getCursor();
	const uint savedSelectionIndex = cursor->getSelectionIndex();
	const bool savedCursorVisible = cursor->isVisible();
	cursor->setSelectionIndex(kDefaultCursor);
	cursor->dispatchSelectionIndexChange(kDefaultCursor);
	cursor->setVisible(false);

	if (!_engine->getMedia()->play("edz10.smk", false)) {
		_incomingDisplay.restore();
		cursor->setSelectionIndex(savedSelectionIndex);
		cursor->dispatchSelectionIndexChange(savedSelectionIndex);
		cursor->setVisible(savedCursorVisible);
		return kLoadFailed;
	}

	_manualAvailable =
		_engine->getMilestones()->isSet(kCircuitManualFlag);
	if (_manualAvailable) {
		_engine->getMedia()->playSoundEffect(kAudioNames[5],
			_audioHandles[5]);
		// RunCircuitChipPlacementPuzzleScene passes physical Y=0x144 in
		// EBX and X=0x5c in ECX to RunMediaSequence.
		if (!_engine->getMedia()->play("ed_wac.smk", false, 92, 324))
			warning("Ripper: circuit puzzle WAC presentation failed");
		else if (!captureManualBacking()) {
			_incomingDisplay.restore();
			cursor->setSelectionIndex(savedSelectionIndex);
			cursor->dispatchSelectionIndexChange(savedSelectionIndex);
			cursor->setVisible(savedCursorVisible);
			return kLoadFailed;
		}
	}
	if (!_engine->getMedia()->playSoundEffect(kAudioNames[1],
			_audioHandles[1], 100, true))
		warning("Ripper: circuit puzzle ambient audio failed");

	_lastMeterTick = g_system->getMillis();
	_correctGroupCount = countCorrectGroups(false);
	_dragPoint = _engine->getInput()->publishMouseState().position;
	_engine->getInput()->discardMouseTransitions();
	cursor->setVisible(true);
	render();
	debugC(1, kDebugPuzzles,
		"Ripper: entered circuit chip puzzle function="
		"RunCircuitChipPlacementPuzzleScene@0x28aa4 "
		"milestone=%u help=0x%x chips=16 slots=16 "
		"manualFlag=0x%x manualAvailable=%d coordinates=physical",
		completionFlag, kHelpSelectionTable, kCircuitManualFlag,
		_engine->getMilestones()->isSet(kCircuitManualFlag));
	debugC(1, kDebugPuzzles,
		"Ripper: circuit chip puzzle solution oneBased="
		"14,4,11,6,15,16,5,1,7,12,9,2,13,8,3,10 "
		"keyword='%s'",
		kCompletionKeyword);

	Result result = kExited;
	bool active = true;
	while (active && !_engine->shouldQuit()) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			break;
		}

		while (_engine->getInput()->hasPendingKey()) {
			const uint16 command = _engine->getInput()->consumeKey();
			if (command == kEscapeCommand) {
				debugC(1, kDebugPuzzles,
					"Ripper: circuit chip puzzle exited by Escape "
					"draggedChip=%d groups=%u",
					_draggedChip, _correctGroupCount);
				active = false;
				break;
			}
			if (command == kHelpCommand) {
				_keywordProgress = 0;
				debugC(1, kDebugPuzzles,
					"Ripper: circuit chip puzzle opening modal "
					"help table=0x%x",
					kHelpSelectionTable);
				if (!_engine->getModalDialog()->run(
						kHelpSelectionTable))
					warning("Ripper: circuit chip puzzle modal help failed");
				render();
				continue;
			}
			if (handleKeyword(command, completionFlag, result)) {
				active = false;
				break;
			}
		}
		if (!active)
			break;

		const MouseState mouse =
			_engine->getInput()->publishMouseState();
		bool redraw = updateCursor(mouse.position);
		if (_draggedChip >= 0 &&
				mouse.position != _dragPoint) {
			_dragPoint = mouse.position;
			redraw = true;
		}
		if ((mouse.pressed & kMouseButtonLeft) != 0) {
			if (handleManualClick(mouse.position)) {
				redraw = true;
			} else {
				const uint previousCount = _correctGroupCount;
				if (handleClick(mouse.position)) {
					_correctGroupCount = countCorrectGroups(true);
					debugC(_correctGroupCount == kGroupCount ? 1 : 2,
						kDebugPuzzles,
						"Ripper: circuit puzzle validation after "
						"placement groups=%u previous=%u "
						"outcome=%s placements=[%s]",
						_correctGroupCount, previousCount,
						_correctGroupCount == kGroupCount ?
							"SOLVED" : "NOT_SOLVED",
						placementString().c_str());
					if (previousCount != _correctGroupCount)
						render();
					animateMeter(previousCount, _correctGroupCount);
					render();
					if (_correctGroupCount == kGroupCount) {
						result = complete(completionFlag) ?
							kSolved : kLoadFailed;
						active = false;
					}
					// The placed chip and the post-animation meter have
					// already been presented above.
					redraw = false;
				}
			}
		}
		redraw |= advanceIdleMeter(g_system->getMillis());
		if (redraw)
			render();
		presentScreen();
		g_system->delayMillis(10);
	}

	stopAudio();
	cursor->setVisible(false);
	if (!_engine->shouldQuit()) {
		g_system->fillScreen(0);
		presentScreen();
	}
	cursor->setSelectionIndex(savedSelectionIndex);
	cursor->dispatchSelectionIndexChange(savedSelectionIndex);
	cursor->refresh();
	cursor->setVisible(savedCursorVisible);
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	debugC(result == kLoadFailed ? 2 : 1, kDebugPuzzles,
		"Ripper: left circuit chip puzzle result=%d milestone=%u "
		"milestoneSet=%d groups=%u draggedChip=%d quit=%d",
		result, completionFlag,
		_engine->getMilestones()->isSet(completionFlag),
		_correctGroupCount, _draggedChip, _engine->shouldQuit());
	return result;
}

} // End of namespace Ripper
