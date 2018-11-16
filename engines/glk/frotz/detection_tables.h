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
	ENTRY0("moonmist", "R4-860918", "284797c3025ffaf76aecfa5c2bbffa86", 129002),
	ENTRY0("moonmist", "R9-861022", "698475de2769c66bc5a1eca600c71561", 128866),
	ENTRY0("nordbert", "R19-870722", "da1e189e19e3b24b2e35bd41fc32d261", 170284),
	ENTRY0("planetfall", "R20-830708", "15815c461a8548b7630d2aee46d07cc7", 107958),
	ENTRY0("planetfall", "R26-831014", "cf6ce61eb2eff9d4f18d7bcba7c12cfb", 108674),
	ENTRY0("planetfall", "R29-840118", "9facd8b974e658520fb762af4c4789dc", 109052),
	ENTRY0("planetfall", "R37-851003", "01844816673414c97d21dc003304989b",109398),
	ENTRY0("planetfall", "R10-880531", "34c69f1d24418fd4d2de195a1d7546c4", 136560),
	ENTRY0("plunderedhearts", "R26-870730", "fe5b9eb91949d41838166364f1753b10", 128962),
	ENTRY0("infocomsampler1", "R26-840731", "5483febc51abd55fb5e04c4c97a0b260", 112610),
	ENTRY0("infocomsampler1", "R53-850407", "47b8b8394e25faec870a798145529688", 126708),
	ENTRY0("infocomsampler1", "R55-850823", "05d9d1a1c3c73fce9e24ab695ece16c8", 126902),
	ENTRY0("infocomsampler2", "R97-870601", "201fa230a942df5aa75bb5b5f609e8ce", 125314),
	ENTRY0("seastalker", "R15-840501", "2f0220b0390deda695e01832a92b5493", 117738),
	ENTRY0("seastalker", "R15-840522", "050961fa7788c309bbf40accbff2ffdf", 117728),
	ENTRY0("seastalker", "R16-850515", "eb39dff7beb3589c8581dd2e3569eb78", 117752),
	ENTRY0("seastalker", "R16-850603", "bccf194b1e823e37db2431b586662773", 117762),
	ENTRY0("seastalker", "R86-840320", "64fb27e7b9fd682ff4f0d0ec6616a468", 116456),
	ENTRY0("sherlockriddle", "R21-871214", "69862f7f07a4e977159ea4da7f2f2ba6", 188444),
	ENTRY0("sherlockriddle", "R26-880127", "2cb2bda2e34eb7f9494cb585720e74cd", 190180),
	ENTRY0("shogun", "R322-890706", "62cca41feb94082442026f44f3e48e19", 344816),
	ENTRY0("sorcerer", "R4-840131", "d4a914fdfe90f5cd055a03b9aa24addd", 109734),
	ENTRY0("sorcerer", "R6-840508", "7ee357c10a9e049fe7c641a4817ee575", 109482),
	ENTRY0("sorcerer", "R13-851021", "7a076459806eaee72015b2b2882a89dc", 108692),
	ENTRY0("sorcerer", "R15-851108", "cde88a011d2ba183ff69b47b0d8881c6", 108682),
	ENTRY0("spellbreaker", "R63-850916", "b7b9eef231dee03fb40a9d98416fa0d5", 128480),
	ENTRY0("spellbreaker", "R87-860904", "852286847f4cdd790075fa824260ff4e", 128916),
	ENTRY0("starcross", "R15-820901", "fb2e6d9a0ad5822f3a8d4aec949e4e3c", 84984),
	ENTRY0("starcross", "R17-821021", "ed1e62e1f0eb9d819be45c076c5729f7", 83792),
	ENTRY0("stationfall", "R107-870430", "cfadfb66afabaa2971ec9b4ae65266ca", 128934),
	ENTRY0("suspect", "R14-841005", "3d759ccb19233f51968fa79d7374b393", 118692),



	FROTZ_TABLE_END_MARKER
};

} // End of namespace Frotz
} // End of namespace Glk
