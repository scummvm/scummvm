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

#ifndef SCOOBY_ROOMS_SCRIPT_COMMAND_EXECUTOR_H
#define SCOOBY_ROOMS_SCRIPT_COMMAND_EXECUTOR_H

#include "common/scummsys.h"

#include "common/array.h"
#include "common/func.h"
#include "common/rect.h"

#include "scooby/rooms/actor_animation_descriptor_catalog.h"
#include "scooby/rooms/collision_probe.h"
#include "scooby/rooms/script/action_executor.h"
#include "scooby/rooms/script/condition_evaluator.h"
#include "scooby/rooms/tile_streamer.h"
#include "scooby/assets/rom.h"
#include "scooby/common/optional.h"
#include "scooby/graphics/tile_scene.h"
#include "scooby/interactions/action_menu_controller.h"
#include "scooby/interactions/dialogue_controller.h"
#include "scooby/movement/scripted_room_movement_executor.h"
#include "scooby/presentation/frame_presenter.h"
#include "scooby/randomness/deterministic_random.h"
#include "scooby/rooms/actor_animator.h"
#include "scooby/rooms/vblank/handler.h"
#include "scooby/runtime/runtime_state.h"

// Owns the executable room-script command mapping and its recovered handler boundaries.
//
// Ghidra g_apRoomScriptCommandHandlers at ROM 0x00002354-0x000023DB contains 34
// pointer entries. The switch in execute() preserves every command index and the shared target used by
// commands 0x0E and 0x1B without treating executable lookup data as a ROM asset.

namespace Scooby {

class RoomScriptCommandExecutor {
public:
	// Builds the exact recovered command-index mapping once for this runtime session.
	// rom: verified cartridge containing the authored command records.
	// scene: logical tile scene updated by nested presentation actions.
	// state: shared script cursor and mutable runtime state.
	// collisionProbe: shared owner of decoded room collision queries.
	// movement: session-owned movement boundaries shared with Scooby wander movement.
	// random: shared deterministic sequence advanced by commands that request a scaled value.
	// presenter: shared retrace and palette-transition owner used by dedicated actions.
	// actorAnimator: owner of the recovered actor-animation update invoked by nested actions.
	// roomVBlank: owner of the installed room callback that advances state during action busy waits.
	// presentRoom: canonical room-presentation owner called by nested Action02.
	// resetRoomStateForReload: canonical room-state reset owner called by command 0x18.
	// reloadAndPresentRoom: canonical room reload and presentation owner called by command 0x11.
	// interactions: owner of the automatic-interaction update (nested Action04) and interaction-display
	//     refresh (nested Action05).
	// roomInterfaceTransitions: owner of the action-table targets at indices 0x1C and 0x1D.
	// dialogue: shared dialogue owner used by command 0x10.
	// invokeRoomScriptDispatchCallback: runtime owner of the scan or execution dispatcher installed by the
	//     current script caller.
	// actionMenu: owner of the action-prompt commit (commands 0x05 and 0x07), action-prompt refresh
	//     (command 0x05), and action-icon renderer (command 0x09).
	// tileStreamer: owner of room-plane restoration called by nested Action05.
	// restartApplication: managed owner of Ghidra RestartApplication.
	RoomScriptCommandExecutor(
		const ScoobyDooRom &rom, TileScene &scene, RuntimeState &state,
		const RoomCollisionProbe &collisionProbe, ScriptedRoomMovementExecutor &movement,
		DeterministicRandom &random, FramePresenter &presenter,
		RoomActorAnimator &actorAnimator, RoomVBlankHandler &roomVBlank,
		Common::Functor0<Optional<SessionExit>> &presentRoom,
		Common::Functor0<Optional<SessionExit>> &resetRoomStateForReload,
		Common::Functor0<Optional<SessionExit>> &reloadAndPresentRoom,
		InteractionController &interactions, RoomInterfaceTransitionExecutor &roomInterfaceTransitions,
		DialogueController &dialogue,
		Common::Functor1<int, Optional<SessionExit>> &invokeRoomScriptDispatchCallback,
		ActionMenuController &actionMenu, RoomTileStreamer &tileStreamer,
		Common::Functor1<const Common::Functor0<void> *, bool> &restartApplication);

