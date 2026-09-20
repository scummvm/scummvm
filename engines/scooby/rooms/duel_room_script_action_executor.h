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

#ifndef SCOOBY_DUEL_ROOM_SCRIPT_ACTION_EXECUTOR_H
#define SCOOBY_DUEL_ROOM_SCRIPT_ACTION_EXECUTOR_H

#include "common/scummsys.h"

#include "common/array.h"
#include "common/func.h"

#include "room_collision_probe.h"
#include "scooby/assets/rom.h"
#include "scooby/graphics/tile_scene.h"
#include "scooby/presentation/frame_presenter.h"
#include "scooby/randomness/deterministic_random.h"
#include "scooby/runtime/runtime_state.h"
#include "scooby/runtime/session_exit.h"

// Owns room-script action 0x32's persistent duel state, presentation resources, and recovered collision
// boundaries.

namespace Scooby {

class DuelRoomScriptActionExecutor {
public:
	// Creates the persistent owner for action 0x32's distinct duel subsystem.
	// rom: verified cartridge containing the authored duel graphics.
	// scene: logical tile scene receiving duel presentation updates.
	// state: shared actor, input, room, and lifecycle state.
	// collisionProbe: shared owner of decoded room collision queries.
	// random: shared deterministic state used for each actor's impact-direction choice.
	// presenter: shared host presentation owner enforcing the room retrace cadence.
	// updateActorAnimationFrames: recovered actor-animation update used by the duel loop.
	// handleRoomVBlank: installed room callback used at every recovered retrace boundary.
	// executeAction02: action-0x02 boundary; returns false for a managed session exit.
	// executeAction03: canonical action-0x03 boundary called during duel teardown.
	// requestSessionExit: parent handover used when the host closes during a wait.
	DuelRoomScriptActionExecutor(const ScoobyDooRom &rom, TileScene &scene,
								 RuntimeState &state, const RoomCollisionProbe &collisionProbe,
								 DeterministicRandom &random,
								 FramePresenter &presenter,
								 Common::Functor0<void> &updateActorAnimationFrames,
								 Common::Functor0<void> &handleRoomVBlank, Common::Functor0<bool> &executeAction02,
								 Common::Functor0<void> &executeAction03,
								 Common::Functor1<SessionExit, void> &requestSessionExit)
		: _rom(rom), _scene(scene), _state(state), _collisionProbe(collisionProbe), _random(random),
		  _presenter(presenter), _updateActorAnimationFrames(updateActorAnimationFrames),
		  _handleRoomVBlank(handleRoomVBlank), _executeAction02(executeAction02),
		  _executeAction03(executeAction03),
		  _requestSessionExit(requestSessionExit) {
	}

	// Runs the complete scripted two-actor duel from presentation setup through winner publication.
	//
	// Ghidra: executeRoomScriptAction32 (0x00003142). Packed pattern data at 0x0001900C-0x0001938B supplies
	// 28 tiles loaded at logical tile index 0x49C. The five eight-word rows at 0x0001938C-0x000193DB supply
	// relative frame-tile indices. Signed movement data at 0x00003D96-0x00003E95 contains eight facing
	// blocks, each with eight X deltas followed by eight Y deltas. Managed scene publication replaces VDP
	// transfers while retaining source extents, tile indices, plane positions, buffer reuse, and call
	// ordering. The installed room callback and host clock supply interrupt-driven progress at every duel
	// retrace.
	void executeRoomScriptAction32();

private:
	struct DuelMovementDelta {
		int16 X;
		int16 Y;
	};

	struct DuelActorBounds {
		int16 Left;
		int16 Top;
		int16 Right;
		int16 Bottom;
	};

	static const int kDuelAnimationFrameTableOffsetField = 0x04;
	static const int kDuelCollisionBoundsFrameOffset = 0x0E;
	static const int kDuelFirstActor = 0;
	static const int kDuelFirstActorDescriptorOffset = 0xF88DC;
	static const int kDuelFirstActorSlot = 0;
	// ROM g_awDuelFrameTileOffsets at 0x0001938C-0x000193DB supplies the authored five-row frame cells.
	static const int kDuelFrameTileOffsetsOffset = 0x1938C;
	static const int kDuelPatternTileIndex = 0x49C;
	// ROM g_abDuelPatternTiles at 0x0001900C-0x0001938B stores the 28 packed tiles loaded for the duel frame.
	static const int kDuelPatternTilesOffset = 0x1900C;
	static const int kDuelSecondActor = 1;
	static const int kDuelSecondActorDescriptorOffset = 0xFA77A;
	static const int kDuelSecondActorSlot = 2;
	static const int kDuelStatusTileByteLength = 0x100;

