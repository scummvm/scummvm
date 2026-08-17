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

#include "ripper/puzzles/key_group.h"

#include "common/debug.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/system.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/milestones.h"
#include "ripper/modal_dialog.h"
#include "ripper/ripper.h"
#include "ripper/settings.h"
#include "ripper/toolbar.h"

namespace Ripper {

namespace {

static const uint kSourceSlotCount = 20;
static const uint kTargetSlotCount = 14;
static const int kSourceX = 14;
static const int kTargetX = 330;
static const int kNavigationWidth = 50;
static const int kNextNavigationLeft = 590;
static const uint kDefaultCursor = 14;
static const uint kPieceCursor = 16;
static const uint kNavigationCursor = 10;
static const uint kHelpSelectionTable = 0x1b5;
static const uint kPreviousPageFlag = 0x156;
static const uint kNextPageFlag = 0x157;
static const uint kCancelledFlag = 0x158;
static const uint16 kEscapeCommand = 0x1b;
static const uint16 kHelpCommand = 0x3b00;
static const uint16 kPreviousPageCommand = 0x4b00;
static const uint16 kNextPageCommand = 0x4d00;
static const char kCompletionKeyword[] = "sponge";

// RunKeyGroupPuzzleScene at 0x3ca1f reads these source-control Y values
// from g_keyGroupPuzzleSourcePieceControlY at 0x84c9a. The controls use
// the puzzle's fixed physical X coordinate of 14.
static const int kSourceY[kSourceSlotCount] = {
	37, 54, 71, 88, 105, 122, 139, 156, 172, 189,
	206, 223, 240, 256, 273, 289, 306, 323, 339, 355
};

// g_keyGroupPuzzleTargetPieceControlY at 0x84cc2 supplies these physical
// Y values. The corresponding fixed X coordinate is 330.
static const int kTargetY[kTargetSlotCount] = {
	44, 65, 87, 108, 129, 150, 171,
	192, 213, 233, 254, 275, 296, 317
};

// g_keyGroupPuzzleSolvedTargetPieces at 0x84cfa.
static const int kSolvedTargetPieces[kTargetSlotCount] = {
	0, 4, 10, 15, 1, 13, 16, 5, 9, 7, 18, 19, 14, 11
};

static RipperEngine *sStateOwner = nullptr;
static int sSourceSlots[kSourceSlotCount];
static int sTargetSlots[kTargetSlotCount];

} // End of anonymous namespace

KeyGroupPuzzle::KeyGroupPuzzle(RipperEngine *engine) :
		Puzzle(engine), _maxPieceWidth(0), _maxPieceHeight(0),
		_heldPiece(-1), _heldPosition(0, 0), _heldOffset(0, 0),
		_keywordIndex(0) {
	for (uint index = 0; index < kSourceSlotCount; ++index)
		_sourceSlots[index] = index;
	for (uint index = 0; index < kTargetSlotCount; ++index)
		_targetSlots[index] = -1;
}

bool KeyGroupPuzzle::loadBackground() {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_engine->getResources()->createReadStreamForPath("kgp_2.pcx"));
	if (!stream || !decodePcxAsset(*stream, _background)) {
		warning("Ripper: could not decode key group background 'kgp_2.pcx'");
		return false;
	}
	if (_background.width != kRipperScreenWidth ||
			_background.height != kRipperScreenHeight ||
			_background.palette.size() != kRipperPaletteByteCount) {
		warning("Ripper: key group background has invalid size=%ux%u colors=%u",
			_background.width, _background.height,
			_background.palette.size() / 3);
		return false;
	}
	return true;
}

