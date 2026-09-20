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

#include "episode_password_menu_controller.h"

#include "common/scummsys.h"
#include "common/algorithm.h"
#include <cstring>

#include "common/array.h"

#include "password_alphabet.h"
#include "password_entry_decoder.h"

namespace Scooby {

MainMenuExit EpisodePasswordMenuController::runSharedEpisodePasswordMenu() {
	// Ghidra 0x00008FCA-0x00008FE9: initialize the episode, select its plane, and present three rows.
	_episodeInitializer.initialize(_state.EpisodeIndex);
	int promptPlaneOffset = _state.EpisodeIndex == 0
								? kEpisodeOneMenuPlaneOffset
								: kEpisodeTwoMenuPlaneOffset;
	Optional<int> selection = _presentMenuAndSelectOption(promptPlaneOffset, kPromptOptionCount);
	if (!selection.hasValue()) {
		return MainMenuExit::HostClosed;
	}

	// Ghidra 0x00008FEA-0x00008FF7: row zero enters the runtime; row two returns to title options.
	if (selection.value() == 0) {
		return _commitSelectedRoom();
	}

	if (selection.value() == 2) {
		return _prepareOptionsAndDispatch();
	}

	// Ghidra 0x00008FF8-0x00009039: stage the grid, drain the pending commit, and rebuild both rows.
	_presentation.stageByteRleData(kPasswordGridMenuPlaneOffset);
	while (_state.RetraceCountdown >= 0) {
		if (!_waitForVerticalBlank()) {
			return MainMenuExit::HostClosed;
		}
	}

	_state.RetraceCountdown = kSelectionCommitCountdown;
	_presentation.commitMenuSceneBuffers();
	_passwordDisplay.refresh(_state);
	_presentation.commitStagedMenuPlane();
	int16 gridColumn = 0;
	int16 gridRow = 0;
	int16 passwordRow = 0;
	int16 passwordPosition = 0;
	while (true) {
		// Ghidra 0x0000903A-0x00009051: compose one five-retrace password-grid update cycle.
		_state.RetraceCountdown = kSelectionCommitCountdown;
		_presentation.composeWideMenuRevealTiles(_presentation.animatedRowMaskOffset(), 0);
		_presentation.composeNarrowMenuRevealTiles(_presentation.animatedRowMaskOffset(), 0);
		while (_state.RetraceCountdown >= 0) {
			// Ghidra 0x00009052-0x0000912D: process buffered directional presses with exact wrapping.
			if (_inputState.acceptBufferedPress(ControllerButtons::Up)) {
				gridRow--;
				if (gridRow < 0) {
					gridRow = kGridRowCount - 1;
				}
			}

			if (_inputState.acceptBufferedPress(ControllerButtons::Down)) {
				gridRow++;
				if (gridRow >= kGridRowCount) {
					gridRow = 0;
				}
			}

			if (_inputState.acceptBufferedPress(ControllerButtons::Right)) {
				gridColumn = static_cast<int16>((gridColumn + 1) & (kGridColumnCount - 1));
			}

			if (_inputState.acceptBufferedPress(ControllerButtons::Left)) {
				gridColumn = static_cast<int16>((gridColumn - 1) & (kGridColumnCount - 1));
			}

			// Ghidra 0x0000912E-0x00009161 and 0x00009184-0x000091C9: dispatch or insert a symbol.
			if (_inputState.wasAnyPressed(kActivationButtonsMask)) {
				if (gridRow == kGridRowCount - 1) {
					Optional<MainMenuExit> exit = runPasswordGridAction(
						gridColumn, passwordPosition, passwordRow);
					if (exit.hasValue()) {
						return exit.value();
					}
				} else {
					_playAudioCommand(0x27);
					_passwordDisplay.insertSymbol(
						passwordRow, passwordPosition,
						kSymbols[gridRow * kGridColumnCount + gridColumn]);
					passwordPosition++;
					if (passwordPosition >= kPasswordPositionCount) {
						passwordPosition = 0;
						passwordRow ^= 1;
					}
				}
			}

			// Ghidra 0x000091CA-0x000091E1: retain the sample and receive one installed-callback retrace.
			_inputState.captureCurrent();
			if (!_waitForVerticalBlank()) {
				return MainMenuExit::HostClosed;
			}
		}

		// Ghidra 0x000091E2-0x00009279: commit, replace terminal sprites 12/13, and publish both rows.
		_presentation.commitMenuSceneBuffers();
		// The g_wSharedMenuTileIndexOrSpriteScratch0008 read at 0x000091E6 is overwritten at
		// 0x000091FC before use.
		int displayPosition = passwordPosition + passwordPosition / 5;
		_presentation.updatePasswordGridCursors(gridColumn, gridRow, displayPosition, passwordRow);
		_presentation.writePriorityTileRow(_passwordDisplay.Rows()[0], kDisplayColumn, kFirstDisplayRow);
		_presentation.writePriorityTileRow(_passwordDisplay.Rows()[1], kDisplayColumn, kFirstDisplayRow + 1);
	}
}

Optional<MainMenuExit> EpisodePasswordMenuController::runPasswordGridAction(
	int16 actionIndex,
	int16 &passwordPosition,
	int16 &passwordRow) {
	// Original executable dispatch table g_apPasswordGridActionHandlers at 0x00009164-0x00009183.
	switch (actionIndex) {
	case 0:
		return movePasswordCursorLeft(passwordPosition);
	case 1:
		return movePasswordCursorRight(passwordPosition);
	case 2:
	case 3:
		return togglePasswordCursorRow(passwordRow);
	case 4:
		return restorePasswordEntryFromCurrentState(passwordPosition, passwordRow);
	case 5:
		return validatePasswordEntry();
	case 6:
		return noOpPasswordGridAction();
	default:
		return exitPasswordGridToMenu();
	}
}

// Handles password-grid action index six as a proved no-op.
//
// Ghidra: noOpPasswordGridAction (0x00009162). The complete body is the sole RTS at
// 0x00009162-0x00009163. Executable dispatch table 0x00009164-0x00009183 selects it only for
// action index six.
Optional<MainMenuExit> EpisodePasswordMenuController::noOpPasswordGridAction() {
	// Ghidra 0x00009162-0x00009163: return without changing caller-owned grid or password state.
	return Optional<MainMenuExit>();
}

// Leaves the password grid and resumes the title-option phase.
//
// Ghidra: exitPasswordGridToMenu (0x0000927A). The contiguous body 0x0000927A-0x00009297 clears
// the link word of sprite-table entry 12 at VRAM 0xFC60, thereby hiding terminal entries 12 and
// 13, before its non-returning branch to RunMainMenu at 0x0000851E. The managed cursor owner
// replaces that VRAM write.
Optional<MainMenuExit> EpisodePasswordMenuController::exitPasswordGridToMenu() {
	// Ghidra 0x0000927A-0x0000927D: receive one installed-callback retrace before changing the cursor table.
	if (!_waitForVerticalBlank()) {
		return Optional<MainMenuExit>(MainMenuExit::HostClosed);
	}

	// Ghidra 0x0000927E-0x00009293: hide sprite entries 12/13, then discard the grid dispatch return address.
	_presentation.updateSelectionCursor(0, Optional<uint16>());

	// Ghidra 0x00009294-0x00009297: tail-transfer to RunMainMenu's option-selection phase.
	return Optional<MainMenuExit>(_prepareOptionsAndDispatch());
}

// Validates the entered password and installs its room and progress state.
//
// Ghidra: validatePasswordEntry (0x00009298). The contiguous body 0x00009298-0x00009381 converts
// 50 symbols into 32 packed bytes, decodes a 29-byte prefix-XOR chain, and preserves the byte
// rotate-through-extend checksum. On success it clears the complete 256-byte progress table before
// copying the recovered state and tail-branching to CommitSelectedRoom at 0x00008CD6.
Optional<MainMenuExit> EpisodePasswordMenuController::validatePasswordEntry() {
	// Ghidra 0x00009298-0x000092D1: map both grouped display rows back to 50 five-bit symbol indices.
	Common::Array<uint8> symbols(static_cast<std::size_t>(kPasswordPositionCount * kPasswordRowCount));
	int destinationIndex = 0;
	for (int rowIndex = 0; rowIndex < kPasswordRowCount; rowIndex++) {
		for (int symbolIndex = 0; symbolIndex < kPasswordPositionCount; symbolIndex++) {
			int displayIndex = symbolIndex + symbolIndex / kSymbolsPerGroup;
			char rowCharacter = _passwordDisplay.Rows()[static_cast<std::size_t>(rowIndex)][static_cast<
				std::size_t>(
				displayIndex)];
			const char *found = std::strchr(kSymbols, rowCharacter);
			symbols[static_cast<std::size_t>(destinationIndex++)] =
				static_cast<uint8>(found - kSymbols);
		}
	}

	// Ghidra 0x000092D2-0x000092F1: rebuild 32 bytes, then invert the original prefix-XOR chain.
	Common::Array<uint8> packedPassword(kPackedPasswordByteCount);
	int bitOffset = 0;
	for (int index = 0; index < static_cast<int>(packedPassword.size()); index++) {
		packedPassword[static_cast<std::size_t>(index)] =
			ReadPasswordByteFromSymbols(MakeSpan(symbols), bitOffset);
	}

	DecodePasswordSymbolXorChain(MakeSpan(packedPassword));

	// Ghidra 0x000092F2-0x0000931B: reproduce the cleared-X rotate chain across all 29 state bytes.
	uint8 roomByte = packedPassword[0];
	uint8 expectedChecksum = packedPassword[1];
	uint8 rollingByte = static_cast<uint8>((roomByte << 2) | (roomByte >> 7));
	bool carry = (roomByte & 0x40) != 0;
	for (int index = 0; index < kPasswordStateByteCount; index++) {
		uint8 previousRollingByte = rollingByte;
		rollingByte = static_cast<uint8>(((rollingByte << 1) | (carry ? 1 : 0)) ^
										 packedPassword[static_cast<std::size_t>(index + 2)]);
		carry = (previousRollingByte & 0x80) != 0;
	}

	if (rollingByte != expectedChecksum) {
		// Ghidra 0x0000931C-0x00009327: reject with command 5 and return to the grid loop.
		_playAudioCommand(5);
		return Optional<MainMenuExit>();
	}

	// Ghidra 0x00009328-0x0000933D: discard the grid return, accept with command 0x59, and wait 61 retraces.
	_playAudioCommand(0x59);
	if (!_waitForFrames(kValidPasswordWaitCounter)) {
		return Optional<MainMenuExit>(MainMenuExit::HostClosed);
	}

	// Ghidra 0x0000933E-0x0000937D: mark password resume and replace the complete progress-table lifetime.
	_state.InitializationFlags |= kPasswordResumeMask;
	Common::fill(_state.ProgressStateBytes.begin(), _state.ProgressStateBytes.end(), 0);
	Common::copy(packedPassword.begin() + 2, packedPassword.begin() + 2 + kPasswordStateByteCount,
			  _state.ProgressStateBytes.begin());
	_state.InitialRoomId = roomByte;

	// Ghidra 0x0000937E-0x00009381: tail-transfer through the existing selected-room commit.
	return Optional<MainMenuExit>(_commitSelectedRoom());
}

// Moves the active password position left with wrap from zero to 24.
//
// Ghidra: movePasswordCursorLeft (0x000093A2). The complete contiguous body is
// 0x000093A2-0x000093B5.
Optional<MainMenuExit> EpisodePasswordMenuController::movePasswordCursorLeft(
	int16 &passwordPosition) {
	// Ghidra 0x000093A2-0x000093AB: issue the movement audio command through its deferred owner.
	_playAudioCommand(0x75);

	// Ghidra 0x000093AC-0x000093B5: subtract as a signed word and wrap a negative result to 24.
	passwordPosition--;
	if (passwordPosition < 0) {
		passwordPosition = kPasswordPositionCount - 1;
	}

	return Optional<MainMenuExit>();
}

// Moves the active password position right with wrap from 24 to zero.
//
// Ghidra: movePasswordCursorRight (0x000093B6). The complete contiguous body is
// 0x000093B6-0x000093CB.
Optional<MainMenuExit> EpisodePasswordMenuController::movePasswordCursorRight(
	int16 &passwordPosition) {
	// Ghidra 0x000093B6-0x000093BF: issue the movement audio command through its deferred owner.
	_playAudioCommand(0x75);

	// Ghidra 0x000093C0-0x000093CB: increment as a word and wrap a signed result above 24 to zero.
	passwordPosition++;
	if (passwordPosition > kPasswordPositionCount - 1) {
		passwordPosition = 0;
	}

	return Optional<MainMenuExit>();
}

// Switches the active password-entry row.
//
// Ghidra: togglePasswordCursorRow (0x000093CC). The complete contiguous body is
// 0x000093CC-0x000093DB; dispatch action indices two and three both select it.
Optional<MainMenuExit> EpisodePasswordMenuController::togglePasswordCursorRow(
	int16 &passwordRow) {
	// Ghidra 0x000093CC-0x000093D5: issue the movement audio command through its deferred owner.
	_playAudioCommand(0x75);

	// Ghidra 0x000093D6-0x000093DB: toggle only bit zero of the caller-owned D5 word.
	passwordRow ^= 1;
	return Optional<MainMenuExit>();
}

// Restores both password rows from current state and resets the entry cursors.
//
// Ghidra: restorePasswordEntryFromCurrentState (0x000093DC). The complete contiguous body is
// 0x000093DC-0x000093F7; PasswordDisplayState::Refresh is the managed BuildPasswordDisplayRows
// boundary.
Optional<MainMenuExit> EpisodePasswordMenuController::restorePasswordEntryFromCurrentState(
	int16 &passwordPosition, int16 &passwordRow) {
	// Ghidra 0x000093DC-0x000093DF: native all-register preservation maps to direct managed ownership.

	// Ghidra 0x000093E0-0x000093ED: play command 0x63, then rebuild both rows from room and progress state.
	_playAudioCommand(0x63);
	_passwordDisplay.refresh(_state);

	// Ghidra 0x000093EE-0x000093F7: restore caller context, then reset original D4 and D5 to zero.
	passwordPosition = 0;
	passwordRow = 0;
	return Optional<MainMenuExit>();
}
} // namespace Scooby