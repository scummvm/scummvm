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
	uint offset = static_cast<uint>(byteOffset);
	uint16 current = static_cast<uint16>((uint16(progressStateBytes[offset]) << 8) |
										 uint16(progressStateBytes[offset + 1]));
	uint16 updated = static_cast<uint16>(current ^ mask);
	progressStateBytes[offset] = static_cast<uint8>(updated >> 8);
	progressStateBytes[offset + 1] = static_cast<uint8>(updated);
}

void RuntimeState::resetRuntimeState(const ScoobyDooRom &rom) {
	// Ghidra 0x0000A27A-0x0000A385: restore runtime globals and the shared half of the fade target.
	menuPage = 0;
	initialRoomPositionCoordinateOffset = 0;
	roomInterfaceHorizontalScroll = 0;
	roomId = 0;
	leadActorMovementPointIndex = -1;
	companionActorMovementPointIndex = -1;
	actorPositionUpdateFlags = 0;
	controllerOneInput = 0xFF;
	previousControllerOneInput = 0xFF;
	// 0x0000A2BA-0x0000A2C9 resets unused controller-two current/previous samples to 0xFF.
	dialogueTimingTableOffset = 4;
	roomCoordinateDataOffset = 0;
	cameraX = 0;
	cameraY = 0;
	randomMoveTimer = 0;
	leftActionPromptIndex = 0;
	rightActionPromptIndex = 0;
	displayFlags = 0;
	interactionFlags = 0;
	roomBehaviorFlags = 0;
	transitionFlags = 0;
	duelStateFlags = 0;
	videoFlags = 0;
	tileStreamingEdgeFlags = 0;
	automaticInteractionFlags = 0;
	roomScriptFlags = 0;
	leftActionPromptCountdown = 0;
	rightActionPromptCountdown = 0;
	// 0x0000A338-0x0000A341 snapshots VDP register 1 byte 0x64 into unread hardware-only RAM.
	foregroundHorizontalScroll = 0;
	backgroundHorizontalScroll = 0;
	foregroundVerticalScroll = 0;
	backgroundVerticalScroll = 0;
	currentInteraction = 0;
	auxiliaryRetraceCountdown = 0;
	roomInterfaceTransitionCountdown = -1;

	Common::Array<PaletteColor> sharedPalette = readPalette(
		rom, 0x32480, 32);
	Common::copy(sharedPalette.begin(), sharedPalette.end(), targetPalette.begin());
}

void RuntimeState::advanceRoomScriptBlock(const ScoobyDooRom &rom) {
	int16 length = rom.readInt16(roomScriptEndOffset);
	roomScriptCurrentOffset = roomScriptEndOffset + static_cast<int>(sizeof(int16));
	roomScriptEndOffset = roomScriptCurrentOffset + length;
}

bool RuntimeState::isButtonDown(ControllerButtons button) const {
	return (controllerOneInput & static_cast<uint8>(button)) == 0;
}

bool RuntimeState::wasButtonDown(ControllerButtons button) const {
	// button was down previously but is now up.
	return (previousControllerOneInput & static_cast<uint8>(button)) == 0 && (controllerOneInput & static_cast<uint8>(button)) != 0;
}

} // namespace Scooby
