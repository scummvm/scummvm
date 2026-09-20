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

#include "duel_room_script_action_executor.h"

#include "common/scummsys.h"
#include "common/algorithm.h"

#include "common/array.h"

#include "scooby/common/span.h"
#include "scooby/graphics/genesis_asset_decoder.h"
#include "scooby/graphics/tile_layer.h"

namespace Scooby {
const Common::Array<int16> DuelRoomScriptActionExecutor::kOppositeDuelFacingDirections{
	4, 5, 6, 7, 0, 1, 2, 3};

const Common::Array<DuelRoomScriptActionExecutor::DuelMovementDelta>
	DuelRoomScriptActionExecutor::kDuelMovementDeltas{
		{0, 0}, {0, -1}, {0, -2}, {0, -3}, {0, -4}, {0, -5}, {0, -6}, {0, -7}, {0, 0}, {1, -1}, {2, -2}, {3, -3}, {3, -3}, {4, -4}, {4, -4}, {5, -5}, {0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0}, {0, 0}, {1, 1}, {2, 2}, {3, 3}, {3, 3}, {4, 4}, {4, 4}, {5, 5}, {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7}, {0, 0}, {-1, 1}, {-2, 2}, {-3, 3}, {-3, 3}, {-4, 4}, {-4, 4}, {-5, 5}, {0, 0}, {-1, 0}, {-2, 0}, {-3, 0}, {-4, 0}, {-5, 0}, {-6, 0}, {-7, 0}, {0, 0}, {-1, -1}, {-2, -2}, {-3, -3}, {-3, -3}, {-4, -4}, {-4, -4}, {-5, -5}};

namespace {
void WriteUInt32BigEndian(Span<uint8> bytes, int offset, uint32 value) {
	bytes[static_cast<std::size_t>(offset)] = static_cast<uint8>(value >> 24);
	bytes[static_cast<std::size_t>(offset + 1)] = static_cast<uint8>(value >> 16);
	bytes[static_cast<std::size_t>(offset + 2)] = static_cast<uint8>(value >> 8);
	bytes[static_cast<std::size_t>(offset + 3)] = static_cast<uint8>(value);
}

void WriteUInt16BigEndian(Span<uint8> bytes, int offset, uint16 value) {
	bytes[static_cast<std::size_t>(offset)] = static_cast<uint8>(value >> 8);
	bytes[static_cast<std::size_t>(offset + 1)] = static_cast<uint8>(value);
}
} // namespace

void DuelRoomScriptActionExecutor::executeRoomScriptAction32() {
	// Ghidra 0x00003142-0x00003265: replace raster-counter setup and every VDP transfer with the same
	// logical custom tiles, border cells, authored pattern tiles, and five-row plane-A frame publication.
	int16 savedFirstActorPositionIndex = _state.ActorPositionIndices[kDuelFirstActorSlot];
	initializeDuelPresentation();

	// Ghidra 0x00003266-0x000032CB: initialize every explicitly written duel flag, position, momentum,
	// knockback, cooldown, health, and target-facing value. Prior snapshots and the cached turn step retain
	// their Work RAM contents because this range does not clear them.
	_state.DuelStateFlags = 0x01;
	_state.RoomBehaviorFlags |= 0x04;
	_state.InteractionFlags |= 0x40;
	_state.ActorPositionIndices[kDuelFirstActorSlot] = 0;
	Common::fill(_duelMomentum.begin(), _duelMomentum.end(), 0);
	Common::fill(_duelKnockback.begin(), _duelKnockback.end(), 0);
	Common::fill(_duelMomentumCooldowns.begin(), _duelMomentumCooldowns.end(), 0);
	Common::fill(_duelKnockbackCooldowns.begin(), _duelKnockbackCooldowns.end(), 0);
	_duelHealth[kDuelFirstActor] = 0x3F;
	_duelHealth[kDuelSecondActor] = 0x3F;
	_duelTurnCooldown = 0;
	_duelLastTargetFacing = -1;

	// Ghidra 0x000032CC-0x000032EF: build both status buffers and publish all 0x80 words from each.
	buildDuelStatusBarTiles();
	publishDuelStatusTiles();

	// Ghidra 0x000032F0-0x00003327: select actor zero's duel descriptor and retain both outcomes of the
	// graphics-ready back edge. The original loop has no explicit retrace call but relies on asynchronous
	// room VBlank publication, so service one callback and clocked host frame before each continuation.
	_state.ActorAnimationDescriptorOffsets[kDuelFirstActorSlot] = kDuelFirstActorDescriptorOffset;
	_state.ActorCompactFrameFlags &= 0xFE;
	_state.ActorAnimationOffsets[kDuelFirstActorSlot] = 2;
	_state.ActorAnimationRestartFlags |= 0x01;
	_state.ActorAnimationDelays[kDuelFirstActorSlot] = -1;
	while (true) {
		_updateActorAnimationFrames();
		if ((_state.ActorGraphicsReadyFlags & 0x01) != 0) {
			break;
		}

		if (!waitForRoomVerticalBlank()) {
			return;
		}
	}

	// Ghidra 0x00003328-0x00003371: retain every silent audio handover, the room callback, both nested
	// actions, and the DBF-inclusive 51- and 101-retrace waits in their original order.
	stopAudioPlayback(0x58);
	stopAudioPlayback(0x71);
	playAudioCommand(0x7B);
	if (!waitForRoomVerticalBlank()) {
		return;
	}

	if (!_executeAction02()) {
		// Native Action02 always returns; a host or script handover ends the managed session here.
		return;
	}

	playAudioCommand(0x79);
	if (!waitForRoomFrames(0x32)) {
		return;
	}

	playAudioCommand(0x32);
	if (!waitForRoomFrames(0x64)) {
		return;
	}

	int defeatedActorSlot = 0;
	while (true) {
		// Ghidra 0x00003372-0x000033D7: advance one retrace and all actor animations, preserve first-health
		// defeat priority, and run the complete active-low Start release-and-repress pause handshake.
		if (!waitForRoomVerticalBlank()) {
			return;
		}

		_updateActorAnimationFrames();
		if (_duelHealth[kDuelFirstActor] < 0) {
			defeatedActorSlot = kDuelFirstActorSlot;
			break;
		}

		if (_duelHealth[kDuelSecondActor] < 0) {
			defeatedActorSlot = kDuelSecondActorSlot;
			break;
		}

		if ((_state.ControllerOneInput & 0x80) == 0 && (_state.PreviousControllerOneInput & 0x80) != 0) {
			pauseAudioDriver();
			playAudioCommand(0x59);
			while ((_state.ControllerOneInput & 0x80) == 0) {
				if (!waitForRoomVerticalBlank()) {
					return;
				}
			}

			while ((_state.ControllerOneInput & 0x80) != 0) {
				if (!waitForRoomVerticalBlank()) {
					return;
				}
			}

			playAudioCommand(0x59);
			resumeAudioDriver();
		}

		// Ghidra 0x000033D8-0x0000343D: republish both complete status buffers, then independently
		// decrement each nonzero momentum, knockback, and AI-turn cooldown byte.
		publishDuelStatusTiles();
		if (_duelMomentumCooldowns[kDuelFirstActor] != 0) {
			_duelMomentumCooldowns[kDuelFirstActor]--;
		}

		if (_duelMomentumCooldowns[kDuelSecondActor] != 0) {
			_duelMomentumCooldowns[kDuelSecondActor]--;
		}

		if (_duelKnockbackCooldowns[kDuelFirstActor] != 0) {
			_duelKnockbackCooldowns[kDuelFirstActor]--;
		}

		if (_duelKnockbackCooldowns[kDuelSecondActor] != 0) {
			_duelKnockbackCooldowns[kDuelSecondActor]--;
		}

		if (_duelTurnCooldown != 0) {
			_duelTurnCooldown--;
		}

		// Ghidra 0x0000343E-0x000034AD: advance second and first knockback independently. A zero
		// magnitude clears its complete actor-specific impact-bit group even when authored scripts normally
		// reach this state through a prior impact handler.
		if (_duelKnockback[kDuelSecondActor] != 0) {
			if (_duelKnockbackCooldowns[kDuelSecondActor] == 0) {
				_duelKnockbackCooldowns[kDuelSecondActor] =
					static_cast<uint8>(7 - _duelKnockback[kDuelSecondActor]);
				_duelKnockback[kDuelSecondActor]--;
			}
		} else {
			_state.DuelStateFlags &= 0x67;
		}

		if (_duelKnockback[kDuelFirstActor] != 0) {
			if (_duelKnockbackCooldowns[kDuelFirstActor] == 0) {
				_duelKnockbackCooldowns[kDuelFirstActor] =
					static_cast<uint8>(7 - _duelKnockback[kDuelFirstActor]);
				_duelKnockback[kDuelFirstActor]--;
			}
		} else {
			_state.DuelStateFlags &= 0xB9;
		}

		// Ghidra 0x000034AE-0x00003561: accelerate the second actor whenever its cooldown permits;
		// independently accelerate the first actor for either active-low control bit or decelerate it only
		// when both are released. Every equality and cooldown branch remains represented.
		if (_duelMomentumCooldowns[kDuelSecondActor] == 0 && _duelMomentum[kDuelSecondActor] != 7) {
			_duelMomentum[kDuelSecondActor]++;
			_duelMomentumCooldowns[kDuelSecondActor] =
				static_cast<uint8>(_duelMomentum[kDuelSecondActor] + 8);
		}

		if ((_state.ControllerOneInput & 0x40) == 0 || (_state.ControllerOneInput & 0x10) == 0) {
			if (_duelMomentumCooldowns[kDuelFirstActor] == 0 && _duelMomentum[kDuelFirstActor] != 7) {
				_duelMomentum[kDuelFirstActor]++;
				_duelMomentumCooldowns[kDuelFirstActor] =
					static_cast<uint8>(_duelMomentum[kDuelFirstActor] + 8);
			}
		} else if (_duelMomentum[kDuelFirstActor] != 0 && _duelMomentumCooldowns[kDuelFirstActor] == 0) {
			_duelMomentumCooldowns[kDuelFirstActor] =
				static_cast<uint8>(7 - _duelMomentum[kDuelFirstActor]);
			_duelMomentum[kDuelFirstActor]--;
		}

		// Ghidra 0x00003562-0x0000362D: preserve both gates for second-actor turning, the D1-facing
		// result, the 20-frame throttle, cached-target branch, every signed turn-step outcome, momentum
		// clamp, and actor-two restart side effect.
		if ((_duelKnockback[kDuelSecondActor] == 0 || _duelMomentum[kDuelSecondActor] == 0) &&
			(_state.ActorAnimationHoldFlags & 0x04) != 0) {
			int16 candidateFacing = _state.ActorAnimationOffsets[kDuelSecondActorSlot];
			if ((_state.DuelStateFlags & 0x08) != 0) {
				candidateFacing = static_cast<int16>(candidateFacing + 1);
			}

			if ((_state.DuelStateFlags & 0x10) != 0) {
				candidateFacing = static_cast<int16>(candidateFacing - 1);
			}

			candidateFacing = static_cast<int16>(candidateFacing & 0x07);
			int16 targetFacing = lookupDuelFacingDirection();
			if (targetFacing != candidateFacing && _duelTurnCooldown == 0) {
				_duelTurnCooldown = 0x14;
				if (targetFacing != _duelLastTargetFacing) {
					_duelLastTargetFacing = targetFacing;
					int16 directionDifference =
						static_cast<int16>((targetFacing - candidateFacing) & 0x07);
					if ((directionDifference > 4 && (_state.DuelStateFlags & 0x08) == 0) ||
						(_state.DuelStateFlags & 0x10) != 0) {
						_duelTurnStep = -1;
					} else {
						_duelTurnStep = 1;
					}
				}

				candidateFacing = static_cast<int16>((candidateFacing + _duelTurnStep) & 0x07);
			}

			if (candidateFacing != _state.ActorAnimationOffsets[kDuelSecondActorSlot]) {
				_duelMomentum[kDuelSecondActor]--;
				if (_duelMomentum[kDuelSecondActor] < 0) {
					_duelMomentum[kDuelSecondActor] = 0;
				}

				_state.ActorAnimationOffsets[kDuelSecondActorSlot] = candidateFacing;
				_state.ActorAnimationRestartFlags |= 0x04;
			}
		}

		// Ghidra 0x0000362E-0x000036A9: preserve the independent knockback-or-momentum and hold gates,
		// combine both duel-state and active-low controller turn inputs, clamp momentum after a facing
		// change, and restart actor zero.
		if ((_duelKnockback[kDuelFirstActor] != 0 || _duelMomentum[kDuelFirstActor] != 0) &&
			(_state.ActorAnimationHoldFlags & 0x01) != 0) {
			int16 candidateFacing = _state.ActorAnimationOffsets[kDuelFirstActorSlot];
			if ((_state.DuelStateFlags & 0x02) != 0) {
				candidateFacing = static_cast<int16>(candidateFacing + 1);
			}

			if ((_state.ControllerOneInput & 0x08) == 0) {
				candidateFacing = static_cast<int16>(candidateFacing + 1);
			}

			if ((_state.DuelStateFlags & 0x04) != 0) {
				candidateFacing = static_cast<int16>(candidateFacing - 1);
			}

			if ((_state.ControllerOneInput & 0x04) == 0) {
				candidateFacing = static_cast<int16>(candidateFacing - 1);
			}

			candidateFacing = static_cast<int16>(candidateFacing & 0x07);
			if (candidateFacing != _state.ActorAnimationOffsets[kDuelFirstActorSlot]) {
				_duelMomentum[kDuelFirstActor]--;
				if (_duelMomentum[kDuelFirstActor] < 0) {
					_duelMomentum[kDuelFirstActor] = 0;
				}

				_state.ActorAnimationOffsets[kDuelFirstActorSlot] = candidateFacing;
				_state.ActorAnimationRestartFlags |= 0x01;
			}
		}

		// Ghidra 0x000036AA-0x00003723: clear only the two environment-impact bits, apply second-actor
		// momentum and knockback vectors with signed word wrap, stage the integer result, and preserve both
		// collision-condition outcomes and the second-impact call edge.
		_duelCollisionFlags &= 0xFC;
		DuelMovementDelta secondMomentumDelta = kDuelMovementDeltas[static_cast<std::size_t>(
			_state.ActorAnimationOffsets[kDuelSecondActorSlot] * 8 + _duelMomentum[kDuelSecondActor])];
		DuelMovementDelta secondKnockbackDelta = kDuelMovementDeltas[static_cast<std::size_t>(
			_state.ActorPositionIndices[kDuelSecondActorSlot] * 8 + _duelKnockback[kDuelSecondActor])];
		_stagedCollisionX = static_cast<int16>(
			getIntegerCoordinate(_state.ActorXFixedCoordinates[kDuelSecondActorSlot]) + secondMomentumDelta.X +
			secondKnockbackDelta.X);
		_stagedCollisionY = static_cast<int16>(
			getIntegerCoordinate(_state.ActorYFixedCoordinates[kDuelSecondActorSlot]) + secondMomentumDelta.Y +
			secondKnockbackDelta.Y);
		_state.ActorXFixedCoordinates[kDuelSecondActorSlot] =
			replaceIntegerCoordinate(_state.ActorXFixedCoordinates[kDuelSecondActorSlot], _stagedCollisionX);
		_state.ActorYFixedCoordinates[kDuelSecondActorSlot] =
			replaceIntegerCoordinate(_state.ActorYFixedCoordinates[kDuelSecondActorSlot], _stagedCollisionY);
		if (_collisionProbe.probeStagedCollision(_stagedCollisionX, _stagedCollisionY)) {
			resolveDuelSecondActorImpact();
			_duelCollisionFlags |= 0x02;
		}

		// Ghidra 0x00003724-0x0000378F: independently apply the first actor's two vectors, stage and
		// publish its wrapped integer coordinates, and retain both first-impact outcomes.
		DuelMovementDelta firstMomentumDelta = kDuelMovementDeltas[static_cast<std::size_t>(
			_state.ActorAnimationOffsets[kDuelFirstActorSlot] * 8 + _duelMomentum[kDuelFirstActor])];
		DuelMovementDelta firstKnockbackDelta = kDuelMovementDeltas[static_cast<std::size_t>(
			_state.ActorPositionIndices[kDuelFirstActorSlot] * 8 + _duelKnockback[kDuelFirstActor])];
		_stagedCollisionX = static_cast<int16>(
			getIntegerCoordinate(_state.ActorXFixedCoordinates[kDuelFirstActorSlot]) + firstMomentumDelta.X +
			firstKnockbackDelta.X);
		_stagedCollisionY = static_cast<int16>(
			getIntegerCoordinate(_state.ActorYFixedCoordinates[kDuelFirstActorSlot]) + firstMomentumDelta.Y +
			firstKnockbackDelta.Y);
		_state.ActorXFixedCoordinates[kDuelFirstActorSlot] =
			replaceIntegerCoordinate(_state.ActorXFixedCoordinates[kDuelFirstActorSlot], _stagedCollisionX);
		_state.ActorYFixedCoordinates[kDuelFirstActorSlot] =
			replaceIntegerCoordinate(_state.ActorYFixedCoordinates[kDuelFirstActorSlot], _stagedCollisionY);
		if (_collisionProbe.probeStagedCollision(_stagedCollisionX, _stagedCollisionY)) {
			resolveDuelFirstActorImpact();
			_duelCollisionFlags |= 0x01;
		}

		// Ghidra 0x00003790-0x0000385F: preserve the overlap call and both outcomes. Actor overlap restores
		// both complete integer-position/facing snapshots; otherwise each environment-impact bit restores
		// only its corresponding actor. Every restoration preserves fractional coordinate words.
		if (testDuelActorOverlap()) {
			resolveDuelActorCollision();
			_state.ActorXFixedCoordinates[kDuelFirstActorSlot] = replaceIntegerCoordinate(
				_state.ActorXFixedCoordinates[kDuelFirstActorSlot], _duelPreviousXCoordinates[kDuelFirstActor]);
			_state.ActorYFixedCoordinates[kDuelFirstActorSlot] = replaceIntegerCoordinate(
				_state.ActorYFixedCoordinates[kDuelFirstActorSlot], _duelPreviousYCoordinates[kDuelFirstActor]);
			_state.ActorXFixedCoordinates[kDuelSecondActorSlot] = replaceIntegerCoordinate(
				_state.ActorXFixedCoordinates[kDuelSecondActorSlot],
				_duelPreviousXCoordinates[kDuelSecondActor]);
			_state.ActorYFixedCoordinates[kDuelSecondActorSlot] = replaceIntegerCoordinate(
				_state.ActorYFixedCoordinates[kDuelSecondActorSlot],
				_duelPreviousYCoordinates[kDuelSecondActor]);
			_state.ActorAnimationOffsets[kDuelFirstActorSlot] = _duelPreviousFacings[kDuelFirstActor];
			_state.ActorAnimationOffsets[kDuelSecondActorSlot] = _duelPreviousFacings[kDuelSecondActor];
			_state.ActorAnimationRestartFlags |= 0x05;
			_state.ActorAnimationDelays[kDuelFirstActorSlot] = -1;
			_state.ActorAnimationDelays[kDuelSecondActorSlot] = -1;
		} else {
			if ((_duelCollisionFlags & 0x01) != 0) {
				_state.ActorXFixedCoordinates[kDuelFirstActorSlot] = replaceIntegerCoordinate(
					_state.ActorXFixedCoordinates[kDuelFirstActorSlot],
					_duelPreviousXCoordinates[kDuelFirstActor]);
				_state.ActorYFixedCoordinates[kDuelFirstActorSlot] = replaceIntegerCoordinate(
					_state.ActorYFixedCoordinates[kDuelFirstActorSlot],
					_duelPreviousYCoordinates[kDuelFirstActor]);
				_state.ActorAnimationOffsets[kDuelFirstActorSlot] = _duelPreviousFacings[kDuelFirstActor];
				_state.ActorAnimationDelays[kDuelFirstActorSlot] = -1;
				_state.ActorAnimationRestartFlags |= 0x01;
			}

			if ((_duelCollisionFlags & 0x02) != 0) {
				_state.ActorXFixedCoordinates[kDuelSecondActorSlot] = replaceIntegerCoordinate(
					_state.ActorXFixedCoordinates[kDuelSecondActorSlot],
					_duelPreviousXCoordinates[kDuelSecondActor]);
				_state.ActorYFixedCoordinates[kDuelSecondActorSlot] = replaceIntegerCoordinate(
					_state.ActorYFixedCoordinates[kDuelSecondActorSlot],
					_duelPreviousYCoordinates[kDuelSecondActor]);
				_state.ActorAnimationOffsets[kDuelSecondActorSlot] = _duelPreviousFacings[kDuelSecondActor];
				_state.ActorAnimationDelays[kDuelSecondActorSlot] = -1;
				_state.ActorAnimationRestartFlags |= 0x04;
			}
		}

		// Ghidra 0x00003860-0x000038A3: snapshot both actor facings and signed integer coordinates,
		// rebuild both status buffers, and return through the unconditional main-loop back edge.
		_duelPreviousFacings[kDuelSecondActor] = _state.ActorAnimationOffsets[kDuelSecondActorSlot];
		_duelPreviousFacings[kDuelFirstActor] = _state.ActorAnimationOffsets[kDuelFirstActorSlot];
		_duelPreviousXCoordinates[kDuelFirstActor] =
			getIntegerCoordinate(_state.ActorXFixedCoordinates[kDuelFirstActorSlot]);
		_duelPreviousYCoordinates[kDuelFirstActor] =
			getIntegerCoordinate(_state.ActorYFixedCoordinates[kDuelFirstActorSlot]);
		_duelPreviousXCoordinates[kDuelSecondActor] =
			getIntegerCoordinate(_state.ActorXFixedCoordinates[kDuelSecondActorSlot]);
		_duelPreviousYCoordinates[kDuelSecondActor] =
			getIntegerCoordinate(_state.ActorYFixedCoordinates[kDuelSecondActorSlot]);
		buildDuelStatusBarTiles();
	}

	// Ghidra 0x000038A4-0x000038DB: retain both signed-health terminal branches, their distinct actor
	// animation/restart bits and first-room-object result, and the selected hold-bit index.
	_state.ActorAnimationOffsets[static_cast<std::size_t>(defeatedActorSlot)] = 8;
	_state.ActorAnimationRestartFlags |= static_cast<uint8>(1 << defeatedActorSlot);
	_state.RoomObjects[0].ScriptStateWords[0] =
		defeatedActorSlot == kDuelSecondActorSlot ? static_cast<int16>(1) : static_cast<int16>(0);

	// Ghidra 0x000038DC-0x00003939: update at least once until the selected actor holds, servicing the
	// interrupt-owned room callback and clock before each continuation; then run action 0x03, replace raster
	// restoration with completion of logical duel presentation, preserve audio ordering, restore the saved
	// position index and descriptor mode, and clear only the three authored lifecycle bits.
	while (true) {
		_updateActorAnimationFrames();
		if ((_state.ActorAnimationHoldFlags & (1 << defeatedActorSlot)) != 0) {
			break;
		}

		if (!waitForRoomVerticalBlank()) {
			return;
		}
	}

	_executeAction03();
	stopAudioPlayback(0x7B);
	playAudioCommand(0x71);
	_state.ActorPositionIndices[kDuelFirstActorSlot] = savedFirstActorPositionIndex;
	_state.InteractionFlags &= 0xBF;
	_state.ActorAnimationDescriptorOffsets[kDuelFirstActorSlot] = 0x32700;
	_state.ActorCompactFrameFlags |= 0x01;
	_state.DuelStateFlags &= 0xFE;
	_state.RoomBehaviorFlags &= 0xFB;
}

void DuelRoomScriptActionExecutor::initializeDuelPresentation() {
	Span<uint8> firstTiles = MakeSpan(_firstDuelStatusTiles);

	int destinationOffset = 0;
	WriteUInt32BigEndian(firstTiles, destinationOffset, 0xFF);
	destinationOffset += static_cast<int>(sizeof(uint32));
	for (int index = 0; index < 6; index++) {
		WriteUInt32BigEndian(firstTiles, destinationOffset, 0xF0);
		destinationOffset += static_cast<int>(sizeof(uint32));
	}

	WriteUInt32BigEndian(firstTiles, destinationOffset, 0xFF);
	destinationOffset += static_cast<int>(sizeof(uint32));
	WriteUInt32BigEndian(firstTiles, destinationOffset, 0xFFFFFFFFu);
	destinationOffset += static_cast<int>(sizeof(uint32));
	for (int index = 0; index < 6; index++) {
		WriteUInt32BigEndian(firstTiles, destinationOffset, 0);
		destinationOffset += static_cast<int>(sizeof(uint32));
	}

	WriteUInt32BigEndian(firstTiles, destinationOffset, 0xFFFFFFFFu);
	_scene.loadTilesAt(firstTiles.slice(0, 0x40), 0x7F8);

	destinationOffset = 0;
	for (int group = 0; group < 2; group++) {
		WriteUInt16BigEndian(firstTiles, destinationOffset, 0x07F8);
		destinationOffset += static_cast<int>(sizeof(uint16));
		for (int index = 0; index < 8; index++) {
			WriteUInt16BigEndian(firstTiles, destinationOffset, 0x07F9);
			destinationOffset += static_cast<int>(sizeof(uint16));
		}

		WriteUInt16BigEndian(firstTiles, destinationOffset, 0x0FF8);
		destinationOffset += static_cast<int>(sizeof(uint16));
		WriteUInt16BigEndian(firstTiles, destinationOffset, _state.BlankTileCell);
		destinationOffset += static_cast<int>(sizeof(uint16));
		WriteUInt16BigEndian(firstTiles, destinationOffset, _state.BlankTileCell);
		destinationOffset += static_cast<int>(sizeof(uint16));
	}

	int columnBank = (_state.DisplayFlags & 0x10) != 0 ? 32 : 0;
	_scene.loadLayerRows(firstTiles.slice(0, 0x30), TileLayer::Foreground, columnBank + 5, 22, 24, 1);
	_scene.loadTilesAt(_rom.readBytes(kDuelPatternTilesOffset, 0x380), kDuelPatternTileIndex);
	for (int row = 0; row < 5; row++) {
		writeDuelFrameCells(kDuelFrameTileOffsetsOffset + row * 0x10, columnBank + 1, row + 22);
		writeDuelFrameCells(kDuelFrameTileOffsetsOffset + row * 0x10 + 8, columnBank + 28, row + 22);
	}
}

void DuelRoomScriptActionExecutor::writeDuelFrameCells(int sourceOffset, int startColumn, int row) {
	Common::Array<TileCell> cells(4);
	for (std::size_t index = 0; index < cells.size(); ++index) {
		uint16 tileOffset =
			_rom.readUInt16(sourceOffset + static_cast<int>(index) * static_cast<int>(sizeof(uint16)));
		cells[index] =
			decodeTileCell(
				static_cast<uint16>(kDuelPatternTileIndex + tileOffset));
	}

	_scene.replaceLayerRow(MakeSpan(cells), TileLayer::Foreground, startColumn, row);
}

void DuelRoomScriptActionExecutor::publishDuelStatusTiles() {
	_scene.loadTilesAt(MakeSpan(_firstDuelStatusTiles), 0x518);
	_scene.loadTilesAt(MakeSpan(_secondDuelStatusTiles), 0x520);
}

bool DuelRoomScriptActionExecutor::waitForRoomFrames(int frameCounter) {
	for (int remaining = frameCounter; remaining >= 0; remaining--) {
		if (!waitForRoomVerticalBlank()) {
			return false;
		}
	}

	return true;
}

bool DuelRoomScriptActionExecutor::waitForRoomVerticalBlank() {
	_handleRoomVBlank();
	if (_presenter.waitForVerticalBlank()) {
		return true;
	}

	_requestSessionExit(SessionExit::HostClosed);
	return false;
}

int16 DuelRoomScriptActionExecutor::getIntegerCoordinate(int coordinate) {
	return static_cast<int16>(coordinate >> 16);
}

int16 DuelRoomScriptActionExecutor::halveWordLogically(int16 value) {
	return static_cast<int16>(static_cast<uint16>(value) >> 1);
}

int32 DuelRoomScriptActionExecutor::replaceIntegerCoordinate(
	int32 coordinate, int16 integer) {
	return (coordinate & 0x0000FFFF) | (static_cast<int32>(integer) << 16);
}

void DuelRoomScriptActionExecutor::pauseAudioDriver() {
	// AUDIO FRONTIER: Preserve pause ordering until the PC audio domain is implemented.
}

void DuelRoomScriptActionExecutor::resumeAudioDriver() {
	// AUDIO FRONTIER: Preserve resume ordering until the PC audio domain is implemented.
}

void DuelRoomScriptActionExecutor::playAudioCommand(int command) {
	// AUDIO FRONTIER: Preserve command ordering until the PC audio domain is implemented.
	(void)command;
}

void DuelRoomScriptActionExecutor::stopAudioPlayback(int command) {
	// AUDIO FRONTIER: Preserve stop ordering until the PC audio domain is implemented.
	(void)command;
}

void DuelRoomScriptActionExecutor::buildDuelStatusBarTiles() {
	// Ghidra 0x0000393A-0x00003985: replace the full first shared workspace with palette-A health tiles.
	buildDuelHealthTiles(MakeSpan(_firstDuelStatusTiles), _duelHealth[kDuelFirstActor], 0xA0000000u);

	// Ghidra 0x00003986-0x000039D3: replace the full second shared workspace with palette-9 health tiles.
	buildDuelHealthTiles(MakeSpan(_secondDuelStatusTiles), _duelHealth[kDuelSecondActor], 0x90000000u);
}

void DuelRoomScriptActionExecutor::buildDuelHealthTiles(Span<uint8> tiles, int16 health,
														uint32 firstPixel) {
	for (int tileIndex = 0; tileIndex < 8; tileIndex++) {
		uint32 packedRow = 0;
		uint32 pixel = firstPixel;
		for (int column = 0; column < 8; column++) {
			if (health < 0) {
				continue;
			}

			packedRow |= pixel;
			pixel >>= 4;
			health = static_cast<int16>(health - 1);
		}

		Span<uint8> tile = tiles.slice(static_cast<std::size_t>(tileIndex * 0x20), 0x20);
		tile.fill(0);
		for (int row = 2; row < 6; row++) {
			WriteUInt32BigEndian(tile, row * static_cast<int>(sizeof(uint32)), packedRow);
		}
	}
}

int16 DuelRoomScriptActionExecutor::lookupDuelFacingDirection() {
	// Ghidra 0x00003A68-0x00003A6B: preserve the original relative-direction call edge.
	int16 relativeDirection = computeDuelRelativeDirection();

	// Ghidra 0x00003A6C-0x00003A79: double the word index and return the opposite-facing table entry.
	return kOppositeDuelFacingDirections[static_cast<std::size_t>(relativeDirection)];
}

int16 DuelRoomScriptActionExecutor::computeDuelRelativeDirection() {
	// Ghidra 0x00003A7A-0x00003AA3: derive both wrapped signed deltas, magnitudes, and sign quadrant.
	int16 sector = 0;
	int16 horizontalMagnitude = static_cast<int16>(
		getIntegerCoordinate(_state.ActorXFixedCoordinates[kDuelSecondActorSlot]) -
		getIntegerCoordinate(_state.ActorXFixedCoordinates[kDuelFirstActorSlot]));
	if (horizontalMagnitude < 0) {
		horizontalMagnitude = static_cast<int16>(-horizontalMagnitude);
		sector = 2;
	}

	int16 verticalMagnitude = static_cast<int16>(
		getIntegerCoordinate(_state.ActorYFixedCoordinates[kDuelSecondActorSlot]) -
		getIntegerCoordinate(_state.ActorYFixedCoordinates[kDuelFirstActorSlot]));
	if (verticalMagnitude < 0) {
		verticalMagnitude = static_cast<int16>(-verticalMagnitude);
		sector = static_cast<int16>(sector + 1);
	}

	// Ghidra 0x00003AA4-0x00003AC1: normalize the quadrant and apply the first signed magnitude threshold.
	if ((sector & 0x02) == 0) {
		sector = static_cast<int16>(sector ^ 0x01);
	}

	sector = static_cast<int16>(sector + sector);
	int16 horizontalThresholdMagnitude = horizontalMagnitude;
	int16 verticalThresholdMagnitude = verticalMagnitude;
	if ((sector & 0x02) != 0) {
		SWAP(horizontalMagnitude, verticalMagnitude);
	}

	if (horizontalMagnitude >= verticalMagnitude) {
		sector = static_cast<int16>(sector + 1);
	}

	// Ghidra 0x00003AC2-0x00003AE3: select and double one original magnitude, orient it, and apply the
	// second threshold.
	int16 thresholdSelector = sector;
	sector = static_cast<int16>(sector + sector);
	thresholdSelector = static_cast<int16>((thresholdSelector + 1) & 0x02);
	if (thresholdSelector == 0) {
		horizontalThresholdMagnitude =
			static_cast<int16>(horizontalThresholdMagnitude + horizontalThresholdMagnitude);
	} else {
		verticalThresholdMagnitude =
			static_cast<int16>(verticalThresholdMagnitude + verticalThresholdMagnitude);
	}

	if ((sector & 0x04) != 0) {
		SWAP(horizontalThresholdMagnitude, verticalThresholdMagnitude);
	}

	if (horizontalThresholdMagnitude >= verticalThresholdMagnitude) {
		sector = static_cast<int16>(sector + 1);
	}

	// Ghidra 0x00003AE4-0x00003AEF: wrap the sector accumulator and return its logical half in D0w.
	sector = static_cast<int16>((sector + 1) & 0x0F);
	return static_cast<int16>(static_cast<uint16>(sector) >> 1);
}

void DuelRoomScriptActionExecutor::resolveDuelActorCollision() {
	// Ghidra 0x00003AF0-0x00003B13: arm both impact groups, issue collision sound 0x72, and resolve
	// the relative and opposite directions used by both damage calculations.
	_state.DuelStateFlags |= 0xC0;
	playAudioCommand(0x72);
	int16 relativeDirection = computeDuelRelativeDirection();
	int16 oppositeDirection = kOppositeDuelFacingDirections[static_cast<std::size_t>(relativeDirection)];

	// Ghidra 0x00003B14-0x00003B41: weight first-actor momentum against both facings and subtract the
	// resulting word from second-actor health with 16-bit wrapping.
	int16 secondActorDamage = _duelMomentum[kDuelFirstActor];
	if (relativeDirection != _state.ActorAnimationOffsets[kDuelFirstActorSlot]) {
		secondActorDamage = halveWordLogically(secondActorDamage);
	}

	if (oppositeDirection == _state.ActorAnimationOffsets[kDuelSecondActorSlot]) {
		secondActorDamage = 0;
	} else if (relativeDirection == _state.ActorAnimationOffsets[kDuelSecondActorSlot]) {
		secondActorDamage = halveWordLogically(secondActorDamage);
	}

	_duelHealth[kDuelSecondActor] = static_cast<int16>(_duelHealth[kDuelSecondActor] -
													   secondActorDamage);

	// Ghidra 0x00003B42-0x00003B89: mirror the facing weights for second-actor momentum, subtract first
	// health, and distinguish unequal impacts with authored audio commands 0x25 and 0x28.
	int16 firstActorDamage = _duelMomentum[kDuelSecondActor];
	if (oppositeDirection != _state.ActorAnimationOffsets[kDuelSecondActorSlot]) {
		firstActorDamage = halveWordLogically(firstActorDamage);
	}

	if (relativeDirection == _state.ActorAnimationOffsets[kDuelFirstActorSlot]) {
		firstActorDamage = 0;
	} else if (oppositeDirection == _state.ActorAnimationOffsets[kDuelFirstActorSlot]) {
		firstActorDamage = halveWordLogically(firstActorDamage);
	}

	_duelHealth[kDuelFirstActor] = static_cast<int16>(_duelHealth[kDuelFirstActor] - firstActorDamage);
	if (secondActorDamage != firstActorDamage) {
		playAudioCommand(secondActorDamage < firstActorDamage ? 0x28 : 0x25);
	}

	// Ghidra 0x00003B8A-0x00003BE9: transfer first momentum into second knockback with a signed minimum
	// of four, clear second momentum, arm both second cooldowns, advance random state, and select its flag.
	_duelKnockback[kDuelSecondActor] = _duelMomentum[kDuelFirstActor];
	if (_duelKnockback[kDuelSecondActor] < 4) {
		_duelKnockback[kDuelSecondActor] = 4;
	}

	_duelMomentum[kDuelSecondActor] = 0;
	_duelMomentumCooldowns[kDuelSecondActor] = 8;
	_duelKnockbackCooldowns[kDuelSecondActor] = 8;
	_state.DuelStateFlags |= _random.advanceDuelImpactFlagChoice() ? 0x10 : 0x08;

	// Ghidra 0x00003BEA-0x00003C61: publish the relative second-actor direction, convert it to the
	// first direction, then preserve the cleared-momentum copy, clamp, cooldown, and random-flag order.
	_state.ActorPositionIndices[kDuelSecondActorSlot] = relativeDirection;
	int16 firstActorDirection = kOppositeDuelFacingDirections[static_cast<std::size_t>(
		relativeDirection)];
	_duelKnockback[kDuelFirstActor] = _duelMomentum[kDuelSecondActor];
	if (_duelKnockback[kDuelFirstActor] < 4) {
		_duelKnockback[kDuelFirstActor] = 4;
	}

	_duelMomentum[kDuelFirstActor] = 0;
	_duelMomentumCooldowns[kDuelFirstActor] = 8;
	_duelKnockbackCooldowns[kDuelFirstActor] = 8;
	_state.DuelStateFlags |= _random.advanceDuelImpactFlagChoice() ? 0x04 : 0x02;
	_state.ActorPositionIndices[kDuelFirstActorSlot] = firstActorDirection;

	// Ghidra 0x00003C62-0x00003C6F: the final opposite-table read restores only D0 to the relative
	// direction. The sole caller does not consume that register clobber, so it has no managed state.
}

void DuelRoomScriptActionExecutor::resolveDuelFirstActorImpact() {
	// Ghidra 0x00003C70-0x00003C79: issue the authored first-boundary impact sound.
	playAudioCommand(0x60);

	// Ghidra 0x00003C7A-0x00003C97: consume an armed first-impact group by charging its prior knockback.
	if ((_state.DuelStateFlags & 0x40) != 0) {
		_duelHealth[kDuelFirstActor] = static_cast<int16>(_duelHealth[kDuelFirstActor] - _duelKnockback[kDuelFirstActor]);
		_state.DuelStateFlags &= 0xBF;
	}

	// Ghidra 0x00003C98-0x00003CB9: transfer current momentum without a clamp, then arm both cooldowns.
	_duelKnockback[kDuelFirstActor] = _duelMomentum[kDuelFirstActor];
	_duelMomentum[kDuelFirstActor] = 0;
	_duelMomentumCooldowns[kDuelFirstActor] = 8;
	_duelKnockbackCooldowns[kDuelFirstActor] = 8;

	// Ghidra 0x00003CBA-0x00003CE5: advance shared random state and select one first-impact direction bit.
	_state.DuelStateFlags |= _random.advanceDuelImpactFlagChoice() ? 0x04 : 0x02;

	// Ghidra 0x00003CE6-0x00003CFD: publish the opposite of the first actor's current authored facing.
	_state.ActorPositionIndices[kDuelFirstActorSlot] =
		kOppositeDuelFacingDirections[static_cast<std::size_t>(_state.ActorAnimationOffsets[kDuelFirstActorSlot])];
}

void DuelRoomScriptActionExecutor::resolveDuelSecondActorImpact() {
	// Ghidra 0x00003CFE-0x00003D07: issue the authored second-boundary impact sound.
	playAudioCommand(0x60);

	// Ghidra 0x00003D08-0x00003D25: consume an armed second-impact group by charging its prior knockback.
	if ((_state.DuelStateFlags & 0x80) != 0) {
		_duelHealth[kDuelSecondActor] = static_cast<int16>(_duelHealth[kDuelSecondActor] - _duelKnockback
																							   [kDuelSecondActor]);
		_state.DuelStateFlags &= 0x7F;
	}

	// Ghidra 0x00003D26-0x00003D47: transfer current momentum without a clamp, then arm both cooldowns.
	_duelKnockback[kDuelSecondActor] = _duelMomentum[kDuelSecondActor];
	_duelMomentum[kDuelSecondActor] = 0;
	_duelMomentumCooldowns[kDuelSecondActor] = 8;
	_duelKnockbackCooldowns[kDuelSecondActor] = 8;

	// Ghidra 0x00003D48-0x00003D73: advance shared random state and select one second-impact direction bit.
	_state.DuelStateFlags |= _random.advanceDuelImpactFlagChoice() ? 0x10 : 0x08;

	// Ghidra 0x00003D74-0x00003D85: publish the opposite of the second actor's current authored facing.
	_state.ActorPositionIndices[kDuelSecondActorSlot] =
		kOppositeDuelFacingDirections[static_cast<std::size_t>(_state.ActorAnimationOffsets[kDuelSecondActorSlot])];
}

bool DuelRoomScriptActionExecutor::testDuelActorOverlap() {
	// Ghidra 0x000039D4-0x00003A0B: resolve actor zero's frame and derive its four signed boundaries.
	DuelActorBounds firstBounds = readDuelActorBounds(kDuelFirstActorSlot, kDuelFirstActorDescriptorOffset);

	// Ghidra 0x00003A0C-0x00003A43: resolve actor two's distinct frame and signed boundaries.
	DuelActorBounds secondBounds = readDuelActorBounds(kDuelSecondActorSlot, kDuelSecondActorDescriptorOffset);

	// Ghidra 0x00003A44-0x00003A67: require inclusive signed overlap on both axes and return zero or one.
	return intervalsOverlapInclusively(firstBounds.Left, firstBounds.Right, secondBounds.Left,
									   secondBounds.Right) &&
		   intervalsOverlapInclusively(firstBounds.Top, firstBounds.Bottom, secondBounds.Top, secondBounds.Bottom);
}

DuelRoomScriptActionExecutor::DuelActorBounds DuelRoomScriptActionExecutor::readDuelActorBounds(int actorSlot,
																								int descriptorOffset) {
	int16 frameTableEntryOffset = static_cast<int16>(
		_rom.readUInt16(descriptorOffset + kDuelAnimationFrameTableOffsetField) +
		_state.ActorAnimationOffsets[static_cast<std::size_t>(actorSlot)] * static_cast<int>(sizeof(
																				int16)));
	int16 frameOffset = _rom.readInt16(descriptorOffset + frameTableEntryOffset);
	int boundsOffset = descriptorOffset + frameOffset + kDuelCollisionBoundsFrameOffset;
	int16 actorX = getIntegerCoordinate(
		_state.ActorXFixedCoordinates[static_cast<std::size_t>(actorSlot)]);
	int16 actorY = getIntegerCoordinate(
		_state.ActorYFixedCoordinates[static_cast<std::size_t>(actorSlot)]);

	DuelActorBounds bounds;
	bounds.Left = static_cast<int16>(actorX - _rom.readInt16(boundsOffset));
	bounds.Top = static_cast<int16>(actorY - _rom.readInt16(
												 boundsOffset + static_cast<int>(sizeof(int16))));
	bounds.Right =
		static_cast<int16>(actorX - _rom.readInt16(
										boundsOffset + 2 * static_cast<int>(sizeof(int16))));
	bounds.Bottom =
		static_cast<int16>(actorY - _rom.readInt16(
										boundsOffset + 3 * static_cast<int>(sizeof(int16))));
	return bounds;
}

bool DuelRoomScriptActionExecutor::intervalsOverlapInclusively(int16 firstStart, int16 firstEnd,
															   int16 secondStart, int16 secondEnd) {
	return firstStart <= secondStart ? firstEnd >= secondStart : secondEnd >= firstStart;
}
} // namespace Scooby