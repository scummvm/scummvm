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

#ifndef SCOOBY_MAIN_MENU_CONTROLLER_H
#define SCOOBY_MAIN_MENU_CONTROLLER_H

#include "common/scummsys.h"

#include "common/array.h"
#include "common/func.h"

#include "main_menu_exit.h"
#include "main_menu_presentation.h"
#include "object_selection/room_object_selection_catalog.h"
#include "object_selection/room_object_selection_controller.h"
#include "passwords/episode_password_menu_controller.h"
#include "passwords/password_display_state.h"
#include "room_selection/room_selection_catalog.h"
#include "room_selection/room_selection_controller.h"
#include "scooby/assets/rom.h"
#include "scooby/common/optional.h"
#include "scooby/input/controller_input.h"
#include "scooby/presentation/frame_presenter.h"
#include "scooby/runtime/episode_initializer.h"
#include "scooby/runtime/runtime_state.h"
#include "scooby/startup/restart_sequence.h"
#include "selection/main_menu_input_state.h"
#include "sound_test/sound_test_catalog.h"
#include "sound_test/sound_test_controller.h"
#include "vblank/main_menu_vblank_handler.h"

// Owns the complete control flow of Ghidra RunMainMenu at 0x00008196 while its meaningful visual,
// selection, and dispatch callees are recovered one at a time. Audio is an explicitly deferred
// silent boundary.
//
// The primary table at 0x000085DC dispatches episode one, episode two, and sound test to
// 0x00008FB8, 0x00008FC2, and 0x0000940C. The alternate table at 0x000085E8 dispatches room
// commit, restart, room selection, and room-object relocation to 0x00008CD6, 0x00008F8A,
// 0x000088A2, and 0x00008C40. Each target remains a distinct managed boundary.

namespace Scooby {

class MainMenuController {
public:
	// Binds menu sequencing to verified assets, shared runtime state, and retrace presentation.
	// rom: verified cartridge address space containing menu assets.
	// menuPresentation: decoded menu assets and logical scene owner.
	// menuVBlank: original callback installed for every main-menu retrace.
	// input: controller sampler shared with the installed menu callback.
	// state: shared state retained across menu and room transitions.
	// presenter: recovered fade and retrace boundary.
	// restartSequence: reset handover shared with other recovered restart callers.
	MainMenuController(const ScoobyDooRom &rom, MainMenuPresentation &menuPresentation,
					   MainMenuVBlankHandler &menuVBlank, ControllerInput &input,
					   RuntimeState &state, FramePresenter &presenter,
					   RestartSequence &restartSequence);

	// Reimplements Ghidra RunMainMenu at 0x00008196 through its selected dispatch target. Audio
	// remains the explicitly deferred nonthrowing menu boundary.
	// beforeMenuRetrace: previously installed callback retained by the leading fade.
	// Returns the room handover, restart request, or host-close outcome selected by the recovered
	// flow.
	//
	// The leading fade retains the callback installed by the preceding phase until the write at
	// 0x00008276 replaces Ghidra g_pVerticalBlankCallback at 0xFF0000. A successful episode prompt
	// returns MainMenuExit::ContinueToRoom through this owner; it does not terminate at a menu-only
	// success state. Ghidra RunAdventure resumes at 0x00000CB8 and performs runtime reset,
	// room-object loading, and actor-runtime initialization.
	MainMenuExit runMainMenu(const Common::Functor0<void> &beforeMenuRetrace);

private:
	enum class DispatchTable {
		Primary,
		Alternate
	};

	static const uint8 kAlternateMenuMask = 0x01;
	static const uint8 kExtendedAlternateMenuMask = 0x02;
	static const int16 kInventoryRoomId = 1;
	static const uint8 kRevealCompleteMask = 0x10;
	static const uint8 kRevealInputMask = 0xF0;
	static const int kMenuOptionSpacing = 0x18;
	static const int kRevealCountdown = 4;
	static const int kRestartMenuAssetOffset = 0x18F58;
	static const int kRestartOptionCount = 4;
	static const int kRoomObjectRelocationMenuAssetOffset = 0x18E08;
	static const int kRoomSelectionMenuAssetOffset = 0x18DD0;
	static const int kSelectionCommitCountdown = 4;

	// Original executable cursor-frame words at 0x00008F14-0x00008F1B.
	static const Common::Array<uint16> kSelectionCursorFrameOffsets;

	bool runMenuReveal();
	bool waitForRevealCommit(bool &skipRemainingReveal);
	MainMenuExit prepareOptionsAndDispatch();
	MainMenuExit presentOptionsAndDispatch();
	bool waitForPendingCommit();

	// Publishes a compressed menu plane, then transfers control to the shared option-selection loop.
	// assetOffset: ROM address of the compressed 32x28 menu plane supplied in A0.
	// optionCount: number of selectable rows preserved from the caller's D0 word.
	// Returns the selected zero-based row, or empty after a host close request.
	//
	// Ghidra: presentMenuAndSelectOption (0x00008D36).
	Optional<int> presentMenuAndSelectOption(int assetOffset, int optionCount);

	// Decodes and publishes one compressed menu plane after exactly one menu retrace.
	// assetOffset: ROM address of the byte-RLE menu plane supplied through original register A0.
	// Returns false when the host requests shutdown during the recovered wait.
	//
	// Ghidra: presentCompressedMenuPlane (0x000096F4-0x00009715).
	bool presentCompressedMenuPlane(int assetOffset);

	MainMenuExit dispatchSelection(DispatchTable dispatchTable, int selection);

