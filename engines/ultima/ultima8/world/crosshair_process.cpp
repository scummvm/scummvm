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
#include "ultima/ultima8/world/crosshair_process.h"
#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/world/world.h"
#include "ultima/ultima8/world/item_factory.h"
#include "ultima/ultima8/world/get_object.h"

namespace Ultima {
namespace Ultima8 {

// p_dynamic_cast stuff
DEFINE_RUNTIME_CLASSTYPE_CODE(CrosshairProcess)

const uint32 CROSSHAIR_SHAPE = 0x4CC;
const float CROSSHAIR_DIST = 400.0;

CrosshairProcess::CrosshairProcess() : Process() {
}

void CrosshairProcess::run() {
	Kernel *kernel = Kernel::get_instance();
	assert(kernel);
	MainActor *mainactor = getMainActor();
	int32 cx, cy, cz, ax, ay, az;
	mainactor->getCentre(cx, cy, cz);
	mainactor->getFootpadWorld(ax, ay, az);
	// TODO: Make a fine adjustment for avatar height (eg, when crouching)
	// for now just put it at 3/4 avatar height which is about right.
	cz += az / 4;
	// TODO: Get the fine angle of the avatar once that is implemented.
	uint16 dir = (mainactor->getDir() + 2) % 8;
	// Dir is 0~7, convert to 0~15/8*pi
	float angle = (3.14 * dir / 4.0);
	float xoff = CROSSHAIR_DIST * cos(angle);
	float yoff = CROSSHAIR_DIST * sin(angle);
	cx -= static_cast<int32>(xoff);
	cy -= static_cast<int32>(yoff);

	if (mainactor->isInCombat()) {
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
		item->move(cx, cy, cz);
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
