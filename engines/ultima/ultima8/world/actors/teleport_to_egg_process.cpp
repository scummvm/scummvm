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

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(TeleportToEggProcess)

TeleportToEggProcess::TeleportToEggProcess() : Process(),
	_mapNum(0), _teleportId(0), _arrivalAnim(0) {
}


TeleportToEggProcess::TeleportToEggProcess(int mapNum, int teleportId, int arrivalAnim)
	: _mapNum(mapNum), _teleportId(teleportId), _arrivalAnim(0) {
	_type = 1; // CONSTANT! (type 1 = persistent)
}


void TeleportToEggProcess::run() {
	MainActor *av = getMainActor();

	av->teleport(_mapNum, _teleportId);

	if (_arrivalAnim)
		av->doAnim(static_cast<Animation::Sequence>(_arrivalAnim), av->getDir());

	terminate();
}

void TeleportToEggProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);

	ws->writeUint32LE(static_cast<uint32>(_mapNum));
	ws->writeUint32LE(static_cast<uint32>(_teleportId));
}

bool TeleportToEggProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;

	_mapNum = static_cast<int>(rs->readUint32LE());
	_teleportId = static_cast<int>(rs->readUint32LE());
	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
