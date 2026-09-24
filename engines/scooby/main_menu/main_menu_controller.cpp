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

#include "main_menu_controller.h"

#include "common/scummsys.h"

#include "common/array.h"
#include "common/func.h"

#include "selection/menu_unlock_sequence.h"

namespace Scooby {
const Common::Array<uint16> MainMenuController::kSelectionCursorFrameOffsets{
	0x0000, 0x0010, 0x0020, 0x0010};

MainMenuController::MainMenuController(const ScoobyDooRom &rom, MainMenuPresentation &menuPresentation,
									   MainMenuVBlankHandler &menuVBlank, ControllerInput &input,
									   RuntimeState &state, FramePresenter &presenter,
									   RestartSequence &restartSequence)
	: _rom(rom), _menuPresentation(menuPresentation), _menuVBlank(menuVBlank), _input(input), _state(state),
	  _presenter(presenter), _restartSequence(restartSequence), _menuInputState(state), _passwordDisplay(),
	  _roomObjectSelectionCatalog(rom), _roomSelectionCatalog(rom), _soundTestCatalog(rom),
	  _episodeInitializer(rom, state),
	  _waitForMainMenuVerticalBlankFunctor(this, &MainMenuController::waitForMainMenuVerticalBlank),
	  _initializeAudioDriverFunctor(this, &MainMenuController::initializeAudioDriver),
	  _playAudioCommandFunctor(this, &MainMenuController::playAudioCommand),
	  _presentMenuAndSelectOptionFunctor(this, &MainMenuController::presentMenuAndSelectOption),
	  _prepareOptionsAndDispatchFunctor(this, &MainMenuController::prepareOptionsAndDispatch),
	  _commitSelectedRoomFunctor(this, &MainMenuController::commitSelectedRoomAtCurrentSelection),
	  _waitForFramesBeforeMainMenuVBlankFunctor(this, &MainMenuController::waitForFramesBeforeMainMenuVBlank),
	  _invokeMainMenuVBlankFunctor(this, &MainMenuController::invokeMainMenuVBlank),
	  _roomObjectSelection(_roomObjectSelectionCatalog, menuPresentation, state,
						   _waitForMainMenuVerticalBlankFunctor),
	  _roomSelection(_roomSelectionCatalog, menuPresentation, state, _waitForMainMenuVerticalBlankFunctor),
	  _soundTest(_soundTestCatalog, menuPresentation, input, state, _waitForMainMenuVerticalBlankFunctor,
				 _initializeAudioDriverFunctor, _playAudioCommandFunctor),
	  _episodePasswordMenu(_episodeInitializer, menuPresentation, _menuInputState, _passwordDisplay, state,
						   _presentMenuAndSelectOptionFunctor, _prepareOptionsAndDispatchFunctor,
						   _commitSelectedRoomFunctor, _waitForMainMenuVerticalBlankFunctor,
						   _waitForFramesBeforeMainMenuVBlankFunctor, _playAudioCommandFunctor) {
}

MainMenuExit MainMenuController::runMainMenu(const Common::Functor0<void> &beforeMenuRetrace) {
	// Ghidra 0x00008196-0x00008199: fade before the write at 0x00008276 replaces
	// g_pVerticalBlankCallback at 0xFF0000, retaining the callback installed by the prior phase in this call.
	if (!_presenter.fadePaletteOut(&beforeMenuRetrace)) {
		return MainMenuExit::HostClosed;
	}

	_state.InitializationFlags &= 0xEB;
	_state.VideoFlags &= 0x7F;
	_selectedRoomId = _state.RoomId;
	_wideRevealOffset = 0xE0;
	_narrowRevealOffset = 0xE0;
	_state.VideoFlags &= 0xF7;
	_state.VideoFlags |= 0x10;
	_state.VideoFlags |= 0x02;
	_menuVBlank.reset();
	_baseTileIndex = _menuPresentation.loadInitialAssets();
	_passwordDisplay.refresh(_state);
	if (!_menuPresentation.fadePaletteIn(_invokeMainMenuVBlankFunctor)) {
		return MainMenuExit::HostClosed;
	}

	_state.VideoFlags |= 0x80;
	if (!waitForMainMenuVerticalBlank()) {
		return MainMenuExit::HostClosed;
	}

	if (!runMenuReveal()) {
		return MainMenuExit::HostClosed;
	}

	return prepareOptionsAndDispatch();
}

bool MainMenuController::runMenuReveal() {
	int revealDataPointer = 0x17064;
	for (int remainingFrames = 0x1B; remainingFrames >= 0; remainingFrames--) {
		_state.RetraceCountdown = kRevealCountdown;
		if (remainingFrames == 0x1B || remainingFrames == 0x0D) {
			playAudioCommand(0x0B);
		}

		_menuPresentation.stageByteRleData(static_cast<int>(_rom.readUInt32(revealDataPointer)));
		revealDataPointer += static_cast<int>(sizeof(uint32));
		int16 wideRowMaskOffset;
		int16 wideColumnMaskOffset;
		if (!isRevealAnimationEnabled()) {
			wideRowMaskOffset = _wideRevealOffset;
			wideColumnMaskOffset = _wideRevealOffset;
			if (_wideRevealOffset != 0) {
				_wideRevealOffset -= 0x20;
			}
		} else {
			wideRowMaskOffset = _menuPresentation.animatedRowMaskOffset();
			wideColumnMaskOffset = 0;
		}

		_menuPresentation.composeWideMenuRevealTiles(wideRowMaskOffset, wideColumnMaskOffset);
		if (_wideRevealOffset == 0) {
			int16 narrowRowMaskOffset;
			int16 narrowColumnMaskOffset;
			if (!isRevealAnimationEnabled()) {
				narrowRowMaskOffset = _narrowRevealOffset;
				narrowColumnMaskOffset = _narrowRevealOffset;
				if (_narrowRevealOffset == 0) {
					_state.InitializationFlags |= kRevealCompleteMask;
				} else {
					_narrowRevealOffset -= 0x20;
				}
			} else {
				narrowRowMaskOffset = _menuPresentation.animatedRowMaskOffset();
				narrowColumnMaskOffset = 0;
			}

			_menuPresentation.composeNarrowMenuRevealTiles(narrowRowMaskOffset, narrowColumnMaskOffset);
		}

		bool skipRemainingReveal = false;
		if (!waitForRevealCommit(skipRemainingReveal)) {
			return false;
		}

		if (skipRemainingReveal) {
			break;
		}

		_menuPresentation.commitRevealSceneBuffers(_wideRevealOffset == 0);
		_menuPresentation.applyMenuSpriteVerticalOffset();

		// Ghidra RunMainMenu 0x00008500-0x00008511: publish the staged reveal frame to plane A at C000.
		_menuPresentation.commitStagedMenuPlane();
		_state.RetraceCountdown = kRevealCountdown;
	}

	return true;
}

bool MainMenuController::waitForRevealCommit(bool &skipRemainingReveal) {
	while (_state.RetraceCountdown >= 0) {
		if (!waitForMainMenuVerticalBlank()) {
			skipRemainingReveal = false;
			return false;
		}

		if ((_state.ControllerOneInput & kRevealInputMask) != kRevealInputMask) {
			skipRemainingReveal = true;
			return true;
		}
	}

	skipRemainingReveal = false;
	return true;
}

MainMenuExit MainMenuController::prepareOptionsAndDispatch() {
	// Ghidra RunMainMenu 0x0000851E-0x0000853D: reset both masks and recompose the completed reveal.
	_wideRevealOffset = 0;
	_narrowRevealOffset = 0;
	_state.InitializationFlags |= kRevealCompleteMask;
	_menuPresentation.composeWideMenuRevealTiles(0, 0);
	_menuPresentation.composeNarrowMenuRevealTiles(0, 0);
	return presentOptionsAndDispatch();
}

MainMenuExit MainMenuController::presentOptionsAndDispatch() {
	int assetOffset;
	int optionCount;
	DispatchTable dispatchTable;
	if ((_state.InitializationFlags & kAlternateMenuMask) == 0) {
		assetOffset = 0x1896C;
		optionCount = 3;
		dispatchTable = DispatchTable::Primary;
	} else {
		dispatchTable = DispatchTable::Alternate;
		if ((_state.InitializationFlags & kExtendedAlternateMenuMask) == 0) {
			assetOffset = 0x18BFA;
			optionCount = 2;
		} else {
			assetOffset = 0x18CBE;
			optionCount = 4;
		}
	}

	_menuPresentation.stageByteRleData(assetOffset);
	if (!waitForPendingCommit()) {
		return MainMenuExit::HostClosed;
	}

	_menuPresentation.commitMenuSceneBuffers();

	// Ghidra RunMainMenu 0x0000859C-0x000085AD: publish the staged option frame to plane A at C000.
	_menuPresentation.commitStagedMenuPlane();
	if ((_state.InitializationFlags & kAlternateMenuMask) != 0) {
		_menuPresentation.writeIndexedTileRow(_passwordDisplay.Rows()[0], 1, 7);
		_menuPresentation.writeIndexedTileRow(_passwordDisplay.Rows()[1], 1, 8);
	}

	Optional<int> selection = selectMainMenuOption(optionCount, _baseTileIndex);
	if (!selection.hasValue()) {
		return MainMenuExit::HostClosed;
	}

	return dispatchSelection(dispatchTable, selection.value());
}

bool MainMenuController::waitForPendingCommit() {
	while (_state.RetraceCountdown >= 0) {
		if (!waitForMainMenuVerticalBlank()) {
			return false;
		}
	}

	return true;
}

Optional<int> MainMenuController::presentMenuAndSelectOption(int assetOffset, int optionCount) {
	// Ghidra 0x00008D36-0x00008D41: preserve D0 while decoding the A0 asset into staging RAM.
	_menuPresentation.stageByteRleData(assetOffset);

	// Ghidra 0x00008D42-0x00008D51: drain the installed callback countdown and arm the next commit.
	if (!waitForPendingCommit()) {
		return Optional<int>();
	}

	_state.RetraceCountdown = kSelectionCommitCountdown;

	// Ghidra 0x00008D52-0x00008D67: commit composed tiles and publish the staged complete plane-A map.
	_menuPresentation.commitMenuSceneBuffers();
	_menuPresentation.commitStagedMenuPlane();

	// Ghidra 0x00008D68-0x00008D69: restore D0 and fall through to SelectMainMenuOption at 0x00008D6A.
	return selectMainMenuOption(optionCount, _baseTileIndex);
}

bool MainMenuController::presentCompressedMenuPlane(int assetOffset) {
	_menuPresentation.stageByteRleData(assetOffset);
	if (!waitForMainMenuVerticalBlank()) {
		return false;
	}

	_menuPresentation.commitStagedMenuPlane();
	return true;
}

bool MainMenuController::waitForFramesBeforeMainMenuVBlank(int frameCounter) {
	Common::Functor0Mem<void, MainMenuController> beforeRetrace(this, &MainMenuController::invokeMainMenuVBlank);
	return _presenter.waitForFrames(frameCounter, &beforeRetrace);
}

MainMenuExit MainMenuController::dispatchSelection(DispatchTable dispatchTable, int selection) {
	Common::Functor0Mem<MainMenuExit, EpisodePasswordMenuController> runEpisodeOnePasswordMenuFunctor(
		&_episodePasswordMenu, &EpisodePasswordMenuController::runEpisodeOnePasswordMenu);
	Common::Functor0Mem<MainMenuExit, EpisodePasswordMenuController> runEpisodeTwoPasswordMenuFunctor(
		&_episodePasswordMenu, &EpisodePasswordMenuController::runEpisodeTwoPasswordMenu);
	Common::Functor0Mem<MainMenuExit, MainMenuController> runSoundTestMenuFunctor(
		this, &MainMenuController::runSoundTestMenu);
	Common::Functor0Mem<MainMenuExit, MainMenuController> commitSelectedRoomFunctor(
		this, &MainMenuController::commitSelectedRoomAtCurrentSelection);
	Common::Functor0Mem<MainMenuExit, MainMenuController> confirmRestartFunctor(
		this, &MainMenuController::confirmRestart);
	Common::Functor0Mem<MainMenuExit, MainMenuController> runRoomSelectionMenuFunctor(
		this, &MainMenuController::runRoomSelectionMenu);
	Common::Functor0Mem<MainMenuExit, MainMenuController> runRoomObjectRelocationMenuFunctor(
		this, &MainMenuController::runRoomObjectRelocationMenu);

	Common::Array<Common::Functor0<MainMenuExit> *> targets;
	if (dispatchTable == DispatchTable::Primary) {
		targets.push_back(&runEpisodeOnePasswordMenuFunctor);
		targets.push_back(&runEpisodeTwoPasswordMenuFunctor);
		targets.push_back(&runSoundTestMenuFunctor);
	} else {
		targets.push_back(&commitSelectedRoomFunctor);
		targets.push_back(&confirmRestartFunctor);
		targets.push_back(&runRoomSelectionMenuFunctor);
		targets.push_back(&runRoomObjectRelocationMenuFunctor);
	}

	return (*targets[static_cast<std::size_t>(selection)])();
}

Optional<int> MainMenuController::selectMainMenuOption(int optionCount, int baseTileIndex) {
	// Ghidra 0x00008D6A-0x00008D8D: initialize sequence, cursor, cadence, limit, and input-baseline state.
	MenuUnlockSequence unlockSequence;
	int cursorFrameIndex = 0;
	int selectionOffset = 0;
	int cursorFrameCountdown = 1;
	int lastSelectionOffset = (optionCount - 1) * kMenuOptionSpacing;
	_menuInputState.beginSelection();
	while (true) {
		// Ghidra 0x00008D8E-0x00008E11: stage both menu buffers and advance the original unlock sequence.
		_state.RetraceCountdown = kSelectionCommitCountdown;
		_menuPresentation.composeWideMenuRevealTiles(_menuPresentation.animatedRowMaskOffset(), 0);
		_menuPresentation.composeNarrowMenuRevealTiles(_menuPresentation.animatedRowMaskOffset(), 0);
		while (_state.RetraceCountdown >= 0) {
			uint8 currentInput = _state.ControllerOneInput;
			if ((_state.InitializationFlags & kExtendedAlternateMenuMask) == 0 &&
				unlockSequence.Observe(currentInput)) {
				_state.InitializationFlags |= kExtendedAlternateMenuMask;
				playAudioCommand(3);
			}

			// Ghidra 0x00008E12-0x00008E49: move one 24-pixel row on bounded Up and Down press edges.
			if (_menuInputState.wasPressed(ControllerButtons::Up) && selectionOffset != 0) {
				selectionOffset -= kMenuOptionSpacing;
			}

			if (_menuInputState.wasPressed(ControllerButtons::Down) && selectionOffset !=
																		   lastSelectionOffset) {
				selectionOffset += kMenuOptionSpacing;
			}

			// Ghidra 0x00008E4A-0x00008E69: accept only a newly pressed active-low Start bit.
			if (_menuInputState.wasPressed(ControllerButtons::Start)) {
				// Ghidra 0x00008EDC-0x00008F13: drain, commit, return the row index, and hide terminal sprite 12.
				if (!waitForPendingCommit()) {
					_menuPresentation.updateSelectionCursor(selectionOffset, Optional<uint16>());
					return Optional<int>();
				}

				_menuPresentation.commitMenuSceneBuffers();
				_state.RetraceCountdown = kSelectionCommitCountdown;
				int selection = selectionOffset / kMenuOptionSpacing;
				_menuPresentation.updateSelectionCursor(selectionOffset, Optional<uint16>());
				return Optional<int>(selection);
			}

			// Ghidra 0x00008E6A-0x00008E81: retain the sample, then run the callback poll and countdown.
			_menuInputState.captureCurrent();
			if (!waitForMainMenuVerticalBlank()) {
				_menuPresentation.updateSelectionCursor(selectionOffset, Optional<uint16>());
				return Optional<int>();
			}
		}

		// Ghidra 0x00008E82-0x00008EDB: commit and publish terminal sprite 12 with its two-commit frame cadence.
		_menuPresentation.commitMenuSceneBuffers();
		uint16 cursorAttributes = static_cast<uint16>(
			baseTileIndex + kSelectionCursorFrameOffsets[static_cast<std::size_t>(cursorFrameIndex)]);
		_menuPresentation.updateSelectionCursor(selectionOffset,
												Optional<uint16>(cursorAttributes));
		cursorFrameCountdown--;
		if (cursorFrameCountdown < 0) {
			cursorFrameCountdown = 1;
			cursorFrameIndex = (cursorFrameIndex + 1) % static_cast<int>(kSelectionCursorFrameOffsets.size());
		}
	}
}

bool MainMenuController::waitForMainMenuVerticalBlank() {
	invokeMainMenuVBlank();
	return _presenter.waitForVerticalBlank();
}

void MainMenuController::invokeMainMenuVBlank() {
	_state.DisplayFlags |= 0x01;
	_menuVBlank.handleMainMenuVBlank();
}

MainMenuExit MainMenuController::runSoundTestMenu() {
	// Ghidra 0x00009664 branches back into RunMainMenu at 0x0000851E after C drains the pending frame.
	return _soundTest.run() ? prepareOptionsAndDispatch() : MainMenuExit::HostClosed;
}

MainMenuExit MainMenuController::commitSelectedRoom(int16 selectedRoomId) {
	// Ghidra 0x00008CD6-0x00008CD9: fade while the menu callback remains installed.
	if (!_presenter.fadePaletteOut(&_invokeMainMenuVBlankFunctor)) {
		return MainMenuExit::HostClosed;
	}

	// Ghidra 0x00008CDA-0x00008D07: commit initialization state and the selected room.
	_state.InitializationFlags |= 0x01;
	if (_state.RoomId != selectedRoomId) {
		_state.InitialRoomPositionCoordinateOffset = 0;
	}

	_state.RoomId = selectedRoomId;

	// Ghidra 0x00008D08-0x00008D33: hardware layout writes collapse into the room handover.
	_state.VideoFlags &= 0xFD;
	return MainMenuExit::ContinueToRoom;
}

MainMenuExit MainMenuController::confirmRestart() {
	// Ghidra 0x00008F8A-0x00008F93: present four options from the dedicated restart asset.
	Optional<int> selection = presentMenuAndSelectOption(kRestartMenuAssetOffset, kRestartOptionCount);
	if (!selection.hasValue()) {
		return MainMenuExit::HostClosed;
	}

	// Ghidra 0x00008F94-0x00008F99: row zero branches to RunMainMenu at its 0x0000851E option phase.
	if (selection.value() == 0) {
		return prepareOptionsAndDispatch();
	}

	// Native nonzero rows fall through into RestartApplication at 0x00008F9A.
	return _restartSequence.run(&_invokeMainMenuVBlankFunctor)
			   ? MainMenuExit::RestartRequested
			   : MainMenuExit::HostClosed;
}

MainMenuExit MainMenuController::runRoomSelectionMenu() {
	while (true) {
		// Ghidra 0x000088A2-0x000088B5: present the selector plane and seed its catalog position.
		if (!presentCompressedMenuPlane(kRoomSelectionMenuAssetOffset)) {
			return MainMenuExit::HostClosed;
		}

		Optional<int16> selection = _roomSelection.select(_selectedRoomId);
		if (!selection.hasValue()) {
			return MainMenuExit::HostClosed;
		}

		// Ghidra 0x000088B6-0x000088BB: reserved IDs restart without replacing g_wSelectedRoomId.
		if (selection.value() <= 1) {
			continue;
		}

		// Ghidra 0x000088BC-0x000088D7: retain the room, restore all sprites, and tail-branch to 0x851E.
		_selectedRoomId = selection.value();
		_menuPresentation.restoreMainMenuSpriteLayout();
		return prepareOptionsAndDispatch();
	}
}

MainMenuExit MainMenuController::runRoomObjectRelocationMenu() {
	// Ghidra 0x00008C40-0x00008C5F: restore the 95 binary-mask tiles, publish the complete selector
	// plane, and retain both outcomes of the room-object catalog selection.
	_menuPresentation.restoreBinaryMaskTiles();
	if (!presentCompressedMenuPlane(kRoomObjectRelocationMenuAssetOffset)) {
		return MainMenuExit::HostClosed;
	}

	Optional<int16> selectedObjectIndex = _roomObjectSelection.select();
	if (!selectedObjectIndex.hasValue()) {
		return MainMenuExit::HostClosed;
	}

	// Ghidra 0x00008C60-0x00008C7F: preserve the selected object index and record context while the
	// room catalog receives that object's current room as its initial selection.
	RoomObject &roomObject =
		_state.RoomObjects[static_cast<std::size_t>(selectedObjectIndex.value())];
	Optional<int16> selectedRoomId = _roomSelection.select(roomObject.RoomId);
	if (!selectedRoomId.hasValue()) {
		return MainMenuExit::HostClosed;
	}

	// Ghidra 0x00008C80-0x00008CBF: an unchanged room skips every mutation. Departing room one scans
	// to the exact object and shifts every later index left; entering room one appends at the sentinel.
	if (selectedRoomId.value() != roomObject.RoomId) {
		if (roomObject.RoomId == kInventoryRoomId) {
			std::size_t inventoryIndex = 0;
			while (_state.InventoryObjectIndices[inventoryIndex] != selectedObjectIndex.value()) {
				inventoryIndex++;
			}

			while (inventoryIndex < _state.InventoryObjectIndices.size() - 1) {
				_state.InventoryObjectIndices[inventoryIndex] = _state.InventoryObjectIndices[inventoryIndex +
																							  1];
				inventoryIndex++;
			}

			_state.InventoryObjectIndices.pop_back();
		} else if (selectedRoomId.value() == kInventoryRoomId) {
			_state.InventoryObjectIndices.push_back(selectedObjectIndex.value());
		}

		roomObject.RoomId = selectedRoomId.value();
	}

	// Ghidra 0x00008CC0-0x00008CD5: replace the selector highlight with all 13 immutable sprite records
	// and translate the tail branch to RunMainMenu's shared 0x0000851E phase into its managed owner.
	_menuPresentation.restoreMainMenuSpriteLayout();
	return prepareOptionsAndDispatch();
}
} // namespace Scooby