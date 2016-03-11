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

#define ADGF_DEFAULT (ADGF_DROPLANGUAGE|ADGF_DROPPLATFORM|ADGF_MACRESFORK)
#define ADGF_GENERIC (ADGF_DROPLANGUAGE|ADGF_DROPPLATFORM|ADGF_USEEXTRAASTITLE|ADGF_AUTOGENTARGET|ADGF_MACRESFORK)

#define FANGAME(n,m,s) { "wage",n,AD_ENTRY1s(n,m,s),Common::EN_ANY,Common::kPlatformMacintosh,ADGF_GENERIC,GUIO0()}
#define FANGAMEN(n,f,m,s) { "wage",n,AD_ENTRY1s(f,m,s),Common::EN_ANY,Common::kPlatformMacintosh,ADGF_GENERIC,GUIO0()}
#define BIGGAME(t,v,f,m,s) { t,v,AD_ENTRY1s(f,m,s),Common::EN_ANY,Common::kPlatformMacintosh,ADGF_DEFAULT,GUIO0()}

static const ADGameDescription gameDescriptions[] = {
	FANGAME("3rd Floor", "913812a1ac7a6b0e48dadd1afa1c7763", 281409),
	BIGGAME("afm", "v1.8", "Another Fine Mess 1.8", "94a9c4f8b3dabd1846d76215a49bd221", 1420723),
	BIGGAME("amot", "v1.8", "A Mess O' Trouble 1.8", "26207bdf0bb539464f136f0669af885f", 1843104),
	FANGAME("Bug Hunt", "595117cbed33e8de1ab3714b33880205", 195699),
	BIGGAME("cantitoe", "", "Camp Cantitoe", "913812a1ac7a6b0e48dadd1afa1c7763", 616985),
	// Problems with letter rendering
	FANGAME("Canal District", "a56aa3cd4a6e070e15ce1d5815c7be0a", 641470),
	// Invalid rect in scene "FINALE"
	FANGAME("Castle of Ert", "327610eb2298a9427a566288312df040", 198955),
	FANGAME("Deep Angst", "b130b3c811cd89024dd5fdd2b71f70b8", 329550),
	// Polygons with ignored byte 1
	FANGAME("Double Trouble", "1652e36857a04c01dc560234c4818619", 542371),
	FANGAMEN("Dungeon World II", "DungeonWorld2", "0154ea11d3cbb536c13b4ae9e6902d48", 230199),
	FANGAME("Eidisi I", "595117cbed33e8de1ab3714b33880205", 172552),
	// Problems(?) with text on the first screen
	FANGAMEN("Enchanted Pencils", "Enchanted Pencils 0.99 (PG)", "595117cbed33e8de1ab3714b33880205", 408913),
	FANGAME("Escape from School!", "913812a1ac7a6b0e48dadd1afa1c7763", 50105),
	FANGAME("Exploration Zeta!", "c477921aeee6ed0f8997ba44447eb2d0", 366599),
	// Crash in console rendering on the first scene
	FANGAME("Fantasy Quest", "4b0e1a1fbaaa4930accd0f9f0e1519c7", 762754),
	FANGAME("Find the Heart", "595117cbed33e8de1ab3714b33880205", 106235), // From Joshua's Worlds 1.0
	FANGAME("Karth of the Jungle", "595117cbed33e8de1ab3714b33880205", 96711),
	FANGAME("Karth of the Jungle", "595117cbed33e8de1ab3714b33880205", 96960), // Alternative version
	FANGAME("Karth of the Jungle II", "c106835ab4436de054e03aec3ce904ce", 201053),
	FANGAMEN("Little Pythagoras", "Little Pythagoras 1.1.1", "94a9c4f8b3dabd1846d76215a49bd221", 628821),
	FANGAME("Lost Crystal", "8174c81ea1858d0079ae040dae2cefd3", 771072),
	FANGAME("Magic Rings", "913812a1ac7a6b0e48dadd1afa1c7763", 109044),
	FANGAME("Midnight Snack", "913812a1ac7a6b0e48dadd1afa1c7763", 67952),
	FANGAME("Minitorian", "913812a1ac7a6b0e48dadd1afa1c7763", 586464),
	FANGAME("Pavilion", "4d991d7d1534d48d90598d86ea6d5d97", 231687),
	FANGAME("Puzzle Piece Search", "595117cbed33e8de1ab3714b33880205", 247693), // From Joshua's Worlds 1.0
	// Empty(?)  first scene
	FANGAME("Pyramid of No Return", "77a55a45f794b4d4a56703d3acce871e", 385145),
	FANGAME("Queen Quest", "4b0e1a1fbaaa4930accd0f9f0e1519c7", 57026),
	FANGAME("Quest for T-Rex", "913812a1ac7a6b0e48dadd1afa1c7763", 592584),
	// Crash in console rendering on the initial scene
	FANGAME("Quest for the Dark Sword", "b35dd0c078da9f35fc25a455f56bb129", 572576),
	FANGAME("Radical Castle 1.0", "677bfee4afeca2f7152eb8b76c85ca8d", 347278),
	BIGGAME("raysmaze", "v1.5", "Ray's Maze1.5", "064b16d8c20724f8debbbdc3aafde538", 1408516),
	BIGGAME("raysmaze", "v1.5/alt", "Ray's Maze1.5", "92cca777800c3d31a77b5ed7f6ee49ad", 1408516),
	BIGGAME("scepters", "", "Scepters", "3311deef8bf82f0b4b1cfa15a3b3289d", 346595),
	// ??? problems with dog bitmap?
	FANGAMEN("Space Adventure", "SpaceAdventure", "f9f3f1c419f56955f7966355b34ea5c8", 155356),
	FANGAME("Star Trek", "44aaef4806578700429de5aaf95c266e", 53320),
	// Crash in bitmap drawing on the first scene
	FANGAME("Strange Disappearance", "d81f2d03a1e863f04fb1e3a5495b720e", 772282),
	FANGAME("Time Bomb", "4b0e1a1fbaaa4930accd0f9f0e1519c7", 64564),
	FANGAMEN("The Hotel Caper", "The Hotel Caper V1.0", "595117cbed33e8de1ab3714b33880205", 231969),
	// Invalid rect in scene "Access Tube 1"
	FANGAMEN("The Phoenix v1.2", "The Phoenix", "4b0e1a1fbaaa4930accd0f9f0e1519c7", 431640),
	FANGAME("The Sultan's Palace", "358799d446ee4fc12f793febd6c94b95", 456855),
	// Admission for on 3rd screen is messed up
	FANGAME("The Tower", "435f420b9dff895ae1ddf1338040c51d", 556539),
	// Doesn't go past first scene
	BIGGAME("twisted", "", "Twisted! 1.6", "26207bdf0bb539464f136f0669af885f", 960954),
	FANGAME("Wishing Well", "913812a1ac7a6b0e48dadd1afa1c7763", 103688),
	FANGAME("Wizard's Warehouse", "913812a1ac7a6b0e48dadd1afa1c7763", 159748),
	FANGAME("ZikTuria", "418e74ca71029a1e9db80d0eb30c0843", 52972),

	AD_TABLE_END_MARKER
};

} // End of namespace Wage
