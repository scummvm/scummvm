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

const char *const AMFV_DESC = "A Mind Forever Voyaging";
const char *const ARTHUR_DESC = "Arthur: The Quest for Excalibur";
const char *const BALLYHOO_DESC = "Ballyhoo";
const char *const BEYONDZORK_DESC = "Beyond Zork";
const char *const BORDERZONE_DESC = "Border Zone";
const char *const BUREAUCRACY_DESC = "Bureaucracy";
const char *const CUTTHROATS_DESC = "Cutthroats";
const char *const DEADLINE_DESC = "Deadline";
const char *const ENCHANTER_DESC = "Enchanter";
const char *const HHGTTG_DESC = "The Hitchhiker's Guide to the Galaxy";
const char *const HIJINX_DESC = "Hollywood Hijinx";
const char *const INFIDEL_DESC = "Infidel";
const char *const JOURNEY_DESC = "Journey";
const char *const LGOP_DESC = "Leather Goddesses of Phobos";
const char *const LGOP2_DESC = "Leather Goddesses of Phobos 2";
const char *const LURKING_DESC = "The Lurking Horror";
const char *const MINIZORK1_DESC = "Mini Zork I: The Great Underground Empire";
const char *const MOONMIST_DESC = "Moonmist";
const char *const NORDBERT_DESC = "Nord and Bert Couldn't Make Head or Tail of It";
const char *const PLANETFALL_DESC = "Planetfall";
const char *const PLUNDERED_DESC = "Plundered Hearts";
const char *const SAMPLER1_DESC = "Infocom Sampler 1";
const char *const SAMPLER2_DESC = "Infocom Sampler 2";
const char *const SEASTALKER_DESC = "Seastalker";
const char *const SHERLOCKRIDDLE_DESC = "Sherlock: The Riddle of the Crown Jewels";
const char *const SHOGUN_DESC = "James Clavell's Shogun";
const char *const SORCERER_DESC = "Sorcerer";
const char *const SPELLBREAKER_DESC = "Spellbreaker";
const char *const STARCROSS_DESC = "Starcross";
const char *const STATIONFALL_DESC = "Stationfall";
const char *const SUSPECT_DESC = "Suspect";
const char *const SUSPENDED_DESC = "Suspended";
const char *const TRINITY_DESC = "Trinity";
const char *const WISHBRINGER_DESC = "Wishbringer";
const char *const WITNESS_DESC = "The Witness";
const char *const ZORK0_DESC = "Zork Zero: The Revenge of Megaboz";
const char *const ZORK1_DESC = "Zork I: The Great Underground Empire";
const char *const ZORK2_DESC = "Zork II: The Wizard of Frobozz";
const char *const ZORK3_DESC = "Zork III: The Dungeon Master";
const char *const ZTUU_DESC = "Zork: The Undiscovered Underground";

#define NONE GUIO4(GUIO_NOSPEECH, GUIO_NOSFX, GUIO_NOMUSIC, GUIO_NOSUBTITLES)
#define ENTRY0(ID, DESCRIPTION, VERSION, MD5, FILESIZE) { ID, DESCRIPTION##_DESC, VERSION, MD5, FILESIZE, Common::EN_ANY, NONE }
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
