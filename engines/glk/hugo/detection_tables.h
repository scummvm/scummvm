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

#include "engines/game.h"
#include "common/gui_options.h"
#include "common/language.h"

namespace Glk {
namespace Hugo {

const PlainGameDescriptor HUGO_GAME_LIST[] = {
	{ "hugo", "Hugo IF Game" },

	{ "acs", "A Crimson Spring" },
	{ "adv350h", "Adventure, 350 point Colossal Cave" },
	{ "annoyotron2", "Aggravatron: Annoyotron II" },
	{ "babyuncleny", "Baby Uncle New Year" },
	{ "captainspeedo14", "Captain Speedo: Victim of the Vacuum" },
	{ "captainspeedo16", "Captain Speedo: Let Them Heat cake!" },
	{ "captainspeedo42", "Captain Speedo: So Long, and Thanks for All the Books!" },
	{ "captainspeedo112", "Captain Speedo: Alert on Aleph V!" },
	{ "captainspeedotng", "Captain Speedo: The New Generation" },
	{ "cb2", "The Clockwork Boy 2" },
	{ "chicken", "The Chicken's Dilemma" },
	{ "clockworkboy", "Tales of a Clockwork Boy"},
	{ "cryptozookeeper", "Cryptozookeeper" },
	{ "dino", "The Loneliness of the Long Distance Runner" },
	{ "distress", "Distress" },
	{ "dddhugo", "Doom, Death, Destruction and All That" },
	{ "down", "Down" },
	{ "dragonhunt", "Dragon Hunt" },
	{ "eastofeastwood", "East of Eastwood" },
	{ "enceladus", "Enceladus" },
	{ "fallacyofdawn", "Fallacy of Dawn" },
	{ "futureboy", "Future Boy!" },
	{ "guiltybastards", "Guilty Bastards" },
	{ "halloweenhorror1", "The Halloween Horror - part 1" },
	{ "halloweenhorror2", "The Halloween Horror - part 2" },
	{ "hammurabi", "Hammurabi" },
	{ "htgessay", "Hauning the Ghosts" },
	{ "hugoclock", "The Hugo Clock" },
	{ "hugozork", "Hugo Zork 1" },
	{ "ish", "Escape from Ice Station Hippo" },
	{ "leather", "Leather" },
	{ "madrigal", "Madrigals of War and Love" },
	{ "marjorie", "Will the Real Marjorie Hopkirk Please Stand Up?" },
	{ "ndrift", "Necrotic Drift" },
	{ "nextday", "The Next Day" },
	{ "nmnl", "Nothing More, Nothing Less" },
	{ "overbrook", "Overbrook, an Interactive House Tour" },
	{ "pantomime", "Pantomime" },
	{ "partyarty", "Party Arty, Man of La Munchies" },
	{ "paxless", "PAXLess, Quest to the IF Suite" },
	{ "pirateadv", "Pirate Adventure" },
	{ "pom", "Persistence of Memory" },
	{ "renga", "Renga in Four Parts" },
	{ "retronemesis", "Retro-Nemesis" },
	{ "scavhunt", "Scavenger Hunt" },
	{ "spinning", "Spinning" },
	{ "spur", "Spur, A Western Misadventure" },
	{ "squest", "SceptreQuest" },
	{ "stormoverlondon", "Storm Over London" },
	{ "teleporttest", "Teleport Test"},
	{ "tetrish", "Tetris" },
	{ "theoniondestiny", "The Onion of Destiny" },
	{ "theprofile", "The Profile" },
	{ "tradingpunches", "Trading Punches" },
	{ "travellingswordsman", "Tales of the Travelling Swordsman" },
	{ "tripkey", "Tripkey" },
	{ "trollface", "Trollface" },
	{ "vaulthugoii", "Vault of Hugo II: Electric Boogaloo" },
	{ "wfte", "Waiting for The End" },
	{ "worldbuilder", "Word Builder" },

	{ nullptr, nullptr }
};

const GlkDetectionEntry HUGO_GAMES[] = {
	DT_ENTRY1("acs", "0.09.0 Beta", "8788f1e28a4cd875f220155b2aeae8c8", 356450),
	DT_ENTRY1("acs", "1.0.04 Text Only", "37682bc8cbcc6706b8bd81d0d0b6745e", 371811),
	DT_ENTRY1("acs", "1.0.04", "1daad86bee94f4519681c441d4f0f2bc", 371658),
	DT_ENTRY0("adv350h", "d6735640ca21797f24e3cadb12be4ae2", 124148),
	DT_ENTRY0("annoyotron2", "944056721054fd1c9af9d1e95e63ce52", 66762),
	DT_ENTRY0("babyuncleny", "e2bedee095297428c8043c35c4fdca17", 71128),
	DT_ENTRY0("captainspeedo14", "c4ee09af9c66d9322adeaa599c97a591", 59700),
	DT_ENTRY0("captainspeedo16", "0d11e711150f08d0c04f9b414ba5c71b", 67984),
	DT_ENTRY0("captainspeedo42", "5a704cc50805400416f25105162025b6", 65544),
	DT_ENTRY0("captainspeedo112", "6ccbc38144b5b83627e022ec83f7ced6", 59706),
	DT_ENTRY0("captainspeedotng", "bc38464151bbba4dd37795ea42f041e8", 63837),
	DT_ENTRY0("cb2", "232827c10abd45c98d77ceffaf9ac9fa", 155732),
	DT_ENTRY0("chicken", "eed5770a3f88d95987d018271ffa2342", 48718),
	DT_ENTRY0("clockworkboy", "c42a897dc519597fb41ed2d310fc67be", 73139),
	DT_ENTRY1("clockworkboy", "Competition Release", "c3c24c9b98933b0e47b229b50c53d501", 63451),
	DT_ENTRY0("cryptozookeeper", "1b8301b969882c46ffa9f635e1695c41", 840567),
	DT_ENTRY1("cryptozookeeper", "1.05", "411494b65bf491ab3dd7237eaa7387a7", 1136856),
	DT_ENTRY0("dddhugo", "8d15ee9bd41d0cc1ffa97b1d3397a2f7", 50908),
	DT_ENTRY0("dino", "e465ea9b77192432c6af82715ef59ef7", 59056),
	DT_ENTRY0("distress", "fe61c8722d8625d3d196b3d390346a55", 188317),
	DT_ENTRY1("down", "r1", "7daf198e81a92b152bf6a0969210aa77", 94323),
	DT_ENTRY1("down", "r4", "4bc119c61d3cdf5d796c36b1d9a023c6", 99561),
	DT_ENTRY0("dragonhunt", "93db9cdf1d2d2800715c93fff0d48a59", 68944),
	DT_ENTRY0("eastofeastwood", "ebc4e37c66fca8a07b5782b57686ce07", 42445),
	DT_ENTRY1("enceladus", "1.00", "49b3e2a3087455c31ce929427f9510d8", 233986),
	DT_ENTRY1("enceladus", "1.01", "092ffa4ca5d35d83013a1094b58ce22c", 234191),
	DT_ENTRY1("fallacyofdawn", "1.05", "8821566e2d3b301c6dc705f2bea54eb1", 729176),
	DT_ENTRY1("fallacyofdawn", "1.07", "bebb4427004a6cded72068b0ea04b7b6", 730694),
	DT_ENTRY1("futureboy", "Demo", "cfce7ee7893bb5adc9ba4ea198f38201", 827396),
	DT_ENTRY1("guiltybastards", "Text Only", "77efc9a102a406a3b123172bb37e87e7", 246971),
	DT_ENTRY0("guiltybastards", "77efc9a102a406a3b123172bb37e87e7", 246971),
	DT_ENTRY0("halloweenhorror1", "db76f8a419767ebe6d1ad304e8001cba", 68923),
	DT_ENTRY0("halloweenhorror2", "34bb57521acd33f1f985f0898d8104a8", 55574),
	DT_ENTRY0("hammurabi", "738739f9dc7ffa041a13445b23e77e37", 46905),
	DT_ENTRY0("htgessay", "c3b5bce395f3f54097077f830dad70ac", 142921),
	DT_ENTRY1("hugoclock", "v1", "9a1ab9fd3fcb52ed99751e8525020739", 105469),
	DT_ENTRY1("hugoclock", "v2", "53a0b99011ccb14ddc45cfeb8c23d417", 120887),
	DT_ENTRY0("hugozork", "b525f8bc83bc735fb5c62edd1b486499", 172150),
	DT_ENTRY0("ish", "023c9083378fcd1a08d97e60910b54da", 113958),
	DT_ENTRY0("leather", "4b8dc6050cec83b304f1eed39f4c6c24", 58409),
	DT_ENTRY0("madrigal", "e63550ddff05b06d8148ec4f05d4eabc", 66902),
	DT_ENTRY1("marjorie", "Demo", "826ff07155c2334a7d26aa826cae82e2", 55205),
	DT_ENTRY1("ndrift", "1.03", "fea92564f4ae4c626841aa4c93fcb31e", 556591),
	DT_ENTRY1("ndrift", "1.04", "8e23d867e3fc13157c1b31195550244d", 556990),
	DT_ENTRY0("nextday", "2c5a9b95f6fb079986f8a4b178c9fcb4", 136844),
	DT_ENTRY0("nmnl", "cee4e1ffae1fd562d507d65c143739ef", 171732),
	DT_ENTRY0("overbrook", "0ed7b671ed9cd3b6362419b5159366e3", 70752),
	DT_ENTRY1("pantomime", "Release 1", "874d6dab0820fd4b550d5d8c87cb8783", 247545),
	DT_ENTRY1("pantomime", "Release 2", "1067b09fda08eafb09d53b51ffe73e7d", 248466),
	DT_ENTRY1("partyarty", "v1", "7047bc315ad1410ef38a771e539c40b3", 84396),
	DT_ENTRY1("partyarty", "v2", "62dac43addf6ea21e5759e098998773b", 86845),
	DT_ENTRY1("paxless", "Release 1", "db00b1242a4a0898c2d0d2d1c77103f4", 61973),
	DT_ENTRY1("paxless", "Release 2", "c4038b711d9f0ecb4ca2586623471a52", 61973),
	DT_ENTRY0("pirateadv", "81c961f121e4465adb4592eee2bcc2d5", 53915),
	DT_ENTRY1("pom", "r1", "9a3724529fce408c2f4a2a1bbb635748", 92889),
	DT_ENTRY1("pom", "r2", "ba86e162ba30e6dbe82abc96648486da", 95577),
	DT_ENTRY0("renga", "c79032bdc349863f02a4fab30beafd35", 64373),
	DT_ENTRY0("retronemesis", "517908b4503c653a0d9bb326d00b22ab", 101526),
	DT_ENTRY0("scavhunt", "665dce1a0f552e95590b983a3f2106da", 129514),
	DT_ENTRY0("spinning", "487d0a0cdcf55407a09cafdf6ca32237", 99140),
	DT_ENTRY0("spur", "77968cf043ecc012b4938c690b81227c", 185338),
	DT_ENTRY0("squest", "3e91849c6e8ea3072aa58e96010b6078", 36322),
	DT_ENTRY0("stormoverlondon", "13767bc8e9ca0795b6084f13da904d32", 119420),
	DT_ENTRY0("teleporttest", "4cc54ad4d5b8f3628aa3925272311efb", 100723),
	DT_ENTRY0("tetrish", "da1299e86f9fcded1f9a41979685ec02", 6464),
	DT_ENTRY0("theoniondestiny", "29435113419753447900f644e8858ed1", 52942),
	DT_ENTRY1("theprofile", "v1.00", "2231e5789fc6d1a956a40d630478ef4a", 78569),
	DT_ENTRY1("theprofile", "v1.02", "618c8c3702f98bc58a0a100b7380d51a", 80015),
	DT_ENTRY1("tradingpunches", "1.6", "e7ca0323da847c364ad12e160af6c494", 294441),
	DT_ENTRY1("tradingpunches", "1.9", "b31b2eed49e788429cd9c5c641a3e713", 315934),
	DT_ENTRY0("travellingswordsman", "37378ff2cfe75d0cfb581b7777036040", 431432),
	DT_ENTRY0("tripkey", "f76297d8ff7658752aa5a29417bbb274", 188058),
	DT_ENTRY0("trollface", "ff47b6d8f36cc2a4c2d41fa878631e77", 90620),
	DT_ENTRY1("trollface", "Strict Mode Release", "acffc9c316749a2d96f452a1feb5c788", 86182),
	DT_ENTRY0("vaulthugoii", "632054772b73993e5d7672c430d04d36", 61530),
	DT_ENTRY0("wfte", "695233c271d6e355652bd11d0cc8da5e", 73099),
	DT_ENTRY1("worldbuilder", "v1", "ca41b45835288f694871769ab22b8a5a", 82732),
	DT_ENTRY1("worldbuilder", "v2", "031ff1a1364cd0d42600dc1bac967255", 90382),

	DT_END_MARKER
};

} // End of namespace Hugo
} // End of namespace Glk
