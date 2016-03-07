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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

namespace Wage {

#define ADGF_DEFAULT (ADGF_DROPLANGUAGE|ADGF_DROPPLATFORM)
#define ADGF_GENERIC (ADGF_DROPLANGUAGE|ADGF_DROPPLATFORM|ADGF_USEEXTRAASTITLE|ADGF_AUTOGENTARGET)

#define FANGAME(n,m,s) { "wage",n,AD_ENTRY1s(n,m,s),Common::EN_ANY,Common::kPlatformMacintosh,ADGF_GENERIC,GUIO0()}
#define FANGAMEN(n,f,m,s) { "wage",n,AD_ENTRY1s(f,m,s),Common::EN_ANY,Common::kPlatformMacintosh,ADGF_GENERIC,GUIO0()}
#define BIGGAME(t,v,f,m,s) { t,v,AD_ENTRY1s(f,m,s),Common::EN_ANY,Common::kPlatformMacintosh,ADGF_DEFAULT,GUIO0()}

static const ADGameDescription gameDescriptions[] = {
	FANGAME("3rd Floor", "a107d7a177970b2259e32681bd8b47c9", 285056),
	BIGGAME("afm", "v1.8", "Another Fine Mess 1.8", "8e5aa915f3253efb2aab52435647b25e", 1456000),
	BIGGAME("amot", "v1.8", "A Mess O' Trouble 1.8", "b3ef53afed282671b704e45df829350c", 1895552),
	FANGAME("Bug Hunt", "2ebd3515a87941063ad66c3cf93c5e78", 200064),
	// Problems with letter rendering
	FANGAME("Canal District", "8856bc699a20fc5b7fc67accee12cac7", 658176),
	BIGGAME("cantitoe", "", "Camp Cantitoe", "098aa5c11c58e1ef274a30a9e01b4755", 621440),
	// Invalid rect in scene "FINALE"
	FANGAME("Castle of Ert", "85f56731635e825e49a672c4fb0490dd", 205312),
	FANGAME("Deep Angst", "635f62bbc569e72b03cab9107927d03d", 335232),
	FANGAMEN("Dungeon World II", "DungeonWorld2", "e10c5e3cc17879c298b1551f33571b15", 234880),
	// Polygons with ignored byte 1
	FANGAME("Double Trouble", "5e9ee13d09ac54918ed111fa9727ac1c", 557184),
	FANGAME("Eidisi I", "299d1de4baccf1c66118396519953652", 180480),
	// Problems(?) with text on the first screen
	FANGAMEN("Enchanted Pencils", "Enchanted Pencils 0.99 (PG)", "35514583fe7ab36fad2569fc87bd887b", 414464),
	FANGAME("Escape from School!", "a854be48d4af20126d18a9cad93a969b", 51840),
	FANGAME("Exploration Zeta!", "b9fbb704017d7ea9613b0160f86527bb", 370944),
	// Crash in console rendering on the first scene
	FANGAME("Fantasy Quest", "599f0b2c7ecce65c39646c05f2c19c1b", 782848),
	FANGAME("Find the Heart", "7ae36ffa295651cd6d2d56981d6b5ff7", 108928), // From Joshua's Worlds 1.0
	FANGAME("Lost Crystal", "4f21ba8ee64f8d655b9eeb1e3ffd50f7", 792064),
	FANGAME("Magic Rings", "6e0d1dd561d3dad8f9a7a20ed1f09b16", 112000),
	FANGAME("Midnight Snack", "346982a32fc701f53bb19771d72063d0", 69504),
	FANGAME("Puzzle Piece Search", "51885fe2effeaa14b2b8b08a53931805", 252928), // From Joshua's Worlds 1.0
	// Empty(?)  first scene
	FANGAME("Pyramid of No Return", "48a9c668ce69206f57e11e1a85970d02", 392192),
	FANGAME("Queen Quest", "730605d312efedb5e3ff108522fcac18", 59776),
	// Crash in console rendering on the initial scene
	FANGAME("Quest for the Dark Sword", "7e4e712d151f6c686f6024b0dedf5d34", 590720),
	FANGAME("Quest for the Dark Sword", "308cf0fdfa129fa46b325b307f7e88c6", 590720), // Alteranative version
	BIGGAME("raysmaze", "v1.5", "Ray's Maze1.5", "e5a3e25dddfffbed461bca3c26073117", 1437184),
	BIGGAME("scepters", "", "Scepters", "b80bff315897776dda7689cdf829fab4", 360832),
	// ??? problems with dog bitmap?
	FANGAMEN("Space Adventure", "SpaceAdventure", "e5b0d8ad6d235ede2f08583342642dfa", 158720),
	FANGAME("Star Trek", "2395856df8dbefe9c0609caa985edf73", 55296),
	// Crash in bitmap drawing on the first scene
	FANGAME("Strange Disappearance", "f9eba5b315853a5599927db2a73c87d3", 781312),
	FANGAME("Time Bomb", "2df84b636237686b624e736a698a16c4", 66432),
	// Invalid rect in scene "Access Tube 1"
	FANGAMEN("The Phoenix v1.2", "The Phoenix", "7fa2a2ac740f22572516843922b7c630", 434560),
	FANGAME("The Sultan's Palace", "589aebf6c14bb5c63d9b4b2c37f31e16", 468096),
	// Admission for on 3rd screen is messed up
	FANGAME("The Tower", "75eba57a12ed181e07f34eaf6aa9d2c4", 568320),
	// Doesn't go past first scene
	BIGGAME("Twisted!", "", "Twisted! 1.6", "8ea8cc13d26d7975dc43ea7e6c193217", 992896),
	FANGAME("Wishing Well", "3ee884d0d1a168d088cf2250d3a83c73", 105600),
	FANGAME("ZikTuria", "e793155bed1a70fa2074a3fcd696b751", 54784),
	FANGAME("Zoony", "e6cc8a914a4215dafbcce6315dd12cf5", 160256),

	AD_TABLE_END_MARKER
};

} // End of namespace Wage
