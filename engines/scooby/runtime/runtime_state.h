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

#ifndef SCOOBY_RUNTIME_STATE_H
#define SCOOBY_RUNTIME_STATE_H

#include "common/scummsys.h"
#include "common/ptr.h"

#include "common/array.h"

#include "scooby/assets/rom.h"
#include "scooby/common/optional.h"
#include "scooby/graphics/palette_color.h"
#include "scooby/interactions/interaction_action_registry.h"
#include "scooby/movement/actor_movement_point.h"
#include "scooby/rooms/room_object.h"
#include "scooby/rooms/room_scene_data.h"
#include "scooby/rooms/scripted_coordinate_sprite_state.h"

// Preserves the shared RAM and register-backed state recovered from Ghidra
// RunAdventure at 0x00000BF4 and its callees.
//
// This is a mechanical, field-for-field translation of the C# RuntimeState
// class: every property becomes a public field with the same name and the
// same default value (C#'s implicit value-type zero-initialization is made
// explicit here via "{}" member initializers). Per-field commentary below
// is the original C# XML doc summary with only its <c>/<see> markup
// stripped; the original's per-field <remarks> describing *other*
// subsystems' use of a field are intentionally not duplicated here -- they
// belong beside the ported code that actually does the reading/writing
// (Rooms/, Interactions/, MainMenu/, Movement/), where they are carried
// over as that file is ported. RoomSceneData? -> Common::ScopedPtr (single
// owner, nullable, replaced wholesale by LoadRoomScene) per this project's
// no-raw-owning-pointers rule; every other C# nullable value type (T?)
// becomes Optional<T> (C++11 has no std::optional).

namespace Scooby {

class RuntimeState {
public:
	RuntimeState() {
	}

	// The active action icon from Ghidra g_wActiveActionIcon at 0xFF06C0–0xFF06C1. Original RAM has no image-
	// backed value; runtime setup clears it before interaction updates. Command 0x09 queues any nonzero value
	// as pending before clearing this word.
	int16 ActiveActionIcon{};

	// The cartridge offset from Ghidra g_pActiveEpisodeDescriptor at 0xFF068E. Process memory starts at zero;
	// LoadRoomObjectTable is the sole writer and publishes the descriptor selected from g_wEpisodeIndex before
	// any room or interaction script consumes its fields.
	int32 ActiveEpisodeDescriptorOffset{};

	// The cartridge offset held by Ghidra g_pRoomPositionCoordinateTable at 0xFF0696–0xFF0699. Work RAM starts
	// at zero; LoadRoomScene publishes the active room's eight immutable signed X/Y tile-coordinate pairs
	// after its decoded payloads. Command 0x0E retains the original word-width index multiplication before
	// scaling the selected pair by eight.
	int32 RoomPositionCoordinateTableOffset{};

	// The active-slot mask from Ghidra g_bActiveActorFlags at 0xFF0AB4. No image-backed value is consumed;
	// room-runtime initialization enables actor slots zero and one with value three. Action 0x15 tests slots
	// two through five in ascending order and selects the first clear bit, retaining sentinel six when all
	// four slots are active.
	uint8 ActiveActorFlags{};

	// The independent signed retrace countdown from Ghidra g_sbAuxiliaryRetraceCountdown at 0xFF0AC7. No
	// image-backed value is consumed; runtime reset writes zero before installed VBlank callbacks decrement it
	// through -1.
	int8 AuxiliaryRetraceCountdown{};

	// The six signed 16.16 X coordinates from Ghidra g_anActorXFixedCoordinates at 0xFF04DC-0xFF04F3. Work RAM
	// starts at zero; movement owners preserve the fractional low words while room placement and animation
	// commands replace or adjust the signed integer high words. Action 0x22 reads slot zero's integer word to
	// select animation 0x0049 below 0x0114 or 0x004A otherwise. Action 0x27 moves only the integer words of
	// slots three and four together within 0x0064–0x0094. TestDuelActorOverlap reads slots zero and two to
	// place their authored inclusive collision intervals; ComputeDuelRelativeDirection subtracts those slots'
	// signed integer words with 16-bit wrap.
	Common::Array<int32> ActorXFixedCoordinates = Common::Array<int32>(6);

	// The six signed 16.16 Y coordinates from Ghidra g_anActorYFixedCoordinates at 0xFF04F4-0xFF050B. Work RAM
	// starts at zero; movement owners preserve the fractional low words while room placement and animation
	// commands replace or adjust the signed integer high words. Action 0x27 changes only slot three's integer
	// word while approaching 0x004A and later descending through zero. TestDuelActorOverlap reads slots zero
	// and two to place their authored inclusive collision intervals; ComputeDuelRelativeDirection subtracts
	// those slots' signed integer words with 16-bit wrap.
	Common::Array<int32> ActorYFixedCoordinates = Common::Array<int32>(6);

	// The six signed animation delays from Ghidra g_anActorAnimationDelays at 0xFF0618-0xFF0623. No image-
	// backed value is consumed; room-runtime initialization writes -1 to every slot so the first room frame
	// can advance each active actor immediately. Action 0x22 explicitly restores slot zero to -1 before
	// requesting its selected restart; command 0x06 does the same for a nonnegative fixed-position object
	// actor. A selected interaction action also writes -1 before restarting its eligible fixed-object actor.
	// RefreshRoomSprites decrements every positive slot once per enabled room update and preserves zero or
	// negative slots.
	Common::Array<int16> ActorAnimationDelays = Common::Array<int16>(6);

	// The six active animation-descriptor cartridge offsets from Ghidra g_apActiveActorAnimationDescriptors at
	// 0xFF0470-0xFF0487. Work RAM starts at zero; room-runtime initialization assigns 0x00032700 and
	// 0x000642D0 to slots zero and one. Action 0x2B temporarily installs 0x000B5D02 in slot zero, and action
	// 0x2C restores 0x00032700.
	Common::Array<int32> ActorAnimationDescriptorOffsets = Common::Array<int32>(6);

	// The six descriptor-relative animation offsets from Ghidra g_awActorAnimationOffsets at
	// 0xFF0488-0xFF0493. No image-backed value is consumed; room-runtime initialization writes 4, 4, 0, 0, 0,
	// 0 before the first room frame. Room-script action 0x0B replaces slot one with its signed room-position
	// index plus four using the original 16-bit wrap. Action 0x22 selects slot-zero offset 0x0049 below signed
	// X 0x0114 or 0x004A otherwise. ChooseRandomInteractionAnimation selects slot zero from base 0x24, a
	// clamped random term, and four times its normalized signed position, retaining 16-bit wrapping. Action
	// 0x2B selects slot-zero offset 0x001E after installing descriptor 0x000B5D02; action 0x27 selects slot-
	// three offset 0x001C for success or 0x001B for failure. Command 0x06 can replace slots zero or one
	// directly, or slots two through five through a room object's signed fixed-position index.
	// RunInteractionActionMenu selects slot zero from its position plus four after dismissal and can assign
	// the selected nonnegative action state to a fixed room object's dynamic slot. Command 0x10 selects slot
	// zero's forward and reverse animations from the transition matrix at 0x000FC740–0x000FC75F, then restarts
	// the steady offset equal to its current position plus four. Command 0x1E selects slot zero from that
	// matrix or slot one from the companion matrix at 0x000FC700–0x000FC71F. TestDuelActorOverlap consumes
	// slots zero and two as wrapped signed indices into their fixed duel descriptors.
	// ResolveDuelActorCollision compares those same slots with the relative and opposite actor directions to
	// weight each momentum word. The two boundary-impact handlers read slots zero and two as authored facings
	// in the 0–7 domain.
	Common::Array<int16> ActorAnimationOffsets = Common::Array<int16>(6);

	// The six descriptor-relative animation-command cursors from Ghidra g_awActorAnimationCommandOffsets at
	// 0xFF0494-0xFF049F. Work RAM starts at zero; each restart resolves a cursor through the active
	// descriptor's animation table before interpreting it.
	Common::Array<int16> ActorAnimationCommandOffsets = Common::Array<int16>(6);

	// The per-slot restart mask from Ghidra g_bActorAnimationRestartFlags at 0xFF0AB5. No image-backed value
	// is consumed; room-runtime initialization sets slots zero and one. Actions 0x22 and 0x2B set slot zero,
	// while action 0x27 sets slot three. Command 0x06 sets a direct slot zero or one, or the dynamic slot
	// owned by a fixed-position room object; command 0x10 sets slot zero for each transition and following
	// steady animation; command 0x1E always sets slot zero but sets slot one only for its negative wait
	// branch; UpdateActorAnimationFrames consumes a restart after selecting its stream unless pending graphics
	// defer it. ChooseRandomInteractionAnimation sets slot zero after publishing its selected offset;
	// RunInteractionActionMenu subsequently sets slot zero and any eligible fixed-object slot.
	uint8 ActorAnimationRestartFlags{};