	// Runs the active-low menu input loop, unlock sequence, and animated selection cursor recovered
	// from Ghidra SelectMainMenuOption at 0x00008D6A.
	// optionCount: number of vertically spaced entries in the active dispatch table.
	// baseTileIndex: first tile index after the initial ring-LZ asset.
	// Returns the zero-based dispatch-table index, or empty after a host close request.
	//
	// Ghidra: selectMainMenuOption (0x00008D6A).
	Optional<int> selectMainMenuOption(int optionCount, int baseTileIndex);

	bool waitForMainMenuVerticalBlank();
	void invokeMainMenuVBlank();
	bool isRevealAnimationEnabled() const { return (_state.InitializationFlags & kRevealCompleteMask) != 0; }

	// Defers Ghidra PlayAudioCommand at 0x00009762 without blocking menu flow.
	// command: original audio command supplied in register D0.
	void playAudioCommand(int command) { (void)command; }

	// Common::Functor1Mem binding target combining waitForFrames with the installed main-menu VBlank
	// callback that the nested lambda previously captured.
	bool waitForFramesBeforeMainMenuVBlank(int frameCounter);

	// Runs the sound test and resumes the shared options phase at its original tail branch.
	MainMenuExit runSoundTestMenu();

	// Recovered boundary for Ghidra CommitSelectedRoom at 0x00008CD6.
	// selectedRoomId: room selection retained in original scratch word 0xFF000A.
	// Returns the room handover, or host closure during the callback-aware fade.
	//
	// Ghidra: commitSelectedRoom (0x00008CD6-0x00008D35). The callback-aware fade completes before
	// initialization bit zero is set. A changed room clears g_swInitialRoomPositionCoordinateOffset;
	// the selected room is then published and video bit one is cleared. VDP layout and
	// callback-pointer writes become the room-presentation handover owned after this method
	// returns.
	MainMenuExit commitSelectedRoom(int16 selectedRoomId);

	// Common::Functor0Mem binding target for commitSelectedRoom, which needs the currently selected
	// room ID baked in since a Common::Functor0 callback takes no arguments.
	MainMenuExit commitSelectedRoomAtCurrentSelection() { return commitSelectedRoom(_selectedRoomId); }

	// Runs the restart-choice menu and propagates its branch to menu continuation or reset.
	// Returns the selected control-flow handover, including a host close during either recovered
	// wait.
	//
	// Ghidra: confirmRestart (0x00008F8A-0x00008F99). Row zero returns to the shared title option
	// phase at 0x0000851E; rows one through three fall through into RestartApplication. Room
	// continuation, reset, and host closure remain distinct outcomes.
	MainMenuExit confirmRestart();

	// Runs the alternate room chooser and resumes the shared options after a valid selection.
	// Returns the shared menu continuation or a host-close request from either recovered wait
	// owner.
	//
	// Ghidra: runRoomSelectionMenu (0x000088A2-0x000088D7).
	MainMenuExit runRoomSelectionMenu();

	// Relocates one selected room object and resumes the shared main-menu option phase.
	// Returns the shared menu continuation or a host-close request from either catalog selector.
	//
	// Ghidra: runRoomObjectRelocationMenu (0x00008C40), body 0x00008C40-0x00008CD5. Room ID one
	// denotes membership in the ordered inventory object-index list rooted at
	// g_wInventoryObjectIndexListStart (0xFF0A1E). The managed list omits the native terminal -1
	// word while preserving scan, shift, and append order.
	MainMenuExit runRoomObjectRelocationMenu();

	// Defers Ghidra InitializeAudioDriver without blocking recovered menu control flow.
	void initializeAudioDriver() {
	}

	const ScoobyDooRom &_rom;
	MainMenuPresentation &_menuPresentation;
	MainMenuVBlankHandler &_menuVBlank;
	ControllerInput &_input;
	RuntimeState &_state;
	FramePresenter &_presenter;
	RestartSequence &_restartSequence;
	MainMenuInputState _menuInputState;
	PasswordDisplayState _passwordDisplay;
	RoomObjectSelectionCatalog _roomObjectSelectionCatalog;
	RoomSelectionCatalog _roomSelectionCatalog;
	SoundTestCatalog _soundTestCatalog;
	EpisodeInitializer _episodeInitializer;
	Common::Functor0Mem<bool, MainMenuController> _waitForMainMenuVerticalBlankFunctor;
	Common::Functor0Mem<void, MainMenuController> _initializeAudioDriverFunctor;
	Common::Functor1Mem<int, void, MainMenuController> _playAudioCommandFunctor;
	Common::Functor2Mem<int, int, Optional<int>, MainMenuController> _presentMenuAndSelectOptionFunctor;
	Common::Functor0Mem<MainMenuExit, MainMenuController> _prepareOptionsAndDispatchFunctor;
	Common::Functor0Mem<MainMenuExit, MainMenuController> _commitSelectedRoomFunctor;
	Common::Functor1Mem<int, bool, MainMenuController> _waitForFramesBeforeMainMenuVBlankFunctor;
	Common::Functor0Mem<void, MainMenuController> _invokeMainMenuVBlankFunctor;
	RoomObjectSelectionController _roomObjectSelection;
	RoomSelectionController _roomSelection;
	SoundTestController _soundTest;
	EpisodePasswordMenuController _episodePasswordMenu;

	// Menu-specific projection of Ghidra g_wSharedMenuTileIndexOrSpriteScratch0008 (0xFF0008).
	// RunMainMenu replaces it before every selection read.
	int _baseTileIndex{};
	int16 _narrowRevealOffset{};

	// Owns the selected-room scratch word from original RAM 0xFF000A-0xFF000B.
	int16 _selectedRoomId{};
	int16 _wideRevealOffset{};
};
} // namespace Scooby

#endif // SCOOBY_MAIN_MENU_CONTROLLER_H
