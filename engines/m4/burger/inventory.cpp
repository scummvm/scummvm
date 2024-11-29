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
#include "m4/burger/burger.h"

namespace M4 {
namespace Burger {

struct InvObject {
	const char *_name;
	const char *_germanName;
	const char *_frenchName;
	int32 _scene, _cel, _cursor;
};

static const InvObject INVENTORY_ITEMS[] = {
	{ "money", "GELD", "ARGENT", 998, 55, 55},
	{ "block of ice", "EISBLOCK", "GLA\xc7ON", 142, 51, 51},
	{ "pantyhose", "STRUMPFHOSE", "COLLAN", 142, 45, 45},
	{ "phone bill", "TELEFONRECHNUNG", "FACTURE DE TELEPHONE", 170, 49, 49},
	{ "whistle", "PFEIFE", "SIFFLET", 170, 40, 40},
	{ "carrot juice", "M\xd6""HRENSAFT", "JUS DE CAROTTES", 172, 25, 25},
	{ "deed", "URKUNDE", "ACTE DE VENTE", 105, 38, 38},
	{ "laxative", "ABF\xdcHRMITTEL", "LAXATIF", 174, 54, 54},
	{ "amplifier", "VERST\xc4""RKER", "AMPLIFICATEUR", 145, 34, 34},
	{ "jawz o' life", "KLEMMBACKE", "LA TRON\xc7ONNEUSE", 137, 37, 37},
	{ "broken puz dispenser", "Bonbonspender,kaputt", "BOITE A PEZ CASSEE", 176, 16, 16},
	{ "spring", "SPRUNGFEDER", "RESSORT", 999, 47, 47},
	{ "puz dispenser", "BONBONSPENDER", "BOITE A PEZ", 999, 17, 17},
	{ "broken mouse trap", "KAPUTTE MAUSEFALLE", "PIEGE A SOURIS CASSE", 143, 18, 18},
	{ "keys", "SCHL\xdc""SSEL", "CLEFS", 138, 31, 31},
	{ "ray gun", "STRAHLENPISTOLE", "PISTOLET A LASER", 604, 50, 50},
	{ "kibble", "TROCKENFUTTER", "CROQUETTES", 602, 44, 44},
	{ "burger morsel", "HAMBURGERST\xdc""CK", "MOR\xc7""EAU DE BURGER", 999, 39, 39},
	{ "matches", "STREICHH\xd6""LZER", "ALLUMETTES", 999, 42, 42},
	{ "jug", "KRUG", "PICHET", 303, 14, 14},
	{ "distilled carrot juice", "DESTILLIERTER SAFT", "CAROTTES DISTILLE", 999, 15, 15},
	{ "gizmo", "GER\xc4""T", "GIZMO", 999, 58, 58},
	{ "kindling", "ANZ\xdc""NDHOLZ", "PETIT BOIS", 999, 20, 20},
	{ "burning kindling", "BRENNENDES HOLZ", "PETIT BOIS ENFLAMME", 999, 21, 21},
	{ "christmas lights", "LICHTERKETTE", "LUMIERES DE NO\xebL", 508, 22, 22},
	{ "christmas lights ", "LICHTERKETTE", "LUMIERES DE NO\xebL", 508, 23, 23 },
	{ "bottle", "FLASCHE", "BOUTEILLE", 999, 24, 24},
	{ "soapy water", "SEIFENWASSER", "EAU DE BAIN", 999, 26, 26},
	{ "rubber gloves", "GUMMIHANDSCHUHE", "GANTS DE CAOUTCHOUC", 503, 35, 35},
	{ "dirty sock", "DRECKIGE SOCKE", "CHAUSSETTE SALE", 504, 36, 36},
	{ "rubber ducky", "GUMMIENTCHEN", "CANARD DE BAIN", 507, 53, 53},
	{ "rolling pin", "NUDELHOLZ", "ROULEAU A PATISSERIE", 999, 52, 52},
	{ "fish", "FISCH", "POISSON", 999, 29, 29},
	{ "hook", "HAKEN", "CROCHET", 999, 30, 30},
	{ "quarter", "VIERTELDOLLAR", "PIECE", 999, 41, 41},
	{ "dog collar", "HUNDEHALSBAND", "COLLIER POUR CHIEN", 999, 33, 33},
	{ "records", "SCHALLPLATTEN", "DISQUES", 405, 32, 32},
	{ "mirror", "SPIEGEL", "MIROIR", 999, 48, 48},
	{ nullptr, nullptr, nullptr, 0, 0, 0 }
};

void Inventory::init() {
	bool isFrench = g_engine->getLanguage() == Common::FR_FRA;

	for (const InvObject *item = INVENTORY_ITEMS; item->_name; ++item) {
		const char *foreignName = isFrench ? item->_frenchName : item->_germanName;
		inv_register_thing(item->_name, foreignName, item->_scene, item->_cel, item->_cursor);

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
