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

#include "ultima/ultima8/world/actors/teleport_to_egg_process.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/world/get_object.h"

#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"

namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(TeleportToEggProcess, Process)

TeleportToEggProcess::TeleportToEggProcess() : Process() {

}


TeleportToEggProcess::TeleportToEggProcess(int mapnum_, int teleport_id_)
	: mapnum(mapnum_), teleport_id(teleport_id_) {
	type = 1; // CONSTANT! (type 1 = persistent)
}


void TeleportToEggProcess::run() {
	MainActor *av = getMainActor();

	av->teleport(mapnum, teleport_id);

	terminate();
}

void TeleportToEggProcess::saveData(ODataSource *ods) {
	Process::saveData(ods);

	ods->write4(static_cast<uint32>(mapnum));
	ods->write4(static_cast<uint32>(teleport_id));
}

bool TeleportToEggProcess::loadData(IDataSource *ids, uint32 version) {
	if (!Process::loadData(ids, version)) return false;

	mapnum = static_cast<int>(ids->read4());
	teleport_id = static_cast<int>(ids->read4());
	return true;
}

} // End of namespace Ultima8
