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

#include "room_script_command_executor.h"

#include "common/scummsys.h"

#include "room_object.h"

namespace Scooby {

RoomScriptCommandExecutor::RoomScriptCommandExecutor(
	const ScoobyDooRom &rom, TileScene &scene, RuntimeState &state,
	const RoomCollisionProbe &collisionProbe, ScriptedRoomMovementExecutor &movement,
	DeterministicRandom &random, FramePresenter &presenter,
	Common::Functor0<void> &updateActorAnimationFrames, Common::Functor0<void> &handleRoomVBlank,
	Common::Functor0<Optional<SessionExit>> &presentRoom,
	Common::Functor0<Optional<SessionExit>> &resetRoomStateForReload,
	Common::Functor0<Optional<SessionExit>> &reloadAndPresentRoom,
	Common::Functor0<Optional<SessionExit>> &processAutomaticInteractions,
	Common::Functor0<void> &refreshInteractionDisplay, Common::Functor0<void> &finishRoomStartup,
	Common::Functor0<void> &executeRoomScriptAction1D, DialogueController &dialogue,
	Common::Functor1<int, Optional<SessionExit>> &invokeRoomScriptDispatchCallback,
	Common::Functor0<void> &commitActionPromptUpdate, Common::Functor0<void> &refreshActionPrompts,
	Common::Functor1<int, void> &drawSelectedActionIcon, RoomTileStreamer &tileStreamer,
	Common::Functor1<const Common::Functor0<void> *, bool> &restartApplication)
	: _rom(rom), _state(state), _presenter(presenter), _reloadAndPresentRoom(reloadAndPresentRoom),
	  _resetRoomStateForReload(resetRoomStateForReload),
	  _updateActorAnimationFrames(updateActorAnimationFrames), _handleRoomVBlank(handleRoomVBlank),
	  _commitActionPromptUpdate(commitActionPromptUpdate),
	  _invokeRoomScriptDispatchCallback(invokeRoomScriptDispatchCallback),
	  _waitForRoomVerticalBlankFunctor(this, &RoomScriptCommandExecutor::waitForRoomVerticalBlank),
	  _requestMovementHostCloseFunctor(this, &RoomScriptCommandExecutor::requestMovementHostClose),
	  _requestSessionExitFunctor(this, &RoomScriptCommandExecutor::requestSessionExit),
	  _command00Functor(this, &RoomScriptCommandExecutor::executeRoomScriptCommand00),
	  _command01Functor(this, &RoomScriptCommandExecutor::executeRoomScriptCommand01),
	  _command03Functor(this, &RoomScriptCommandExecutor::executeRoomScriptCommand03),
	  _command04Functor(this, &RoomScriptCommandExecutor::executeRoomScriptCommand04),
	  _command06Functor(this, &RoomScriptCommandExecutor::executeRoomScriptCommand06),
	  _command07Functor(this, &RoomScriptCommandExecutor::executeRoomScriptCommand07),
	  _command0AFunctor(this, &RoomScriptCommandExecutor::executeRoomScriptCommand0A),
	  _command0CFunctor(this, &RoomScriptCommandExecutor::executeRoomScriptCommand0C),
	  _command0DFunctor(this, &RoomScriptCommandExecutor::executeRoomScriptCommand0D),
	  _command11Functor(this, &RoomScriptCommandExecutor::executeRoomScriptCommand11),
	  _command12Functor(this, &RoomScriptCommandExecutor::executeRoomScriptCommand12),
	  _command13Functor(this, &RoomScriptCommandExecutor::executeRoomScriptCommand13),
	  _command14Functor(this, &RoomScriptCommandExecutor::executeRoomScriptCommand14),
	  _command15Functor(this, &RoomScriptCommandExecutor::executeRoomScriptCommand15),
	  _command16Functor(this, &RoomScriptCommandExecutor::executeRoomScriptCommand16),
	  _command17Functor(this, &RoomScriptCommandExecutor::executeRoomScriptCommand17),
	  _command18Functor(this, &RoomScriptCommandExecutor::executeRoomScriptCommand18),
	  _command02Functor(&_command02Executor, &RoomScriptCommand02Executor::executeRoomScriptCommand02),
	  _command05Functor(&_command05Executor, &RoomScriptCommand05Executor::executeRoomScriptCommand05),
	  _command08Functor(&_command08Executor, &RoomScriptCommand08Executor::executeRoomScriptCommand08),
	  _command09Functor(&_command09Executor, &RoomScriptCommand09Executor::executeRoomScriptCommand09),
	  _command0BFunctor(&_command0BExecutor, &RoomScriptCommand0BExecutor::executeRoomScriptCommand0B),
	  _command0EFunctor(&_command0EOr1BExecutor,
						&RoomScriptCommand0EOr1BExecutor::executeRoomScriptCommand0EOr1B),
	  _command0FFunctor(&_command0FExecutor, &RoomScriptCommand0FExecutor::executeRoomScriptCommand0F),
	  _command10Functor(&_command10Executor, &RoomScriptCommand10Executor::executeRoomScriptCommand10),
	  _command19Functor(&_command19Executor, &RoomScriptCommand19Executor::executeRoomScriptCommand19),
	  _command1AFunctor(&_command1AExecutor, &RoomScriptCommand1AExecutor::executeRoomScriptCommand1A),
	  _command1CFunctor(&_command1CExecutor, &RoomScriptCommand1CExecutor::executeRoomScriptCommand1C),
	  _command1DFunctor(&_command1DExecutor, &RoomScriptCommand1DExecutor::executeRoomScriptCommand1D),
	  _command1EFunctor(&_command1EExecutor, &RoomScriptCommand1EExecutor::executeRoomScriptCommand1E),
	  _command1FFunctor(&_command1FExecutor, &RoomScriptCommand1FExecutor::executeRoomScriptCommand1F),
	  _command20Functor(&_command20Executor, &RoomScriptCommand20Executor::executeRoomScriptCommand20),
	  _command21Functor(&_command21Executor, &RoomScriptCommand21Executor::executeRoomScriptCommand21),
	  _actorDescriptorCatalog(rom), _command01Executor(rom, state), _command02Executor(rom, state),
	  _command05Executor(rom, state, _actorDescriptorCatalog, updateActorAnimationFrames,
						 _waitForRoomVerticalBlankFunctor, commitActionPromptUpdate, refreshActionPrompts),
	  _command08Executor(rom, state, random), _command09Executor(rom, state, drawSelectedActionIcon),
	  _command0BExecutor(rom, state, _waitForRoomVerticalBlankFunctor),
	  _command0EOr1BExecutor(rom, state, updateActorAnimationFrames, _waitForRoomVerticalBlankFunctor,
							 movement, _requestMovementHostCloseFunctor),
	  _command0FExecutor(rom, state, _actorDescriptorCatalog, movement, _requestMovementHostCloseFunctor),
	  _command10Executor(rom, scene, state, dialogue, updateActorAnimationFrames,
						 _waitForRoomVerticalBlankFunctor, _requestSessionExitFunctor),
	  _command19Executor(rom, state, updateActorAnimationFrames, _waitForRoomVerticalBlankFunctor),
	  _command1AExecutor(rom, scene, state), _command1CExecutor(rom, state), _command1DExecutor(rom, state),
	  _command1EExecutor(rom, state, presenter, updateActorAnimationFrames, handleRoomVBlank,
						 _requestSessionExitFunctor),
	  _command1FExecutor(rom, state, presenter, updateActorAnimationFrames, handleRoomVBlank,
						 _requestSessionExitFunctor),
	  _command20Executor(rom, state), _command21Executor(rom, state),
	  _actions(rom, scene, state, collisionProbe, random, presenter, updateActorAnimationFrames,
			   handleRoomVBlank, presentRoom, processAutomaticInteractions, refreshInteractionDisplay,
			   finishRoomStartup, executeRoomScriptAction1D, tileStreamer, restartApplication),
	  _conditionEvaluator(rom, state), _handlers(kHandlerCount), _interactionModeSnapshot(0) {
	_handlers[0x00] = &_command00Functor;
	_handlers[0x01] = &_command01Functor;
	_handlers[0x02] = &_command02Functor;
	_handlers[0x03] = &_command03Functor;
	_handlers[0x04] = &_command04Functor;
	_handlers[0x05] = &_command05Functor;
	_handlers[0x06] = &_command06Functor;
	_handlers[0x07] = &_command07Functor;
	_handlers[0x08] = &_command08Functor;
	_handlers[0x09] = &_command09Functor;
	_handlers[0x0A] = &_command0AFunctor;
	_handlers[0x0B] = &_command0BFunctor;
	_handlers[0x0C] = &_command0CFunctor;
	_handlers[0x0D] = &_command0DFunctor;
	_handlers[0x0E] = &_command0EFunctor;
	_handlers[0x0F] = &_command0FFunctor;
	_handlers[0x10] = &_command10Functor;
	_handlers[0x11] = &_command11Functor;
	_handlers[0x12] = &_command12Functor;
	_handlers[0x13] = &_command13Functor;
	_handlers[0x14] = &_command14Functor;
	_handlers[0x15] = &_command15Functor;
	_handlers[0x16] = &_command16Functor;
	_handlers[0x17] = &_command17Functor;
	_handlers[0x18] = &_command18Functor;
	_handlers[0x19] = &_command19Functor;
	_handlers[0x1A] = &_command1AFunctor;
	_handlers[0x1B] = &_command0EFunctor;
	_handlers[0x1C] = &_command1CFunctor;
	_handlers[0x1D] = &_command1DFunctor;
	_handlers[0x1E] = &_command1EFunctor;
	_handlers[0x1F] = &_command1FFunctor;
	_handlers[0x20] = &_command20Functor;
	_handlers[0x21] = &_command21Functor;
}

Optional<SessionExit> RoomScriptCommandExecutor::execute(uint16 command, int mode,
														 int16 interactionModeSnapshot) {
	_interactionModeSnapshot = interactionModeSnapshot;
	(*_handlers[static_cast<std::size_t>(command)])(mode);
	return _requestedSessionExit;
}

void RoomScriptCommandExecutor::executeRoomScriptCommand00(int mode) {
	(void)mode;

	// Ghidra 0x0000250C-0x0000250D: test the current command word solely for A5's post-increment;
	// the dispatcher has already read the word, so preserve only the two-byte cursor advance.
	_state.RoomScriptStartOffset += static_cast<int>(sizeof(uint16));

	// Ghidra 0x0000250E-0x0000250F: return without consuming the inherited mode or changing other state.
}

void RoomScriptCommandExecutor::executeRoomScriptCommand03(int mode) {
	(void)mode;
	int commandOffset = _state.RoomScriptStartOffset;

	// Ghidra 0x00002732-0x00002739: evaluate the complete shared condition and branch on its Z result.
	if (_conditionEvaluator.evaluateRoomScriptCondition()) {
		// Ghidra 0x0000273A-0x0000273F: true consumes the fixed 16-byte condition record and returns.
		_state.RoomScriptStartOffset = commandOffset + 16;
	} else {
		// Ghidra 0x00002740-0x00002749: false sign-extends record field +2 as the replacement A5 offset.
		_state.RoomScriptStartOffset = commandOffset + _rom.readInt16(commandOffset + 2);
	}
}

void RoomScriptCommandExecutor::executeRoomScriptCommand04(int mode) {
	int commandOffset = _state.RoomScriptStartOffset;

	// Ghidra 0x0000274A-0x00002751: evaluate the complete shared condition and branch to the authored
	// signed failure displacement when it is false.
	if (!_conditionEvaluator.evaluateRoomScriptCondition()) {
		// Ghidra 0x0000278C-0x00002795: false replaces A5 with command start plus signed field +2.
		_state.RoomScriptStartOffset = commandOffset + _rom.readInt16(commandOffset + 2);
		return;
	}

	// Ghidra 0x00002752-0x00002769: save the parent cursors, install the nested exclusive bound from signed
	// field +2, start at fixed record offset +0x10, and invoke the currently installed script dispatcher.
	int parentEndOffset = _state.RoomScriptEndOffset;
	_state.RoomScriptEndOffset = commandOffset + _rom.readInt16(commandOffset + 2);
	_state.RoomScriptStartOffset = commandOffset + 16;
	_requestedSessionExit = _invokeRoomScriptDispatchCallback(mode);
	if (_requestedSessionExit.hasValue()) {
		// A native non-returning nested command never reaches either original callback-return path.
		return;
	}

	// Ghidra 0x0000276A-0x0000277B: bit zero discards the saved parent cursors and returns with the nested
	// dispatcher's A5 and A6 values still active.
	if ((_state.AutomaticInteractionFlags & 0x01) != 0) {
		return;
	}

	// Ghidra 0x0000277C-0x0000278B: preserve the nested post-dispatch A5, restore the parent A5/A6 pair,
	// then replace A5 with the nested cursor plus the parent's signed field +4.
	int nestedCursorOffset = _state.RoomScriptStartOffset;
	_state.RoomScriptEndOffset = parentEndOffset;
	_state.RoomScriptStartOffset = nestedCursorOffset + _rom.readInt16(commandOffset + 4);
}

void RoomScriptCommandExecutor::executeRoomScriptCommand06(int mode) {
	int commandOffset = _state.RoomScriptStartOffset;

	// Ghidra 0x00002B5A-0x00002B5D: mode zero retains the common six-byte cursor advance.
	if (mode != 0) {
		uint16 objectIdentity = _rom.readUInt16(commandOffset + 2);
		int16 animationOffset = _rom.readInt16(commandOffset + 4);
		switch (objectIdentity) {
		case 1:
			// Ghidra 0x00002B5E-0x00002B75: identity one selects actor slot zero and joins the
			// restart-pending loop without passing through the identity-two comparison.
			_state.ActorAnimationOffsets[0] = animationOffset;
			_state.ActorAnimationRestartFlags |= 0x01;
			while (true) {
				_updateActorAnimationFrames();
				if ((_state.ActorAnimationRestartFlags & 0x01) == 0) {
					break;
				}

				if (!waitForRoomVerticalBlank()) {
					return;
				}
			}

			break;
		case 2:
			// Ghidra 0x00002B76-0x00002B9F: identity two selects actor slot one; both outcomes of
			// the restart-bit test remain in the loop before the common epilogue.
			_state.ActorAnimationOffsets[1] = animationOffset;
			_state.ActorAnimationRestartFlags |= 0x02;
			while (true) {
				_updateActorAnimationFrames();
				if ((_state.ActorAnimationRestartFlags & 0x02) == 0) {
					break;
				}

				if (!waitForRoomVerticalBlank()) {
					return;
				}
			}

			break;
		default: {
			// Ghidra 0x00002BA0-0x00002BB3: every identity other than one or two retains the
			// low-word 0x1A-byte object-table calculation and the signed fixed-position-index branch.
			uint16 tableByteOffset = static_cast<uint16>((objectIdentity - 3) * 0x1A);
			RoomObject &roomObject = _state.RoomObjects[static_cast<std::size_t>(tableByteOffset / 0x1A)];
			if (roomObject.FixedPositionIndex >= 0) {
				// Ghidra 0x00002BB4-0x00002BEB: retarget the corresponding actor slot, force its
				// delay due, request restart, and retain both graphics-ready loop outcomes.
				int actorSlot = static_cast<uint8>(roomObject.FixedPositionIndex) + 2;
				uint8 actorMask = static_cast<uint8>(1 << (actorSlot & 0x07));
				_state.ActorAnimationOffsets[actorSlot] = animationOffset;
				_state.ActorAnimationDelays[actorSlot] = -1;
				_state.ActorAnimationRestartFlags |= actorMask;
				while (true) {
					_updateActorAnimationFrames();
					if ((_state.ActorGraphicsReadyFlags & actorMask) != 0) {
						break;
					}

					if (!waitForRoomVerticalBlank()) {
						return;
					}
				}
			}

			// Ghidra 0x00002BEC-0x00002BF1: the indexed destination extension writes record +0x00
			// for both signed fixed-position-index outcomes; UpdateActorAnimationFrames preserves D0/A0.
			roomObject.TilePatchIndex = animationOffset;
			break;
		}
		}
	}

	// Ghidra 0x00002BF2-0x00002BF7: every native return path consumes the command and its two arguments.
	_state.RoomScriptStartOffset = commandOffset + 6;
}

void RoomScriptCommandExecutor::executeRoomScriptCommand07(int mode) {
	int commandOffset = _state.RoomScriptStartOffset;

	// Ghidra 0x00002BF8-0x00002BFB: mode zero retains the common four-byte cursor advance.
	if (mode != 0) {
		// Ghidra 0x00002BFC-0x00002C1F: retain the low-word object-table calculation and both active-room
		// outcomes. A match sets only object flag bit zero and skips every inventory branch.
		uint16 objectIdentity = _rom.readUInt16(commandOffset + 2);
		uint16 tableByteOffset = static_cast<uint16>((objectIdentity - 3) * 0x1A);
		RoomObject &roomObject = _state.RoomObjects[static_cast<std::size_t>(tableByteOffset / 0x1A)];
		if (roomObject.RoomId == _state.RoomId) {
			roomObject.Flags |= 0x01;
		}
		// Ghidra 0x00002C20-0x00002C27: a non-active object enters page handling only for room ID one.
		else if (roomObject.RoomId == 1) {
			// Ghidra 0x00002C28-0x00002C37: count every nonnegative inventory word through the negative
			// sentinel. The vector's size is the typed representation of the same complete ordered scan.
			uint16 inventoryCount = static_cast<uint16>(_state.InventoryObjectIndices.size());

			// Ghidra 0x00002C38-0x00002C4F: preserve the logical word shift, wrapped MenuPage-minus-one
			// comparison, unchanged-page exit, and changed-page write before the prompt commit call.
			uint16 inventoryPage = static_cast<uint16>(inventoryCount >> 2);
			if (inventoryPage != static_cast<uint16>(_state.MenuPage - 1)) {
				_state.MenuPage = static_cast<int16>(inventoryPage);
				_commitActionPromptUpdate();
			}
		}
	}

	// Ghidra 0x00002C50-0x00002C55: every native return path consumes the command and its argument.
	_state.RoomScriptStartOffset = commandOffset + 4;
}

void RoomScriptCommandExecutor::executeRoomScriptCommand0A(int mode) {
	int commandOffset = _state.RoomScriptStartOffset;

	// Ghidra 0x00002DC0-0x00002DC3: dispatcher mode zero skips the complete object and prompt update.
	if (mode != 0) {
		// Ghidra 0x00002DC4-0x00002DD9: retain the unsigned word multiply and low-word object-table index,
		// then replace the complete inventory-graphic word from command field +4.
		uint16 objectIdentity = _rom.readUInt16(commandOffset + 2);
		uint16 tableByteOffset = static_cast<uint16>((objectIdentity - 3) * 0x1A);
		RoomObject &roomObject = _state.RoomObjects[static_cast<std::size_t>(tableByteOffset / 0x1A)];
		roomObject.InventoryGraphicIndex = _rom.readInt16(commandOffset + 4);

		// Ghidra 0x00002DDA-0x00002DE3: preserve both room-ID outcomes; only inventory room one calls on.
		if (roomObject.RoomId == 1) {
			// Ghidra 0x00002DE4-0x00002DEB: wait exactly one retrace before committing the prompt update.
			// Host closure replaces this native returning path with the managed session-exit handover.
			if (!waitForRoomVerticalBlank()) {
				return;
			}

			_commitActionPromptUpdate();
		}
	}

	// Ghidra 0x00002DEC-0x00002DF1: every native mode and room branch consumes the six-byte command.
	_state.RoomScriptStartOffset = commandOffset + 6;
}

void RoomScriptCommandExecutor::executeRoomScriptCommand0C(int mode) {
	int commandOffset = _state.RoomScriptStartOffset;

	// Ghidra 0x00002E9E-0x00002EAD: mode zero skips dispatch; a nonzero mode selects the nested action
	// from the managed equivalent of g_apRoomScriptActionHandlers.
	if (mode != 0) {
		_requestedSessionExit = _actions.execute(_rom.readUInt16(commandOffset + 2));
		if (_requestedSessionExit.hasValue()) {
			// The original non-returning action never reaches this command's shared cursor advance.
			return;
		}
	}

	// Ghidra 0x00002EAE-0x00002EB3: consume the command and action-index words in either mode.
	_state.RoomScriptStartOffset = commandOffset + 4;
}

void RoomScriptCommandExecutor::executeRoomScriptCommand0D(int mode) {
	int commandOffset = _state.RoomScriptStartOffset;

	// Ghidra 0x000048B6-0x000048B9: dispatcher mode zero skips both audio call branches.
	if (mode != 0) {
		// Ghidra 0x000048BA-0x000048BF: retain both outcomes of the signed command-word test.
		int16 encodedCommand = _rom.readInt16(commandOffset + 2);
		if (encodedCommand < 0) {
			// Ghidra 0x000048C0-0x000048CB: clear only bit 15, extend the result, and call the silent
			// StopAudioPlayback handover before bypassing the play branch.
			stopAudioPlayback(static_cast<uint16>(encodedCommand) & 0x7FFF);
		} else {
			// Ghidra 0x000048CC-0x000048D1: extend the nonnegative word unchanged and call the silent
			// PlayAudioCommand handover.
			playAudioCommand(encodedCommand);
		}
	}

	// Ghidra 0x000048D2-0x000048D7: every mode and sign branch consumes the four-byte command.
	_state.RoomScriptStartOffset = commandOffset + 4;
}

void RoomScriptCommandExecutor::playAudioCommand(int command) {
	// AUDIO FRONTIER: Preserve command ordering while the PC audio domain remains deliberately deferred.
	(void)command;
}

void RoomScriptCommandExecutor::stopAudioPlayback(int command) {
	// AUDIO FRONTIER: Preserve command ordering while the PC audio domain remains deliberately deferred.
	(void)command;
}

void RoomScriptCommandExecutor::executeRoomScriptCommand11(int mode) {
	int commandOffset = _state.RoomScriptStartOffset;

	// Ghidra 0x00004F0E-0x00004F11: scan mode skips every state and presentation effect.
	if (mode != 0) {
		// Ghidra 0x00004F12-0x00004F21: replace the room identity and lead actor's signed position index
		// independently from the coordinate-table offset between them in the record.
		_state.RoomId = _rom.readInt16(commandOffset + 2);
		_state.ActorPositionIndices[0] = _rom.readInt16(commandOffset + 6);

		// Ghidra 0x00004F22-0x00004F39: save every display flag, clear only the room-update gate during
		// the complete callback-aware fade, and restore the exact byte before either continuation.
		uint8 displayFlags = _state.DisplayFlags;
		_state.DisplayFlags &= 0xBF;
		bool fadeCompleted = _presenter.fadePaletteOut(&_handleRoomVBlank);
		_state.DisplayFlags = displayFlags;
		if (!fadeCompleted) {
			_requestedSessionExit = SessionExit::HostClosed;
			return;
		}

		// Ghidra 0x00004F3A-0x00004F49: scale the coordinate index by four with word wrapping, publish
		// the initial coordinate byte offset, and invoke the separate room-reload owner.
		_state.InitialRoomPositionCoordinateOffset =
			static_cast<int16>(_rom.readUInt16(commandOffset + 4) << 2);
		_requestedSessionExit = _reloadAndPresentRoom();
		if (_requestedSessionExit.hasValue()) {
			// A native non-returning descendant never reaches this handler's final A5 advance.
			return;
		}
	}

	// Ghidra 0x00004F4A-0x00004F4F: every native mode consumes the complete eight-byte record.
	_state.RoomScriptStartOffset = commandOffset + 8;
}

void RoomScriptCommandExecutor::executeRoomScriptCommand12(int mode) {
	int commandOffset = _state.RoomScriptStartOffset;

	// Ghidra 0x00004F50-0x00004F53: scan mode skips only the room-object write.
	if (mode != 0) {
		// Ghidra 0x00004F54-0x00004F69: subtract the three-identity prefix, retain the signed low word of
		// the 0x1A-byte product used by indexed addressing, and replace shape word +0x02 unconditionally.
		uint16 objectIdentity = _rom.readUInt16(commandOffset + 2);
		int16 tableByteOffset = static_cast<int16>((objectIdentity - 3) * 0x1A);
		_state.RoomObjects[static_cast<std::size_t>(tableByteOffset / 0x1A)].ShapeIndex =
			_rom.readInt16(commandOffset + 4);
	}

	// Ghidra 0x00004F6A-0x00004F6F: every native mode consumes the complete six-byte command.
	_state.RoomScriptStartOffset = commandOffset + 6;
}

void RoomScriptCommandExecutor::executeRoomScriptCommand13(int mode) {
	int commandOffset = _state.RoomScriptStartOffset;

	// Ghidra 0x00002446-0x0000245F: execution mode skips the flag test; scan mode with bit four set
	// latches only stop bit zero and returns with the exact command cursor unchanged.
	if (mode == 0 && (_state.AutomaticInteractionFlags & 0x10) != 0) {
		_state.AutomaticInteractionFlags |= 0x01;
		return;
	}

	// Ghidra 0x00002460-0x00002469: every other path adds the signed field at +0x02 to the original
	// command cursor without imposing a fixed record size or a defensive progress requirement.
	_state.RoomScriptStartOffset = commandOffset + _rom.readInt16(commandOffset + 2);
}

void RoomScriptCommandExecutor::executeRoomScriptCommand14(int mode) {
	int commandOffset = _state.RoomScriptStartOffset;

	// Ghidra 0x00002E7E-0x00002E81: scan mode skips only the room-object write.
	if (mode != 0) {
		// Ghidra 0x00002E82-0x00002E97: subtract the three-identity prefix, retain the signed low word of
		// the 0x1A-byte product used by indexed addressing, and replace shape word +0x02 unconditionally.
		uint16 objectIdentity = _rom.readUInt16(commandOffset + 2);
		int16 tableByteOffset = static_cast<int16>((objectIdentity - 3) * 0x1A);
		_state.RoomObjects[static_cast<std::size_t>(tableByteOffset / 0x1A)].ShapeIndex =
			_rom.readInt16(commandOffset + 4);
	}

	// Ghidra 0x00002E98-0x00002E9D: every native mode consumes the complete six-byte command.
	_state.RoomScriptStartOffset = commandOffset + 6;
}

void RoomScriptCommandExecutor::executeRoomScriptCommand15(int mode) {
	int commandOffset = _state.RoomScriptStartOffset;

	// Ghidra 0x000048D8-0x000048DB: scan mode skips the complete countdown and animation loop.
	if (mode != 0) {
		// Ghidra 0x000048DC-0x000048F1: copy the complete signed word, update animations before every
		// signed test, and retain the back edge until the installed callback decrements through -1.
		_state.RetraceCountdown = _rom.readInt16(commandOffset + 2);
		while (true) {
			_updateActorAnimationFrames();
			if (_state.RetraceCountdown < 0) {
				break;
			}

			if (!waitForRoomVerticalBlank()) {
				return;
			}
		}
	}

	// Ghidra 0x000048F2-0x000048F7: every native mode consumes the complete four-byte command.
	_state.RoomScriptStartOffset = commandOffset + 4;
}

void RoomScriptCommandExecutor::executeRoomScriptCommand16(int mode) {
	int commandOffset = _state.RoomScriptStartOffset;

	// Ghidra 0x00002AD0-0x00002AD3: scan mode skips every actor-selection and visibility effect.
	if (mode != 0) {
		// Ghidra 0x00002AD4-0x00002AE7: identities one and two select actor bits zero and one directly.
		uint16 objectIdentity = _rom.readUInt16(commandOffset + 2);
		int actorSlot;
		if (objectIdentity == 1) {
			actorSlot = 0;
		} else if (objectIdentity == 2) {
			actorSlot = 1;
		} else {
			// Ghidra 0x00002AE8-0x00002AF9: every other identity retains the signed low word of the
			// 0x1A-byte table offset, reads byte +0x19, adds two, and keeps its modulo-eight bit number.
			int16 tableByteOffset = static_cast<int16>((objectIdentity - 3) * 0x1A);
			RoomObject &roomObject = _state.RoomObjects[static_cast<std::size_t>(tableByteOffset / 0x1A)];
			actorSlot = (static_cast<uint8>(roomObject.FixedPositionIndex) + 2) & 0x07;
		}

		// Ghidra 0x00002AFA-0x00002B07: managed callback execution is non-reentrant, so one byte update
		// replaces the interrupt-masked dynamic bit set while preserving all other actor slots.
		_state.ActorVisibilityBlockFlags |= static_cast<uint8>(1 << actorSlot);
	}

	// Ghidra 0x00002B08-0x00002B0D: every native mode and identity consumes the four-byte command.
	_state.RoomScriptStartOffset = commandOffset + 4;
}

void RoomScriptCommandExecutor::executeRoomScriptCommand17(int mode) {
	int commandOffset = _state.RoomScriptStartOffset;

	// Ghidra 0x00004FBE-0x00004FC1: scan mode skips every actor-selection and animation effect.
	if (mode != 0) {
		// Ghidra 0x00004FC2-0x00004FD7: identities one and two select actor bits zero and one directly.
		uint16 objectIdentity = _rom.readUInt16(commandOffset + 2);
		int actorSlot;
		if (objectIdentity == 1) {
			actorSlot = 0;
		} else if (objectIdentity == 2) {
			actorSlot = 1;
		} else {
			// Ghidra 0x00004FD8-0x00004FED: every other identity retains the signed low-word table
			// offset, zero-extends byte +0x19, adds two, and keeps the modulo-eight bit number.
			int16 tableByteOffset = static_cast<int16>((objectIdentity - 3) * 0x1A);
			RoomObject &roomObject = _state.RoomObjects[static_cast<std::size_t>(tableByteOffset / 0x1A)];
			actorSlot = (static_cast<uint8>(roomObject.FixedPositionIndex) + 2) & 0x07;
		}

		// Ghidra 0x00004FEE-0x00004FFB: update every actor at least once and retain the back edge until
		// the selected actor's animation command stream sets its hold bit. Native room VBlank can interrupt
		// this loop to decrement animation delays; service and present that retrace before each continuation.
		uint8 actorMask = static_cast<uint8>(1 << actorSlot);
		while (true) {
			_updateActorAnimationFrames();
			if ((_state.ActorAnimationHoldFlags & actorMask) != 0) {
				break;
			}

			if (!waitForRoomVerticalBlank()) {
				return;
			}
		}
	}

	// Ghidra 0x00004FFC-0x00005001: every native mode and identity consumes the four-byte command.
	_state.RoomScriptStartOffset = commandOffset + 4;
}

bool RoomScriptCommandExecutor::waitForRoomVerticalBlank() {
	_handleRoomVBlank();
	if (_presenter.waitForVerticalBlank()) {
		return true;
	}

	_requestedSessionExit = SessionExit::HostClosed;
	return false;
}

void RoomScriptCommandExecutor::executeRoomScriptCommand18(int mode) {
	int commandOffset = _state.RoomScriptStartOffset;

	// Ghidra 0x00004EE4-0x00004EE7: scan mode skips every room-selection and reset effect.
	if (mode != 0) {
		// Ghidra 0x00004EE8-0x00004EF7: replace the room identity and lead actor's signed position index
		// independently from the coordinate-table offset between them in the command record.
		_state.RoomId = _rom.readInt16(commandOffset + 2);
		_state.ActorPositionIndices[0] = _rom.readInt16(commandOffset + 6);

		// Ghidra 0x00004EF8-0x00004F07: scale the coordinate index by four with word wrapping, publish
		// the signed initial byte offset, and retain the separate room-state reset function boundary.
		_state.InitialRoomPositionCoordinateOffset =
			static_cast<int16>(_rom.readUInt16(commandOffset + 4) << 2);
		_requestedSessionExit = _resetRoomStateForReload();
		if (_requestedSessionExit.hasValue()) {
			// A native returning reset has no host-close path; the managed wait hands that result outward.
			return;
		}
	}

	// Ghidra 0x00004F08-0x00004F0D: every native mode consumes the complete eight-byte command.
	_state.RoomScriptStartOffset = commandOffset + 8;
}
} // namespace Scooby