	// The six selected descriptor-relative frame offsets from Ghidra g_awActorSelectedFrameOffsets at
	// 0xFF04A0-0xFF04AB. Work RAM starts at zero; room-runtime initialization explicitly clears slots zero and
	// one before their first animation commands select authored frame records.
	Common::Array<int16> ActorSelectedFrameOffsets = Common::Array<int16>(6);

	// The six compact-frame horizontal anchors from Ghidra g_awActorFrameAnchorX at 0xFF04AC-0xFF04B7. Work
	// RAM starts at zero; compact conversion derives each value from the selected frame record and current
	// horizontal scale.
	Common::Array<int16> ActorFrameAnchorX = Common::Array<int16>(6);

	// The six compact-frame vertical anchors from Ghidra g_awActorFrameAnchorY at 0xFF04B8-0xFF04C3. Work RAM
	// starts at zero; compact conversion derives each value from the selected frame record and current
	// vertical scale.
	Common::Array<int16> ActorFrameAnchorY = Common::Array<int16>(6);

	// The six published compact-frame horizontal anchors from Ghidra g_awActorPublishedFrameAnchorX at
	// 0xFF04C4-0xFF04CF. Work RAM starts at zero; sprite publication snapshots the current anchors before
	// composition consumes them.
	Common::Array<int16> ActorPublishedFrameAnchorX = Common::Array<int16>(6);

	// The six published compact-frame vertical anchors from Ghidra g_awActorPublishedFrameAnchorY at
	// 0xFF04D0-0xFF04DB. Work RAM starts at zero; sprite publication snapshots the current anchors before
	// composition consumes them.
	Common::Array<int16> ActorPublishedFrameAnchorY = Common::Array<int16>(6);

	// The six staged animation delays from Ghidra g_aswActorStagedAnimationDelays at 0xFF050C-0xFF0517. Work
	// RAM starts at zero; animation command four replaces a slot before frame selection copies it to the
	// current delay.
	Common::Array<int16> ActorStagedAnimationDelays = Common::Array<int16>(6);

	// The compact compressed-source offsets sharing original pointer table g_apActorFrameWorkPointers at
	// 0xFF0540-0xFF0557. Work RAM starts at zero; managed state separates the temporary ROM source identity
	// from the decoded tile buffers that later occupy the alias.
	Common::Array<Optional<int32>> ActorCompactFrameSourceOffsets = Common::Array<Optional<int32>>(6);

	// The decoded actor tile buffers sharing original pointer table g_apActorFrameWorkPointers at
	// 0xFF0540-0xFF0557. Work RAM starts at zero; direct decoding and compact conversion replace individual
	// buffers before room-sprite publication consumes them. Empty managed buffers represent the initial absent
	// pointers until a pending-upload bit establishes a decoded extent.
	Common::Array<Common::Array<uint8>> ActorTileData = Common::Array<Common::Array<uint8>>(6);

	// The six published frame-data references from Ghidra g_apActorCurrentFrameData at 0xFF0558-0xFF056F. The
	// original initializes every slot to raw marker 0xFF000000; managed state represents that absence as <see
	// langword="null"/> until publication.
	Common::Array<Optional<int32>> ActorCurrentFrameDataOffsets = Common::Array<Optional<int32>>(6);

	// The six staged frame-data references from Ghidra g_apActorPendingFrameData at 0xFF0570-0xFF0587. The
	// original initializes every slot to raw marker 0xFF000000; managed state represents that absence as <see
	// langword="null"/> until frame selection.
	Common::Array<Optional<int32>> ActorPendingFrameDataOffsets = Common::Array<Optional<int32>>(6);

	// The six pending tile-transfer word counts from Ghidra g_awActorTileTransferWordCounts at
	// 0xFF0588-0xFF0593. Work RAM starts at zero; selected frame dimensions choose the complete publication
	// extent before decoding.
	Common::Array<uint16> ActorTileTransferWordCounts = Common::Array<uint16>(6);

	// The six current horizontal scale factors from Ghidra g_awActorHorizontalScaleFactors at
	// 0xFF05A0-0xFF05AB. Work RAM starts at zero; movement and transition owners replace them before compact-
	// frame conversion and sprite composition consume the raw words.
	Common::Array<uint16> ActorHorizontalScaleFactors = Common::Array<uint16>(6);

	// The six current vertical scale factors from Ghidra g_awActorVerticalScaleFactors at 0xFF05AC-0xFF05B7.
	// Work RAM starts at zero; movement and transition owners replace them before compact-frame conversion and
	// sprite composition consume the raw words.
	Common::Array<uint16> ActorVerticalScaleFactors = Common::Array<uint16>(6);

	// The decoded-tile publication mask from Ghidra g_bActorTileUploadPendingFlags at 0xFF0AB6. No image-
	// backed value is consumed; room-runtime initialization clears every slot. ResetRoomStateForReload updates
	// animations once, waits at least one retrace, and repeats while any slot remains set. Action 0x05 also
	// waits for the complete byte to become zero before replacing room graphics.
	uint8 ActorTileUploadPendingFlags{};

	// The compact-frame conversion mask from Ghidra g_bActorCompactConversionPendingFlags at 0xFF0AB9. No
	// image-backed value is consumed; room-runtime initialization clears every slot.
	uint8 ActorCompactConversionPendingFlags{};

	// The compact-frame source mask from Ghidra g_bActorCompactFrameFlags at 0xFF0AB8. No image-backed value
	// is consumed; room-runtime initialization marks slots zero and one as compact. Actions 0x2B and 0x2C
	// clear and restore slot zero around its alternate descriptor.
	uint8 ActorCompactFrameFlags{};

	// The animation-command hold mask from Ghidra g_bActorAnimationHoldFlags at 0xFF0ABC. Work RAM starts at
	// zero; command nine sets a slot until a restart or explicit position update resumes it. Action 0x15
	// advances all streams until slot zero reaches this hold state, while action 0x27 does the same for slot
	// three. Command 0x10 can set slot zero before dialogue and, for each enabled position transition, waits
	// first for restart to clear it and then for animation command nine to set it again. Command 0x17 advances
	// all streams at least once and waits for the direct or room-object-selected actor bit. Command 0x1D
	// projects a selected bit into progress byte-zero bit three. A negative command 0x1E selector waits until
	// the chosen lead or companion bit is set.
	uint8 ActorAnimationHoldFlags{};

	// The completed actor-graphics mask from Ghidra g_bActorGraphicsReadyFlags at 0xFF0ABD. Work RAM starts at
	// zero; frame restart and animation replacement clear slots, while sprite refresh sets them after pending
	// tile data has been published. Actions 0x22 and 0x2B repeatedly advance animation and allow room VBlank
	// publication until slot zero is set. Command 0x19 waits on its direct or room-object-selected bit before
	// restoring that actor's visibility.
	uint8 ActorGraphicsReadyFlags{};

	// The six-slot frame-publication latch from Ghidra g_bActorFramePublicationLatchFlags at 0xFF0AC1. Work
	// RAM starts at zero. Sprite construction sets a slot after snapshotting pending frame identity, scale,
	// and anchors; tile publication clears it so the next completed frame can replace that snapshot.
	uint8 ActorFramePublicationLatchFlags{};

	// The persistent six-slot sprite-presence mask from Ghidra g_bActorSpritePresenceFlags at 0xFF0AC4. Work
	// RAM starts at zero. Each sprite build snapshots and clears it, then sets slots whose actor sprites are
	// appended; the snapshot retains a previously published frame until replacement graphics are ready.
	uint8 ActorSpritePresenceFlags{};

	// The animation-loop exit mask from Ghidra g_bActorAnimationLoopExitFlags at 0xFF0AC3. Work RAM starts at
	// zero; movement and script owners set slots that command three consumes once. Action 0x21 sets slot zero
	// only after publishing its accepted directional result. Command 0x20 sets a direct or room-object-
	// selected slot only when that actor's movement bit is already clear.
	uint8 ActorAnimationLoopExitFlags{};

	// The transition-frame readiness mask from Ghidra g_bActorTransitionFrameReadyFlags at 0xFF0AC6. Work RAM
	// starts at zero; paired transition animation frames set their slots before conversion and publication
	// consume the complete pair. Action 0x15 clears every slot after selecting the paired actor.
	uint8 ActorTransitionFrameReadyFlags{};

	// The unsigned actor slot paired with slot zero during room transitions from Ghidra
	// g_wTransitionActorIndex at 0xFF08B0. Work RAM starts at zero. Action 0x15 writes the first inactive slot
	// from two through five, or sentinel six when all four candidates are active.
	uint16 TransitionActorIndex{};

	// The per-slot sprite-priority mask from Ghidra g_bActorHighPriorityFlags at 0xFF0ABB. No image-backed
	// value is consumed; room-runtime initialization clears every slot.
	uint8 ActorHighPriorityFlags{};

