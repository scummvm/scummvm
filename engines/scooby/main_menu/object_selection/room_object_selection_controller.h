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

#ifndef SCOOBY_ROOM_OBJECT_SELECTION_CONTROLLER_H
#define SCOOBY_ROOM_OBJECT_SELECTION_CONTROLLER_H

#include "common/scummsys.h"

#include "common/func.h"

#include "room_object_selection_catalog.h"
#include "scooby/common/optional.h"
#include "scooby/common/span.h"
#include "scooby/input/controller_buttons.h"
#include "scooby/main_menu/main_menu_presentation.h"
#include "scooby/runtime/runtime_state.h"

// Owns the paged room-object selector used by the hidden object-relocation menu.

namespace Scooby {

class RoomObjectSelectionController {
public:
	// Binds authored object labels and managed runtime state to the installed menu callback.
	// catalog: both decoded episode room-object catalogs.
	// presentation: logical menu text and sprite owner.
	// state: managed room-object and controller state retained by the menu callback.
	// waitForVerticalBlank: one menu retrace, including callback and host-close handling.
	RoomObjectSelectionController(const RoomObjectSelectionCatalog &catalog,
								  MainMenuPresentation &presentation, RuntimeState &state,
								  Common::Functor0<bool> &waitForVerticalBlank)
		: _catalog(catalog), _presentation(presentation), _state(state),
		  _waitForVerticalBlank(waitForVerticalBlank) {
	}

	// Displays the active episode's room-object catalog until B confirms one record.
	// Returns the selected object index, or empty after a host close request.
	//
	// Ghidra: SelectRoomObjectFromCatalog (0x00008AB4-0x00008C3F).
	Optional<int16> select();

private:
	static const int kDisplayColumn = 1;
	static const int kFirstDisplayRow = 7;
	static const int kPageSize = 16;

	bool publishPage(Span<const Common::String> labels, int pageIndex, int lastRowIndex);
	static bool wasPressed(uint8 currentInput, uint8 previousInput,
						   ControllerButtons button);

	const RoomObjectSelectionCatalog &_catalog;
	MainMenuPresentation &_presentation;
	RuntimeState &_state;
	Common::Functor0<bool> &_waitForVerticalBlank;
};
} // namespace Scooby

#endif // SCOOBY_ROOM_OBJECT_SELECTION_CONTROLLER_H
