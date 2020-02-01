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

#include "ultima/ultima8/world/actors/clear_feign_death_process.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/world/get_object.h"

#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"

namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(ClearFeignDeathProcess, Process)

ClearFeignDeathProcess::ClearFeignDeathProcess() : Process() {

}

ClearFeignDeathProcess::ClearFeignDeathProcess(Actor *actor_) {
	assert(actor_);
	item_num = actor_->getObjId();

	type = 0x243; // constant !
}

void ClearFeignDeathProcess::run() {
	Actor *a = getActor(item_num);

	if (!a) {
		// actor gone?
		terminate();
		return;
	}

	a->clearActorFlag(Actor::ACT_FEIGNDEATH);

	AudioProcess *audioproc = AudioProcess::get_instance();
	if (audioproc) audioproc->playSFX(59, 0x60, item_num, 0);

	// done
	terminate();
}

void ClearFeignDeathProcess::saveData(ODataSource *ods) {
	Process::saveData(ods);
}

bool ClearFeignDeathProcess::loadData(IDataSource *ids, uint32 version) {
	if (!Process::loadData(ids, version)) return false;

	return true;
}

} // End of namespace Ultima8
