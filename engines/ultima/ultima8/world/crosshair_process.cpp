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

#include "ultima/ultima8/misc/debugger.h"

#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/world/actors/cru_avatar_mover_process.h"
#include "ultima/ultima8/world/crosshair_process.h"
#include "ultima/ultima8/world/item_factory.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/ultima8.h"

#include "math/utils.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(CrosshairProcess)

static const uint32 CROSSHAIR_SHAPE = 0x4CC;
static const float CROSSHAIR_DIST = 400.0;

CrosshairProcess *CrosshairProcess::_instance = nullptr;

CrosshairProcess::CrosshairProcess() : Process() {
	_instance = this;
	_type = 1; // persistent
}

CrosshairProcess::~CrosshairProcess() {
	if (_instance == this)
		_instance = nullptr;
}

void CrosshairProcess::run() {
	Actor *actor = getControlledActor();
	if (!actor)
		return;

	if (actor->isInCombat()) {
		Kernel *kernel = Kernel::get_instance();
		assert(kernel);
		Point3 pt = actor->getLocation();
		actor->addFireAnimOffsets(pt.x, pt.y, pt.z);

		const CruAvatarMoverProcess *mover = dynamic_cast<CruAvatarMoverProcess *>(Ultima8Engine::get_instance()->getAvatarMoverProcess());
		if (!mover) {
			warning("lost CruAvatarMoverProcess");
			return;
		}
		double angle = mover->getAvatarAngleDegrees() + 90.0;
		if (angle < 90.0) {
			// -1 is used to record the avatar is not in combat, so shouldn't happen?
			return;
		}
		// Convert angle to 0~2pi
		double rads = Math::deg2rad(angle);
		float xoff = CROSSHAIR_DIST * cos(rads);
		float yoff = CROSSHAIR_DIST * sin(rads);
		pt.x -= static_cast<int32>(xoff);
		pt.y -= static_cast<int32>(yoff);

		Item *item;
		if (_itemNum) {
			item = getItem(_itemNum);
		} else {
			// Create a new sprite
			item = ItemFactory::createItem(CROSSHAIR_SHAPE, 0, 0, Item::FLG_DISPOSABLE,
										   0, 0, Item::EXT_SPRITE, true);
			setItemNum(item->getObjId());
		}
		if (item)
			item->move(pt.x, pt.y, pt.z);
		else
			_itemNum = 0; // sprite gone? can happen during teleport.
	} else {
		if (_itemNum) {
			Item *item = getItem(_itemNum);
			if (item)
				item->destroy();
			_itemNum = 0;
		}
	}
}

void CrosshairProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);
}

bool CrosshairProcess::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Process::loadData(rs, version)) return false;
	_type = 1; // should be persistent but older savegames may not know that.
	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
