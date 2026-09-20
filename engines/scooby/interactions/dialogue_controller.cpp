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

#include "dialogue_controller.h"

#include "common/scummsys.h"

#include "scooby/common/contracts.h"
#include "scooby/graphics/genesis_asset_decoder.h"
#include "scooby/graphics/tile_layer.h"

namespace Scooby {
const Common::Array<uint8> DialogueController::kDialogueCountdownStepBytes{
	0x00, 0x0A, 0x00, 0x05, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00};

void DialogueController::drawDialogueText(const DialogueTextSource &source) {
	// Ghidra 0x0000A054-0x0000A06B: the 0x3FF DBF loop writes all 1,024 Window cells. One logical fill
	// replaces the hardware address setup and individually interrupt-masked WriteSingleVramWord calls.
	_scene.fillWindow(decodeTileCell(_state.BlankTileCell));

	// Ghidra 0x0000A06C-0x0000A07D: preserve the distinct measurement boundary, raw-length publication,
	// first text-row address, and initial two-row Window boundary.
	int16 remainingLength = measureNullTerminatedString(source);
	_state.TextDisplayCountdown = remainingLength;
	int sourceIndex = 0;
	int row = 1;
	uint8 windowPositionBits = kInitialWindowRowCount;

	// Ghidra 0x0000A07E-0x0000A0DF: wrap every signed length greater than 30. Search backward from byte 30
	// for a space or a hyphen one byte ahead without adding a managed fallback for delimiter-free text.
	while (remainingLength > kCenteringWidth) {
		int16 distanceFromWidth = 0;
		int searchIndex = kCenteringWidth + 1;
		while (source.readByte(_rom, sourceIndex + --searchIndex) != kSpace &&
			   source.readByte(_rom, sourceIndex + searchIndex + 1) != kHyphen) {
			distanceFromWidth = static_cast<int16>(distanceFromWidth + 1);
		}

		int16 lineLength = static_cast<int16>(
			-static_cast<int16>(distanceFromWidth - kCenteringWidth));
		remainingLength = static_cast<int16>(remainingLength - lineLength);
		writeCenteredLine(source, sourceIndex, lineLength, row);
		row++;
		windowPositionBits = static_cast<uint8>(windowPositionBits + 1);
	}

	// Ghidra 0x0000A0E0-0x0000A10F: zero skips the final write loop; every other signed-word bit pattern
	// reaches the same DBF writer without a managed positivity guard.
	if (remainingLength != 0) {
		writeCenteredLine(source, sourceIndex, remainingLength, row);
	}

	// Ghidra 0x0000A110-0x0000A11F: publish the complete low-byte register-18 value. Logical clipping
	// replaces the hardware register write while retaining both its low-five-bit boundary and bit-seven side.
	_state.WindowVerticalPositionBits = windowPositionBits;
	_scene.setWindowVerticalPosition(windowPositionBits);

	// Ghidra 0x0000A120-0x0000A12F: reload the originally measured length, shift its word left four,
	// and retain the wrapped countdown used by dialogue dismissal.
	_state.TextDisplayCountdown = static_cast<int16>(_state.TextDisplayCountdown << 4);
}

int16 DialogueController::drawTimedInteractionText(const DialogueTextSource &source, int16 row,
												   int16 horizontalOffset) {
	// Ghidra 0x00009FA2-0x00009FBF: preserve caller registers, form the wrapping interface destination,
	// measure without consuming the source, start at one line, and retain the signed width comparison.
	uint16 destinationAddress = static_cast<uint16>(
		kInteractionFirstTextCellAddress + (row << 7) + (horizontalOffset << 1));
	int16 remainingLength = measureNullTerminatedString(source);
	int sourceIndex = 0;
	int16 lineCount = 1;

	// Ghidra 0x00009FC4-0x0000A017: for every signed length above 30, search backward for the original
	// space/hyphen break, write exactly that many cells, and advance one complete interface row.
	while (remainingLength > kInteractionLineWidth) {
		int16 distanceFromWidth = 0;
		int searchIndex = kInteractionLineWidth + 1;
		while (source.readByte(_rom, sourceIndex + --searchIndex) != kSpace &&
			   source.readByte(_rom, sourceIndex + searchIndex + 1) != kHyphen) {
			distanceFromWidth = static_cast<int16>(distanceFromWidth + 1);
		}

		int16 lineLength = static_cast<int16>(
			-static_cast<int16>(distanceFromWidth - kInteractionLineWidth));
		remainingLength = static_cast<int16>(remainingLength - lineLength);
		writeInteractionLine(source, sourceIndex, lineLength, destinationAddress);
		destinationAddress = static_cast<uint16>(destinationAddress + kInterfaceRowByteStride);
		lineCount = static_cast<int16>(lineCount + 1);
	}

	// Ghidra 0x0000A018-0x0000A041: zero skips the final loop; every other signed-word bit pattern uses
	// the same DBF-shaped writer without adding a managed positivity guard.
	if (remainingLength != 0) {
		writeInteractionLine(source, sourceIndex, remainingLength, destinationAddress);
	}

	// Ghidra 0x0000A042-0x0000A053: return the line-count word and publish the low word of its unsigned
	// multiplication by 300 after restoring the original caller-owned register context.
	_state.TextDisplayCountdown = static_cast<int16>(static_cast<uint16>(lineCount) * 300);
	return lineCount;
}

void DialogueController::writeInteractionLine(const DialogueTextSource &source, int &sourceIndex,
											  int16 lineLength, uint16 destinationAddress) {
	int16 characterCounter = static_cast<int16>(lineLength - 1);
	do {
		uint16 packedCell = static_cast<uint16>(_state.InterfaceTileAttributes +
												source.readByte(_rom, sourceIndex++));
		uint16 cellOffset = static_cast<uint16>(
			static_cast<uint16>(destinationAddress - kInterfaceNameTableAddress) /
			static_cast<int>(sizeof(uint16)));
		_scene.fillLayerRows(decodeTileCell(packedCell),
							 TileLayer::Interface,
							 cellOffset % kInterfacePlaneColumnCount, cellOffset / kInterfacePlaneColumnCount,
							 1, 1);
		destinationAddress = static_cast<uint16>(destinationAddress + sizeof(uint16));
		characterCounter = static_cast<int16>(characterCounter - 1);
	} while (characterCounter != -1);
}

void DialogueController::writeCenteredLine(const DialogueTextSource &source, int &sourceIndex,
										   int16 lineLength, int row) {
	uint16 destinationAddress = static_cast<uint16>(
		kFirstTextCellAddress + (row - 1) * kWindowRowByteStride + kCenteringWidth - (lineLength & 0xFFFE));
	int16 characterCounter = static_cast<int16>(lineLength - 1);
	do {
		uint16 packedCell = static_cast<uint16>(_state.InterfaceTileAttributes +
												source.readByte(_rom, sourceIndex++));
		uint16 cellOffset = static_cast<uint16>(
			static_cast<uint16>(destinationAddress - kWindowNameTableAddress) /
			static_cast<int>(sizeof(uint16)));
		_scene.setWindowCell(decodeTileCell(packedCell), cellOffset % 32,
							 cellOffset / 32);
		destinationAddress = static_cast<uint16>(destinationAddress + sizeof(uint16));
		characterCounter = static_cast<int16>(characterCounter - 1);
	} while (characterCounter != -1);
}

int16 DialogueController::measureNullTerminatedString(const DialogueTextSource &source) {
	// Ghidra 0x0000A236-0x0000A239: preserve A0 and clear D0. Passing the source value keeps the caller's
	// logical pointer unchanged while the managed index owns this function's temporary traversal.
	int16 length = 0;
	int sourceIndex = 0;

	// Ghidra 0x0000A23A-0x0000A243: test and advance every byte, incrementing only the low word for each
	// nonzero value. The unchecked cast retains addq.w wrapping instead of widening the original count.
	while (source.readByte(_rom, sourceIndex++) != 0) {
		length = static_cast<int16>(length + 1);
	}

	// Ghidra 0x0000A244-0x0000A247: restoring A0 is inherent in the value input; return the exact D0 word.
	return length;
}

void DialogueController::clearDialogueText() {
	// Ghidra 0x0000A172-0x0000A181: replacing the shared workspace's first byte with NUL becomes an empty
	// lifetime-specific value supplied to the distinct draw boundary before returning with all its side effects.
	drawDialogueText(DialogueTextSource(Common::String()));
}

Optional<SessionExit> DialogueController::presentDialogueAndWait(
	const DialogueTextSource &source) {
	// Ghidra 0x0000A130-0x0000A13B: retain only the caller's display bit three for later OR restoration.
	uint8 dialogueDisplayFlag = static_cast<uint8>(_state.DisplayFlags & kDialogueDisplayMask);

	// Ghidra 0x0000A13C-0x0000A14F: clear that bit, then preserve all three distinct original boundaries
	// in order. A managed non-returning dismissal bypasses every cleanup and restoration below.
	_state.DisplayFlags &= static_cast<uint8>(~kDialogueDisplayMask);
	drawDialogueText(source);
	Optional<SessionExit> requestedExit = waitForDialogueDismissal();
	if (requestedExit.hasValue()) {
		return requestedExit;
	}

	clearDialogueText();

	// Ghidra 0x0000A150-0x0000A15F: replace the VDP register-18 write with the same logical Window
	// clipping byte and publish the recovered RAM mirror after the scene update.
	_scene.setWindowVerticalPosition(kInitialWindowRowCount);
	_state.WindowVerticalPositionBits = kInitialWindowRowCount;

	// Ghidra 0x0000A160-0x0000A167: OR only the saved bit into the callback-updated display byte.
	_state.DisplayFlags |= dialogueDisplayFlag;

	// Ghidra 0x0000A168-0x0000A171: request an interaction-display refresh and return normally.
	_state.InteractionFlags |= kInteractionRefreshMask;
	return Optional<SessionExit>();
}

Optional<SessionExit> DialogueController::waitForDialogueDismissal() {
	// Ghidra 0x0000A18C-0x0000A191: logically shift the complete unsigned countdown word once.
	_state.TextDisplayCountdown = static_cast<int16>(
		static_cast<uint16>(_state.TextDisplayCountdown) >> 1);

	// Ghidra 0x0000A192-0x0000A1A9: A release has priority over the B release test. Native interrupts
	// updated input during these spins; each managed continuation services that asynchronous boundary.
	while (true) {
		while ((_state.ControllerOneInput & kActionButtonMask) == 0) {
			if (!waitForRoomRetrace()) {
				return Optional<SessionExit>(SessionExit::HostClosed);
			}
		}

		if ((_state.ControllerOneInput & kAlternateActionButtonMask) != 0) {
			break;
		}

		if (!waitForRoomRetrace()) {
			return Optional<SessionExit>(SessionExit::HostClosed);
		}
	}

	while (true) {
		// Ghidra 0x0000A1AA-0x0000A1B1: both original services run on every complete loop iteration.
		// A nested managed exit from automatic processing bypasses all subsequent native work.
		_updateActorAnimationFrames();
		SDM_ASSERT(_processAutomaticInteractions != nullptr,
				   "DialogueController processAutomaticInteractions callback is not bound.");
		Optional<SessionExit> automaticInteractionExit = (*_processAutomaticInteractions)();
		if (automaticInteractionExit.hasValue()) {
			return automaticInteractionExit;
		}

		// Ghidra 0x0000A1B2-0x0000A1C9: only the conjunction of room-script bits one and zero returns.
		if ((_state.RoomScriptFlags & 0x02) != 0 && (_state.RoomScriptFlags & 0x01) != 0) {
			return Optional<SessionExit>();
		}

		// Ghidra 0x0000A1CA-0x0000A1E7: retain both independent gates around random animation selection.
		if ((_state.InteractionFlags & kDialogueInteractionAnimationMask) != 0 &&
			(_state.ActorAnimationHoldFlags & kActorZeroHoldMask) != 0) {
			SDM_ASSERT(_chooseRandomInteractionAnimation != nullptr,
					   "DialogueController chooseRandomInteractionAnimation callback is not bound.");
			(*_chooseRandomInteractionAnimation)();
		}

		// Ghidra 0x0000A1E8-0x0000A1EB: replace the native VBlank handshake with the installed room
		// callback followed by the host presentation boundary.
		if (!waitForRoomRetrace()) {
			return Optional<SessionExit>(SessionExit::HostClosed);
		}

		// Ghidra 0x0000A1EC-0x0000A1FF: sign-extend the word byte offset, read the selected big-endian word,
		// publish wrapping subtraction, and return only when the original unsigned operation borrowed.
		int16 timingOffset = _state.DialogueTimingTableOffset;
		uint16 countdownStep = static_cast<uint16>(
			(kDialogueCountdownStepBytes[static_cast<std::size_t>(timingOffset)] << 8) |
			kDialogueCountdownStepBytes[static_cast<std::size_t>(timingOffset + 1)]);
		uint16 countdown = static_cast<uint16>(_state.TextDisplayCountdown);
		_state.TextDisplayCountdown = static_cast<int16>(countdown - countdownStep);
		if (countdown < countdownStep) {
			return Optional<SessionExit>();
		}

		// Ghidra 0x0000A200-0x0000A21B: A has priority. A held waits for its release and then returns;
		// A released alone reaches the distinct B path.
		if ((_state.ControllerOneInput & kActionButtonMask) == 0) {
			while ((_state.ControllerOneInput & kActionButtonMask) == 0) {
				if (!waitForRoomRetrace()) {
					return Optional<SessionExit>(SessionExit::HostClosed);
				}
			}

			return Optional<SessionExit>();
		}

		// Ghidra 0x0000A21C-0x0000A233: released B repeats the complete service loop. Held B waits for
		// release and returns, preserving the branch that differs from the held-A path above.
		if ((_state.ControllerOneInput & kAlternateActionButtonMask) != 0) {
			continue;
		}

		while ((_state.ControllerOneInput & kAlternateActionButtonMask) == 0) {
			if (!waitForRoomRetrace()) {
				return Optional<SessionExit>(SessionExit::HostClosed);
			}
		}

		// Ghidra 0x0000A234-0x0000A235: every normal dismissal path converges on the original return.
		return Optional<SessionExit>();
	}
}

bool DialogueController::waitForRoomRetrace() {
	_handleRoomVBlank();
	return _presenter.waitForVerticalBlank();
}
} // namespace Scooby