bool KeyGroupPuzzle::loadPiece(uint piece) {
	const Common::String name = Common::String::format("kg_p%u.bbm", piece);
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_engine->getResources()->createReadStreamForPath(name));
	if (!stream || !decodeBitmapAsset(*stream, _pieces[piece])) {
		warning("Ripper: could not decode key group piece '%s'", name.c_str());
		return false;
	}
	const BitmapAssetFrame &frame = _pieces[piece];
	if (frame.width == 0 || frame.height == 0 ||
			frame.pixels.size() != (uint32)frame.width * frame.height ||
			frame.width > kRipperScreenWidth ||
			frame.height > kRipperScreenHeight) {
		warning("Ripper: key group piece '%s' has invalid size=%ux%u pixels=%u",
			name.c_str(), frame.width, frame.height, frame.pixels.size());
		return false;
	}
	_maxPieceWidth = MAX<uint>(_maxPieceWidth, frame.width);
	_maxPieceHeight = MAX<uint>(_maxPieceHeight, frame.height);
	return true;
}

bool KeyGroupPuzzle::loadAssets() {
	if (!loadBackground())
		return false;
	for (uint piece = 0; piece < kSourceSlotCount; ++piece) {
		if (!loadPiece(piece))
			return false;
	}
	debugC(1, kDebugPuzzles,
		"Ripper: loaded key group puzzle assets background=%ux%u pieces=%u "
		"controlSize=%ux%u paletteEntries=%u",
		_background.width, _background.height, kSourceSlotCount,
		_maxPieceWidth, _maxPieceHeight, _background.palette.size() / 3);
	return true;
}

void KeyGroupPuzzle::loadPersistentState() {
	if (sStateOwner != _engine) {
		sStateOwner = _engine;
		for (uint index = 0; index < kSourceSlotCount; ++index)
			sSourceSlots[index] = index;
		for (uint index = 0; index < kTargetSlotCount; ++index)
			sTargetSlots[index] = -1;
	}
	for (uint index = 0; index < kSourceSlotCount; ++index)
		_sourceSlots[index] = sSourceSlots[index];
	for (uint index = 0; index < kTargetSlotCount; ++index)
		_targetSlots[index] = sTargetSlots[index];
}

void KeyGroupPuzzle::storePersistentState() const {
	for (uint index = 0; index < kSourceSlotCount; ++index)
		sSourceSlots[index] = _sourceSlots[index];
	for (uint index = 0; index < kTargetSlotCount; ++index)
		sTargetSlots[index] = _targetSlots[index];
}

void KeyGroupPuzzle::applyPalette() const {
	Common::Array<byte> palette = _background.palette;
	_engine->applySharedPalettePatch(palette.data(),
		kRipperPaletteColorCount);
	_engine->getSettings()->applyVideoPalette(palette.data(),
		kRipperPaletteColorCount, true);
	g_system->getPaletteManager()->setPalette(palette.data(), 0,
		kRipperPaletteColorCount);
}

void KeyGroupPuzzle::drawFrame(byte *screen, uint pitch,
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

bool KeyGroupPuzzle::render() const {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1 ||
			screen->w < kRipperScreenWidth || screen->h < kRipperScreenHeight) {
		if (screen)
			g_system->unlockScreen();
		return false;
	}
	for (uint y = 0; y < _background.height; ++y) {
		memcpy(screen->getBasePtr(0, y),
			_background.pixels.data() + y * _background.width,
			_background.width);
	}
	byte *pixels = (byte *)screen->getPixels();
	for (uint index = 0; index < kSourceSlotCount; ++index) {
		if (_sourceSlots[index] < 0)
			continue;
		const Common::Point origin = slotOrigin(kSourceSlotBank, index);
		drawFrame(pixels, screen->pitch, _pieces[_sourceSlots[index]],
			origin.x, origin.y);
	}
	for (uint index = 0; index < kTargetSlotCount; ++index) {
		if (_targetSlots[index] < 0)
			continue;
		const Common::Point origin = slotOrigin(kTargetSlotBank, index);
		drawFrame(pixels, screen->pitch, _pieces[_targetSlots[index]],
			origin.x, origin.y);
	}
	if (_heldPiece >= 0)
		drawFrame(pixels, screen->pitch, _pieces[_heldPiece],
			_heldPosition.x, _heldPosition.y);
	g_system->unlockScreen();
	applyPalette();
	presentScreen();
	return true;
}

