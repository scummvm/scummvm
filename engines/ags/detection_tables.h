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

namespace AGS {

static const PlainGameDescriptor GAME_NAMES[] = {
	{ "ags", "Adventure Game Studio Game" },

	{ "atotk", "A Tale Of Two Kingdoms" },
	{ "bcremake", "Black Cauldron Remake" },
	{ "blackwell1", "The Blackwell Legacy"},
	{ "blackwell2", "Blackwell Unbound"},
	{ "blackwell3", "The Blackwell Convergence"},
	{ "blackwell4", "The Blackwell Deception"},
	{ "blackwell5", "The Blackwell Epiphany"},
	{ "kq2agdi", "Kings Quest II - Romancing The Stones" },
	{ "kq1agdi", "King's Quest I: Quest for the Crown Remake" },
	{ "kq2agdi", "King's Quest II: Romancing The Stones" },
	{ "kq3agdi", "King's Quest III Redux: To Heir is Human" },
	{ "geminirue", "Gemini Rue"},
	{ "goldenwake", "A Golden Wake"},
	{ "kathyrain", "Kathy Rain"},
	{ "qfi", "Quest for Infamy"},
	{ "oott", "Order of the Thorne: The King's Challenge"},
	{ "primordia", "Primordia"},
	{ "resonance", "Resonance"},
	{ "mage", "Mage's Initiation: Reign of the Elements"},
	{ "unavowed", "Unavowed"},

	{ 0, 0 }
};

#define ENGLISH_ENTRY(ID, FILENAME, MD5, SIZE) { \
		{ \
			ID, \
			nullptr, \
			AD_ENTRY1s(FILENAME, MD5, SIZE), \
			Common::EN_ANY, \
			Common::kPlatformUnknown, \
			ADGF_NO_FLAGS, \
			GUIO1(GUIO_NOSPEECH) \
		} \
	}

static const AGSGameDescription GAME_DESCRIPTIONS[] = {
	ENGLISH_ENTRY("bcremake", "bc.exe", "0710e2ec71042617f565c01824f0cf3c", 7683255),
	ENGLISH_ENTRY("atotk", "atotk.exe", "37cf2d4d07842d45b59c6dd9387c1ee7", 42872046),
	ENGLISH_ENTRY("atotk", "atotk.exe", "37cf2d4d07842d45b59c6dd9387c1ee7", 42740200),
	ENGLISH_ENTRY("blackwell1", "blackwell1.exe", "605e124cb7e0b56841c471e2d641c224", 18822697), // GOG
	ENGLISH_ENTRY("blackwell2", "unbound.exe", "5c3a940514d91431e8e1c372018851ca", 14493753), // GOG
	ENGLISH_ENTRY("blackwell3", "convergence.exe", "2260c1a21aba7ac00baf0100d4ca54f1", 172575801), // GOG
	ENGLISH_ENTRY("blackwell4", "deception.exe", "b3b192cf20a2f7666ddea3410dbd87cc", 303459336), // GOG
	ENGLISH_ENTRY("blackwell5", "epiphany.exe", "c1cddd6fcdbcd030beda9f10d4e4270a", 281849897), // GOG
	ENGLISH_ENTRY("kq1agdi", "kq1vga.exe", "688f1807c9d8df26fc0f174dc756054e", 8278611),  // 4.1c
	ENGLISH_ENTRY("kq2agdi", "kq2vga.exe", "40cfb7563df7dacf6530b19289a4745b", 12563246),	// 3.1
	ENGLISH_ENTRY("kq2agdi", "kq2vga.exe", "40cfb7563df7dacf6530b19289a4745b", 12574643),  // 3.1c
	ENGLISH_ENTRY("kq3agdi", "kq3redux.exe", "e569fb2ceabdc4a1609348c23ebc0821", 11986266),  // 1.1
	ENGLISH_ENTRY("geminirue", "gemini rue.exe", "f3c0c7d3892bdd8963e8ce017f73de08", 61986506), // GOG
	ENGLISH_ENTRY("goldenwake", "a-golden-wake.exe", "dbe281d93c914899886e77c09c3c65ec", 130844360), // Steam
	ENGLISH_ENTRY("kathyrain", "kathyrain.exe", "434e24a12ba3cfb07d7b4b2f0e0bb1bf", 197487159), // Steam
	ENGLISH_ENTRY("qfi", "qfi.exe", "0702df6e67ef87fd3c51d09303803126", 534847265), // GOG
	ENGLISH_ENTRY("oott", "OotT-TKC.exe", "11c2421258465cba4bd773c49d918ee3", 467834855), // GOG
	ENGLISH_ENTRY("primordia", "primordia.exe", "22313e59c3233001488c26f18c80cc08", 973495830), // GOG
	ENGLISH_ENTRY("resonance", "resonance.exe", "2e635c22bcbf0ed3d46f1bcde71812d4", 849404957), // GOG
	// For some macOS and iOS releases the executable was renamed to ac2game.dat
	ENGLISH_ENTRY("mage", "ac2game.dat", "2e822f554994f36e0c62da2acda874da", 30492258), // GOG, Mac
	ENGLISH_ENTRY("unavowed", "ac2game.dat", "b1ff7d96667707daf4266975cea2bf90", 1755457364), // Steam, Mac

	{ AD_TABLE_END_MARKER }
};

} // namespace AGS
