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

#include "ultima/ultima4/core/coords.h"
#include "ultima/ultima4/game/moongate.h"
#include "ultima/ultima4/core/types.h"
#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Ultima4 {

Moongates *g_moongates;

Moongates::Moongates() {
	g_moongates = this;
}

Moongates::~Moongates() {
	g_moongates = nullptr;
}

void Moongates::add(int phase, const Coords &coords) {
	if (contains(phase))
		error("Error: A moongate for phase %d already exists", phase);

	(*this)[phase] = coords;
}

const Coords *Moongates::getGateCoordsForPhase(int phase) {
	iterator moongate;

	moongate = find(phase);
	if (moongate != end())
		return &moongate->_value;

	return nullptr;
}

bool Moongates::findActiveGateAt(int trammel, int felucca, const Coords &src, Coords &dest) {
	const Coords *moongate_coords;

	moongate_coords = getGateCoordsForPhase(trammel);
	if (moongate_coords && (src == *moongate_coords)) {
		moongate_coords = getGateCoordsForPhase(felucca);
		if (moongate_coords) {
			dest = *moongate_coords;
			return true;
		}
	}

	return false;
}

bool Moongates::isEntryToShrineOfSpirituality(int trammel, int felucca) {
	return (trammel == 4 && felucca == 4) ? true : false;
}

} // End of namespace Ultima4
} // End of namespace Ultima
