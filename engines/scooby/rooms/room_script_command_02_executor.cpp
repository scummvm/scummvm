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

#include "room_script_command_02_executor.h"

#include "scooby/interactions/interaction_action.h"
#include "scooby/interactions/interaction_action_registry.h"

namespace Scooby {

void RoomScriptCommand02Executor::executeRoomScriptCommand02(int mode) {
	int32 commandOffset = _state.RoomScriptStartOffset;

	// Ghidra 0x000025AC-0x000025AF: a nonzero execution mode skips registration.
	if (mode == 0) {
		// Ghidra 0x000025B0-0x000025BB: scan mode still skips registration unless bit two is set.
		if ((_state.AutomaticInteractionFlags & kRegistrationEnabledMask) != 0) {
			// Ghidra 0x000025BC-0x000025D3: resolve the episode text base and enforce the signed five-entry
			// bound before any indexed table write.
			uint32 interactionTextBaseOffset =
				_rom.readUInt32(_state.ActiveEpisodeDescriptorOffset + kInteractionTextBaseField);
			if (_state.InteractionActions.count() < InteractionActionRegistry::Capacity) {
				// Ghidra 0x000025D4-0x0000261B: append the two wrapped episode-relative text addresses, signed
				// state value, wrapped word script length, and nested-script start as one entry.
				_state.InteractionActions.registerEntry(InteractionAction(
					static_cast<int32>(interactionTextBaseOffset + _rom.readUInt32(commandOffset + 2)),
					static_cast<int32>(interactionTextBaseOffset + _rom.readUInt32(commandOffset + 6)),
					_rom.readInt16(commandOffset + 12),
					static_cast<int16>(_rom.readUInt16(commandOffset + 10) - kNestedScriptField),
					commandOffset + kNestedScriptField));
			}
		}
	}

	// Ghidra 0x0000261C-0x00002625: every branch rereads and sign-extends record field +0x0A before replacing
	// A5; a zero or backward displacement is therefore preserved without a managed guard.
	_state.RoomScriptStartOffset = commandOffset + _rom.readInt16(commandOffset + 10);
}
} // namespace Scooby