	// The movement-active mask from Ghidra g_bActorMovementFlags at 0xFF0AB7. No image-backed value is
	// consumed; room-runtime initialization clears every actor bit. Bits 0–5 correspond to actor slots 0–5;
	// movement owners and the room update set or clear them. Command 0x1C projects a selected bit into
	// progress byte-zero bit two, command 0x1F waits until both the selected movement and path bits are clear,
	// and command 0x20 clears a selected active movement bit or requests that actor's animation-loop exit when
	// movement is already clear.
	uint8 ActorMovementFlags{};

	// The signed lead-actor horizontal movement step from Ghidra g_nLeadActorHorizontalMovementStep at
	// 0xFF062E–0xFF062F. Work RAM starts at zero; scripted movement installs this signed 8.8 multiplier, while
	// direct movement and collision adjustment can replace or clear it.
	int16 LeadActorHorizontalMovementStep{};

	// The signed lead-actor vertical movement step from Ghidra g_nLeadActorVerticalMovementStep at
	// 0xFF0630–0xFF0631. Work RAM starts at zero; scripted movement installs this signed 8.8 multiplier, while
	// direct movement and collision adjustment can replace or clear it.
	int16 LeadActorVerticalMovementStep{};

	// The signed companion horizontal movement step from Ghidra g_nCompanionActorHorizontalMovementStep at
	// 0xFF063A–0xFF063B. Work RAM starts at zero; companion movement installs this signed 8.8 multiplier and
	// the room updater clears it at the signed target boundary.
	int16 CompanionActorHorizontalMovementStep{};

	// The signed companion vertical movement step from Ghidra g_nCompanionActorVerticalMovementStep at
	// 0xFF063C–0xFF063D. Work RAM starts at zero; companion movement installs this signed 8.8 multiplier and
	// the room updater clears it at the signed target boundary.
	int16 CompanionActorVerticalMovementStep{};

	// The six-slot path-traversal mask from Ghidra g_bActorPathActiveFlags at 0xFF0AC0. Ghidra has no image-
	// backed byte for this Work RAM location; InitializeRoomActors clears the complete mask before script
	// command 0x21 installs path state and sets a selected slot. UpdateRoomActorsAndInterface clears each slot
	// at its path sentinel or completion. Action 0x25 advances animations while slot zero remains active, and
	// action 0x26 publishes zero while that slot is active or one once it is clear. Command 0x1F advances
	// animations until both this mask and the movement mask are clear for its direct or room-object-selected
	// actor.
	uint8 ActorPathActiveFlags{};

	// The six active path-stream pointers from Ghidra g_apActorPathStreams at 0xFF05F4-0xFF060B. Work RAM has
	// no image-backed values; command 0x21 installs the cartridge offset of its inline four-byte record stream
	// for the selected actor. The room updater reads signed coordinate pairs and control sentinels from that
	// stream through the matching byte offset.
	Common::Array<int32> ActorPathStreamOffsets = Common::Array<int32>(6);

	// The six signed path-update countdowns from Ghidra g_asbActorPathCadenceCountdowns at 0xFF0AA5-0xFF0AAA.
	// Work RAM has no image-backed values; command 0x21 seeds a selected slot from command byte +7. The room
	// updater advances that actor only after decrementing the countdown below zero, then reloads it from
	// ActorPathCadenceReloads.
	Common::Array<int8> ActorPathCadenceCountdowns = Common::Array<int8>(6);

	// The six retained path-update cadence bytes from Ghidra g_abActorPathCadenceReloads at 0xFF0AAB-0xFF0AB0.
	// Work RAM has no image-backed values; command 0x21 stores the same command byte in this array and the
	// current countdown before path traversal starts.
	Common::Array<uint8> ActorPathCadenceReloads = Common::Array<uint8>(6);

	// The six unsigned path-record byte offsets from Ghidra g_awActorPathRecordOffsets at 0xFF060C-0xFF0617.
	// Work RAM starts at zero; command 0x21 clears a selected slot before traversal advances it by four per
	// record. Action 0x27 conditionally clears slot two when its path is inactive and uses the retained
	// terminal offset to select the first room object's result.
	Common::Array<uint16> ActorPathRecordOffsets = Common::Array<uint16>(6);

	// The six authored room-position indices from Ghidra g_anActorPositionIndices at 0xFF05B8-0xFF05C3. No
	// image-backed value is consumed; room-runtime initialization clears all six before movement and room
	// scripts select positions. Command 0x0F copies its source room object's selector into slot zero after
	// lead movement or slot one after companion movement. Room-script action 0x0B reads slot one as a signed
	// word before deriving its animation offset. Command 0x10 can replace slot zero with encoded bits 14–15
	// during dialogue and restores the exact prior signed word after playing the reverse transition. Commands
	// 0x11 and 0x18 replace slot zero directly before reloading or resetting their selected room. Command 0x1E
	// replaces slot zero only for masked selector one and slot one for every other selector before choosing an
	// authored transition from the prior and next signed words. ChooseRandomInteractionAnimation rewrites
	// slot-zero position two to zero, while position three is remapped only in its local animation
	// calculation. ResolveDuelActorCollision writes the relative direction to slot two and its opposite to
	// slot zero. The two boundary-impact handlers rewrite their actor's slot with the opposite current facing.
	Common::Array<int16> ActorPositionIndices = Common::Array<int16>(6);

	// The signed 16.16 horizontal movement steps for dynamic actor slots two through five from Ghidra
	// g_dwRoomObjectActorHorizontalStepBase at 0xFF051C–0xFF052B. Work RAM starts at zero;
	// InitializeRoomActors and command 0x05 copy one of the 36 immutable values at 0x000325E0–0x0003266F when
	// a fixed-position room object activates its slot. MoveRoomObjectToPosition retains one base step for
	// modes zero and one or multiplies it through 32-bit wrapping repeated addition for larger nonnegative
	// modes.
	Common::Array<int32> RoomObjectActorHorizontalSteps = Common::Array<int32>(4);

	// The signed 16.16 vertical movement steps for dynamic actor slots two through five from Ghidra
	// g_dwRoomObjectActorVerticalStepBase at 0xFF0530–0xFF053F. Work RAM starts at zero; initialization and
	// MoveRoomObjectToPosition derive each value by logically shifting its matching horizontal step right
	// once.
	Common::Array<int32> RoomObjectActorVerticalSteps = Common::Array<int32>(4);

	// The six signed target X coordinates from Ghidra g_anActorTargetXCoordinates at 0xFF05DC–0xFF05E7. Work
	// RAM starts at zero; movement owners and room-object activation replace individual targets before the
	// room updater converges each actor's 16.16 position. Room-object scripted movement writes attached actor
	// slots two through five after scaling their movement steps. Dynamic-actor overshoot clamps retain the
	// original adjacent-long read: the following target supplies the fractional word, and slot five continues
	// into target Y slot zero.
	Common::Array<int16> ActorTargetXCoordinates = Common::Array<int16>(6);

	// The six signed target Y coordinates from Ghidra g_anActorTargetYCoordinates at 0xFF05E8–0xFF05F3. Work
	// RAM starts at zero; movement owners and room-object activation replace individual targets before the
	// room updater converges each actor's 16.16 position. Room-object scripted movement writes attached actor
	// slots two through five after scaling their movement steps. Dynamic-actor overshoot clamps retain the
	// original adjacent-long read: the following target supplies the fractional word, and slot five continues
	// into the high word of path-stream pointer slot zero.
	Common::Array<int16> ActorTargetYCoordinates = Common::Array<int16>(6);

	// The cached horizontal interpolation values from Ghidra g_awActorPreviousHorizontalScales at
	// 0xFF05C4-0xFF05CF. Work RAM starts at zero; room-runtime initialization explicitly clears slots zero and
	// one before sprite composition.
	Common::Array<uint16> ActorPreviousHorizontalScales = Common::Array<uint16>(6);

	// The cached vertical interpolation values from Ghidra g_awActorPreviousVerticalScales at
	// 0xFF05D0-0xFF05DB. Work RAM starts at zero; room-runtime initialization explicitly clears slots zero and
	// one before sprite composition.
	Common::Array<uint16> ActorPreviousVerticalScales = Common::Array<uint16>(6);

	// The priority-update inhibition mask from Ghidra g_bActorPriorityUpdateBlockFlags at 0xFF0AC2. No image-
	// backed value is consumed; room-runtime initialization clears every slot.
	uint8 ActorPriorityUpdateBlockFlags{};

	// The visibility-block mask from Ghidra g_bActorVisibilityBlockFlags at 0xFF0ABF. No image-backed value is
	// consumed; room-runtime initialization clears every actor bit. Bit one blocks actor slot one from
	// interaction targeting, updates, and sprite publication. Action 0x09 sets it with interaction bit seven,
	// action 0x0A clears it, and InitializeRoomActors reasserts it while that exclusion mode remains active.
	// Actions 0x35 and 0x37 later block slots zero and one together. Action 0x27 blocks slot two only after
	// success, then clears it after the shared fade on both outcomes. Command 0x16 blocks a direct or room-
	// object-selected actor bit; command 0x19 clears that same bit after its animation hold completes.
	uint8 ActorVisibilityBlockFlags{};

