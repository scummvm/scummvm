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

const char *const INVENTORY_NAMES[123] = {
	"SOLDIER'S HELMET",
	"BUCKET",
	"RIPLEY PHOTO",
	"LEAD PIPE",
	"SEVEN SPOKES",
	"MALLET",
	"GONG",
	"WHEELED TOY",
	"CHARCOAL",
	"POST MARKED ENVELOPE",
	"TURTLE",
	"TURTLE TREATS",
	"PLANK",
	"PASS",
	"KEY",
	"ACE OF SPADES CARD",
	"STEP LADDER",
	"BILLIARD BALL",
	"CIGAR",
	"GLASS JAR",
	"ROMANOV EMERALD",
	"CORK",
	"LEVER KEY",
	"EDGER",
	"SURGICAL TUBE",
	"PUMP ROD",
	"FAUCET PIPE",
	"FAUCET HANDLE",
	"RUBBER PLUG",
	"GARDEN HOSE",
	"MENENDEZ'S LETTER",
	"KEYS",
	"BROKEN WHEEL",
	"PUMP GRIPS",
	"CLOCK FACING",
	"SHOVEL",
	"WOODEN LADDER",
	"ROPE",
	"GREEN VINE",
	"CRYSTAL SKULL",
	"BROWN VINE",
	"PULL CORD",
	"SPARK PLUG TOOL",
	"RONGORONGO TABLET",
	"SLEEVE",
	"LIGHTER",
	"STICK AND SHELL MAP",
	"DRIFTWOOD STUMP",
	"OBSIDIAN DISK",
	"SOLDIER'S SHIELD",
	"TWELVETREES' NOTE",
	"TWO SOLDIERS' SHIELDS",
	"WOODEN POST",
	"CRANK",
	"WOODEN BEAM",
	"EMPEROR'S SEAL",
	"POLE",
	"REBUS AMULET",
	"DRIFTWOOD PUFFIN",
	"SHRUNKEN HEAD",
	"METAL RIM",
	"CHINESE YUAN",
	"JOURNAL",
	"SILVER BUTTERFLY",
	"POSTAGE STAMP",
	"GERMAN BANKNOTE",
	"WHALE BONE HORN",
	"CHISEL",
	"INCENSE BURNER",
	"POMERANIAN MARKS",
	"PERUVIAN INTI",
	"SIKKIMESE RUPEE",
	"SERENITY WHEEL",
	"PEACE WHEEL",
	"INSIGHT WHEEL",
	"TRUTH WHEEL",
	"WISDOM WHEEL",
	"CIGAR WITHOUT BAND",
	"CIGAR BAND",
	"FARMER'S SHOVEL",
	"MATCH",
	"LIT LIGHTER",
	"VON SELTSAM'S NOTE",
	"PRAYER WHEEL BROCHURE",
	"ENVELOPE",
	"VON SELTSAM'S LETTER",
	"HORN/PULL CORD",
	"FAUCET PIPE/HOSE",
	"FAUCET PIPE/HOSE/TUBE",
	"JAR/CORK",
	"JAR/RUBBER PLUG",
	"JAR/GRIPS",
	"JAR/PLUG/GRIPS",
	"JAR/PLUG/CORK/GRIPS",
	"JAR/CORK/PLUG",
	"JAR/CORK/GRIPS",
	"VINES",
	"TUBE/HOSE",
	"US DOLLARS",
	"JAR/FAUCET PIPE",
	"EMERALD/CORK",
	"TWELVETREES' MAP",
	"PERIODIC TABLE",
	"LEVER KEY/PUMP ROD",
	"JAR/LEVER KEY",
	"PUMP ROD/GRIPS",
	"JAR/PERIODIC TABLE",
	"ROPE/GREEN VINE",
	"ROPE/VINES",
	"HORN/PULL CORD/WATER",
	"LADDER/ROPE",
	"LADDER/GREEN VINE",
	"LADDER/VINES",
	"WITHERED GREEN VINE",
	"WITHERED VINES",
	"VINE DUST",
	"JAR/PIPE/HOSE",
	"LADDER/BROWN VINE",
	"ROPE/BROWN VINE",
	"MESSAGE LOG",
	"WITHERED BROWN VINE",
	"TOMB MESSAGE",
	"FAUCET PIPE/TUBE"
};

static const uint16 INVENTORY_SCENES[123] = {
	203, 203, 998, 207, 207, 204, 204, 504,
	205, 304, 999, 305, 408, 204, 409, 406,
	403, 406, 456, 407, 407, 407, 407, 408,
	407, 407, 407, 407, 407, 407, 407, 406,
	207, 407, 501, 504, 504, 504, 504, 510,
	504, 604, 604, 607, 603, 608, 608, 600,
	605, 805, 603, 805, 805, 806, 844, 810,
	603, 207, 608, 504, 207, 999, 998, 204,
	999, 405, 608, 709, 709, 401, 501, 701,
	703, 704, 705, 705, 706, 406, 406, 801,
	801, 999, 999, 999, 406, 999, 999, 999,
	999, 999, 999, 999, 999, 999, 999, 999,
	999, 407, 998, 407, 407, 603, 407, 407,
	407, 407, 407, 999, 999, 999, 999, 999,
	999, 999, 999, 999, 999, 999, 999, 999,
	999, 409, 999
};

void Inventory::init() {
	for (int i = 0; i < INVENTORY_COUNT; ++i) {
		_G(inv_suppress_click_sound) = true;
		inv_move_object(INVENTORY_NAMES[i], INVENTORY_SCENES[i]);
	}

	_G(inv_suppress_click_sound) = false;
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

} // namespace Riddle
} // namespace M4
