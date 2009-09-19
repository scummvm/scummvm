/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "worldstats.h"

namespace Asylum {

WorldStats::WorldStats() {
	// TODO Auto-generated constructor stub

}

WorldStats::~WorldStats() {
	barriers.clear();
	actors.clear();
	actions.clear();
}

int WorldStats::getActionAreaIndexById(uint32 id) {
	for (uint32 i = 0; i < numActions; i++) {
		if (actions[i].id == id)
			return i;
	}

	return -1;
}

ActionArea* WorldStats::getActionAreaById(uint32 id) {
	return &actions[getActionAreaIndexById(id)];
}

int WorldStats::getBarrierIndexById(uint32 id) {
	for (uint32 i = 0; i < numBarriers; i++) {
		if (barriers[i].id == id)
			return i;
	}

	return -1;
}

Barrier* WorldStats::getBarrierById(uint32 id) {
	return &barriers[getBarrierIndexById(id)];
}

Barrier* WorldStats::getBarrierByIndex(uint32 idx) {
	return &barriers[idx];
}

} // end of namespace Asylum