	// The actor-position update mask from Ghidra g_bActorPositionUpdateFlags at 0xFF0ABE. No image-backed
	// value is consumed; runtime reset clears every actor bit. Command 0x19 sets a direct or room-object-
	// selected bit so the actor animator stages its current frame and clears the request.
	uint8 ActorPositionUpdateFlags{};

	// The lead actor's four directional animation latches from Ghidra
	// g_bLeadActorDirectionalAnimationLatchFlags at 0xFF0ACC. Work RAM has no image-backed value;
	// InitializeRoomActors clears the byte before UpdateRoomActorsAndInterface uses bits zero through three to
	// avoid restarting a transition animation while its direction remains active.
	uint8 LeadActorDirectionalAnimationLatchFlags{};

	// The automatic-interaction gates from Ghidra g_bAutomaticInteractionFlags at 0xFF0AC9. No image-backed
	// value is consumed; runtime reset clears every gate. Room-script action 0x01 sets bit zero without
	// changing the other gates. Command 0x01 also sets bit zero when its scan condition matches, tests bit one
	// before evaluating that condition, and sets bit five on its zero-secondary-interaction path. Bit two
	// enables command 0x02 menu-entry registration. ProcessAutomaticInteractions temporarily preserves bit
	// seven, sets bit four, and invokes the scan dispatcher; command 0x13 converts that gate to stop bit zero
	// without advancing its cursor, allowing the owner to execute the nested script before restoring the
	// complete saved byte. RunInteractionActionMenu clears the complete byte before selected-script execution
	// and treats bit zero set by that execution as a one-frame unwind request.
	uint8 AutomaticInteractionFlags{};

	// Owns the count and five related entry arrays recovered from Ghidra g_wInteractionActionCount at
	// 0xFF080E, g_awInteractionActionStateValues at 0xFF0824–0xFF082D, g_awInteractionActionScriptLengths at
	// 0xFF082E–0xFF0837, g_adwInteractionActionLabelTextPointers at 0xFF0838–0xFF084B,
	// g_adwInteractionActionResponseTextPointers at 0xFF084C–0xFF085F, and
	// g_adwInteractionActionScriptPointers at 0xFF0860–0xFF0873. Process memory starts with zero entries;
	// command 02 registers coherent choices while later menu behavior consumes them.
	InteractionActionRegistry InteractionActions;

	// The background horizontal displacement from Ghidra g_wBackgroundHorizontalScroll at 0xFF07FA. No image-
	// backed value is consumed; runtime reset writes zero before room scrolling.
	int16 BackgroundHorizontalScroll{};

	// The background vertical displacement from Ghidra g_wBackgroundVerticalScroll at 0xFF07FE. No image-
	// backed value is consumed; runtime reset writes zero before room scrolling.
	int16 BackgroundVerticalScroll{};

	// The last action icon committed to the display from Ghidra g_wCachedActionIcon at 0xFF06C4–0xFF06C5. No
	// image-backed value is consumed; display refresh writes it before the room updater uses it when the
	// active icon is zero.
	int16 CachedActionIcon{};

	// The room camera X origin from Ghidra g_wCameraX at 0xFF06CA. No image-backed value is consumed; runtime
	// reset writes zero before room loading selects the authored origin. Actions 0x0F and 0x10 clear bit zero
	// before a scripted pan; UpdateScrollingAndStreamTiles adds CameraPanStep until CameraPanTargetX is
	// reached. Action 0x05 saves and zeros the word, toggles bit eight every fourth presentation iteration,
	// then restores it.
	int16 CameraX{};

	// The signed horizontal pan step from Ghidra g_swCameraPanStep at 0xFF08B4–0xFF08B5. No image-backed value
	// is consumed: actions 0x0F and 0x10 assign it before transition bit five lets
	// UpdateScrollingAndStreamTiles add it each retrace.
	int16 CameraPanStep{};

	// The exact horizontal pan target from Ghidra g_wCameraPanTargetX at 0xFF08B6–0xFF08B7. No image-backed
	// value is consumed: actions 0x0F and 0x10 assign it before transition bit five becomes visible to
	// UpdateScrollingAndStreamTiles.
	int16 CameraPanTargetX{};

	// The unsigned active-room height from Ghidra g_wRoomHeightTiles at 0xFF06C8–0xFF06C9. No image-backed
	// value is consumed; LoadRoomScene copies room-layout word two before deriving coordinate tables, camera
	// bounds, collision rows, and streamed edges.
	uint16 RoomHeightTiles{};

	// The unsigned active-room width from Ghidra g_wRoomWidthTiles at 0xFF06C6–0xFF06C7. No image-backed value
	// is consumed; LoadRoomScene copies descriptor word zero before room scripts run. The value counts 8-pixel
	// tile columns and supplies the decoded tile-map row stride. Action 0x0F derives the signed 16-bit
	// rightmost camera origin from it.
	uint16 RoomWidthTiles{};

	// The room camera Y origin from Ghidra g_wCameraY at 0xFF06CE. No image-backed value is consumed; runtime
	// reset writes zero before room loading selects the authored origin. Action 0x05 saves and zeros the word
	// for its temporary presentation, then restores it before redraw.
	int16 CameraY{};

	// The selected interaction from Ghidra g_wCurrentInteraction at 0xFF06AE. No image-backed value is
	// consumed; runtime reset writes zero.
	int16 CurrentInteraction{};

	// The selected description flags from Ghidra g_wInteractionDescriptionFlags at 0xFF06B0–0xFF06B1. Work RAM
	// starts at zero; RunSelectedInteraction is the sole writer and byte-tests the big-endian high byte at
	// 0xFF06B0. Word mask 0x0100 selects a person; 0x0200 then selects feminine rather than masculine
	// placeholder text. RunInteractionActionMenu independently consumes the low byte: bits zero through three
	// select the generated mask's nonzero palette index, while bits four and five become palette bits 13 and
	// 14.
	uint16 InteractionDescriptionFlags{};

	// The signed selected action state from Ghidra g_wSelectedInteractionActionStateValue at
	// 0xFF06B2–0xFF06B3. Work RAM starts at zero. RunInteractionActionMenu is the sole writer and consumer; a
	// negative selected value suppresses the fixed room object's actor-state update.
	int16 SelectedInteractionActionStateValue{};

	// The five signed rendered-line counts from Ghidra g_awInteractionActionLineCounts at 0xFF081A–0xFF0823.
	// Work RAM starts at zero. RunInteractionActionMenu writes only each rendered unused entry, so values
	// deliberately persist across nested frames and later invocations.
	Common::Array<int16> InteractionActionLineCounts = Common::Array<int16>(5);

	// The cartridge pointer from Ghidra g_pRoomNameText at 0xFF06D2–0xFF06D5. Work RAM has no consumed image
	// value; LoadRoomScene derives the active room's NUL-terminated name from room record field zero and
	// episode-descriptor base +0x20 before refresh paths consume it.
	int32 RoomNameTextOffset{};

	// The signed cursor X coordinate from Ghidra g_wCursorX at 0xFF06DC–0xFF06DD. Work RAM starts at zero.
	// Cursor movement clamps it to -8–247; sprite construction snaps it to lower-interface columns when the
	// horizontal step is not two.
	int16 CursorX{};

	// The signed cursor Y coordinate from Ghidra g_wCursorY at 0xFF06DE–0xFF06DF. Work RAM starts at zero.
	// Cursor movement clamps it to the active interface range; BuildRoomSpriteTable selects the direct cursor
	// below 168 or the lower interface otherwise.
	int16 CursorY{};

	// The signed horizontal cursor step from Ghidra g_wCursorHorizontalStep at 0xFF06E0–0xFF06E1. Work RAM
	// starts at zero. Cursor updates select step two or forty; BuildRoomSpriteTable writes two for direct
	// cursor presentation and tests it before snapping.
	int16 CursorHorizontalStep{};

	// The signed vertical cursor step from Ghidra g_wCursorVerticalStep at 0xFF06E2–0xFF06E3. Work RAM starts
	// at zero. Cursor updates select step two or three, and direct cursor sprite construction restores two.
	int16 CursorVerticalStep{};

