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
namespace Quest {

const PlainGameDescriptor QUEST_GAME_LIST[] = {
	{ "quest", "Quest Game" },

	{ "adventureq", "Adventure!" },
	{ "attemptedassassination", "Attempted Assassination" },
	{ "beam", "Beam" },
	{ "bladesentinel", "The Blade Sentinel" },
	{ "gatheredindarkness", "Gathered in Darkness" },
	{ "hauntedhorror", "Haunted Horror" },
	{ "lovesong", "Lovesong" },
	{ "magicworld", "Magic World" },
	{ "redsaucemonday", "Red Sauce Monday" },
	{ "worldsend", "World's End" },

#ifdef QUEST_EXT
	// The games below are newer games that the Geas interpreter doesn't yet support
	{ "arcii", "ARC II" },
	{ "attackonfrightside", "Attack On Frightside" },
	{ "balaclava", "Balaclava" },
	{ "bearsepicquest", "Bear's Epic Quest" },
	{ "caught", "Caught!" },
	{ "cuttings", "Cuttings" },
	{ "draculacrl", "Dracula: CRL remake" },
	{ "dreampieces", "Dream Pieces" },
	{ "elections4", "It's election time in Pakistan: Go rich boy, go!" },
	{ "escapebathhouse", "Escape from the Mechanical Bathhouse" },
	{ "everyman", "Everyman" },
	{ "exittheroom", "Exit the Room" },
	{ "firstTimes", "First Times" },
	{ "giftofthemagi", "Gift of the Magi" },
	{ "medievalistsquest", "Medievalist's Quest" },
	{ "micky", "Micky" },
	{ "parishotel", "Welcome to the Paris Hotel" },
	{ "questforloot", "Quest for loot and something else" },
	{ "signos", "Signos" },
	{ "sleepingassassin", "El asesino durmiente (The Sleeping Assassin)" },
	{ "spondre", "Spondre" },
	{ "murderjennylee", "The Brutal Murder of Jenny Lee" },
	{ "thelasthero", "The Last Hero" },
	{ "myothianfalcon", "The Myothian Falcon" },
	{ "tokindlealight", "To Kindle a Light" },
	{ "welcometoparishotel", "Welcome to the Paris Hotel!" },
	{ "xanadu", "Xanadu - The World's Only Hope" },
#endif
	{ nullptr, nullptr }
};

const GlkDetectionEntry QUEST_GAMES[] = {
	DT_ENTRY0("adventureq", "93a358f817066494dbdabf222fc20595", 6974),
	DT_ENTRY0("attemptedassassination", "e8cf55898bcc5ee43a2527d5fefeaaff", 18833),
	DT_ENTRY0("hauntedhorror", "89a5d511aed564d4810b372d271e33fa", 19635),
	DT_ENTRY0("magicworld", "463cf8919c7321f3af305534b7ae78f3", 15176),
	DT_ENTRY0("redsaucemonday", "5a2f3e25d4a8c77e0c53d980dbb37451", 20324),
	DT_ENTRY0("worldsend", "4f5daac10085927bf5180bea24f7ef0d", 73396),

	// Competition 2001
	DT_ENTRY0("lovesong", "2ea679cc6ee0735141571eb96075523d", 41264),

	// Competition 2002
	DT_ENTRY0("bladesentinel", "c348d637606430e2597678927c2f6a69", 51473),

	// Competition 2006
	DT_ENTRY1("beam", "1.10", "d696f04cb7e6851e0bcbde0dbd78cbd0", 68543),

	// Competition 2007
	DT_ENTRY0("gatheredindarkness", "40f75b697ffb77bba88da6b431efad78", 1052274),

#ifdef QUEST_EXT
	// The games below are newer games that the Geas interpreter doesn't yet support
	DT_ENTRY0("arcii", "3c208e2afd9f032508de9bebf1b83f9a", 17268977),
	DT_ENTRY0("attackonfrightside", "84542fc6460833bbf2594ed83f8b1fc7", 46019),
	DT_ENTRY0("balaclava", "8b30af05d9986f9f962c677181ecc766", 57719),
	DT_ENTRY0("bearsepicquest", "e6896a65527f456b4362aaebcf39e354", 62075),
	DT_ENTRY0("caught", "4502d89d8e304fe4165d46eb22f21f10", 5168593),
	DT_ENTRY0("cuttings", "e0ded5a6b78e8c9482e746d55f61972c", 6583866),
	DT_ENTRY0("draculacrl", "1af3ec877584b290f7ab1a1be8f944a5", 4548737),
	DT_ENTRY0("elections4", "d0bc0cd54182d6099808767068592b94", 591994),
	DT_ENTRY0("escapebathhouse", "02f7ba339e50c8faa8c5dc3a1c051d7b", 1067048),
	DT_ENTRY0("everyman", "410c7211d3f0c700f34e97ed258e33f1", 56218),
	DT_ENTRY1("exittheroom", "1.9", "5b600a1833b59ad115cb868ccc9d6f14", 129094),
	DT_ENTRY0("firstTimes", "31d878c82d99856d473762612f154eb6", 10253826),
	DT_ENTRY0("giftofthemagi", "b33132ce71c8a2eed0f6c1c1af284765", 78647),
	DT_ENTRY0("medievalistsquest", "e0a15bc2a74a0bd6bb5c24661ea35829", 127977271),
	DT_ENTRY0("micky", "9c2aa213bb73d8083506ee6f64436d9d", 287227),
	DT_ENTRY0("parishotel", "c9a42bc3f306aba5e318b0a74115e0d4", 474983),
	DT_ENTRY0("questforloot", "f7e32aec0f961a59a69bead3fadff4f0", 1357373),
	DT_ENTRY0("sleepingassassin", "9c2aa213bb73d8083506ee6f64436d9d", 287227),
	DT_ENTRY1("spondre", "1.1a", "c639077eb487eb6d1b63cda2c9ba5a9b", 1169469),
	DT_ENTRY0("murderjennylee", "27d0f3ff28fd13e556203ab3d53edbe6", 80486),
	DT_ENTRY0("thelasthero", "31e10b8a7f11a6289955b89437f8178c", 62512),
	DT_ENTRY1("tokindlealight", "1.2", "5d3b57830b003046a621620ba0869d7c", 811845),
	DT_ENTRY0("welcometoparishotel", "c9a42bc3f306aba5e318b0a74115e0d4", 474983),
	DT_ENTRY0("xanadu", "fef25e3473755ec572d4236d56f918e2", 396973),

	// Competition 2011
	DT_ENTRY1("myothianfalcon", "1.2", "b5e7680eeb4f27c195f58472fa8eb146", 97861),

	// Competition 2012
	DT_ENTRY0("signos", "636793562d75ee82a4ea10d3bd3c62d6", 2311079),

	// Competition 2013
	DT_ENTRY1("dreampieces", "5.0", "fcb0fcc94ba24ba308415fd02a6f6c95", 274779),
#endif

	DT_END_MARKER
};

} // End of namespace Quest
} // End of namespace Glk
