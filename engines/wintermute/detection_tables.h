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

namespace Wintermute {

#define GAMEOPTION_SHOW_FPS GUIO_GAMEOPTIONS1
#define GAMEOPTION_BILINEAR GUIO_GAMEOPTIONS2

static const PlainGameDescriptor wintermuteGames[] = {
	{"5ld",             "Five Lethal Demons"},
	{"5ma",             "Five Magical Amulets"},
	{"actualdest",      "Actual Destination"},
	{"agustin",         "Boredom of Agustin Cordes"},
	{"alimardan1",      "Alimardan's Mischief"},
	{"alimardan2",      "Alimardan Meets Merlin"},
	{"alphapolaris",    "Alpha Polaris"},
	{"apeiron",         "Apeiron"},
	{"artofmurder1",    "Art of Murder 1: FBI Confidential"},
	{"basisoctavus",    "Basis Octavus"},
	{"bickadoodle",     "Bickadoodle"},
	{"bookofgron",      "Book of Gron Part One"},
	{"bthreshold",      "Beyond the Threshold"},
	{"carolreed4",      "Carol Reed 4 - East Side Story"},
	{"carolreed5",      "Carol Reed 5 - The Colour of Murder"},
	{"carolreed6",      "Carol Reed 6 - Black Circle"},
	{"carolreed7",      "Carol Reed 7 - Blue Madonna"},
	{"carolreed8",      "Carol Reed 8 - Amber's Blood"},
	{"carolreed9",      "Carol Reed 9 - Cold Case Summer"},
	{"carolreed10",     "Carol Reed 10 - Bosch's Damnation"},
	{"carolreed11",     "Carol Reed 11 - Shades Of Black"},
	{"carolreed12",     "Carol Reed 12 - Profound Red"},
	{"carolreed13",     "Carol Reed 13 - The Birdwatcher"},
	{"carolreed14",     "Carol Reed 14 - The Fall Of April"},
	{"chivalry",        "Chivalry is Not Dead"},
	{"conspiracao",     "Conspiracao Dumont"},
	{"corrosion",       "Corrosion: Cold Winter Waiting"},
	{"darkfallls",      "Dark Fall: Lost Souls"},
	{"deadcity",        "Dead City"},
	{"dfafadventure",   "DFAF Adventure"},
	{"drbohus",         "Dr. Bohus"},
	{"dreamcat",        "Dreamcat"},
	{"dreaming",        "Des Reves Elastiques Avec Mille Insectes Nommes Georges"},
	{"driller",         "The Driller Incident"},
	{"dirtysplit",      "Dirty Split"},
	{"dreamscape",      "Dreamscape"},
	{"erinmyers",       "The Death of Erin Myers"},
	{"escapemansion",   "Escape from the Mansion"},
	{"everydaygray",    "Everyday Grey"},
	{"facenoir",        "Face Noir"},
	{"findinghope",     "Finding Hope"},
	{"four",            "Four"},
	{"framed",          "Framed"},
	{"ghostsheet",      "Ghost in the Sheet"},
	{"hamlet",          "Hamlet or the last game without MMORPG features, shaders and product placement"},
	{"helga",           "Helga Deep In Trouble"},
	{"jamesperis",      "James Peris: No License Nor Control"},
	{"knossos",         "K'NOSSOS"},
	{"kulivocko",       "Kulivocko"},
	{"lifein3minutes",  "Life In 3 Minutes"},
	{"lonelyrobot",     "Project Lonely Robot"},
	{"looky",           "Looky"},
	{"julia",           "J.U.L.I.A."},
	{"juliastars",      "J.U.L.I.A.: Among the Stars"},
	{"juliastarshd",    "J.U.L.I.A.: Among the Stars HD"},
	{"juliauntold",     "J.U.L.I.A.: Untold"},
	{"mentalrepairs",   "Mental Repairs Inc"},
	{"mirage",          "Mirage"},
	{"nighttrain",      "Night Train"},
	{"oknytt",          "Oknytt"},
	{"one",             "One"},
	{"onehelluvaday",   "One Helluva Day"},
	{"openquest",       "Open Quest"},
	{"paintaria",       "Paintaria"},
	{"palladion",       "Palladion"},
	{"pigeons",         "Pigeons in the Park"},
	{"pizzamorgana",    "Pizza Morgana: Episode 1 - Monsters and Manipulations in the Magical Forest"},
	{"projectdoom",     "Project: Doom"},
	{"projectjoe",      "Project Joe"},
	{"rebeccacarlson1", "Rebecca Carlson Mystery 01 - Silent Footsteps"},
	{"reptilesquest",   "On the Tracks of Dinosaurs"},
	{"reversion1",      "Reversion: The Escape"},
	{"reversion2",      "Reversion: The Meeting"},
	{"rhiannon",        "Rhiannon: Curse of the four Branches"},
	{"ritter",          "1 1/2 Ritter: Auf der Suche nach der hinreissenden Herzelinde"},
	{"rosemary",        "Rosemary"},
	{"satanandsons",    "Satan and Sons"},
	{"securanote",      "Securanote"},
	{"shaban",          "Shaban"},
	{"shinestar",       "The Shine of a Star"},
	{"spaceinvaders",   "Space Invaders"},
	{"spacemadness",    "Space Madness"},
	{"sofiasdebt",      "Sofia's Debt"},
	{"tanya1",          "Tanya Grotter and the Magical Double Bass"},
	{"tanya2",          "Tanya Grotter and the Disappearing Floor"},
	{"theancientmark1", "The Ancient Mark - Episode 1"},
	{"thebox",          "The Box"},
	{"thekite",         "The Kite"},
	{"tib",             "Fairy Tales About Toshechka and Boshechka"},
	{"tradestory",      "The Trader of Stories"},
	{"twc",             "the white chamber"},
	{"war",             "War"},
	{"vsevolod",        "Vsevolod"},
	{"wintermute",      "Wintermute engine game"},
	{"wmedemo",         "Wintermute Engine Technology Demo"},
	{"wtetris",         "Wilma Tetris"},
	{"zilm",            "Zilm: A Game of Reflex"},
	{"zbang",           "Zbang! The Game"},
	{0, 0}
};

// Duplicates WME_ENTRY1s, for consistency
#define WME_ENTRY1s(f1, h1, s1) { {f1, 0, h1, s1}, AD_LISTEND }
#define WME_ENTRY2s(f1, h1, s1, f2, h2, s2) { {f1, 0, h1, s1}, {f2, 0, h2, s2}, AD_LISTEND }
#define WME_ENTRY3s(f1, h1, s1, f2, h2, s2, f3, h3, s3) { {f1, 0, h1, s1}, {f2, 0, h2, s2}, {f3, 0, h3, s3}, AD_LISTEND }

#define WME_PLATENTRY(shortName, extraName, hashEntry, lang, plat, status, version) \
	{ \
		{ \
			shortName, \
			extraName, \
			hashEntry, \
			lang, \
			plat, \
			status, \
			GUIO0(), \
		}, \
		version \
	}

// Convenience variant, as most of the games are Windows-games
#define WME_WINENTRY(shortName, extraName, hashEntry, lang, status, version) \
	{ \
		{ \
			shortName, \
			extraName, \
			hashEntry, \
			lang, \
			Common::kPlatformWindows, \
			status, \
			GUIO0(), \
		}, \
		version \
	}

/* To add new entries:
 * Make sure you have a target name defined at the top of the file
 *
 * If the game has only one language, and can be detected using only one file,
 * then use WME_WINENTRY, with WME_ENTRY1s as exemplified below.
 *
 * If the game has more than one language, and the main data file is common across
 * the versions, then you should use WME_WINENTRY with WME_ENTRY2s/WME_ENTRY3s, with
 * the language file as the first hit, and the data file as the second. (Make sure to
 * NOT create a WME_ENTRY1s matching the same data file as the 2/3 file match)
 */

static const WMEGameDescription gameDescriptions[] = {
	// Five Lethal Demons (Czech)
	WME_WINENTRY("5ld", "",
		WME_ENTRY2s("czech.dcp", "9021b7a1e154d6764228116f894b213b", 186,
					"data.dcp", "1037a77cbd001e0644898addc022322c", 15407750), Common::CZ_CZE, ADGF_UNSTABLE, WME_1_6_1),

	// Five Lethal Demons (English)
	WME_WINENTRY("5ld", "",
		WME_ENTRY2s("english.dcp", "7eab045218855c9c4454c7ada7d70ef4", 170340,
					"data.dcp", "1037a77cbd001e0644898addc022322c", 15407750), Common::EN_ANY, ADGF_UNSTABLE, WME_1_6_1),

	// Five Lethal Demons (Polish)
	WME_WINENTRY("5ld", "",
		WME_ENTRY2s("polish.dcp", "726e279fe3c2dc81a9241408d3585c81", 173792,
					"data.dcp", "1037a77cbd001e0644898addc022322c", 15407750), Common::PL_POL, ADGF_UNSTABLE, WME_1_6_1),

	// Five Magical Amulets (Czech)
	WME_WINENTRY("5ma", "",
		WME_ENTRY2s("czech.dcp", "7b2515a8ceb955c72bc14f0f1fca869e", 184,
					"data.dcp", "0134e92bcd5fd2837df3971087e96067", 163316498), Common::CZ_CZE, ADGF_UNSTABLE, WME_1_3_3),

	// Five Magical Amulets (English)
	WME_WINENTRY("5ma", "",
		WME_ENTRY2s("english.dcp", "2f97bca09260ba23b645da9f0855ce7f", 893681,
					"data.dcp", "0134e92bcd5fd2837df3971087e96067", 163316498), Common::EN_ANY, ADGF_UNSTABLE, WME_1_3_3),

	// Five Magical Amulets (German)
	WME_WINENTRY("5ma", "",
		WME_ENTRY2s("german.dcp", "bfa74aae81672803d0d0748ac0a532b7", 885150,
					"data.dcp", "0134e92bcd5fd2837df3971087e96067", 163316498), Common::DE_DEU, ADGF_UNSTABLE, WME_1_3_3),

	// Five Magical Amulets (Polish)
	WME_WINENTRY("5ma", "",
		WME_ENTRY2s("polish.dcp", "bb877d48795471a17f25b0b5109100d1", 1132197,
					"data.dcp", "0134e92bcd5fd2837df3971087e96067", 163316498), Common::PL_POL, ADGF_UNSTABLE, WME_1_3_3),

	// Actual Destination
	WME_WINENTRY("actualdest", "",
		WME_ENTRY1s("data.dcp", "6926f44b26f21ceb1d840eaab9aeb510", 9081740), Common::EN_ANY, ADGF_UNSTABLE, WME_1_8_11),

	// Alimardan's Mischief (Steam, August 2017)
	WME_WINENTRY("alimardan1", "",
		WME_ENTRY2s("language.dcp", "95d5ce5de1bad7cf2db67a934c36f5cb", 660483,
					"data.dcp", "ef27d8483d6f19d5398fc81ea05e5320", 1258766332), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Alimardan's Mischief (Steam, October 2017)
	WME_WINENTRY("alimardan1", "",
		WME_ENTRY2s("language.dcp", "650077b587f2ee42478b3a545a0f55d2", 660483,
					"data.dcp", "d74f56d9ce3b7442b921393af7fb6167", 1258761838), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Alimardan Meets Merlin
	WME_WINENTRY("alimardan2", "",
		WME_ENTRY2s("language.dcp", "0cbb7e1e84a342782c24938af4da1402", 686580,
					"data.dcp", "e3ffb5d177737a57bb391995d9361055", 413397623), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Alpha Polaris (English) (1.1.0) (1280x800 Steam)
	// NOTE: This is a 2.5D game that is out of ScummVM scope
	WME_WINENTRY("alphapolaris", "1.1.0 (1280x800 Steam)",
		WME_ENTRY2s("data.dcp", "2d1bd35749b2538915b53ce79288bceb", 706580987,
					"data.dcp", "2d1bd35749b2538915b53ce79288bceb", 706580987), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Alpha Polaris (German) (1.1.0) (1280x800 Steam)
	// NOTE: This is a 2.5D game that is out of ScummVM scope
	WME_WINENTRY("alphapolaris", "1.1.0 (1280x800 Steam)",
		WME_ENTRY2s("data.dcp", "2d1bd35749b2538915b53ce79288bceb", 706580987,
					"german_language_pack.dcp", "0ed4ef1647445c73b5915b60f85ed8e4", 19251966), Common::DE_DEU, ADGF_UNSTABLE, WME_1_9_3),

	// Alpha Polaris (Polish) (1.1.0) (1280x800 Steam)
	// NOTE: This is a 2.5D game that is out of ScummVM scope
	WME_WINENTRY("alphapolaris", "1.1.0 (1280x800 Steam)",
		WME_ENTRY2s("data.dcp", "2d1bd35749b2538915b53ce79288bceb", 706580987,
					"polish_language_pack.dcp", "91f80c5f8d522541d666d11b60b0ea6c", 15006039), Common::PL_POL, ADGF_UNSTABLE, WME_1_9_3),

	// Alpha Polaris (Russian) (1.1.0) (1280x800 Steam)
	// NOTE: This is a 2.5D game that is out of ScummVM scope
	WME_WINENTRY("alphapolaris", "1.1.0 (1280x800 Steam)",
		WME_ENTRY2s("data.dcp", "2d1bd35749b2538915b53ce79288bceb", 706580987,
					"russian_language_pack.dcp", "58575db652d371af537b4b8841e962f8", 49395113), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_3),

	// Alpha Polaris (English) (1.1.0) (1280x768 Steam)
	// NOTE: This is a 2.5D game that is out of ScummVM scope
	WME_WINENTRY("alphapolaris", "1.1.0 (1280x768 Steam)",
		WME_ENTRY2s("data.dcp", "481b1bff44178ef2f3d879df05f65a96", 706580987,
					"data.dcp", "481b1bff44178ef2f3d879df05f65a96", 706580987), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Alpha Polaris (German) (1.1.0) (1280x768 Steam)
	// NOTE: This is a 2.5D game that is out of ScummVM scope
	WME_WINENTRY("alphapolaris", "1.1.0 (1280x768 Steam)",
		WME_ENTRY2s("data.dcp", "481b1bff44178ef2f3d879df05f65a96", 706580987,
					"german_language_pack.dcp", "2ffd9b2bdf2c2b3646067644572390c0", 19251966), Common::DE_DEU, ADGF_UNSTABLE, WME_1_9_3),

	// Alpha Polaris (Polish) (1.1.0) (1280x768 Steam)
	// NOTE: This is a 2.5D game that is out of ScummVM scope
	WME_WINENTRY("alphapolaris", "1.1.0 (1280x768 Steam)",
		WME_ENTRY2s("data.dcp", "481b1bff44178ef2f3d879df05f65a96", 706580987,
					"polish_language_pack.dcp", "4a24986189321f39b9f48cbc4889d89a", 15006039), Common::PL_POL, ADGF_UNSTABLE, WME_1_9_3),

	// Alpha Polaris (Russian) (1.1.0) (1280x768 Steam)
	// NOTE: This is a 2.5D game that is out of ScummVM scope
	WME_WINENTRY("alphapolaris", "1.1.0 (1280x768 Steam)",
		WME_ENTRY2s("data.dcp", "481b1bff44178ef2f3d879df05f65a96", 706580987,
					"russian_language_pack.dcp", "f39ad478a711fa4b34d419ed4aac97bf", 49395113), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_3),

	// Apeiron
	WME_WINENTRY("apeiron", "",
		WME_ENTRY1s("data.dcp", "c0d6190de52b3da9995cbd4b78976bc8", 89912566), Common::SE_SWE, ADGF_UNSTABLE, WME_1_9_1),

	// Art of Murder: FBI Confidential (English)
	// NOTE: This is a 2.5D game that is out of ScummVM scope
	WME_WINENTRY("artofmurder1", "",
		WME_ENTRY2s("data.dcp", "7e4c1dc8b1fb08541f7784d6288acfc8", 633692059,
					"us.dcp", "4a02b65edc45444ac69ff8a514e952b8", 176592548), Common::EN_ANY, ADGF_UNSTABLE, WME_1_8_1),

	// Art of Murder: FBI Confidential (French)
	// NOTE: This is a 2.5D game that is out of ScummVM scope
	WME_WINENTRY("artofmurder1", "",
		WME_ENTRY2s("data.dcp", "bba4e56a66fd6406a26515cfc86ac125", 23782002,
					"fr.dcp", "5665d84d70cb3e20472e1c3eb9d884c3", 125728694), Common::FR_FRA, ADGF_UNSTABLE, WME_1_8_1),

	// Art of Murder: FBI Confidential (German)
	// NOTE: This is a 2.5D game that is out of ScummVM scope
	WME_WINENTRY("artofmurder1", "",
		WME_ENTRY2s("data.dcp", "09e57d77b68dafa73a0924f11f61b059", 630742753,
					"i18n.dcp", "583940f6d3fb4097e7cb1e2cc9a43a7b", 156078991), Common::DE_DEU, ADGF_UNSTABLE, WME_1_8_1),

	// Art of Murder: FBI Confidential (Spanish)
	// NOTE: This is a 2.5D game that is out of ScummVM scope
	WME_WINENTRY("artofmurder1", "",
		WME_ENTRY2s("data.dcp", "340f04f9f66a0ab978b78d317544bbed", 23757600,
					"es.dcp", "47b3a609993b3c18ce5bfb1af734ea3e", 148997124), Common::ES_ESP, ADGF_UNSTABLE, WME_1_8_1),

	// Art of Murder: FBI Confidential (Polish)
	// NOTE: This is a 2.5D game that is out of ScummVM scope
	WME_WINENTRY("artofmurder1", "",
		WME_ENTRY2s("data.dcp", "592862a5dd1ae90d53561815a535ab2e", 630734278,
					"i18n.dcp", "b43bd7b64991ad9d5d285753767fe3f4", 176591837), Common::PL_POL, ADGF_UNSTABLE, WME_1_8_1),

	// Basis Octavus
	// NOTE: This is a 2.5D game that is out of ScummVM scope
	WME_WINENTRY("basisoctavus", "",
		WME_ENTRY1s("data.dcp", "021ef97f8f49ec33f83beae0d6e38f08", 49336909), Common::CZ_CZE, ADGF_UNSTABLE, WME_1_9_1),

	// Boredom of Agustin Cordes
	WME_WINENTRY("agustin", "",
		WME_ENTRY1s("data.dcp", "abb79c16c9b92e9b06525a4c7c3f5861", 2461949), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Beyond the Threshold
	WME_WINENTRY("bthreshold", "",
		WME_ENTRY1s("data.dcp", "d49bf9ccb2e74507447c82d6ad3e2bc4", 12773712), Common::EN_ANY, ADGF_UNSTABLE, WME_1_7_94),

	// Bickadoodle
	WME_WINENTRY("bickadoodle", "",
		WME_ENTRY1s("data.dcp", "84db4d1594cac95e25614985775d10a8", 35303844), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// Bickadoodle (Ver 1.1)
	WME_WINENTRY("bickadoodle", "Version 1.1",
		WME_ENTRY1s("data.dcp", "8bb52ac9a9ee129c5059e8e808b669d7", 35337760), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// Bickadoodle (Ver 1.2)
	WME_WINENTRY("bickadoodle", "Version 1.2",
		WME_ENTRY1s("data.dcp", "1796a48f3ed72dd785ce93334ab883cc", 35337760), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// Bickadoodle (download from http://aethericgames.com/games/bickadoodle/download-bickadoodle/)
	WME_WINENTRY("bickadoodle", "",
		WME_ENTRY1s("data.dcp", "1584d83577c32add0fce27fae91141a2", 35337728), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// Book of Gron Part One
	WME_WINENTRY("bookofgron", "",
		WME_ENTRY1s("data.dcp", "e61b2ebee044a82fa0f8ca0fce2c8946", 83129531), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_1),

	// Carol Reed 4 - East Side Story (English)
	WME_WINENTRY("carolreed4", "",
		WME_ENTRY1s("data.dcp", "b26377797f060afc2d440d820100c1ce", 529320536), Common::EN_ANY, ADGF_UNSTABLE, WME_1_7_2),

	// Carol Reed 4 - East Side Story (Russian)
	WME_WINENTRY("carolreed4", "",
		WME_ENTRY1s("data.dcp", "549e6fb7afebcc4a8092e3898a5c5cc3", 618787625), Common::RU_RUS, ADGF_UNSTABLE, WME_1_8_3),

	// Carol Reed 4 - East Side Story (Demo) (English)
	WME_WINENTRY("carolreed4", "Demo",
		WME_ENTRY1s("data.dcp", "b3f8b09bb4b05ee3e9d14697525257f9", 59296246), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_7_2),

	// Carol Reed 5 - The Colour of Murder
	WME_WINENTRY("carolreed5", "",
		WME_ENTRY1s("data.dcp", "3fcfca44209545d0e26774156427b494", 603660415), Common::EN_ANY, ADGF_UNSTABLE, WME_1_8_6),

	// Carol Reed 5 - The Colour of Murder (1.0 Demo)
	WME_WINENTRY("carolreed5", "Demo",
		WME_ENTRY1s("data.dcp", "27b3efc018ade5ee8f4adf08b4e3c0dd", 92019500), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_8_6),

	// Carol Reed 6 - Black Circle
	WME_WINENTRY("carolreed6", "",
		WME_ENTRY1s("data.dcp", "0e4c532beecf23d85012168753f41189", 456258147), Common::EN_ANY, ADGF_UNSTABLE, WME_1_8_11),

	// Carol Reed 6 - Black Circle (Demo)
	WME_WINENTRY("carolreed6", "Demo",
		WME_ENTRY1s("data.dcp", "e18bc4adc843e6dcf08cc20be00d1608", 94399373), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_8_11),

	// Carol Reed 7 - Blue Madonna
	WME_WINENTRY("carolreed7", "",
		WME_ENTRY1s("data.dcp", "24e3db3e2fabfc956713796d87a3efb0", 495471147), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// Carol Reed 7 - Blue Madonna (Demo)
	WME_WINENTRY("carolreed7", "Demo",
		WME_ENTRY1s("data.dcp", "0372ad0c775266f6355e9e8ae397a2f1", 103719442), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_1),

	// Carol Reed 8 - Amber's Blood
	WME_WINENTRY("carolreed8", "",
		WME_ENTRY1s("data.dcp", "859d16b0d5b9b255e470cbded2c6cedc", 502714557), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// Carol Reed 8 - Amber's Blood (Demo)
	WME_WINENTRY("carolreed8", "Demo",
		WME_ENTRY1s("data.dcp", "e8c6b6e5d2e79bc87bf591f47202c233", 110106328), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_1),

	// Carol Reed 9 - Cold Case Summer
	WME_WINENTRY("carolreed9", "",
		WME_ENTRY1s("data.dcp", "2b343b48a7aee508d728a546b414a255", 620005266), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 9 - Cold Case Summer (Demo)
	WME_WINENTRY("carolreed9", "Demo",
		WME_ENTRY1s("data.dcp", "b3312b0b9f209a9a01b2ba9135829640", 86362217), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_3),

	// Carol Reed 10 - Bosch's Damnation
	WME_WINENTRY("carolreed10", "",
		WME_ENTRY1s("data.dcp", "5c698e28fe10854ea52964e3768e50e4", 588439599), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 10 - Bosch's Damnation (Demo)
	WME_WINENTRY("carolreed10", "Demo",
		WME_ENTRY1s("data.dcp", "72b29cb3f0bc83efa6e3bae41b4e899a", 79971080), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_3),

	// Carol Reed 11 - Shades Of Black
	WME_WINENTRY("carolreed11", "",
		WME_ENTRY1s("data.dcp", "a1e9f54b48493e9c6d737e3a94779d8c", 525450069), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 11 - Shades Of Black (Demo)
	WME_WINENTRY("carolreed11", "Demo",
		WME_ENTRY1s("data.dcp", "8c27236d29747b6bb6a0a563fb769f20", 52692061), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_3),

	// Carol Reed 11 - Shades Of Black (HD Demo)
	WME_WINENTRY("carolreed11", "HD Demo",
		WME_ENTRY1s("data.dcp", "ba124af0a0ee3113f3f9dc0c1159ec26", 77909344), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_3),

	// Carol Reed 12 - Profound Red
	WME_WINENTRY("carolreed12", "",
		WME_ENTRY1s("data.dcp", "edc1e69dc52ec2a65fa431a51c783c2e", 944403699), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 13 - The Birdwatcher
	WME_WINENTRY("carolreed13", "",
		WME_ENTRY1s("data.dcp", "954392d19f0c928b700f17badd35d488", 1267391173), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 14 - The Fall Of April
	WME_WINENTRY("carolreed14", "",
		WME_ENTRY1s("data.dcp", "38c9088dca07c9623562e8efdea4c856", 1256960150), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Chivalry is Not Dead
	WME_WINENTRY("chivalry", "",
		WME_ENTRY1s("data.dcp", "ebd0915d9a12df5224be22f53bb23eb6", 7278306), Common::EN_ANY, ADGF_NO_FLAGS, WME_1_8_0),

	// Chivalry is Not Dead (Version from deirdrakai.com)
	WME_WINENTRY("chivalry", "",
		WME_ENTRY1s("data.dcp", "ae6d91b9517f4d2851a8ad94c96951c8", 7278302), Common::EN_ANY, ADGF_NO_FLAGS, WME_1_8_0),

	// Conspiracao Dumont (Demo)
	WME_WINENTRY("conspiracao", "Demo",
		WME_ENTRY1s("ConspiracaoDumont.exe", "106f3f2c8f18bb5ffffeed634ace256c", 32908032), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, LATEST_VERSION),

	// Corrosion: Cold Winter Waiting
	WME_WINENTRY("corrosion", "",
		WME_ENTRY1s("data.dcp", "ae885b1a8faa0b27f43c0e8f0df02fc9", 525931618), Common::EN_ANY, ADGF_NO_FLAGS, WME_1_9_1),

	// Corrosion: Cold Winter Waiting (Enhanced Edition)
	WME_WINENTRY("corrosion", "Enhanced Edition",
		WME_ENTRY1s("data.dcp", "eeeb63910f6fc6ad1fe9c06db0b0ea2c", 643363346), Common::EN_ANY, ADGF_NO_FLAGS, WME_1_9_1),

	// Corrosion: Cold Winter Waiting (Enhanced Edition) (v1.2.0)
	WME_WINENTRY("corrosion", "Enhanced Edition v1.2.0",
		WME_ENTRY2s("data.dcp", "d14d3c858026b1422a2a420e9a49fd16", 643372377,
					"steam_001.dcp", "e5ee8fcadec5dcc24484300c77017bd0", 41986), Common::EN_ANY, ADGF_NO_FLAGS, WME_1_9_1),

	// Corrosion: Cold Winter Waiting (Enhanced Edition) (v1.2.1)
	WME_WINENTRY("corrosion", "Enhanced Edition v1.2.1",
		WME_ENTRY2s("data.dcp", "1d3e4d97f135d61c6532b3bc08272156", 643363379,
					"steam_001.dcp", "c69ca653d0b630302afece19c5e9b52a", 42039), Common::EN_ANY, ADGF_NO_FLAGS, WME_1_9_1),

	// Corrosion: Cold Winter Waiting (Enhanced Edition) (v1.2.2)
	WME_WINENTRY("corrosion", "Enhanced Edition v1.2.2",
		WME_ENTRY2s("data.dcp", "1d3e4d97f135d61c6532b3bc08272156", 643363379,
					"steam_001.dcp", "e10b58c16e9487cfaf395c3b88ca1fd1", 42049), Common::EN_ANY, ADGF_NO_FLAGS, WME_1_9_1),

	// Dark Fall: Lost Souls
	// NOTE: This is a 2.5D game that is out of ScummVM scope
	WME_WINENTRY("darkfallls", "",
		WME_ENTRY2s("actors.dcp", "170c7ade10edd7ff6d24b16f2eecd818", 453328582,
					"stageplay.dcp", "ed67b4d6e35c19597241eb1d863aa315", 226453373), Common::EN_ANY, ADGF_UNSTABLE, WME_1_8_10),

	// Dead City (Czech)
	WME_WINENTRY("deadcity", "",
		WME_ENTRY2s("data.dcp", "7ebfd50d1a22370ed7b079bcaa631d62", 9070205,
					"data.dcp", "7ebfd50d1a22370ed7b079bcaa631d62", 9070205), Common::CZ_CZE, ADGF_UNSTABLE, WME_1_4_1),

	// Dead City (English)
	WME_WINENTRY("deadcity", "",
		WME_ENTRY2s("english.dcp", "c591046d6de7e381d76f70e0787b2b1f", 415935,
					"data.dcp", "7ebfd50d1a22370ed7b079bcaa631d62", 9070205), Common::EN_ANY, ADGF_UNSTABLE, WME_1_4_1),

	// Dead City (Italian)
	WME_WINENTRY("deadcity", "",
		WME_ENTRY2s("italian.dcp", "92d8efb94436bec7bd1b7fe0b548192e", 454037,
					"data.dcp", "7ebfd50d1a22370ed7b079bcaa631d62", 9070205), Common::IT_ITA, ADGF_UNSTABLE, WME_1_4_1),

	// Dead City (Russian)
	WME_WINENTRY("deadcity", "",
		WME_ENTRY2s("russian.dcp", "a0ae71e9e1185596fffb07ad2c951eb9", 653317,
					"data.dcp", "7ebfd50d1a22370ed7b079bcaa631d62", 9070205), Common::RU_RUS, ADGF_UNSTABLE, WME_1_4_1),

	// DFAF Adventure
	WME_WINENTRY("dfafadventure", "",
		WME_ENTRY1s("data.dcp","5704ebef961176f647742aa66bd09352", 10083417), Common::EN_ANY, ADGF_UNSTABLE | GF_LOWSPEC_ASSETS, WME_1_9_1),

	// Dirty Split (Czech)
	WME_WINENTRY("dirtysplit", "",
		WME_ENTRY2s("czech.dcp", "08a71446467cf8f9444cfea446b46ad6", 127697934,
					"data.dcp", "8b4b81b718bf65f30a67fc0b1e329eb5", 88577623), Common::CZ_CZE, ADGF_UNSTABLE, WME_1_8_5),

	// Dirty Split (English)
	WME_WINENTRY("dirtysplit", "",
		WME_ENTRY1s("data.dcp", "8f3dae199361ece0f59fb20cfff6eed3", 88577621), Common::EN_ANY, ADGF_UNSTABLE, WME_1_8_5),

	// Dirty Split (French)
	WME_WINENTRY("dirtysplit", "",
		WME_ENTRY2s("french.dcp", "a0508dedebd0fe478d0158fa4c2a1136", 125534323,
					"data.dcp", "e6d70c7f5d181b761cfcf974adf9186a", 88577623), Common::FR_FRA, ADGF_UNSTABLE, WME_1_8_5),

	// Dirty Split (German)
	WME_WINENTRY("dirtysplit", "",
		WME_ENTRY1s("data.dcp", "139d8a25579e969f8b37d20e6e3de5f9", 92668291), Common::DE_DEU, ADGF_UNSTABLE, WME_1_8_5),

	// Dirty Split (Italian)
	WME_WINENTRY("dirtysplit", "",
		WME_ENTRY2s("italian.dcp", "8108807fbd8af70be1ec452d0fd1131b", 125513726,
					"data.dcp", "35a150e22af274185883fdbb142c6fb1", 88577623), Common::IT_ITA, ADGF_UNSTABLE, WME_1_8_5),

	// Dirty Split (Spanish)
	WME_WINENTRY("dirtysplit", "",
		WME_ENTRY2s("spanish.dcp", "b3982c0a5e85b42e1e38240fef004aa4", 164428596,
					"data.dcp", "63766d6c68b9f00b632ea1736fc8a95c", 88577621), Common::ES_ESP, ADGF_UNSTABLE, WME_1_8_5),

	// Des Reves Elastiques Avec Mille Insectes Nommes Georges
	WME_WINENTRY("dreaming", "",
		WME_ENTRY1s("data.dcp", "4af26d97ea063fc1277ce30ae431de90", 8804073), Common::EN_ANY, ADGF_UNSTABLE, WME_1_8_6),

	// The Driller Incident (English)
	WME_WINENTRY("driller", "",
		WME_ENTRY1s("data.dcp","9cead7a85244263e0a5ff8f69dd7a1fc", 13671792), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// The Driller Incident (Russian)
	WME_WINENTRY("driller", "",
		WME_ENTRY1s("data.dcp","5bec2442339dd1ecf221873fff704617", 13671830), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_1),

	// Dr. Bohus
	WME_WINENTRY("drbohus", "",
		WME_ENTRY1s("data.dcp","a79e640ea15f7ca36addc08ab7b1db49", 59344013), Common::CZ_CZE, ADGF_UNSTABLE, WME_1_2_43),

	// Dreamcat
	WME_WINENTRY("dreamcat", "",
		WME_ENTRY1s("data.dcp","189bd4eef29034f4ff4ed30120eaac4e", 7758040), Common::EN_ANY, ADGF_UNSTABLE | GF_LOWSPEC_ASSETS, WME_1_9_1),

	// Dreamscape (Beta)
	// NOTE: Package is called "Dreamland.rar", however game title is "Dreamscape"
	WME_WINENTRY("dreamscape", "Beta",
		WME_ENTRY1s("data.dcp", "7a5752ed4446c862be9f02d7932acf54", 17034377), Common::EN_ANY, ADGF_UNSTABLE, WME_1_8_11),

	// The Death of Erin Myers
	WME_WINENTRY("erinmyers", "",
		WME_ENTRY1s("data.dcp", "beebf574c043ebeedf23ac8df6e1f988", 245235110), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// Escape from the Mansion
	WME_WINENTRY("escapemansion", "Beta 1",
		WME_ENTRY1s("data.dcp", "d8e348b2312cc36a929cad75f12e0b3a", 21452380), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// Escape from the Mansion
	WME_WINENTRY("escapemansion", "Beta 2",
		WME_ENTRY1s("data.dcp", "ded5fa6c5f2afdaf2cafb53e52cd3dd8", 21455763), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// Escape from the Mansion
	WME_WINENTRY("escapemansion", "1.3",
		WME_ENTRY1s("data.dcp", "1e5d231b56c8a228cd15cb690f50253e", 29261972), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// Everyday Grey (work name was Philip Kane)
	// NOTE: This game seems to be mistakenly marked as 2.5D at Wintermute wiki
	WME_WINENTRY("everydaygray", "",
		WME_ENTRY1s("data.dcp", "77afff4101adbddac07ecf22ffbb8212", 24436150), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// Face Noir
	// NOTE: This is a 2.5D game that is out of ScummVM scope	
	WME_WINENTRY("facenoir", "",
		WME_ENTRY1s("data.dcp", "e162db79d9091faa1d670fc5cdcb4ba9", 555549627), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// Finding Hope (English)
	WME_WINENTRY("findinghope", "",
		WME_ENTRY1s("data.dcp", "d7c3857b622e038781bf42abd3c64962", 110247769), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// Finding Hope (Russian)
	// NOTE: this game distribution contained unpacked game files instead of usual game.dcp package
	WME_WINENTRY("findinghope", "",
		WME_ENTRY1s("default.game", "57db79e9de62b9d1c7a335822f6242ff", 742), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_1),

	// Four
	WME_WINENTRY("four", "",
		WME_ENTRY1s("data.dcp", "ec05cd5e37c9a524053b8859635a4234", 62599855), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// Framed (Beta)
	WME_WINENTRY("framed", "Beta",
		WME_ENTRY1s("data.dcp", "e7259fb36f2c6f9f28242291e0c3de98", 34690568), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_8_11),

	// Ghost in the Sheet
	WME_WINENTRY("ghostsheet", "",
		WME_ENTRY2s("english.dcp", "e6d0aad2c89996bcabe416105a3d6d3a", 12221017,
					"data.dcp", "b2f8b05328e4881e15e98e845b63f451", 168003), Common::EN_ANY, ADGF_UNSTABLE, WME_1_8_0),

	// Ghost in the Sheet (Demo)
	WME_WINENTRY("ghostsheet", "Demo",
		WME_ENTRY1s("data.dcp", "dc1f6595f412ac25a52eaf47dad4ab81", 169083), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_8_0),

	// Hamlet or the last game without MMORPG features, shaders and product placement (English)
	WME_WINENTRY("hamlet", "",
		WME_ENTRY1s("data.dcp", "f624add957a77c9930529fb28cc2450f", 88183022), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// Hamlet or the last game without MMORPG features, shaders and product placement (Steam) (English)
	WME_WINENTRY("hamlet", "v1.03",
		WME_ENTRY1s("data.dcp", "74130d3c13f4a8caa2aafb9ee23f2639", 88184289), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// Hamlet or the last game without MMORPG features, shaders and product placement (Steam) (French)
	WME_WINENTRY("hamlet", "v1.03",
		WME_ENTRY1s("data.dcp", "5facdd153473cd3dafd4c6cfd5c683c6", 88145395), Common::FR_FRA, ADGF_UNSTABLE, WME_1_9_1),

	// Hamlet or the last game without MMORPG features, shaders and product placement (Steam) (German)
	WME_WINENTRY("hamlet", "v1.03",
		WME_ENTRY1s("data.dcp", "fbd9ff90d8cc695b4a1524873792471d", 88204928), Common::DE_DEU, ADGF_UNSTABLE, WME_1_9_1),

	// Hamlet or the last game without MMORPG features, shaders and product placement (Steam) (Hungarian)
	WME_WINENTRY("hamlet", "v1.03",
		WME_ENTRY1s("data.dcp", "8eb59bb531d3dbfe1d6800b2e82f5613", 88284666), Common::HU_HUN, ADGF_UNSTABLE, WME_1_9_1),

	// Hamlet or the last game without MMORPG features, shaders and product placement (Steam) (Italian)
	WME_WINENTRY("hamlet", "v1.03",
		WME_ENTRY1s("data.dcp", "ed16bdedc212c2a754b065ded5d91f00", 88259077), Common::IT_ITA, ADGF_UNSTABLE, WME_1_9_1),

	// Hamlet or the last game without MMORPG features, shaders and product placement (Steam) (Japanese)
	WME_WINENTRY("hamlet", "v1.03",
		WME_ENTRY1s("data.dcp", "06200da35604641c676c363babecb498", 88252762), Common::JA_JPN, ADGF_UNSTABLE, WME_1_9_1),

	// Hamlet or the last game without MMORPG features, shaders and product placement (Steam) (Koreana)
	WME_WINENTRY("hamlet", "v1.03",
		WME_ENTRY1s("data.dcp", "29b8a8fcb4d50533571125be65c0fb93", 88093017), Common::KO_KOR, ADGF_UNSTABLE, WME_1_9_1),

	// Hamlet or the last game without MMORPG features, shaders and product placement (Steam) (Polish)
	WME_WINENTRY("hamlet", "v1.03",
		WME_ENTRY1s("data.dcp", "b1f993e048cded6902630343fbc14fe2", 88295172), Common::PL_POL, ADGF_UNSTABLE, WME_1_9_1),

	// Hamlet or the last game without MMORPG features, shaders and product placement (Steam) (Portuguese)
	WME_WINENTRY("hamlet", "v1.03",
		WME_ENTRY1s("data.dcp", "f55674e17df2816145d3473401081f05", 88245004), Common::PT_POR, ADGF_UNSTABLE, WME_1_9_1),

	// Hamlet or the last game without MMORPG features, shaders and product placement (Steam) (Russian)
	WME_WINENTRY("hamlet", "v1.03",
		WME_ENTRY1s("data.dcp", "22cb24e8d37201e49bd2a76c33a1b98d", 88326328), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_1),

	// Hamlet or the last game without MMORPG features, shaders and product placement (Steam) (Spanish)
	WME_WINENTRY("hamlet", "v1.03",
		WME_ENTRY1s("data.dcp", "262c84ebabc473a678b8e3a18b57fa89", 88332992), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_1),

	// Hamlet or the last game without MMORPG features, shaders and product placement (Steam) (Turkish)
	WME_WINENTRY("hamlet", "v1.03",
		WME_ENTRY1s("data.dcp", "5c9deef7e072f75081165b9c519f5b69", 88239961), Common::TR_TUR, ADGF_UNSTABLE, WME_1_9_1),

	// Helga Deep In Trouble (Czech)
	WME_WINENTRY("helga", "",
		WME_ENTRY2s("data.dcp", "25cb955a60b58326f2eeda1ce288fb37", 183251259,
					"data.dcp", "25cb955a60b58326f2eeda1ce288fb37", 183251259), Common::CZ_CZE, ADGF_UNSTABLE, WME_1_9_3),

	// Helga Deep In Trouble (English)
	WME_WINENTRY("helga", "",
		WME_ENTRY2s("english.dcp", "bfa136b21bdbc7d8691c0770a6d40bc3", 135931,
					"data.dcp", "25cb955a60b58326f2eeda1ce288fb37", 183251259), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Helga Deep In Trouble (Demo) (Czech)
	WME_WINENTRY("helga", "Demo",
		WME_ENTRY2s("data.dcp", "45134ed93bc391edf148b79cdcbf2a09", 154266028,
					"data.dcp", "45134ed93bc391edf148b79cdcbf2a09", 154266028), Common::CZ_CZE, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_3),

	// Helga Deep In Trouble (Demo) (English)
	WME_WINENTRY("helga", "Demo",
		WME_ENTRY2s("english.dcp", "b3a93e678f0ef97200f691cd1724643f", 135864,
					"data.dcp", "45134ed93bc391edf148b79cdcbf2a09", 154266028), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_3),

	// James Peris: No License Nor Control (English)
	WME_WINENTRY("jamesperis", "",
		WME_ENTRY1s("data.dcp", "a420961e170cb7d168a0d2bae2fe5218", 225294032), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// James Peris: No License Nor Control (Spanish)
	WME_WINENTRY("jamesperis", "",
		WME_ENTRY1s("data.dcp", "a420961e170cb7d168a0d2bae2fe5218", 225294032), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_1),

	// James Peris: No License Nor Control (English)
	WME_WINENTRY("jamesperis", "",
		WME_ENTRY1s("data.dcp", "d6049dfb5dbe812bae1e96924a012500", 225299340), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// James Peris: No License Nor Control (Spanish)
	WME_WINENTRY("jamesperis", "",
		WME_ENTRY1s("data.dcp", "d6049dfb5dbe812bae1e96924a012500", 225299340), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_1),

	// James Peris: No License Nor Control (Demo) (English)
	WME_WINENTRY("jamesperis", "Demo",
		WME_ENTRY1s("data.dcp", "edb9f9c7a08993c1e28f4e477b5f9830", 116113507), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_1),

	// James Peris: No License Nor Control (Demo) (Spanish)
	WME_WINENTRY("jamesperis", "Demo",
		WME_ENTRY1s("data.dcp", "edb9f9c7a08993c1e28f4e477b5f9830", 116113507), Common::ES_ESP, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_1),

	// J.U.L.I.A. (English)
	// NOTE: This is a 2.5D game that is out of ScummVM scope, however it is reported as completable with VFX low
	WME_WINENTRY("julia", "",
		WME_ENTRY1s("data.dcp", "c2264b4f8fcd132d2913ff5b6076a24f", 10109741), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// J.U.L.I.A. (English, Bundle in a box-version)
	// NOTE: This is a 2.5D game that is out of ScummVM scope, however it is reported as completable with VFX low
	WME_WINENTRY("julia", "Version 1.2",
		WME_ENTRY1s("data.dcp", "fe90023ccc22f35185b40b910e0d03a2", 10101373), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// J.U.L.I.A. (English) (Demo)
	// NOTE: This is a 2.5D game that is out of ScummVM scope, however it is reported as completable with VFX low
	WME_WINENTRY("julia", "Demo",
		WME_ENTRY1s("data.dcp", "f0bbc3394555a9811f6050dae428cab6", 7655237), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_3),

	// J.U.L.I.A. (English) (Greenlight Demo)
	// NOTE: This is a 2.5D game that is out of ScummVM scope, however it is reported as completable with VFX low
	WME_WINENTRY("julia", "Greenlight Demo",
		WME_ENTRY1s("data.dcp", "4befd448d36b0dae9c3ab1aa7cb8b78d", 7271886), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_3),

	// J.U.L.I.A.: Among the Stars (Steam, January 2017) (English)
	// NOTE: This is a 2.5D game that is out of ScummVM scope
	WME_WINENTRY("juliastars", "",
		WME_ENTRY2s("data_sd.dcp", "9949302dfaea943113e2f0ee0dd468be", 4249680,
					"data_sd.dcp", "9949302dfaea943113e2f0ee0dd468be", 4249680), Common::EN_ANY, ADGF_UNSTABLE, WME_LITE),

	// J.U.L.I.A.: Among the Stars HD (Steam, January 2017) (English)
	// NOTE: This is a 2.5D game that is out of ScummVM scope
	WME_WINENTRY("juliastarshd", "",
		WME_ENTRY2s("data_hd.dcp", "fd579fa333f117882190993ea4f3bba5", 5164463,
					"data_hd.dcp", "fd579fa333f117882190993ea4f3bba5", 5164463), Common::EN_ANY, ADGF_UNSTABLE, WME_LITE),

	// J.U.L.I.A.: Among the Stars (Steam, November 2016) (German)
	// NOTE: This is a 2.5D game that is out of ScummVM scope
	WME_WINENTRY("juliastars", "",
		WME_ENTRY2s("data_sd.dcp", "dfaf7e730a66412f68d11cddb0c8737d", 4505667,
					"german_sd.dcp", "23ceb8625cebfe32aaa5950e89ac68ba", 123326075), Common::DE_DEU, ADGF_UNSTABLE, WME_LITE),

	// J.U.L.I.A.: Among the Stars HD (Steam, November 2016) (German)
	// NOTE: This is a 2.5D game that is out of ScummVM scope
	WME_WINENTRY("juliastarshd", "",
		WME_ENTRY2s("data_hd.dcp", "f40b3d0778e37c61cf309d214446d233", 5264780,
					"german_hd.dcp", "8d85f83a3fc8f1bec4e5ba2158b05b1e", 152499998), Common::DE_DEU, ADGF_UNSTABLE, WME_LITE),

	// J.U.L.I.A.: Untold
	WME_WINENTRY("juliauntold", "",
		WME_ENTRY1s("data.dcp", "fe995e26253f6e0a925dd7850fce17a9", 26459827), Common::EN_ANY, ADGF_UNSTABLE, WME_LITE),

	// K'NOSSOS (Alpha Demo)
	WME_WINENTRY("knossos", "Alpha Demo",
		WME_ENTRY1s("K'NOSSOS.exe", "694e488cc99bb78d2fe906f82d3ee9e1", 867516208), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_3),

	// Kulivocko (Czech)
	WME_WINENTRY("kulivocko", "",
		WME_ENTRY1s("data.dcp", "44306dc470e9b27474043932eccee02f", 155106392), Common::CZ_CZE, ADGF_UNSTABLE, WME_1_9_1),

	// Kulivocko (Czech) (Demo)
	WME_WINENTRY("kulivocko", "Demo",
		WME_ENTRY1s("data.dcp", "63b164bdfadecbb0deb5da691afb8154", 48362234), Common::CZ_CZE, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_1),

	// Life In 3 Minutes
	WME_WINENTRY("lifein3minutes", "",
		WME_ENTRY1s("data.dcp", "c6368950e37a95bf098b02b4eaa5b929", 141787214), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),

	// Looky Demo (English)
	WME_WINENTRY("looky", "Demo",
		WME_ENTRY2s("english.dcp", "1388e1dd320f4d553dea3b0316812f9d", 1358442,
					"data.dcp", "7074bcd7bc7ad7eb04c271aafb964c32", 13815660), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_1),

	// Looky Demo (German)
	WME_WINENTRY("looky", "Demo",
		WME_ENTRY2s("german.dcp", "606c048426dfbe94442b59fd34a5c76e", 14339496,
					"data.dcp", "7074bcd7bc7ad7eb04c271aafb964c32", 13815660), Common::DE_DEU, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_1),

	// Looky (English)
	WME_WINENTRY("looky", "",
		WME_ENTRY2s("english.dcp", "71ed521b7a1d1a23c3805c26f16de2b9", 245968038,
					"data.dcp", "d0f2bb73425db45fcff6690637c430dd", 1342439), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// Looky (German)
	WME_WINENTRY("looky", "",
		WME_ENTRY2s("german.dcp", "bf4c2b8c26342342441a6d64934ab832", 107027865,
					"data.dcp", "50de0beaa5ad621aa9f020df901d1e74", 1342214), Common::DE_DEU, ADGF_UNSTABLE, WME_1_9_1),

	// Mental Repairs Inc (English)
	// NOTE: This is a 2.5D game that is out of ScummVM scope
	WME_WINENTRY("mentalrepairs", "",
		WME_ENTRY2s("data.dcp", "414d423bbff697f22fb38932f030e897", 59518068,
					"english.dcp", "7573eb584e662adbc5fa3b1448e56106", 3160232), Common::EN_ANY, ADGF_UNSTABLE, WME_1_8_6),

	// Mental Repairs Inc (German)
	// NOTE: This is a 2.5D game that is out of ScummVM scope
	WME_WINENTRY("mentalrepairs", "",
		WME_ENTRY2s("data.dcp", "414d423bbff697f22fb38932f030e897", 59518068,
					"german.dcp", "af59a05ef29768e7fced3794a7a380a3", 3249142), Common::DE_DEU, ADGF_UNSTABLE, WME_1_8_6),

	// Mirage
	WME_WINENTRY("mirage", "",
		WME_ENTRY1s("data.dcp", "d230b0b99c0aa77b9ecd094d8ee5573b", 17844056), Common::EN_ANY, ADGF_UNSTABLE, WME_1_8_11),

	// Night Train Demo
	WME_WINENTRY("nighttrain", "Demo",
		WME_ENTRY1s("data.dcp", "5a027ef84b083a730c9a4c85ec1d3a32", 131760816), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_1),

	// Oknytt
	WME_WINENTRY("oknytt", "Version 1.0",
		WME_ENTRY1s("data.dcp", "6456cf8f429905c83f07509f9da536dd", 109502959), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Oknytt (Version 1.12 English) (These are detected along with d_sounds.dcp to avoid mass-detecting in the languages-subfolder.)
	WME_WINENTRY("oknytt", "Version 1.12",
		WME_ENTRY2s("english.dcp", "cea08a6b5c88f40cb9937f77a72dce2e", 293273567,
					"d_sounds.dcp", "8df683174ef01e6f717e2df926fa0b4a", 154943401), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Oknytt (Version 1.12 German)
	WME_WINENTRY("oknytt", "Version 1.12",
		WME_ENTRY2s("german.dcp", "a4f8e76dd6bec15656e83a871e36920c", 304292572,
					"d_sounds.dcp", "8df683174ef01e6f717e2df926fa0b4a", 154943401), Common::DE_DEU, ADGF_UNSTABLE, WME_1_9_3),

	// Oknytt (Version 1.12 Russian)
	WME_WINENTRY("oknytt", "Version 1.12",
		WME_ENTRY2s("russian.dcp", "82e0ae002dd6a4106cbd1e4f8db9cfe0", 362681666,
					"d_sounds.dcp", "8df683174ef01e6f717e2df926fa0b4a", 154943401), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_3),

	// Oknytt (Version 1.13 English) (These are detected along with d_sounds.dcp to avoid mass-detecting in the languages-subfolder.)
	WME_WINENTRY("oknytt", "Version 1.13",
		WME_ENTRY2s("english.dcp", "d2afd722c78cfe66b7d4250d11f6ae16", 293274135,
					"d_sounds.dcp", "7d04dff8ca11174486bd4b7a80fdcabb", 154943401), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Oknytt (Version 1.13 German)
	WME_WINENTRY("oknytt", "Version 1.13",
		WME_ENTRY2s("german.dcp", "0fc6401d8d76b04f6da49206ecafa0dc", 304292574,
					"d_sounds.dcp", "7d04dff8ca11174486bd4b7a80fdcabb", 154943401), Common::DE_DEU, ADGF_UNSTABLE, WME_1_9_3),

	// Oknytt (Version 1.13 Russian)
	WME_WINENTRY("oknytt", "Version 1.13",
		WME_ENTRY2s("russian.dcp", "dd24a1c0b36a82e2b02fb6c1050d4aad", 362681669,
					"d_sounds.dcp", "7d04dff8ca11174486bd4b7a80fdcabb", 154943401), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_3),

	// Oknytt (Version 1.13 Spanish)
	WME_WINENTRY("oknytt", "Version 1.13",
		WME_ENTRY2s("spanish.dcp", "10c46152cb29581671f3b6b7c229c957", 319406572,
					"d_sounds.dcp", "7d04dff8ca11174486bd4b7a80fdcabb", 154943401), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_3),

	// One (Demo)
	WME_WINENTRY("one", "Demo",
		WME_ENTRY1s("data.dcp", "dc2e1682227716099b6acaf5cbce104d", 1676301), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_2_44),

	// One Helluva Day (Demo) (multi-language)
	WME_WINENTRY("onehelluvaday", "Demo",
		WME_ENTRY1s("data.dcp", "144e23fca7c1c54103dad9c1342de2b6", 229963509), Common::UNK_LANG, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_3),

	// Open Quest
	WME_WINENTRY("openquest", "",
		WME_ENTRY1s("data.dcp", "16893e3fc15a211a49654ae66f684f28", 82281736), Common::EN_ANY, ADGF_UNSTABLE | GF_LOWSPEC_ASSETS, WME_1_9_1),

	// Paintaria
	WME_WINENTRY("paintaria", "",
		WME_ENTRY1s("data.dcp", "354c08440c98150ff0d4008dd2865880", 48326040), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// Palladion (Alpha6h)
	// NOTE: This is a 2.5D game that is out of ScummVM scope
	WME_WINENTRY("palladion", "Alpha6h",
		WME_ENTRY1s("Palladion_Alpha6h.exe", "55aec582159410ec6221d4c3fd53db24", 95240860), Common::DE_DEU, ADGF_UNSTABLE | ADGF_DEMO, WME_1_8_6),

	// Pigeons in the Park
	WME_WINENTRY("pigeons", "",
		WME_ENTRY1s("data.dcp", "9143a5b6ff8206aefe3c4c643add3ec7", 2611100), Common::EN_ANY, ADGF_UNSTABLE, WME_1_8_1),

	// Pizza Morgana (Demo)
	// NOTE: This is a 2.5D game that is out of ScummVM scope
	WME_WINENTRY("pizzamorgana", "Demo",
		WME_ENTRY2s("english.dcp", "7fa6149bb44574109668ce585d6c41c9", 9282608,
					"data.dcp", "a69994c463ff5fcc6fe1800662f5b7d0", 34581370), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_1),

	// Project: Doom (Beta)
	WME_WINENTRY("projectdoom", "Beta",
		WME_ENTRY1s("data.dcp", "d5894b65a40706845434b99870bcab92", 99223761), Common::EN_ANY, ADGF_UNSTABLE, WME_1_8_11),

	// Project Joe (Demo)
	WME_WINENTRY("projectjoe", "Demo",
		WME_ENTRY1s("data.dcp", "ada3c08542901295076b5349e655e73f", 160780037), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_2_44),

	// Project Lonely Robot (Beta)
	WME_WINENTRY("lonelyrobot", "Beta",
		WME_ENTRY1s("data.dcp", "a0cf7ad5bab957416dcda454e9f28ef0", 3420120), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_0),

	// Rebecca Carlson Mystery 01 - Silent Footsteps
	WME_WINENTRY("rebeccacarlson1", "",
		WME_ENTRY1s("data.dcp", "5fac4824b7ffdaee982dc18a45428182", 773144334), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Rebecca Carlson Mystery 01 - Silent Footsteps (Demo)
	WME_WINENTRY("rebeccacarlson1", "Demo",
		WME_ENTRY1s("data.dcp", "a46c41edb1ae5e19a3e0fc5cac3ce83b", 212698165), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_3),

	// On the Tracks of Dinosaurs (Beta 2.5) (English)
	// NOTE: This is a 2.5D game that is out of ScummVM scope
	WME_WINENTRY("reptilesquest", "Beta 2.5",
		WME_ENTRY2s("data.dcp", "b624d3b19e37c094801a28d817bc3d76", 27345755,
					"english.dcp", "42188d46ee079b555d578ea2b406fa19", 3897), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_1),

	// On the Tracks of Dinosaurs (Beta 2.5) (Italian)
	// NOTE: This is a 2.5D game that is out of ScummVM scope
	WME_WINENTRY("reptilesquest", "Beta 2.5",
		WME_ENTRY2s("data.dcp", "b624d3b19e37c094801a28d817bc3d76", 27345755,
					"italian.dcp", "361f41b1151119f66de851f07b78b7cc", 4132), Common::IT_ITA, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_1),

	// On the Tracks of Dinosaurs (Beta 2.5) (Russian)
	// NOTE: This is a 2.5D game that is out of ScummVM scope
	WME_WINENTRY("reptilesquest", "Beta 2.5",
		WME_ENTRY2s("data.dcp", "b624d3b19e37c094801a28d817bc3d76", 27345755,
					"russian.dcp", "90d2bf541ba381bbb3375aa7d31613fb", 42175), Common::RU_RUS, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_1),

	// Reversion: The Escape Version 1.0
	WME_WINENTRY("reversion1", "Version 1.0",
		WME_ENTRY1s("data.dcp", "cd616f98ebfd047e0c540b50b4b70761", 254384531), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape Version 1.1 (Spanish)
	WME_WINENTRY("reversion1", "Version 1.1",
		WME_ENTRY2s("data.dcp", "cb9865dc7e1db2990a8cf4bc13cf4999", 257643032,
					"data.dcp", "cb9865dc7e1db2990a8cf4bc13cf4999", 257643032), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape Version 1.1 (Chinese)
	WME_WINENTRY("reversion1", "Version 1.1",
		WME_ENTRY2s("chinese.dcp", "cf97150739499a4c15f51dc534ff85a1", 6330561,
					"data.dcp", "cb9865dc7e1db2990a8cf4bc13cf4999", 257643032), Common::ZH_CNA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape Version 1.1 (English)
	WME_WINENTRY("reversion1", "Version 1.1",
		WME_ENTRY2s("english.dcp", "7b2f061d7c91365c5d04605f1de032b3", 5702699,
					"data.dcp", "cb9865dc7e1db2990a8cf4bc13cf4999", 257643032), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape Version 1.1 (French)
	WME_WINENTRY("reversion1", "Version 1.1",
		WME_ENTRY2s("french.dcp", "214204b6022c5ed67fada44557690faf", 6327400,
					"data.dcp", "cb9865dc7e1db2990a8cf4bc13cf4999", 257643032), Common::FR_FRA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape Version 1.1 (German)
	WME_WINENTRY("reversion1", "Version 1.1",
		WME_ENTRY2s("german.dcp", "96677823b36d580a4a29e3659071071c", 6340699,
					"data.dcp", "cb9865dc7e1db2990a8cf4bc13cf4999", 257643032), Common::DE_DEU, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape Version 1.1 (Italian)
	WME_WINENTRY("reversion1", "Version 1.1",
		WME_ENTRY2s("italian.dcp", "9ce80c1835108f10170a02969f71efe1", 6301836,
					"data.dcp", "cb9865dc7e1db2990a8cf4bc13cf4999", 257643032), Common::IT_ITA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape Version 1.1 (Portuguese)
	WME_WINENTRY("reversion1", "Version 1.1",
		WME_ENTRY2s("portugues.dcp", "8772501afa2c630a7c697eb99e9c7bda", 5053303,
					"data.dcp", "cb9865dc7e1db2990a8cf4bc13cf4999", 257643032), Common::PT_BRA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape Version 1.3 (Spanish)
	WME_WINENTRY("reversion1", "Version 1.3",
		WME_ENTRY2s("data.dcp", "9ebb12f6fd7c038d079f81beb3bd96d5", 254185907,
					"data.dcp", "9ebb12f6fd7c038d079f81beb3bd96d5", 254185907), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape Version 1.3 (Chinese)
	WME_WINENTRY("reversion1", "Version 1.3",
		WME_ENTRY2s("xlanguage_nz.dcp", "92c4065156e464211685bf799b3279fd", 5130600,
					"data.dcp", "9ebb12f6fd7c038d079f81beb3bd96d5", 254185907), Common::ZH_CNA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape Version 1.3 (English)
	WME_WINENTRY("reversion1", "Version 1.3",
		WME_ENTRY2s("xlanguage_en.dcp", "05845e1283920a6e4044f2a54f7a9519", 4818543,
					"data.dcp", "9ebb12f6fd7c038d079f81beb3bd96d5", 254185907), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape Version 1.3 (French)
	WME_WINENTRY("reversion1", "Version 1.3",
		WME_ENTRY2s("xlanguage_fr.dcp", "441795490e9307eb2ed07830779881ac", 5425959,
					"data.dcp", "9ebb12f6fd7c038d079f81beb3bd96d5", 254185907), Common::FR_FRA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape Version 1.3 (German)
	WME_WINENTRY("reversion1", "Version 1.3",
		WME_ENTRY2s("xlanguage_de.dcp", "b588041015b93e54b4c246ca77d01e76", 5423798,
					"data.dcp", "9ebb12f6fd7c038d079f81beb3bd96d5", 254185907), Common::DE_DEU, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape Version 1.3 (Italian)
	WME_WINENTRY("reversion1", "Version 1.3",
		WME_ENTRY2s("xlanguage_it.dcp", "a1f4199079b75ee10cded41f05b45d5f", 5386424,
					"data.dcp", "9ebb12f6fd7c038d079f81beb3bd96d5", 254185907), Common::IT_ITA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape Version 1.3 (Latvian)
	WME_WINENTRY("reversion1", "Version 1.3",
		WME_ENTRY2s("xlanguage_lv.dcp", "351e8226a2a86dd15b8568b9d695a3c0", 4959028,
					"data.dcp", "9ebb12f6fd7c038d079f81beb3bd96d5", 254185907), Common::LV_LAT, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape Version 1.3 (Portuguese)
	WME_WINENTRY("reversion1", "Version 1.3",
		WME_ENTRY2s("xlanguage_pt.dcp", "3d653debd37e56756a79401e1004c4d2", 4149165,
					"data.dcp", "9ebb12f6fd7c038d079f81beb3bd96d5", 254185907), Common::PT_BRA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape Version 1.3.2369 (Spanish)
	WME_WINENTRY("reversion1", "Version 1.3.2369",
		WME_ENTRY2s("data.dcp", "aecb5deeea7b0baa871fbd0cef35a648", 254219204,
					"data.dcp", "aecb5deeea7b0baa871fbd0cef35a648", 254219204), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape Version 1.3.2369 (Chinese)
	WME_WINENTRY("reversion1", "Version 1.3.2369",
		WME_ENTRY2s("xlanguage_nz.dcp", "7146dfa43ffdf0886e034fffe2c8a0c0", 13722261,
					"data.dcp", "aecb5deeea7b0baa871fbd0cef35a648", 254219204), Common::ZH_CNA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape Version 1.3.2369 (English)
	WME_WINENTRY("reversion1", "Version 1.3.2369",
		WME_ENTRY2s("xlanguage_en.dcp", "64b6fa7eedc09c231f6ce046e77fee05", 11339619,
					"data.dcp", "aecb5deeea7b0baa871fbd0cef35a648", 254219204), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape Version 1.3.2369 (French)
	WME_WINENTRY("reversion1", "Version 1.3.2369",
		WME_ENTRY2s("xlanguage_fr.dcp", "d561d562224afea809153a1fd9fdb0c0", 11963210,
					"data.dcp", "aecb5deeea7b0baa871fbd0cef35a648", 254219204), Common::FR_FRA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape Version 1.3.2369 (German)
	WME_WINENTRY("reversion1", "Version 1.3.2369",
		WME_ENTRY2s("xlanguage_de.dcp", "4e3f614c36bd6bae74b8cc83e663a8f0", 14040310,
					"data.dcp", "aecb5deeea7b0baa871fbd0cef35a648", 254219204), Common::DE_DEU, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape Version 1.3.2369 (Italian)
	WME_WINENTRY("reversion1", "Version 1.3.2369",
		WME_ENTRY2s("xlanguage_it.dcp", "10d09b7fe61946f09dd91d5e8d090f94", 11913752,
					"data.dcp", "aecb5deeea7b0baa871fbd0cef35a648", 254219204), Common::IT_ITA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape Version 1.3.2369 (Latvian)
	WME_WINENTRY("reversion1", "Version 1.3.2369",
		WME_ENTRY2s("xlanguage_lv.dcp", "704359ab5040b0dab6545064d7aa6eb9", 11414925,
					"data.dcp", "aecb5deeea7b0baa871fbd0cef35a648", 254219204), Common::LV_LAT, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape Version 1.3.2369 (Polish)
	WME_WINENTRY("reversion1", "Version 1.3.2369",
		WME_ENTRY2s("xlanguage_pl.dcp", "c4ad33f57e1e998169552d521c1d6638", 11532215,
					"data.dcp", "aecb5deeea7b0baa871fbd0cef35a648", 254219204), Common::PL_POL, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape Version 1.3.2369 (Portuguese)
	WME_WINENTRY("reversion1", "Version 1.3.2369",
		WME_ENTRY2s("xlanguage_pt.dcp", "886886b6b14aadac844078de856799a6", 10620797,
					"data.dcp", "aecb5deeea7b0baa871fbd0cef35a648", 254219204), Common::PT_BRA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, July 2014) (Spanish)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "438e04e70a107539a56fcd332d31f04d", 254298579,
					"data.dcp", "438e04e70a107539a56fcd332d31f04d", 254298579), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_3),
	
	// Reversion: The Escape (Steam, July 2014) (German)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "438e04e70a107539a56fcd332d31f04d", 254298579,
			"xlanguage_de.dcp", "c11ede8af2fafd0d32f32cfc6240ffc2", 14040310), Common::DE_DEU, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, July 2014) (English)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "438e04e70a107539a56fcd332d31f04d", 254298579,
			"xlanguage_en.dcp", "c94c947f71d986131ac9554eca99c020", 11339619), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, July 2014) (French)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "438e04e70a107539a56fcd332d31f04d", 254298579,
			"xlanguage_fr.dcp", "10895b608051647a0976738c57dba0be", 11963210), Common::FR_FRA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, July 2014) (Italian)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "438e04e70a107539a56fcd332d31f04d", 254298579,
			"xlanguage_it.dcp", "2468ef3b3c8c4eba9146ae5f31ac5f0c", 11913752), Common::IT_ITA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, July 2014) (Latvian)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "438e04e70a107539a56fcd332d31f04d", 254298579,
			"xlanguage_lv.dcp", "846f1b99257c9f034d4cd5c89986a8c5", 11414925), Common::LV_LAT, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, July 2014) (Chinese)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "438e04e70a107539a56fcd332d31f04d", 254298579,
			"xlanguage_nz.dcp", "8dadcc96212743162df313629cd3adfd", 13722261), Common::ZH_CNA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, July 2014) (Polish)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "438e04e70a107539a56fcd332d31f04d", 254298579,
			"xlanguage_pl.dcp", "3a353627d7f19261d8092ef48ed9c1bd", 11532215), Common::PL_POL, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, July 2014) (Portuguese)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "438e04e70a107539a56fcd332d31f04d", 254298579,
			"xlanguage_pt.dcp", "47d7336a1c94ca406f7e4a45b0ddccc4", 10620797), Common::PT_BRA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, August 2014) (Spanish)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
					"data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::ES_ESP, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2014) (German)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
			"xlanguage_de.dcp", "7fa066e8102d8bf84b5451af2ee8859f", 14040310,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::DE_DEU, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2014) (English)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
			"xlanguage_en.dcp", "421a94e151d424ee666709673ddbea5e", 11339619,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::EN_ANY, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2014) (French)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
			"xlanguage_fr.dcp", "4902f9ba3841650becff2d687e7a9963", 11963210,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::FR_FRA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2014) (Italian)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
			"xlanguage_it.dcp", "a248da64e7701a7549d25dbb02e7dca8", 11913752,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::IT_ITA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2014) (Latvian)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
			"xlanguage_lv.dcp", "613a9ada4dffe012b7a25f69818c9353", 11414925,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::LV_LAT, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2014) (Chinese)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
			"xlanguage_nz.dcp", "b61971e451832f32262a2e5361b40126", 15631919,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::ZH_CNA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2014) (Polish)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
			"xlanguage_pl.dcp", "1d95192d1bab8ab25395fc3c1b16f160", 11963326,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::PL_POL, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2014) (Portuguese)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
			"xlanguage_pt.dcp", "5a20a886350ab866e5056696100598e7", 10620797,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::PT_BRA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2014) (Spanish)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
					"data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::ES_ESP, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2014) (German)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
			"xlanguage_de.dcp", "7fa066e8102d8bf84b5451af2ee8859f", 14040310,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::DE_DEU, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2014) (English)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
			"xlanguage_en.dcp", "421a94e151d424ee666709673ddbea5e", 11339619,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::EN_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2014) (French)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
			"xlanguage_fr.dcp", "4902f9ba3841650becff2d687e7a9963", 11963210,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::FR_FRA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2014) (Italian)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
			"xlanguage_it.dcp", "a248da64e7701a7549d25dbb02e7dca8", 11913752,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::IT_ITA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2014) (Latvian)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
			"xlanguage_lv.dcp", "613a9ada4dffe012b7a25f69818c9353", 11414925,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::LV_LAT, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2014) (Chinese)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
			"xlanguage_nz.dcp", "b61971e451832f32262a2e5361b40126", 15631919,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::ZH_CNA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2014) (Polish)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
			"xlanguage_pl.dcp", "1d95192d1bab8ab25395fc3c1b16f160", 11963326,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::PL_POL, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2014) (Portuguese)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
			"xlanguage_pt.dcp", "5a20a886350ab866e5056696100598e7", 10620797,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::PT_BRA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, September 2014) (Polish)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
			"xlanguage_pl.dcp", "d38188219fd5b47a0f9c6777ef1ec93e", 11550402,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::PL_POL, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, September 2014) (Polish)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
			"xlanguage_pl.dcp", "d38188219fd5b47a0f9c6777ef1ec93e", 11550402,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::PL_POL, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (Spanish)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "2ab1e724ea83e61049cc46a3604a9a53", 254281202,
					"data.dcp", "2ab1e724ea83e61049cc46a3604a9a53", 254281202), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_3),
	
	// Reversion: The Escape (Steam, January 2016) (German)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "2ab1e724ea83e61049cc46a3604a9a53", 254281202,
			"xlanguage_de.dcp", "25aa33603fc2fcec328158221c46a536", 14023446), Common::DE_DEU, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, January 2016) (English)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "2ab1e724ea83e61049cc46a3604a9a53", 254281202,
			"xlanguage_en.dcp", "eb6a32dfe6c964a4c244ee9022bb67c8", 11324849), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, January 2016) (French)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "2ab1e724ea83e61049cc46a3604a9a53", 254281202,
			"xlanguage_fr.dcp", "110153166e64550b0b41d5c27ef2fb46", 11949050), Common::FR_FRA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, January 2016) (Italian)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "2ab1e724ea83e61049cc46a3604a9a53", 254281202,
			"xlanguage_it.dcp", "961c4b62b3657c854c98773d4c54bc45", 11898661), Common::IT_ITA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, January 2016) (Latvian)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "2ab1e724ea83e61049cc46a3604a9a53", 254281202,
			"xlanguage_lv.dcp", "44e5b05c60cebdb1c4d969827d5870e3", 11427642), Common::LV_LAT, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, January 2016) (Chinese)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "2ab1e724ea83e61049cc46a3604a9a53", 254281202,
			"xlanguage_nz.dcp", "e2391e04c9f6c3c7a66c9754e1c90594", 13736623), Common::ZH_CNA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, January 2016) (Polish)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "2ab1e724ea83e61049cc46a3604a9a53", 254281202,
			"xlanguage_pl.dcp", "b359baf8e1558fab6656f0d3bf85d1d1", 11543266), Common::PL_POL, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, January 2016) (Portuguese)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "2ab1e724ea83e61049cc46a3604a9a53", 254281202,
			"xlanguage_pt.dcp", "86778ac5438c22ebc0d59be9334fae3a", 10600764), Common::PT_BRA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, January 2016) (Russian)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "2ab1e724ea83e61049cc46a3604a9a53", 254281202,
			"xlanguage_ru.dcp", "43460a03a7069d268250008a79776dff", 10831308), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, January 2016) (Spanish)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
					"data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::ES_ESP, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (German)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
			"xlanguage_de.dcp", "1a234a256680db57d23a475957164cf0", 14023446,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::DE_DEU, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (English)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
			"xlanguage_en.dcp", "86ff4e6b3142eaf6b9135acc9ba3f7b9", 11324849,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::EN_ANY, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (French)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
			"xlanguage_fr.dcp", "423929742fd86381b66f8edb518391a9", 11949050,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::FR_FRA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (Italian)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
			"xlanguage_it.dcp", "b4324c09be872001ff0838e3d1502aec", 11898661,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::IT_ITA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (Latvian)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
			"xlanguage_lv.dcp", "4230c16f5b8625b30b961c0bc4312935", 11427642,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::LV_LAT, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (Chinese)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
			"xlanguage_nz.dcp", "71eec69ab20f009154575afe52c1eeed", 13736623,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::ZH_CNA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (Polish)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
			"xlanguage_pl.dcp", "63d8ba6c1ae1472d8ec9447cb388282a", 11543298,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::PL_POL, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (Portuguese)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
			"xlanguage_pt.dcp", "5fcdc5402993dbb45eb17c641c15afe3", 10600764,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::PT_BRA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (Russian)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
			"xlanguage_ru.dcp", "c8e2bfb6e8adca0d93855d583a6a46c0", 10854300,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::RU_RUS, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (Spanish)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
					"data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::ES_ESP, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (German)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
			"xlanguage_de.dcp", "1a234a256680db57d23a475957164cf0", 14023446,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::DE_DEU, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (English)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
			"xlanguage_en.dcp", "86ff4e6b3142eaf6b9135acc9ba3f7b9", 11324849,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::EN_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (French)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
			"xlanguage_fr.dcp", "423929742fd86381b66f8edb518391a9", 11949050,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::FR_FRA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (Italian)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
			"xlanguage_it.dcp", "b4324c09be872001ff0838e3d1502aec", 11898661,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::IT_ITA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (Latvian)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
			"xlanguage_lv.dcp", "4230c16f5b8625b30b961c0bc4312935", 11427642,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::LV_LAT, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (Chinese)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
			"xlanguage_nz.dcp", "71eec69ab20f009154575afe52c1eeed", 13736623,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::ZH_CNA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (Polish)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
			"xlanguage_pl.dcp", "63d8ba6c1ae1472d8ec9447cb388282a", 11543298,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::PL_POL, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (Portuguese)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
			"xlanguage_pt.dcp", "5fcdc5402993dbb45eb17c641c15afe3", 10600764,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::PT_BRA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (Russian)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
			"xlanguage_ru.dcp", "c8e2bfb6e8adca0d93855d583a6a46c0", 10854300,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::RU_RUS, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (Spanish)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "e63ddf8801404555010c32bf849b3428", 254307881,
					"data.dcp", "e63ddf8801404555010c32bf849b3428", 254307881,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::ES_ESP, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (German)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "e63ddf8801404555010c32bf849b3428", 254307881,
			"xlanguage_de.dcp", "c24e6dc396fa879e6c24e6f5b4afc9c0", 14032355,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::DE_DEU, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (English)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "e63ddf8801404555010c32bf849b3428", 254307881,
			"xlanguage_en.dcp", "d47e3b4c94ce34976481bcd9ba97d1d4", 11333684,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::EN_ANY, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (French)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "e63ddf8801404555010c32bf849b3428", 254307881,
			"xlanguage_fr.dcp", "fe5a3476e54456d2140f1164d8030fc5", 11957951,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::FR_FRA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (Italian)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "e63ddf8801404555010c32bf849b3428", 254307881,
			"xlanguage_it.dcp", "f21108641cf7c48444ec61902133033f", 11905940,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::IT_ITA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (Latvian)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "e63ddf8801404555010c32bf849b3428", 254307881,
			"xlanguage_lv.dcp", "3019b4f0854aa5f25eed35d4e04f589b", 11436318,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::LV_LAT, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (Chinese)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "e63ddf8801404555010c32bf849b3428", 254307881,
			"xlanguage_nz.dcp", "1dd070d3f754555782a08bd4d3657ed5", 15652628,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::ZH_CNA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (Polish)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "e63ddf8801404555010c32bf849b3428", 254307881,
			"xlanguage_pl.dcp", "8ba30c378de8a7a7102ef14f17ad7687", 11982966,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::PL_POL, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (Portuguese)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "e63ddf8801404555010c32bf849b3428", 254307881,
			"xlanguage_pt.dcp", "2de479d003cbb3d83df38e26eeb9e342", 10613872,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::PT_BRA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (Russian)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "e63ddf8801404555010c32bf849b3428", 254307881,
			"xlanguage_ru.dcp", "7f887ba9508c7c844861ec8f427a39ff", 10863181,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::RU_RUS, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (Spanish)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "fe268238ebdf069db1954278f174614f", 254307881,
					"data.dcp", "fe268238ebdf069db1954278f174614f", 254307881,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::ES_ESP, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (German)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "fe268238ebdf069db1954278f174614f", 254307881,
			"xlanguage_de.dcp", "af050ff3795c482502ec76a0d95b1e67", 14032355,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::DE_DEU, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (English)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "fe268238ebdf069db1954278f174614f", 254307881,
			"xlanguage_en.dcp", "7f827f7b37d5dccc569941b32bb12b4b", 11333684,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::EN_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (French)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "fe268238ebdf069db1954278f174614f", 254307881,
			"xlanguage_fr.dcp", "b31a1cc4ce0b2280bff48aac8bfb7c3f", 11957951,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::FR_FRA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (Italian)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "fe268238ebdf069db1954278f174614f", 254307881,
			"xlanguage_it.dcp", "75f20f0ac0c77c0e8c33838df9e20abc", 11905940,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::IT_ITA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (Latvian)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "fe268238ebdf069db1954278f174614f", 254307881,
			"xlanguage_lv.dcp", "32b6f5d51a7a4e4e52a89146b02d69a2", 11436318,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::LV_LAT, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (Chinese)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "fe268238ebdf069db1954278f174614f", 254307881,
			"xlanguage_nz.dcp", "6647dfd8dc5a1858aaba6571ade3fccb", 13742970,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::ZH_CNA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (Polish)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "fe268238ebdf069db1954278f174614f", 254307881,
			"xlanguage_pl.dcp", "8762cbda33ddb6def83890761c608652", 11551887,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::PL_POL, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (Portuguese)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "fe268238ebdf069db1954278f174614f", 254307881,
			"xlanguage_pt.dcp", "4be1f4670d6d1719bdf545a273af83b7", 10613872,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::PT_BRA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (Russian)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "fe268238ebdf069db1954278f174614f", 254307881,
			"xlanguage_ru.dcp", "4d5cbc9424e4a9f04ae161ede75f9d67", 10863181,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::RU_RUS, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (Spanish)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "2ac5dac7b06ab6ca54beaee85b60c0da", 254290282,
					"data.dcp", "2ac5dac7b06ab6ca54beaee85b60c0da", 254290282), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, August 2016) (German)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "2ac5dac7b06ab6ca54beaee85b60c0da", 254290282,
			"xlanguage_de.dcp", "03018955611279d3035ed4c44dc3d739", 14032355), Common::DE_DEU, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, August 2016) (English)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "2ac5dac7b06ab6ca54beaee85b60c0da", 254290282,
			"xlanguage_en.dcp", "f06b7408164cdc08e9aa94dc29001ed0", 11333684), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, August 2016) (French)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "2ac5dac7b06ab6ca54beaee85b60c0da", 254290282,
			"xlanguage_fr.dcp", "f489d5893560843c1eacbf5f76eb7638", 11957951), Common::FR_FRA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, August 2016) (Italian)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "2ac5dac7b06ab6ca54beaee85b60c0da", 254290282,
			"xlanguage_it.dcp", "82eb7fc23fee6db2b350126804b02139", 11905940), Common::IT_ITA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, August 2016) (Latvian)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "2ac5dac7b06ab6ca54beaee85b60c0da", 254290282,
			"xlanguage_lv.dcp", "49f76e07d2066ec6c4a5507ae9c1aad8", 11436318), Common::LV_LAT, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, August 2016) (Chinese)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "2ac5dac7b06ab6ca54beaee85b60c0da", 254290282,
			"xlanguage_nz.dcp", "21e2f1f04d7d683e926cf1934e488630", 13742970), Common::ZH_CNA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, August 2016) (Polish)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "2ac5dac7b06ab6ca54beaee85b60c0da", 254290282,
			"xlanguage_pl.dcp", "bf880fbe62c7743063b37447e228d3a8", 11551855), Common::PL_POL, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, August 2016) (Portuguese)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "2ac5dac7b06ab6ca54beaee85b60c0da", 254290282,
			"xlanguage_pt.dcp", "d81ab6c6316114b70e64e32b761f1084", 10613872), Common::PT_BRA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, August 2016) (Russian)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "2ac5dac7b06ab6ca54beaee85b60c0da", 254290282,
			"xlanguage_ru.dcp", "7f5379114f17593d591e739dc3f873fc", 10840189), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, September 2016) (Portuguese)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "2ac5dac7b06ab6ca54beaee85b60c0da", 254290282,
			"xlanguage_pt.dcp", "bb163dacb99fe93c4a9845fcca72c383", 10639302), Common::PT_BRA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, September 2016) (Portuguese)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "e63ddf8801404555010c32bf849b3428", 254307881,
			"xlanguage_pt.dcp", "8b6d596a84ea8730e0fd0e60c79083de", 10639302,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::PT_BRA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, September 2016) (Portuguese)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "fe268238ebdf069db1954278f174614f", 254307881,
			"xlanguage_pt.dcp", "8b6d596a84ea8730e0fd0e60c79083de", 10639302,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::PT_BRA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (Spanish)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "5e4d40075f69fa7702530e38c349d2fd", 254293949,
					"data.dcp", "5e4d40075f69fa7702530e38c349d2fd", 254293949), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, November 2016) (German)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "5e4d40075f69fa7702530e38c349d2fd", 254293949,
			"xlanguage_de.dcp", "d2471dceea100734afeb4ea77a0153ec", 14036212), Common::DE_DEU, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, November 2016) (English)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "5e4d40075f69fa7702530e38c349d2fd", 254293949,
			"xlanguage_en.dcp", "bf22fff27640014127929fe5347e1460", 11336876), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, November 2016) (French)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "5e4d40075f69fa7702530e38c349d2fd", 254293949,
			"xlanguage_fr.dcp", "5beabe999a110f31bddbc868f743c640", 11961697), Common::FR_FRA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, November 2016) (Italian)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "5e4d40075f69fa7702530e38c349d2fd", 254293949,
			"xlanguage_it.dcp", "a98fa205490f7205d8cc301ab05ed115", 11564269), Common::IT_ITA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, November 2016) (Latvian)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "5e4d40075f69fa7702530e38c349d2fd", 254293949,
			"xlanguage_lv.dcp", "6983daaf6bba6a496a73e272dbe274c4", 11436318), Common::LV_LAT, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, November 2016) (Chinese)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "5e4d40075f69fa7702530e38c349d2fd", 254293949,
			"xlanguage_nz.dcp", "51b9d4cb6c4a571503e1d75fc4b4024c", 13739377), Common::ZH_CNA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, November 2016) (Polish)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "5e4d40075f69fa7702530e38c349d2fd", 254293949,
			"xlanguage_pl.dcp", "bf880fbe62c7743063b37447e228d3a8", 11551855), Common::PL_POL, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, November 2016) (Portuguese)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "5e4d40075f69fa7702530e38c349d2fd", 254293949,
			"xlanguage_pt.dcp", "44a1ec3499e968373140dfef041099af", 10642716), Common::PT_BRA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, November 2016) (Russian)
	WME_WINENTRY("reversion1", "",
		WME_ENTRY2s("data.dcp", "5e4d40075f69fa7702530e38c349d2fd", 254293949,
			"xlanguage_ru.dcp", "c5860426055e72a0d1ce3467e1835572", 10843778), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, November 2016) (Spanish)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
					"data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::ES_ESP, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (German)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
			"xlanguage_de.dcp", "41e6f33c9c9fd9c8942153cc75975c06", 14036212,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::DE_DEU, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (English)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
			"xlanguage_en.dcp", "1740dfcf1008d5bf5cfff091d17df145", 11336876,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::EN_ANY, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (French)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
			"xlanguage_fr.dcp", "3638aba23c17a06ee03253352345cf74", 11961697,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::FR_FRA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (Italian)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
			"xlanguage_it.dcp", "80abe88fb49b8e30aa9eb7ce8bfa1600", 11564269,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::IT_ITA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (Latvian)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
			"xlanguage_lv.dcp", "aa6be87b3227a7786206f05786e9982a", 11436318,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::LV_LAT, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (Chinese)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
			"xlanguage_nz.dcp", "858fcd838458a584b4c0c71e1b7524c8", 15649035,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::ZH_CNA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (Polish)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
			"xlanguage_pl.dcp", "8ba30c378de8a7a7102ef14f17ad7687", 11982966,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::PL_POL, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (Portuguese)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
			"xlanguage_pt.dcp", "f3c2c3fa8fa63a41552cae6789c7138d", 10642716,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::PT_BRA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (Russian)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
			"xlanguage_ru.dcp", "4e01e8d4759fddbcb6eb07d19c0c3335", 10866770,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::RU_RUS, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (Spanish)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
					"data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::ES_ESP, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (German)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
			"xlanguage_de.dcp", "41e6f33c9c9fd9c8942153cc75975c06", 14036212,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::DE_DEU, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (English)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
			"xlanguage_en.dcp", "1740dfcf1008d5bf5cfff091d17df145", 11336876,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::EN_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (French)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
			"xlanguage_fr.dcp", "3638aba23c17a06ee03253352345cf74", 11961697,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::FR_FRA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (Italian)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
			"xlanguage_it.dcp", "80abe88fb49b8e30aa9eb7ce8bfa1600", 11564269,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::IT_ITA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (Latvian)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
			"xlanguage_lv.dcp", "aa6be87b3227a7786206f05786e9982a", 11436318,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::LV_LAT, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (Chinese)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
			"xlanguage_nz.dcp", "858fcd838458a584b4c0c71e1b7524c8", 15649035,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::ZH_CNA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (Polish)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
			"xlanguage_pl.dcp", "8762cbda33ddb6def83890761c608652", 11551887,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::PL_POL, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (Portuguese)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
			"xlanguage_pt.dcp", "f3c2c3fa8fa63a41552cae6789c7138d", 10642716,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::PT_BRA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (Russian)
	WME_PLATENTRY("reversion1", "",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
			"xlanguage_ru.dcp", "4e01e8d4759fddbcb6eb07d19c0c3335", 10866770,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::RU_RUS, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Chinese)
	WME_WINENTRY("reversion2", "",
		WME_ENTRY2s("xlanguage_nz.dcp", "8c3709474a87a7876109025dff41ff3f", 8746015,
					"data.dcp", "cb9865dc7e1db2990a8cf4bc13cf4999", 257643032), Common::ZH_CNA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (English)
	WME_WINENTRY("reversion2", "",
		WME_ENTRY2s("xlanguage_en.dcp", "ca357d86618d1ab76a21c913f4403cbd", 8414976,
					"data.dcp", "f7938cbfdc48f07934550245a3286921", 255672016), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Spanish)
	WME_WINENTRY("reversion2", "",
		WME_ENTRY2s("data.dcp", "f7938cbfdc48f07934550245a3286921", 255672016,
					"data.dcp", "f7938cbfdc48f07934550245a3286921", 255672016), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting Version 2.0.2412 (Spanish)
	WME_PLATENTRY("reversion2", "Version 2.0.2412",
		WME_ENTRY3s("data.dcp", "f4ffc4df24b7bebad56a24930f33a2bc", 255766600,
					"data.dcp", "f4ffc4df24b7bebad56a24930f33a2bc", 255766600,
					"Linux.dcp", "21858bd77dc86b03f701fd47900e2f51", 984535), Common::ES_ESP, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting Version 2.0.2412 (Chinese)
	WME_PLATENTRY("reversion2", "Version 2.0.2412",
		WME_ENTRY3s("data.dcp", "f4ffc4df24b7bebad56a24930f33a2bc", 255766600,
					"xlanguage_nz.dcp", "17c79af4928e24484bee77a7e807cc2a", 10737127,
					"Linux.dcp", "21858bd77dc86b03f701fd47900e2f51", 984535), Common::ZH_CNA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting Version 2.0.2412 (English)
	WME_PLATENTRY("reversion2", "Version 2.0.2412",
		WME_ENTRY3s("data.dcp", "f4ffc4df24b7bebad56a24930f33a2bc", 255766600,
					"xlanguage_en.dcp", "0598bf752ce93b42bcaf1094df537c7b", 8533057,
					"Linux.dcp", "21858bd77dc86b03f701fd47900e2f51", 984535), Common::EN_ANY, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Desura, before April 2014) (Spanish)
	WME_WINENTRY("reversion2", "",
		WME_ENTRY2s("data.dcp", "c4aa6295bc6cef6995430232298fdd5f", 255654527,
					"data.dcp", "c4aa6295bc6cef6995430232298fdd5f", 255654527), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_3),
	
	// Reversion: The Meeting (Desura, before April 2014) (English)
	WME_WINENTRY("reversion2", "",
		WME_ENTRY2s("data.dcp", "c4aa6295bc6cef6995430232298fdd5f", 255654527,
			"xlanguage_en.dcp", "74f3c75dc4f62e784682016b37990122", 8440487), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),
	
	// Reversion: The Meeting (Desura, before April 2014) (Chinese)
	WME_WINENTRY("reversion2", "",
		WME_ENTRY2s("data.dcp", "c4aa6295bc6cef6995430232298fdd5f", 255654527,
			"xlanguage_nz.dcp", "42f775cb68f7718d0751758a66816b98", 8734800), Common::ZH_CNA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, July 2014) (Spanish)
	WME_WINENTRY("reversion2", "",
		WME_ENTRY2s("data.dcp", "706adf37f47db7306b27ad75c35120e5", 256020490,
					"data.dcp", "706adf37f47db7306b27ad75c35120e5", 256020490), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, July 2014) (English)
	WME_WINENTRY("reversion2", "",
		WME_ENTRY2s("data.dcp", "706adf37f47db7306b27ad75c35120e5", 256020490,
			"xlanguage_en.dcp", "be5b15b4c591481d72a1bd869d7f1e99", 8533063), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, July 2014) (Chinese)
	WME_WINENTRY("reversion2", "",
		WME_ENTRY2s("data.dcp", "706adf37f47db7306b27ad75c35120e5", 256020490,
			"xlanguage_nz.dcp", "6aa4147492146055ec60d5a288c5ac60", 8827469), Common::ZH_CNA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, July 2014) (Spanish)
	WME_PLATENTRY("reversion2", "",
		WME_ENTRY3s("data.dcp", "c53b2f750ca06b0559c5776bea4be854", 256038089,
					"data.dcp", "c53b2f750ca06b0559c5776bea4be854", 256038089,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::ES_ESP, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, July 2014) (English)
	WME_PLATENTRY("reversion2", "",
		WME_ENTRY3s("data.dcp", "c53b2f750ca06b0559c5776bea4be854", 256038089,
			"xlanguage_en.dcp", "476780a51ecae86d1d1d6cacd25793ae", 8533063,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::EN_ANY, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, July 2014) (Chinese)
	WME_PLATENTRY("reversion2", "",
		WME_ENTRY3s("data.dcp", "c53b2f750ca06b0559c5776bea4be854", 256038089,
			"xlanguage_nz.dcp", "e2bfc09860a5448c65edc75812b34129", 10737127,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::ZH_CNA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, August 2014) (Spanish)
	WME_WINENTRY("reversion2", "",
		WME_ENTRY2s("data.dcp", "2e5d0c90715f8a19ef2f590e9535a907", 256020512,
					"data.dcp", "2e5d0c90715f8a19ef2f590e9535a907", 256020512), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, August 2014) (English)
	WME_WINENTRY("reversion2", "",
		WME_ENTRY2s("data.dcp", "2e5d0c90715f8a19ef2f590e9535a907", 256020512,
			"xlanguage_en.dcp", "be5b15b4c591481d72a1bd869d7f1e99", 8533063), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, August 2014) (Chinese)
	WME_WINENTRY("reversion2", "",
		WME_ENTRY2s("data.dcp", "2e5d0c90715f8a19ef2f590e9535a907", 256020512,
			"xlanguage_nz.dcp", "6aa4147492146055ec60d5a288c5ac60", 8827469), Common::ZH_CNA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, August 2014) (Spanish)
	WME_PLATENTRY("reversion2", "",
		WME_ENTRY3s("data.dcp", "970f577fea6e5e78abdf84f2ed26f545", 256038111,
					"data.dcp", "970f577fea6e5e78abdf84f2ed26f545", 256038111,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::ES_ESP, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, August 2014) (English)
	WME_PLATENTRY("reversion2", "",
		WME_ENTRY3s("data.dcp", "970f577fea6e5e78abdf84f2ed26f545", 256038111,
			"xlanguage_en.dcp", "476780a51ecae86d1d1d6cacd25793ae", 8533063,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::EN_ANY, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, August 2014) (Chinese)
	WME_PLATENTRY("reversion2", "",
		WME_ENTRY3s("data.dcp", "970f577fea6e5e78abdf84f2ed26f545", 256038111,
			"xlanguage_nz.dcp", "e2bfc09860a5448c65edc75812b34129", 10737127,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::ZH_CNA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, August 2014) (Spanish)
	WME_PLATENTRY("reversion2", "",
		WME_ENTRY3s("data.dcp", "970f577fea6e5e78abdf84f2ed26f545", 256038111,
					"data.dcp", "970f577fea6e5e78abdf84f2ed26f545", 256038111,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::ES_ESP, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, August 2014) (English)
	WME_PLATENTRY("reversion2", "",
		WME_ENTRY3s("data.dcp", "970f577fea6e5e78abdf84f2ed26f545", 256038111,
			"xlanguage_en.dcp", "476780a51ecae86d1d1d6cacd25793ae", 8533063,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::EN_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, August 2014) (Chinese)
	WME_PLATENTRY("reversion2", "",
		WME_ENTRY3s("data.dcp", "970f577fea6e5e78abdf84f2ed26f545", 256038111,
			"xlanguage_nz.dcp", "e2bfc09860a5448c65edc75812b34129", 10737127,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::ZH_CNA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, February 2016) (Spanish)
	WME_WINENTRY("reversion2", "",
		WME_ENTRY2s("data.dcp", "6e507ba64db28dcd766a5fe41c95e152", 256017952,
					"data.dcp", "6e507ba64db28dcd766a5fe41c95e152", 256017952), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, February 2016) (English)
	WME_WINENTRY("reversion2", "",
		WME_ENTRY2s("data.dcp", "6e507ba64db28dcd766a5fe41c95e152", 256017952,
			"xlanguage_en.dcp", "0542c42dcaacfdfd29f6e7009a061b1f", 8523503), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, February 2016) (Chinese)
	WME_WINENTRY("reversion2", "",
		WME_ENTRY2s("data.dcp", "6e507ba64db28dcd766a5fe41c95e152", 256017952,
			"xlanguage_nz.dcp", "299aee7eaf5e861da1f993114f11bf3e", 8845182), Common::ZH_CNA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, February 2016) (Russian)
	WME_WINENTRY("reversion2", "",
		WME_ENTRY2s("data.dcp", "6e507ba64db28dcd766a5fe41c95e152", 256017952,
			"xlanguage_ru.dcp", "15cd8cba2d58413707f03ff3b821e964", 10418988), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, February 2016) (Spanish)
	WME_PLATENTRY("reversion2", "",
		WME_ENTRY3s("data.dcp", "cfc3be215c525d1171ccdab3236df45b", 256035551,
					"data.dcp", "cfc3be215c525d1171ccdab3236df45b", 256035551,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::ES_ESP, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, February 2016) (English)
	WME_PLATENTRY("reversion2", "",
		WME_ENTRY3s("data.dcp", "cfc3be215c525d1171ccdab3236df45b", 256035551,
			"xlanguage_en.dcp", "ebb6433c36d0dd38a3ac8fe1c2971614", 8523503,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::EN_ANY, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, February 2016) (Chinese)
	WME_PLATENTRY("reversion2", "",
		WME_ENTRY3s("data.dcp", "cfc3be215c525d1171ccdab3236df45b", 256035551,
			"xlanguage_nz.dcp", "885d3f7345606cbd159b16ddb46a7146", 10754840,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::ZH_CNA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, February 2016) (Russian)
	WME_PLATENTRY("reversion2", "",
		WME_ENTRY3s("data.dcp", "cfc3be215c525d1171ccdab3236df45b", 256035551,
			"xlanguage_ru.dcp", "b91e24a9d8966c420c261050cb59047e", 10441980,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::RU_RUS, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, February 2016) (Spanish)
	WME_PLATENTRY("reversion2", "",
		WME_ENTRY3s("data.dcp", "3dd1cf8187c0f46d545b386dc4f6091c", 256035551,
					"data.dcp", "3dd1cf8187c0f46d545b386dc4f6091c", 256035551,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::ES_ESP, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, February 2016) (English)
	WME_PLATENTRY("reversion2", "",
		WME_ENTRY3s("data.dcp", "3dd1cf8187c0f46d545b386dc4f6091c", 256035551,
			"xlanguage_en.dcp", "33ec450364bf50e3c228ac9edcc38892", 8523503,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::EN_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, February 2016) (Chinese)
	WME_PLATENTRY("reversion2", "",
		WME_ENTRY3s("data.dcp", "3dd1cf8187c0f46d545b386dc4f6091c", 256035551,
			"xlanguage_nz.dcp", "51bdea5ca4483edbcde520e87df643f6", 8845182,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::ZH_CNA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, February 2016) (Russian)
	WME_PLATENTRY("reversion2", "",
		WME_ENTRY3s("data.dcp", "3dd1cf8187c0f46d545b386dc4f6091c", 256035551,
			"xlanguage_ru.dcp", "aff303c2f2bfbd1d824398d1b742cb9e", 10441980,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::RU_RUS, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, September 2016) (Italian)
	WME_WINENTRY("reversion2", "",
		WME_ENTRY2s("data.dcp", "6e507ba64db28dcd766a5fe41c95e152", 256017952,
			"xlanguage_it.dcp", "633e4350a0e45dbb40b26404f0a546e9", 11484621), Common::IT_ITA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, September 2016) (Italian)
	WME_PLATENTRY("reversion2", "",
		WME_ENTRY3s("data.dcp", "970f577fea6e5e78abdf84f2ed26f545", 256038111,
			"xlanguage_it.dcp", "d9b30c7af8406bae02b9344e73a87b25", 11484621,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::IT_ITA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, September 2016) (Russian)
	WME_PLATENTRY("reversion2", "",
		WME_ENTRY3s("data.dcp", "970f577fea6e5e78abdf84f2ed26f545", 256038111,
			"xlanguage_ru.dcp", "177b5a206cffba5e353fd3c691ebb0ce", 10441980,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::RU_RUS, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, September 2016) (Italian)
	WME_PLATENTRY("reversion2", "",
		WME_ENTRY3s("data.dcp", "970f577fea6e5e78abdf84f2ed26f545", 256038111,
			"xlanguage_it.dcp", "d9b30c7af8406bae02b9344e73a87b25", 11484621,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::IT_ITA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, September 2016) (Russian)
	WME_PLATENTRY("reversion2", "",
		WME_ENTRY3s("data.dcp", "970f577fea6e5e78abdf84f2ed26f545", 256038111,
			"xlanguage_ru.dcp", "177b5a206cffba5e353fd3c691ebb0ce", 10441980,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::RU_RUS, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, November 2016) (Spanish)
	WME_WINENTRY("reversion2", "",
		WME_ENTRY2s("data.dcp", "ff3d33de24036afc5697f901fc7d872d", 256030699,
					"data.dcp", "ff3d33de24036afc5697f901fc7d872d", 256030699), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, November 2016) (German)
	WME_WINENTRY("reversion2", "",
		WME_ENTRY2s("data.dcp", "ff3d33de24036afc5697f901fc7d872d", 256030699,
			"xlanguage_de.dcp", "48b1f6185480fb3b64ba5f8996df4bda", 8727399), Common::DE_DEU, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, November 2016) (English)
	WME_WINENTRY("reversion2", "",
		WME_ENTRY2s("data.dcp", "ff3d33de24036afc5697f901fc7d872d", 256030699,
			"xlanguage_en.dcp", "3323ede6bb321aa0e3af6727b105e73a", 8535530), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, November 2016) (Italian)
	WME_WINENTRY("reversion2", "",
		WME_ENTRY2s("data.dcp", "ff3d33de24036afc5697f901fc7d872d", 256030699,
			"xlanguage_it.dcp", "0e315976351c9508355597a3506d521d", 11498415), Common::IT_ITA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, November 2016) (Chinese)
	WME_WINENTRY("reversion2", "",
		WME_ENTRY2s("data.dcp", "ff3d33de24036afc5697f901fc7d872d", 256030699,
			"xlanguage_nz.dcp", "45f52816d5ec5f8e0c1bd70a7aa17f7c", 8847936), Common::ZH_CNA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, November 2016) (Russian)
	WME_WINENTRY("reversion2", "",
		WME_ENTRY2s("data.dcp", "ff3d33de24036afc5697f901fc7d872d", 256030699,
			"xlanguage_ru.dcp", "a5925d2b4bda5b1d38cf83f851d42404", 10431458), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, November 2016) (Spanish)
	WME_PLATENTRY("reversion2", "",
		WME_ENTRY3s("data.dcp", "9cb5465054014620ece739039fe3602b", 256048298,
					"data.dcp", "9cb5465054014620ece739039fe3602b", 256048298,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::ES_ESP, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, November 2016) (English)
	WME_PLATENTRY("reversion2", "",
		WME_ENTRY3s("data.dcp", "9cb5465054014620ece739039fe3602b", 256048298,
			"xlanguage_en.dcp", "28b1230564debb10fbeed1477b998194", 8535530,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::EN_ANY, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, November 2016) (Italian)
	WME_PLATENTRY("reversion2", "",
		WME_ENTRY3s("data.dcp", "9cb5465054014620ece739039fe3602b", 256048298,
			"xlanguage_it.dcp", "b2cc0807a94d94befb4350db13659ef6", 11498415,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::IT_ITA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, November 2016) (Chinese)
	WME_PLATENTRY("reversion2", "",
		WME_ENTRY3s("data.dcp", "9cb5465054014620ece739039fe3602b", 256048298,
			"xlanguage_nz.dcp", "fcceb1300b9819abaee6832b7aef7f90", 10757594,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::ZH_CNA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, November 2016) (Russian)
	WME_PLATENTRY("reversion2", "",
		WME_ENTRY3s("data.dcp", "9cb5465054014620ece739039fe3602b", 256048298,
			"xlanguage_ru.dcp", "227b88befc98e5c8c0e25825ac80b6ca", 10454450,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::RU_RUS, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, November 2016) (Spanish)
	WME_PLATENTRY("reversion2", "",
		WME_ENTRY3s("data.dcp", "9cb5465054014620ece739039fe3602b", 256048298,
					"data.dcp", "9cb5465054014620ece739039fe3602b", 256048298,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::ES_ESP, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, November 2016) (English)
	WME_PLATENTRY("reversion2", "",
		WME_ENTRY3s("data.dcp", "9cb5465054014620ece739039fe3602b", 256048298,
			"xlanguage_en.dcp", "28b1230564debb10fbeed1477b998194", 8535530,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::EN_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, November 2016) (Italian)
	WME_PLATENTRY("reversion2", "",
		WME_ENTRY3s("data.dcp", "9cb5465054014620ece739039fe3602b", 256048298,
			"xlanguage_it.dcp", "b2cc0807a94d94befb4350db13659ef6", 11498415,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::IT_ITA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, November 2016) (Chinese)
	WME_PLATENTRY("reversion2", "",
		WME_ENTRY3s("data.dcp", "9cb5465054014620ece739039fe3602b", 256048298,
			"xlanguage_nz.dcp", "fcceb1300b9819abaee6832b7aef7f90", 10757594,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::ZH_CNA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, November 2016) (Russian)
	WME_PLATENTRY("reversion2", "",
		WME_ENTRY3s("data.dcp", "9cb5465054014620ece739039fe3602b", 256048298,
			"xlanguage_ru.dcp", "227b88befc98e5c8c0e25825ac80b6ca", 10454450,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::RU_RUS, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Rhiannon: Curse of the four Branches
	WME_WINENTRY("rhiannon", "",
		WME_ENTRY1s("data.dcp", "870f348900b735f1cc79c0608ce32b0e", 1046169851), Common::EN_ANY, ADGF_UNSTABLE, WME_1_8_6),

	// Rhiannon: Curse of the four Branches (English PC DVD)
	WME_WINENTRY("rhiannon", "DVD",
		WME_ENTRY1s("data.dcp", "6736bbc921bb6ce5161b3ad095a97bd4", 1053441028), Common::EN_ANY, ADGF_UNSTABLE, WME_1_8_6),

	// Rhiannon: Curse of the four Branches (Premium Edition)
	WME_WINENTRY("rhiannon", "Premium Edition",
		WME_ENTRY1s("data.dcp", "8debcadeda038f43d9ca9b97d827e7f3", 928484275), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// Rhiannon: Curse of The Four Branches (Demo: Chapter 1)
	WME_WINENTRY("rhiannon", "Demo: Chapter 1",
		WME_ENTRY1s("data.dcp", "d3b6ad94e9a8b29ca3cbda96c8857bd6", 72680676), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_8_6),

	// Rhiannon: Curse of The Four Branches (Demo: Chapter 5)
	WME_WINENTRY("rhiannon", "Demo: Chapter 5",
		WME_ENTRY1s("data.dcp", "88be3fa1389889b2079e54d780803a75", 88644508), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_8_6),

	// Rhiannon: Curse of The Four Branches (Demo 1)
	WME_WINENTRY("rhiannon", "Demo 1",
		WME_ENTRY1s("data.dcp", "5ee8d6684c374032a6baab4e40333c77", 95651432), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_8_6),

	// Rhiannon: Curse of The Four Branches (Demo 2)
	WME_WINENTRY("rhiannon", "Demo 2",
		WME_ENTRY1s("data.dcp", "52f111205bf294e8630652981eb31c3d", 79687595), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_8_6),

	// 1 1/2 Ritter: Auf der Suche nach der hinreissenden Herzelinde (German)
	WME_WINENTRY("ritter", "",
		WME_ENTRY1s("data.dcp", "5ac416cee605d3a30f4d59687b1cdab2", 364260278), Common::DE_DEU, ADGF_UNSTABLE, WME_1_8_6),

	// 1 1/2 Ritter: Auf der Suche nach der hinreissenden Herzelinde (Russian)
	WME_WINENTRY("ritter", "",
		WME_ENTRY2s("data.dcp", "5ac416cee605d3a30f4d59687b1cdab2", 364260278,
				"string3.tab", "e9d0e6341e8994be5fa6d70d39ac630b", 813982), Common::RU_RUS, ADGF_UNSTABLE, WME_1_8_6),

	// Satan and Sons (Demo)
	WME_WINENTRY("satanandsons", "Demo",
		WME_ENTRY1s("data.dcp", "16a6ba8174b697bbba9299619d1e20c4", 67539054), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_3_2),

	// Rosemary
	WME_WINENTRY("rosemary", "",
		WME_ENTRY1s("data.dcp", "4f2631138bd4d27587d9043f8aeff3df", 29483643), Common::EN_ANY, ADGF_UNSTABLE, WME_1_8_6),

	// Securanote
	WME_PLATENTRY("securanote", "",
		WME_ENTRY1s("data.dcp", "5213d3e59b9e95b7fbd5c56f7de5341a", 2625554), Common::EN_ANY, Common::kPlatformIOS, ADGF_UNSTABLE, WME_LITE),

	// Shaban (English)
	// TODO: add checksum of localization.txt
	WME_WINENTRY("shaban", "",
		WME_ENTRY1s("data.dcp", "35f702ca9baabc5c620e0be230195c8a", 755388466), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_2),

	// Shaban (Russian)
	WME_WINENTRY("shaban", "",
		WME_ENTRY2s("data.dcp", "e5636aa0244ed749eeaa5f820330e6e3", 755730897,
					"localization.txt", "519f041b1ebf0c72c455d9747f9b12d2", 7412), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_2),

	// Shaban (English) (Demo)
	WME_WINENTRY("shaban", "Demo",
		WME_ENTRY2s("data.dcp", "7586c35bc43d06553a10e18a5a576fcc", 338067285,
					"localization.txt", "16b02da5787e433dd0461b0ad855380c", 5844), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_2),

	// Shaban (German) (Demo)
	WME_WINENTRY("shaban", "Demo",
		WME_ENTRY2s("data.dcp", "044d4ad8b9a079a99892fd7f7673076c", 181051276,
					"localization.txt", "2d20495860b7abd1c9190d425021ca5c", 6329), Common::DE_DEU, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_2),

	// Shaban (Spanish) (Demo)
	WME_WINENTRY("shaban", "Demo",
		WME_ENTRY2s("data.dcp", "044d4ad8b9a079a99892fd7f7673076c", 181051276,
					"localization.txt", "bb5ece9c042b031838549a2de215dc9c", 6233), Common::ES_ESP, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_2),

	// Shaban (French) (Demo)
	WME_WINENTRY("shaban", "Demo",
		WME_ENTRY2s("data.dcp", "044d4ad8b9a079a99892fd7f7673076c", 181051276,
					"localization.txt", "8f43b60d735ceee0fdb49cd86291596d", 6274), Common::FR_FRA, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_2),

	// Shaban (Italian) (Demo)
	WME_WINENTRY("shaban", "Demo",
		WME_ENTRY2s("data.dcp", "044d4ad8b9a079a99892fd7f7673076c", 181051276,
					"localization.txt", "ffc7629568700d030602c9b065e27ce4", 5841), Common::IT_ITA, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_2),

	// Shaban (Japanese) (Demo)
	WME_WINENTRY("shaban", "Demo",
		WME_ENTRY2s("data.dcp", "044d4ad8b9a079a99892fd7f7673076c", 181051276,
					"localization.txt", "cc1e9eb0ed1e78ff84439a4fdfa9cbf5", 6816), Common::JA_JPN, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_2),

	// Shaban (Dutch) (Demo)
	WME_WINENTRY("shaban", "Demo",
		WME_ENTRY2s("data.dcp", "044d4ad8b9a079a99892fd7f7673076c", 181051276,
					"localization.txt", "ce2330fad0157b4ad247d72e812cb8a0", 5806), Common::NL_NLD, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_2),

	// Shaban (Portuguese) (Demo)
	WME_WINENTRY("shaban", "Demo",
		WME_ENTRY2s("data.dcp", "044d4ad8b9a079a99892fd7f7673076c", 181051276,
					"localization.txt", "e69948ef75907fa591f4062b05470012", 5919), Common::PT_BRA, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_2),

	// The Shine of a Star
	WME_WINENTRY("shinestar", "",
		WME_ENTRY1s("data.dcp", "f05abe9e2427a5e4f73648fa09c4ba8e", 94113060), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// Sofia's Debt (English)
	WME_WINENTRY("sofiasdebt", "",
		WME_ENTRY1s("SD.exe", "e9515f9ba1a2925bb6733476a826a650", 9915047), Common::EN_ANY, ADGF_UNSTABLE, WME_1_1_39),

	// Sofia's Debt (English)
	WME_WINENTRY("sofiasdebt", "",
		WME_ENTRY1s("Sofia's Debt.exe", "e9515f9ba1a2925bb6733476a826a650", 9915047), Common::EN_ANY, ADGF_UNSTABLE, WME_1_1_39),

	// Sofia's Debt (Spanish)
	// NOTE: dcp is bundled at the end of executable, so the file header checksum is the same, but the size is different
	WME_WINENTRY("sofiasdebt", "",
		WME_ENTRY1s("La Deuda de Sofia.exe", "e9515f9ba1a2925bb6733476a826a650", 9844976), Common::ES_ESP, ADGF_UNSTABLE, WME_1_1_39),

	// Space Invaders (Demo)
	WME_WINENTRY("spaceinvaders", "Demo",
		WME_ENTRY1s("data.dcp", "3f27adefdf72f2c1601cf555c80a509f", 1308361), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_3),

	// Space Madness
	WME_WINENTRY("spacemadness", "1.0.2",
		WME_ENTRY1s("data.dcp", "b9b83135dc7a9e1b4b5f50195dbeb630", 39546622), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// Space Madness (Demo) (English)
	WME_WINENTRY("spacemadness", "Demo",
		WME_ENTRY2s("data.dcp", "5c88a51d010ad24225dee432ed38b238", 25987377,
					"lang-english.dcp", "1e224f3f50cc9c1f91a4aedae1700788", 586837), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// Space Madness (Demo) (German)
	WME_WINENTRY("spacemadness", "Demo",
		WME_ENTRY2s("data.dcp", "5c88a51d010ad24225dee432ed38b238", 25987377,
					"lang-german.dcp", "78d06de2b4a8c68517eb6df3ded86d82", 1524330), Common::DE_DEU, ADGF_UNSTABLE, WME_1_9_1),

	// Tanya Grotter and the Magical Double Bass
	WME_WINENTRY("tanya1", "",
		WME_ENTRY1s("data.dcp", "035bbdaff078cc4053ecf4b518c0d0fd", 1007507786), Common::RU_RUS, ADGF_UNSTABLE, WME_1_8_0),

	// Tanya Grotter and the Disappearing Floor
	WME_WINENTRY("tanya2", "",
		WME_ENTRY1s("data.dcp", "9c15f14990f630177e063da885d03e6d", 936959767), Common::RU_RUS, ADGF_UNSTABLE, WME_1_8_3),

	// The Ancient Mark - Episode 1
	WME_WINENTRY("theancientmark1", "",
		WME_ENTRY1s("data.dcp", "ca04c26f03b2bd307368b306b297ddd7", 364664692), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// The Box
	WME_WINENTRY("thebox", "",
		WME_ENTRY1s("data.dcp", "ec5f0c7e8174e307701447b53afe7e2f", 108372483), Common::EN_ANY, ADGF_UNSTABLE, WME_1_8_11),

	// The Kite (Version 1.1)
	WME_WINENTRY("thekite", "Version 1.1",
		WME_ENTRY1s("data.dcp", "92d29428f464469bda2d81b03d4d5c3e", 47332296), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// The Kite (Version 1.2.e) (English)
	WME_WINENTRY("thekite", "Version 1.2.e",
		WME_ENTRY1s("data.dcp", "92451578b1bdd2b32a1db592a4f6d5fc", 47360539), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// The Kite (Version 1.2.i) (Italian)
	WME_WINENTRY("thekite", "Version 1.2.i",
		WME_ENTRY1s("data.dcp", "d3435b106a1b3b4c1df8ad596d271586", 47509274), Common::IT_ITA, ADGF_UNSTABLE, WME_1_9_1),

	// The Kite (Version 1.2.r) (Russian)
	WME_WINENTRY("thekite", "Version 1.2.r",
		WME_ENTRY1s("data.dcp", "d531e097dd884737469da014ed882cde", 47554582), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_1),

	// The Kite (Version 1.3.e) (English)
	WME_WINENTRY("thekite", "Version 1.3.e",
		WME_ENTRY1s("data.dcp", "9761827b51370263b7623721545d7627", 47382987), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// The Kite (Version 1.4.e) (English)
	WME_WINENTRY("thekite", "Version 1.4.e",
		WME_ENTRY1s("data.dcp", "b7b72a291c614ad45ae6a4e9c8d55f97", 47240677), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// The Kite (Version 1.4.f) (French)
	WME_WINENTRY("thekite", "Version 1.4.f",
		WME_ENTRY1s("data.dcp", "a1e2e8a1a0c54b2bd3f7a3b628e9483c", 47341826), Common::FR_FRA, ADGF_UNSTABLE, WME_1_9_1),

	// The Kite (Version 1.4.i) (Italian)
	WME_WINENTRY("thekite", "Version 1.4.i",
		WME_ENTRY1s("data.dcp", "69a58ff96306ec80114e0c4b3339a405", 47312894), Common::IT_ITA, ADGF_UNSTABLE, WME_1_9_1),

	// The Kite (Version 1.4.r) (Russian)
	WME_WINENTRY("thekite", "Version 1.4.r",
		WME_ENTRY1s("data.dcp", "16618c3488b07fae5d00fb94c408d2b8", 47472358), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_1),

	// Fairy Tales About Toshechka and Boshechka
	WME_WINENTRY("tib", "",
		WME_ENTRY1s("data.dcp", "87d296ef3f46570ed18f000d3885db77", 340264526), Common::RU_RUS, ADGF_UNSTABLE, WME_1_7_2),

	// The Trader of Stories (Demo)
	WME_WINENTRY("tradestory", "Demo",
		WME_ENTRY1s("data.dcp", "0a0b51191636cc8ead89b905281c3218", 40401902), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_0),

	// the white chamber (Version 1.7 - Definitive Edition) (built 2008.06.26) (multi-language)
	WME_WINENTRY("twc", "Definitive Edition",
		WME_ENTRY1s("data.dcp", "325abfaeb5fbfcc30d91296f1390a454", 186451273), Common::UNK_LANG, ADGF_UNSTABLE, WME_1_8_6),

	// the white chamber (Version 1.7 - Definitive Edition) (built 2008.06.29) (multi-language)
	WME_WINENTRY("twc", "Definitive Edition",
		WME_ENTRY1s("data.dcp", "0011d01142547c61e51ba24dc42b579e", 186451273), Common::UNK_LANG, ADGF_UNSTABLE, WME_1_8_6),

	// Vsevolod Prologue (Demo)
	WME_WINENTRY("vsevolod", "Prologue",
		WME_ENTRY1s("data.dcp", "f2dcffd2692dbfcc9371fa1a87970fe7", 388669493), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_1),

	// War
	WME_WINENTRY("war", "",
		WME_ENTRY1s("data.dcp", "003e317cda6d0137bbd5e5d7f089ee4d", 32591890), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),

	// Wintermute Engine Technology Demo (1.2)
	WME_WINENTRY("wmedemo", "1.2",
		WME_ENTRY1s("data.dcp", "511f447de05211a4dc979fafce74c046", 6031195), Common::EN_ANY, ADGF_UNSTABLE, WME_1_7_0),

	// Wilma Tetris
	WME_WINENTRY("wtetris", "",
		WME_ENTRY1s("data.dcp", "946e3a0496e6c12fb344c9ed861ff015", 2780093), Common::EN_ANY, ADGF_UNSTABLE, WME_1_1_39),

	// Wilma Tetris
	WME_WINENTRY("wtetris", "",
		WME_ENTRY1s("wtetris.exe", "fb89ec8c4fa2279120a8939449c0bda9", 3918669), Common::EN_ANY, ADGF_UNSTABLE, WME_1_1_39),

	// Zilm: A Game of Reflex 1.0
	WME_WINENTRY("Zilm", "1.0",
		WME_ENTRY1s("data.dcp", "098dffaf03d8adbb4cb5633e4733e63c", 351726), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),

	// Zbang! The Game (Demo)
	// NOTE: This is a 2.5D game that is out of ScummVM scope
	WME_WINENTRY("zbang", "0.89",
		WME_ENTRY1s("data.dcp", "db9101f08d12ab95c81042d154bb0ea8", 7210044), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_7_1),
	{
		AD_TABLE_END_MARKER,
		LATEST_VERSION
	}
};

} // End of namespace Wintermute

#undef WEM_ENTRY1s
#undef WEM_ENTRY2s
#undef WEM_ENTRY3s
#undef WME_WINENTRY
#undef WME_PLATENTRY
