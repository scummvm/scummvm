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
	if ((_state.InteractionFlags & 0x80) != 0 || (_state.ActorMovementFlags & 0x02) != 0 ||
		(_state.VideoFlags & 0x04) != 0 || _state.RandomMoveTimer >= 0 ||
		(_state.ActorAnimationHoldFlags & 0x02) == 0) {
		return true;
	}

	_state.RandomMoveTimer = 100;
	if (_state.RandomMoveCount == 0) {
		return true;
	}

	uint16 selectedMove = _random.scaleNextRandomValue(
		static_cast<uint16>(_state.RandomMoveCount));
	int moveAddress = _state.RoomMovementDataOffset + selectedMove * 4;
	uint16 packedMove = _rom.readUInt16(moveAddress);
	uint16 rotatedMove = static_cast<uint16>((packedMove << 4) | (packedMove >> 12));
	_state.RandomMoveDirection = static_cast<int16>(rotatedMove & 0x03);
	_state.RandomMoveAnimation = kAnimationByPackedIndex[static_cast<std::size_t>((rotatedMove & 0x0C) >> 2)];
	int16 targetX = static_cast<int16>((packedMove & 0x0FFF) << 3);
	int16 targetY = static_cast<int16>(_rom.readInt16(moveAddress + 2) << 3);
	return _movement.movePlayerToRoomPosition(targetX, targetY, 1, 0).Completed;
}
} // namespace Scooby