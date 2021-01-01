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

#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/world/actors/cru_avatar_mover_process.h"
#include "ultima/ultima8/world/crosshair_process.h"
#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/item_factory.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/ultima8.h"

#include "common/math.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(CrosshairProcess)

static const uint32 CROSSHAIR_SHAPE = 0x4CC;
static const float CROSSHAIR_DIST = 400.0;

CrosshairProcess *CrosshairProcess::_instance = nullptr;

CrosshairProcess::CrosshairProcess() : Process() {
	_instance = this;
}

void CrosshairProcess::run() {
	MainActor *mainactor = getMainActor();
	assert(mainactor);
	if (mainactor->isInCombat()) {
		Kernel *kernel = Kernel::get_instance();
		assert(kernel);
		int32 ax, ay, az;
		mainactor->getLocation(ax, ay, az);
		mainactor->addFireAnimOffsets(ax, ay, az);

		const CruAvatarMoverProcess *mover = dynamic_cast<CruAvatarMoverProcess *>(Ultima8Engine::get_instance()->getAvatarMoverProcess());
		if (!mover) {
			warning("lost CruAvatarMoverProcess!");
			return;
		}
		double angle = mover->getAvatarAngleDegrees() + 90.0;
		// Convert angle to 0~2pi
		double rads = Common::deg2rad(angle);
		float xoff = CROSSHAIR_DIST * cos(rads);
		float yoff = CROSSHAIR_DIST * sin(rads);
		ax -= static_cast<int32>(xoff);
		ay -= static_cast<int32>(yoff);

		Item *item;
		if (_itemNum) {
			item = getItem(_itemNum);
		} else {
			// Create a new sprite
			item = ItemFactory::createItem(CROSSHAIR_SHAPE, 0, 0, Item::FLG_DISPOSABLE,
										   0, 0, Item::EXT_SPRITE, true);
			setItemNum(item->getObjId());
		}
		assert(item);
		item->move(ax, ay, az);
	} else {
		if (_itemNum) {
			Item *item = getItem(_itemNum);
			assert(item);
			item->destroy();
			_itemNum = 0;
		}
	}
}

void CrosshairProcess::saveData(Common::WriteStream *ws) {
	Process::saveData(ws);
}

bool CrosshairProcess::loadData(Common::ReadStream *rs, uint32 version) {
	return Process::loadData(rs, version);
}

} // End of namespace Ultima8
} // End of namespace Ultima
