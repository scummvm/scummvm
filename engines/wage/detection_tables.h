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

namespace Wage {

#define ADGF_DEFAULT (ADGF_DROPLANGUAGE|ADGF_DROPPLATFORM|ADGF_MACRESFORK)
#define ADGF_GENERIC (ADGF_DEFAULT|ADGF_USEEXTRAASTITLE|ADGF_AUTOGENTARGET)
#define ADGF_DEMO (ADGF_GENERIC|ADGF_DEMO)

#define FANGAME(n,m,s) { "wage",n,AD_ENTRY1s(n,m,s),Common::EN_ANY,Common::kPlatformMacintosh,ADGF_GENERIC,GUIO0()}
#define FANGAMEN(n,f,m,s) { "wage",n,AD_ENTRY1s(f,m,s),Common::EN_ANY,Common::kPlatformMacintosh,ADGF_GENERIC,GUIO0()}
#define FANGAMEND(n,f,m,s) { "wage",n,AD_ENTRY1s(f,m,s),Common::EN_ANY,Common::kPlatformMacintosh,ADGF_DEMO,GUIO0()}
#define BIGGAME(t,v,f,m,s) { t,v,AD_ENTRY1s(f,m,s),Common::EN_ANY,Common::kPlatformMacintosh,ADGF_DEFAULT,GUIO0()}

static const ADGameDescription gameDescriptions[] = {
	FANGAME("3rd Floor", "3ed49d2163e46d2c9b33fd80927d9e22", 281409),
	FANGAME("3rd Floor", "3ed49d2163e46d2c9b33fd80927d9e22", 281423), // alt version
	BIGGAME("afm", "v1.8", "Another Fine Mess 1.8", "abc7188469a9a7083fd4caec55a4f76e", 1420723),
	BIGGAME("amot", "v1.8", "A Mess O' Trouble 1.8", "6b59e5bb9a4b74ecdd9f66d4e36a59cf", 1843104),
	// Crash on third screen
	FANGAME("Brownie's Dream", "6fdcce532bcd50b7e4f3f6bab50a0ee6", 440704),
	FANGAMEN("Brownie's Time Travels", "Brownie's Time Travels v1.2", "55842a100b56e236c5ad69563e01fc24", 471589),
	FANGAME("Bug Hunt", "738e2e8a1020be48c5ef42da571674ae", 195699),
	FANGAME("Bug Hunt", "118a41121143488719d28daa9af8cd39", 195779), // alt version
	BIGGAME("cantitoe", "", "Camp Cantitoe", "1780c41d14b876461a19dbeceebf2a37", 616985),
	FANGAME("Canal District", "34e7a8e84b33ba8ea38b4ffd76ef074f", 641470),
	FANGAME("Carbon Copy", "9e781acd63290ae390d515cffc742011", 519445),
	// Invalid rect in scene "FINALE"
	FANGAME("Castle of Ert", "a45b439bb3a9c8a4a14b996024222068", 198955),
	FANGAMEN("Castle of Ert", "Castle of Ert.1", "a45b439bb3a9c8a4a14b996024222068", 198983), // alt version
	FANGAMEND("Death Mall", "Death Mall Demo", "1c78fc15fb037b242a0bc6bac7d4d889", 254874),
	FANGAME("Deep Angst", "7f8821f7b279269a91f9aadfed98eec0", 329550), // Original gile name "Deep Angst™"
	FANGAME("Deep Ennui", "7fa4368834a22a9d4b7246a6297b455f", 86075),
	// Polygons with ignored byte 1
	FANGAME("Double Trouble", "3f0c032377d87704267283380800633a", 542371),
	BIGGAME("drakmythcastle", "disk I", "Drakmyth Castle disk I of II", "5b1fd760fbc081c608acebfe1d07a58a", 793784),
	BIGGAME("drakmythcastle", "disk II", "Drakmyth Castle II", "1116f9c2c781f79e1f9c868b51ae7fa5", 1685659),
	// Crash at start in GUI rendering
	FANGAME("Dune Eternity", "6b29f82e235815ffc4c9f30dc09968dd", 290201), // Original file name is "***DUNE ETERNITY*** "
	FANGAMEN("Dungeon World II", "DungeonWorld2", "753df07166ca48e303d782cc72dd4053", 230199),
	// Made for bigger resolution
	FANGAME("Dynasty of Dar", "b2e9a5cca28acb85617b1477a5fca3e2", 275693),
	FANGAME("Edg's World", "0a3a3aaa36088c514b668f1f62120006", 106769),
	FANGAME("Eidisi I", "3d778c0fe7addf5f29e7593ba0fd3953", 172552),
	FANGAME("Eidisi I", "8c2fb325a49344568c5536bba36a2556", 172566), // alt version
	// Problems(?) with text on the first screen
	FANGAMEN("Enchanted Pencils", "Enchanted Pencils 0.99 (PG)", "9a9777a83e58bebfa6f1662d5e236384", 408913),
	FANGAME("Escape!", "3ada261c2d1d9ce6b9da068237472689", 65075), // Original file name "Escape!†"
	FANGAME("Escape from School!", "2055747bb874052333190eb993246a7f", 50105),
	FANGAME("Escape from School!", "fcc581e52d1fc8ea4603d7c953fa935a", 50119), // Original file name "Escape from School!†"
	FANGAME("Everyman 1", "e20cebf0091a1b1bf023aac6f28c9011", 335705),
	FANGAME("Exploration Zeta!", "6127d9c04ad68f0cbb5f6aa1d95b48a2", 366599),
	// Cannot proceed past the first scene
	FANGAMEND("Explorer", "Explorer DEMO", "a9ebdecf6c8de95a03e593d877dacc13", 461228),
	// Crash in console rendering on the first scene
	FANGAME("Fantasy Quest", "b42b0e86e2c84464283640c74b25e015", 762754),
	FANGAME("Find the Heart", "aa244c15f2ba8cef468714be34223acd", 106235), // From Joshua's Worlds 1.0
	FANGAME("Find the Heart", "a6834cb230cea1953f5bf1f8f7aacabd", 105885), // From Joshua's Worlds. Alt version
	FANGAME("Find the Heart", "a6834cb230cea1953f5bf1f8f7aacabd", 105871), // Standalone
	FANGAMEN("Fortune Teller", "Fortune Teller 1.1", "7d2628eeea67b33379e01c0aef8dd196", 73931),
	FANGAMEN("Haunted House", "Haunted House 1.5", "5db2f95c7abaa9d060b94271a5bc57f8", 177500),
	// Cropped graphics on first scene, cannot pass to in-game
	FANGAME("Intro to Gothic", "6f732eaad6e3b85795f8ee6c6a40d837", 208067),
	// Lots of unhandled comparisons
	FANGAME("Jamie the Demon Slayer", "fa0ca9618c18425b6d9bf913f762d91b", 232789),
	FANGAMEN("Journey", "The Journey 1.6.2 US", "e66f37472e1414a088eb5d5acc4df794", 820572),
	FANGAMEN("Jumble", "LSJUMBLE", "7c46851d2f90c7da9efe40b1688869c2", 647339), // Original file name is "LSJUMBLE† "
	FANGAME("Karth of the Jungle", "5f2346834821dc3c4008e139cd37b3cb", 96711),
	FANGAME("Karth of the Jungle", "444f9426f342135fbcc32180e5ba5b1c", 96960), // Alternative version
	FANGAME("Karth of the Jungle II", "32161b27de894fd9e3f054afc4013f34", 201053),
	FANGAMEN("Little Pythagoras", "Little Pythagoras 1.1.1", "75906fa955de695ac3e8164e7d88ac7b", 628821),
	FANGAME("Lost Crystal", "d5e27a83f2884a24c6ec26c6cb776fe9", 771072),
	FANGAMEN("Lost In Kookyville", "Lost In Kookyville 1.2.4", "5ab6259706b33230dbfba05618c2c5c9", 721569),
	FANGAME("Magic Rings", "450e986694b96f3b9e6cc64e57b753dc", 109044),
	// No way to click on the house
	FANGAME("Messy House", "705df61da9e7d742b7ad678e59eb7bfb", 177120),
	FANGAME("Midnight Snack", "76986389f9a08dd95450c8b9cf408653", 67952),
	FANGAME("Midnight Snack", "76986389f9a08dd95450c8b9cf408653", 67966), // Alt version
	FANGAME("Mike's House", "3d23c2b88cefd958bcbc4d4c711003d8", 87357),
	FANGAME("Minitorian", "15fbb2bd75d83155ed21edbc5dc9558f", 586464),
	FANGAME("M'Lord's Warrior", "0bebb2c62529c89590f6c5be6e1e9838", 465639), // Original file name is "M'Lord's Warrior †"
	// Unhandled comparison case
	FANGAME("Mountain of Mayhem", "4088fc534042081b7ab7b49675ab4a6e", 750003), // Original file name "Mountain of Mayhem †"
	// No way to pass through the first screen
	FANGAME("Nightcrawler Ned", "0cf27bf82de299c69405f7910146bf00", 366542),
	// Crash on startup
	FANGAMEN("Parrot Talk", "PARROT TALK V1", "b1570b0779891d5d50a3cf0146d28202", 118936),
	// Crash on startup
	FANGAMEN("Parrot Talk", "PARROT TALKV2", "0c1e920ed3ff74b8f22eaaf0d3496d5a", 118884),
	FANGAME("Pavilion", "3a33149569325a44d98544452323c819", 231687),
	FANGAMEN("Pencils", "Pencils.99", "9c200938488565080e12989e784586e2", 408551),
	// Polygons with byte 1
	FANGAME("Periapt", "fb4052819126b88d7e03ebc00c669a9d", 406006),
	FANGAME("Psychotic!", "6b4ae6261b405e2feac58c5a2ddb67c5", 247693),
	FANGAME("Puzzle Piece Search", "6b4ae6261b405e2feac58c5a2ddb67c5", 247693), // From Joshua's Worlds 1.0
	FANGAME("The Puzzle Piece Search", "fb99797c429c18ec68418fdd12af17a1", 247338), // From Joshua's Worlds
	FANGAME("The Puzzle Piece Search", "fb99797c429c18ec68418fdd12af17a1", 247324), // Stnadalone
	// Empty(?)  first scene
	FANGAME("Pyramid of No Return", "38383ac85cc16703f13f8d82f1398123", 385145),
	// Cropped graphics at the first scene
	FANGAME("Psychotic!", "29b30e6aae9cc6db5eccb09f695ff25e", 367309),
	FANGAME("P-W Adventure", "9bf86fb946683500d23887ef185726ab", 219216),
	FANGAMEN("Pyramid of Ert", "Pyramid of Ert V1.2", "fb931cd35440a66864a434c773b496da", 315783),
	FANGAME("Queen Quest", "8273e29afe64a984eb0ce7b43fdf3a59", 57039), // alt version
	FANGAME("Quest for T-Rex", "f16f2cd525c9aeb4733295d8d842b902", 592584),
	FANGAME("Quest for the Dark Sword", "4815d9a770904b26c463b7e4fcd121c7", 572576),
	FANGAME("Radical Castle", "09b70763c7a48a76240bd0e42737caaa", 355601),
	FANGAME("Radical Castle 1.0", "8ae2e29ffeca52a5c7fae66dec4764a3", 347278),
	BIGGAME("raysmaze", "v1.5", "Ray's Maze1.5", "521583e59bdc1d611f963cef1dc25869", 1408516),
	BIGGAME("raysmaze", "v1.5/alt", "Ray's Maze1.5", "120e65bec953b981b2e0aed45ad45d70", 1408516),
	FANGAME("Ray's World Builder Demo", "d252ee8e38c9abc50455d071a367d031", 116056),
	// Unhandled comparison case
	FANGAME("Sands of Time", "b00ea866cb04cd87124e5720bc2c84c7", 122672), // Original file name "Sands of Time†"
	BIGGAME("scepters", "", "Scepters", "f8db17cd96be056cf8a8bb9cfe46cf3a", 346595),
	BIGGAME("scepters", "", "Scepters", "1fd7ca93ef16f4752fb46ee9cfa0949a", 347540), // alt version
	FANGAME("Schmoozer", "e0f416bae626e2c638055b7f495d8c78", 221500),
	// ??? problems with dog bitmap?
	FANGAMEN("Space Adventure", "SpaceAdventure", "7b6c883b3510e21cfabf4c8caaeb1f16", 155356),
	FANGAMEN("Space Adventure", "SpaceAdventure", "3bd6fc9327f35db5390a9bf86afcd872", 155356), // alt version
	FANGAMEN("Spear of Destiny", "SpearOfDestiny", "f1252ff34dd279f4ec1844bb403a578c", 333665), // Original file name "SpearOfDestiny†"
	FANGAME("Star Trek", "fe20d06bc50c7fcebda0db533e141d4a", 53320),
	FANGAME("Strange Disappearance", "782fae517f7374cd7f43f428331ce445", 772282),
	// Code 0x03 in text
	FANGAME("Swamp Witch", "4f146c0a5c59e7d4717a0423271fa89d", 739781), // Original file name "Swamp Witch†"
	FANGAME("Sweetspace Now!", "1d419bc0b04c51468ddc40a90125bf00", 123813), // Comes with Jumble
	// Wrong scrolling in the first console text
	FANGAMEN("Sword of Siegfried", "Sword of Siegfried 1.0", "1ee92830690f89ea142ac0847176a0c3", 234763),
	FANGAME("Terrorist", "68208fa5e426312fb12402894add5e4a", 524469), // Original file name "Terrorist†"
	FANGAME("Time Bomb", "b7a369d57d43ec8d9fd53832fd38d7db", 64564),
	FANGAME("Time Bomb", "b7a369d57d43ec8d9fd53832fd38d7db", 64578), // Alt version
	FANGAMEND("The Ashland Revolution", "The Ashland Revolution Demo", "3c7a1bdeab48a077a4f54fe69da61a9f", 145023), // Original file name "The Ashland Revolution Demo†"
	FANGAME("The Axe-orcist", "bfdf6a4ce87e6b368977af3b683466db", 308764),
	FANGAMEN("The Hotel Caper", "The Hotel Caper V1.0", "0d11a6ca1357e27ffff5231fe89cc429", 231969),
	FANGAMEN("The Hotel Caper", "The Hotel Caper V1.0", "6c80fa6a36d16aa0edef86d8800c90db", 231969), // alt version
	// Invalid rect in scene "Access Tube 1"
	FANGAMEN("The Phoenix v1.2", "The Phoenix", "0a4a01b83c993408ae824cc4f63957ea", 431640),
	FANGAME("The Phoenix", "0a4a01b83c993408ae824cc4f63957ea", 431643),
	FANGAME("The Sultan's Palace", "98c845323489344d7e2c9d1c3e53d1fc", 456855),
	// Admission for on 3rd screen is messed up
	FANGAME("The Tower", "135fe861928d15b5acd8b355460c54bf", 556539),
	// Polygons with ignored byte 1 and 2 on second scene
	FANGAME("The Village", "b9b5cfbfc7f482eae7587b55edc135ed", 314828),
	FANGAME("The Wizard's Apprentice", "7eff3cb7d1a3f59639c62cf196039745", 782824),
	// Messed up first scene
	FANGAMEND("Tombworld", "Demo TombWorld", "f7c86166e29fb8b57f7a1400d4963a4e", 664252), // Original file name "Demo TombWorld©"
	// Doesn't go past first scene
	BIGGAME("twisted", "", "Twisted! 1.6", "97ab265eddf0cfed6d43d062c853cbc0", 960954),
	FANGAME("Volcano II", "4dbb7ec6111c0f872da8ed8ba14763c9", 82991), // Original file name "Volcano II†"
	FANGAME("Wishing Well", "ece06c419cbb2d32941e6b5c7d9d7c1a", 103688),
	FANGAME("Wizard's Warehouse", "ee1b86841583e2b58ac39bf97017dc7b", 159748),
	FANGAMEN("Wizard's Warehouse 2", "WizWarehouse 2.0", "6502bd974fe149fe76d6d5ae9d1e6878", 230870),
	FANGAME("ZikTuria", "1b934fca68d633d231dccd2047d2d274", 52972),
	FANGAME("Zoony", "7bb293b81117cbd974ce54fafa06f258", 154990), // original filename "Zoony™"

	AD_TABLE_END_MARKER
};

} // End of namespace Wage
