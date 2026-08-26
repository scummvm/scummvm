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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "hollywood/gameplay/travel_screen.h"

#include "common/events.h"
#include "common/path.h"
#include "common/ptr.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "hollywood/gameplay/cursor.h"
#include "hollywood/gameplay/game_state.h"
#include "hollywood/hollywood.h"
#include "hollywood/resource.h"

namespace Hollywood {

const char *const kTravelScreenArchiveName = "RESOURCE.I04";
const uint kTravelScreenFramebufferBytes = HollywoodEngine::kSceneBufferWidth * HollywoodEngine::kSceneBufferHeight;
const uint kTravelScreenPaletteChunk = 0;
const uint kTravelScreenFramebufferChunk = 1;
const uint kTravelScreenFirstTileChunk = 2;
const uint kTravelScreenTileChunkCount = 5;
const uint kTravelScreenFillRunChunk = 7;
const uint kTravelScreenFillRunBytes = 0xf45;
const uint kTravelScreenTileWidth = 200;
const uint kTravelScreenTileHeight = 0x5d;
const uint kTravelScreenTileBytes = kTravelScreenTileWidth * kTravelScreenTileHeight;
const uint kTravelScreenVisibleSlotCount = 7;
const byte kTravelScreenInvalidSlot = 0xff;
const int kTravelSlotX[] = { 0x5e, 0x5e, 0x5e, 0x5e, 0x16a, 0x16a, 0x16a };
const int kTravelSlotY[] = { 0x37, 0x94, 0xf1, 0x14e, 0x37, 0x94, 0xf1 };
const byte kTravelDestinationTileSourceIds[] = { 0, 0, 1, 2, 3, 0, 4 };
const uint16 kTravelDestinationStateIds[] = { 1000, 2000, 3000, 4000, 5000, 6000, 8000 };
const uint16 kTravelSameAreaStateIds[] = { 0x03f4, 0x07ee, 0x0bc2, 0x0faa, 0x1392, 0x177a, 0x1f4a };
const byte kEgyptChapterId = 2;
const uint16 kEgyptChapterEntryState = 2000;
const uint16 kTravelInterludeState = 9140;
const byte kTravelScreenNormalRamp[] = {
	0x00, 0x00, 0x00, 0x05, 0x05, 0x05, 0x0d, 0x0d, 0x0d,
	0x15, 0x15, 0x15, 0x1e, 0x1e, 0x1e, 0x28, 0x28, 0x28,
	0x31, 0x31, 0x31
};
const byte kTravelScreenHighlightRamp[] = {
	0x32, 0x00, 0x00, 0x32, 0x07, 0x07, 0x33, 0x0e, 0x0e,
	0x34, 0x15, 0x15, 0x34, 0x1e, 0x1e, 0x35, 0x25, 0x25,
	0x36, 0x2e, 0x2e
};

TravelScreen::TravelScreen(HollywoodEngine *vm) :
		_vm(vm),
		_palette(),
		_tilePixels(),
		_fillRuns(),
		_framebuffer(),
		_selectionMask(),
		_screen(),
		_displayPalette() {
}

bool TravelScreen::showViewer() {
	if (!load(false))
		return false;

	present();

	bool done = false;
	while (!done && !Engine::shouldQuit()) {
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				Engine::quitGame();
				return true;
			case Common::EVENT_MAINMENU:
				_vm->openMainMenuDialog();
				if (_vm->isSceneRestartRequested())
					return true;
				_displayPalette.markAllDirty();
				present();
				break;
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
					done = true;
				break;
			case Common::EVENT_RBUTTONDOWN:
				done = true;
				break;
			default:
				break;
			}
		}
		if (!done)
			g_system->delayMillis(10);
	}

	return true;
}

