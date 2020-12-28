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
#include "ultima/ultima8/gumps/cru_pickup_area_gump.h"
#include "ultima/ultima8/gumps/cru_pickup_gump.h"

#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/world/get_object.h"
#include "ultima/ultima8/world/item.h"

namespace Ultima {
namespace Ultima8 {

static const int PICKUP_GUMP_GAP = 5;
static const int PICKUP_GUMP_HEIGHT = 30;

CruPickupAreaGump *CruPickupAreaGump::_instance;

DEFINE_RUNTIME_CLASSTYPE_CODE(CruPickupAreaGump)

CruPickupAreaGump::CruPickupAreaGump() : Gump(PICKUP_GUMP_GAP, PICKUP_GUMP_GAP, 200, 500, 0) {
    _instance = this;
}

void CruPickupAreaGump::addPickup(const Item *item) {
	if (!item)
		return;

	uint32 shapeno = item->getShape();

	Std::list<Gump *>::iterator it;

	for (it = _children.begin(); it != _children.end(); it++) {
		CruPickupGump *pug = dynamic_cast<CruPickupGump *>(*it);
		if (!pug)
			return;
		if (pug->getShapeNo() == shapeno) {
			// Already a notification for this object, update it
			pug->updateForNewItem(item);
			break;
		}
	}
	if (it == _children.end()) {
		int32 yoff = PICKUP_GUMP_GAP;
		if (_children.size() > 0)
			yoff += PICKUP_GUMP_HEIGHT;

		Gump *newgump = new CruPickupGump(item, yoff);
		newgump->InitGump(this, false);
	}
}

CruPickupAreaGump *CruPickupAreaGump::get_instance() {
	return _instance;
}

} // End of namespace Ultima8
} // End of namespace Ultima
