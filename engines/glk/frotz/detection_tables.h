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
	{ "lurkinghorror", "The Lurking Horror" },
	{ "minizork1", "Mini Zork I: The Great Underground Empire" },
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
	ENTRY0("amfv", "R77-850814", "b7ffaed0ca4a90450f92b34066133377", 262016),
	ENTRY0("amfv", "R79-851122", "1e37dbcf7ccc9244dbfc3229796362f4", 262544),
	ENTRY0("arthur", "R54-890606", "ced2c66d03a49de0e8190b468332f081", 271360),
	ENTRY0("arthur", "R74-890714", "13d13f375f85a874c82a8ac7ad69dc41", 269200),
	ENTRY0("ballyhoo", "R97-851218", "7944e832a7d7b34037c7b6791de43dbd", 128556),
	ENTRY0("beyondzork", "R49-870917", "a5547795def620d0a75a064f9a37ab2d", 261900),
	ENTRY0("beyondzork", "R51-870923", "73948f415596fa4d9afe442b2c19e61f", 261548),
	ENTRY0("beyondzork", "R57-871221", "c56cac07a500e5864a994b19286bc07c", 261388),
	ENTRY0("borderzone", "R9-871008", "189231ed0675f6be3be86856f49211af", 178372),
	ENTRY0("bureaucracy", "R86-870212", "2bb00311d4c201082cfcd278ae5db921", 243144),
	ENTRY0("bureaucracy", "R116-870602", "a8ae194257a989ed3d82648a507466f2", 243340),
	ENTRY0("cutthroats", "R23-840809", "059801d9f90fffeb3645816c37c7eda2", 112558),
	ENTRY0("deadline", "R22-820809", "1610e84ca2505885566e648c1c525976", 111782),
	ENTRY0("deadline", "R26-821108", "e1ae6af1098067b86076c34865ae713c", 108372),
	ENTRY0("deadline", "R27-831006", "166ffb7cabc6b85f210655f371c89c46", 108454),
	ENTRY0("enchanter", "R10-830810", "7b41d915b4c2e31423d99925e9438aa4", 109126),
	ENTRY0("enchanter", "R15-831107", "e70f21aad650dd196fa3601cab5e0fc5", 109230),
	ENTRY0("enchanter", "R16-831118", "46187e0691f6f5ecdd5a336885db6aad", 109234),
	ENTRY0("enchanter", "R29-860820", "f87cdafad3682ead25cfc473656ff713", 111126),
	ENTRY0("hhgttg", "R47-840914", "fdda8f4239819402c62db866bb61a648", 112622),
	ENTRY0("hhgttg", "R56-841221", "a214fcb42bc9f554d07d983a12f6a062", 113444),
	ENTRY0("hhgttg", "R58-851002", "e867d49ad1fb9406ff4e0678a4ee2ac9", 113332),
	ENTRY0("hhgttg", "R59-851108", "34f6abc1f2a42be127ef434fc475f0ee", 113334),
	ENTRY0("hhgttg", "R31-871119", "379022bcd4ec74b90274c6100c33f579", 158412),
	ENTRY0("hollywoodhijinx", "R37-861215", "7b52824057ae24e098c228c41460ef75", 109650),
	ENTRY0("infidel", "R22-830916", "38f713e53af720624434529ea780040c", 93556),
	ENTRY0("journey", "R30-890322", "c9893bc0399080bd3850d4db2120d110", 280472),
	ENTRY0("journey", "R77-890616", "8a4ab56f62e1b7c918b837794182dbcd", 282176),
	ENTRY0("journey", "R83-890706", "c33ea33ab8aec6c617734dcfe1211067", 282312),
	ENTRY0("lgop", "R0", "69b3534570851b90d7f53ebe9d224a6a", 128998),
	ENTRY0("lgop", "R4-880405", "6bdae7434df7c03f3589ece0bed3317d", 159928),
	ENTRY0("lgop", "R59-860730", "e81237e220a612c5a93fbcc1fdf85a0a", 129022),
	ENTRY0("lurkinghorror", "R203", "e2d2505510479fec0405727e3d0abc10", 128986),
	ENTRY0("lurkinghorror", "R219", "83936d75c2cfd71fb64bf63c4696b9ac", 129704),
	ENTRY0("lurkinghorror", "R221", "c60cd0bf3c6eda867241378c7cb5464a", 129944),
	ENTRY0("minizork1", "R34-871124", "0d7700679e5e63dec97f712698610a46", 52216),




	FROTZ_TABLE_END_MARKER
};

} // End of namespace Frotz
} // End of namespace Glk