bool TravelScreen::runSelection(byte currentChapterId, uint16 &selectedStateId) {
	if (!load(true))
		return false;

	_vm->gameplayMusic()->stop();
	_vm->cursor()->enterInteractiveMode();
	_vm->cursor()->updatePosition(g_system->getEventManager()->getMousePos());

	byte previousSlot = kTravelScreenInvalidSlot;
	uint32 lastMillis = g_system->getMillis();
	present();

	while (!Engine::shouldQuit()) {
		Common::Event event;
		bool selectRequested = false;
		byte requestedSlot = kTravelScreenInvalidSlot;

		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				Engine::quitGame();
				break;
			case Common::EVENT_MAINMENU:
				_vm->openMainMenuDialog();
				if (_vm->isSceneRestartRequested()) {
					_vm->cursor()->leaveInteractiveMode();
					return true;
				}
				_displayPalette.markAllDirty();
				present();
				break;
			case Common::EVENT_MOUSEMOVE:
				_vm->cursor()->updatePosition(event.mouse);
				break;
			case Common::EVENT_LBUTTONDOWN:
				selectRequested = true;
				requestedSlot = slotAtPoint(event.mouse.x, event.mouse.y);
				break;
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_RETURN ||
						event.kbd.keycode == Common::KEYCODE_KP_ENTER ||
						event.kbd.keycode == Common::KEYCODE_SPACE) {
					selectRequested = true;
					requestedSlot = slotAtPoint(_vm->cursor()->surfaceX(), _vm->cursor()->surfaceY());
				}
				break;
			default:
				break;
			}

			if (Engine::shouldQuit())
				break;
		}

		if (selectRequested && isActiveSlot(requestedSlot)) {
			GameplayState &state = _vm->gameState();
			const byte destinationId = state.travelScreenSlotIds[requestedSlot];
			selectedStateId = destinationState(destinationId, currentChapterId);
			if (currentChapterId != kEgyptChapterId &&
					selectedStateId == kEgyptChapterEntryState &&
					state.scene2100MummyBranchState == 1 && state.scene2110TreasureGranted) {
				state.scene2100MummyBranchState = 2;
				state.scene2010TravelReturnSpeechState = 1;
				state.scene9140ReturnStateId = kEgyptChapterEntryState;
				selectedStateId = kTravelInterludeState;
			}
			_vm->cursor()->leaveInteractiveMode();
			return true;
		}

		const byte slotIndex = slotAtPoint(_vm->cursor()->surfaceX(), _vm->cursor()->surfaceY());
		if (slotIndex != previousSlot) {
			applySlotPalette(previousSlot, false);
			if (isActiveSlot(slotIndex)) {
				applySlotPalette(slotIndex, true);
				previousSlot = slotIndex;
			} else {
				previousSlot = kTravelScreenInvalidSlot;
			}
			_displayPalette.markAllDirty();
		}

		const uint32 now = g_system->getMillis();
		const uint32 delta = now - lastMillis;
		lastMillis = now;
		_vm->cursor()->advance(delta);
		present();
		g_system->delayMillis(10);
	}

	_vm->cursor()->leaveInteractiveMode();
	return true;
}

bool TravelScreen::load(bool loadSelectionMask) {
	Common::Array<byte> framebufferChunk;
	if (!readChunk(kTravelScreenPaletteChunk, _palette, kPaletteSize) ||
			!readChunk(kTravelScreenFramebufferChunk, framebufferChunk, kTravelScreenFramebufferBytes))
		return false;

	_framebuffer.resize(kTravelScreenFramebufferBytes);
	memcpy(_framebuffer.data(), framebufferChunk.data(), framebufferChunk.size());

	_tilePixels.resize(kTravelScreenTileBytes * kTravelScreenTileChunkCount);
	for (uint chunk = 0; chunk < kTravelScreenTileChunkCount; ++chunk) {
		Common::Array<byte> sourceChunk;
		if (!readChunk(kTravelScreenFirstTileChunk + chunk, sourceChunk, kTravelScreenTileBytes))
			return false;
		memcpy(_tilePixels.data() + chunk * kTravelScreenTileBytes,
			sourceChunk.data(), sourceChunk.size());
	}

	composeUnlockedSlots();

	if (loadSelectionMask) {
		if (!readChunk(kTravelScreenFillRunChunk, _fillRuns, kTravelScreenFillRunBytes))
			return false;
		expandSelectionMask();
	}

	return true;
}

bool TravelScreen::readChunk(uint index, Common::Array<byte> &destination, uint expectedSize) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		_vm->resources()->createChunkReadStream(Common::Path(kTravelScreenArchiveName), index));
	if (!stream) {
		warning("Failed to open %s chunk %u", kTravelScreenArchiveName, index);
		return false;
	}

	if ((uint)stream->size() != expectedSize) {
		warning("%s chunk %u has unexpected size %u, expected %u",
			kTravelScreenArchiveName, index, (uint)stream->size(), expectedSize);
		return false;
	}

	destination.resize(expectedSize);
	if (stream->read(destination.data(), destination.size()) != destination.size()) {
		warning("Failed to read %s chunk %u", kTravelScreenArchiveName, index);
		return false;
	}

	return true;
}

