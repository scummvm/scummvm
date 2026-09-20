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

#include "room_script_action_executor.h"

#include "common/scummsys.h"

namespace Scooby {

RoomScriptActionExecutor::RoomScriptActionExecutor(
	const ScoobyDooRom &rom, TileScene &scene, RuntimeState &state,
	const RoomCollisionProbe &collisionProbe, DeterministicRandom &random,
	FramePresenter &presenter, Common::Functor0<void> &updateActorAnimationFrames,
	Common::Functor0<void> &handleRoomVBlank, Common::Functor0<Optional<SessionExit>> &presentRoom,
	Common::Functor0<Optional<SessionExit>> &processAutomaticInteractions,
	Common::Functor0<void> &refreshInteractionDisplay, Common::Functor0<void> &finishRoomStartup,
	Common::Functor0<void> &executeRoomScriptAction1D, RoomTileStreamer &tileStreamer,
	Common::Functor1<const Common::Functor0<void> *, bool> &restartApplication)
	: _rom(rom), _state(state), _random(random), _presenter(presenter),
	  _updateActorAnimationFrames(updateActorAnimationFrames), _handleRoomVBlank(handleRoomVBlank),
	  _presentRoom(presentRoom), _processAutomaticInteractions(processAutomaticInteractions),
	  _finishRoomStartup(finishRoomStartup),
	  _waitForRoomVerticalBlankFunctor(this, &RoomScriptActionExecutor::waitForRoomVerticalBlank),
	  _requestSessionExitFunctor(this, &RoomScriptActionExecutor::requestSessionExit),
	  _tryExecuteRoomScriptAction02Functor(this, &RoomScriptActionExecutor::tryExecuteRoomScriptAction02),
	  _action00Functor(this, &RoomScriptActionExecutor::executeRoomScriptAction00),
	  _action01Functor(this, &RoomScriptActionExecutor::executeRoomScriptAction01),
	  _action02Functor(this, &RoomScriptActionExecutor::executeRoomScriptAction02),
	  _action03Functor(this, &RoomScriptActionExecutor::executeRoomScriptAction03),
	  _action04Functor(this, &RoomScriptActionExecutor::executeRoomScriptAction04),
	  _action06Functor(this, &RoomScriptActionExecutor::executeRoomScriptAction06),
	  _action09Functor(this, &RoomScriptActionExecutor::executeRoomScriptAction09),
	  _action0AFunctor(this, &RoomScriptActionExecutor::executeRoomScriptAction0A),
	  _action0BFunctor(this, &RoomScriptActionExecutor::executeRoomScriptAction0B),
	  _action11Functor(this, &RoomScriptActionExecutor::executeRoomScriptAction11),
	  _action12Functor(this, &RoomScriptActionExecutor::executeRoomScriptAction12),
	  _action13Functor(this, &RoomScriptActionExecutor::executeRoomScriptAction13),
	  _action14Functor(this, &RoomScriptActionExecutor::executeRoomScriptAction14),
	  _action1AFunctor(this, &RoomScriptActionExecutor::executeRoomScriptAction1A),
	  _action1BFunctor(this, &RoomScriptActionExecutor::executeRoomScriptAction1B),
	  _action1FFunctor(this, &RoomScriptActionExecutor::executeRoomScriptAction1F),
	  _action20Functor(this, &RoomScriptActionExecutor::executeRoomScriptAction20),
	  _action21Functor(this, &RoomScriptActionExecutor::executeRoomScriptAction21),
	  _action22Functor(this, &RoomScriptActionExecutor::executeRoomScriptAction22),
	  _action23Functor(this, &RoomScriptActionExecutor::executeRoomScriptAction23),
	  _action24Functor(this, &RoomScriptActionExecutor::executeRoomScriptAction24),
	  _action25Functor(this, &RoomScriptActionExecutor::executeRoomScriptAction25),
	  _action26Functor(this, &RoomScriptActionExecutor::executeRoomScriptAction26),
	  _action28Functor(this, &RoomScriptActionExecutor::executeRoomScriptAction28),
	  _action29Functor(this, &RoomScriptActionExecutor::executeRoomScriptAction29),
	  _action2AFunctor(this, &RoomScriptActionExecutor::executeRoomScriptAction2A),
	  _action2BFunctor(this, &RoomScriptActionExecutor::executeRoomScriptAction2B),
	  _action2CFunctor(this, &RoomScriptActionExecutor::executeRoomScriptAction2C),
	  _action2DFunctor(this, &RoomScriptActionExecutor::executeRoomScriptAction2D),
	  _action2EFunctor(this, &RoomScriptActionExecutor::executeRoomScriptAction2E),
	  _action2FFunctor(this, &RoomScriptActionExecutor::executeRoomScriptAction2F),
	  _action30Functor(this, &RoomScriptActionExecutor::executeRoomScriptAction30),
	  _action31Functor(this, &RoomScriptActionExecutor::executeRoomScriptAction31),
	  _action33Functor(this, &RoomScriptActionExecutor::executeRoomScriptAction33),
	  _action34Functor(this, &RoomScriptActionExecutor::executeRoomScriptAction34),
	  _action35Functor(this, &RoomScriptActionExecutor::executeRoomScriptAction35),
	  _action36Functor(this, &RoomScriptActionExecutor::executeRoomScriptAction36),
	  _action37Functor(this, &RoomScriptActionExecutor::executeRoomScriptAction37),
	  _action38Functor(this, &RoomScriptActionExecutor::executeRoomScriptAction38),
	  _action39Functor(this, &RoomScriptActionExecutor::executeRoomScriptAction39),
	  _action3AFunctor(this, &RoomScriptActionExecutor::executeRoomScriptAction3A),
	  _action3BFunctor(this, &RoomScriptActionExecutor::executeRoomScriptAction3B),
	  _action3CFunctor(this, &RoomScriptActionExecutor::executeRoomScriptAction3C),
	  _action3DFunctor(this, &RoomScriptActionExecutor::executeRoomScriptAction3D),
	  _action05Or0CFunctor(&_action05Or0CExecutor, &RoomScriptAction05Or0CExecutor::executeRoomScriptAction05Or0C),
	  _action07Functor(&_foregroundOverrideExecutor, &RoomScriptForegroundOverrideExecutor::executeRoomScriptAction07),
	  _action08Functor(&_foregroundOverrideExecutor, &RoomScriptForegroundOverrideExecutor::executeRoomScriptAction08),
	  _action0DFunctor(&_cameraPanExecutor, &RoomScriptCameraPanExecutor::executeRoomScriptAction0D),
	  _action0EFunctor(&_cameraPanExecutor, &RoomScriptCameraPanExecutor::executeRoomScriptAction0E),
	  _action0FFunctor(&_cameraPanExecutor, &RoomScriptCameraPanExecutor::executeRoomScriptAction0F),
	  _action10Functor(&_cameraPanExecutor, &RoomScriptCameraPanExecutor::executeRoomScriptAction10),
	  _action15Functor(&_pairedActorTransitionExecutor, &RoomScriptPairedActorTransitionExecutor::executeRoomScriptAction15),
	  _action16Functor(&_pairedActorTransitionExecutor, &RoomScriptPairedActorTransitionExecutor::executeRoomScriptAction16),
	  _action17Functor(&_coordinateSpriteExecutor, &RoomScriptCoordinateSpriteExecutor::executeRoomScriptAction17),
	  _action18Functor(&_coordinateSpriteExecutor, &RoomScriptCoordinateSpriteExecutor::executeRoomScriptAction18),
	  _action19Functor(&_coordinateSpriteExecutor, &RoomScriptCoordinateSpriteExecutor::executeRoomScriptAction19),
	  _action1EFunctor(&_action1EExecutor, &RoomScriptAction1EExecutor::executeRoomScriptAction1E),
	  _action27Functor(&_action27Executor, &RoomScriptAction27Executor::executeRoomScriptAction27),
	  _action32Functor(&_duelActionExecutor, &DuelRoomScriptActionExecutor::executeRoomScriptAction32),
	  _action05Or0CExecutor(rom, scene, state, updateActorAnimationFrames, _waitForRoomVerticalBlankFunctor,
							refreshInteractionDisplay, executeRoomScriptAction1D, tileStreamer),
	  _action1EExecutor(rom, scene, state, presenter, updateActorAnimationFrames, handleRoomVBlank,
						restartApplication, _requestSessionExitFunctor),
	  _action27Executor(state, presenter, updateActorAnimationFrames, handleRoomVBlank,
						_requestSessionExitFunctor),
	  _cameraPanExecutor(state, updateActorAnimationFrames, _waitForRoomVerticalBlankFunctor),
	  _coordinateSpriteExecutor(rom, scene, state),
	  _duelActionExecutor(rom, scene, state, collisionProbe, random, presenter, updateActorAnimationFrames,
						  handleRoomVBlank, _tryExecuteRoomScriptAction02Functor, _action03Functor,
						  _requestSessionExitFunctor),
	  _foregroundOverrideExecutor(rom, scene, state, _waitForRoomVerticalBlankFunctor),
	  _pairedActorTransitionExecutor(state, updateActorAnimationFrames, _waitForRoomVerticalBlankFunctor),
	  _handlers(kHandlerCount) {
	_handlers[0x00] = &_action00Functor;
	_handlers[0x01] = &_action01Functor;
	_handlers[0x02] = &_action02Functor;
	_handlers[0x03] = &_action03Functor;
	_handlers[0x04] = &_action04Functor;
	_handlers[0x05] = &_action05Or0CFunctor;
	_handlers[0x06] = &_action06Functor;
	_handlers[0x07] = &_action07Functor;
	_handlers[0x08] = &_action08Functor;
	_handlers[0x09] = &_action09Functor;
	_handlers[0x0A] = &_action0AFunctor;
	_handlers[0x0B] = &_action0BFunctor;
	_handlers[0x0C] = &_action05Or0CFunctor;
	_handlers[0x0D] = &_action0DFunctor;
	_handlers[0x0E] = &_action0EFunctor;
	_handlers[0x0F] = &_action0FFunctor;
	_handlers[0x10] = &_action10Functor;
	_handlers[0x11] = &_action11Functor;
	_handlers[0x12] = &_action12Functor;
	_handlers[0x13] = &_action13Functor;
	_handlers[0x14] = &_action14Functor;
	_handlers[0x15] = &_action15Functor;
	_handlers[0x16] = &_action16Functor;
	_handlers[0x17] = &_action17Functor;
	_handlers[0x18] = &_action18Functor;
	_handlers[0x19] = &_action19Functor;
	_handlers[0x1A] = &_action1AFunctor;
	_handlers[0x1B] = &_action1BFunctor;
	_handlers[0x1C] = &_finishRoomStartup;
	_handlers[0x1D] = &executeRoomScriptAction1D;
	_handlers[0x1E] = &_action1EFunctor;
	_handlers[0x1F] = &_action1FFunctor;
	_handlers[0x20] = &_action20Functor;
	_handlers[0x21] = &_action21Functor;
	_handlers[0x22] = &_action22Functor;
	_handlers[0x23] = &_action23Functor;
	_handlers[0x24] = &_action24Functor;
	_handlers[0x25] = &_action25Functor;
	_handlers[0x26] = &_action26Functor;
	_handlers[0x27] = &_action27Functor;
	_handlers[0x28] = &_action28Functor;
	_handlers[0x29] = &_action29Functor;
	_handlers[0x2A] = &_action2AFunctor;
	_handlers[0x2B] = &_action2BFunctor;
	_handlers[0x2C] = &_action2CFunctor;
	_handlers[0x2D] = &_action2DFunctor;
	_handlers[0x2E] = &_action2EFunctor;
	_handlers[0x2F] = &_action2FFunctor;
	_handlers[0x30] = &_action30Functor;
	_handlers[0x31] = &_action31Functor;
	_handlers[0x32] = &_action32Functor;
	_handlers[0x33] = &_action33Functor;
	_handlers[0x34] = &_action34Functor;
	_handlers[0x35] = &_action35Functor;
	_handlers[0x36] = &_action36Functor;
	_handlers[0x37] = &_action37Functor;
	_handlers[0x38] = &_action38Functor;
	_handlers[0x39] = &_action39Functor;
	_handlers[0x3A] = &_action3AFunctor;
	_handlers[0x3B] = &_action3BFunctor;
	_handlers[0x3C] = &_action3CFunctor;
	_handlers[0x3D] = &_action3DFunctor;
}

Optional<SessionExit> RoomScriptActionExecutor::execute(uint16 action) {
	(*_handlers[static_cast<std::size_t>(action)])();
	return _requestedSessionExit;
}

// Preserves the intentional no-op action-table entry.
//
// Ghidra: executeRoomScriptAction00 (0x00002FAC).
void RoomScriptActionExecutor::executeRoomScriptAction00() {
	// Ghidra 0x00002FAC-0x00002FAD: return without reading or changing any state.
}

// Enables automatic-interaction gate zero without disturbing the byte's other gates.
//
// Ghidra: executeRoomScriptAction01 (0x0000453E).
void RoomScriptActionExecutor::executeRoomScriptAction01() {
	// Ghidra 0x0000453E-0x00004545: set gate zero while preserving all other automatic-interaction bits.
	_state.AutomaticInteractionFlags |= 0x01;

	// Ghidra 0x00004546-0x00004547: return without any further state changes.
}

// Calls the separate room-presentation owner without absorbing its original boundary.
//
// Ghidra: executeRoomScriptAction02 (0x00004548).
void RoomScriptActionExecutor::executeRoomScriptAction02() {
	// Ghidra 0x00004548-0x0000454B: call the separate PresentRoom owner at 0x0000219A.
	_requestedSessionExit = _presentRoom();

	// Ghidra 0x0000454C-0x0000454D: return without adding action-owned state.
}

bool RoomScriptActionExecutor::tryExecuteRoomScriptAction02() {
	executeRoomScriptAction02();
	return !_requestedSessionExit.hasValue();
}

// Fades the palette while preserving the complete display-flag byte around the operation.
//
// Ghidra: executeRoomScriptAction03 (0x0000454E).
void RoomScriptActionExecutor::executeRoomScriptAction03() {
	// Ghidra 0x0000454E-0x0000455B: save every display flag and clear only the room-update gate.
	uint8 displayFlags = _state.DisplayFlags;
	_state.DisplayFlags &= 0xBF;

	// Ghidra 0x0000455C-0x00004565: run the separate recovered fade owner, then restore the complete
	// saved byte so callback-side flag mutations during the fade do not escape this action.
	bool fadeCompleted = _presenter.fadePaletteOut(&_handleRoomVBlank);
	_state.DisplayFlags = displayFlags;
	if (!fadeCompleted) {
		_requestedSessionExit = SessionExit::HostClosed;
	}

	// Ghidra 0x00004566-0x00004567: return after the exact whole-byte restoration.
}

// Advances automatic interactions and actor animation until A or B receives a new press.
//
// Ghidra: executeRoomScriptAction04 (0x00004568). Each input-wait back edge services one room callback and
// clocked host frame because native VBlank updates both controller samples.
void RoomScriptActionExecutor::executeRoomScriptAction04() {
	while (true) {
		// Ghidra 0x00004568-0x0000456F: every repeated native iteration calls both separate original
		// owners. A managed non-returning handover from the first call bypasses the second as native control did.
		_requestedSessionExit = _processAutomaticInteractions();
		if (_requestedSessionExit.hasValue()) {
			return;
		}

		_updateActorAnimationFrames();

		// Ghidra 0x00004570-0x00004589: A has priority. A released falls through to B, newly pressed A
		// returns, and held A repeats immediately without sampling either B state.
		if ((_state.ControllerOneInput & 0x40) == 0) {
			if ((_state.PreviousControllerOneInput & 0x40) != 0) {
				return;
			}

			if (!waitForRoomVerticalBlank()) {
				return;
			}

			continue;
		}

		// Ghidra 0x0000458A-0x000045A3: preserve all B outcomes independently. Released or held B
		// repeats the complete loop; only a current press following a previous release returns.
		if ((_state.ControllerOneInput & 0x10) != 0) {
			if (!waitForRoomVerticalBlank()) {
				return;
			}

			continue;
		}

		if ((_state.PreviousControllerOneInput & 0x10) == 0) {
			if (!waitForRoomVerticalBlank()) {
				return;
			}

			continue;
		}

		return;
	}
}

// Finishes the room/interface startup transition when its progress bit remains active.
//
// Ghidra: executeRoomScriptAction06 (0x0000481A).
void RoomScriptActionExecutor::executeRoomScriptAction06() {
	// Ghidra 0x0000481A-0x00004825: preserve both progress-bit-one outcomes.
	if ((_state.ProgressStateBytes[0] & 0x02) == 0) {
		// Ghidra 0x00004826-0x00004827: a clear transition bit returns without side effects.
		return;
	}

	// Ghidra 0x00004828-0x0000482D: the set outcome calls and returns through the separate canonical
	// FinishRoomStartup boundary; do not absorb that original function into this action.
	_finishRoomStartup();
}

// Establishes actor-slot-one exclusion and blocks its visibility.
//
// Ghidra: executeRoomScriptAction09 (0x00004496).
void RoomScriptActionExecutor::executeRoomScriptAction09() {
	// Ghidra 0x00004496-0x0000449D: set only actor-slot-one exclusion mode.
	_state.InteractionFlags |= 0x80;
	// Ghidra 0x0000449E-0x000044A5: set only actor slot one's visibility block.
	_state.ActorVisibilityBlockFlags |= 0x02;
	// Ghidra 0x000044A6-0x000044A7: return with every other flag preserved.
}

// Ends actor-slot-one exclusion and clears its visibility block.
//
// Ghidra: executeRoomScriptAction0A (0x000044A8).
void RoomScriptActionExecutor::executeRoomScriptAction0A() {
	// Ghidra 0x000044A8-0x000044AF: clear only actor-slot-one exclusion mode.
	_state.InteractionFlags &= 0x7F;
	// Ghidra 0x000044B0-0x000044B7: clear only actor slot one's visibility block.
	_state.ActorVisibilityBlockFlags &= 0xFD;
	// Ghidra 0x000044B8-0x000044B9: return with every other flag preserved.
}

// Derives slot one's animation offset and suspends its normal update path.
//
// Ghidra: executeRoomScriptAction0B (0x00002FFC).
void RoomScriptActionExecutor::executeRoomScriptAction0B() {
	// Ghidra 0x00002FFC-0x00003003: load slot one's signed position index and add four with word wrap.
	// Ghidra 0x00003004-0x00003009: replace slot one's raw descriptor-relative animation offset.
	_state.ActorAnimationOffsets[1] = static_cast<int16>(_state.ActorPositionIndices[1] + 4);

	// Ghidra 0x0000300A-0x00003011: set only the actor-slot update suspension flag.
	_state.VideoFlags |= 0x04;

	// Ghidra 0x00003012-0x00003013: return with every other video flag preserved.
}

// Suppresses normal actor-zero movement and automatic animation selection.
//
// Ghidra: executeRoomScriptAction11 (0x000043AC).
void RoomScriptActionExecutor::executeRoomScriptAction11() {
	// Ghidra 0x000043AC-0x000043B3: set only the special actor-zero update-mode flag.
	_state.RoomBehaviorFlags |= 0x04;

	// Ghidra 0x000043B4-0x000043B5: return with every other room-behavior flag preserved.
}

// Restores normal actor-zero movement and automatic animation selection.
//
// Ghidra: executeRoomScriptAction12 (0x000043B6).
void RoomScriptActionExecutor::executeRoomScriptAction12() {
	// Ghidra 0x000043B6-0x000043BD: clear only the special actor-zero update-mode flag.
	_state.RoomBehaviorFlags &= 0xFB;

	// Ghidra 0x000043BE-0x000043BF: return with every other room-behavior flag preserved.
}

// Chooses between the two action-0x13 audio commands through the shared random sequence.
//
// Ghidra: executeRoomScriptAction13 (0x00004374).
void RoomScriptActionExecutor::executeRoomScriptAction13() {
	// Ghidra 0x00004374-0x00004379: advance the shared random longword and scale its low word to [0, 2).
	uint16 selection = _random.scaleNextRandomValue(2);

	// Ghidra 0x0000437A-0x00004385: preserve the complete nonzero branch, add command base 0x26
	// with word arithmetic, and replace the original tail branch with the silent managed audio handover.
	if (selection != 0) {
		playAudioCommand(static_cast<uint16>(selection + 0x26));
		return;
	}

	// Ghidra 0x00004386-0x0000438F: zero selects command 0x25 and reaches the same audio handover.
	playAudioCommand(0x25);
}

// Chooses between the two action-0x14 audio commands through the shared random sequence.
//
// Ghidra: executeRoomScriptAction14 (0x00004390).
void RoomScriptActionExecutor::executeRoomScriptAction14() {
	// Ghidra 0x00004390-0x00004395: advance the shared random longword and scale its low word to [0, 2).
	uint16 selection = _random.scaleNextRandomValue(2);

	// Ghidra 0x00004396-0x000043A1: preserve the complete nonzero branch, add command base 0x29
	// with word arithmetic, and replace the original tail branch with the silent managed audio handover.
	if (selection != 0) {
		playAudioCommand(static_cast<uint16>(selection + 0x29));
		return;
	}

	// Ghidra 0x000043A2-0x000043AB: zero selects command 0x28 and reaches the same audio handover.
	playAudioCommand(0x28);
}

// Starts the optional phase-table vertical-scroll offset from its authored first delay.
//
// Ghidra: executeRoomScriptAction1A (0x00004274). Authored delay sequence g_abRoomScrollPhaseDelays at
// 0x0003247C-0x0003247F contains 0x77, 0x77, 0x0E, 0x00. The word write of one leaves room delay-table index
// zero in the packed cursor's high byte for UpdateScrollingAndStreamTiles.
void RoomScriptActionExecutor::executeRoomScriptAction1A() {
	// Ghidra 0x00004274-0x00004283: park the phase cursor and initialize the packed delay cursor word.
	// Its high byte is the room delay-table index, so writing word one selects entry zero.
	_state.ScrollAnimationPhaseOffset = -1;
	_state.ScrollAnimationDelayCursor = 1;

	// Ghidra 0x00004284-0x0000428D: load entry zero of g_abRoomScrollPhaseDelays at 0x3247C-0x3247F.
	_state.ScrollAnimationDelay = static_cast<int8>(_rom.readByte(0x3247C));

	// Ghidra 0x0000428E-0x00004297: enable the optional offset and return with every other flag preserved.
	_state.RoomBehaviorFlags |= 0x80;
}

// Disables the optional phase-table vertical-scroll offset.
//
// Ghidra: executeRoomScriptAction1B (0x00004298).
void RoomScriptActionExecutor::executeRoomScriptAction1B() {
	// Ghidra 0x00004298-0x000042A1: clear only room-behavior bit seven and return.
	_state.RoomBehaviorFlags &= 0x7F;
}

// Reverses lead-actor vertical movement polarity.
//
// Ghidra: executeRoomScriptAction1F (0x000041A2).
void RoomScriptActionExecutor::executeRoomScriptAction1F() {
	// Ghidra 0x000041A2-0x000041AB: set only transition bit zero and return.
	_state.TransitionFlags |= 0x01;
}

// Restores normal lead-actor vertical movement polarity.
//
// Ghidra: executeRoomScriptAction20 (0x000041AC).
void RoomScriptActionExecutor::executeRoomScriptAction20() {
	// Ghidra 0x000041AC-0x000041B5: clear only transition bit zero and return.
	_state.TransitionFlags &= 0xFE;
}

// Waits for a prioritized direction and publishes its scripted result.
//
// Ghidra: executeRoomScriptAction21 (0x0000415A).
void RoomScriptActionExecutor::executeRoomScriptAction21() {
	while (true) {
		// Ghidra 0x0000415A-0x0000415D: advance every actor animation before each input sample.
		_updateActorAnimationFrames();

		// Ghidra 0x0000415E-0x00004171: active-low Left has first priority and publishes result zero.
		if ((_state.ControllerOneInput & 0x04) == 0) {
			_state.RoomObjects[0].ScriptStateWords[0] = 0;
			break;
		}

		// Ghidra 0x00004172-0x00004185: active-low Right has second priority and publishes result one.
		if ((_state.ControllerOneInput & 0x08) == 0) {
			_state.RoomObjects[0].ScriptStateWords[0] = 1;
			break;
		}

		// Ghidra 0x00004186-0x00004197: active-low Down publishes result two; no accepted direction
		// retains the complete back edge. The original relies on asynchronous controller polling, so
		// service the installed room callback and one clocked host frame before each managed continuation.
		if ((_state.ControllerOneInput & 0x02) == 0) {
			_state.RoomObjects[0].ScriptStateWords[0] = 2;
			break;
		}

		if (!waitForRoomVerticalBlank()) {
			return;
		}
	}

	// Ghidra 0x00004198-0x000041A1: request actor zero's one-shot animation-loop exit and return.
	_state.ActorAnimationLoopExitFlags |= 0x01;
}

// Selects actor zero's position-dependent animation and waits for its graphics publication.
//
// Ghidra: executeRoomScriptAction22 (0x00004120).
void RoomScriptActionExecutor::executeRoomScriptAction22() {
	// Ghidra 0x00004120-0x00004131: retain both signed-X branches around the 0x0114 threshold.
	if (static_cast<int16>(_state.ActorXFixedCoordinates[0] >> 16) < 0x0114) {
		_state.ActorAnimationOffsets[0] = 0x0049;
	} else {
		_state.ActorAnimationOffsets[0] = 0x004A;
	}

	// Ghidra 0x00004132-0x00004147: publish the selection, force its delay ready, and request restart.
	_state.ActorAnimationDelays[0] = -1;
	_state.ActorAnimationRestartFlags |= 0x01;

	// Ghidra 0x00004148-0x00004157: advance at least once and retain the graphics-ready back edge.
	// Sprite publication occurs asynchronously during the original room interrupt, so service the installed
	// room callback and one clocked host frame before each managed continuation.
	while (true) {
		_updateActorAnimationFrames();
		if ((_state.ActorGraphicsReadyFlags & 0x01) != 0) {
			break;
		}

		if (!waitForRoomVerticalBlank()) {
			return;
		}
	}

	// Ghidra 0x00004158-0x00004159: return after actor zero's selected graphics have been published.
}

// Suppresses coordinate-driven compact-actor scale recalculation.
//
// Ghidra: executeRoomScriptAction23 (0x0000410C).
void RoomScriptActionExecutor::executeRoomScriptAction23() {
	// Ghidra 0x0000410C-0x00004115: set only transition bit one and return.
	_state.TransitionFlags |= 0x02;
}

// Restores coordinate-driven compact-actor scale recalculation.
//
// Ghidra: executeRoomScriptAction24 (0x00004116).
void RoomScriptActionExecutor::executeRoomScriptAction24() {
	// Ghidra 0x00004116-0x0000411F: clear only transition bit one and return.
	_state.TransitionFlags &= 0xFD;
}

// Advances actor animations until actor slot zero's path traversal finishes.
//
// Ghidra: executeRoomScriptAction25 (0x000040FC).
void RoomScriptActionExecutor::executeRoomScriptAction25() {
	while (true) {
		// Ghidra 0x000040FC-0x000040FF: advance every actor animation at least once.
		_updateActorAnimationFrames();

		// Ghidra 0x00004100-0x00004109: return when slot zero's path is inactive; otherwise retain the
		// complete back edge. The original relies on asynchronous room interrupts to update path state,
		// so service the installed room callback and one clocked host frame before each continuation.
		if ((_state.ActorPathActiveFlags & 0x01) == 0) {
			break;
		}

		if (!waitForRoomVerticalBlank()) {
			return;
		}
	}

	// Ghidra 0x0000410A-0x0000410B: return after the first observed inactive path state.
}

// Publishes whether actor slot zero's path traversal has finished.
//
// Ghidra: executeRoomScriptAction26 (0x000040E0).
void RoomScriptActionExecutor::executeRoomScriptAction26() {
	// Ghidra 0x000040E0-0x000040E7: initialize the first room object's result to path-still-active zero.
	_state.RoomObjects[0].ScriptStateWords[0] = 0;

	// Ghidra 0x000040E8-0x000040F1: preserve the active path branch and return with that zero result.
	if ((_state.ActorPathActiveFlags & 0x01) != 0) {
		return;
	}

	// Ghidra 0x000040F2-0x000040FB: an inactive slot-zero path overwrites the result with one and returns.
	_state.RoomObjects[0].ScriptStateWords[0] = 1;
}

// Requests an unwind from every nested interaction-action menu after the current script.
//
// Ghidra: executeRoomScriptAction28 (0x00003F4E).
void RoomScriptActionExecutor::executeRoomScriptAction28() {
	// Ghidra 0x00003F4E-0x00003F57: set only the one-shot action-menu unwind flag and return.
	_state.RoomScriptFlags |= 0x04;
}

// Enables paired actor-zero and actor-slot-two transition processing.
//
// Ghidra: executeRoomScriptAction29 (0x00003F3A).
void RoomScriptActionExecutor::executeRoomScriptAction29() {
	// Ghidra 0x00003F3A-0x00003F43: set only transition bit three and return.
	_state.TransitionFlags |= 0x08;
}

// Restores independent actor processing after the paired transition mode.
//
// Ghidra: executeRoomScriptAction2A (0x00003F44).
void RoomScriptActionExecutor::executeRoomScriptAction2A() {
	// Ghidra 0x00003F44-0x00003F4D: clear only transition bit three and return.
	_state.TransitionFlags &= 0xF7;
}

// Installs actor zero's alternate direct animation and waits for its graphics publication.
//
// Ghidra: executeRoomScriptAction2B (0x00003ED2).
void RoomScriptActionExecutor::executeRoomScriptAction2B() {
	// Ghidra 0x00003ED2-0x00003ED9: enter the alternate actor-zero transition mode.
	_state.TransitionFlags |= 0x10;

	// Ghidra 0x00003EDA-0x00003EE1: select direct rather than compact frame decoding for actor zero.
	_state.ActorCompactFrameFlags &= 0xFE;

	// Ghidra 0x00003EE2-0x00003EF3: install the alternate descriptor and select animation 0x001E.
	_state.ActorAnimationDescriptorOffsets[0] = 0xB5D02;
	_state.ActorAnimationOffsets[0] = 0x1E;

	// Ghidra 0x00003EF4-0x00003F0B: restart actor zero, update every animation stream, and retain the
	// backward branch until sprite publication sets actor zero's graphics-ready bit. The original loop
	// relies on asynchronous room VBlank interrupts; service its callback and clock on each continuation.
	_state.ActorAnimationRestartFlags |= 0x01;
	while (true) {
		_updateActorAnimationFrames();
		if ((_state.ActorGraphicsReadyFlags & 0x01) != 0) {
			break;
		}

		if (!waitForRoomVerticalBlank()) {
			return;
		}
	}

	// Ghidra 0x00003F0C-0x00003F15: clear display bit seven and return.
	_state.DisplayFlags &= 0x7F;
}

// Restores actor zero's normal compact descriptor and leaves its alternate transition mode.
//
// Ghidra: executeRoomScriptAction2C (0x00003F16).
void RoomScriptActionExecutor::executeRoomScriptAction2C() {
	// Ghidra 0x00003F16-0x00003F1D: restore compact-frame conversion for actor slot zero.
	_state.ActorCompactFrameFlags |= 0x01;

	// Ghidra 0x00003F1E-0x00003F27: restore actor zero's normal animation descriptor.
	_state.ActorAnimationDescriptorOffsets[0] = 0x32700;

	// Ghidra 0x00003F28-0x00003F2F: leave the alternate actor-zero transition mode.
	_state.TransitionFlags &= 0xEF;

	// Ghidra 0x00003F30-0x00003F39: clear display bit seven and return.
	_state.DisplayFlags &= 0x7F;
}

// Suspends actor-zero animation synchronization and fixes the cursor at the transition position.
//
// Ghidra: executeRoomScriptAction2D (0x00003EBE).
void RoomScriptActionExecutor::executeRoomScriptAction2D() {
	// Ghidra 0x00003EBE-0x00003EC7: set only transition bit six and return.
	_state.TransitionFlags |= 0x40;
}

// Restores normal actor-zero animation synchronization and cursor updates.
//
// Ghidra: executeRoomScriptAction2E (0x00003EC8).
void RoomScriptActionExecutor::executeRoomScriptAction2E() {
	// Ghidra 0x00003EC8-0x00003ED1: clear only transition bit six and return.
	_state.TransitionFlags &= 0xBF;
}

// Queues the first room object's script result as the next scripted interaction.
//
// Ghidra: executeRoomScriptAction2F (0x00003E96).
void RoomScriptActionExecutor::executeRoomScriptAction2F() {
	// Ghidra 0x00003E96-0x00003E9D: set the selected-interaction loop's scripted-processing flag.
	_state.RoomScriptFlags |= 0x08;

	// Ghidra 0x00003E9E-0x00003EA7: snapshot the first room object's result word for that loop.
	_state.ScriptedInteraction = _state.RoomObjects[0].ScriptStateWords[0];

	// Ghidra 0x00003EA8-0x00003EA9: return with every other room-script flag and object field intact.
}

// Suppresses random interaction-animation selection until action 0x31 restores it.
//
// Ghidra: executeRoomScriptAction30 (0x00003EAA).
void RoomScriptActionExecutor::executeRoomScriptAction30() {
	// Ghidra 0x00003EAA-0x00003EB3: set only transition bit seven and return.
	_state.TransitionFlags |= 0x80;
}

// Restores random interaction-animation selection suppressed by action 0x30.
//
// Ghidra: executeRoomScriptAction31 (0x00003EB4).
void RoomScriptActionExecutor::executeRoomScriptAction31() {
	// Ghidra 0x00003EB4-0x00003EBD: clear only transition bit seven and return.
	_state.TransitionFlags &= 0x7F;
}

// Selects direct room-object tile-patch transfers without changing other video state.
//
// Ghidra: executeRoomScriptAction33 (0x0000312E).
void RoomScriptActionExecutor::executeRoomScriptAction33() {
	// Ghidra 0x0000312E-0x00003135: set only the direct room-object tile-patch transfer flag.
	_state.VideoFlags |= 0x01;

	// Ghidra 0x00003136-0x00003137: return with every other video-state flag preserved.
}

// Restores cached room-object tile-patch application after action 0x33.
//
// Ghidra: executeRoomScriptAction34 (0x00003138).
void RoomScriptActionExecutor::executeRoomScriptAction34() {
	// Ghidra 0x00003138-0x0000313F: clear only the direct room-object tile-patch transfer flag.
	_state.VideoFlags &= 0xFE;

	// Ghidra 0x00003140-0x00003141: return with every other video-state flag preserved.
}

// Runs the scripted actor-zero handover and selects actor two's corresponding animation.
//
// Ghidra: executeRoomScriptAction35 (0x0000301E).
void RoomScriptActionExecutor::executeRoomScriptAction35() {
	// Ghidra 0x0000301E-0x0000302F: select actor-zero animation 0x45 and wait until its prior decoded
	// frame is published. The original room VBlank remains active during the busy loop; invoke its callback
	// and clocked host frame once for each managed continuation instead of discarding the wait branch.
	_state.ActorAnimationOffsets[0] = 0x45;
	while ((_state.ActorTileUploadPendingFlags & 0x01) != 0) {
		if (!waitForRoomVerticalBlank()) {
			return;
		}
	}

	// Ghidra 0x00003030-0x0000303D: restart actor zero and immediately advance all animation streams.
	_state.ActorAnimationRestartFlags |= 0x01;
	_updateActorAnimationFrames();

	// Ghidra 0x0000303E-0x0000304D: replace only actor zero's signed integer coordinate halves,
	// preserving both fractional low words.
	_state.ActorXFixedCoordinates[0] = (_state.ActorXFixedCoordinates[0] & 0x0000FFFF) | (0x0128 << 16);
	_state.ActorYFixedCoordinates[0] = (_state.ActorYFixedCoordinates[0] & 0x0000FFFF) | (0x0040 << 16);

	// Ghidra 0x0000304E-0x00003071: advance until actor zero holds. Interaction bit seven bypasses
	// actor-one traversal waiting; otherwise a still-active path returns through the complete update loop.
	while (true) {
		_updateActorAnimationFrames();
		if ((_state.ActorAnimationHoldFlags & 0x01) == 0) {
			if (!waitForRoomVerticalBlank()) {
				return;
			}

			continue;
		}

		if ((_state.InteractionFlags & 0x80) != 0) {
			break;
		}

		if ((_state.ActorPathActiveFlags & 0x02) == 0) {
			break;
		}

		if (!waitForRoomVerticalBlank()) {
			return;
		}
	}

	// Ghidra 0x00003072-0x0000308D: independently resample interaction bit seven and preserve both
	// authored actor-two animation outcomes.
	if ((_state.InteractionFlags & 0x80) == 0) {
		_state.ActorAnimationOffsets[2] = 0;
	} else {
		_state.ActorAnimationOffsets[2] = 4;
	}

	// Ghidra 0x0000308E-0x0000309B: restart actor two and immediately advance all animation streams.
	_state.ActorAnimationRestartFlags |= 0x04;
	_updateActorAnimationFrames();

	// Ghidra 0x0000309C-0x000030A5: block visibility for actors zero and one, then return.
	_state.ActorVisibilityBlockFlags |= 0x03;
}

// Restores normal actor-slot updates suspended by action 0x0B.
//
// Ghidra: executeRoomScriptAction36 (0x00003014).
void RoomScriptActionExecutor::executeRoomScriptAction36() {
	// Ghidra 0x00003014-0x0000301B: clear only the actor-slot update suspension flag.
	_state.VideoFlags &= 0xFB;

	// Ghidra 0x0000301C-0x0000301D: return with every other video flag preserved.
}

// Runs the second scripted actor-zero handover and selects actor two's paired animation.
//
// Ghidra: executeRoomScriptAction37 (0x000030A6).
void RoomScriptActionExecutor::executeRoomScriptAction37() {
	// Ghidra 0x000030A6-0x000030B7: select actor-zero animation 0x47 and wait until its prior decoded
	// frame is published. The original room VBlank remains active during the busy loop; invoke its callback
	// and clocked host frame once for each managed continuation instead of discarding the wait branch.
	_state.ActorAnimationOffsets[0] = 0x47;
	while ((_state.ActorTileUploadPendingFlags & 0x01) != 0) {
		if (!waitForRoomVerticalBlank()) {
			return;
		}
	}

	// Ghidra 0x000030B8-0x000030C5: restart actor zero and immediately advance all animation streams.
	_state.ActorAnimationRestartFlags |= 0x01;
	_updateActorAnimationFrames();

	// Ghidra 0x000030C6-0x000030D5: replace only actor zero's signed integer coordinate halves,
	// preserving both fractional low words.
	_state.ActorXFixedCoordinates[0] = (_state.ActorXFixedCoordinates[0] & 0x0000FFFF) | (0x0058 << 16);
	_state.ActorYFixedCoordinates[0] = (_state.ActorYFixedCoordinates[0] & 0x0000FFFF) | (0x0050 << 16);

	// Ghidra 0x000030D6-0x000030F9: advance until actor zero holds. Interaction bit seven bypasses
	// actor-one traversal waiting; otherwise a still-active path returns through the complete update loop.
	while (true) {
		_updateActorAnimationFrames();
		if ((_state.ActorAnimationHoldFlags & 0x01) == 0) {
			if (!waitForRoomVerticalBlank()) {
				return;
			}

			continue;
		}

		if ((_state.InteractionFlags & 0x80) != 0) {
			break;
		}

		if ((_state.ActorPathActiveFlags & 0x02) == 0) {
			break;
		}

		if (!waitForRoomVerticalBlank()) {
			return;
		}
	}

	// Ghidra 0x000030FA-0x00003115: independently resample interaction bit seven and preserve both
	// authored actor-two animation outcomes.
	if ((_state.InteractionFlags & 0x80) == 0) {
		_state.ActorAnimationOffsets[2] = 1;
	} else {
		_state.ActorAnimationOffsets[2] = 5;
	}

	// Ghidra 0x00003116-0x00003123: restart actor two and immediately advance all animation streams.
	_state.ActorAnimationRestartFlags |= 0x04;
	_updateActorAnimationFrames();

	// Ghidra 0x00003124-0x0000312D: block visibility for actors zero and one, then return.
	_state.ActorVisibilityBlockFlags |= 0x03;
}

// Disables the low-bit exit gate shared by room-script and dialogue processing.
//
// Ghidra: executeRoomScriptAction38 (0x00002FD6).
void RoomScriptActionExecutor::executeRoomScriptAction38() {
	// Ghidra 0x00002FD6-0x00002FDD: clear only the gate's bit-one component.
	_state.RoomScriptFlags &= 0xFD;
	// Ghidra 0x00002FDE-0x00002FDF: return with every other room-script flag preserved.
}

// Arms the low-bit exit gate, latching it immediately for password-restored startup.
//
// Ghidra: executeRoomScriptAction39 (0x00002FE0).
void RoomScriptActionExecutor::executeRoomScriptAction39() {
	// Ghidra 0x00002FE0-0x00002FE7: enable the shared room-script and dialogue exit gate.
	_state.RoomScriptFlags |= 0x02;

	// Ghidra 0x00002FE8-0x00002FF1: a clear password-resume bit branches to the shared return
	// without changing room-script bit zero.
	if ((_state.InitializationFlags & 0x04) != 0) {
		// Ghidra 0x00002FF2-0x00002FF9: password-restored startup latches the exit immediately.
		_state.RoomScriptFlags |= 0x01;
	}

	// Ghidra 0x00002FFA-0x00002FFB: both branches return with room-script bits two through seven intact.
}

// Suppresses interaction-display refreshes until action 0x3B restores them.
//
// Ghidra: executeRoomScriptAction3A (0x00002FC2).
void RoomScriptActionExecutor::executeRoomScriptAction3A() {
	// Ghidra 0x00002FC2-0x00002FC9: set the independent interaction-display suppression flag.
	_state.VideoFlags |= 0x20;

	// Ghidra 0x00002FCA-0x00002FCB: return to ExecuteRoomScriptCommand0C.
}

// Restores interaction-display refreshes suppressed by action 0x3A.
//
// Ghidra: executeRoomScriptAction3B (0x00002FCC).
void RoomScriptActionExecutor::executeRoomScriptAction3B() {
	// Ghidra 0x00002FCC-0x00002FD3: clear only the interaction-display suppression flag.
	_state.VideoFlags &= 0xDF;

	// Ghidra 0x00002FD4-0x00002FD5: return to ExecuteRoomScriptCommand0C.
}

// Enables actor-slot-one synchronization until action 0x3D restores normal updates.
//
// Ghidra: executeRoomScriptAction3C (0x00002FAE).
void RoomScriptActionExecutor::executeRoomScriptAction3C() {
	// Ghidra 0x00002FAE-0x00002FB5: set only the actor-slot-one synchronization flag.
	_state.VideoFlags |= 0x40;

	// Ghidra 0x00002FB6-0x00002FB7: return to ExecuteRoomScriptCommand0C.
}

// Restores normal actor-slot-one updates after action 0x3C synchronization.
//
// Ghidra: executeRoomScriptAction3D (0x00002FB8).
void RoomScriptActionExecutor::executeRoomScriptAction3D() {
	// Ghidra 0x00002FB8-0x00002FBF: clear only the actor-slot-one synchronization flag.
	_state.VideoFlags &= 0xBF;

	// Ghidra 0x00002FC0-0x00002FC1: return to ExecuteRoomScriptCommand0C.
}

bool RoomScriptActionExecutor::waitForRoomVerticalBlank() {
	_handleRoomVBlank();
	if (_presenter.waitForVerticalBlank()) {
		return true;
	}

	_requestedSessionExit = SessionExit::HostClosed;
	return false;
}

// Defers Ghidra PlayAudioCommand at 0x00009762 without blocking room-script flow.
void RoomScriptActionExecutor::playAudioCommand(int command) {
	// AUDIO FRONTIER: command ordering is retained while audio playback remains deferred.
	(void)command;
}
} // namespace Scooby