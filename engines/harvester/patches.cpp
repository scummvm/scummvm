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

#include "harvester/patches.h"

#include "common/debug.h"
#include "harvester/detection.h"
#include "harvester/script/script.h"

namespace Harvester {

void applyObjectPatches(ObjectRecord &object) {	
	// The original game has a known issue with the depth checks in the Pottsdam's bathroom
	// which allows the player to walk through them at certain depths.
	if (!object.initialOwnerOrRoom.equalsIgnoreCase("ST_BATHRM") || !object.objectName.empty() ||
			object.initialY != 0 || object.boundsY2 != 479 || object.initialZ != -8 || object.zExtent != 10)
		return;

	if ((object.initialX == 0 && object.boundsX2 == 195) ||
			(object.initialX == 449 && object.boundsX2 == 639)) {
		// The original HARVEST.SCR gives these walls Z -8 and extent 10, ending at
		// depth 2. The room allows depth 4, so Steve can walk through either wall
		// at depths 3 and 4. Extend the walls to depth 4 to close that gap.
		object.zExtent = 12;
		debugC(2, kDebugRoom, "Harvester: patched bathroom wall x=%d..%d depth=-8..4",
			object.initialX, object.boundsX2);
	}
}

} // End of namespace Harvester