	// The interaction-display flags from Ghidra g_bDisplayFlags at 0xFF09DC. No image-backed value is
	// consumed; runtime reset clears every flag. Bit seven gates several actor, cursor, and movement update
	// paths; actions 0x2B and 0x2C clear it after installing actor zero's selected animation descriptor. Bit
	// six gates the complete room update block in HandleRoomVBlank. Action 0x03 saves the whole byte, clears
	// bit six only during its fade, then restores every saved flag. Action 0x05 brackets its temporary
	// presentation and final palette publication with bit six, action 0x27 brackets its fade with it, and
	// action 0x1E clears it before its terminal fade and reset handover. Command 0x11 saves the whole byte,
	// clears bit six for its fade, and restores the saved byte before reloading the room. Bit five queues
	// staged action-prompt publication; CommitPendingActionPromptDuringVBlank clears it only when publishing
	// the staged patterns and interface cells. ResetRoomStateForReload clears bit three before loading, then
	// sets bits two and six. SaveAndClearInteractionTileBlock and RunInteractionActionMenu also clear bit
	// three unconditionally; their bit-four-clear branches select the right interface half, while bit four set
	// selects the left half. The action menu sets bit zero before each strip update and waits until
	// HandleRoomVBlank clears it.
	uint8 DisplayFlags{};

	// The packed base tile, palette, and priority attributes from Ghidra g_wInterfaceTileAttributes at
	// 0xFF068C–0xFF068D. No image-backed value is consumed; LoadRoomScene establishes it from the first free
	// pattern address before room text consumers run. Command 0x10 saves the complete word, replaces only
	// palette bits 13–14 while presenting its dialogue, then restores the saved value and regenerates the
	// associated 95 mask tiles. RunInteractionActionMenu clears those palette bits before each label phase,
	// ORs the selected description's palette bits before its response, and uses a cleared local copy for
	// cleanup mask generation without writing that final copy back.
	uint16 InterfaceTileAttributes{};

	// The shared text-display countdown from Ghidra g_wTextDisplayCountdown at 0xFF080A–0xFF080B. Work RAM has
	// no image-backed value, so managed state starts at zero. DrawDialogueText writes text length multiplied
	// by 16, DrawTimedInteractionText writes rendered line count multiplied by 300, and
	// WaitForDialogueDismissal shifts then decrements the word with unsigned-borrow termination.
	int16 TextDisplayCountdown{};

	// The complete vertical Window position byte from Ghidra g_bWindowVerticalPositionBits at 0xFF0AD1. Work
	// RAM has no image-backed value, so managed state starts at zero. Bits zero through four select an eight-
	// pixel boundary and bit seven selects its visible side. DrawDialogueText writes its rendered row count,
	// PresentDialogueAndWait restores two, and ExecuteRoomScriptAction05Or0C brackets temporary content with
	// zero then two.
	uint8 WindowVerticalPositionBits{};

	// The packed lower-interface tile base from Ghidra g_wActionPromptTileAttributes at 0xFF0802–0xFF0803. No
	// image-backed value is consumed; LoadRoomScene derives it after the generated mask tiles, and action-
	// prompt owners add authored cell offsets without removing attributes.
	uint16 ActionPromptTileAttributes{};

	// The packed blank tile cell from Ghidra g_wBlankTileCell at 0xFF08B8–0xFF08B9. The program-image initial
	// value is zero; LoadRoomScene replaces it for room, dialogue, and duel presentation.
	uint16 BlankTileCell{};

	// The packed cursor pattern base from Ghidra g_wCursorSpriteTileAttributes at 0xFF06E4–0xFF06E5. No image-
	// backed value is consumed; LoadRoomScene writes the cursor asset's first tile with priority bit 15 before
	// BuildRoomSpriteTable consumes it.
	uint16 CursorSpriteTileAttributes{};

	// The short lower-interface sprite base from Ghidra g_wShortInterfaceSpriteTileIndex at 0xFF06E6–0xFF06E7.
	// No image-backed value is consumed; LoadRoomScene writes the first of eight replacement tiles before
	// BuildRoomSpriteTable selects it when display bit four is clear.
	uint16 ShortInterfaceSpriteTileIndex{};

	// The tall lower-interface sprite base from Ghidra g_wTallInterfaceSpriteTileIndex at 0xFF06E8–0xFF06E9.
	// No image-backed value is consumed; LoadRoomScene writes it before the interface and optional scripted-
	// coordinate sprite branches consume the shared pattern range.
	uint16 TallInterfaceSpriteTileIndex{};

	// The word offset into the dialogue timing table from Ghidra g_wDialogueTimingTableOffset at 0xFF080C. No
	// image-backed value is consumed; runtime reset selects offset four.
	int16 DialogueTimingTableOffset{};

	// The duel-state and impact mask from Ghidra g_bDuelStateFlags at 0xFF09EA. The program-image value is
	// zero and runtime reset clears every bit. Bit zero marks the duel active. Bits one and two select the
	// first actor's paired impact directions; bits three and four select the second actor's pair. Bit five has
	// no recovered duel role and is preserved. Bits six and seven mark the first and second impact groups
	// active until the action loop clears each group with its pair. The first and second boundary-impact
	// handlers consume bits six and seven after applying the corresponding actor's prior knockback.
	uint8 DuelStateFlags{};

	// The active episode identity from Ghidra g_wEpisodeIndex at 0xFF06AA. No image-backed value is consumed;
	// Reset initializes it to zero before menu dispatch. LoadRoomObjectTable derives and publishes
	// ActiveEpisodeDescriptorOffset from this index before consumers read the selected descriptor's immutable
	// fields.
	int16 EpisodeIndex{};

	// The signed byte offset into the active room-position coordinate table from Ghidra
	// g_swInitialRoomPositionCoordinateOffset at 0xFF069E–0xFF069F. No image-backed value is consumed; runtime
	// reset and room-changing menu commits write zero. Commands 0x11 and 0x18 replace it with their unsigned
	// coordinate index multiplied by four with word wrapping; LoadRoomScene is the sole reader and uses it to
	// select the initial actor coordinate pair.
	int16 InitialRoomPositionCoordinateOffset{};

	// The mixed startup and menu flags from Ghidra g_bInitializationFlags at 0xFF09ED. Bits zero and one
	// select and extend the committed-room menu, bit two records a validated password resume, bit three
	// controls the original byte-RLE decoder's VDP setup, and bit four marks the menu reveal complete. No
	// image-backed value is consumed; InitializeRuntimeState writes zero.
	uint8 InitializationFlags{};

	// The initial room identifier sharing Ghidra g_swInitialRoomOrFirstDuelMomentum at 0xFF0A90–0xFF0A91. The
	// program-image initial value is zero. Managed state separates this startup/password lifetime from the
	// action-0x32 first-actor momentum lifetime.
	int16 InitialRoomId{};

	// The interaction-state flags from Ghidra g_bInteractionFlags at 0xFF09DE. No image-backed value is
	// consumed; runtime reset clears every flag. Bit zero redirects targeting into SecondaryInteraction;
	// command 0x01 can set it while clearing that word. Bit seven excludes actor slot one from room
	// initialization and ambient movement; actions 0x35 and 0x37 also use it to skip waiting for that slot's
	// path and select their alternate actor-slot-two animation. Action 0x09 sets bit seven and action 0x0A
	// clears it. Bit two suppresses command 0x05 action-prompt publication after a room-object relocation. Bit
	// six selects an authored foreground override: UpdateScrollingAndStreamTiles derives foreground
	// displacement from actor zero relative to the camera, and all four edge streamers skip normal foreground-
	// plane transfers. Action 0x07 sets it, action 0x08 clears it, action 0x32 brackets duel presentation with
	// it, and LoadRoomScene clears it. Bit one requests a complete interaction-display refresh; room
	// presentation, dialogue presentation, and action 0x05 set it before RefreshInteractionDisplay consumes
	// and clears it through both recovered refresh branches. Command 0x10 sets bit four when its bit-12 option
	// is present and always clears bit four after dialogue returns, including the branch where it was not set
	// by that command. The interaction-action menu sets bit two for its complete interface lifetime and clears
	// it after restoration; it sets bit four only around selected-label dismissal.
	uint8 InteractionFlags{};

	// The current interaction mode from Ghidra g_wInteractionMode at 0xFF06BE–0xFF06BF. No image-backed RAM
	// value exists, so managed state starts at zero. RunSelectedInteraction snapshots this word before scan-
	// mode command dispatch; command 0x01 compares that stable snapshot with its record field at +2.
	int16 InteractionMode{};

	// The signed menu entries decoded from the active interaction table.
	Common::Array<int16> InteractionMenuItems{};

	// The object indices from Ghidra g_wInventoryObjectIndexListStart at 0xFF0A1E. The original stores words
	// followed by -1; the managed list carries the same ordered indices without retaining the RAM sentinel. No
	// image-backed value is consumed, and LoadRoomObjectTable starts the list empty for both authored
	// episodes. Command 0x05 removes an object by shifting every following index left, or appends it
	// immediately before the original sentinel. Command 0x07 counts the complete list before deriving an
	// inventory page.
	Common::Array<int32> InventoryObjectIndices{};

