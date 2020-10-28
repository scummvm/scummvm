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
#include "ultima/ultima8/misc/direction.h"
#include "ultima/ultima8/world/actors/avatar_gravity_process.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(AvatarGravityProcess)

AvatarGravityProcess::AvatarGravityProcess()
	: GravityProcess() {

}

AvatarGravityProcess::AvatarGravityProcess(MainActor *avatar, int gravity)
	: GravityProcess(avatar, gravity) {

}

void AvatarGravityProcess::run() {
	if (!Mouse::get_instance()->isMouseDownEvent(Shared::BUTTON_RIGHT)) {
		// right mouse button not down, so fall normally

		GravityProcess::run();
		return;
	}

	// right mouse button down, so see if we can cling to a ledge
	MainActor *avatar = getMainActor();
	Direction direction = avatar->getDir();
	if (avatar->tryAnim(Animation::climb40, direction) == Animation::SUCCESS) {

		// we can, so perform a hang animation
		// CHECKME: do we need to perform any other checks?

		if (avatar->getLastAnim() != Animation::hang)
			avatar->doAnim(Animation::hang, dir_current);

		return;
	} else {

		// fall normally
		GravityProcess::run();
		return;
	}
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
