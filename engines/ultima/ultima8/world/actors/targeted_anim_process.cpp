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

#include "ultima/ultima8/world/actors/targeted_anim_process.h"
#include "ultima/ultima8/world/actors/animation_tracker.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(TargetedAnimProcess)

TargetedAnimProcess::TargetedAnimProcess() : ActorAnimProcess(),
		_x(0), _y(0), _z(0) {
}

TargetedAnimProcess::TargetedAnimProcess(Actor *actor, Animation::Sequence action, Direction dir, int32 coords[3]) :
	ActorAnimProcess(actor, action, dir),
	_x(coords[0]), _y(coords[1]), _z(coords[2]) {
}

bool TargetedAnimProcess::init() {
	if (!ActorAnimProcess::init())
		return false;

	_tracker->setTargetedMode(_x, _y, _z);
	return true;
}


void TargetedAnimProcess::saveData(Common::WriteStream *ws) {
	ActorAnimProcess::saveData(ws);

	ws->writeUint32LE(static_cast<uint32>(_x));
	ws->writeUint32LE(static_cast<uint32>(_y));
	ws->writeUint32LE(static_cast<uint32>(_z));

}

bool TargetedAnimProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!ActorAnimProcess::loadData(rs, version)) return false;

	_x = rs->readUint32LE();
	_y = rs->readUint32LE();
	_z = rs->readUint32LE();

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
