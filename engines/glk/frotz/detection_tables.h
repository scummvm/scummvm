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

#include "engines/game.h"
#include "common/gui_options.h"
#include "common/language.h"

namespace Glk {
namespace Frotz {

/**
 * Game descriptor for ZCode games
 */
struct FrotzGameDescription {
	const char *const _gameId;
	const char *const _extra;
	const char *const _md5;
	size_t _filesize;
	Common::Language _language;
	const char *const _guiOptions;
};

const PlainGameDescriptor FROTZ_GAME_LIST[] = {
	{ "zcode", "Unknown Z-code game" },

	// Infocom games
	{ "amfv", "A Mind Forever Voyaging" },
	{ "arthur", "Arthur: The Quest for Excalibur" },
	{ "ballyhoo", "Ballyhoo" },
	{ "beyondzork", "Beyond Zork" },
	{ "borderzone", "Border Zone" },
	{ "bureaucracy", "Bureaucracy" },
	{ "cutthroats", "Cutthroats" },
	{ "deadline", "Deadline" },
	{ "enchanter", "Enchanter" },
	{ "hhgttg", "The Hitchhiker's Guide to the Galaxy" },
	{ "hollywoodhijinx", "Hollywood Hijinx" },
	{ "infidel", "Infidel" },
	{ "journey", "Journey" },
	{ "lgop", "Leather Goddesses of Phobos" },
	{ "lgop2", "Leather Goddesses of Phobos 2" },
	{ "lurkinghorror", "The Lurking Horror" },
	{ "minizork", "Mini Zork I: The Great Underground Empire" },
	{ "moonmist", "Moonmist" },
	{ "nordbert", "Nord and Bert Couldn't Make Head or Tail of It" },
	{ "planetfall", "Planetfall" },
	{ "plunderedhearts", "Plundered Hearts" },
	{ "infocomsampler1", "Infocom Sampler 1" },
	{ "infocomsampler2", "Infocom Sampler 2" },
	{ "seastalker", "Seastalker" },
	{ "sherlockriddle", "Sherlock: The Riddle of the Crown Jewels" },
	{ "shogun", "James Clavell's Shogun" },
	{ "sorcerer", "Sorcerer" },
	{ "spellbreaker", "Spellbreaker" },
	{ "starcross", "Starcross" },
	{ "stationfall", "Stationfall" },
	{ "suspect", "Suspect" },
	{ "suspended", "Suspended" },
	{ "trinity", "Trinity" },
	{ "wishbringer", "Wishbringer" },
	{ "thewitness", "The Witness" },
	{ "zork0", "Zork Zero: The Revenge of Megaboz" },
	{ "zork1", "Zork I: The Great Underground Empire" },
	{ "zork2", "Zork II: The Wizard of Frobozz" },
	{ "zork3", "Zork III: The Dungeon Master" },
	{ "ztuu", "Zork: The Undiscovered Underground" },
	{ nullptr, nullptr }
};

#define NONE GUIO4(GUIO_NOSPEECH, GUIO_NOSFX, GUIO_NOMUSIC, GUIO_NOSUBTITLES)
#define ENTRY0(ID, VERSION, MD5, FILESIZE) { ID, VERSION, MD5, FILESIZE, Common::EN_ANY, NONE }
#define FROTZ_TABLE_END_MARKER { nullptr, nullptr, nullptr, 0, Common::EN_ANY, "" }

const FrotzGameDescription FROTZ_GAMES[] = {
	ENTRY0("hhgttg", "R47-840914", "fdda8f4239819402c62db866bb61a648", 112622),
	ENTRY0("hhgttg", "R56-841221", "a214fcb42bc9f554d07d983a12f6a062", 113444),
	ENTRY0("hhgttg", "R58-851002", "e867d49ad1fb9406ff4e0678a4ee2ac9", 113332),
	ENTRY0("hhgttg", "R59-851108", "34f6abc1f2a42be127ef434fc475f0ee", 113334),
	ENTRY0("hhgttg", "R31-871119", "379022bcd4ec74b90274c6100c33f579", 158412),

	FROTZ_TABLE_END_MARKER
};

} // End of namespace Frotz
} // End of namespace Glk