Common::Point KeyGroupPuzzle::slotOrigin(SlotBank bank, uint index) const {
	if (bank == kSourceSlotBank && index < kSourceSlotCount)
		return Common::Point(kSourceX, kSourceY[index]);
	if (bank == kTargetSlotBank && index < kTargetSlotCount)
		return Common::Point(kTargetX, kTargetY[index]);
	return Common::Point(-1, -1);
}

KeyGroupPuzzle::SlotHit KeyGroupPuzzle::findSlot(
		const Common::Point &point) const {
	for (uint index = 0; index < kSourceSlotCount; ++index) {
		const Common::Point origin = slotOrigin(kSourceSlotBank, index);
		if (Common::Rect(origin.x, origin.y,
				origin.x + _maxPieceWidth,
				origin.y + _maxPieceHeight).contains(point))
			return SlotHit(kSourceSlotBank, index);
	}
	for (uint index = 0; index < kTargetSlotCount; ++index) {
		const Common::Point origin = slotOrigin(kTargetSlotBank, index);
		if (Common::Rect(origin.x, origin.y,
				origin.x + _maxPieceWidth,
				origin.y + _maxPieceHeight).contains(point))
			return SlotHit(kTargetSlotBank, index);
	}
	return SlotHit();
}

int &KeyGroupPuzzle::slotValue(const SlotHit &slot) {
	if (slot.bank == kSourceSlotBank)
		return _sourceSlots[slot.index];
	return _targetSlots[slot.index];
}

int KeyGroupPuzzle::slotValue(const SlotHit &slot) const {
	if (slot.bank == kSourceSlotBank)
		return _sourceSlots[slot.index];
	return _targetSlots[slot.index];
}

bool KeyGroupPuzzle::pickUp(const SlotHit &slot,
		const Common::Point &point) {
	if (slot.bank == kNoSlotBank || slotValue(slot) < 0)
		return false;
	const Common::Point origin = slotOrigin(slot.bank, slot.index);
	_heldPiece = slotValue(slot);
	_heldPosition = origin;
	_heldOffset = point - origin;
	slotValue(slot) = -1;
	debugC(2, kDebugPuzzles,
		"Ripper: key group picked piece=%d bank=%s slot=%d "
		"origin=%d,%d pointerOffset=%d,%d",
		_heldPiece, slot.bank == kSourceSlotBank ? "source" : "target",
		slot.index, origin.x, origin.y, _heldOffset.x, _heldOffset.y);
	return true;
}

bool KeyGroupPuzzle::putDown(const SlotHit &slot) {
	if (_heldPiece < 0 || slot.bank == kNoSlotBank || slotValue(slot) >= 0)
		return false;
	const int piece = _heldPiece;
	slotValue(slot) = piece;
	_heldPiece = -1;
	const Common::Point origin = slotOrigin(slot.bank, slot.index);
	debugC(2, kDebugPuzzles,
		"Ripper: key group placed piece=%d bank=%s slot=%d origin=%d,%d",
		piece, slot.bank == kSourceSlotBank ? "source" : "target",
		slot.index, origin.x, origin.y);
	return true;
}

void KeyGroupPuzzle::updateHeldPosition(const Common::Point &point) {
	if (_heldPiece >= 0)
		_heldPosition = point - _heldOffset;
}

void KeyGroupPuzzle::updateCursor(const Common::Point &point) {
	const bool navigationActive = _heldPiece < 0 &&
		(point.x < kNavigationWidth || point.x >= kNextNavigationLeft);
	const SlotHit slot = navigationActive ? SlotHit() : findSlot(point);
	if (slot.bank != _hoveredSlot.bank || slot.index != _hoveredSlot.index) {
		debugC(3, kDebugPuzzles,
			"Ripper: key group hover bank=%d slot=%d previousBank=%d "
			"previousSlot=%d point=%d,%d held=%d",
			slot.bank, slot.index, _hoveredSlot.bank, _hoveredSlot.index,
			point.x, point.y, _heldPiece);
		_hoveredSlot = slot;
	}
	uint cursor = kDefaultCursor;
	if (navigationActive)
		cursor = kNavigationCursor;
	else if (slot.bank != kNoSlotBank)
		cursor = kPieceCursor;
	_engine->getCursor()->update(cursor);
	_engine->getCursor()->setVisible(true);
}

