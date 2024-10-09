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

#define INVENTORY_COUNT 123

struct InvObject {
	const char *_name;
	int16 _scene, _cel, _cursor;
};

static const InvObject INVENTORY_ITEMS[] = {
	{ "SOLDIER'S HELMET",       203,  14,  10 },
	{ "BUCKET",					203,  15,  11 },
	{ "RIPLEY PHOTO",			998,  16,  12 },
	{ "LEAD PIPE",				207,  17,  13 },
	{ "SEVEN SPOKES",			207,  18,  14 },
	{ "MALLET",					204,  55,  21 },
	{ "GONG",					204,  56,  22 },
	{ "WHEELED TOY",			504,  57,  53 },
	{ "CHARCOAL",				205,  58,  54 },
	{ "POST MARKED ENVELOPE",	304,  25,  34 },
	{ "TURTLE",					999,  59,  55 },
	{ "TURTLE TREATS",			305,  60,  56 },
	{ "PLANK",					408,  26,  23 },
	{ "PASS",					204,  61,  57 },
	{ "KEY",					409,  27,  35 },
	{ "ACE OF SPADES CARD",		406,  28,  24 },
	{ "STEP LADDER",			403,  62,  58 },
	{ "BILLIARD BALL",			406,  29,  25 },
	{ "CIGAR",					456,  30,  26 },
	{ "GLASS JAR",				407,  31,  27 },
	{ "ROMANOV EMERALD",		407,  32,  28 },
	{ "CORK",					407,  33,  29 },
	{ "LEVER KEY",				407,  34,  30 },
	{ "EDGER",					408,  88,  84 },
	{ "SURGICAL TUBE",			407,  45,  43 },
	{ "PUMP ROD",				407,  35,  31 },
	{ "FAUCET PIPE",			407,  46,  44 },
	{ "FAUCET HANDLE",			407,  36,  32 },
	{ "RUBBER PLUG",			407,  37,  33 },
	{ "GARDEN HOSE",			407,  38,  36 },
	{ "MENENDEZ'S LETTER",		407,  39,  37 },
	{ "KEYS",					406,  40,  38 },
	{ "BROKEN WHEEL",			207,  63,  59 },
	{ "PUMP GRIPS",				407,  41,  39 },
	{ "CLOCK FACING",			501,  47,  45 },
	{ "SHOVEL",					504,  64,  60 },
	{ "WOODEN LADDER",			504,  65,  61 },
	{ "ROPE",					504,  66,  62 },
	{ "GREEN VINE",				504,  67,  63 },
	{ "CRYSTAL SKULL",			510,  42,  40 },
	{ "BROWN VINE",				504,  68,  64 },
	{ "PULL CORD",				604,  69,  65 },
	{ "SPARK PLUG TOOL",		604,  70,  66 },
	{ "RONGORONGO TABLET",		607,  71,  67 },
	{ "SLEEVE",					603,  72,  68 },
	{ "LIGHTER",				608,  73,  69 },
	{ "STICK AND SHELL MAP",	608,  74,  70 },
	{ "DRIFTWOOD STUMP",		600,  75,  71 },
	{ "OBSIDIAN DISK",			605,  76,  72 },
	{ "SOLDIER'S SHIELD",		805,  77,  73 },
	{ "TWELVETREES' NOTE",		603,  89,  85 },
	{ "TWO SOLDIERS' SHIELDS",	805,  78,  74 },
	{ "WOODEN POST",			805,  79,  75 },
	{ "CRANK",					806,  80,  76 },
	{ "WOODEN BEAM",			844,  81,  77 },
	{ "EMPEROR'S SEAL",			810,  82,  78 },
	{ "POLE",					603,  90,  86 },
	{ "REBUS AMULET",			207,  19,  15 },
	{ "DRIFTWOOD PUFFIN",		608,  20,  16 },
	{ "SHRUNKEN HEAD",			504,  43,  41 },
	{ "METAL RIM",				207,  23,  19 },
	{ "CHINESE YUAN",			999,  22,  18 },
	{ "JOURNAL",				998,  24,  20 },
	{ "SILVER BUTTERFLY",		204,  44,  42 },
	{ "POSTAGE STAMP",			999,  83,  79 },
	{ "GERMAN BANKNOTE",		405,  84,  80 },
	{ "WHALE BONE HORN",		608,  85,  81 },
	{ "CHISEL",					709,  86,  82 },
	{ "INCENSE BURNER",			709,  87,  83 },
	{ "POMERANIAN MARKS",		401,  22,  18 },
	{ "PERUVIAN INTI",			501,  22,  18 },
	{ "SIKKIMESE RUPEE",		701,  22,  18 },
	{ "SERENITY WHEEL",			703,  48,  46 },
	{ "PEACE WHEEL",			704,  49,  47 },
	{ "INSIGHT WHEEL",			705,  50,  48 },
	{ "TRUTH WHEEL",			705,  51,  49 },
	{ "WISDOM WHEEL",			706,  52,  50 },
	{ "CIGAR WITHOUT BAND",		406,  53,  51 },
	{ "CIGAR BAND",				406,  54,  52 },
	{ "FARMER'S SHOVEL",		801,  91,  87 },
	{ "MATCH",					801,  92,  88 },
	{ "LIT LIGHTER",			999,  96,  92 },
	{ "VON SELTSAM'S NOTE",		999,  97,  93 },
	{ "PRAYER WHEEL BROCHURE",	999,  98,  94 },
	{ "ENVELOPE",				406,  99,  95 },
	{ "VON SELTSAM'S LETTER",	999, 100,  96 },
	{ "HORN/PULL CORD",			999, 101,  97 },
	{ "FAUCET PIPE/HOSE",		999, 102,  98 },
	{ "FAUCET PIPE/HOSE/TUBE",	999, 103,  99 },
	{ "JAR/CORK",				999, 104, 100 },
	{ "JAR/RUBBER PLUG",		999, 105, 101 },
	{ "JAR/GRIPS",				999, 106, 102 },
	{ "JAR/PLUG/GRIPS",			999, 107, 103 },
	{ "JAR/PLUG/CORK/GRIPS",	999, 108, 104 },
	{ "JAR/CORK/PLUG",			999, 109, 105 },
	{ "JAR/CORK/GRIPS",			999, 110, 106 },
	{ "VINES",					999, 111, 107 },
	{ "TUBE/HOSE",				407, 112, 108 },
	{ "US DOLLARS",				998,  21,  17 },
	{ "JAR/FAUCET PIPE",		407, 113, 109 },
	{ "EMERALD/CORK",			407, 114, 110 },
	{ "TWELVETREES' MAP",		603,  95,  91 },
	{ "PERIODIC TABLE",			407,  93,  89 },
	{ "LEVER KEY/PUMP ROD",		407, 115, 111 },
	{ "JAR/LEVER KEY",			407, 116, 112 },
	{ "PUMP ROD/GRIPS",			407, 117, 113 },
	{ "JAR/PERIODIC TABLE",		407, 118, 114 },
	{ "ROPE/GREEN VINE",		999, 119, 115 },
	{ "ROPE/VINES",				999, 120, 116 },
	{ "HORN/PULL CORD/WATER",	999, 101,  97 },
	{ "LADDER/ROPE",			999, 121, 117 },
	{ "LADDER/GREEN VINE",		999, 122, 118 },
	{ "LADDER/VINES",			999, 123, 119 },
	{ "WITHERED GREEN VINE",	999,  67,  63 },
	{ "WITHERED VINES",			999, 111, 107 },
	{ "VINE DUST",				999, 124, 120 },
	{ "JAR/PIPE/HOSE",			999, 125, 121 },
	{ "LADDER/BROWN VINE",		999, 126, 122 },
	{ "ROPE/BROWN VINE",		999, 127, 123 },
	{ "MESSAGE LOG",			999,  94,  90 },
	{ "WITHERED BROWN VINE",	999,  68,  64 },
	{ "TOMB MESSAGE",			409, 128, 124 },
	{ "FAUCET PIPE/TUBE",		999, 137, 125 },
	{ nullptr,                    0,   0,   0 }
};

void Inventory::init() {
	for (const InvObject *item = INVENTORY_ITEMS; item->_name; ++item) {
		inv_register_thing(item->_name, nullptr, item->_scene, item->_cel, item->_cursor);

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

} // namespace Riddle
} // namespace M4
