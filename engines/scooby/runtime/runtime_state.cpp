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

#include "runtime_state.h"

// #include "common/algorithm.h"

#include "scooby/graphics/genesis_asset_decoder.h"

namespace Scooby {

void RuntimeState::xorProgressStateWord(int byteOffset, uint16 mask) {
	std::size_t offset = static_cast<std::size_t>(byteOffset);
	uint16 current = static_cast<uint16>((uint16(ProgressStateBytes[offset]) << 8) |
										 uint16(ProgressStateBytes[offset + 1]));
	uint16 updated = static_cast<uint16>(current ^ mask);
	ProgressStateBytes[offset] = static_cast<uint8>(updated >> 8);
	ProgressStateBytes[offset + 1] = static_cast<uint8>(updated);
}

void RuntimeState::resetRuntimeState(const ScoobyDooRom &rom) {
	// Ghidra 0x0000A27A-0x0000A385: restore runtime globals and the shared half of the fade target.
	MenuPage = 0;
	InitialRoomPositionCoordinateOffset = 0;
	RoomInterfaceHorizontalScroll = 0;
	RoomId = 0;
	LeadActorMovementPointIndex = -1;
	CompanionActorMovementPointIndex = -1;
	ActorPositionUpdateFlags = 0;
	ControllerOneInput = 0xFF;
	PreviousControllerOneInput = 0xFF;
	// 0x0000A2BA-0x0000A2C9 resets unused controller-two current/previous samples to 0xFF.
	DialogueTimingTableOffset = 4;
	RoomCoordinateDataOffset = 0;
	CameraX = 0;
	CameraY = 0;
	RandomMoveTimer = 0;
	LeftActionPromptIndex = 0;
	RightActionPromptIndex = 0;
	DisplayFlags = 0;
	InteractionFlags = 0;
	RoomBehaviorFlags = 0;
	TransitionFlags = 0;
	DuelStateFlags = 0;
	VideoFlags = 0;
	TileStreamingEdgeFlags = 0;
	AutomaticInteractionFlags = 0;
	RoomScriptFlags = 0;
	LeftActionPromptCountdown = 0;
	RightActionPromptCountdown = 0;
	// 0x0000A338-0x0000A341 snapshots VDP register 1 byte 0x64 into unread hardware-only RAM.
	ForegroundHorizontalScroll = 0;
	BackgroundHorizontalScroll = 0;
	ForegroundVerticalScroll = 0;
	BackgroundVerticalScroll = 0;
	CurrentInteraction = 0;
	AuxiliaryRetraceCountdown = 0;
	RoomInterfaceTransitionCountdown = -1;

	Common::Array<PaletteColor> sharedPalette = readPalette(
		rom, 0x32480, 32);
	Common::copy(sharedPalette.begin(), sharedPalette.end(), TargetPalette.begin());
}

void RuntimeState::advanceRoomScriptBlock(const ScoobyDooRom &rom) {
	int16 length = rom.readInt16(RoomScriptEndOffset);
	RoomScriptStartOffset = RoomScriptEndOffset + static_cast<int>(sizeof(int16));
	RoomScriptEndOffset = RoomScriptStartOffset + length;
}
} // namespace Scooby