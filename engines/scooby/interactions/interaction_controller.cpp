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

#include "interaction_controller.h"

#include "common/scummsys.h"

#include "scooby/common/contracts.h"
#include "scooby/rooms/room_object.h"

namespace Scooby {

void InteractionController::snapshotInteractionState() {
	_state.PreviousInteraction = _state.CurrentInteraction;
	_state.PreviousDisplayFlags = _state.DisplayFlags;
	_state.PreviousInteractionFlags = _state.InteractionFlags;
	_state.PreviousRoomNameTextOffset = _state.RoomNameTextOffset;
	_state.PreviousActionIcon = _state.ActiveActionIcon;
	_state.PreviousSecondaryInteraction = _state.SecondaryInteraction;
}

Optional<SessionExit> InteractionController::updateAutomaticInteraction(bool &restartRoomLoop) {
	restartRoomLoop = false;
	_state.CurrentInteraction = _resolver.findAutomaticInteraction();
	if (_state.CurrentInteraction == 0) {
		return Optional<SessionExit>();
	}

	if (_state.CurrentInteraction != _state.PreviousInteraction) {
		_state.AutomaticInteractionFlags &= static_cast<uint8>(~kAutomaticInteractionLatchMask);
	}

	if ((_state.AutomaticInteractionFlags & kAutomaticInteractionLatchMask) != 0) {
		return Optional<SessionExit>();
	}

	_state.InteractionMode = 11;
	_state.DisplayFlags |= kInteractionPendingMask;
	Optional<SessionExit> requestedExit = runSelectedInteraction();
	if (requestedExit.hasValue()) {
		return requestedExit;
	}

	_state.AutomaticInteractionFlags |= kAutomaticInteractionLatchMask;
	_state.DisplayFlags &= static_cast<uint8>(~kInteractionPendingMask);
	if (!_waitForRoomVerticalBlank()) {
		return Optional<SessionExit>(SessionExit::HostClosed);
	}

	restartRoomLoop = true;
	return Optional<SessionExit>();
}

void InteractionController::chooseRandomInteractionAnimation() {
	// Ghidra 0x000020DA-0x000020ED: retain both independent transition suppression branches and return
	// without advancing random state or touching actor state when either bit is set.
	if ((_state.TransitionFlags & kSuppressRandomAnimationMask) != 0) {
		return;
	}

	if ((_state.TransitionFlags & kAlternateActorModeMask) != 0) {
		return;
	}

	// Ghidra 0x000020EE-0x000020FB: scale the next shared value by seven, subtract three in the low word,
	// and clamp only signed-negative outcomes to zero.
	int16 randomAnimationOffset = static_cast<int16>(_random.scaleNextRandomValue(7) - 3);
	if (randomAnimationOffset < 0) {
		randomAnimationOffset = 0;
	}

	// Ghidra 0x000020FC-0x0000211F: exact position two mutates shared state to zero. Exact position three
	// substitutes two only in the local calculation; every other signed word passes through unchanged.
	if (_state.ActorPositionIndices[0] == 2) {
		_state.ActorPositionIndices[0] = 0;
	}

	int16 positionIndex = _state.ActorPositionIndices[0];
	if (positionIndex == 3) {
		positionIndex = 2;
	}

	// Ghidra 0x00002120-0x00002135: shift and add with word wrapping, publish actor zero's descriptor-
	// relative animation offset, then request its restart while preserving every other slot.
	int16 positionAnimationOffset = static_cast<int16>(positionIndex << 2);
	_state.ActorAnimationOffsets[0] =
		static_cast<int16>(positionAnimationOffset + randomAnimationOffset + 0x24);
	_state.ActorAnimationRestartFlags |= kActorZeroRestartMask;

	// Ghidra 0x00002136-0x00002137: both suppression paths and the selection path return here.
}

bool InteractionController::updateCursorInteraction() {
	if (_state.CursorY < 168) {
		if (_state.CursorY >= 8) {
			int16 selectedInteraction = _resolver.findAdjustedCursorInteraction();
			if ((_state.InteractionFlags & kAlternateInteractionMask) == 0) {
				_state.CurrentInteraction = selectedInteraction;
			} else {
				_state.SecondaryInteraction = selectedInteraction == _state.CurrentInteraction
												  ? static_cast<int16>(0)
												  : selectedInteraction;
			}
		}
	} else if (!_actionMenu.updateActionMenu()) {
		return false;
	}

	_actionMenu.synchronizeActionIcon();
	return true;
}

Optional<SessionExit> InteractionController::runSelectedInteraction() {
	while (true) {
		// Ghidra 0x000019B0-0x000019BD: every pass clears all registered actions and independently clears
		// room-script bit three before resolving the possibly scripted replacement interaction.
		_state.InteractionActions.reset();
		_state.RoomScriptFlags &= static_cast<uint8>(~kScriptedInteractionMask);

		// Ghidra 0x000019BE-0x000019F7: load both descriptor tables before preserving the signed low-
		// identity return and the distinct mode-two inventory-room rejection branch.
		int32 descriptorOffset = _state.ActiveEpisodeDescriptorOffset;
		int interactionScriptBaseOffset =
			static_cast<int>(_rom.readUInt32(descriptorOffset + kEpisodeInteractionScriptBaseField));
		int interactionRecordTableOffset =
			static_cast<int>(_rom.readUInt32(descriptorOffset + kEpisodeInteractionRecordTableField));
		int16 objectIndex = static_cast<int16>(_state.CurrentInteraction - 3);
		if (objectIndex < 0) {
			return Optional<SessionExit>();
		}

		if (_state.InteractionMode == kInventorySelectionMode &&
			_state.RoomObjects[objectIndex].RoomId == kInventoryRoomId) {
			return Optional<SessionExit>();
		}

		// Ghidra 0x000019F8-0x00001A15: preserve the wrapped eight-byte record index, header-relative
		// signed bound, description word, ignored third word, and separate initialization-scan call.
		int16 interactionRecordByteOffset = static_cast<int16>(objectIndex << 3);
		int interactionScriptHeaderOffset = interactionScriptBaseOffset + static_cast<int>(_rom.readUInt32(
																			  interactionRecordTableOffset + interactionRecordByteOffset));
		int interactionScriptStartOffset = interactionScriptHeaderOffset + kInteractionScriptHeaderByteCount;
		int interactionScriptEndOffset =
			interactionScriptHeaderOffset + _rom.readInt16(interactionScriptHeaderOffset);
		_state.InteractionDescriptionFlags =
			_rom.readUInt16(interactionScriptHeaderOffset + static_cast<int>(sizeof(uint16)));
		_state.RoomScriptStartOffset = interactionScriptStartOffset;
		_state.RoomScriptEndOffset = interactionScriptEndOffset;
		SDM_ASSERT(_runRoomScriptInitializationCommands != nullptr,
				   "InteractionController runRoomScriptInitializationCommands callback is not bound.");
		Optional<SessionExit> initializationExit = (*_runRoomScriptInitializationCommands)();
		if (initializationExit.hasValue()) {
			return initializationExit;
		}

		// Ghidra 0x00001A16-0x00001A21: registered initialization actions alone select the first distinct
		// icon refresh; zero retains the branch that skips it.
		if (_state.InteractionActions.count() != 0) {
			_actionMenu.refreshSelectedActionIcons();
		}

		// Ghidra 0x00001A22-0x00001A43: overwrite the automatic byte with scan gate one, snapshot the
		// stable interaction mode and complete interaction byte, then invoke the scan dispatcher.
		_state.AutomaticInteractionFlags = kSelectedInteractionScanMask;
		int16 interactionMode = _state.InteractionMode;
		uint8 interactionFlags = _state.InteractionFlags;
		SDM_ASSERT(_invokeRoomScriptDispatchCallback != nullptr,
				   "InteractionController invokeRoomScriptDispatchCallback callback is not bound.");
		Optional<SessionExit> scanExit = (*_invokeRoomScriptDispatchCallback)(0);
		if (scanExit.hasValue()) {
			return scanExit;
		}

		// Ghidra 0x00001A44-0x00001A7F: marker-clear skips nested execution. Marker-set preserves every
		// automatic bit while adding bit six, resolves the signed +6 bound, advances by eight, clears only
		// marker bit zero, refreshes icons unconditionally, and runs the separate execution dispatcher.
		if ((_state.AutomaticInteractionFlags & kAutomaticInteractionMarkerMask) != 0) {
			_state.AutomaticInteractionFlags |= kMatchedSelectedInteractionMask;
			int nestedScriptHeaderOffset = _state.RoomScriptStartOffset;
			_state.RoomScriptEndOffset = nestedScriptHeaderOffset + _rom.readInt16(nestedScriptHeaderOffset + 6);
			_state.RoomScriptStartOffset =
				nestedScriptHeaderOffset + kNestedSelectedInteractionScriptHeaderByteCount;
			_state.AutomaticInteractionFlags &= static_cast<uint8>(~kAutomaticInteractionMarkerMask);
			_actionMenu.refreshSelectedActionIcons();
			SDM_ASSERT(_invokeRoomScriptDispatchCallback != nullptr,
					   "InteractionController invokeRoomScriptDispatchCallback callback is not bound.");
			Optional<SessionExit> executionExit = (*_invokeRoomScriptDispatchCallback)(1);
			if (executionExit.hasValue()) {
				return executionExit;
			}
		}

		// Ghidra 0x00001A80-0x00001AAB: restore the selected interaction's initial cursor and bound. Mode
		// eight alone saves the complete automatic byte around its menu and sets bit six only for a nonzero
		// post-menu action count. A managed non-returning menu path bypasses both native restorations.
		_state.RoomScriptStartOffset = interactionScriptStartOffset;
		_state.RoomScriptEndOffset = interactionScriptEndOffset;
		if (interactionMode == kInteractionActionMenuMode) {
			uint8 automaticInteractionFlags = _state.AutomaticInteractionFlags;
			SDM_ASSERT(_runInteractionActionMenu != nullptr,
					   "InteractionController runInteractionActionMenu callback is not bound.");
			Optional<SessionExit> menuExit = (*_runInteractionActionMenu)();
			if (menuExit.hasValue()) {
				return menuExit;
			}

			_state.AutomaticInteractionFlags = automaticInteractionFlags;
			if (_state.InteractionActions.count() != 0) {
				_state.AutomaticInteractionFlags |= kMatchedSelectedInteractionMask;
			}
		}

		// Ghidra 0x00001AAC-0x00001B1D: automatic bit six skips all mode behavior. Modes five and six
		// preserve different room-bit writes and different predicates before converging on every suppress,
		// clear, and set outcome of the interaction-bit-zero toggle.
		if ((_state.AutomaticInteractionFlags & kMatchedSelectedInteractionMask) == 0) {
			bool applyInteractionToggle = false;
			if (interactionMode == kClearRoomBehaviorMode) {
				applyInteractionToggle = (_state.RoomBehaviorFlags & kRoomBehaviorInteractionMask) == 0;
				_state.RoomBehaviorFlags &= static_cast<uint8>(~kRoomBehaviorInteractionMask);
				if (!applyInteractionToggle) {
					_state.InteractionFlags &= static_cast<uint8>(~kAlternateInteractionMask);
				}
			} else if (interactionMode == kSetRoomBehaviorMode) {
				_state.RoomBehaviorFlags |= kRoomBehaviorInteractionMask;
				applyInteractionToggle = (interactionFlags & kAlternateInteractionMask) == 0;
				if (!applyInteractionToggle) {
					_state.InteractionFlags &= static_cast<uint8>(~kAlternateInteractionMask);
				}
			}

			if (applyInteractionToggle && (_state.AutomaticInteractionFlags & kSuppressInteractionToggleMask) ==
											  0) {
				if ((_state.InteractionFlags & kAlternateInteractionMask) != 0) {
					_state.InteractionFlags &= static_cast<uint8>(~kAlternateInteractionMask);
				} else {
					_state.SecondaryInteraction = 0;
					_state.InteractionFlags |= kAlternateInteractionMask;
				}
			}

			// Ghidra 0x00001B1E-0x00001B9F: preserve both dialogue suppressors, the wrapped mode table
			// index, every template byte, and all three description-placeholder selection branches.
			if ((_state.InteractionFlags & kAlternateInteractionMask) == 0 &&
				interactionMode != kSuppressFallbackDialogueMode) {
				int16 templatePointerByteOffset = static_cast<int16>((interactionMode - 1) << 2);
				int templateOffset = static_cast<int>(
					_rom.readUInt32(kInteractionDialogueTemplateTableOffset + templatePointerByteOffset));
				Common::String dialogueText;
				while (true) {
					uint8 value = _rom.readByte(templateOffset++);
					if (value == 0) {
						break;
					}

					if (value != kInteractionDescriptionPlaceholder) {
						dialogueText.push_back(static_cast<char>(value));
						continue;
					}

					int replacementOffset = kInteractionDescriptionNeutralOffset;
					if ((_state.InteractionDescriptionFlags & kInteractionDescriptionPersonMask) != 0) {
						replacementOffset =
							(_state.InteractionDescriptionFlags & kInteractionDescriptionFeminineMask) == 0
								? kInteractionDescriptionMasculineOffset
								: kInteractionDescriptionFeminineOffset;
					}

					copyNullTerminatedString(replacementOffset, dialogueText);
				}

				Optional<SessionExit> dialogueExit =
					_dialogue.presentDialogueAndWait(DialogueTextSource(dialogueText));
				if (dialogueExit.hasValue()) {
					return dialogueExit;
				}
			}
		}

		// Ghidra 0x00001BA0-0x00001BD5: compare the active icon with current mutable interaction mode,
		// then preserve both room-script-bit-three outcomes. A set bit selects scripted mode eight and
		// repeats from the complete reset path; a clear bit returns.
		if (_state.ActiveActionIcon != _state.InteractionMode) {
			_state.ActiveActionIcon = 0;
		}

		if ((_state.RoomScriptFlags & kScriptedInteractionMask) == 0) {
			// Ghidra 0x00001BD6-0x00001BD9: both native terminal RTS instructions are represented by this
			// normal completion and the identity-negative managed return above.
			return Optional<SessionExit>();
		}

		_state.InteractionMode = kInteractionActionMenuMode;
		_state.CurrentInteraction = _state.ScriptedInteraction;
	}
}

void InteractionController::refreshInteractionDisplay() {
	// Ghidra 0x0000A828-0x0000A873: when the interaction display is hidden, video bit five suppresses
	// room-name publication before the explicit-refresh bit is inspected. Otherwise redraw on a forced
	// refresh, a transition from the interaction display, or a changed room-name pointer.
	if ((_state.DisplayFlags & kInteractionDisplayMask) == 0) {
		if ((_state.VideoFlags & kSuppressRoomNameRefreshMask) != 0) {
			return;
		}

		if ((_state.InteractionFlags & kCompleteRefreshMask) != 0) {
			_state.InteractionFlags &= static_cast<uint8>(~kCompleteRefreshMask);
		} else if ((_state.PreviousDisplayFlags & kInteractionDisplayMask) == 0 &&
				   _state.RoomNameTextOffset == _state.PreviousRoomNameTextOffset) {
			return;
		}

		_dialogue.drawDialogueText(DialogueTextSource(_state.RoomNameTextOffset));
		return;
	}

	// Ghidra 0x0000A874-0x0000A8E3: a forced refresh bypasses every snapshot comparison and is consumed.
	// Without it, preserve each original comparison, including only bit zero of both interaction flag bytes.
	if ((_state.InteractionFlags & kCompleteRefreshMask) != 0) {
		_state.InteractionFlags &= static_cast<uint8>(~kCompleteRefreshMask);
	} else if ((_state.PreviousDisplayFlags & kInteractionDisplayMask) != 0 &&
			   _state.CurrentInteraction == _state.PreviousInteraction &&
			   _state.ActiveActionIcon == _state.PreviousActionIcon &&
			   _state.SecondaryInteraction == _state.PreviousSecondaryInteraction &&
			   (_state.InteractionFlags & kAlternateInteractionMask) ==
				   (_state.PreviousInteractionFlags & kAlternateInteractionMask)) {
		return;
	}

	Common::String interactionText;

	// Ghidra 0x0000A8E4-0x0000A8F9: a zero current interaction publishes the empty shared text buffer.
	if (_state.CurrentInteraction != 0) {
		int actionTextOffset = kInteractionActionTextTableOffset;

		// Ghidra 0x0000A8FA-0x0000A91F: walk complete NUL terminators with signed-word DBF wrapping,
		// retaining out-of-authored-range traversal instead of constraining state to the ten known entries.
		if (_state.ActiveActionIcon != 0) {
			int16 remainingPrefixes = static_cast<int16>(_state.ActiveActionIcon - 1);
			do {
				while (_rom.readByte(actionTextOffset++) != 0) {
				}

				remainingPrefixes--;
			} while (remainingPrefixes != -1);
		}

		copyNullTerminatedString(actionTextOffset, interactionText);

		// Ghidra 0x0000A920-0x0000A935: overwrite the prefix terminator with the current target text.
		copyNullTerminatedString(resolveInteractionLabelAddress(_state.CurrentInteraction), interactionText);

		// Ghidra 0x0000A936-0x0000A977: alternate interaction mode always appends its selected connector;
		// only the final secondary descriptor is conditional on the secondary word being nonzero.
		if ((_state.InteractionFlags & kAlternateInteractionMask) != 0) {
			copyNullTerminatedString((_state.RoomBehaviorFlags & kUseToConnectorMask) != 0
										 ? kInteractionToConnectorOffset
										 : kInteractionWithConnectorOffset,
									 interactionText);
			if (_state.SecondaryInteraction != 0) {
				copyNullTerminatedString(resolveInteractionLabelAddress(_state.SecondaryInteraction),
										 interactionText);
			}
		}
	}

	// Ghidra 0x0000A978-0x0000A983: publish the composed workspace through the separate text renderer.
	_dialogue.drawDialogueText(DialogueTextSource(interactionText));
}

void InteractionController::copyNullTerminatedString(int sourceOffset, Common::String &destination) {
	// Ghidra 0x00005B56-0x00005B5B: copy one byte, advance both pointers, and take the loop-back branch
	// for every nonzero byte. Casting byte to char preserves its value until DialogueTextSource casts back.
	while (true) {
		uint8 value = _rom.readByte(sourceOffset++);
		if (value == 0) {
			// Ghidra 0x00005B5C-0x00005B5D: the zero branch leaves both native pointers past the NUL.
			// The managed string's implicit terminator represents the equivalent destination cursor.
			return;
		}

		destination.push_back(static_cast<char>(value));
	}
}

int InteractionController::resolveInteractionLabelAddress(int16 interaction) {
	// Ghidra 0x00005B5E-0x00005B6B: load the active descriptor's label table first and label base second.
	int32 descriptorOffset = _state.ActiveEpisodeDescriptorOffset;
	int labelTableOffset = static_cast<int>(_rom.readUInt32(descriptorOffset + kEpisodeObjectLabelTableField));
	int labelBaseOffset = static_cast<int>(_rom.readUInt32(descriptorOffset + kEpisodeObjectLabelBaseField));

	// Ghidra 0x00005B6C-0x00005B79: the first wrapping word decrement retains its signed-negative return
	// branch before identity one selects the fixed Shaggy label.
	int16 selector = static_cast<int16>(interaction - 1);
	if (selector < 0) {
		return labelBaseOffset;
	}

	if (selector == 0) {
		return kShaggyInteractionLabelOffset;
	}

	// Ghidra 0x00005B7A-0x00005B85: a second wrapping decrement selects Scooby only on exact zero.
	selector--;
	if (selector == 0) {
		return kScoobyInteractionLabelOffset;
	}

	// Ghidra 0x00005B86-0x00005B91: every remaining word pattern receives the third decrement, a
	// wrapping low-word shift by three, and the record's unsigned longword displacement added to the base.
	selector--;
	int16 tableByteOffset = static_cast<int16>(selector << kObjectLabelRecordShift);
	int relativeLabelOffset = static_cast<int>(
		_rom.readUInt32(labelTableOffset + kObjectLabelDisplacementField + tableByteOffset));
	return labelBaseOffset + relativeLabelOffset;
}

Optional<SessionExit> InteractionController::processAutomaticInteractions() {
	// Ghidra 0x0000AFDA-0x0000AFE9: managed locals preserve the caller's dynamic script window and the
	// complete automatic-interaction byte; the bound mode callback replaces the native code pointer.
	uint8 automaticInteractionFlags = _state.AutomaticInteractionFlags;
	int32 scriptStartOffset = _state.RoomScriptStartOffset;
	int32 scriptEndOffset = _state.RoomScriptEndOffset;

	// Ghidra 0x0000AFEA-0x0000B005: resolve both episode-relative tables once, then reproduce the native
	// pre-decrement by omitting exactly the terminal decoded object from the indexed traversal.
	int32 descriptorOffset = _state.ActiveEpisodeDescriptorOffset;
	int interactionRecordTableOffset =
		static_cast<int>(_rom.readUInt32(descriptorOffset + kEpisodeInteractionRecordTableField));
	int interactionScriptBaseOffset =
		static_cast<int>(_rom.readUInt32(descriptorOffset + kEpisodeInteractionScriptBaseField));
	for (std::size_t objectIndex = 0; objectIndex + 1 < _state.RoomObjects.size(); objectIndex++) {
		RoomObject &roomObject = _state.RoomObjects[objectIndex];

		// Ghidra 0x0000B006-0x0000B023: every rejected flag and room branch still advances both native
		// record cursors. Indexed traversal preserves that convergence without omitting either branch.
		if ((roomObject.Flags & kAutomaticRoomObjectMask) == 0) {
			continue;
		}

		if (roomObject.RoomId != kInventoryRoomId && roomObject.RoomId != _state.RoomId) {
			continue;
		}

		// Ghidra 0x0000B024-0x0000B059: preserve only latch bit seven, enter scan mode, resolve this
		// object's six-byte script header, and dispatch even when its signed-length window is empty.
		_state.AutomaticInteractionFlags = static_cast<uint8>(
			(_state.AutomaticInteractionFlags & kAutomaticInteractionLatchMask) |
			kAutomaticInteractionScanMask);
		int interactionRecordOffset =
			interactionRecordTableOffset + static_cast<int>(objectIndex) * kAutomaticInteractionRecordByteCount;
		int scriptHeaderOffset =
			interactionScriptBaseOffset + static_cast<int>(_rom.readUInt32(interactionRecordOffset));
		_state.RoomScriptStartOffset = scriptHeaderOffset + kInteractionScriptHeaderByteCount;
		_state.RoomScriptEndOffset = _state.RoomScriptStartOffset + _rom.readInt16(scriptHeaderOffset);
		SDM_ASSERT(_invokeRoomScriptDispatchCallback != nullptr,
				   "InteractionController invokeRoomScriptDispatchCallback callback is not bound.");
		Optional<SessionExit> scanExit = (*_invokeRoomScriptDispatchCallback)(0);
		if (scanExit.hasValue()) {
			return scanExit;
		}

		// Ghidra 0x0000B05E-0x0000B077: a clear marker leaves the object eligible and converges on the
		// common cursor advance; a set marker retains only latch bit seven and clears object flag bit two.
		if ((_state.AutomaticInteractionFlags & kAutomaticInteractionMarkerMask) == 0) {
			continue;
		}

		_state.AutomaticInteractionFlags &= kAutomaticInteractionLatchMask;
		roomObject.Flags &= static_cast<uint8>(~kAutomaticRoomObjectMask);

		// Ghidra 0x0000B078-0x0000B099: command 0x13 leaves A5 on its marker. Its signed length at +2
		// remains relative to that marker, while execution begins after the complete four-byte header.
		int nestedScriptHeaderOffset = _state.RoomScriptStartOffset;
		_state.RoomScriptEndOffset =
			nestedScriptHeaderOffset + _rom.readInt16(
										   nestedScriptHeaderOffset + static_cast<int>(sizeof(uint16)));
		_state.RoomScriptStartOffset = nestedScriptHeaderOffset + kNestedInteractionScriptHeaderByteCount;
		SDM_ASSERT(_invokeRoomScriptDispatchCallback != nullptr,
				   "InteractionController invokeRoomScriptDispatchCallback callback is not bound.");
		Optional<SessionExit> executionExit = (*_invokeRoomScriptDispatchCallback)(1);
		if (executionExit.hasValue()) {
			return executionExit;
		}

		// Ghidra 0x0000B09A-0x0000B0A5: the indexed loop advances both logical records and preserves the
		// native low-word DBF count through the proved episode table cardinalities.
	}

	// Ghidra 0x0000B0A6-0x0000B0B7: restore the complete saved byte before the caller's dynamic script
	// window and return normally. Managed non-returning paths above deliberately never reach this range.
	_state.AutomaticInteractionFlags = automaticInteractionFlags;
	_state.RoomScriptStartOffset = scriptStartOffset;
	_state.RoomScriptEndOffset = scriptEndOffset;
	return Optional<SessionExit>();
}

} // namespace Scooby