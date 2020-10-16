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
namespace Quest {

const PlainGameDescriptor QUEST_GAME_LIST[] = {
	{ "quest", "Quest Game" },

	{ "adventureq", "Adventure!" },
	{ "attemptedassassination", "Attempted Assassination" },
	{ "hauntedhorror", "Haunted Horror" },
	{ "magicworld", "Magic World" },
	{ "redsaucemonday", "Red Sauce Monday" },
	{ "worldsend", "World's End" },

#ifdef QUEST_EXT
	// The games below are newer games that the Geas interpreter doesn't yet support
	{ "attackonfrightside", "Attack On Frightside" },
	{ "balaclava", "Balaclava" },
	{ "bearsepicquest", "Bear's Epic Quest" },
	{ "caught", "Caught!" },
	{ "cuttings", "Cuttings" },
	{ "draculacrl", "Dracula: CRL remake" },
	{ "elections4", "It's election time in Pakistan: Go rich boy, go!" },
	{ "everyman", "Everyman" },
	{ "firstTimes", "First Times" },
	{ "giftofthemagi", "Gift of the Magi" },
	{ "medievalistsquest", "Medievalist's Quest" },
	{ "parishotel", "Welcome to the Paris Hotel" },
	{ "questforloot", "Quest for loot and something else" },
	{ "sleepingassassin", "El asesino durmiente (The Sleeping Assassin)" },
	{ "spondre", "Spondre" },
	{ "murderjennylee", "The Brutal Murder of Jenny Lee" },
	{ "thelasthero", "The Last Hero" },
	{ "tokindlealight", "To Kindle a Light" },
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

#ifdef QUEST_EXT
	// The games below are newer games that the Geas interpreter doesn't yet support
	DT_ENTRY0("attackonfrightside", "84542fc6460833bbf2594ed83f8b1fc7", 46019),
	DT_ENTRY0("balaclava", "8b30af05d9986f9f962c677181ecc766", 57719),
	DT_ENTRY0("bearsepicquest", "e6896a65527f456b4362aaebcf39e354", 62075),
	DT_ENTRY0("caught", "4502d89d8e304fe4165d46eb22f21f10", 5168593),
	DT_ENTRY0("cuttings", "e0ded5a6b78e8c9482e746d55f61972c", 6583866),
	DT_ENTRY0("draculacrl", "1af3ec877584b290f7ab1a1be8f944a5", 4548737),
	DT_ENTRY0("elections4", "d0bc0cd54182d6099808767068592b94", 591994),
	DT_ENTRY0("everyman", "410c7211d3f0c700f34e97ed258e33f1", 56218),
	DT_ENTRY0("firstTimes", "31d878c82d99856d473762612f154eb6", 10253826),
	DT_ENTRY0("giftofthemagi", "b33132ce71c8a2eed0f6c1c1af284765", 78647),
	DT_ENTRY0("medievalistsquest", "e0a15bc2a74a0bd6bb5c24661ea35829", 127977271),
	DT_ENTRY0("parishotel", "c9a42bc3f306aba5e318b0a74115e0d4", 474983),
	DT_ENTRY0("questforloot", "f7e32aec0f961a59a69bead3fadff4f0", 1357373),
	DT_ENTRY0("sleepingassassin", "9c2aa213bb73d8083506ee6f64436d9d", 287227),
	DT_ENTRY0("spondre", "c639077eb487eb6d1b63cda2c9ba5a9b", 1169469),
	DT_ENTRY0("murderjennylee", "27d0f3ff28fd13e556203ab3d53edbe6", 80486),
	DT_ENTRY0("thelasthero", "31e10b8a7f11a6289955b89437f8178c", 62512),
	DT_ENTRY0("tokindlealight", "5d3b57830b003046a621620ba0869d7c", 811845),
	DT_ENTRY0("xanadu", "fef25e3473755ec572d4236d56f918e2", 396973),
#endif

	DT_END_MARKER
};

} // End of namespace Quest
} // End of namespace Glk
