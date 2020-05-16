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

#include "ultima/ultima8/misc/pent_include.h"

#include "ultima/ultima8/world/teleport_egg.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(TeleportEgg)

TeleportEgg::TeleportEgg() {
}


TeleportEgg::~TeleportEgg() {
}

uint16 TeleportEgg::hatch() {
	if (!isTeleporter()) return 0; // teleport target

	// teleport to destination egg
	perr << "Teleport!!!!!!!!" << Std::endl;

	MainActor *av = getMainActor();
	av->teleport(_mapNum, getTeleportId());

	return 0;
}

void TeleportEgg::saveData(Common::WriteStream *ws) {
	Egg::saveData(ws);
}

bool TeleportEgg::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Egg::loadData(rs, version)) return false;

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