	// The pending lower-interface command from Ghidra g_wMenuCommand at 0xFF06EA–0xFF06EB. No image-backed
	// value is consumed; the room updater derives it from the cursor and the action menu consumes and clears
	// it.
	int16 MenuCommand{};

	// The current interaction-menu page from Ghidra g_wMenuPage at 0xFF06EC. No image-backed value is
	// consumed; runtime reset selects page zero. When command 0x05 appends an inventory object, it derives the
	// candidate page from the prior item count divided by four and replaces this word unless that page equals
	// the current page plus one. Command 0x07 independently replaces it and commits the prompt update unless
	// the derived inventory page equals this word minus one.
	int16 MenuPage{};

	// The signed interaction-strip column from Ghidra g_wInteractionStripColumn at 0xFF06EE–0xFF06EF. Work RAM
	// starts at zero. RunInteractionActionMenu writes zero while the strip is visible and -1 while hidden;
	// sprite construction rejects only negative values.
	int16 InteractionStripColumn{};

	// The signed interaction-strip row from Ghidra g_wInteractionStripRow at 0xFF06F0–0xFF06F1. Work RAM
	// starts at zero. The menu owner derives its tile row before sprite construction scales it by eight.
	int16 InteractionStripRow{};

	// The packed strip-height bits from Ghidra g_wInteractionStripHeightAttributes at 0xFF06F2–0xFF06F3. Work
	// RAM starts at zero. The menu owner stores visible count minus one in bits 8–15; sprite construction
	// combines them with the four-tile width.
	uint16 InteractionStripHeightAttributes{};

	// The lead actor's signed queued movement-point index from Ghidra g_nLeadActorMovementPointIndex at
	// 0xFF065A. No image-backed value is consumed; runtime reset writes -1 for no queued point.
	int16 LeadActorMovementPointIndex{};

	// The two queued lead-actor points represented by Ghidra g_awLeadActorMovementTargetXCoordinates,
	// g_awLeadActorMovementTargetYCoordinates, g_awLeadActorMovementHorizontalSteps,
	// g_awLeadActorMovementVerticalSteps, and g_awLeadActorMovementPositionIndices at 0xFF065C–0xFF066F. Work
	// RAM starts with zero records; MoveLeadActorToRoomPosition writes the coordinates, signed 8.8
	// multipliers, and destination positions before the room updater consumes them in descending
	// LeadActorMovementPointIndex order.
	Common::Array<ActorMovementPoint> LeadActorMovementPoints = Common::Array<ActorMovementPoint>(2);

	// The left action-prompt countdown from Ghidra g_sbLeftActionPromptCountdown at 0xFF0AB1. No image-backed
	// value is consumed; runtime reset writes zero. Prompt queuing waits for a negative value before writing
	// eight, and each unsuppressed room retrace decrements a nonnegative value through -1 before publishing
	// the left prompt cells.
	int8 LeftActionPromptCountdown{};

	// The left action-prompt index from Ghidra g_wLeftActionPromptIndex at 0xFF0804. No image-backed value is
	// consumed; runtime reset selects index zero. Prompt queuing replaces the word, and retrace publication
	// uses its wrapping signed-word 0x300-byte mode stride into ROM 0x00030016–0x00030915.
	int16 LeftActionPromptIndex{};

	// The next action icon from Ghidra g_wPendingActionIcon at 0xFF06C2–0xFF06C3. Original RAM has no image-
	// backed value; every recovered drawing path writes it before use.
	int16 PendingActionIcon{};

	// The complete table from Ghidra g_abProgressStateBytes at original RAM 0xFF2A00. Managed storage starts
	// all-zero; episode initialization clears all 256 bytes before copying authored defaults, the password
	// display encodes the first 29 bytes, password entry can replace the table, and room-script command 0x08
	// updates indexed bits or big-endian words. Command 0x1C clears byte-zero bit two and restores it only
	// while the selected actor is moving; command 0x1D does the same for byte-zero bit three and the selected
	// actor's animation hold. Shared actions 0x05 and 0x0C return immediately when byte-zero bit one is
	// already set, otherwise set it before their distinct paths. ResetRoomStateForReload skips prompt refresh
	// while the same bit is set. Action 0x1D also sets that bit while its interface tile block is saved;
	// action 0x06 calls FinishRoomStartup only while it remains set, and that owner clears it.
	Common::Array<uint8> ProgressStateBytes = Common::Array<uint8>(256);

	// The prior active action-icon identity from Ghidra g_wPreviousActionIcon at 0xFF06B8–0xFF06B9. Work RAM
	// has no image-backed value, so managed state starts at zero. RunAdventure and PresentRoom copy
	// ActiveActionIcon here before RefreshInteractionDisplay compares the snapshot.
	int16 PreviousActionIcon{};

	// The prior room-name cartridge pointer from Ghidra g_pPreviousRoomNameText at 0xFF06D6–0xFF06D9. Work RAM
	// has no image-backed value, so managed state starts at zero. RunAdventure and PresentRoom copy
	// RoomNameTextOffset here; RefreshInteractionDisplay is the sole reader.
	int32 PreviousRoomNameTextOffset{};

	// The prior display flags from Ghidra g_bPreviousDisplayFlags at 0xFF09DD. Work RAM has no image-backed
	// value, so managed state starts at zero. RunAdventure and PresentRoom copy DisplayFlags here before
	// RefreshInteractionDisplay reads two flag branches.
	uint8 PreviousDisplayFlags{};

	// The prior interaction flags from Ghidra g_bPreviousInteractionFlags at 0xFF09DF. Work RAM has no image-
	// backed value, so managed state starts at zero. RunAdventure and PresentRoom copy InteractionFlags here;
	// RefreshInteractionDisplay is the sole reader.
	uint8 PreviousInteractionFlags{};

	// The previous selected interaction from Ghidra g_wPreviousInteraction at 0xFF06B4–0xFF06B5. No image-
	// backed value is consumed; interface entry clears it and interaction refresh otherwise maintains the
	// snapshot.
	int16 PreviousInteraction{};

	// The prior secondary-interaction identity from Ghidra g_wPreviousSecondaryInteraction at
	// 0xFF06BC–0xFF06BD. Work RAM has no image-backed value, so managed state starts at zero. RunAdventure and
	// PresentRoom copy SecondaryInteraction here; RefreshInteractionDisplay is the sole reader.
	int16 PreviousSecondaryInteraction{};

	// The ambient companion animation base from Ghidra g_wRandomMoveAnimation at 0xFF063E–0xFF063F. Work RAM
	// starts at zero; ambient movement selects it and the room updater combines it with the chosen direction
	// after animation completion.
	int16 RandomMoveAnimation{};

	// The ambient-movement entry count from Ghidra g_wRandomMoveCount at 0xFF06A0–0xFF06A1. Work RAM starts at
	// zero; LoadRoomScene counts entries before the first 0xFFFFFFFF sentinel or retains eight when no earlier
	// sentinel exists.
	int16 RandomMoveCount{};

	// The ambient companion direction or position from Ghidra g_wRandomMoveDirection at 0xFF0640–0xFF0641.
	// Work RAM starts at zero; ambient movement selects it and the room updater uses it for animation and
	// position transitions.
	int16 RandomMoveDirection{};

	// The cartridge offset from Ghidra g_pRoomMovementData at 0xFF069A–0xFF069D. Work RAM starts at zero;
	// LoadRoomScene points it immediately after the eight initial coordinate pairs. Offsets +0x00–+0x1F hold
	// up to eight four-byte ambient entries, while +0x20–+0x27 hold two signed waypoint pairs shared by lead
	// and companion route selection.
	int32 RoomMovementDataOffset{};

	// The ambient actor movement timer from Ghidra g_wRandomMoveTimer at 0xFF0688. No image-backed value is
	// consumed; runtime reset writes zero.
	int16 RandomMoveTimer{};

	// The signed automatic idle-animation countdown from Ghidra g_wLeadActorIdleAnimationTimer at 0xFF0686.
	// Work RAM has no image-backed value; InitializeRoomActors writes 300. The room updater decrements it
	// while actor zero is stationary, resets it on movement paths, and requests animation offset 0x20 plus the
	// signed room-position index after it becomes negative.
	int16 LeadActorIdleAnimationTimer{};

	// The command replayed after the sound-test menu from Ghidra g_lResumeAudioCommand at 0xFF06A6. No image-
	// backed value is consumed; RunStartupSequence writes 0x2F before the first menu, and LoadRoomObjectTable
	// replaces it with the selected episode header's sign-extended word.
	int32 ResumeAudioCommand{0x2F};

	// The companion actor's signed queued movement-point index from Ghidra g_nCompanionActorMovementPointIndex
	// at 0xFF0670. No image-backed value is consumed; runtime reset writes -1 for no queued point.
	int16 CompanionActorMovementPointIndex{};

