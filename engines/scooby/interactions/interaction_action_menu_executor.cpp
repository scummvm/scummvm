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

#include "interaction_action_menu_executor.h"

#include "common/array.h"

#include "dialogue_text_source.h"
#include "interaction_action_registry.h"
#include "scooby/common/span.h"
#include "scooby/graphics/binary_mask_tile_generator.h"
#include "scooby/graphics/genesis_asset_decoder.h"
#include "scooby/graphics/tile_layer.h"

namespace Scooby {

InteractionActionMenuExecutor::InteractionActionMenuExecutor(
	const ScoobyDooRom &rom, TileScene &scene,
	SharedGraphicsWorkspace &graphicsWorkspace,
	RuntimeState &state, FramePresenter &presenter,
	Common::Functor0<void> &updateActorAnimationFrames, Common::Functor0<void> &handleRoomVBlank,
	Common::Functor0<void> &chooseRandomInteractionAnimation,
	Common::Functor0<Optional<SessionExit>> &runRoomScriptInitializationCommands,
	Common::Functor0<Optional<SessionExit>> &runRoomScriptCommands, DialogueController &dialogue,
	Common::Functor0<void> &commitActionPromptUpdate, Common::Functor0<void> &refreshActionPrompts)
	: _rom(rom), _scene(scene), _graphicsWorkspace(graphicsWorkspace), _state(state), _presenter(presenter),
	  _updateActorAnimationFrames(updateActorAnimationFrames), _handleRoomVBlank(handleRoomVBlank),
	  _chooseRandomInteractionAnimation(chooseRandomInteractionAnimation),
	  _runRoomScriptInitializationCommands(runRoomScriptInitializationCommands),
	  _runRoomScriptCommands(runRoomScriptCommands), _dialogue(dialogue),
	  _commitActionPromptUpdate(commitActionPromptUpdate), _refreshActionPrompts(refreshActionPrompts) {
}

Optional<SessionExit> InteractionActionMenuExecutor::runInteractionActionMenu() {
	// Ghidra 0x00001C0C-0x00001C1B: preserve the first independent registration scan and the immediate
	// zero-action return before changing any interface, interaction, or script-frame state.
	Optional<SessionExit> initialScanExit = _runRoomScriptInitializationCommands();
	if (initialScanExit.hasValue()) {
		return initialScanExit;
	}

	if (_state.InteractionActions.count() == 0) {
		return Optional<SessionExit>();
	}

	// Ghidra 0x00001C1C-0x00001C39: clear the Window through its original draw boundary, hide the strip,
	// and set only interaction bit two before waiting for any existing transition.
	_dialogue.drawDialogueText(DialogueTextSource(Common::String()));
	_state.InteractionStripColumn = -1;
	_state.InteractionFlags |= kInteractionMenuActiveMask;

	// Ghidra 0x00001C40-0x00001C69: the native spin observes an interrupt-owned signed byte. Managed
	// continuations service that callback, then preserve both display-side branches after clearing bit three.
	while (_state.RoomInterfaceTransitionCountdown >= 0) {
		if (!waitForRoomRetrace()) {
			return Optional<SessionExit>(SessionExit::HostClosed);
		}
	}

	_state.DisplayFlags &= 0xF7;
	bool savedBlockOnRight = (_state.DisplayFlags & kDisplaySideMask) == 0;
	int interfaceStartColumn = savedBlockOnRight ? kInterfaceBlockColumnCount : 0;

	// Ghidra 0x00001C6A-0x00001C8B: replace all 192 ReadSingleVramWord calls with the same six complete
	// 32-cell rows and 0x80-byte row stride in the logical interface layer.
	_scene.copyPackedLayerRows(_graphicsWorkspace.interfaceTileBlock(), TileLayer::Interface,
							   interfaceStartColumn, kInterfaceBlockStartRow, kInterfaceBlockColumnCount,
							   kInterfaceBlockRowCount);

	// Ghidra 0x00001C8C-0x00001CA7: retain the distinct fill boundary, then update animations before every
	// signed-countdown test until the installed room callback advances the value below zero.
	fillInteractionTextWindow(interfaceStartColumn);
	if (!runInterfaceTransitionCountdown()) {
		return Optional<SessionExit>(SessionExit::HostClosed);
	}

	// Ghidra 0x00001CA8-0x00001CC3: the initial native saved pointers, depth zero, and cleared used word
	// become one typed base frame. The shared line counts retain their process lifetime across every frame.
	InteractionActionMenuFrame currentFrame(_state.RoomScriptStartOffset, _state.RoomScriptEndOffset, 0);
	Common::Array<InteractionActionMenuFrame> parentFrames;
	Common::Array<int16> &lineCounts = _state.InteractionActionLineCounts;

	while (true) {
		// Ghidra 0x00001CC4-0x00001CE5: restore the current frame's full script window, rescan it without
		// consuming that window, and preserve the zero-count branch that unwinds exactly one frame.
		_state.RoomScriptStartOffset = currentFrame.ScriptStartOffset;
		_state.RoomScriptEndOffset = currentFrame.ScriptEndOffset;
		Optional<SessionExit> frameScanExit = _runRoomScriptInitializationCommands();
		if (frameScanExit.hasValue()) {
			return frameScanExit;
		}

		if (_state.InteractionActions.count() == 0) {
			if (parentFrames.empty()) {
				break;
			}

			currentFrame = parentFrames.back();
			parentFrames.pop_back();
			continue;
		}

		// Ghidra 0x00001CE6-0x00001D21: fill the selected interface half, regenerate its six-palette mask,
		// and retain the release loop until both active-low action buttons are simultaneously released.
		fillInteractionTextWindow(interfaceStartColumn);
		_state.InterfaceTileAttributes &= kInterfacePaletteClearMask;
		publishBinaryMaskTiles(6, 0, _state.InterfaceTileAttributes);
		while ((_state.ControllerOneInput & kActionButtonMask) == 0 ||
			   (_state.ControllerOneInput & kAlternateActionButtonMask) == 0) {
			if (!waitForRoomRetrace()) {
				return Optional<SessionExit>(SessionExit::HostClosed);
			}
		}

		// Ghidra 0x00001D22-0x00001D71: draw each registered unused entry in order, retain line counts for
		// used entries, and stop on either the complete signed count or the five-row boundary.
		int16 visibleRow = 0;
		int16 remainingActions = static_cast<int16>(_state.InteractionActions.count());
		int actionIndex = 0;
		while (true) {
			if ((currentFrame.UsedFlags & (1 << actionIndex)) == 0) {
				int16 horizontalOffset = (_state.DisplayFlags & kDisplaySideMask) == 0
											 ? static_cast<int16>(0)
											 : static_cast<int16>(0x20);
				lineCounts[actionIndex] = _dialogue.drawTimedInteractionText(
					DialogueTextSource(_state.InteractionActions[actionIndex].LabelTextOffset), visibleRow,
					horizontalOffset);
				visibleRow = static_cast<int16>(visibleRow + lineCounts[actionIndex]);
			}

			actionIndex++;
			remainingActions = static_cast<int16>(remainingActions - 1);
			if (remainingActions <= 0 || visibleRow >= InteractionActionRegistry::Capacity) {
				break;
			}
		}

		// Ghidra 0x00001D72-0x00001DA9: publish the initial strip geometry, then scan all five flag bits
		// from four down to zero without constraining the scan to the current registered count.
		int16 selectedRow = 0;
		_state.InteractionStripHeightAttributes = static_cast<uint16>((lineCounts[0] - 1) << 8);
		_state.InteractionStripRow = 0;
		_state.InteractionStripColumn = 0;
		int16 selectedIndex = remainingActions;
		for (int candidate = InteractionActionRegistry::Capacity - 1; candidate >= 0; candidate--) {
			if ((currentFrame.UsedFlags & (1 << candidate)) == 0) {
				selectedIndex = static_cast<int16>(candidate);
			}
		}

		while (true) {
			// Ghidra 0x00001DAA-0x00001E57: request one strip publication, update actor frames, preserve
			// both edge-triggered direction branches and every used-entry skip, then wait for bit zero to clear.
			_state.DisplayFlags |= kInteractionStripPublicationMask;
			_updateActorAnimationFrames();
			bool movedUp = false;
			if ((_state.ControllerOneInput & kUpDirectionMask) == 0 &&
				(_state.PreviousControllerOneInput & kUpDirectionMask) != 0 && selectedIndex != 0) {
				int16 candidate = static_cast<int16>(selectedIndex - 1);
				while (candidate >= 0 && (currentFrame.UsedFlags & (1 << candidate)) != 0) {
					candidate = static_cast<int16>(candidate - 1);
				}

				if (candidate >= 0) {
					selectedIndex = candidate;
					selectedRow = static_cast<int16>(selectedRow - lineCounts[candidate]);
					movedUp = true;
				}
			}

			if (!movedUp && (_state.ControllerOneInput & kDownDirectionMask) == 0 &&
				(_state.PreviousControllerOneInput & kDownDirectionMask) != 0) {
				int16 candidate = static_cast<int16>(selectedIndex + 1);
				if (candidate < _state.InteractionActions.count() && candidate <
																		 InteractionActionRegistry::Capacity) {
					while ((currentFrame.UsedFlags & (1 << candidate)) != 0) {
						candidate = static_cast<int16>(candidate + 1);
						if (candidate == InteractionActionRegistry::Capacity) {
							break;
						}
					}

					if (candidate != InteractionActionRegistry::Capacity) {
						selectedRow = static_cast<int16>(selectedRow + lineCounts[selectedIndex]);
						selectedIndex = candidate;
					}
				}
			}

			_state.InteractionStripRow = selectedRow;
			_state.InteractionStripHeightAttributes = static_cast<uint16>((lineCounts[selectedIndex] - 1)
																		  << 8);
			while ((_state.DisplayFlags & kInteractionStripPublicationMask) != 0) {
				if (!waitForRoomRetrace()) {
					return Optional<SessionExit>(SessionExit::HostClosed);
				}
			}

			// Ghidra 0x00001E5A-0x00001E79: either held action button selects; both released repeat the
			// complete publication loop. The strip is hidden before any selected-entry side effect.
			if ((_state.ControllerOneInput & kAlternateActionButtonMask) != 0 &&
				(_state.ControllerOneInput & kActionButtonMask) != 0) {
				continue;
			}

			_state.InteractionStripColumn = -1;
			break;
		}

		// Ghidra 0x00001E7A-0x00001EF5: mark the exact selected bit, preserve the separate fill, mask,
		// text, random-animation, and dismissal boundaries, then restart actor zero at position plus four.
		currentFrame.UsedFlags = static_cast<uint8>(currentFrame.UsedFlags | (1 << selectedIndex));
		const InteractionAction &selectedAction = _state.InteractionActions[selectedIndex];
		fillInteractionTextWindow(interfaceStartColumn);
		_state.InterfaceTileAttributes &= kInterfacePaletteClearMask;
		publishBinaryMaskTiles(6, 1, _state.InterfaceTileAttributes);
		_dialogue.drawDialogueText(DialogueTextSource(selectedAction.LabelTextOffset));
		_chooseRandomInteractionAnimation();
		_state.InteractionFlags |= kInteractionAnimationMask;
		Optional<SessionExit> dismissalExit = _dialogue.waitForDialogueDismissal();
		if (dismissalExit.hasValue()) {
			return dismissalExit;
		}

		_state.InteractionFlags &= static_cast<uint8>(~kInteractionAnimationMask);
		_state.ActorAnimationOffsets[0] = static_cast<int16>(_state.ActorPositionIndices[0] + 4);
		_state.ActorAnimationRestartFlags |= 0x01;

		// Ghidra 0x00001EF6-0x00001F2D: publish the selected state word and its exact signed-length script
		// window while retaining the independently addressed response pointer.
		_state.SelectedInteractionActionStateValue = selectedAction.RoomObjectStateValue;
		_state.RoomScriptStartOffset = selectedAction.ScriptStartOffset;
		_state.RoomScriptEndOffset = selectedAction.ScriptStartOffset + selectedAction.ScriptLength;

		// Ghidra 0x00001F2E-0x00001F6D: clear text through DrawDialogueText, derive both palette arguments
		// from the description low byte, OR palette bits into shared attributes, and regenerate all mask tiles.
		_dialogue.drawDialogueText(DialogueTextSource(Common::String()));
		uint8 descriptionOptions = static_cast<uint8>(_state.InteractionDescriptionFlags);
		uint8 nonzeroPaletteIndex = static_cast<uint8>(descriptionOptions & 0x0F);
		_state.InterfaceTileAttributes |= static_cast<uint16>((descriptionOptions & 0x30) << 9);
		publishBinaryMaskTiles(nonzeroPaletteIndex, 1, _state.InterfaceTileAttributes);

		// Ghidra 0x00001F6E-0x00001FBB: preserve all three independent signed-negative guards before
		// assigning a fixed object's actor slot, forcing its delay ready, and setting only its restart bit.
		int16 roomObjectIndex = static_cast<int16>(_state.CurrentInteraction - 3);
		if (roomObjectIndex >= 0) {
			int8 fixedPositionIndex = _state.RoomObjects[roomObjectIndex].FixedPositionIndex;
			if (fixedPositionIndex >= 0 && _state.SelectedInteractionActionStateValue >= 0) {
				int actorIndex = fixedPositionIndex + 2;
				_state.ActorAnimationOffsets[actorIndex] = _state.SelectedInteractionActionStateValue;
				_state.ActorAnimationDelays[actorIndex] = -1;
				_state.ActorAnimationRestartFlags |= static_cast<uint8>(1 << actorIndex);
			}
		}

		// Ghidra 0x00001FBC-0x00001FE9: present the response, clear it through the distinct draw boundary,
		// clear the complete automatic byte, execute the selected script, and restore only its start cursor.
		Optional<SessionExit> responseExit =
			_dialogue.presentDialogueAndWait(DialogueTextSource(selectedAction.ResponseDialogueOffset));
		if (responseExit.hasValue()) {
			return responseExit;
		}

		_dialogue.drawDialogueText(DialogueTextSource(Common::String()));
		_state.AutomaticInteractionFlags = 0;
		int32 selectedScriptStartOffset = _state.RoomScriptStartOffset;
		Optional<SessionExit> executionExit = _runRoomScriptCommands();
		if (executionExit.hasValue()) {
			return executionExit;
		}

		_state.RoomScriptStartOffset = selectedScriptStartOffset;

		// Ghidra 0x00001FEA-0x0000202D: automatic bit zero unwinds one frame; room-script bit two selects
		// the separate full-unwind path; otherwise save the complete current frame and enter the selected script.
		if ((_state.AutomaticInteractionFlags & kAutomaticFrameUnwindMask) != 0) {
			if (parentFrames.empty()) {
				break;
			}

			currentFrame = parentFrames.back();
			parentFrames.pop_back();
			continue;
		}

		if ((_state.RoomScriptFlags & kRoomScriptMenuExitMask) != 0) {
			// Ghidra 0x0000202E-0x00002049: clear only bit two and discard every saved nested frame.
			_state.RoomScriptFlags &= static_cast<uint8>(~kRoomScriptMenuExitMask);
			parentFrames.clear();
			break;
		}

		parentFrames.push_back(currentFrame);
		currentFrame = InteractionActionMenuFrame(_state.RoomScriptStartOffset, _state.RoomScriptEndOffset, 0);
	}

	// Ghidra 0x0000204A-0x00002069: each ordinary unwind decrements one logical depth and either restores
	// the prior used flags plus both script pointers or converges here after the base frame becomes negative.

	// Ghidra 0x0000206A-0x000020BB: hide and refill the strip, run the second inclusive transition
	// countdown, restore all 192 saved interface words, and generate the cleanup mask from cleared local
	// attributes without writing that cleared value back to g_wInterfaceTileAttributes.
	_state.InteractionStripColumn = -1;
	fillInteractionTextWindow(interfaceStartColumn);
	if (!runInterfaceTransitionCountdown()) {
		return Optional<SessionExit>(SessionExit::HostClosed);
	}

	_scene.loadLayerRows(_graphicsWorkspace.interfaceTileBlock(), TileLayer::Interface,
						 interfaceStartColumn, kInterfaceBlockStartRow, kInterfaceBlockColumnCount,
						 kInterfaceBlockRowCount);
	publishBinaryMaskTiles(0x0F, 1,
						   static_cast<uint16>(_state.InterfaceTileAttributes &
											   kInterfacePaletteClearMask));

	// Ghidra 0x000020BC-0x000020D1: clear only interaction bit two, retain the distinct VBlank, prompt
	// commit, and prompt refresh calls in order, then return normally.
	_state.InteractionFlags &= static_cast<uint8>(~kInteractionMenuActiveMask);
	if (!waitForRoomRetrace()) {
		return Optional<SessionExit>(SessionExit::HostClosed);
	}

	_commitActionPromptUpdate();
	_refreshActionPrompts();
	return Optional<SessionExit>();
}

bool InteractionActionMenuExecutor::runInterfaceTransitionCountdown() {
	_state.RoomInterfaceTransitionCountdown = 0x10;
	while (true) {
		_updateActorAnimationFrames();
		if (_state.RoomInterfaceTransitionCountdown < 0) {
			return true;
		}

		if (!waitForRoomRetrace()) {
			return false;
		}
	}
}

void InteractionActionMenuExecutor::publishBinaryMaskTiles(uint8 nonzeroPaletteIndex,
														   uint8 zeroPaletteIndex,
														   uint16 destinationAttributes) {
	int firstMaskTileIndex = (destinationAttributes & kInterfaceTileIndexMask) + kInterfaceMaskTileBaseDelta;
	Common::Array<uint8> maskTiles =
		generate(_rom, nonzeroPaletteIndex, zeroPaletteIndex);
	_scene.loadTilesAt(MakeSpan(maskTiles), firstMaskTileIndex);
}

bool InteractionActionMenuExecutor::waitForRoomRetrace() {
	_handleRoomVBlank();
	return _presenter.waitForVerticalBlank();
}

void InteractionActionMenuExecutor::fillInteractionTextWindow(int startColumn) {
	// Ghidra 0x000022F2-0x000022FB: add the interaction-mask tile delta to the complete packed interface
	// attribute word with native word wrapping.
	uint16 replacementCellWord = static_cast<uint16>(
		_state.InterfaceTileAttributes + kInterfaceMaskTileBaseDelta);
	TileCell replacementCell = decodeTileCell(replacementCellWord);

	// Ghidra 0x000022FC-0x00002315: replace six DBF rows of 32 explicit VRAM word writes and their
	// 0x80-byte row stride with the corresponding logical interface rectangle.
	_scene.fillLayerRows(replacementCell, TileLayer::Interface, startColumn, kInterfaceBlockStartRow,
						 kInterfaceBlockColumnCount, kInterfaceBlockRowCount);

	// Ghidra 0x00002316-0x0000231F: returned D0=0x000F and D1=0x0010 are overwritten on every caller path.
}
} // namespace Scooby