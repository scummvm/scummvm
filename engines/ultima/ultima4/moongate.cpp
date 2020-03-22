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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ultima/ultima4/error.h"
#include "ultima/ultima4/coords.h"
#include "ultima/ultima4/moongate.h"
#include "ultima/ultima4/types.h"
#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Ultima4 {

typedef Std::map<int, Coords> MoongateList; /* map moon phase to map coordinates */

MoongateList gates;

void moongateAdd(int phase, const Coords &coords) {
	if (gates.contains(phase))
		errorFatal("Error: A moongate for phase %d already exists", phase);

	gates[phase] = coords;
}

const Coords *moongateGetGateCoordsForPhase(int phase) {
	MoongateList::iterator moongate;

	moongate = gates.find(phase);
	if (moongate != gates.end())
		return &moongate->_value;
	return NULL;
}

bool moongateFindActiveGateAt(int trammel, int felucca, const Coords &src, Coords &dest) {
	const Coords *moongate_coords;

	moongate_coords = moongateGetGateCoordsForPhase(trammel);
	if (moongate_coords && (src == *moongate_coords)) {
		moongate_coords = moongateGetGateCoordsForPhase(felucca);
		if (moongate_coords) {
			dest = *moongate_coords;
			return true;
		}
	}
	return false;
}

bool moongateIsEntryToShrineOfSpirituality(int trammel, int felucca) {
	return (trammel == 4 && felucca == 4) ? true : false;
}

} // End of namespace Ultima4
} // End of namespace Ultima
