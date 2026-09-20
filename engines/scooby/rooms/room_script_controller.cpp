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

#include "room_script_controller.h"

namespace Scooby {

RoomScriptController::RoomScriptController(
	const ScoobyDooRom &rom, TileScene &scene, RuntimeState &state,
	const RoomCollisionProbe &collisionProbe, ScriptedRoomMovementExecutor &movement,
	DeterministicRandom &random, FramePresenter &presenter,
	Common::Functor0<void> &updateActorAnimationFrames, Common::Functor0<void> &handleRoomVBlank,
	Common::Functor0<Optional<SessionExit>> &presentRoom,
	Common::Functor0<Optional<SessionExit>> &resetRoomStateForReload,
	Common::Functor0<Optional<SessionExit>> &reloadAndPresentRoom, InteractionController &interactions,
	Common::Functor0<void> &refreshInteractionDisplay, Common::Functor0<void> &finishRoomStartup,
	Common::Functor0<void> &executeRoomScriptAction1D, DialogueController &dialogue,
	Common::Functor0<void> &commitActionPromptUpdate, Common::Functor0<void> &refreshActionPrompts,
	Common::Functor1<int, void> &drawSelectedActionIcon, RoomTileStreamer &tileStreamer,
	Common::Functor1<const Common::Functor0<void> *, bool> &restartApplication)
	: _rom(rom), _state(state), _interactions(interactions),
	  _processAutomaticInteractionsFunctor(&_interactions,
										   &InteractionController::processAutomaticInteractions),
	  _invokeRoomScriptDispatchCallbackFunctor(this, &RoomScriptController::invokeRoomScriptDispatchCallback),
	  _runRoomScriptInitializationCommandsFunctor(
		  this, &RoomScriptController::runRoomScriptInitializationCommands),
	  _commands(rom, scene, state, collisionProbe, movement, random, presenter, updateActorAnimationFrames,
				handleRoomVBlank, presentRoom, resetRoomStateForReload, reloadAndPresentRoom,
				_processAutomaticInteractionsFunctor, refreshInteractionDisplay, finishRoomStartup,
				executeRoomScriptAction1D, dialogue, _invokeRoomScriptDispatchCallbackFunctor,
				commitActionPromptUpdate, refreshActionPrompts, drawSelectedActionIcon, tileStreamer,
				restartApplication) {
	_interactions.bindRoomScriptDispatch(_invokeRoomScriptDispatchCallbackFunctor,
										 _runRoomScriptInitializationCommandsFunctor);
}

Optional<SessionExit> RoomScriptController::runRoomScriptInitializationCommands() {
	// Ghidra 0x00002320-0x0000232F: overwrite the complete automatic-interaction byte with initialization
	// gate two and independently clear every registered action by resetting its signed count.
	_state.AutomaticInteractionFlags = 0x04;
	_state.InteractionActions.reset();

	// Ghidra 0x00002330-0x00002343: preserve both script offsets. The explicit mode argument below replaces
	// saving and installing the mutable native callback pointer without introducing code-address state.
	int scriptStartOffset = _state.RoomScriptStartOffset;
	int scriptEndOffset = _state.RoomScriptEndOffset;

	// Ghidra 0x00002344-0x00002347: invoke the separate scan dispatcher exactly once. A managed
	// non-returning descendant bypasses the native restoration range just as its original target did.
	Optional<SessionExit> requestedExit = dispatchRoomScriptCommands();
	if (requestedExit.hasValue()) {
		return requestedExit;
	}

	// Ghidra 0x00002348-0x00002353: explicit mode needs no callback restoration; restore A5 and A6 in
	// their native order and return while retaining every other handler update.
	_state.RoomScriptStartOffset = scriptStartOffset;
	_state.RoomScriptEndOffset = scriptEndOffset;
	return Optional<SessionExit>();
}

Optional<SessionExit> RoomScriptController::dispatchRoomScriptCommands() {
	while (true) {
		// Ghidra 0x000023DC-0x000023E1: compare the complete cursor addresses and return whenever the
		// exclusive bound is at or below the current command, including on the first iteration.
		if (_state.RoomScriptEndOffset <= _state.RoomScriptStartOffset) {
			return Optional<SessionExit>();
		}

		// Ghidra 0x000023E2-0x000023F3: read the command without consuming it, select its original table
		// entry, clear D0 to choose scan mode, and preserve a managed non-returning descendant handover.
		uint16 command = _rom.readUInt16(_state.RoomScriptStartOffset);
		Optional<SessionExit> requestedExit = _commands.execute(
			command, 0, _state.InteractionMode);
		if (requestedExit.hasValue()) {
			return requestedExit;
		}

		// Ghidra 0x000023F4-0x00002405: marker bit zero returns immediately; its clear branch always
		// repeats through the bound comparison without assuming that the selected handler advanced A5.
		if ((_state.AutomaticInteractionFlags & 0x01) != 0) {
			return Optional<SessionExit>();
		}
	}
}

Optional<SessionExit> RoomScriptController::invokeRoomScriptDispatchCallback(int mode) {
	if (mode == 0) {
		return dispatchRoomScriptCommands();
	}

	return runRoomScriptCommands();
}

Optional<SessionExit> RoomScriptController::runRoomExitScript() {
	// Ghidra 0x0000221A-0x0000222F: retain the caller's display byte. Managed execution mode replaces
	// saving and installing the mutable original callback pointer.
	uint8 previousDisplayFlags = _state.DisplayFlags;

	// Ghidra 0x00002230-0x00002257: preserve low-word room-index multiplication, signed indexed
	// displacement, both descriptor fields, and the four-byte length-prefixed script header.
	int16 roomRecordByteOffset =
		static_cast<int16>(static_cast<uint16>(_state.RoomId - 2) * kRoomRecordByteCount);
	int roomRecordTableOffset =
		static_cast<int>(_rom.readUInt32(_state.ActiveEpisodeDescriptorOffset + kEpisodeRoomRecordTableField));
	int relativeScriptOffset =
		static_cast<int>(_rom.readUInt32(roomRecordTableOffset + roomRecordByteOffset + kRoomExitScriptField));
	int scriptHeaderOffset =
		static_cast<int>(_rom.readUInt32(_state.ActiveEpisodeDescriptorOffset + kEpisodeRoomScriptBaseField)) +
		relativeScriptOffset;
	int16 scriptLength = _rom.readInt16(scriptHeaderOffset);
	_state.RoomScriptStartOffset = scriptHeaderOffset + kRoomScriptHeaderByteCount;
	_state.RoomScriptEndOffset = _state.RoomScriptStartOffset + scriptLength;

	// Ghidra 0x00002258-0x00002263: force display bit seven during the complete execution-mode script.
	_state.DisplayFlags |= 0x80;
	Optional<SessionExit> requestedExit = runRoomScriptCommands();
	if (requestedExit.hasValue()) {
		// Original non-returning targets never reach native restoration; hand their managed exit outward.
		return requestedExit;
	}

	// Ghidra 0x00002264-0x00002277: OR only the saved bit seven into the handler-updated display byte.
	// The runtime-owned mode selection needs no executable callback-pointer restoration.
	_state.DisplayFlags |= static_cast<uint8>(previousDisplayFlags & 0x80);
	return Optional<SessionExit>();
}

Optional<SessionExit> RoomScriptController::runRoomEntryScript() {
	// Ghidra 0x00002278-0x0000228D: save the callback pointer and display byte, then install execution-mode
	// dispatch. The managed call is already explicit, so only the observable display snapshot remains.
	uint8 previousDisplayFlags = _state.DisplayFlags;

	// Ghidra 0x0000228E-0x000022B5: preserve unsigned low-word room-index multiplication, signed indexed
	// displacement, both descriptor fields, and the complete four-byte length-prefixed script header.
	int16 roomRecordByteOffset =
		static_cast<int16>(static_cast<uint16>(_state.RoomId - 2) * kRoomRecordByteCount);
	int roomRecordTableOffset =
		static_cast<int>(_rom.readUInt32(_state.ActiveEpisodeDescriptorOffset + kEpisodeRoomRecordTableField));
	int relativeScriptOffset =
		static_cast<int>(_rom.readUInt32(roomRecordTableOffset + roomRecordByteOffset + kRoomEntryScriptField));
	int scriptHeaderOffset =
		static_cast<int>(_rom.readUInt32(_state.ActiveEpisodeDescriptorOffset + kEpisodeRoomScriptBaseField)) +
		relativeScriptOffset;
	int16 scriptLength = _rom.readInt16(scriptHeaderOffset);
	_state.RoomScriptStartOffset = scriptHeaderOffset + kRoomScriptHeaderByteCount;
	_state.RoomScriptEndOffset = _state.RoomScriptStartOffset + scriptLength;

	// Ghidra 0x000022B6-0x000022C1: force display bit seven and invoke the execution-mode dispatcher even
	// for an empty script window. Managed non-returning handovers leave before the native restoration path.
	_state.DisplayFlags |= 0x80;
	Optional<SessionExit> requestedExit = runRoomScriptCommands();
	if (requestedExit.hasValue()) {
		return requestedExit;
	}

	// Ghidra 0x000022C2-0x000022D5: OR only the saved bit seven into handler-updated display state. The
	// explicit managed dispatcher needs no callback-pointer restoration.
	_state.DisplayFlags |= static_cast<uint8>(previousDisplayFlags & 0x80);
	return Optional<SessionExit>();
}

Optional<SessionExit> RoomScriptController::runRoomScriptCommands() {
	while (true) {
		// Ghidra 0x00002406-0x00002419: yield when both low room-script control bits are set.
		if ((_state.RoomScriptFlags & 0x03) == 0x03) {
			return Optional<SessionExit>();
		}

		// Ghidra 0x0000241A-0x00002431: stop at the exclusive bound, then dispatch the current
		// big-endian command word with this loop's original D0 mode value one.
		if (_state.RoomScriptEndOffset <= _state.RoomScriptStartOffset) {
			return Optional<SessionExit>();
		}

		uint16 command = _rom.readUInt16(_state.RoomScriptStartOffset);
		Optional<SessionExit> requestedExit = _commands.execute(
			command, 1, _state.InteractionMode);
		if (requestedExit.hasValue()) {
			// A native non-returning target hands process control back to the managed application loop.
			return requestedExit;
		}

		// Ghidra 0x00002432-0x00002445: an automatic-interaction latch yields immediately; otherwise
		// repeat only while the handler-updated cursor remains below the exclusive script bound.
		if ((_state.AutomaticInteractionFlags & 0x01) != 0 || _state.RoomScriptStartOffset >= _state.RoomScriptEndOffset) {
			return Optional<SessionExit>();
		}
	}
}
} // namespace Scooby