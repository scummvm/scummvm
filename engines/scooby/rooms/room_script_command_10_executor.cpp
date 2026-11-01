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
#include "scooby/graphics/genesis_asset_decoder.h"
#include "scooby/interactions/dialogue_text_source.h"

namespace Scooby {

void RoomScriptCommand10Executor::scriptCommand10Dialog(int mode) {
	int commandOffset = _state.roomScriptCurrentOffset;

	// Ghidra 0x00004D80-0x00004D83: scan mode bypasses the complete stateful body and joins the common
	// eight-byte cursor advance.
	if (mode != 0) {
		uint16 commandFlags = _rom.readUInt16(commandOffset + 2);

		// Ghidra 0x00004D84-0x00004DA1: retain the caller's interaction snapshot and both bit-12 outcomes.
		// The managed caller already owns D7; only the hold and interaction bits survive as runtime state.
		if ((commandFlags & 0x1000) != 0) {
			_state.actorAnimationHoldFlags |= kShaggyActorMask;
			_state.interactionFlags |= 0x10;
		}

		int16 previousPositionIndex = 0;

		// Ghidra 0x00004DA2-0x00004E11: preserve the first independent bit-13 branch. Rotating the command
		// word left by two and masking with three selects original bits 14-15 without consuming other flags.
		if ((commandFlags & 0x2000) != 0) {
			int16 temporaryPositionIndex = static_cast<int16>(commandFlags >> 14);
			previousPositionIndex = _state.actorPositionIndices[0];
			_state.actorPositionIndices[0] = temporaryPositionIndex;
			if (!playShaggyActorPositionTransition(previousPositionIndex, temporaryPositionIndex)) {
				return;
			}
		}

		// Ghidra 0x00004E12-0x00004E41: clear existing dialogue, save the complete tile attributes, replace
		// only palette bits 13-14, and regenerate every mask pattern with the encoded low-nibble color.
		_dialogue.clearDialogueText();
		uint16 savedTileAttributes = _state.interfaceTileAttributes;
		_state.interfaceTileAttributes = static_cast<uint16>(
			(savedTileAttributes & kPaletteAttributePreservationMask) | ((commandFlags & 0x0030) << 9));
		generateDialogueMaskTiles(static_cast<uint8>(commandFlags & 0x000F));

		// Ghidra 0x00004E42-0x00004E53: add the command's complete 32-bit displacement to the episode's
		// dialogue base with original longword wrapping, then preserve the presentation boundary.
		uint32 dialogueBaseOffset =
			_rom.readUInt32(_state.activeEpisodeDescriptorOffset + kDialogueBaseField);
		int dialogueOffset = static_cast<int>(dialogueBaseOffset + _rom.readUInt32(commandOffset + 4));
		Optional<SessionExit> requestedExit =
			_dialogue.presentDialogueAndWait(DialogueTextSource(dialogueOffset));
		if (requestedExit.hasValue()) {
			_requestSessionExit(requestedExit.value());
			return;
		}

		// Ghidra 0x00004E54-0x00004E6D: always clear interaction bit four, restore the exact saved tile
		// attributes, and regenerate all mask patterns with the fixed nonzero palette index 15.
		_state.interactionFlags &= 0xEF;
		_state.interfaceTileAttributes = savedTileAttributes;
		generateDialogueMaskTiles(0x0F);

		// Ghidra 0x00004E6E-0x00004EDD: preserve the second independent bit-13 test. Restore the saved
		// position before selecting the reverse matrix entry, then repeat both hold-state waits and restart
		// the restored position's steady animation. Original D7 restoration has no separate managed state.
		if ((commandFlags & 0x2000) != 0) {
			int16 temporaryPositionIndex = _state.actorPositionIndices[0];
			_state.actorPositionIndices[0] = previousPositionIndex;
			if (!playShaggyActorPositionTransition(temporaryPositionIndex, previousPositionIndex)) {
				return;
			}
		}
	}

	// Ghidra 0x00004EDE-0x00004EE3: both modes consume the command, flags, and longword displacement.
	_state.roomScriptCurrentOffset = commandOffset + kRecordSize;
}

void RoomScriptCommand10Executor::generateDialogueMaskTiles(uint8 nonzeroPaletteIndex) {
	Common::Array<uint8> packedTiles = colorise4bppMaskTiles(
		_rom, nonzeroPaletteIndex, 1);
	int firstTileIndex = (_state.interfaceTileAttributes & 0x07FF) + kMaskTileBaseDelta;
	_scene.loadTilesAt(makeSpan(packedTiles), firstTileIndex);
}

bool RoomScriptCommand10Executor::playShaggyActorPositionTransition(int16 previousPositionIndex,
																  int16 nextPositionIndex) {
	_state.actorAnimationOffsets[0] =
		resolveLeadPositionTransition(previousPositionIndex, nextPositionIndex);
	_state.actorAnimationRestartFlags |= kShaggyActorMask;

	while (true) {
		_updateActorAnimationFrames();
		if ((_state.actorAnimationHoldFlags & kShaggyActorMask) == 0) {
			break;
		}

		if (!_waitForRoomVerticalBlank()) {
			return false;
		}
	}

	while (true) {
		_updateActorAnimationFrames();
		if ((_state.actorAnimationHoldFlags & kShaggyActorMask) != 0) {
			break;
		}

		if (!_waitForRoomVerticalBlank()) {
			return false;
		}
	}

	_state.actorAnimationOffsets[0] = static_cast<int16>(_state.actorPositionIndices[0] + 4);
	_state.actorAnimationRestartFlags |= kShaggyActorMask;
	return true;
}
} // namespace Scooby
