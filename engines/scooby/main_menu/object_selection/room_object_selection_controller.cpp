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

#include "room_object_selection_controller.h"

// #include "common/algorithm.h"
#include "common/scummsys.h"

namespace Scooby {

Optional<int16> RoomObjectSelectionController::select() {
	Span<const Common::String> labels = _catalog.getLabels(_state.EpisodeIndex);
	// Original g_wLastRoomObjectIndex at 0xFF06F4 stores N - 1, not the record count.
	int lastObjectIndex = static_cast<int>(_state.RoomObjects.size()) - 1;
	int pageIndex = 0;
	int rowIndex = 0;
	// Ghidra 0x00008BC8-0x00008BCE and 0x00008C0E-0x00008C14 use (last - 1) >> 4.
	int lastPageIndex = (lastObjectIndex - 1) / kPageSize;
	while (true) {
		int lastRowIndex = MIN(kPageSize - 1, lastObjectIndex - pageIndex * kPageSize);
		if (!publishPage(labels, pageIndex, lastRowIndex)) {
			return Optional<int16>();
		}

		while (true) {
			if (!_waitForVerticalBlank()) {
				return Optional<int16>();
			}

			_presentation.updateCatalogSelectionHighlight(rowIndex);
			uint8 currentInput = _state.ControllerOneInput;
			uint8 previousInput = _state.PreviousControllerOneInput;
			if (wasPressed(currentInput, previousInput, ControllerButtons::Up)) {
				if (rowIndex != 0) {
					rowIndex--;
				} else if (pageIndex != 0) {
					rowIndex = kPageSize - 1;
					pageIndex--;
					break;
				}
			}

			if (wasPressed(currentInput, previousInput, ControllerButtons::Down)) {
				if (rowIndex != lastRowIndex) {
					rowIndex++;
				} else if (pageIndex != lastPageIndex) {
					rowIndex = 0;
					pageIndex++;
					break;
				}
			}

			if (wasPressed(currentInput, previousInput, ControllerButtons::A) && pageIndex != 0) {
				rowIndex = 0;
				pageIndex--;
				break;
			}

			if (wasPressed(currentInput, previousInput, ControllerButtons::C) && pageIndex !=
																					 lastPageIndex) {
				rowIndex = 0;
				pageIndex++;
				break;
			}

			if (wasPressed(currentInput, previousInput, ControllerButtons::B)) {
				return Optional<int16>(
					static_cast<int16>(pageIndex * kPageSize + rowIndex));
			}
		}
	}
}

bool RoomObjectSelectionController::publishPage(Span<const Common::String> labels, int pageIndex,
												int lastRowIndex) {
	if (!_waitForVerticalBlank()) {
		return false;
	}

	int firstObjectIndex = pageIndex * kPageSize;
	for (int rowIndex = 0; rowIndex <= lastRowIndex; rowIndex++) {
		_presentation.replacePriorityTileRow(labels[static_cast<std::size_t>(firstObjectIndex + rowIndex)],
											 kDisplayColumn, kFirstDisplayRow + rowIndex);
	}

	return true;
}

bool RoomObjectSelectionController::wasPressed(uint8 currentInput, uint8 previousInput,
											   ControllerButtons button) {
	uint8 mask = static_cast<uint8>(button);
	return (currentInput & mask) == 0 && (previousInput & mask) != 0;
}
} // namespace Scooby