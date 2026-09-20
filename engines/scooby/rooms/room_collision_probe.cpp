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

#include "room_collision_probe.h"

#include "common/array.h"

#include "scooby/common/contracts.h"

namespace Scooby {
namespace {
inline uint16 ReadBigEndian16(const Common::Array<uint8> &bytes, int offset) {
	std::size_t o = static_cast<std::size_t>(offset);
	return static_cast<uint16>((uint16(bytes[o]) << 8) | uint16(bytes[o + 1]));
}
} // namespace

RoomCollisionProbeResult RoomCollisionProbe::probeLeadActorCollision(int candidateX, int candidateY,
																	 int16 maximumX,
																	 int16 maximumY) const {
	// Retain the candidate X word and reject each signed bound independently.
	int16 x = static_cast<int16>(candidateX >> 16);
	int16 y = static_cast<int16>(candidateY >> 16);
	if (x < 0) {
		return RoomCollisionProbeResult(false, x);
	}
	if (y < 0) {
		return RoomCollisionProbeResult(false, x);
	}
	if (x > maximumX) {
		return RoomCollisionProbeResult(false, x);
	}
	if (y > maximumY) {
		return RoomCollisionProbeResult(false, x);
	}

	// Preserve unsigned tile division, word-sized row arithmetic, bit-15 removal, and the distinct empty-cell return.
	uint16 rowByteStride = static_cast<uint16>(_state.RoomWidthTiles * sizeof(uint16));
	int16 collisionByteOffset = static_cast<int16>(
		(static_cast<uint16>(x) >> 3) * sizeof(uint16) +
		(static_cast<uint16>(y) >> 3) * rowByteStride);
	SDM_ASSERT(_state.ActiveRoomScene, "ProbeLeadActorCollision requires an active room scene.");
	uint16 collisionCell = ReadBigEndian16(_state.ActiveRoomScene->CollisionCells, collisionByteOffset);
	uint16 descriptorIndex = collisionCell & kCollisionCellDescriptorIndexMask;
	if (descriptorIndex == 0) {
		return RoomCollisionProbeResult(true, 0);
	}

	// Use signed low-word offsets exactly as the descriptor and 32-byte shape address calculations do.
	int16 descriptorByteOffset = static_cast<int16>(descriptorIndex * sizeof(uint16));
	uint16 descriptor = _rom.readUInt16(kCollisionCellShapeDescriptorsOffset + descriptorByteOffset);
	int16 shapeByteOffset =
		static_cast<int16>((descriptor & kCollisionDescriptorShapeIndexMask) * kCollisionShapeByteCount);

	// Retain both independent orientation branches and test the selected four-bit pixel in the row mask.
	int pixelX = x & 0x07;
	int pixelY = y & 0x07;
	if ((descriptor & kCollisionDescriptorHorizontalOrientationMask) == 0) {
		pixelX ^= 0x07;
	}
	if ((descriptor & kCollisionDescriptorVerticalOrientationMask) != 0) {
		pixelY ^= 0x07;
	}

	int16 retainedDataRegister = static_cast<int16>(pixelX << 2);
	uint32 rowMask = _rom.readUInt32(kCollisionShapeRowMasksOffset + shapeByteOffset +
									 pixelY * static_cast<int>(sizeof(uint32)));
	uint32 selectedPixelMask = kCollisionShapePixelMask << retainedDataRegister;
	return RoomCollisionProbeResult((rowMask & selectedPixelMask) == 0, retainedDataRegister);
}

bool RoomCollisionProbe::probeStagedCollision(int16 x, int16 y) const {
	// Each negative staged coordinate independently returns blocked.
	if (x < 0 || y < 0) {
		return true;
	}

	// Preserve unsigned tile division, wrapped word row arithmetic, direct low-eleven-bit cell selection,
	// and the distinct empty-cell return.
	uint16 rowByteStride = static_cast<uint16>(_state.RoomWidthTiles * sizeof(uint16));
	int16 collisionByteOffset = static_cast<int16>(
		(static_cast<uint16>(x) >> 3) * sizeof(uint16) +
		(static_cast<uint16>(y) >> 3) * rowByteStride);
	SDM_ASSERT(_state.ActiveRoomScene, "ProbeStagedCollision requires an active room scene.");
	uint16 collisionCell = ReadBigEndian16(_state.ActiveRoomScene->CollisionCells, collisionByteOffset);
	uint16 descriptorIndex = collisionCell & kCollisionDescriptorShapeIndexMask;
	if (descriptorIndex == 0) {
		return false;
	}

	// Resolve the packed descriptor and retain the signed low-word mask displacement.
	uint16 descriptor = _rom.readUInt16(kCollisionCellShapeDescriptorsOffset +
										descriptorIndex * static_cast<int>(sizeof(uint16)));
	int16 shapeByteOffset =
		static_cast<int16>((descriptor & kCollisionDescriptorShapeIndexMask) * kCollisionShapeByteCount);

	// Apply both independent orientation bits and return whether the selected four-bit pixel is occupied.
	int pixelX = x & 0x07;
	int pixelY = y & 0x07;
	if ((descriptor & kCollisionDescriptorHorizontalOrientationMask) == 0) {
		pixelX ^= 0x07;
	}
	if ((descriptor & kCollisionDescriptorVerticalOrientationMask) != 0) {
		pixelY ^= 0x07;
	}

	uint32 rowMask = _rom.readUInt32(kCollisionShapeRowMasksOffset + shapeByteOffset +
									 pixelY * static_cast<int>(sizeof(uint32)));
	uint32 selectedPixelMask = kCollisionShapePixelMask << (pixelX << 2);
	return (rowMask & selectedPixelMask) != 0;
}
} // namespace Scooby