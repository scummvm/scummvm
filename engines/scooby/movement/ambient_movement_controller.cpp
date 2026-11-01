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

#include "ambient_movement_controller.h"

#include "common/array.h"

namespace Scooby {
namespace {
// Original embedded animation words at 0x00000E8C-0x00000E93.
const Common::Array<int16> kAnimationByPackedIndex = {0x000C, 0x0051, 0x0014, 0x001E};
} // namespace

bool AmbientMovementController::tryStartRandomMove() {
	if ((_state.interactionFlags & 0x80) != 0 || (_state.actorMovementFlags & kScoobyActorMask) != 0 ||
		(_state.videoFlags & 0x04) != 0 || _state.randomMoveTimer >= 0 ||
		(_state.actorAnimationHoldFlags & kScoobyActorMask) == 0) {
		return true;
	}

	_state.randomMoveTimer = 100;
	if (_state.randomMoveCount == 0) {
		return true;
	}

	uint16 selectedMove = _random.scaleNextRandomValue(
		static_cast<uint16>(_state.randomMoveCount));
	int moveAddress = _state.roomMovementDataOffset + selectedMove * 4;
	uint16 packedMove = _rom.readUInt16(moveAddress);
	uint16 rotatedMove = static_cast<uint16>((packedMove << 4) | (packedMove >> 12));
	_state.randomMoveDirection = static_cast<int16>(rotatedMove & 0x03);
	_state.randomMoveAnimation = kAnimationByPackedIndex[static_cast<size_t>((rotatedMove & 0x0C) >> 2)];
	int16 targetX = static_cast<int16>((packedMove & 0x0FFF) << 3);
	int16 targetY = static_cast<int16>(_rom.readInt16(moveAddress + 2) << 3);
	return _movement.moveScoobyToRoomPosition(targetX, targetY, 1, 0).completed;
}
} // namespace Scooby