bool KeyGroupPuzzle::isSolved() const {
	for (uint index = 0; index < kTargetSlotCount; ++index) {
		if (_targetSlots[index] != kSolvedTargetPieces[index])
			return false;
	}
	return true;
}

bool KeyGroupPuzzle::complete(uint completionFlag, const char *source) {
	if (!_engine->getMilestones()->set(completionFlag, true, source))
		return false;
	debugC(1, kDebugPuzzles,
		"Ripper: solved key group puzzle milestone=%u target=[%s] source='%s'",
		completionFlag, targetStateString().c_str(), source);
	return true;
}

bool KeyGroupPuzzle::serviceKeyword(uint16 command) {
	if (command > 0xff)
		return false;
	char character = (char)command;
	if (character >= 'A' && character <= 'Z')
		character += 'a' - 'A';
	if (character == kCompletionKeyword[_keywordIndex])
		++_keywordIndex;
	else
		_keywordIndex = 0;
	if (_keywordIndex + 1 != ARRAYSIZE(kCompletionKeyword))
		return false;
	_keywordIndex = 0;
	debugC(1, kDebugPuzzles,
		"Ripper: key group completion keyword matched keyword='%s'",
		kCompletionKeyword);
	return true;
}

bool KeyGroupPuzzle::setNavigationFlags(bool previous, bool next,
		bool cancelled) {
	bool success = true;
	if (!_engine->getMilestones()->set(kPreviousPageFlag, previous,
			"key-group-navigation"))
		success = false;
	if (!_engine->getMilestones()->set(kNextPageFlag, next,
			"key-group-navigation"))
		success = false;
	if (!_engine->getMilestones()->set(kCancelledFlag, cancelled,
			"key-group-navigation"))
		success = false;
	debugC(success ? 1 : 2, kDebugPuzzles,
		"Ripper: key group navigation previous=%d next=%d cancelled=%d "
		"flags=%u,%u,%u success=%d",
		previous, next, cancelled, kPreviousPageFlag, kNextPageFlag,
		kCancelledFlag, success);
	return success;
}

Common::String KeyGroupPuzzle::targetStateString() const {
	Common::String state;
	for (uint index = 0; index < kTargetSlotCount; ++index) {
		if (!state.empty())
			state += ',';
		state += Common::String::format("%d", _targetSlots[index]);
	}
	return state;
}

