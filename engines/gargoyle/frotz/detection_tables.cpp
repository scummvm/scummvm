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

#include "gargoyle/frotz/detection_tables.h"

namespace Gargoyle {
namespace Frotz {

const char *const AMFV = "A Mind Forever Voyaging";
const char *const ARTHUR = "Arthur: The Quest for Excalibur";
const char *const BALLYHOO = "Ballyhoo";
const char *const BEYONDZORK = "Beyond Zork";
const char *const BORDERZONE = "Border Zone";
const char *const BUREAUCRACY = "Bureaucracy";
const char *const CUTTHROATS = "Cutthroats";
const char *const DEADLINE = "Deadline";
const char *const ENCHANTER = "Enchanter";
const char *const HHGTTG = "The Hitchhiker's Guide to the Galaxy";
const char *const HIJINX = "Hollywood Hijinx";
const char *const INFIDEL = "Infidel";
const char *const JOURNEY = "Journey";
const char *const LGOP = "Leather Goddesses of Phobos";
const char *const LGOP2 = "Leather Goddesses of Phobos 2";
const char *const LURKING = "The Lurking Horror";
const char *const MINIZORK1 = "Mini Zork I: The Great Underground Empire";
const char *const MOONMIST = "Moonmist";
const char *const NORDBERT = "Nord and Bert Couldn't Make Head or Tail of It";
const char *const PLANETFALL = "Planetfall";
const char *const PLUNDERED = "Plundered Hearts";
const char *const SAMPLER1 = "Infocom Sampler 1";
const char *const SAMPLER2 = "Infocom Sampler 2";
const char *const SEASTALKER = "Seastalker";
const char *const SHERLOCKRIDDLE = "Sherlock: The Riddle of the Crown Jewels";
const char *const SHOGUN = "James Clavell's Shogun";
const char *const SORCERER = "Sorcerer";
const char *const SPELLBREAKER = "Spellbreaker";
const char *const STARCROSS = "Starcross";
const char *const STATIONFALL = "Stationfall";
const char *const SUSPECT = "Suspect";
const char *const SUSPENDED = "Suspended";
const char *const TRINITY = "Trinity";
const char *const WISHBRINGER = "Wishbringer";
const char *const WITNESS = "The Witness";
const char *const ZORK0 = "Zork Zero: The Revenge of Megaboz";
const char *const ZORK1 = "Zork I: The Great Underground Empire";
const char *const ZORK2 = "Zork II: The Wizard of Frobozz";
const char *const ZORK3 = "Zork III: The Dungeon Master";
const char *const ZTUU = "Zork: The Undiscovered Underground";

#define NONE GUIO4(GUIO_NOSPEECH, GUIO_NOSFX, GUIO_NOMUSIC, GUIO_NOSUBTITLES)
#define ENTRY0(ID, DESC, VERSION, MD5, FILESIZE) { ID, DESC, VERSION, MD5, FILESIZE, Common::EN_ANY, NONE }
#define FROTZ_TABLE_END_MARKER { nullptr, nullptr, nullptr, nullptr, 0, Common::EN_ANY, "" }

const FrotzGameDescription FROTZ_GAMES[] = {
	ENTRY0("hhgttg", HHGTTG, "v31 Solid Gold", "379022bcd4ec74b90274c6100c33f579", 158412),
	ENTRY0("hhgttg", HHGTTG, "v47", "fdda8f4239819402c62db866bb61a648", 112622),
	ENTRY0("hhgttg", HHGTTG, "v56", "a214fcb42bc9f554d07d983a12f6a062", 113444),
	ENTRY0("hhgttg", HHGTTG, "v58", "e867d49ad1fb9406ff4e0678a4ee2ac9", 113332),
	ENTRY0("hhgttg", HHGTTG, "v59", "34f6abc1f2a42be127ef434fc475f0ee", 113334),

	FROTZ_TABLE_END_MARKER
};


} // End of namespace Frotz
} // End of namespace Gargoyle
