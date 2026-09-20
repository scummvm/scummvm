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

#ifndef SCOOBY_EPISODE_PASSWORD_MENU_CONTROLLER_H
#define SCOOBY_EPISODE_PASSWORD_MENU_CONTROLLER_H

#include "common/scummsys.h"

#include "common/func.h"

#include "password_display_state.h"
#include "scooby/common/optional.h"
#include "scooby/input/controller_buttons.h"
#include "scooby/main_menu/main_menu_exit.h"
#include "scooby/main_menu/main_menu_presentation.h"
#include "scooby/main_menu/selection/main_menu_input_state.h"
#include "scooby/runtime/episode_initializer.h"
#include "scooby/runtime/runtime_state.h"

// Owns episode initialization, its three-choice prompt, and the shared password grid.
//
// Episode one enters the shared body at 0x00008FCA after writing index zero; episode two enters
// from 0x00008FC2 after writing index one. Prompt row zero tail-dispatches to room commit, row one
// opens the password grid, and row two returns to title options. The row-zero result unwinds the
// menu to the session owner so runtime initialization continues with the selected episode identity
// intact.

namespace Scooby {

class EpisodePasswordMenuController {
public:
	// Binds the shared episode prompt to menu, state, input, presentation, and handover owners.
	// episodeInitializer: descriptor-backed episode progress initializer.
	// presentation: logical menu plane, sprite, and text owner.
	// inputState: comparison sample shared with option selection.
	// passwordDisplay: two-row password text retained across menu phases.
	// state: shared episode, progress, controller, and retrace state.
	// presentMenuAndSelectOption: recovered three-choice menu presenter.
	// prepareOptionsAndDispatch: shared title-options tail used by the native row-two branch.
	// commitSelectedRoom: room handover reached by prompt row zero.
	// waitForVerticalBlank: one installed-callback menu retrace with host-close propagation.
	// waitForFrames: a callback-aware recovered multi-retrace wait.
	// playAudioCommand: deferred high-level audio command boundary.
	EpisodePasswordMenuController(EpisodeInitializer &episodeInitializer,
								  MainMenuPresentation &presentation,
								  MainMenuInputState &inputState,
								  PasswordDisplayState &passwordDisplay,
								  RuntimeState &state,
								  Common::Functor2<int, int, Optional<int>> &presentMenuAndSelectOption,
								  Common::Functor0<MainMenuExit> &prepareOptionsAndDispatch,
								  Common::Functor0<MainMenuExit> &commitSelectedRoom,
								  Common::Functor0<bool> &waitForVerticalBlank,
								  Common::Functor1<int, bool> &waitForFrames,
								  Common::Functor1<int, void> &playAudioCommand)
		: _episodeInitializer(episodeInitializer), _presentation(presentation), _inputState(inputState),
		  _passwordDisplay(passwordDisplay), _state(state),
		  _presentMenuAndSelectOption(presentMenuAndSelectOption),
		  _prepareOptionsAndDispatch(prepareOptionsAndDispatch), _commitSelectedRoom(commitSelectedRoom),
		  _waitForVerticalBlank(waitForVerticalBlank), _waitForFrames(waitForFrames),
		  _playAudioCommand(playAudioCommand) {
	}

	// Selects episode one before entering the shared password and room handover.
	// Returns the shared menu flow's room, menu, or host-close handover.
	//
	// Ghidra: runEpisodeOnePasswordMenu (0x00008FB8-0x00008FC1).
	MainMenuExit runEpisodeOnePasswordMenu() {
		// Ghidra 0x00008FB8-0x00008FBF: select the first descriptor-backed episode.
		_state.EpisodeIndex = 0;

		// Ghidra 0x00008FC0-0x00008FC1: skip the episode-two write and enter the shared password flow.
		return runSharedEpisodePasswordMenu();
	}

	// Selects episode two before entering the shared password and room handover.
	// Returns the shared menu flow's room, menu, or host-close handover.
	//
	// Ghidra: runEpisodeTwoPasswordMenu (0x00008FC2-0x00009161, 0x00009184-0x00009279).
	MainMenuExit runEpisodeTwoPasswordMenu() {
		// Ghidra 0x00008FC2-0x00008FC9: select the second descriptor-backed episode.
		_state.EpisodeIndex = 1;
		return runSharedEpisodePasswordMenu();
	}

private:
	static const uint8 kActivationButtonsMask =
		static_cast<uint8>(static_cast<uint8>(ControllerButtons::B) |
						   static_cast<uint8>(ControllerButtons::A) |
						   static_cast<uint8>(ControllerButtons::Start));
	static const int kDisplayColumn = 1;
	static const int kEpisodeOneMenuPlaneOffset = 0x18A56;
	static const int kEpisodeTwoMenuPlaneOffset = 0x18B3C;
	static const int kFirstDisplayRow = 7;
	static const int16 kGridColumnCount = 8;
	static const int16 kGridRowCount = 5;
	static const int kPackedPasswordByteCount = 32;
	static const int16 kPasswordPositionCount = 25;
	static const int kPasswordRowCount = 2;
	static const int kPasswordGridMenuPlaneOffset = 0x18E48;
	static const uint8 kPasswordResumeMask = 0x04;
	static const int kPasswordStateByteCount = 29;
	static const int kPromptOptionCount = 3;
	static const int16 kSelectionCommitCountdown = 4;
	static const int kSymbolsPerGroup = 5;
	static const int kValidPasswordWaitCounter = 0x3C;

	MainMenuExit runSharedEpisodePasswordMenu();
	Optional<MainMenuExit> runPasswordGridAction(int16 actionIndex,
												 int16 &passwordPosition,
												 int16 &passwordRow);
	Optional<MainMenuExit> noOpPasswordGridAction();
	Optional<MainMenuExit> exitPasswordGridToMenu();
	Optional<MainMenuExit> validatePasswordEntry();
	Optional<MainMenuExit> movePasswordCursorLeft(int16 &passwordPosition);
	Optional<MainMenuExit> movePasswordCursorRight(int16 &passwordPosition);
	Optional<MainMenuExit> togglePasswordCursorRow(int16 &passwordRow);
	Optional<MainMenuExit> restorePasswordEntryFromCurrentState(int16 &passwordPosition,
																int16 &passwordRow);

	EpisodeInitializer &_episodeInitializer;
	MainMenuPresentation &_presentation;
	MainMenuInputState &_inputState;
	PasswordDisplayState &_passwordDisplay;
	RuntimeState &_state;
	Common::Functor2<int, int, Optional<int>> &_presentMenuAndSelectOption;
	Common::Functor0<MainMenuExit> &_prepareOptionsAndDispatch;
	Common::Functor0<MainMenuExit> &_commitSelectedRoom;
	Common::Functor0<bool> &_waitForVerticalBlank;
	Common::Functor1<int, bool> &_waitForFrames;
	Common::Functor1<int, void> &_playAudioCommand;
};
} // namespace Scooby

#endif // SCOOBY_EPISODE_PASSWORD_MENU_CONTROLLER_H