	// Dispatches one command through its original table index.
	// command: unsigned command word read from the current script cursor.
	// mode: original D0 mode selected by the calling script loop.
	// interactionModeSnapshot: original D7 word retained across scan-mode command dispatch.
	// Returns the managed handover from an original non-returning nested action, otherwise no value.
	Optional<SessionExit> execute(uint16 command, int mode,
								  int16 interactionModeSnapshot);

private:
	// Consumes the one-word command record without changing command-specific state.
	// mode: ignored because scan and execution modes consume the same record.
	//
	// Ghidra: executeRoomScriptCommand00 (0x0000250C).
	void scriptCommand00NoOp(int mode);

	// Chooses the fixed record end or its authored signed failure displacement.
	// mode: ignored because the shared condition record has the same meaning in either mode.
	//
	// Ghidra: executeRoomScriptCommand03 (0x00002732).
	void scriptCommand03IfStatement(int mode);

	// Runs a conditional nested script window and merges its resulting cursor into the parent window.
	// mode: selects the scan or execution dispatcher installed by the current script owner.
	//
	// Ghidra: executeRoomScriptCommand04 (0x0000274A). The original dynamically scoped
	// g_pRoomScriptDispatchCallback is represented by the runtime-owned callback rather than a mutable
	// executable pointer. Signed script displacements and the auto-flag early return retain their original
	// cursor and bound ownership.
	void scriptCommand04IfStatementNested(int mode);

	// Selects an actor animation directly or through its fixed-position room object.
	// mode: zero scans past the command; nonzero applies its animation selection.
	//
	// Ghidra: executeRoomScriptCommand06 (0x00002B5A). Each managed continuation services
	// the installed room callback and one clocked host frame because the original tight loops allow
	// asynchronous interrupt publication.
	void scriptCommand06PlayObjAnim(int mode);

	// Marks an active object pending or aligns the inventory page for an inventory object.
	// mode: zero scans past the command; nonzero applies its object update.
	//
	// Ghidra: executeRoomScriptCommand07 (0x00002BF8). The typed inventory list replaces
	// the original contiguous words and their trailing -1 sentinel without removing its count path.
	void scriptCommand07RefreshObject(int mode);

	// Replaces an object's inventory graphic and refreshes inventory prompts when required.
	// mode: zero scans past the command; nonzero applies its object update.
	//
	// Ghidra: executeRoomScriptCommand0A (0x00002DC0). The installed room callback and
	// host presenter replace the original vertical-blank handshake before the canonical prompt commit.
	void scriptCommand0ASetInventoryGfx(int mode);

	// Dispatches one authored nested room-script action and advances past its command record.
	// mode: zero scans past the command; nonzero executes its nested action.
	//
	// Ghidra: executeRoomScriptCommand0C (0x00002E9E). The executable 62-entry table at
	// 0x00002EB4-0x00002FAB is represented by RoomScriptActionExecutor rather than
	// read as cartridge asset data.
	void scriptCommand0CRunAction(int mode);

	// Routes one encoded audio command to its play or stop boundary.
	// mode: zero scans past the command; nonzero dispatches its audio request.
	//
	// Ghidra: executeRoomScriptCommand0D (0x000048B6). A negative word stops sequence (word & 0x7FFF);
	// otherwise the word starts that sequence through the GEMS driver.
	void scriptCommand0DPlayOrStopAudio(int mode);

	static void playAudioCommand(int command);
	static void stopAudioPlayback(int command);

	// Reloads a selected room from one explicit room, coordinate, and actor-position record.
	// mode: zero scans the eight-byte record; nonzero performs the fade and room reload.
	//
	// Ghidra: executeRoomScriptCommand11 (0x00004F0E). The coordinate index is shifted
	// within a word, preserving 16-bit wrap before LoadRoomScene consumes it as a byte offset.
	void scriptCommand11ChangeRoom(int mode);

	// Replaces one room object's complete shape index word.
	// mode: zero scans the six-byte record; nonzero applies its shape index.
	//
	// Ghidra: executeRoomScriptCommand12 (0x00004F50). The object identity is converted to
	// the original signed low-word byte offset before selecting the typed 0x1A-byte record.
	void scriptCommand12SetObjShape(int mode);

	// Stops automatic-interaction scanning at its marker or follows the marker's signed displacement.
	// mode: zero recognizes an active automatic-interaction scan; nonzero follows the displacement.
	//
	// Ghidra: executeRoomScriptCommand13 (0x00002446). The automatic scan stop leaves the
	// cursor on this command so its owner can derive the nested execution window from the same record.
	void scriptCommand13Jump(int mode);

