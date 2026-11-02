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
#include "scooby/graphics/genesis_asset_decoder.h"
#include "scooby/graphics/tile_layer.h"
#include "scooby/scooby.h"

namespace Scooby {

namespace {

// Managed-only mouse support (input/mouse_input.h): finds the unused entry drawn on the choice-strip line
// under pos, using the strip geometry of RoomSpriteRenderer::appendInteractionStripSprites (eight-pixel
// lines from y 0xA8) and the drawing order above, which skips used entries.
const int kInteractionStripTopY = 0xA8;
const int kInteractionStripLineHeight = 8;

bool findInteractionActionAt(const Common::Point &pos, int usedFlags, const Common::Array<int16> &lineCounts,
							 int actionCount, int16 &actionIndex, int16 &actionRow) {
	if (pos.y < kInteractionStripTopY) {
		return false;
	}

	int line = (pos.y - kInteractionStripTopY) / kInteractionStripLineHeight;
	int visibleRow = 0;
	for (int index = 0; index < actionCount && visibleRow < InteractionActionRegistry::capacity; index++) {
		if ((usedFlags & (1 << index)) != 0) {
			continue;
		}

		if (line >= visibleRow && line < visibleRow + lineCounts[index]) {
			actionIndex = static_cast<int16>(index);
			actionRow = static_cast<int16>(visibleRow);
			return true;
		}

		visibleRow += lineCounts[index];
	}

	return false;
}

} // namespace

InteractionActionMenuExecutor::InteractionActionMenuExecutor(
	const ScoobyDooRom &rom, TileScene &scene,
	SharedGraphicsWorkspace &graphicsWorkspace,
	RuntimeState &state, FramePresenter &presenter,
	RoomActorAnimator &actorAnimator, RoomVBlankHandler &roomVBlank,
	Common::Functor0<void> &chooseRandomInteractionAnimation,
	Common::Functor0<Optional<SessionExit>> &runRoomScriptInitializationCommands,
	Common::Functor0<Optional<SessionExit>> &runRoomScriptCommands, DialogueController &dialogue,
	ActionMenuController &actionMenu)
	: _rom(rom), _scene(scene), _graphicsWorkspace(graphicsWorkspace), _state(state), _presenter(presenter),
	  _actorAnimator(actorAnimator), _roomVBlank(roomVBlank),
	  _chooseRandomInteractionAnimation(chooseRandomInteractionAnimation),
	  _runRoomScriptInitializationCommands(runRoomScriptInitializationCommands),
	  _runRoomScriptCommands(runRoomScriptCommands), _dialogue(dialogue),
	  _actionMenu(actionMenu) {
}

Optional<SessionExit> InteractionActionMenuExecutor::runInteractionActionMenu() {
	// Ghidra 0x00001C0C-0x00001C1B: preserve the first independent registration scan and the immediate
	// zero-action return before changing any interface, interaction, or script-frame state.
	Optional<SessionExit> initialScanExit = _runRoomScriptInitializationCommands();
	if (initialScanExit.hasValue()) {
		return initialScanExit;
	}

	if (_state.interactionActions.count() == 0) {
		return Optional<SessionExit>();
	}

	// Ghidra 0x00001C1C-0x00001C39: clear the Window through its original draw boundary, hide the strip,
	// and set only interaction bit two before waiting for any existing transition.
	_dialogue.drawDialogueText(DialogueTextSource(Common::String()));
	_state.interactionStripColumn = -1;
	_state.interactionFlags |= kInteractionMenuActiveMask;

	// Ghidra 0x00001C40-0x00001C69: the native spin observes an interrupt-owned signed byte. Managed
	// continuations service that callback, then preserve both display-side branches after clearing bit three.
	while (_state.roomInterfaceTransitionCountdown >= 0) {
		if (!waitForRoomRetrace()) {
			return Optional<SessionExit>(SessionExit::HostClosed);
		}
	}

	_state.displayFlags &= 0xF7;
	bool savedBlockOnRight = (_state.displayFlags & kDisplaySideMask) == 0;
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
	InteractionActionMenuFrame currentFrame(_state.roomScriptCurrentOffset, _state.roomScriptEndOffset, 0);
	Common::Array<InteractionActionMenuFrame> parentFrames;
	Common::Array<int16> &lineCounts = _state.interactionActionLineCounts;

	while (true) {
		// Ghidra 0x00001CC4-0x00001CE5: restore the current frame's full script window, rescan it without
		// consuming that window, and preserve the zero-count branch that unwinds exactly one frame.
		_state.roomScriptCurrentOffset = currentFrame.scriptStartOffset;
		_state.roomScriptEndOffset = currentFrame.scriptEndOffset;
		Optional<SessionExit> frameScanExit = _runRoomScriptInitializationCommands();
		if (frameScanExit.hasValue()) {
			return frameScanExit;
		}

		if (_state.interactionActions.count() == 0) {
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
		_state.interfaceTileAttributes &= kInterfacePaletteClearMask;
		publishBinaryMaskTiles(6, 0, _state.interfaceTileAttributes);
		while (_state.isButtonDown(ControllerButtons::A) ||
			   _state.isButtonDown(ControllerButtons::B)) {
			if (!waitForRoomRetrace()) {
				return Optional<SessionExit>(SessionExit::HostClosed);
			}
		}

		// Ghidra 0x00001D22-0x00001D71: draw each registered unused entry in order, retain line counts for
		// used entries, and stop on either the complete signed count or the five-row boundary.
		int16 visibleRow = 0;
		int16 remainingActions = static_cast<int16>(_state.interactionActions.count());
		int actionIndex = 0;
		while (true) {
			if ((currentFrame.usedFlags & (1 << actionIndex)) == 0) {
				int16 horizontalOffset = (_state.displayFlags & kDisplaySideMask) == 0
											 ? static_cast<int16>(0)
											 : static_cast<int16>(0x20);
				lineCounts[actionIndex] = _dialogue.drawTimedInteractionText(
					DialogueTextSource(_state.interactionActions[actionIndex].labelTextOffset), visibleRow,
					horizontalOffset);
				visibleRow += lineCounts[actionIndex];
			}

			actionIndex++;
			remainingActions--;
			if (remainingActions <= 0 || visibleRow >= InteractionActionRegistry::capacity) {
				break;
			}
		}

		// Ghidra 0x00001D72-0x00001DA9: publish the initial strip geometry, then scan all five flag bits
		// from four down to zero without constraining the scan to the current registered count.
		int16 selectedRow = 0;
		_state.interactionStripHeightAttributes = static_cast<uint16>((lineCounts[0] - 1) << 8);
		_state.interactionStripRow = 0;
		_state.interactionStripColumn = 0;
		int16 selectedIndex = remainingActions;
		for (int candidate = InteractionActionRegistry::capacity - 1; candidate >= 0; candidate--) {
			if ((currentFrame.usedFlags & (1 << candidate)) == 0) {
				selectedIndex = static_cast<int16>(candidate);
			}
		}

		// Managed-only mouse support: this choice loop reads hover and clicks itself (input/mouse_input.h).
		MouseInput &mouse = g_engine->getMouseInput();
		MouseInput::Mode previousMouseMode = mouse.mode();
		mouse.setMode(MouseInput::Mode::Handled);
		while (true) {
			// Ghidra 0x00001DAA-0x00001E57: request one strip publication, update actor frames, preserve
			// both edge-triggered direction branches and every used-entry skip, then wait for bit zero to clear.
			_state.displayFlags |= kInteractionStripPublicationMask;
			_actorAnimator.updateActorAnimationFrames();

			// Managed-only mouse support: hovering an entry selects it; clicking it chooses it like B.
			bool mouseChose = false;
			{
				Common::Point click;
				int16 index;
				int16 row;
				if (mouse.takeMoved() &&
					findInteractionActionAt(mouse.contentPosition(), currentFrame.usedFlags, lineCounts,
											_state.interactionActions.count(), index, row)) {
					selectedIndex = index;
					selectedRow = row;
				}

				if (mouse.takeLeftClick(click) &&
					findInteractionActionAt(click, currentFrame.usedFlags, lineCounts,
											_state.interactionActions.count(), index, row)) {
					selectedIndex = index;
					selectedRow = row;
					mouseChose = true;
				}
			}

			bool movedUp = false;
			if (_state.isButtonDown(ControllerButtons::Up) &&
				(_state.previousControllerOneInput & kUpDirectionMask) != 0 && selectedIndex != 0) {
				int16 candidate = static_cast<int16>(selectedIndex - 1);
				while (candidate >= 0 && (currentFrame.usedFlags & (1 << candidate)) != 0) {
					candidate--;
				}

				if (candidate >= 0) {
					selectedIndex = candidate;
					selectedRow -= lineCounts[candidate];
					movedUp = true;
				}
			}

			if (!movedUp && _state.isButtonDown(ControllerButtons::Down) &&
				(_state.previousControllerOneInput & kDownDirectionMask) != 0) {
				int16 candidate = static_cast<int16>(selectedIndex + 1);
				if (candidate < _state.interactionActions.count() && candidate <
																		 InteractionActionRegistry::capacity) {
					while ((currentFrame.usedFlags & (1 << candidate)) != 0) {
						candidate++;
						if (candidate == InteractionActionRegistry::capacity) {
							break;
						}
					}

					if (candidate != InteractionActionRegistry::capacity) {
						selectedRow += lineCounts[selectedIndex];
						selectedIndex = candidate;
					}
				}
			}

			_state.interactionStripRow = selectedRow;
			_state.interactionStripHeightAttributes = static_cast<uint16>((lineCounts[selectedIndex] - 1)
																		  << 8);
			while ((_state.displayFlags & kInteractionStripPublicationMask) != 0) {
				if (!waitForRoomRetrace()) {
					return Optional<SessionExit>(SessionExit::HostClosed);
				}
			}

			// Ghidra 0x00001E5A-0x00001E79: either held action button selects; both released repeat the
			// complete publication loop. The strip is hidden before any selected-entry side effect.
			if (!_state.isButtonDown(ControllerButtons::B) &&
				!_state.isButtonDown(ControllerButtons::A) && !mouseChose) {
				continue;
			}

			_state.interactionStripColumn = -1;
			break;
		}

		// Managed-only mouse support: dialogue after the choice takes clicks again.
		mouse.setMode(previousMouseMode);

		// Ghidra 0x00001E7A-0x00001EF5: mark the exact selected bit, preserve the separate fill, mask,
		// text, random-animation, and dismissal boundaries, then restart actor zero at position plus four.
		currentFrame.usedFlags |= 1 << selectedIndex;
		const InteractionAction &selectedAction = _state.interactionActions[selectedIndex];
		fillInteractionTextWindow(interfaceStartColumn);
		_state.interfaceTileAttributes &= kInterfacePaletteClearMask;
		publishBinaryMaskTiles(6, 1, _state.interfaceTileAttributes);
		_dialogue.drawDialogueText(DialogueTextSource(selectedAction.labelTextOffset));
		_chooseRandomInteractionAnimation();
		_state.interactionFlags |= kInteractionAnimationMask;
		Optional<SessionExit> dismissalExit = _dialogue.waitForDialogueDismissal();
		if (dismissalExit.hasValue()) {
			return dismissalExit;
		}

		_state.interactionFlags &= static_cast<uint8>(~kInteractionAnimationMask);
		_state.actorAnimationOffsets[kShaggyActorSlot] = static_cast<int16>(_state.actorPositionIndices[kShaggyActorSlot] + 4);
		_state.actorAnimationRestartFlags |= kShaggyActorMask;

		// Ghidra 0x00001EF6-0x00001F2D: publish the selected state word and its exact signed-length script
		// window while retaining the independently addressed response pointer.
		_state.selectedInteractionActionStateValue = selectedAction.roomObjectStateValue;
		_state.roomScriptCurrentOffset = selectedAction.scriptStartOffset;
		_state.roomScriptEndOffset = selectedAction.scriptStartOffset + selectedAction.scriptLength;

		// Ghidra 0x00001F2E-0x00001F6D: clear text through DrawDialogueText, derive both palette arguments
		// from the description low byte, OR palette bits into shared attributes, and regenerate all mask tiles.
		_dialogue.drawDialogueText(DialogueTextSource(Common::String()));
		uint8 descriptionOptions = static_cast<uint8>(_state.interactionDescriptionFlags);
		uint8 nonzeroPaletteIndex = static_cast<uint8>(descriptionOptions & 0x0F);
		_state.interfaceTileAttributes |= static_cast<uint16>((descriptionOptions & 0x30) << 9);
		publishBinaryMaskTiles(nonzeroPaletteIndex, 1, _state.interfaceTileAttributes);

		// Ghidra 0x00001F6E-0x00001FBB: preserve all three independent signed-negative guards before
		// assigning a fixed object's actor slot, forcing its delay ready, and setting only its restart bit.
		int16 roomObjectIndex = static_cast<int16>(_state.currentInteraction - 3);
		if (roomObjectIndex >= 0) {
			int8 fixedPositionIndex = _state.roomObjects[roomObjectIndex].fixedPositionIndex;
			if (fixedPositionIndex >= 0 && _state.selectedInteractionActionStateValue >= 0) {
				int actorIndex = fixedPositionIndex + 2;
				_state.actorAnimationOffsets[actorIndex] = _state.selectedInteractionActionStateValue;
				_state.actorAnimationDelays[actorIndex] = -1;
				_state.actorAnimationRestartFlags |= static_cast<uint8>(1 << actorIndex);
			}
		}

		// Ghidra 0x00001FBC-0x00001FE9: present the response, clear it through the distinct draw boundary,
		// clear the complete automatic byte, execute the selected script, and restore only its start cursor.
		Optional<SessionExit> responseExit =
			_dialogue.presentDialogueAndWait(DialogueTextSource(selectedAction.responseDialogueOffset));
		if (responseExit.hasValue()) {
			return responseExit;
		}

		_dialogue.drawDialogueText(DialogueTextSource(Common::String()));
		_state.automaticInteractionFlags = 0;
		int32 selectedScriptStartOffset = _state.roomScriptCurrentOffset;
		Optional<SessionExit> executionExit = _runRoomScriptCommands();
		if (executionExit.hasValue()) {
			return executionExit;
		}

		_state.roomScriptCurrentOffset = selectedScriptStartOffset;

		// Ghidra 0x00001FEA-0x0000202D: automatic bit zero unwinds one frame; room-script bit two selects
		// the separate full-unwind path; otherwise save the complete current frame and enter the selected script.
		if ((_state.automaticInteractionFlags & kAutomaticFrameUnwindMask) != 0) {
			if (parentFrames.empty()) {
				break;
			}

			currentFrame = parentFrames.back();
			parentFrames.pop_back();
			continue;
		}

		if ((_state.roomScriptFlags & kRoomScriptMenuExitMask) != 0) {
			// Ghidra 0x0000202E-0x00002049: clear only bit two and discard every saved nested frame.
			_state.roomScriptFlags &= static_cast<uint8>(~kRoomScriptMenuExitMask);
			parentFrames.clear();
			break;
		}

		parentFrames.push_back(currentFrame);
		currentFrame = InteractionActionMenuFrame(_state.roomScriptCurrentOffset, _state.roomScriptEndOffset, 0);
	}

	// Ghidra 0x0000204A-0x00002069: each ordinary unwind decrements one logical depth and either restores
	// the prior used flags plus both script pointers or converges here after the base frame becomes negative.

	// Ghidra 0x0000206A-0x000020BB: hide and refill the strip, run the second inclusive transition
	// countdown, restore all 192 saved interface words, and generate the cleanup mask from cleared local
	// attributes without writing that cleared value back to g_wInterfaceTileAttributes.
	_state.interactionStripColumn = -1;
	fillInteractionTextWindow(interfaceStartColumn);
	if (!runInterfaceTransitionCountdown()) {
		return Optional<SessionExit>(SessionExit::HostClosed);
	}

	_scene.loadLayerRows(_graphicsWorkspace.interfaceTileBlock(), TileLayer::Interface,
						 interfaceStartColumn, kInterfaceBlockStartRow, kInterfaceBlockColumnCount,
						 kInterfaceBlockRowCount);
	publishBinaryMaskTiles(0x0F, 1,
						   static_cast<uint16>(_state.interfaceTileAttributes &
											   kInterfacePaletteClearMask));

	// Ghidra 0x000020BC-0x000020D1: clear only interaction bit two, retain the distinct VBlank, prompt
	// commit, and prompt refresh calls in order, then return normally.
	_state.interactionFlags &= static_cast<uint8>(~kInteractionMenuActiveMask);
	if (!waitForRoomRetrace()) {
		return Optional<SessionExit>(SessionExit::HostClosed);
	}

	_actionMenu.commitActionPromptUpdate();
	_actionMenu.refreshActionPrompts();
	return Optional<SessionExit>();
}

bool InteractionActionMenuExecutor::runInterfaceTransitionCountdown() {
	_state.roomInterfaceTransitionCountdown = 0x10;
	while (true) {
		_actorAnimator.updateActorAnimationFrames();
		if (_state.roomInterfaceTransitionCountdown < 0) {
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
		colorise4bppMaskTiles(_rom, nonzeroPaletteIndex, zeroPaletteIndex);
	_scene.loadTilesAt(makeSpan(maskTiles), firstMaskTileIndex);
}

bool InteractionActionMenuExecutor::waitForRoomRetrace() {
	_roomVBlank.handleRoomVBlank();
	return _presenter.waitForVerticalBlank();
}

void InteractionActionMenuExecutor::fillInteractionTextWindow(int startColumn) {
	// Ghidra 0x000022F2-0x000022FB: add the interaction-mask tile delta to the complete packed interface
	// attribute word with native word wrapping.
	uint16 replacementCellWord = static_cast<uint16>(
		_state.interfaceTileAttributes + kInterfaceMaskTileBaseDelta);
	TileCell replacementCell = decodeTileCell(replacementCellWord);

	// Ghidra 0x000022FC-0x00002315: replace six DBF rows of 32 explicit VRAM word writes and their
	// 0x80-byte row stride with the corresponding logical interface rectangle.
	_scene.fillLayerRows(replacementCell, TileLayer::Interface, startColumn, kInterfaceBlockStartRow,
						 kInterfaceBlockColumnCount, kInterfaceBlockRowCount);

	// Ghidra 0x00002316-0x0000231F: returned D0=0x000F and D1=0x0010 are overwritten on every caller path.
}
} // namespace Scooby