	// The two queued companion points represented by Ghidra g_awCompanionActorMovementTargetXCoordinates,
	// g_awCompanionActorMovementTargetYCoordinates, g_awCompanionActorMovementHorizontalSteps,
	// g_awCompanionActorMovementVerticalSteps, and g_awCompanionActorMovementPositionIndices at
	// 0xFF0672–0xFF0685. Work RAM starts with zero records; MovePlayerToRoomPosition writes the coordinates
	// and invokes the separate route builders that write signed 8.8 multipliers and destination positions
	// before the room updater consumes them in descending CompanionActorMovementPointIndex order.
	Common::Array<ActorMovementPoint> CompanionActorMovementPoints = Common::Array<ActorMovementPoint>(2);

	// The right action-prompt countdown from Ghidra g_sbRightActionPromptCountdown at 0xFF0AB2. No image-
	// backed value is consumed; runtime reset writes zero. Prompt queuing waits for a negative value before
	// writing eight, and each unsuppressed room retrace decrements a nonnegative value through -1 before
	// publishing the right prompt cells.
	int8 RightActionPromptCountdown{};

	// The right action-prompt index from Ghidra g_wRightActionPromptIndex at 0xFF0806. No image-backed value
	// is consumed; runtime reset selects index zero. Prompt queuing replaces the word, and retrace publication
	// uses its wrapping signed-word 0x300-byte mode stride into ROM 0x00030016–0x00030915.
	int16 RightActionPromptIndex{};

	// The word offset into the active vertical-scroll phase table from Ghidra g_wScrollAnimationPhaseOffset at
	// 0xFF08AA. No image-backed value is consumed; the episode opening and room-script action 0x1A each
	// initialize it before scrolling reads it.
	int16 ScrollAnimationPhaseOffset{};

	// The packed delay cursor from Ghidra g_wScrollAnimationDelayCursor at 0xFF08AC–0xFF08AD. No image-backed
	// value is consumed. The episode opening reads and writes the complete word as its one-entry delay index.
	// Action 0x1A writes word one, selecting room-delay entry zero in the high byte;
	// UpdateScrollingAndStreamTiles updates only that byte and preserves the low-byte value one.
	int16 ScrollAnimationDelayCursor{};

	// The signed retrace countdown from Ghidra g_swRetraceCountdown at 0xFF08AE. No image-backed value is
	// consumed; managed state starts at zero, and each recovered wait owner writes its inclusive count before
	// the installed VBlank callback decrements it through -1. Action 0x1E writes 300 after streaming its final
	// foreground row and updates actor animations until the countdown becomes negative. Command 0x15 copies
	// its signed record field and performs the same animation update before every test, including one update
	// when the authored count is already negative.
	int16 RetraceCountdown{};

	// The active room identifier from Ghidra g_wRoomId at 0xFF06AC. No image-backed value is consumed; runtime
	// reset writes zero before the selected episode descriptor restores its room. Command 0x11 replaces it
	// before fading and reloading the selected room; command 0x18 replaces it before resetting the selected
	// room without presenting it.
	int16 RoomId{};

	// The mutable state shared by room-script actions 0x17–0x19 and BuildRoomSpriteTable for their optional
	// coordinate-driven sprite.
	ScriptedCoordinateSpriteState ScriptedCoordinateSprite;

	// The packed dynamic-tile base from Ghidra g_wRoomDynamicTileBaseAttribute at 0xFF068A–0xFF068B. No image-
	// backed value is consumed; managed state starts at zero, and LoadRoomScene replaces it with the first
	// free tile index plus one and priority bit 15 before room-script actions 0x07 or 0x1E consume it.
	uint16 RoomDynamicTileBaseAttribute{};

	// The mixed room-behavior mask from Ghidra g_bRoomBehaviorFlags at 0xFF09E8. Its bits gate actor refresh,
	// sprite construction, room presentation, and screen shake. No image-backed value is consumed; runtime
	// reset clears every flag. Command 0x01 clears bit zero for interaction mode five and sets it for mode
	// six. Bit seven enables the phase-table vertical-scroll offset in UpdateScrollingAndStreamTiles; actions
	// 0x1A and 0x1B set and clear it. Bit four publishes the optional scripted coordinate sprite in
	// BuildRoomSpriteTable; actions 0x17 and 0x18 set it, while action 0x19 clears it without disturbing bit
	// five or the prepared sprite state. Bit five clamps that sprite's coordinate cursor at its final entry,
	// and bit six selects its second four-tile frame. Bit three couples actor zero with TransitionActorIndex
	// for synchronized animation, scale, and sprite publication; actions 0x15 and 0x16 set and clear it. Bit
	// two suppresses normal actor-zero movement and automatic animation selection in
	// UpdateRoomActorsAndInterface; when duel-state bit zero is active, that updater retains the duel path
	// instead. Actions 0x11 and 0x12 set and clear bit two, and action 0x32 brackets the duel with the same
	// mode. Bit one disables automatic camera following in UpdateRoomActorsAndInterface; actions 0x0D, 0x0F,
	// and 0x10 set it, while action 0x0E clears it.
	uint8 RoomBehaviorFlags{};

	// The decoded room-coordinate data offset corresponding to Ghidra g_pRoomCoordinateData at 0xFF0692. No
	// image-backed value is consumed; runtime reset clears the pointer equivalent.
	int32 RoomCoordinateDataOffset{};

	// The horizontal room/interface split displacement from Ghidra g_wRoomInterfaceHorizontalScroll at
	// 0xFF0800. No image-backed value is consumed; runtime reset writes zero.
	int16 RoomInterfaceHorizontalScroll{};

	// The signed room/interface transition countdown from Ghidra g_sbRoomInterfaceTransitionCountdown at
	// 0xFF0ACB. No image-backed value is consumed; runtime reset writes -1 for inactive. Action 0x1D and
	// FinishRoomStartup write 16; RunInteractionActionMenu does so before its active menu and cleanup
	// transitions. UpdateRoomActorsAndInterface advances the split and decrements it through zero to -1.
	int8 RoomInterfaceTransitionCountdown{};

	// The logical identity encoded by Ghidra g_wSavedInteractionTileBlockVramAddress at 0xFF0876–0xFF0877.
	// Process reset clears the word, represented by absent managed state and the pattern-memory destination at
	// byte offset zero. Action 0x1D replaces it with the selected left (0xAA80) or right (0xAAC0) 6-row by
	// 32-cell block. FinishRoomStartup is the sole consumer and preserves all three outcomes.
	Optional<bool> SavedInteractionTileBlockOnRight{};

	// The decoded records from Ghidra g_aRoomObjectStates at 0xFF1200. No image-backed RAM value is consumed;
	// LoadRoomObjectTable replaces the complete array from the selected episode's authored source stream.
	Common::Array<RoomObject> RoomObjects{};

	// The active decoded room resources replacing the workspaces rooted at Ghidra g_awRoomBackgroundCells
	// (0xFF7000), g_awMenuRowShiftQueueOrRoomForegroundCells (0xFF8C00), their snapshots, and
	// g_abWideRevealOrRoomCollisionWorkspace (0xFFE000). Managed state starts absent; LoadRoomScene publishes
	// one complete resource set per selected room.
	Common::ScopedPtr<RoomSceneData> ActiveRoomScene{};

	// The exclusive ROM end offset corresponding to original register A6. Automatic-interaction processing
	// replaces it with each scan and nested execution bound, then restores the caller's value.
	// RunInteractionActionMenu also saves and restores complete parent bounds around selected scripts.
	int32 RoomScriptEndOffset{};

	// The room-script control flags from Ghidra g_bRoomScriptFlags at 0xFF0ACA. Bits one and zero jointly stop
	// room-script and dialogue processing; room retrace and held Start in the main menu latch bit zero, while
	// actions 0x38 and 0x39 clear and set bit one. Bits two and three independently gate the action-menu and
	// selected-interaction loops. Action 0x28 sets bit two so RunInteractionActionMenu clears it after room-
	// script processing, discards every saved nested menu/script frame, and exits instead of nesting again.
	// Action 0x2F is the sole bit-three setter; RunSelectedInteraction clears and tests it around scripted
	// processing. No image-backed value is consumed; runtime reset clears every flag.
	uint8 RoomScriptFlags{};

	// The current ROM script offset corresponding to original register A5. Automatic-interaction processing
	// replaces it with each scan and nested execution cursor, then restores the caller's value.
	// RunInteractionActionMenu restores each parent cursor and the selected script's initial cursor.
	int32 RoomScriptStartOffset{};

	// The scripted interaction from Ghidra g_wScriptedInteraction at 0xFF06DA–0xFF06DB. No image-backed value
	// is consumed; managed state starts at zero. Action 0x2F copies the first room object's script result
	// here, and RunSelectedInteraction consumes it when bit three is set.
	int16 ScriptedInteraction{};

	// The signed phase delay from Ghidra g_sbScrollAnimationDelay at 0xFF0ACF. No image-backed value is
	// consumed; opening and room-scroll owners load it before decrementing it.
	int8 ScrollAnimationDelay{};

