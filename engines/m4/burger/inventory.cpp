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
#include "m4/burger/inventory.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {

struct InvObject {
	const char *_name;
	const char *_verbs;
	int32 _scene, _cel, _cursor;
};

static const InvObject INVENTORY_ITEMS[] = {
	{ "money", "GELD", 998, 55, 55 },
	{ "block of ice", "EISBLOCK", 142, 51, 51 },
	{ "pantyhose", "STRUMPFHOSE", 142, 45, 45 },
	{ "phone bill", "TELEFONRECHNUNG", 170, 49, 49 },
	{ "whistle", "PFEIFE", 170, 40, 40 },
	{ "carrot juice", "M\xd6""HRENSAFT", 172, 25, 25 },
	{ "deed", "URKUNDE", 105, 38, 38 },
	{ "laxative", "ABF\xdcHRMITTEL", 174, 54, 54 },
	{ "amplifier", "VERST\xc4""RKER", 145, 34, 34 },
	{ "jawz o' life", "KLEMMBACKE", 137, 37, 37 },
	{ "broken puz dispenser", "Bonbonspender,kaputt", 176, 16, 16 },
	{ "spring", "SPRUNGFEDER", 999, 47, 47 },
	{ "puz dispenser", "BONBONSPENDER", 999, 17, 17 },
	{ "broken mouse trap", "KAPUTTE MAUSEFALLE", 143, 18, 18 },
	{ "keys", "SCHL\xdc""SSEL", 138, 31, 31 },
	{ "ray gun", "STRAHLENPISTOLE", 604, 50, 50 },
	{ "kibble", "TROCKENFUTTER", 602, 44, 44 },
	{ "burger morsel", "HAMBURGERST\xdc""CK", 999, 39, 39 },
	{ "matches", "STREICHH\xd6""LZER", 999, 42, 42 },
	{ "jug", "KRUG", 303, 14, 14 },
	{ "distilled carrot juice", "DESTILLIERTER SAFT", 999, 15, 15 },
	{ "gizmo", "GER\xc4""T", 999, 58, 58 },
	{ "kindling", "ANZ\xdc""NDHOLZ", 999, 20, 20 },
	{ "burning kindling", "BRENNENDES HOLZ", 999, 21, 21 },
	{ "christmas lights", "LICHTERKETTE", 508, 22, 22 },
	{ "christmas lights ", "LICHTERKETTE", 508, 23, 23 },
	{ "bottle", "FLASCHE", 999, 24, 24 },
	{ "soapy water", "SEIFENWASSER", 999, 26, 26 },
	{ "rubber gloves", "GUMMIHANDSCHUHE", 503, 35, 35 },
	{ "dirty sock", "DRECKIGE SOCKE", 504, 36, 36 },
	{ "rubber ducky", "GUMMIENTCHEN", 507, 53, 53 },
	{ "rolling pin", "NUDELHOLZ", 999, 52, 52 },
	{ "fish", "FISCH", 999, 29, 29 },
	{ "hook", "HAKEN", 999, 30, 30 },
	{ "quarter", "VIERTELDOLLAR", 999, 41, 41 },
	{ "dog collar", "HUNDEHALSBAND", 999, 33, 33 },
	{ "records", "SCHALLPLATTEN", 405, 32, 32 },
	{ "mirror", "SPIEGEL", 999, 48, 48 },
	{ nullptr, nullptr, 0, 0, 0 }
};

void Inventory::init() {
	for (const InvObject *item = INVENTORY_ITEMS; item->_name; ++item) {
		inv_register_thing(item->_name, item->_verbs, item->_scene, item->_cel, item->_cursor);

		_items.push_back(InventoryItem(item->_name, item->_scene));
	}
}

void Inventory::add(const Common::String &name, const Common::String &verb, int32 sprite, int32 cursor) {
	_GI(inventory)->add(name, verb, sprite, cursor);
	_GI(inventory)->_must_redraw_all = true;
}

void Inventory::set_scroll(int32 scroll) {
	_GI(inventory)->set_scroll(scroll);
}

void Inventory::remove(const Common::String &name) {
	_GI(inventory)->remove(name);
	_GI(inventory)->_must_redraw_all = true;
}

void Inventory::reset() {
	for (const InvObject *item = INVENTORY_ITEMS; item->_name; ++item)
		inv_move_object(item->_name, item->_scene);
}

} // namespace Burger
} // namespace M4
