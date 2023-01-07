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

#include "ultima/ultima8/world/actors/avatar_gravity_process.h"
#include "ultima/ultima8/world/actors/avatar_mover_process.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/kernel/mouse.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(AvatarGravityProcess)

AvatarGravityProcess::AvatarGravityProcess()
	: GravityProcess() {

}

AvatarGravityProcess::AvatarGravityProcess(MainActor *avatar, int gravity)
	: GravityProcess(avatar, gravity) {

}

void AvatarGravityProcess::run() {
	AvatarMoverProcess *amp = Ultima8Engine::get_instance()->getAvatarMoverProcess();
	if (amp && amp->hasMovementFlags(AvatarMoverProcess::MOVE_ANY_DIRECTION)) {
		// See if we can cling to a ledge
		MainActor *avatar = getMainActor();
		if (avatar->tryAnim(Animation::climb40, dir_current) == Animation::SUCCESS) {

			// We can climb, so perform a hang animation
			// CHECKME: do we need to perform any other checks?
			if (avatar->getLastAnim() != Animation::hang)
				avatar->doAnim(Animation::hang, dir_current);

			return;
		}
	}

	// fall normally
	GravityProcess::run();
	return;
}

void AvatarGravityProcess::saveData(Common::WriteStream *ws) {
	GravityProcess::saveData(ws);
}

bool AvatarGravityProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!GravityProcess::loadData(rs, version)) return false;

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