	// The active-low controller-one state from Ghidra g_bControllerOneInput at 0xFF09E0. No image-backed value
	// is consumed; runtime reset and ResetRoomStateForReload write 0xFF for no buttons held, with the room
	// reset doing so immediately before scene loading. Action 0x04 gives A priority over B and returns only on
	// a new press relative to the prior sample. Action 0x05 tests A and B independently as exits from each
	// temporary-presentation iteration. Action 0x21 tests Left, Right, then Down and publishes result 0, 1, or
	// 2.
	uint8 ControllerOneInput{0xFF};

	// The prior controller-one sample from Ghidra g_bPreviousControllerOneInput at 0xFF09E2. No image-backed
	// value is consumed; runtime reset writes 0xFF. Action 0x04 compares A first and B only while current A is
	// released, distinguishing each new press from a held button.
	uint8 PreviousControllerOneInput{0xFF};

	// The consumer-selected controller-one comparison baseline from Ghidra g_bControllerOneChangeBaseline at
	// 0xFF09E4. No image-backed value is consumed; managed state starts at zero until a menu input handler
	// accepts a buffered press.
	uint8 ControllerOneChangeBaseline{};

	// The controller-one changes accumulated in Ghidra g_bControllerOneAccumulatedChanges at 0xFF09E6. No
	// image-backed value is consumed; managed state starts at zero, polling accumulates changes, and a menu
	// input handler clears them when accepting a buffered press.
	uint8 ControllerOneAccumulatedChanges{};

	// The secondary selected interaction from Ghidra g_wSecondaryInteraction at 0xFF06B6–0xFF06B7. No image-
	// backed RAM value exists, so managed state starts at zero. Cursor and action-menu targeting replace it
	// while interaction bit zero is active; command 0x01 independently rechecks, matches, and clears it along
	// its recovered branches.
	int16 SecondaryInteraction{};

	// The shared 64-color fade target formed by Ghidra g_awSharedPalette at 0xFF0778-0xFF07B7 and
	// g_awRoomPalette at 0xFF07B8-0xFF07F7. No image-backed RAM value is consumed; runtime reset copies the 32
	// authored colors from ROM g_awDefaultSharedPalette at 0x00032480-0x000324BF into the first half, and room
	// loading replaces the second half before presentation.
	Common::Array<PaletteColor> TargetPalette = Common::Array<PaletteColor>(64);

	// The animated-palette enable mask from Ghidra g_bAnimatedPaletteRangeFlags at 0xFF0ACD. No image-backed
	// value exists; LoadRoomScene clears the byte. Bits zero through four select the five configured ranges,
	// while command 0x1A retains modulo-eight selection for its signed disable branch.
	uint8 AnimatedPaletteRangeFlags{};

	// The five unsigned first-color indices from Ghidra g_awAnimatedPaletteRangeStartIndices at
	// 0xFF087C–0xFF0885. RAM has no image-backed values; command 0x1A writes a selected slot before enabling
	// it, and RotateAnimatedPaletteRanges consumes active slots.
	Common::Array<uint16> AnimatedPaletteRangeStartIndices = Common::Array<uint16>(5);

	// The five unsigned last-color indices from Ghidra g_awAnimatedPaletteRangeEndIndices at
	// 0xFF0886–0xFF088F. RAM has no image-backed values; command 0x1A writes a selected slot before enabling
	// it, and palette rotation treats the endpoint as inclusive.
	Common::Array<uint16> AnimatedPaletteRangeEndIndices = Common::Array<uint16>(5);

	// The five signed reload intervals from Ghidra g_aswAnimatedPaletteRangeIntervals at 0xFF0890–0xFF0899.
	// RAM has no image-backed values; command 0x1A stores a nonnegative control word, and palette rotation
	// reloads the matching countdown after it becomes negative.
	Common::Array<int16> AnimatedPaletteRangeIntervals = Common::Array<int16>(5);

	// The five signed live countdowns from Ghidra g_aswAnimatedPaletteRangeCountdowns at 0xFF089A–0xFF08A3.
	// RAM has no image-backed values; command 0x1A initializes a selected slot from its interval, and palette
	// rotation decrements enabled slots through -1.
	Common::Array<int16> AnimatedPaletteRangeCountdowns = Common::Array<int16>(5);

	// The directional tile-streaming mask from Ghidra g_bTileStreamingEdgeFlags at 0xFF0AB3. No image-backed
	// value is consumed; runtime reset clears every edge. Bits zero through three request the right, left,
	// top, and bottom streamers independently. The scrolling owner consumes them in that order and clears the
	// complete byte unless video bit one bypasses edge processing.
	uint8 TileStreamingEdgeFlags{};

	// The room-transition flags from Ghidra g_bTransitionFlags at 0xFF09E9. No image-backed value is consumed;
	// runtime reset clears every flag. Bit seven suppresses ChooseRandomInteractionAnimation; room-script
	// actions 0x30 and 0x31 set and clear it. Bit six skips normal actor-zero animation synchronization and
	// forces cursor position (0x80, 0x50); actions 0x2D and 0x2E set and clear it. Bit five marks a scripted
	// horizontal camera pan active. Actions 0x0F and 0x10 set it after writing CameraPanStep and
	// CameraPanTargetX; UpdateScrollingAndStreamTiles advances the camera and clears it at the exact target.
	// Bit four selects actor zero's alternate transition mode, suppressing its normal synchronization, random
	// animation selection, and Start exit while redirecting movement and cursor processing; actions 0x2B and
	// 0x2C set and clear it. Bit three couples actor zero with actor slot two for positioning, priority, slot
	// processing, animation restarts, and compact-frame conversion; actions 0x29 and 0x2A set and clear it.
	// Bit two makes sprite construction select the highest-numbered staged actor immediately; action 0x27 owns
	// its complete set-through-clear lifecycle. Bit one suppresses coordinate-driven horizontal and vertical
	// scale recalculation for compact actors; actions 0x23 and 0x24 set and clear it. Bit zero reverses lead-
	// actor vertical movement polarity so Up adds and Down subtracts the scaled step; actions 0x1F and 0x20
	// set and clear it.
	uint8 TransitionFlags{};

	// The video-state flags from Ghidra g_bVideoFlags at 0xFF09EB. No image-backed value is consumed; runtime
	// reset clears every flag. Bit zero selects the direct TransferRoomObjectTilePatch path from
	// ApplyRoomObjectTilePatch; room-script actions 0x33 and 0x34 set and clear it. Bit one bypasses normal
	// scroll derivation and edge streaming while retaining publication of the current scroll words; action
	// 0x1E sets it for its direct vertical-scroll sequence. Bit two gates ambient random movement and skips
	// the actor slot-update block; room-script action 0x0B sets it after deriving slot one's animation offset,
	// and action 0x36 clears it. Bit five independently suppresses RefreshInteractionDisplay; room-script
	// action 0x3A sets it and action 0x3B clears it. Bit six switches actor slot one from normal movement to
	// the path that copies slot zero's integer coordinates and derives its animation offset from slot zero's
	// position index with the original input-dependent adjustment; actions 0x3C and 0x3D set and clear it.
	uint8 VideoFlags{};

	// The foreground horizontal displacement from Ghidra g_wForegroundHorizontalScroll at 0xFF07F8. No image-
	// backed value is consumed; runtime reset writes zero before room scrolling.
	int16 ForegroundHorizontalScroll{};

	// The foreground vertical displacement from Ghidra g_wForegroundVerticalScroll at 0xFF07FC. No image-
	// backed value is consumed; runtime reset writes zero before room scrolling. Action 0x1E steps it from
	// zero through 2535, holding each value for two retraces while video bit one prevents normal scroll
	// derivation from overwriting it.
	int16 ForegroundVerticalScroll{};

	// XORs one big-endian word in the mutable progress table.
	// byteOffset: signed byte offset loaded from the room-script command record.
	// mask: original 16-bit one-hot result to combine with the current word.
	void xorProgressStateWord(int byteOffset, uint16 mask);

	// Restores the shared runtime state while preserving the episode selected by the title menu.
	//
	// Ghidra: resetRuntimeState (0x0000A27A-0x0000A385). The reset preserves g_wEpisodeIndex at
	// 0xFF06AA, initializes controller-one current and prior samples to active-low 0xFF, writes -1
	// to queued movement indices and the room/interface transition countdown, and decodes all 32
	// colors from g_awDefaultSharedPalette at 0x00032480-0x000324BF. Controller-two slots and the
	// saved VDP-register byte have no managed runtime consumer.
	void resetRuntimeState(const ScoobyDooRom &rom);

	// Advances the recovered A5/A6 script window to the next length-prefixed block.
	void advanceRoomScriptBlock(const ScoobyDooRom &rom);
};
} // namespace Scooby

#endif // SCOOBY_RUNTIME_STATE_H
