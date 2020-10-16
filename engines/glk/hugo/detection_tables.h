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
namespace Hugo {

const PlainGameDescriptor HUGO_GAME_LIST[] = {
	{ "hugo", "Hugo IF Game" },

	{ "acs", "A Crimson Spring" },
	{ "annoyotron2", "Aggravatron: Annoyotron II" },
	{ "cb2", "The Clockwork Boy 2" },
	{ "cryptozookeeper", "Cryptozookeeper" },
	{ "hugoclock", "The Hugo Clock" },
	{ "adv350h", "Adventure, 350 point Colossal Cave" },
	{ "distress", "Distress" },
	{ "down", "Down" },
	{ "dragonhunt", "Dragon Hunt" },
	{ "eastofeastwood", "East of Eastwood" },
	{ "fallacyofdawn", "Fallacy of Dawn" },
	{ "futureboy", "Future Boy!" },
	{ "guiltybastards", "Guilty Bastards" },
	{ "halloweenhorror1", "The Halloween Horror - part 1" },
	{ "halloweenhorror2", "The Halloween Horror - part 2" },
	{ "hammurabi", "Hammurabi" },
	{ "htgessay", "Hauning the Ghosts" },
	{ "hugozork", "Hugo Zork 1" },
	{ "ish", "Escape from Ice Station Hippo" },
	{ "ndrift", "Necrotic Drift" },
	{ "nextday", "The Next Day" },
	{ "nmnl", "Nothing More, Nothing Less" },
	{ "partyarty", "Party Arty, Man of La Munchies" },
	{ "paxless", "PAXLess, Quest to the IF Suite" },
	{ "pirateadv", "Pirate Adventure" },
	{ "pantomime", "Pantomime" },
	{ "pom", "Persistence of Memory" },
	{ "renga", "Renga in Four Parts" },
	{ "retronemesis", "Retro-Nemesis" },
	{ "scavhunt", "Scavenger Hunt" },
	{ "spinning", "Spinning" },
	{ "spur", "Spur, A Western Misadventure" },
	{ "squest", "SceptreQuest" },
	{ "teleporttest", "Teleport Test"},
	{ "tetrish", "Tetris" },
	{ "tradingpunches", "Trading Punches" },
	{ "travellingswordsman", "Tales of the Travelling Swordsman" },
	{ "tripkey", "Tripkey" },

	{ "wfte", "Waiting for The End" },
	{ "worldbuilder", "Word Builder" },


	{ nullptr, nullptr }
};

const GlkDetectionEntry HUGO_GAMES[] = {
	DT_ENTRY1("acs", "Text Only", "37682bc8cbcc6706b8bd81d0d0b6745e", 371811),
	DT_ENTRY0("acs", "1daad86bee94f4519681c441d4f0f2bc", 371658),
	DT_ENTRY1("acs", "0.09.0 Beta",  "8788f1e28a4cd875f220155b2aeae8c8", 356450),
	DT_ENTRY0("annoyotron2", "944056721054fd1c9af9d1e95e63ce52", 66762),
	DT_ENTRY0("cb2", "232827c10abd45c98d77ceffaf9ac9fa", 155732),
	DT_ENTRY0("cryptozookeeper", "1b8301b969882c46ffa9f635e1695c41", 840567),
	DT_ENTRY0("hugoclock", "53a0b99011ccb14ddc45cfeb8c23d417", 120887),
	DT_ENTRY0("adv350h", "d6735640ca21797f24e3cadb12be4ae2", 124148),
	DT_ENTRY0("distress", "fe61c8722d8625d3d196b3d390346a55", 188317),
	DT_ENTRY0("down", "4bc119c61d3cdf5d796c36b1d9a023c6", 99561),
	DT_ENTRY1("down", "09-30-97", "7daf198e81a92b152bf6a0969210aa77", 94323),
	DT_ENTRY0("dragonhunt", "93db9cdf1d2d2800715c93fff0d48a59", 68944),
	DT_ENTRY0("eastofeastwood", "ebc4e37c66fca8a07b5782b57686ce07", 42445),
	DT_ENTRY0("fallacyofdawn", "8821566e2d3b301c6dc705f2bea54eb1", 729176),
	DT_ENTRY1("futureboy", "Demo", "cfce7ee7893bb5adc9ba4ea198f38201", 827396),
	DT_ENTRY1("guiltybastards", "Text Only", "77efc9a102a406a3b123172bb37e87e7", 246971),
	DT_ENTRY0("guiltybastards", "77efc9a102a406a3b123172bb37e87e7", 246971),
	DT_ENTRY0("halloweenhorror1", "db76f8a419767ebe6d1ad304e8001cba", 68923),
	DT_ENTRY0("halloweenhorror2", "34bb57521acd33f1f985f0898d8104a8", 55574),
	DT_ENTRY0("hammurabi", "738739f9dc7ffa041a13445b23e77e37", 46905),
	DT_ENTRY0("htgessay", "c3b5bce395f3f54097077f830dad70ac", 142921),
	DT_ENTRY0("hugozork", "b525f8bc83bc735fb5c62edd1b486499", 172150),
	DT_ENTRY0("ish", "023c9083378fcd1a08d97e60910b54da", 113958),
	DT_ENTRY0("ndrift", "fea92564f4ae4c626841aa4c93fcb31e", 556591),
	DT_ENTRY0("nextday", "2c5a9b95f6fb079986f8a4b178c9fcb4", 136844),
	DT_ENTRY0("nmnl", "cee4e1ffae1fd562d507d65c143739ef", 171732),
	DT_ENTRY0("partyarty", "62dac43addf6ea21e5759e098998773b", 86845),
	DT_ENTRY0("paxless", "db00b1242a4a0898c2d0d2d1c77103f4", 61973),
	DT_ENTRY0("pirateadv", "81c961f121e4465adb4592eee2bcc2d5", 53915),
	DT_ENTRY0("pantomime", "1067b09fda08eafb09d53b51ffe73e7d", 248466),
	DT_ENTRY1("pom", "15feb99", "ba86e162ba30e6dbe82abc96648486da", 95577),
	DT_ENTRY0("pom", "9a3724529fce408c2f4a2a1bbb635748", 92889),
	DT_ENTRY0("renga", "c79032bdc349863f02a4fab30beafd35", 64373),
	DT_ENTRY0("retronemesis", "517908b4503c653a0d9bb326d00b22ab", 101526),
	DT_ENTRY0("scavhunt", "665dce1a0f552e95590b983a3f2106da", 129514),
	DT_ENTRY0("spinning", "487d0a0cdcf55407a09cafdf6ca32237", 99140),
	DT_ENTRY0("spur", "77968cf043ecc012b4938c690b81227c", 185338),
	DT_ENTRY0("squest", "3e91849c6e8ea3072aa58e96010b6078", 36322),
	DT_ENTRY0("teleporttest", "4cc54ad4d5b8f3628aa3925272311efb", 100723),
	DT_ENTRY0("tetrish", "da1299e86f9fcded1f9a41979685ec02", 6464),
	DT_ENTRY0("tradingpunches", "b31b2eed49e788429cd9c5c641a3e713", 315934),
	DT_ENTRY1("tradingpunches", "1.6", "e7ca0323da847c364ad12e160af6c494", 294441),
	DT_ENTRY0("travellingswordsman", "37378ff2cfe75d0cfb581b7777036040", 431432),
	DT_ENTRY0("tripkey", "f76297d8ff7658752aa5a29417bbb274", 188058),
	DT_ENTRY0("wfte", "695233c271d6e355652bd11d0cc8da5e", 73099),
	DT_ENTRY0("worldbuilder", "031ff1a1364cd0d42600dc1bac967255", 90382),

	DT_END_MARKER
};

} // End of namespace Hugo
} // End of namespace Glk