	// Ghidra g_awOppositeDuelFacingDirections at 0x00003D86-0x00003D95 maps each relative direction to its
	// opposite facing. The eight immutable program-image words are 4, 5, 6, 7, 0, 1, 2, 3.
	static const Common::Array<int16> kOppositeDuelFacingDirections;

	// The 64 logical X/Y pairs from Ghidra g_aswDuelMovementDeltas at 0x00003D96-0x00003E95. Each facing
	// contributes eight signed X words followed by eight signed Y words in ROM; this typed projection
	// preserves the same facing-then-magnitude indexing.
	static const Common::Array<DuelMovementDelta> kDuelMovementDeltas;

	void initializeDuelPresentation();
	void writeDuelFrameCells(int sourceOffset, int startColumn, int row);
	void publishDuelStatusTiles();
	bool waitForRoomFrames(int frameCounter);
	bool waitForRoomVerticalBlank();
	static int16 getIntegerCoordinate(int coordinate);
	static int16 halveWordLogically(int16 value);
	static int32 replaceIntegerCoordinate(int32 coordinate, int16 integer);
	static void pauseAudioDriver();
	static void resumeAudioDriver();
	static void playAudioCommand(int command);
	static void stopAudioPlayback(int command);
	void buildDuelStatusBarTiles();
	static void buildDuelHealthTiles(Span<uint8> tiles, int16 health,
									 uint32 firstPixel);
	int16 lookupDuelFacingDirection();
	int16 computeDuelRelativeDirection();
	void resolveDuelActorCollision();
	void resolveDuelFirstActorImpact();
	void resolveDuelSecondActorImpact();
	bool testDuelActorOverlap();
	DuelActorBounds readDuelActorBounds(int actorSlot, int descriptorOffset);
	static bool intervalsOverlapInclusively(int16 firstStart, int16 firstEnd,
											int16 secondStart, int16 secondEnd);

	const ScoobyDooRom &_rom;
	TileScene &_scene;
	RuntimeState &_state;
	const RoomCollisionProbe &_collisionProbe;
	DeterministicRandom &_random;
	FramePresenter &_presenter;
	Common::Functor0<void> &_updateActorAnimationFrames;
	Common::Functor0<void> &_handleRoomVBlank;
	Common::Functor0<bool> &_executeAction02;
	Common::Functor0<void> &_executeAction03;
	Common::Functor1<SessionExit, void> &_requestSessionExit;

	// The first and second prior integer X coordinates from Ghidra g_swFirstDuelPreviousX at
	// 0xFF0A84-0xFF0A85 and g_swSecondDuelPreviousX at 0xFF0A88-0xFF0A89. Both program-image values are
	// zero and remain live across action invocations.
	Common::Array<int16> _duelPreviousXCoordinates = Common::Array<int16>(2);

	// The first and second prior integer Y coordinates from Ghidra g_swFirstDuelPreviousY at
	// 0xFF0A86-0xFF0A87 and g_swSecondDuelPreviousY at 0xFF0A8A-0xFF0A8B. Both program-image values are
	// zero and remain live across action invocations.
	Common::Array<int16> _duelPreviousYCoordinates = Common::Array<int16>(2);

	// The first and second prior facings from Ghidra g_swFirstDuelPreviousFacing at 0xFF0A8C-0xFF0A8D and
	// g_swSecondDuelPreviousFacing at 0xFF0A8E-0xFF0A8F. Both program-image values are zero and remain live
	// across action invocations.
	Common::Array<int16> _duelPreviousFacings = Common::Array<int16>(2);

	// The first and second signed momentum words from Ghidra g_swInitialRoomOrFirstDuelMomentum at
	// 0xFF0A90-0xFF0A91 and g_swSecondDuelMomentum at 0xFF0A92-0xFF0A93. Both program-image values are
	// zero; the first entry is separated from RuntimeState::InitialRoomId's disjoint lifetime.
	// ResolveDuelActorCollision weights both words by the actors' facings, transfers the first to
	// second-actor knockback, and clears the second before copying it into first-actor knockback. Each
	// actor-specific boundary-impact handler transfers its own momentum to knockback without a clamp, then
	// clears that momentum.
	Common::Array<int16> _duelMomentum = Common::Array<int16>(2);