	// Replaces one room object's complete shape index word through command slot 0x14.
	// mode: zero scans the six-byte record; nonzero applies its shape index.
	//
	// Ghidra: executeRoomScriptCommand14 (0x00002E7E). This remains a distinct original
	// boundary even though its complete 32-byte body is identical to command 0x12.
	void scriptCommand14SetObjShape(int mode);

	// Advances actor animations for an inclusive authored retrace countdown.
	// mode: zero scans the four-byte record; nonzero performs the timed animation wait.
	//
	// Ghidra: executeRoomScriptCommand15 (0x000048D8). The room callback and host retrace
	// replace the asynchronous interrupt that decrements the original signed countdown.
	void scriptCommand15WaitNFrames(int mode);

	// Blocks visibility for a direct actor or a room object's assigned actor slot.
	// mode: zero scans the four-byte record; nonzero applies its actor visibility block.
	//
	// Ghidra: executeRoomScriptCommand16 (0x00002AD0). Dynamic byte-bit selection remains
	// modulo eight, including the room object's -1 fixed-position sentinel branch.
	void scriptCommand16HideActor(int mode);

	// Advances animations until a direct actor or room object's assigned actor reaches a hold command.
	// mode: zero scans the four-byte record; nonzero waits for the selected actor hold.
	//
	// Ghidra: executeRoomScriptCommand17 (0x00004FBE). The selected actor is updated at
	// least once, and dynamic byte-bit selection retains the fixed-position -1 sentinel branch.
	// The installed room callback and host presenter replace interrupt-driven delay progress on each
	// continuation of the native tight loop.
	void scriptCommand17WaitForObjAnim(int mode);

	bool waitForRoomVerticalBlank();

	// Selects a room and opening actor coordinate before resetting its runtime state.
	// mode: zero scans the eight-byte record; nonzero applies the room selection and reset.
	//
	// Ghidra: executeRoomScriptCommand18 (0x00004EE4). The coordinate index shifts within
	// a word before the separate room-state reset boundary consumes the selected room state.
	void scriptCommand18ChangeRoomNoFade(int mode);

	// Records the non-returning handover from a nested movement command.
	void requestMovementHostClose() {
		_requestedSessionExit = SessionExit::HostClosed;
	}

	// Records an explicit session-exit request from a nested command owner.
	// exit: the requested session-exit value.
	void requestSessionExit(SessionExit exit) {
		_requestedSessionExit = exit;
	}

	// Shared by the command implementations below.
	static const uint8 kFixedPositionMask = 0x02;
	static int replaceIntegerCoordinate(int coordinate, int16 integer);

	// ---- Command 0x01, implemented in command_01.cpp ----

	// Evaluates the interaction condition in scan mode or follows its signed record displacement.
	// mode: zero evaluates the condition; nonzero skips directly by the record displacement.
	// interactionMode: stable D7 interaction-mode snapshot retained by the scan dispatcher.
	//
	// Ghidra: executeRoomScriptCommand01 (0x0000246A).
	void executeRoomScriptCommand01(int mode, int16 interactionMode);

	static const uint8 kAlternateInteractionMask = 0x01;
	static const uint8 kConditionEvaluationMask = 0x02;
	static const uint8 kRoomBehaviorModeMask = 0x01;
	static const uint8 kSecondaryInteractionLatchMask = 0x20;
	static const uint8 kStopDispatchMask = 0x01;

	// ---- Command 0x02, implemented in command_02.cpp ----

	// Conditionally registers one menu choice, then follows the record's signed cursor displacement.
	// mode: zero scans and may register the entry; nonzero only advances the cursor.
	//
	// Ghidra: executeRoomScriptCommand02 (0x000025AC). Active episode descriptor field +0x20 is 0x0014199C for
	// episode zero and 0x001AF33C for episode one. The original stores five parallel arrays;
	// InteractionActionRegistry preserves each indexed relationship as one value without retaining mutable
	// cartridge pointers.
	void scriptCommand02AddPlayerChoiceInteractionLine(int mode);

	static const int kInteractionTextBaseField = 0x20;
	static const int kNestedScriptField = 0x0E;
	static const uint8 kRegistrationEnabledMask = 0x04;

	// ---- Command 0x05, implemented in command_05.cpp ----

	// Relocates one room object and consumes its complete six-byte command record.
	// mode: zero scans the record; any nonzero value applies the relocation.
	//
	// Ghidra: executeRoomScriptCommand05 (0x000028A0). The 36-entry descriptor and movement-step catalogs
	// remain ROM-owned authored data. Four dynamic actor movement slots replace the original overlapping RAM
	// aliases, while the typed inventory list replaces its trailing -1 word.
	void scriptCommand05MoveObject(int mode);

