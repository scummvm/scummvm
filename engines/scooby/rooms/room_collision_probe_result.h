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

#ifndef SCOOBY_ROOM_COLLISION_PROBE_RESULT_H
#define SCOOBY_ROOM_COLLISION_PROBE_RESULT_H

#include "common/scummsys.h"

// Preserves both observable outputs of the native lead-actor collision probe.

namespace Scooby {

struct RoomCollisionProbeResult {
	bool IsClear; // Whether the candidate point did not select an occupied collision nibble.
	int16 RetainedDataRegister;
	// Original D0 word consumed when direction input selects no explicit result.

	RoomCollisionProbeResult() : IsClear(false), RetainedDataRegister(0) {
	}

	RoomCollisionProbeResult(bool isClear, int16 retainedDataRegister)
		: IsClear(isClear), RetainedDataRegister(retainedDataRegister) {
	}
};
} // namespace Scooby

#endif // SCOOBY_ROOM_COLLISION_PROBE_RESULT_H
