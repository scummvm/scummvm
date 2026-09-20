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

#include "room_script_command_10_executor.h"

#include "lead_actor_position_transition_animation_catalog.h"
#include "scooby/common/optional.h"
#include "scooby/common/span.h"
#include "scooby/graphics/binary_mask_tile_generator.h"
#include "scooby/interactions/dialogue_text_source.h"

namespace Scooby {

void RoomScriptCommand10Executor::executeRoomScriptCommand10(int mode) {
	int commandOffset = _state.RoomScriptStartOffset;

	// Ghidra 0x00004D80-0x00004D83: scan mode bypasses the complete stateful body and joins the common
	// eight-byte cursor advance.
	if (mode != 0) {
		uint16 commandFlags = _rom.readUInt16(commandOffset + 2);

		// Ghidra 0x00004D84-0x00004DA1: retain the caller's interaction snapshot and both bit-12 outcomes.
		// The managed caller already owns D7; only the hold and interaction bits survive as runtime state.
		if ((commandFlags & 0x1000) != 0) {
			_state.ActorAnimationHoldFlags |= kLeadActorMask;
			_state.InteractionFlags |= 0x10;
		}

		int16 previousPositionIndex = 0;

		// Ghidra 0x00004DA2-0x00004E11: preserve the first independent bit-13 branch. Rotating the command
		// word left by two and masking with three selects original bits 14-15 without consuming other flags.
		if ((commandFlags & 0x2000) != 0) {
			int16 temporaryPositionIndex = static_cast<int16>(commandFlags >> 14);
			previousPositionIndex = _state.ActorPositionIndices[0];
			_state.ActorPositionIndices[0] = temporaryPositionIndex;
			if (!playLeadActorPositionTransition(previousPositionIndex, temporaryPositionIndex)) {
				return;
			}
		}

		// Ghidra 0x00004E12-0x00004E41: clear existing dialogue, save the complete tile attributes, replace
		// only palette bits 13-14, and regenerate every mask pattern with the encoded low-nibble color.
		_dialogue.clearDialogueText();
		uint16 savedTileAttributes = _state.InterfaceTileAttributes;
		_state.InterfaceTileAttributes = static_cast<uint16>(
			(savedTileAttributes & kPaletteAttributePreservationMask) | ((commandFlags & 0x0030) << 9));
		generateDialogueMaskTiles(static_cast<uint8>(commandFlags & 0x000F));

		// Ghidra 0x00004E42-0x00004E53: add the command's complete 32-bit displacement to the episode's
		// dialogue base with original longword wrapping, then preserve the presentation boundary.
		uint32 dialogueBaseOffset =
			_rom.readUInt32(_state.ActiveEpisodeDescriptorOffset + kDialogueBaseField);
		int dialogueOffset = static_cast<int>(dialogueBaseOffset + _rom.readUInt32(commandOffset + 4));
		Optional<SessionExit> requestedExit =
			_dialogue.presentDialogueAndWait(DialogueTextSource(dialogueOffset));
		if (requestedExit.hasValue()) {
			_requestSessionExit(requestedExit.value());
			return;
		}

		// Ghidra 0x00004E54-0x00004E6D: always clear interaction bit four, restore the exact saved tile
		// attributes, and regenerate all mask patterns with the fixed nonzero palette index 15.
		_state.InteractionFlags &= 0xEF;
		_state.InterfaceTileAttributes = savedTileAttributes;
		generateDialogueMaskTiles(0x0F);

		// Ghidra 0x00004E6E-0x00004EDD: preserve the second independent bit-13 test. Restore the saved
		// position before selecting the reverse matrix entry, then repeat both hold-state waits and restart
		// the restored position's steady animation. Original D7 restoration has no separate managed state.
		if ((commandFlags & 0x2000) != 0) {
			int16 temporaryPositionIndex = _state.ActorPositionIndices[0];
			_state.ActorPositionIndices[0] = previousPositionIndex;
			if (!playLeadActorPositionTransition(temporaryPositionIndex, previousPositionIndex)) {
				return;
			}
		}
	}

	// Ghidra 0x00004EDE-0x00004EE3: both modes consume the command, flags, and longword displacement.
	_state.RoomScriptStartOffset = commandOffset + kRecordSize;
}

void RoomScriptCommand10Executor::generateDialogueMaskTiles(uint8 nonzeroPaletteIndex) {
	Common::Array<uint8> packedTiles = generate(
		_rom, nonzeroPaletteIndex, 1);
	int firstTileIndex = (_state.InterfaceTileAttributes & 0x07FF) + kMaskTileBaseDelta;
	_scene.loadTilesAt(MakeSpan(packedTiles), firstTileIndex);
}

bool RoomScriptCommand10Executor::playLeadActorPositionTransition(int16 previousPositionIndex,
																  int16 nextPositionIndex) {
	_state.ActorAnimationOffsets[0] =
		ResolveLeadPositionTransition(previousPositionIndex, nextPositionIndex);
	_state.ActorAnimationRestartFlags |= kLeadActorMask;

	while (true) {
		_updateActorAnimationFrames();
		if ((_state.ActorAnimationHoldFlags & kLeadActorMask) == 0) {
			break;
		}

		if (!_waitForRoomVerticalBlank()) {
			return false;
		}
	}

	while (true) {
		_updateActorAnimationFrames();
		if ((_state.ActorAnimationHoldFlags & kLeadActorMask) != 0) {
			break;
		}

		if (!_waitForRoomVerticalBlank()) {
			return false;
		}
	}

	_state.ActorAnimationOffsets[0] = static_cast<int16>(_state.ActorPositionIndices[0] + 4);
	_state.ActorAnimationRestartFlags |= kLeadActorMask;
	return true;
}
} // namespace Scooby