	static const uint8 kCompactActorMask = 0x08;
	static const int kDynamicActorCount = 4;
	static const int kDynamicActorFirstSlot = 2;
	static const int kDynamicActorSlotLimit = kDynamicActorFirstSlot + kDynamicActorCount;
	static const int16 kInventoryRoomId = 1;
	static const uint8 kObjectTilePatchPendingMask = 0x01;
	static const uint8 kSuppressActionPromptRefreshMask = 0x04;

	// Returns false when host closure interrupted a synchronous wait (the caller must abort without consuming
	// the command record), true on normal completion.
	bool relocateFixedObject(RoomObject &roomObject, int16 newRoomId);

	// Returns true when host closure interrupted a synchronous wait (the caller must abort without consuming
	// the command record), false on normal completion.
	bool relocatePortableObject(RoomObject &roomObject, int objectIndex, uint16 encodedObjectId,
								int16 newRoomId);

	// Returns true when host closure interrupted the wait, false otherwise.
	bool refreshActionPromptsAfterRelocation(uint16 encodedObjectId);

	// ---- Command 0x08, implemented in command_08.cpp ----

	// Transfers one selected value through the command's complete source and destination operation matrix.
	// mode: zero scans past the command; nonzero applies its value transfer.
	//
	// Ghidra: executeRoomScriptCommand08 (0x00002C56). Room-object addresses retain the original wrapped word
	// sum and even alignment, including offsets that cross into a neighboring record. The progress-word XOR
	// uses the command destination held in D3; raw instruction 0x00002D08 instead indexes through inherited
	// A3.W, which neither dispatcher establishes.
	void scriptCommand08SetVariable(int mode);

	// Preserves a mutable reference to the resolved room-object record and its aligned command-word offset,
	struct RoomObjectWordAddress {
		RoomObject &targetRoomObject;
		int byteOffset;

		RoomObjectWordAddress(RoomObject &newTargetRoomObject, int newByteOffset)
			: targetRoomObject(newTargetRoomObject), byteOffset(newByteOffset) {
		}
	};

	RoomObjectWordAddress resolveRoomObjectWordAddress(uint16 objectId, uint16 fieldOffset);

	// ---- Command 0x09, implemented in command_09.cpp ----

	// Replaces an indexed room-object byte and retires any active action icon.
	// mode: zero scans past the command; nonzero applies its object and icon updates.
	//
	// Ghidra: executeRoomScriptCommand09 (0x00002D5E). The signed command displacement is relative to record
	// field +0x16 and may select any byte in an adjacent typed record. The canonical icon renderer and its
	// hardware handover preserve the original resolved A1 address.
	void executeRoomScriptCommand09(int mode);

	static const int kActionIconByteOffset = 0x16;

	// Preserves a mutable reference to the resolved room-object record and its command-byte offset.
	struct RoomObjectByteLocation {
		RoomObject &targetRoomObject;
		int byteOffset;

		RoomObjectByteLocation(RoomObject &newTargetRoomObject, int newByteOffset)
			: targetRoomObject(newTargetRoomObject), byteOffset(newByteOffset) {
		}
	};

	RoomObjectByteLocation resolveRoomObjectByte(uint16 objectId, int8 displacement);

	// ---- Command 0x0B, implemented in command_0b.cpp ----

	// Updates one object's shape and tile patch, optionally waiting for publication.
	// mode: zero scans past the command; nonzero applies its complete object update.
	//
	// Ghidra: executeRoomScriptCommand0B (0x00002DF2). Encoded identity bit 15 controls synchronous completion
	// independently from the low 15-bit object identity. The callback-aware clocked frame replaces the
	// original asynchronous interrupt progress observed by the busy loop.
	void scriptCommand0BSetBackgroundGfx(int mode);

	static const uint8 kTilePatchPendingMask = 0x01;
	static const uint16 kTilePatchRetirementMask = 0xC000;

	// ---- Command 0x0E/0x1B, implemented in command_0e_or_1b.cpp ----

	// Places or moves one encoded actor or room object from an indexed or immediate destination.
	// mode: zero scans the record; nonzero executes every identity and movement branch.
	//
	// Ghidra: executeRoomScriptCommand0EOr1B (0x000049BA). Command 0x0E consumes ten bytes and scales one
	// indexed room-position pair by eight; command 0x1B consumes twelve bytes and uses its immediate signed
	// coordinates. Encoded identity bit 15 suppresses nonnegative movement completion waits but is masked from
	// identity selection; room-object direct placement still waits for a requested animation's graphics.
	// Negative movement mode replaces only the integer half of each 16.16 coordinate; actor zero restarts at
	// delay -1, while actor one retains the original asymmetric delay 1.
	void scriptCommand0EOr1BMoveToPosition(int mode);

