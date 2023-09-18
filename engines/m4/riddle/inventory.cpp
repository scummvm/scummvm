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

#include "m4/adv_r/adv_inv.h"
#include "m4/riddle/inventory.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {

struct InvObject {
	const char *_name;
	const char *_verbs;
	int32 _scene, _cel, _cursor;
};

static const InvObject INVENTORY_ITEMS[] = {
//	{ "mirror", "SPIEGEL", 999, 48, 48 },
	{ nullptr, nullptr, 0, 0, 0 }
};

void Inventory::init() {
	for (const InvObject *item = INVENTORY_ITEMS; item->_name; ++item) {
		inv_register_thing(item->_name, item->_verbs, item->_scene, item->_cel, item->_cursor);

		_items.push_back(InventoryItem(item->_name, item->_scene));
	}
}

void Inventory::add(const Common::String &name, const Common::String &verb, int32 sprite, int32 cursor) {
#ifdef TODO
	_GI(inventory)->add(name, verb, sprite, cursor);
	_GI(inventory)->_must_redraw_all = true;
#endif
}

void Inventory::set_scroll(int32 scroll) {
#ifdef TODO
	_GI(inventory)->set_scroll(scroll);
#endif
}

void Inventory::remove(const Common::String &name) {
#ifdef TODO
	_GI(inventory)->remove(name);
	_GI(inventory)->_must_redraw_all = true;
#endif
}

void Inventory::reset() {
	for (const InvObject *item = INVENTORY_ITEMS; item->_name; ++item)
		inv_move_object(item->_name, item->_scene);
}

} // namespace Riddle
} // namespace M4
