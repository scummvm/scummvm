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

#ifndef SCOOBY_ROOMS_SCENE_DATA_H
#define SCOOBY_ROOMS_SCENE_DATA_H

#include "common/scummsys.h"

#include "common/array.h"

#include "scooby/common/span.h"

// Owns the decoded cell and collision resources retained for the active room
// between loading, viewport drawing, tile streaming, patching, and collision probes.

namespace Scooby {

class RoomSceneData {
public:
	// Creates independent working maps and pristine snapshots from one completed room decode.
	RoomSceneData(Span<const uint8> newBackgroundCells,
				  Span<const uint8> newForegroundCells,
				  Span<const uint8> newCollisionCells)
		: backgroundCells(newBackgroundCells.toArray()), backgroundCellSnapshot(newBackgroundCells.toArray()),
		  foregroundCells(newForegroundCells.toArray()), foregroundCellSnapshot(newForegroundCells.toArray()),
		  collisionCells(newCollisionCells.toArray()) {
	}

	// Ghidra g_awRoomBackgroundCells at 0xFF7000. Positive room-object tile
	// patches replace rectangular cell ranges in place.
	Common::Array<uint8> backgroundCells;

	// Ghidra g_awRoomBackgroundCellSnapshot at 0xFFA800. Negative room-object
	// tile patches restore rectangular ranges from this unchanged copy.
	Common::Array<uint8> backgroundCellSnapshot;

	// Ghidra g_awMenuRowShiftQueueOrRoomForegroundCells at 0xFF8C00 (room lifetime).
	Common::Array<uint8> foregroundCells;

	// Ghidra g_abNarrowRevealOrRoomForegroundSnapshotWorkspace at 0xFFC400 (room lifetime).
	Common::Array<uint8> foregroundCellSnapshot;

	// Ghidra g_abWideRevealOrRoomCollisionWorkspace at 0xFFE000 (room lifetime).
	Common::Array<uint8> collisionCells;
};
} // namespace Scooby

#endif // SCOOBY_ROOMS_SCENE_DATA_H