KeyGroupPuzzle::Result KeyGroupPuzzle::run(uint completionFlag) {
	// DispatchSceneEntryAction at 0x36892 maps both actions 16 and 60 to
	// RunKeyGroupPuzzleScene at 0x3ca1f with the caller-supplied flag in EAX.
	if (!_incomingDisplay.capture() || !loadAssets()) {
		_incomingDisplay.restore();
		return kLoadFailed;
	}

	loadPersistentState();
	_heldPiece = -1;
	_hoveredSlot = SlotHit();
	_keywordIndex = 0;
	_engine->getToolbar()->leave();
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	_engine->getCursor()->setSelectionIndex(kDefaultCursor);
	_engine->getCursor()->dispatchSelectionIndexChange(kDefaultCursor);
	_engine->getCursor()->setVisible(true);
	if (!render()) {
		_incomingDisplay.restore();
		return kLoadFailed;
	}
	debugC(1, kDebugPuzzles,
		"Ripper: entered key group puzzle function=RunKeyGroupPuzzleScene@0x3ca1f "
		"milestone=%u sourceControls=%u targetControls=%u help=0x%x "
		"keyword='%s' target=[%s]",
		completionFlag, kSourceSlotCount, kTargetSlotCount,
		kHelpSelectionTable, kCompletionKeyword, targetStateString().c_str());
	debugC(3, kDebugPuzzles,
		"Ripper: key group retail solution=[0,4,10,15,1,13,16,5,9,7,18,19,14,11]");

	Result result = kExited;
	bool active = true;
	while (active && !_engine->shouldQuit()) {
		if (!serviceEngineEvents())
			break;

		while (_engine->getInput()->hasPendingKey()) {
			const uint16 command = _engine->getInput()->consumeKey();
			if (command == kEscapeCommand) {
				if (!setNavigationFlags(false, false, true))
					result = kLoadFailed;
				active = false;
				break;
			}
			if (command == kPreviousPageCommand) {
				if (!setNavigationFlags(true, false, false))
					result = kLoadFailed;
				active = false;
				break;
			}
			if (command == kNextPageCommand) {
				if (!setNavigationFlags(false, true, false))
					result = kLoadFailed;
				active = false;
				break;
			}
			if (command == kHelpCommand) {
				debugC(1, kDebugPuzzles,
					"Ripper: key group puzzle opening modal help table=0x%x",
					kHelpSelectionTable);
				if (!_engine->getModalDialog()->run(kHelpSelectionTable))
					warning("Ripper: key group puzzle modal help failed");
				if (!render()) {
					result = kLoadFailed;
					active = false;
					break;
				}
				continue;
			}
			if (serviceKeyword(command)) {
				result = complete(completionFlag, "key-group-keyword") ?
					kSolved : kLoadFailed;
				active = false;
				break;
			}
		}
		if (!active)
			break;

		const MouseState mouse = _engine->getInput()->publishMouseState();
		const Common::Point previousHeldPosition = _heldPosition;
		updateHeldPosition(mouse.position);
		updateCursor(mouse.position);
		bool redraw = _heldPiece >= 0 && previousHeldPosition != _heldPosition;
		if ((mouse.pressed & kMouseButtonLeft) != 0) {
			if (_heldPiece < 0 && mouse.position.x < kNavigationWidth) {
				if (!setNavigationFlags(true, false, false))
					result = kLoadFailed;
				active = false;
			} else if (_heldPiece < 0 &&
					mouse.position.x >= kNextNavigationLeft) {
				if (!setNavigationFlags(false, true, false))
					result = kLoadFailed;
				active = false;
			} else {
				const SlotHit slot = findSlot(mouse.position);
				bool changed = false;
				const char *operation;
				if (_heldPiece < 0) {
					changed = pickUp(slot, mouse.position);
					operation = "pick";
				} else {
					changed = putDown(slot);
					operation = "place";
				}
				if (changed) {
					storePersistentState();
					const bool solved = isSolved();
					debugC(solved ? 1 : 2, kDebugPuzzles,
						"Ripper: key group validation after %s outcome=%s "
						"held=%d target=[%s] milestone=%u milestoneSet=%d",
						operation, solved ? "SOLVED" : "NOT_SOLVED",
						_heldPiece, targetStateString().c_str(), completionFlag,
						_engine->getMilestones()->isSet(completionFlag));
					if (solved) {
						result = complete(completionFlag,
							"key-group-puzzle") ? kSolved : kLoadFailed;
						active = false;
					}
					redraw = true;
				}
			}
		}

		if (redraw && !render()) {
			result = kLoadFailed;
			active = false;
		} else if (!redraw) {
			presentScreen();
		}
		g_system->delayMillis(10);
	}

	storePersistentState();
	if (result == kLoadFailed)
		_incomingDisplay.restore();
	_engine->getCursor()->setSelectionIndex(0);
	_engine->getCursor()->dispatchSelectionIndexChange(0);
	_engine->getCursor()->refresh();
	_engine->getCursor()->setVisible(true);
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	debugC(result == kLoadFailed ? 2 : 1, kDebugPuzzles,
		"Ripper: left key group puzzle result=%d milestone=%u milestoneSet=%d "
		"held=%d target=[%s] quit=%d",
		result, completionFlag,
		_engine->getMilestones()->isSet(completionFlag), _heldPiece,
		targetStateString().c_str(), _engine->shouldQuit());
	return result;
}

} // End of namespace Ripper