	static const uint16 kIndexedPositionCommand = 0x0E;
	static const int kIndexedRecordSize = 10;
	static const int kImmediateRecordSize = 12;

	Common::Point readTargetCoordinates(int commandOffset, uint16 command);
	bool placeActorDirectly(int actorSlot, int16 targetX, int16 targetY,
							int16 animationOffset,
							int16 initialDelay, uint8 actorMask);

	// ---- Command 0x0F, implemented in command_0f.cpp ----

	// Moves one encoded target relative to an actor or room-object coordinate source.
	// mode: zero scans the record; nonzero executes every descriptor, source, and target branch.
	//
	// Ghidra: executeRoomScriptCommand0F (0x00004C20). The 12-byte record masks target bit 15 for selection
	// but retains the encoded word for lead and room-object nonnegative movement completion policy;
	// room-object direct placement still waits for requested animation graphics, while companion-direct
	// movement does not inspect it. Source identities one and two use actor integer coordinates; every other
	// source uses its room-position selector and the target animation descriptor's type-two record before
	// fixed or shape-relative placement.
	void scriptCommand0FWalkRelativeTo(int mode);

	static const int kEpisodeShapeDataBaseField = 0x1C;
	static const int kEpisodeShapePointerTableField = 0x24;
	static const int kCommand0FRecordSize = 12;

	bool tryResolveTargetAnimationDescriptor(uint16 targetIdentity, int &descriptorOffset);
	Common::Point resolveRelativeDestination(int commandOffset, int animationDescriptorOffset);
	RoomObject &resolveRoomObject(uint16 objectId);
	static int16 getIntegerCoordinate(int fixedCoordinate);

	// Resolves signed coordinates through an animation descriptor's type-two interaction record.
	// animationDescriptorOffset: ROM offset of the selected actor animation descriptor.
	// positionOffset: signed position selector used to index the descriptor's word table.
	// Returns the type-two record's signed X and Y coordinates.
	//
	// Ghidra: findInteractionTypeTwoRecord (0x0000197E). The complete body is 0x0000197E-0x000019AF.
	// Descriptor-relative offsets and the doubled position selector retain signed word wrapping and sign
	// extension. Four-byte {type, recordOffset} entries are scanned without a sentinel, bound, or fallback
	// until type 2; that entry resolves a descriptor-relative record whose signed coordinate words are at
	// +0x0E/+0x10.
	Common::Point
	findInteractionTypeTwoRecord(int animationDescriptorOffset, int16 positionOffset);

	// ---- Command 0x10, implemented in command_10.cpp ----

	// Presents one authored dialogue with optional hold setup and reversible actor movement.
	// mode: zero scans the eight-byte record; nonzero executes every encoded branch.
	//
	// Ghidra: executeRoomScriptCommand10 (0x00004D80). Flag bit 12 primes actor-zero hold
	// and interaction state. Bit 13 independently gates the transition before and after dialogue, bits
	// 14-15 select its temporary position, bits 4-5 replace tile palette bits 13-14, and bits 0-3 select
	// the mask's nonzero palette index. The managed scene receives the same 95 generated patterns without
	// retaining the original VRAM transport.
	void scriptCommand10Dialog(int mode);

	static const int kDialogueBaseField = 0x20;
	static const int kMaskTileBaseDelta = 0x20;
	static const uint16 kPaletteAttributePreservationMask = 0x9FFF;
	static const int kCommand10RecordSize = 8;

	void generateDialogueMaskTiles(uint8 nonzeroPaletteIndex);
	bool playShaggyActorTurnAnimation(int16 previousPositionIndex, int16 nextPositionIndex);

	// ---- Command 0x19, implemented in command_19.cpp ----

	// Publishes a selected actor's current frame before restoring its visibility.
	// mode: zero scans the four-byte record; nonzero performs the publication transition.
	//
	// Ghidra: executeRoomScriptCommand19 (0x00002B0E). Every identity other than one or two retains the
	// wrapped object-table branch, and byte-bit selection remains modulo eight.
	void scriptCommand19ShowActor(int mode);

	// ---- Command 0x1A, implemented in command_1a.cpp ----

