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

/*
 * Based on ScottFree interpreter version 1.14 developed by Swansea
 * University Computer Society without disassembly of any other game
 * drivers, only of game databases as permitted by EEC law (for purposes
 * of compatibility).
 *
 * Licensed under GPLv2
 *
 * https://github.com/angstsmurf/spatterlight/tree/master/terps/scott
 */

#include "common/gui_options.h"
#include "common/language.h"
#include "engines/game.h"

namespace Glk {
namespace Scott {

const PlainGameDescriptor SCOTT_GAME_LIST[] = {
	// Scott Adams games
	{ "adventureland",		"Adventureland" },
	{ "pirateadventure",	"Pirate Adventure" },
	{ "missionimpossible",	"Mission Impossible" },
	{ "voodoocastle",		"Voodoo Castle" },
	{ "thecount",			"The Count" },
	{ "strangeodyssey",		"Strange Odyssey" },
	{ "mysteryfunhouse",	"Mystery Fun House" },
	{ "pyramidofdoom",		"Pyramid Of Doom" },
	{ "ghosttown",			"Ghost Town" },
	{ "savageisland",		"Savage Island" },
	{ "savageisland1",		"Savage Island, Part 1" },
	{ "savageisland2",		"Savage Island, Part 2" },
	{ "goldenvoyage",		"The Golden Voyage" },
	{ "claymorguesorcerer",	"Sorcerer of Claymorgue Castle" },
	{ "pirateisle",			"Return to Pirate Isle" },
	{ "buckaroobanzai",		"Buckaroo Banzai" },
	{ "marveladventure",	"Marvel Adventure #1" },
	{ "marveladventure2",	"Marvel Adventure #2" },
	{ "scottsampler",		"Adventure International's Mini-Adventure Sampler" },
	{ "robinofsherwood",	"Robin Of Sherwood" },

	// 11 Mysterious Adventures by Brian Howarth
	{ "goldenbaton", "Mysterious Adventures 1: The Golden Baton" },
	{ "timemachine", "Mysterious Adventures 2: The Time Machine" },
	{ "arrowofdeath1", "Mysterious Adventures 3: Arrow of Death Part 1" },
	{ "arrowofdeath2", "Mysterious Adventures 4: Arrow of Death Part 2" },
	{ "pulsar7", "Mysterious Adventures 5: Escape from Pulsar 7" },
	{ "circus", "Mysterious Adventures 6: Circus" },
	{ "feasibility", "Mysterious Adventures 7: Feasibility Experiment" },
	{ "akyrz", "Mysterious Adventures 8: The Wizard of Akyrz" },
	{ "perseus", "Mysterious Adventures 9: Perseus and Andromeda" },
	{ "10indians", "Mysterious Adventures 10: Ten Little Indians" },
	{ "waxworks11", "Mysterious Adventures 11: Waxworks" },
	{ "mysadv1", "11 Mysterious Adventures - Disk 1" },
	{ "mysadv2", "11 Mysterious Adventures - Disk 2" },

	// Other Games
	{ "desert",            "Desert Adventure" },
	{ "jamesbondadv",      "James Bond Adventure" },
	{ "burglarsadv",       "Burglar's Adventure" },
	{ "underseaconquest",  "Undersea Conquest part 1" },
	{ "gammaworld",        "Gamma World" },
	{ "marooned",          "Marooned" },
	{ "minersadv",         "Miner's Adventure" },
	{ "romulanadv",        "Romulan Adventure" },
	{ "topsecretadv",      "Top Secret Adventure" },
	{ "gremlins",		   "Gremlins" },
	{ "seasofblood",	   "Seas Of Blood" },

	{ nullptr, nullptr }
};

const GlkDetectionEntry SCOTT_GAMES[] = {
	// PC game versions
	DT_ENTRY0("adventureland",     "7c6f495d757a54e73d259efc718d8024", 15896),
	DT_ENTRY0("pirateadventure",   "ea535fa7684508410151b4561de1f323", 16325),
	DT_ENTRY0("missionimpossible", "379c77a9a483886366b3b5c425e56410", 15275),
	DT_ENTRY0("voodoocastle",      "a530a6857d1092eaa177eee575c94c71", 15852),
	DT_ENTRY0("thecount",          "5ebb4ade985670bb2eac54f8fa202214", 17476),
	DT_ENTRY0("strangeodyssey",    "c57bb6df04dc77a2b232bc5bcab6e417", 17489),
	DT_ENTRY0("mysteryfunhouse",   "ce2931ac3d5cbc270a5cb7be9e614f6e", 17165),
	DT_ENTRY0("pyramidofdoom",     "4e6127fad6b5d75eccd3f3b101f8c9c8", 17673),
	DT_ENTRY0("ghosttown",         "2c08327ab06d5490bd9e367ddaeca627", 17831),
	DT_ENTRY0("savageisland1",     "8feb77f11d32e9567ce2fc7d435eaf44", 19533),
	DT_ENTRY0("savageisland2",     "20c40a349f7a214ac515fb1d63c30a87", 18367),
	DT_ENTRY0("goldenvoyage",      "e2a8f956ab215012d1495550c4c11ee8", 18513),
	DT_ENTRY0("claymorguesorcerer","f986d7e1ee074f65b6c1d00461c9b3c3", 19232),
	DT_ENTRY0("pirateisle",        "6d98f422cc986d959a3c74351785aea3", 19013),
	DT_ENTRY0("buckaroobanzai",    "ee2c7139d58423c5e25dd918fcb48383", 17989),
	DT_ENTRY0("marveladventure",   "aadcc04e6b37eb9d30a58b5bc775842e", 18876),
	DT_ENTRY0("marveladventure2",  "f7e772643a24eaf94cfb2d48e3949ec1", 18643),
	DT_ENTRY0("scottsampler",      "d569a769f304dc02b3062d97458ddd01", 13854),

	// PDA game versions
	DT_ENTRY0("adventureland",     "ae541fc1085da2f7d561b72ed20a6bc1", 18003),
	DT_ENTRY0("pirateadventure",   "cbd47ab4fcfe00231ffd71d52378d410", 18482),
	DT_ENTRY0("missionimpossible", "9251ab2c64e63559d8a6e9e6246760a5", 17227),
	DT_ENTRY0("voodoocastle",      "be849c5747c7fc3b201984afb4403b8e", 18140),
	DT_ENTRY0("thecount",          "85b75b6079b5ee572b5259b29a0e5d21", 19999),
	DT_ENTRY0("strangeodyssey",    "c423cae841ac1927b5b2e503607b21bc", 20115),
	DT_ENTRY0("mysteryfunhouse",   "326b98b991d401605074e64d474ce566", 19700),
	DT_ENTRY0("pyramidofdoom",     "8ef9010399f055da9adb15ce7745a11c", 20320),
	DT_ENTRY0("ghosttown",         "fcdcca8b2acf76ba2d0006cefa3630a1", 20687),
	DT_ENTRY0("savageisland1",     "c8aaa80f07c40fa8e4b17432644919dc", 22669),
	DT_ENTRY0("savageisland2",     "2add0f28d9b236c866890cdf8d86ee60", 21169),
	DT_ENTRY0("goldenvoyage",      "675126bd0477e8ed9230ad3db5afc45f", 21401),
	DT_ENTRY0("claymorguesorcerer","0ef0def798d895ed766041fa99dd28a0", 22346),
	DT_ENTRY0("pirateisle",        "0bf1bcc649422798332a38c88588fdff", 22087),
	DT_ENTRY0("buckaroobanzai",    "a0a5423967287dae9cbeb9abe8324479", 21038),

	// 11 Mysterious Adventures
	DT_ENTRY0("goldenbaton",   "2ebf7b1dd515aff34b99b40f27af00e3", 13373),
	DT_ENTRY0("timemachine",   "a10cde2063bb56694000013b32dc0798", 13742),
	DT_ENTRY0("arrowofdeath1", "6f7040fe091eeb9ebdc2fd7533b94ceb", 13412),
	DT_ENTRY0("arrowofdeath2", "c5758034b83b06258e46091a80abebde", 15554),
	DT_ENTRY0("pulsar7",       "7597704b423c1ca2bea3a48263b5f4dc", 17777),
	DT_ENTRY0("circus",        "93a7947e2edb7c45036446168b427546", 13621),
	DT_ENTRY0("feasibility",   "c8f423a79ad0e508f43ba4ce91f1e573", 13441),
	DT_ENTRY0("akyrz",         "645c089d6d66324472d939993793eb57", 16803),
	DT_ENTRY0("perseus",       "2f5bb15b461fbdc000243acabd324c67", 15080),
	DT_ENTRY0("10indians",     "3bb85fd505b4d42bd1767c1a76347e2f", 14215),
	DT_ENTRY0("waxworks11",    "af134c32cc0d50329d6e7335639ded88", 16068),
	DT_ENTRY0("goldenbaton",   "028303fd062c39b59b28982cde75f085", 53374),
	DT_ENTRY0("timemachine",   "b2ff405412d92b06373d9a5efbb048d4", 54062),
	DT_ENTRY0("arrowofdeath1", "f11f85802eff8aed9e212aef7f26ede1", 68150),
	DT_ENTRY0("arrowofdeath2", "8f2609b1270248c93cdf254435f9d410", 77544),
	DT_ENTRY0("pulsar7",       "72e14dbfca7ced5c0aadf019b47b4024", 55516),
	DT_ENTRY0("circus",        "64d44e66c4f5353150f8fbaceaa99800", 48624),
	DT_ENTRY0("feasibility",   "2d717aa4f0eba77b77e259f81f92fc37", 78456),
	DT_ENTRY0("akyrz",         "3467e09eb247c474ae7e8a8d503968d0", 65008),
	DT_ENTRY0("perseus",       "4d8389c0ea3425bd4d92492538f63b19", 56914),
	DT_ENTRY0("10indians",     "72e91da7590138b78d370a3cb1448f3c", 64660),
	DT_ENTRY0("waxworks11",    "4d4ee8aa1f24e1745ab1aa017590dcff", 54556),

	// Other games
	DT_ENTRY0("desert",           "c43e19228bae08eab74bc080e17bbe16", 12287),
	DT_ENTRY0("jamesbondadv",     "5c59396a2db040148a5d86beb37b2bb9", 12405),
	DT_ENTRY0("burglarsadv",      "0072d8afcd30aa1577350dcfad269e47", 11541),
	DT_ENTRY0("underseaconquest", "d57705f8f17f0b6044a575accf9cbfd1",  5616),
	DT_ENTRY0("gammaworld",       "b980c44e8a49aa9d71e92f6b6bf1d136", 11531),
	DT_ENTRY0("marooned",         "a1ac54630a0583c19269901ec10cd0b1", 12576),
	DT_ENTRY0("minersadv",        "0000d9da5a13701601fb3e7399daa128", 11898),
	DT_ENTRY0("romulanadv",       "d97b5cb5ed66eb276ef9f1c1bae0b8dd", 13959),
	DT_ENTRY0("topsecretadv",     "effb411e74dfe3a8d69b57b9bc3a2cef", 15575),

	// ZX Spectrum games
	DT_ENTRY1("marveladventure", "ZXSpectrum", "0eec511d3cde815c73e5464ab0cdbef9", 40727),

	// 11 Mysterious Adventures
	DT_ENTRY1("goldenbaton",	"ZXSpectrum", "cb7dadc9d5f8bce453b9139265e4dd7d", 32060),
	DT_ENTRY1("timemachine",	"ZXSpectrum", "b22d1f4d46c99ff4443d541d3fe424c1", 30928),
	DT_ENTRY1("arrowofdeath1",	"ZXSpectrum", "3a5c3f4079c1c0347f03420db8ad4596", 34105),
	DT_ENTRY1("arrowofdeath2",	"ZXSpectrum", "d3f8943c4f5f71ce00139065055a72ee", 38043),
	DT_ENTRY1("pulsar7",		"ZXSpectrum", "441edd90fc7f9ff39a5eebe035a974e9", 29961),
	DT_ENTRY1("circus",			"ZXSpectrum", "ed99306a2fb23bf6579068a4d74034ee", 27746),
	DT_ENTRY1("feasibility",	"ZXSpectrum", "5e381e83f15d77e3542be4a4cffc8e25", 37395),
	DT_ENTRY1("akyrz",			"ZXSpectrum", "b0f8676817475753f1edd7f1eeea31fb", 33753),
	DT_ENTRY1("perseus",		"ZXSpectrum", "84d5fbb16a37e495abf09d191fd8b1a2", 31504),
	DT_ENTRY1("10indians",		"ZXSpectrum", "afde056c152de79ea20453c42a2d08af", 31664),
	DT_ENTRY1("waxworks11",		"ZXSpectrum", "6c6fbbbb50032463a6ea71c6750ea1f5", 32662),

	// C64 Games
	DT_ENTRY1("robinofsherwood", "C64", "10109d9776b9372f9c768b53a664b113", 174848),
	DT_ENTRY1("robinofsherwood", "C64", "552c95ec15d750cbfa02c1f11dcbca1e", 36278),
	DT_ENTRY1("robinofsherwood", "C64", "4262f85382d1bc3b8924a1929511a558", 45807),
	DT_ENTRY1("robinofsherwood", "C64", "bf3a4d72cff5ef97bebce6b12c756df2", 46736),
	DT_ENTRY1("gremlins",		 "C64", "33c920f7ba150dfa1a832d510eebd8fe", 174848),
	DT_ENTRY1("gremlins",		 "C64", "947e35037cf02269ac013857925137ce", 174848),
	DT_ENTRY1("gremlins",		 "C64", "95b2582a89c59961d5b709c9b32e4131", 174848),
	DT_ENTRY1("gremlins",		 "C64", "644c366956202d41df0ea1c4303c5895", 174848),
	DT_ENTRY1("gremlins",		 "C64", "108063b2a16a199794f2ecf52ce26377", 56724),
	DT_ENTRYL1("gremlins", Common::DE_DEU, "C64", "c60977950ff22ae57483f073345b8373", 174848),
	DT_ENTRYL1("gremlins", Common::DE_DEU, "C64", "e5d743d8727c8aca011a737bbb5ad818", 49155),
	DT_ENTRY1("seasofblood",	 "C64", "267c3fe2bb150365de0358f07b5df15c", 174848),
	DT_ENTRY1("seasofblood",	 "C64", "0300c2d21289157539bbd03ab4e366ee", 174848),
	DT_ENTRY1("seasofblood",	 "C64", "46280fb1d701a41492b8434467c31029", 41481),
	DT_ENTRY1("marveladventure", "C64", "d80e133dd396565f773052cb317e8222", 174848),
	DT_ENTRY1("marveladventure2","C64", "5d0ea85ca1f260ca718a6bbb6da4cdb9", 174848),
	DT_ENTRY1("marveladventure2","C64", "3d88539a6dd7e6e179bb61041125cc0f", 36466),

	//Scott Adams
	DT_ENTRY1("adventureland",		"C64", "05cf6c64ecde5288ae2e46099bfd19a3", 27152),
	DT_ENTRY1("adventureland",		"C64", "5af919881417920ec6a3961b4577f587", 27152),
	DT_ENTRY1("adventureland",		"C64", "ccd3e3c805134b4fc36ad92e1cae623f", 174848),
	DT_ENTRY1("adventureland",		"C64", "547036c586bfcd53e741ecfad74e3001", 175531),
	DT_ENTRY1("adventureland",		"C64", "6c3de0b0ef39fad9d63e788de8cd972c", 175531),
	DT_ENTRY1("claymorguesorcerer", "C64", "a1db488c49ad221fa0dc79591cb5a3db", 28663),
	DT_ENTRY1("claymorguesorcerer", "C64", "1ebaf9a378355246aa7ed2623bb27fab", 49373),
	DT_ENTRY1("claymorguesorcerer", "C64", "dde67117a432420ef05f8d665fbbbe10", 37167),
	DT_ENTRY1("claymorguesorcerer", "C64", "8d2af429e53df1c4da0d21bdc9de6826", 48223),
	DT_ENTRY1("claymorguesorcerer", "C64", "64fcee173adecc0f03c595e25d4def04", 174848),
	DT_ENTRY1("savageisland",		"C64", "414d459ceb211230356ad823475866b3", 174848),
	DT_ENTRY1("savageisland",		"C64", "f0087b1f42ea9a0656462bf339278b08", 174848),

	// 11 Mysterious Adventures
	DT_ENTRY1("mysadv1",		"C64", "b4d8fc4eabed4f2400717303561ad0fa", 174848),
	DT_ENTRY1("mysadv2",		"C64", "3ce5ea1a0473244bf469fd3c51f1dc48", 174848),
	DT_ENTRY1("goldenbaton",	"C64", "f2711fe0376442f6f320da1b73b5b1a3", 20848),
	DT_ENTRY1("goldenbaton",	"C64", "ed22cb234af638e7d9f570b937f9fc52", 174848),
	DT_ENTRY1("timemachine",	"C64", "cc8e94d3fb665d5d23b728e9c1f262ba", 20530),
	DT_ENTRY1("arrowofdeath1",	"C64", "da044c4c57dc194002ba47f5b2921411", 23366),
	DT_ENTRY1("arrowofdeath2",	"C64", "3d40cb011167e7ae9f6695cdd1f4a1bf", 24546),
	DT_ENTRY1("pulsar7",		"C64", "db68753e4c4c536693edca2f58747044", 18111),
	DT_ENTRY1("circus",			"C64", "4f732cb27e2a0bb484443a7dd1974ccf", 17001),
	DT_ENTRY1("feasibility",	"C64", "6c7ed2fd5f0247a55beb266344967662", 23163),
	DT_ENTRY1("akyrz",			"C64", "94e4b070e45204b12d1655091c56045d", 19425),
	DT_ENTRY1("akyrz",			"C64", "c423ad31ab3f6b45f0215c2e7fc3eb7e", 174848),
	DT_ENTRY1("perseus",		"C64", "96a1ccb3212808eee03e74cdc1f0d1a4", 20523),
	DT_ENTRY1("10indians",		"C64", "79ee3669ccfff7338dfc1810228005dc", 20383),
	DT_ENTRY1("waxworks11",		"C64", "facc11aa8b51e88a807236b765203eb0", 18961),
	DT_END_MARKER
};

} // End of namespace Scott
} // End of namespace Glk