	// The signed knockback words from Ghidra g_swFirstDuelKnockback at 0xFF0A94-0xFF0A95 and
	// g_swSecondDuelKnockback at 0xFF0A96-0xFF0A97. Both program-image values are zero.
	// ResolveDuelActorCollision copies the opposite actor's momentum into each slot and applies a signed
	// minimum of four before the action loop consumes it. Each actor-specific boundary-impact handler
	// instead replaces its own slot with current momentum without clamping.
	Common::Array<int16> _duelKnockback = Common::Array<int16>(2);

	// The signed health words from Ghidra g_swFirstDuelHealth at 0xFF0A98-0xFF0A99 and g_swSecondDuelHealth
	// at 0xFF0A9A-0xFF0A9B. Both program-image values are zero. ResolveDuelActorCollision subtracts
	// facing-weighted momentum with signed word wrapping. The two boundary-impact handlers conditionally
	// subtract their actor's prior knockback with the same wrapping when the corresponding duel-state group
	// bit is set.
	Common::Array<int16> _duelHealth = Common::Array<int16>(2);

	// The momentum cooldown bytes from Ghidra g_bFirstDuelMomentumCooldown at 0xFF0AA0 and
	// g_bSecondDuelMomentumCooldown at 0xFF0AA1. Both program-image values are zero; ResolveDuelActorCollision
	// sets both to eight after clearing the corresponding momentum, and each boundary-impact handler sets its
	// actor's slot to eight.
	Common::Array<uint8> _duelMomentumCooldowns = Common::Array<uint8>(2);

	// The knockback cooldown bytes from Ghidra g_bFirstDuelKnockbackCooldown at 0xFF0AA2 and
	// g_bSecondDuelKnockbackCooldown at 0xFF0AA3. Both program-image values are zero;
	// ResolveDuelActorCollision sets both to eight when it arms the two impact groups, and each
	// boundary-impact handler sets its actor's slot to eight.
	Common::Array<uint8> _duelKnockbackCooldowns = Common::Array<uint8>(2);

	// The shared first duel-status workspace from Ghidra g_abSharedTextWorkspace at 0xFF08BC-0xFF09BB. Its
	// program-image value is zero; this owner retains only the duel lifetime.
	Common::Array<uint8> _firstDuelStatusTiles = Common::Array<uint8>(kDuelStatusTileByteLength);

	// The duel-owned lifetime of Ghidra g_abSharedTileWorkspace at 0xFF0328-0xFF0427. Its program-image
	// value is zero and remains live across action invocations.
	Common::Array<uint8> _secondDuelStatusTiles = Common::Array<uint8>(kDuelStatusTileByteLength);

	// The per-frame impact bits from Ghidra g_bDuelCollisionFlags at 0xFF09EC. Its program-image initial
	// value is zero.
	uint8 _duelCollisionFlags{};

	// The cached target facing from Ghidra g_swLastDuelTargetFacing at 0xFF0A9C-0xFF0A9D. Its program-image
	// initial value is zero.
	int16 _duelLastTargetFacing{};

	// The cached signed turn step from Ghidra g_swDuelTurnStep at 0xFF0A9E-0xFF0A9F. Its program-image
	// initial value is zero and it is deliberately retained while the target is unchanged.
	int16 _duelTurnStep{};

	// The AI turn cooldown from Ghidra g_bDuelTurnCooldown at 0xFF0AA4. Its program-image initial value is
	// zero.
	uint8 _duelTurnCooldown{};

	// The high signed integer word of Ghidra g_nStagedCollisionX at 0xFF064A-0xFF064D. Work RAM starts at
	// zero; this action overwrites the high word immediately before each collision probe, so no retained
	// path-tracing fraction is consumed.
	int16 _stagedCollisionX{};

	// The high signed integer word of Ghidra g_nStagedCollisionY at 0xFF064E-0xFF0651. Work RAM starts at
	// zero; this action overwrites the high word immediately before each collision probe, so no retained
	// path-tracing fraction is consumed.
	int16 _stagedCollisionY{};
};
} // namespace Scooby

#endif // SCOOBY_DUEL_ROOM_SCRIPT_ACTION_EXECUTOR_H
