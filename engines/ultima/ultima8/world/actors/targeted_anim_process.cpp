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

#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(TargetedAnimProcess, ActorAnimProcess)

TargetedAnimProcess::TargetedAnimProcess() : ActorAnimProcess() {

}

TargetedAnimProcess::TargetedAnimProcess(Actor *actor_, Animation::Sequence action_, uint32 dir_, int32 coords[3]) : ActorAnimProcess(actor_, action_, dir_) {
	_x = coords[0];
	_y = coords[1];
	_z = coords[2];
}

bool TargetedAnimProcess::init() {
	if (!ActorAnimProcess::init())
		return false;

	_tracker->setTargetedMode(_x, _y, _z);
	return true;
}


void TargetedAnimProcess::saveData(ODataSource *ods) {
	ActorAnimProcess::saveData(ods);

	ods->write4(static_cast<uint32>(_x));
	ods->write4(static_cast<uint32>(_y));
	ods->write4(static_cast<uint32>(_z));

}

bool TargetedAnimProcess::loadData(IDataSource *ids, uint32 version) {
	if (!ActorAnimProcess::loadData(ids, version)) return false;

	_x = ids->read4();
	_y = ids->read4();
	_z = ids->read4();

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