	// Configures, disables, or immediately restores one animated palette range.
	// mode: zero scans the ten-byte record; nonzero evaluates its signed selector and control word.
	//
	// Ghidra: executeRoomScriptCommand1A (0x00002626). The selector comparison is signed, and the disable
	// transfer retains its exclusive end and word-width arithmetic.
	void executeRoomScriptCommand1A(int mode);

	// ---- Command 0x1C, implemented in command_1c.cpp ----

	// Mirrors the selected actor's movement-active bit into progress byte-zero bit two.
	// mode: zero scans the four-byte record; nonzero evaluates every actor-identity branch.
	//
	// Ghidra: executeRoomScriptCommand1C (0x0000255E). Object-selected actors retain the signed low word of
	// the native record offset and the fixed-position byte's unsigned modulo-eight bit.
	void executeRoomScriptCommand1C(int mode);

	// ---- Command 0x1D, implemented in command_1d.cpp ----

	// Mirrors the selected actor's animation-hold bit into progress byte-zero bit three.
	// mode: zero scans the four-byte record; nonzero evaluates every actor-identity branch.
	//
	// Ghidra: executeRoomScriptCommand1D (0x00002510). Object-selected actors retain the signed low word of
	// the native record offset and the fixed-position byte's unsigned modulo-eight bit.
	void executeRoomScriptCommand1D(int mode);

	// ---- Command 0x1E, implemented in command_1e.cpp ----

	// Selects one lead or companion position-transition animation and optionally waits for its hold.
	// mode: zero scans the six-byte record; nonzero executes every selector and sign branch.
	//
	// Ghidra: executeRoomScriptCommand1E (0x0000268E). Selector bit fifteen requests the hold wait
	// independently of the actor choice, and the two actor branches deliberately restart asymmetrically.
	// Managed continuations service the installed room callback and one clocked host frame because native
	// VBlank can interrupt either tight wait and decrement positive animation delays.
	void executeRoomScriptCommand1E(int mode);

	// ---- Command 0x1F, implemented in command_1f.cpp ----

	// Advances animation until the selected actor has neither movement nor path traversal active.
	// mode: zero scans the four-byte record; nonzero evaluates every identity and wait branch.
	//
	// Ghidra: executeRoomScriptCommand1F (0x00004F70). The native loop checks movement before path state and
	// always performs at least one animation update. Managed continuations service the installed room callback
	// and one clocked host frame to replace asynchronous movement and path progress.
	void executeRoomScriptCommand1F(int mode);

	// ---- Command 0x20, implemented in command_20.cpp ----

	// Cancels selected active movement or requests an animation-loop exit when movement is clear.
	// mode: zero scans the four-byte record; nonzero evaluates every identity and state branch.
	//
	// Ghidra: executeRoomScriptCommand20 (0x000048F8). The movement-active and movement-clear branches
	// deliberately update different masks without changing the other one.
	void executeRoomScriptCommand20(int mode);

	// ---- Command 0x21, implemented in command_21.cpp ----

	// Installs an inline path stream and starts traversal for the selected actor.
	// mode: zero scans the variable-length record; nonzero installs all selected path state.
	//
	// Ghidra: executeRoomScriptCommand21 (0x00004944). Unlike the preceding actor-selection commands, dynamic
	// selection sign-extends the room object's fixed-position byte before indexing every six-slot path array.
	// The record extent is also a signed word even though authored extents are positive.
	void executeRoomScriptCommand21(int mode);

	const ScoobyDooRom &_rom;
	RuntimeState &_state;
	FramePresenter &_presenter;
	Common::Functor0<Optional<SessionExit>> &_reloadAndPresentRoom;
	Common::Functor0<Optional<SessionExit>> &_resetRoomStateForReload;
	RoomActorAnimator &_actorAnimator;
	RoomVBlankHandler &_roomVBlank;
	ActionMenuController &_actionMenu;
	TileScene &_scene;
	DeterministicRandom &_random;
	DialogueController &_dialogue;
	ScriptedRoomMovementExecutor &_movement;
	Common::Functor1<int, Optional<SessionExit>> &_invokeRoomScriptDispatchCallback;

	ActorAnimationDescriptorCatalog _actorDescriptorCatalog;
	RoomScriptActionExecutor _actions;
	RoomScriptConditionEvaluator _conditionEvaluator;

	int16 _interactionModeSnapshot;
	Optional<SessionExit> _requestedSessionExit;
};
} // namespace Scooby

#endif // SCOOBY_ROOMS_SCRIPT_COMMAND_EXECUTOR_H