void TravelScreen::composeUnlockedSlots() {
	const GameplayState &state = _vm->gameState();
	for (uint slot = 2; slot < kTravelScreenVisibleSlotCount; ++slot) {
		const byte destinationId = state.travelScreenSlotIds[slot];
		if (destinationId == GameplayState::kTravelScreenDisabledSlot ||
				destinationId >= ARRAYSIZE(kTravelDestinationTileSourceIds))
			continue;

		const byte sourceId = kTravelDestinationTileSourceIds[destinationId];
		if (sourceId >= kTravelScreenTileChunkCount)
			continue;

		for (uint row = 0; row < kTravelScreenTileHeight; ++row) {
			const int dstY = kTravelSlotY[slot] + row;
			if (dstY < 0 || dstY >= HollywoodEngine::kSceneBufferHeight)
				continue;

			const uint sourceOffset = sourceId * kTravelScreenTileBytes +
				row * kTravelScreenTileWidth;
			const uint destinationOffset = dstY * HollywoodEngine::kSceneBufferWidth +
				kTravelSlotX[slot];
			for (uint column = 0; column < kTravelScreenTileWidth; ++column) {
				const int dstX = kTravelSlotX[slot] + column;
				if (dstX < 0 || dstX >= HollywoodEngine::kSceneBufferWidth)
					continue;

				const byte pixel = _tilePixels[sourceOffset + column];
				if (pixel != 0xff)
					_framebuffer[destinationOffset + column] = pixel;
			}
		}
	}
}

void TravelScreen::expandSelectionMask() {
	_selectionMask.resize(kTravelScreenFramebufferBytes);
	_selectionMask.clear(kTravelScreenInvalidSlot);

	uint destinationOffset = 0;
	uint sourceOffset = 0;
	while (destinationOffset < _selectionMask.size() && sourceOffset + 3 <= _fillRuns.size()) {
		const byte fill = _fillRuns[sourceOffset];
		const uint16 runLength = readUint16LE(_fillRuns, sourceOffset + 1);
		sourceOffset += 3;

		const uint count = MIN<uint>(runLength, _selectionMask.size() - destinationOffset);
		if (count != 0) {
			memset(_selectionMask.data() + destinationOffset, fill, count);
			destinationOffset += count;
		}
	}
}

void TravelScreen::applySlotPalette(byte slotIndex, bool highlighted) {
	if (!isActiveSlot(slotIndex))
		return;

	const byte destinationId = _vm->gameState().travelScreenSlotIds[slotIndex];
	const uint paletteOffset = 0x252 + destinationId * 0x18;
	if (paletteOffset + ARRAYSIZE(kTravelScreenNormalRamp) > _palette.size())
		return;

	memcpy(_palette.data() + paletteOffset,
		highlighted ? kTravelScreenHighlightRamp : kTravelScreenNormalRamp,
		ARRAYSIZE(kTravelScreenNormalRamp));
}

bool TravelScreen::isActiveSlot(byte slotIndex) const {
	return slotIndex < kTravelScreenVisibleSlotCount &&
		_vm->gameState().travelScreenSlotIds[slotIndex] != GameplayState::kTravelScreenDisabledSlot &&
		_vm->gameState().travelScreenSlotIds[slotIndex] < ARRAYSIZE(kTravelDestinationStateIds);
}

byte TravelScreen::slotAtPoint(int x, int y) const {
	if (x < 0 || y < 0 ||
			x >= HollywoodEngine::kSceneBufferWidth ||
			y >= HollywoodEngine::kSceneBufferHeight ||
			_selectionMask.empty())
		return kTravelScreenInvalidSlot;

	return _selectionMask.data()[y * HollywoodEngine::kSceneBufferWidth + x];
}

uint16 TravelScreen::destinationState(byte destinationId, byte currentChapterId) const {
	if (destinationId >= ARRAYSIZE(kTravelDestinationStateIds))
		return 0xffff;

	const uint16 destinationStateId = kTravelDestinationStateIds[destinationId];
	if (destinationStateId / 1000 == currentChapterId)
		return kTravelSameAreaStateIds[destinationId];

	return destinationStateId;
}

void TravelScreen::present() {
	presentIndexedFrame(_framebuffer.surface(), _palette, _screen, _displayPalette);
}

} // End of namespace Hollywood
