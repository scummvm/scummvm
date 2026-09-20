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

#include "action_menu_controller.h"

#include "common/scummsys.h"

#include "scooby/common/span.h"
#include "scooby/graphics/tile_cell.h"
#include "scooby/graphics/tile_layer.h"
#include "scooby/rooms/room_object.h"

namespace Scooby {
const Common::Array<int16> ActionMenuController::kActionIconByCommand{0, 2, 10, 3, 7, 9, 6, 8, 4, 1, 5};

namespace {
void WriteUInt16BigEndian(Span<uint8> bytes, int offset, uint16 value) {
	bytes[static_cast<std::size_t>(offset)] = static_cast<uint8>(value >> 8);
	bytes[static_cast<std::size_t>(offset + 1)] = static_cast<uint8>(value);
}

void CopyBytes(Span<const uint8> source, Span<uint8> destination) {
	for (std::size_t i = 0; i < source.size(); ++i) {
		destination[i] = source[i];
	}
}
} // namespace

void ActionMenuController::refreshSelectedActionIcons() {
	// Ghidra 0x00001BDA-0x00001BDD: native full-register preservation becomes the managed call frame.

	// Ghidra 0x00001BDE-0x00001BF5: clear only display bit three, stage the active icon, and invoke the
	// existing renderer in mode zero even when the staged identity is zero.
	_state.DisplayFlags &= static_cast<uint8>(~kSelectedActionIconRefreshMask);
	_state.PendingActionIcon = _state.ActiveActionIcon;
	drawSelectedActionIcon(0);

	// Ghidra 0x00001BF6-0x00001C05: independently stage the cached icon and preserve the second
	// unconditional mode-zero draw instead of coalescing equal or zero identities.
	_state.PendingActionIcon = _state.CachedActionIcon;
	drawSelectedActionIcon(0);

	// Ghidra 0x00001C06-0x00001C0B: managed return restores the native register-preservation boundary.
}

void ActionMenuController::synchronizeActionIcon() {
	int16 previousActionIcon = 0;
	if (_state.PreviousInteraction != _state.CurrentInteraction &&
		resolveActionIcon(_state.PreviousInteraction, previousActionIcon) &&
		previousActionIcon != _state.ActiveActionIcon) {
		_state.PendingActionIcon = previousActionIcon;
		drawSelectedActionIcon(0);
	}

	int16 currentActionIcon = 0;
	if (_state.ActiveActionIcon == 0 && resolveActionIcon(_state.CurrentInteraction, currentActionIcon)) {
		_state.CachedActionIcon = currentActionIcon;
		if (currentActionIcon != 0) {
			_state.PendingActionIcon = currentActionIcon;
			drawSelectedActionIcon(2);
		}
	}
}

bool ActionMenuController::updateActionMenu() {
	int16 command = _state.MenuCommand;
	if ((_state.DisplayFlags & kMenuDisplayMask) == 0) {
		if (command == 0) {
			return true;
		}

		if ((_state.ControllerOneInput & 0x10) == 0) {
			int16 selectedAction = kActionIconByCommand[static_cast<std::size_t>(command)];
			if (_state.ActiveActionIcon == 0) {
				_state.PendingActionIcon = _state.CachedActionIcon;
				drawSelectedActionIcon(0);
			}

			if (selectedAction != _state.ActiveActionIcon) {
				_state.PendingActionIcon = _state.ActiveActionIcon;
				drawSelectedActionIcon(0);
				_state.InteractionFlags &= static_cast<uint8>(~kAlternateInteractionMask);
				_state.ActiveActionIcon = selectedAction;
				_state.PendingActionIcon = selectedAction;
				drawSelectedActionIcon(1);
			}
		}

		_state.MenuCommand = 0;
		return _waitForRoomVerticalBlank();
	}

	if (command == 5 || command == 10) {
		if ((_state.InteractionFlags & kAlternateInteractionMask) == 0) {
			_state.CurrentInteraction = 0;
		} else {
			_state.SecondaryInteraction = 0;
		}

		if ((_state.ControllerOneInput & 0x10) == 0) {
			if (command == 10) {
				int16 lastPage = _state.InteractionMenuItems.empty()
									 ? static_cast<int16>(0)
									 : static_cast<int16>((static_cast<int>(_state.InteractionMenuItems.size()) - 1) >> 2);
				if (!_state.InteractionMenuItems.empty() && lastPage != _state.MenuPage) {
					_state.MenuPage++;
					queueRightActionPrompt(1);
					commitActionPromptUpdate();
				}
			} else if (_state.MenuPage != 0) {
				_state.MenuPage--;
				queueLeftActionPrompt(1);
				commitActionPromptUpdate();
			}

			refreshActionPrompts();
		}
	} else {
		int16 adjustedCommand = command > 4 ? static_cast<int16>(command - 1) : command;
		int menuIndex = _state.MenuPage * 4 + adjustedCommand;
		int16 interaction = menuIndex > static_cast<int>(_state.InteractionMenuItems.size())
								? static_cast<int16>(0)
								: static_cast<int16>(_state.InteractionMenuItems[static_cast<std::size_t>(
														 menuIndex - 1)] +
													 3);
		if ((_state.InteractionFlags & kAlternateInteractionMask) == 0) {
			_state.CurrentInteraction = interaction;
		} else {
			_state.SecondaryInteraction =
				interaction == _state.CurrentInteraction ? static_cast<int16>(0) : interaction;
		}
	}

	_state.MenuCommand = 0;
	return _waitForRoomVerticalBlank();
}

bool ActionMenuController::resolveActionIcon(int16 interaction, int16 &actionIcon) {
	if (interaction == 0) {
		actionIcon = 0;
		return false;
	}

	actionIcon = interaction < 3
					 ? static_cast<int16>(10)
					 : static_cast<int16>(_state.RoomObjects[interaction - 3].ActionIcon);
	return true;
}

void ActionMenuController::composeInventoryGrid() {
	// Ghidra 0x00005D1C-0x00005D51: count through the original sentinel and clamp only a page above the
	// unsigned last-page quotient. The typed list omits the sentinel but retains its word-count result.
	int16 itemCount = static_cast<int16>(_state.InventoryObjectIndices.size());
	if (itemCount != 0) {
		int16 lastPage = static_cast<int16>(static_cast<uint16>(itemCount - 1) >> 2);
		if (lastPage < _state.MenuPage) {
			_state.MenuPage = lastPage;
		}
	}

	int composedSlotCount = 0;

	// Ghidra 0x00005D52-0x00005D87: independently recheck the count, preserve the signed page-start
	// rejection branch, and select the episode's immutable inventory-graphic record table.
	if (itemCount != 0) {
		int16 lastItemIndex = static_cast<int16>(itemCount - 1);
		int16 pageStart = static_cast<int16>(_state.MenuPage << 2);
		if (pageStart <= lastItemIndex) {
			int16 remainingLastIndex = static_cast<int16>(lastItemIndex - pageStart);
			int16 inventoryListIndex = pageStart;
			int graphicTableOffset = static_cast<int>(
				_rom.readUInt32(_state.ActiveEpisodeDescriptorOffset + kEpisodeInventoryGraphicTableField));
			int patternDestinationOffset = 0;

			// Ghidra 0x00005D88-0x00005DF9: consume at least one selected item, stop after its page has
			// no further item or all eight slots are consumed, and retain the zero-graphic no-write branch.
			do {
				int32 roomObjectIndex =
					_state.InventoryObjectIndices[static_cast<std::size_t>(inventoryListIndex++)];
				int16 graphicIndex =
					_state.RoomObjects[static_cast<std::size_t>(roomObjectIndex)].InventoryGraphicIndex;
				if (graphicIndex != 0) {
					int graphicRecordOffset = graphicTableOffset + static_cast<int16>(
																	   static_cast<uint16>(graphicIndex - 1) * kInventoryGraphicRecordSize);
					uint16 graphicAttributes = _rom.readUInt16(graphicRecordOffset);
					CopyBytes(_rom.readBytes(graphicRecordOffset + static_cast<int>(sizeof(uint16)),
											 kInventoryGraphicTileByteCount),
							  _graphicsWorkspace.inventoryPatterns().slice(
								  static_cast<std::size_t>(patternDestinationOffset),
								  static_cast<std::size_t>(kInventoryGraphicTileByteCount)));
					patternDestinationOffset += kInventoryGraphicTileByteCount;

					int slotByteOffset = (composedSlotCount % 4) * kInventorySlotColumnByteCount;
					if (composedSlotCount >= 4) {
						slotByteOffset += kInventorySlotRowByteCount;
					}

					uint16 firstTileCell = static_cast<uint16>(
						(kFirstInventoryTileAttribute + composedSlotCount * kInventoryGraphicTileCount) |
						graphicAttributes);
					for (int row = 0; row < kInventoryIconRowCount; row++) {
						for (int column = 0; column < kInventoryIconColumnCount; column++) {
							uint16 cell = static_cast<uint16>(
								firstTileCell + column * kInventoryIconRowCount + row);
							WriteUInt16BigEndian(MakeSpan(_inventoryCellWorkspace),
												 slotByteOffset + row * kInventoryGridRowByteCount +
													 column * static_cast<int>(sizeof(uint16)),
												 cell);
						}

						WriteUInt16BigEndian(MakeSpan(_inventoryCellWorkspace),
											 slotByteOffset + row * kInventoryGridRowByteCount +
												 kInventoryIconColumnCount * static_cast<int>(sizeof(uint16)),
											 kBlankInventoryCell);
					}
				}

				composedSlotCount++;
				remainingLastIndex = static_cast<int16>(remainingLastIndex - 1);
			} while (remainingLastIndex >= 0 && composedSlotCount != kInventorySlotCount);
		}
	}

	// Ghidra 0x00005DFA-0x00005E49: whether item composition ended by exhaustion or was skipped by an
	// earlier branch, clear every trailing slot as three rows of four cells plus one blank separator cell.
	for (int slot = composedSlotCount; slot < kInventorySlotCount; slot++) {
		int slotByteOffset = (slot % 4) * kInventorySlotColumnByteCount;
		if (slot >= 4) {
			slotByteOffset += kInventorySlotRowByteCount;
		}

		for (int row = 0; row < kInventoryIconRowCount; row++) {
			for (int column = 0; column <= kInventoryIconColumnCount; column++) {
				WriteUInt16BigEndian(MakeSpan(_inventoryCellWorkspace),
									 slotByteOffset + row * kInventoryGridRowByteCount +
										 column * static_cast<int>(sizeof(uint16)),
									 kBlankInventoryCell);
			}
		}
	}
}

void ActionMenuController::commitActionPromptTiles() {
	// Ghidra 0x00005CB6-0x00005CC7: publish all 0x600 staged words at pattern byte address 0xB000.
	_scene.overwritePackedPatternBytes(_graphicsWorkspace.inventoryPatterns(), kInventoryPatternDestinationByteOffset);

	// Ghidra 0x00005CC8-0x00005CE5: publish six complete 20-word rows at AACC, advancing by the
	// original 0x80-byte plane row stride after every write.
	_scene.loadLayerRows(MakeSpan(_inventoryCellWorkspace), TileLayer::Interface,
						 kInventoryGridStartColumn, kInventoryGridStartRow, kInventoryGridColumnCount,
						 kInventoryIconRowCount * 2);

	// Ghidra 0x00005CE6-0x00005CF5: fill every word in the following 64-cell row with literal zero.
	Common::Array<TileCell> blankCells(kInterfaceColumnCount);
	_scene.replaceLayerRow(MakeSpan(blankCells), TileLayer::Interface, 0, kInterfaceClearRow);
}

void ActionMenuController::refreshActionPrompts() {
	// Ghidra 0x00005B92-0x00005BA1: queue left mode two only for page zero; every nonzero page queues zero.
	int16 leftMode = 0;
	if (_state.MenuPage == 0) {
		leftMode = 2;
	}

	queueLeftActionPrompt(leftMode);

	// Ghidra 0x00005BA2-0x00005BC1: scan the complete inclusive object table and count every inventory
	// object whose room ID is exactly one, preserving the original word-width accumulator.
	int16 inventoryObjectCount = 0;
	for (std::size_t index = 0; index < _state.RoomObjects.size(); index++) {
		const RoomObject &roomObject = _state.RoomObjects[index];
		if (roomObject.RoomId == 1) {
			inventoryObjectCount = static_cast<int16>(inventoryObjectCount + 1);
		}
	}

	// Ghidra 0x00005BC2-0x00005BDB: queue right mode two for an empty inventory or the exact final page;
	// the distinct nonfinal-page branch queues zero, and the right call remains unconditional.
	int16 rightMode = 0;
	if (inventoryObjectCount == 0) {
		rightMode = 2;
	} else {
		int16 lastPage = static_cast<int16>(static_cast<uint16>(inventoryObjectCount - 1) >> 2);
		if (lastPage == _state.MenuPage) {
			rightMode = 2;
		}
	}

	queueRightActionPrompt(rightMode);
}

void ActionMenuController::drawSelectedActionIcon(int mode) {
	// Ghidra 0x00005BDC-0x00005BE5: a zero pending identity leaves every interface cell unchanged.
	if (_state.PendingActionIcon == 0) {
		return;
	}

	// Ghidra 0x00005BE6-0x00005C31: retain low-word mode multiplication, linear icon lookup, both row-band
	// branches, and the exact ten-byte horizontal slot stride.
	int iconIndex = 0;
	while (kActionIconByCommand[static_cast<std::size_t>(iconIndex + 1)] != _state.PendingActionIcon) {
		iconIndex++;
	}

	bool lowerBand = iconIndex > 4;
	if (lowerBand) {
		iconIndex -= 5;
	}

	int startColumn = kActionIconFirstColumn + iconIndex * kActionIconColumnStride;
	int startRow = lowerBand ? kActionIconLowerStartRow : kActionIconUpperStartRow;
	int sourceCellByteOffset = (lowerBand ? kActionIconSourceLowerRowByteOffset : kActionIconSourceUpperRowByteOffset) +
							   iconIndex * kActionIconColumnStride * static_cast<int>(sizeof(uint16));
	uint16 modeByteOffset =
		static_cast<uint16>(static_cast<uint16>(mode) * kActionIconModeByteCount);
	int sourceOffset =
		kRoomInterfaceCellTemplatesOffset + static_cast<int16>(sourceCellByteOffset + modeByteOffset);

	// Ghidra 0x00005C32-0x00005C53: publish four cells across two rows, advancing each source and
	// destination row by 0x80 bytes and adding the packed tile attributes to every authored word.
	Common::Array<uint8> packedCells(kActionIconCellColumnCount * kActionIconCellRowCount *
									 sizeof(uint16));
	Span<uint8> packedCellsSpan = MakeSpan(packedCells);
	for (int row = 0; row < kActionIconCellRowCount; row++) {
		for (int column = 0; column < kActionIconCellColumnCount; column++) {
			uint16 packedCell = static_cast<uint16>(
				_rom.readUInt16(sourceOffset + row * kInterfaceColumnCount * static_cast<int>(sizeof(uint16)) +
								column * static_cast<int>(sizeof(uint16))) +
				_state.ActionPromptTileAttributes);
			WriteUInt16BigEndian(packedCellsSpan,
								 (row * kActionIconCellColumnCount + column) * static_cast<int>(sizeof(uint16)),
								 packedCell);
		}
	}

	_scene.loadLayerRows(packedCellsSpan, TileLayer::Interface, startColumn, startRow,
						 kActionIconCellColumnCount, kActionIconCellRowCount);

	// Ghidra 0x00005C54-0x00005C59: native register restoration becomes ordinary managed return.
}

void ActionMenuController::queueLeftActionPrompt(int16 mode) {
	// Ghidra 0x00005C5A-0x00005C63: every nonnegative signed countdown takes the back edge while the
	// installed room callback and shared frame clock perform the original prompt and retrace work.
	while (_state.LeftActionPromptCountdown >= 0) {
		if (!_waitForRoomVerticalBlank()) {
			return;
		}
	}

	// Ghidra 0x00005C64-0x00005C73: start the inclusive countdown at eight, store the exact mode word,
	// and return without changing the right prompt.
	_state.LeftActionPromptCountdown = 8;
	_state.LeftActionPromptIndex = mode;
}

void ActionMenuController::queueRightActionPrompt(int16 mode) {
	// Ghidra 0x00005C74-0x00005C7D: every nonnegative signed countdown takes the back edge while the
	// installed room callback and shared frame clock perform the original prompt and retrace work.
	while (_state.RightActionPromptCountdown >= 0) {
		if (!_waitForRoomVerticalBlank()) {
			return;
		}
	}

	// Ghidra 0x00005C7E-0x00005C8D: start the inclusive countdown at eight, store the exact mode word,
	// and return without changing the left prompt.
	_state.RightActionPromptCountdown = 8;
	_state.RightActionPromptIndex = mode;
}

void ActionMenuController::commitActionPromptUpdate() {
	// Ghidra 0x00005CF6-0x00005D01: retain the actor-zero-only pending-upload loop. The original interrupt
	// advances publication asynchronously, so service one callback-aware clocked frame on each back edge.
	while ((_state.ActorTileUploadPendingFlags & 0x01) != 0) {
		if (!_waitForRoomVerticalBlank()) {
			return;
		}
	}

	// Ghidra 0x00005D02-0x00005D05: compose the complete inventory grid exactly once after actor zero is idle.
	composeInventoryGrid();

	// Ghidra 0x00005D06-0x00005D19: queue publication through display bit five and retain its busy loop until
	// the installed room VBlank owner commits the staged patterns and cells, then clears the same latch.
	_state.DisplayFlags |= kActionPromptPublicationPendingMask;
	while ((_state.DisplayFlags & kActionPromptPublicationPendingMask) != 0) {
		if (!_waitForRoomVerticalBlank()) {
			return;
		}
	}

	// Ghidra 0x00005D1A-0x00005D1B: return only after the pending-publication latch has cleared.
}
} // namespace Scooby