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


#include "ultima/ultima8/world/actors/targeted_anim_process.h"
#include "ultima/ultima8/world/actors/animation_tracker.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(TargetedAnimProcess)

TargetedAnimProcess::TargetedAnimProcess() : ActorAnimProcess(),
		_pt() {
}

TargetedAnimProcess::TargetedAnimProcess(Actor *actor, Animation::Sequence action, Direction dir, const Point3 &pt) :
	ActorAnimProcess(actor, action, dir),
	_pt(pt) {
}

bool TargetedAnimProcess::init() {
	if (!ActorAnimProcess::init())
		return false;

	_tracker->setTargetedMode(_pt);
	return true;
}


void TargetedAnimProcess::saveData(Common::WriteStream *ws) {
	ActorAnimProcess::saveData(ws);

	ws->writeUint32LE(static_cast<uint32>(_pt.x));
	ws->writeUint32LE(static_cast<uint32>(_pt.y));
	ws->writeUint32LE(static_cast<uint32>(_pt.z));
}

bool TargetedAnimProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!ActorAnimProcess::loadData(rs, version)) return false;

	_pt.x = rs->readUint32LE();
	_pt.y = rs->readUint32LE();
	_pt.z = rs->readUint32LE();

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
