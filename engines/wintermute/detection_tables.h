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

namespace Wintermute {

#define GAMEOPTION_SHOW_FPS          GUIO_GAMEOPTIONS1
#define GAMEOPTION_BILINEAR          GUIO_GAMEOPTIONS2
#ifdef ENABLE_WME3D
#define GAMEOPTION_FORCE_2D_RENDERER GUIO_GAMEOPTIONS3
#endif

static const PlainGameDescriptor wintermuteGames[] = {
	{"5ld",             "Five Lethal Demons"},
	{"5ma",             "Five Magical Amulets"},
	{"8squares",        "Eight Squares in The Garden"},
	{"actualdest",      "Actual Destination"},
	{"agustin",         "Boredom of Agustin Cordes"},
	{"alavi",           "Alavi Detective - Murder of Miss Rojan"},
	{"alimardan1",      "Alimardan's Mischief"},
	{"alimardan2",      "Alimardan Meets Merlin"},
	{"alone",           "Alone, Under Strange Night"},
	{"alphapolaris",    "Alpha Polaris"},
	{"apeiron",         "Apeiron"},
	{"artofmurder1",    "Art of Murder 1: FBI Confidential"},
	{"awakening",       "Awakening: Burning Ashes"},
	{"barbapoca1",      "El Engaño de Barbapoca Chapter 1: Esta Pizza Esta de Muerte"},
	{"barbapoca2",      "El Engaño de Barbapoca Chapter 2: Competencia Fantasmal"},
	{"barrowhilldp",    "Barrow Hill - The Dark Path"},
	{"basisoctavus",    "Basis Octavus"},
	{"bickadoodle",     "Bickadoodle"},
	{"bookmania",       "BookMania"},
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
	{"carolreed15",     "Carol Reed 15 - Geospots"},
	{"carolreed16",     "Carol Reed 16 - Quarantine Diary"},
	{"chaos",           "Chaos"},
	{"chivalry",        "Chivalry is Not Dead"},
	{"colapso1",        "Colapso: Episode 1"},
	{"colorsoncanvas",  "Colors on Canvas"},
	{"conspiracao",     "Conspiracao Dumont"},
	{"corrosion",       "Corrosion: Cold Winter Waiting"},
	{"darkfallls",      "Dark Fall: Lost Souls"},
	{"darksummer",      "Twilight: Dark Summer"},
	{"deadcity",        "Dead City"},
	{"devilincapital",  "Devil In The Capital"},
	{"dfafadventure",   "DFAF Adventure"},
	{"dirtysplit",      "Dirty Split"},
	{"drbohus",         "Dr. Bohus"},
	{"drdoylemotch",    "Dr. Doyle - Mystery Of The Cloche Hat"},
	{"dreamcat",        "Dreamcat"},
	{"dreaming",        "Des Reves Elastiques Avec Mille Insectes Nommes Georges"},
	{"dreams",          "Dreams"},
	{"dreamscape",      "Dreamscape"},
	{"driller",         "The Driller Incident"},
	{"erinmyers",       "The Death of Erin Myers"},
	{"escapemansion",   "Escape from the Mansion"},
	{"everydaygray",    "Everyday Grey"},
	{"facenoir",        "Face Noir"},
	{"findinghope",     "Finding Hope"},
	{"forgottensound1", "Forgotten Sound 1 - Revelation"},
	{"forgottensound2", "Forgotten Sound 2 - Destiny"},
	{"four",            "Four"},
	{"foxtail",         "FoxTail"},
	{"framed",          "Framed"},
	{"fred",            "Fred"},
	{"ghostsheet",      "Ghost in the Sheet"},
	{"goldencalf",      "The Golden Calf"},
	{"guttenburg",      "The Guttenburg Project"},
	{"hamlet",          "Hamlet or the last game without MMORPG features, shaders and product placement"},
	{"helga",           "Helga Deep In Trouble"},
	{"hor",             "Hor"},
	{"idiotstale",      "The Idiot's Tale"},
	{"imustkill",       "I Must Kill...: Fresh Meat"},
	{"jamesperis",      "James Peris: No License Nor Control"},
	{"klaymen1",        "Klaymen Episodes: Pilot"},
	{"knossos",         "K'NOSSOS"},
	{"kulivocko",       "Kulivocko"},
	{"lifein3minutes",  "Life In 3 Minutes"},
	{"lonelyrobot",     "Project Lonely Robot"},
	{"looky",           "Looky"},
	{"lovmamuta",       "Lov Mamuta"},
	{"julia",           "J.U.L.I.A."},
	{"juliastars",      "J.U.L.I.A.: Among the Stars"},
	{"juliauntold",     "J.U.L.I.A.: Untold"},
	{"lotl",            "Limbo of the Lost"},
	{"machumayu",       "Machu Mayu"},
	{"mentalrepairs",   "Mental Repairs Inc"},
	{"mirage",          "Mirage"},
	{"miskatonic1",     "Miskatonic. Part One"},
	{"msos",            "Monday Starts on Saturday"},
	{"mukhtar",         "Mukhtar and his Team"},
	{"mystictriddle",   "Mystic Triddle"},
	{"mythguff",        "Myth: A Guff's Tale"},
	{"nightinthefog",   "Night in the Fog"},
	{"nighttrain",      "Night Train"},
	{"nosebound1",      "Nose Bound: Episode 1"},
	{"octave",          "Octave"},
	{"oknytt",          "Oknytt"},
	{"one",             "One"},
	{"onehelluvaday",   "One Helluva Day"},
	{"openquest",       "Open Quest"},
	{"paintaria",       "Paintaria"},
	{"palladion",       "Palladion"},
	{"papasdaughters1", "Papa's Daughters"},
	{"papasdaughters2", "Papa's Daughters Go to the Sea"},
	{"petka02",         "Red Comrades 0.2: Operation F."},
	{"pigeons",         "Pigeons in the Park"},
	{"pizzamorgana",    "Pizza Morgana: Episode 1 - Monsters and Manipulations in the Magical Forest"},
	{"polechudes",      "Pole Chudes"},
	{"preciouspills",   "Precious Pills, Blasting Bacon and Mowing Minds"},
	{"projectdoom",     "Project: Doom"},
	{"projectjoe",      "Project Joe"},
	{"qajarycat",       "Qajary Cat"},
	{"rebeccacarlson1", "Rebecca Carlson Mystery 01 - Silent Footsteps"},
	{"reptilesquest",   "On the Tracks of Dinosaurs"},
	{"reversion1",      "Reversion: The Escape"},
	{"reversion2",      "Reversion: The Meeting"},
	{"reversion3",      "Reversion: The Return"},
	{"rhiannon",        "Rhiannon: Curse of the four Branches"},
	{"ritter",          "1 1/2 Ritter: Auf der Suche nach der hinreissenden Herzelinde"},
	{"rosemary",        "Rosemary"},
	{"royalmahjong",    "Royal Mahjong: King's Journey"},
	{"satanandsons",    "Satan and Sons"},
	{"securanote",      "Securanote"},
	{"shaban",          "Shaban"},
	{"shadowofnebula",  "Shadow Of Nebula"},
	{"shelter",         "Shelter"},
	{"shinestar",       "The Shine of a Star"},
	{"spaceinvaders",   "Space Invaders"},
	{"spacemadness",    "Space Madness"},
	{"sof1",            "Stroke of Fate: Operation Valkyrie"},
	{"sof2",            "Stroke of Fate: Operation Bunker"},
	{"sofiasdebt",      "Sofia's Debt"},
	{"sotv1",           "Shadows on the Vatican - Act I: Greed"},
	{"sotv2",           "Shadows on the Vatican - Act II: Wrath"},
	{"strangechange",   "Strange Change"},
	{"sunny",           "Sunny"},
	{"sunrise",         "Sunrise: The game"},
	{"susanrose1",      "Susan Rose: Mysterious Child"},
	{"susanrose2",      "Susan Rose: Delicate Murder"},
	{"tanya1",          "Tanya Grotter and the Magical Double Bass"},
	{"tanya2",          "Tanya Grotter and the Disappearing Floor"},
	{"tehran1933",      "Murder In Tehran's Alleys 1933"},
	{"tehran2016",      "Murder In Tehran's Alleys 2016"},
	{"theancientmark1", "The Ancient Mark - Episode 1"},
	{"tetriks",         "TeTRIks"},
	{"thebox",          "The Box"},
	{"thekite",         "The Kite"},
	{"thelastcrownmh",  "The Last Crown - Midnight Horror"},
	{"thelostcrowngha", "The Lost Crown - A Ghost-Hunting Adventure"},
	{"tib",             "Fairy Tales About Toshechka and Boshechka"},
	{"todaymama",       "Today, Mama!"},
	{"tradestory",      "The Trader of Stories"},
	{"twc",             "The White Chamber"},
	{"war",             "War"},
	{"vsevolod",        "Vsevolod"},
	{"wayoflove",       "The Way Of Love: Sub Zero"},
	{"wintermute",      "Wintermute engine game"},
	{"wmedemo",         "Wintermute Engine Technology Demo"},
	{"wmedemo3d",       "Wintermute 3D Characters Technology Demo"},
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
					"data.dcp", "1037a77cbd001e0644898addc022322c", 15407750), Common::CS_CZE, ADGF_UNSTABLE, WME_1_6_1),

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
					"data.dcp", "0134e92bcd5fd2837df3971087e96067", 163316498), Common::CS_CZE, ADGF_UNSTABLE, WME_1_3_3),

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

	// Eight Squares in The Garden
	WME_WINENTRY("8squares", "",
		WME_ENTRY1s("data.dcp", "8788577bffe33ae059e13aa862dabd31", 29931982), Common::EN_ANY, ADGF_UNSTABLE, WME_1_8_6),

	// Actual Destination
	WME_WINENTRY("actualdest", "",
		WME_ENTRY1s("data.dcp", "6926f44b26f21ceb1d840eaab9aeb510", 9081740), Common::EN_ANY, ADGF_UNSTABLE, WME_1_8_11),

	// Alavi Detective - Murder of Miss Rojan (Persian) (Beta version -old version-, between 1387/12 to 1392/6)
	WME_WINENTRY("alavi", "Beta version",
		WME_ENTRY1s("data.dcp", "587b46baa3f82dfe09bdb51f01231fa2", 37191939), Common::FA_IRN, ADGF_UNSTABLE | ADGF_DEMO | GF_3D, WME_ANDISHE_VARAN),

	// Alavi Detective - Murder of Miss Rojan (Persian) (Try version, between 1388/8)
	WME_WINENTRY("alavi", "Try version",
		WME_ENTRY1s("data.dcp", "07b847ebafbf8e58daa341d60598c84b", 36628230), Common::FA_IRN, ADGF_UNSTABLE | ADGF_DEMO | GF_3D, WME_ANDISHE_VARAN),

	// Alavi Detective - Murder of Miss Rojan (Hayoola) (Persian)
	WME_WINENTRY("alavi", "",
		WME_ENTRY1s("data.dcp", "bffb07282d27b6c5351ed4fce92db4da", 516186290), Common::FA_IRN, ADGF_UNSTABLE | GF_3D, WME_ANDISHE_VARAN),

	// Alimardan's Mischief (Retail) (Persian)
	WME_WINENTRY("alimardan1", "",
		WME_ENTRY1s("data.dcp", "ab1aa5f3d6d5f10d3f131a78be80bf34", 1747086434), Common::FA_IRN, ADGF_UNSTABLE, WME_1_9_1),

	// Alimardan's Mischief (English)
	WME_WINENTRY("alimardan1", "",
		WME_ENTRY1s("data.dcp", "55c28b8d4f49120f980d237951a6c73f", 1834957694), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// Alimardan's Mischief (Steam, August 2017) (English)
	WME_WINENTRY("alimardan1", "",
		WME_ENTRY1s("data.dcp", "ef27d8483d6f19d5398fc81ea05e5320", 1258766332), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Alimardan's Mischief (Steam, October 2017) (English)
	WME_WINENTRY("alimardan1", "",
		WME_ENTRY1s("data.dcp", "d74f56d9ce3b7442b921393af7fb6167", 1258761838), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Alimardan Meets Merlin (Hayoola) (Persian)
	WME_WINENTRY("alimardan2", "",
		WME_ENTRY1s("data.dcp", "10f3f0e1b05187ab99fca2ad026c51c2", 412587946), Common::FA_IRN, ADGF_UNSTABLE, WME_1_9_3),

	// Alimardan Meets Merlin (Steam) (English)
	WME_WINENTRY("alimardan2", "",
		WME_ENTRY1s("data.dcp", "e3ffb5d177737a57bb391995d9361055", 413397623), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Alone, Under Strange Night (English)
	WME_WINENTRY("alone", "",
		WME_ENTRY1s("data.dcp", "af7638e15b5eeeedb92768d32e02e7f4", 66172118), Common::EN_ANY, ADGF_UNSTABLE, WME_1_8_5),

	// Alpha Polaris (English)
	WME_WINENTRY("alphapolaris", "",
		WME_ENTRY1s("data.dcp", "6d5d2264a3135dae3a9a74de50b4ea68", 706780438), Common::EN_ANY, ADGF_UNSTABLE | GF_3D, WME_1_9_1),

	// Alpha Polaris (Polish)
	WME_WINENTRY("alphapolaris", "",
		WME_ENTRY1s("data.dcp", "24ebca07b7cf0bd94ec994f26bdccf83", 710319064), Common::PL_POL, ADGF_UNSTABLE | GF_3D, WME_1_9_1),

	// Alpha Polaris (Russian)
	WME_WINENTRY("alphapolaris", "",
		WME_ENTRY1s("data.dcp", "dd8c252e039cd3a935c7490614e5e6f6", 706780433), Common::RU_RUS, ADGF_UNSTABLE | GF_3D, WME_1_9_1),

	// Alpha Polaris (English) (1.1.0) (1280x800 Steam)
	WME_WINENTRY("alphapolaris", "1.1.0 (1280x800 Steam)",
		WME_ENTRY2s("data.dcp", "2d1bd35749b2538915b53ce79288bceb", 706580987,
					"data.dcp", "2d1bd35749b2538915b53ce79288bceb", 706580987), Common::EN_ANY, ADGF_UNSTABLE | GF_3D, WME_1_9_3),

	// Alpha Polaris (German) (1.1.0) (1280x800 Steam)
	WME_WINENTRY("alphapolaris", "1.1.0 (1280x800 Steam)",
		WME_ENTRY2s("data.dcp", "2d1bd35749b2538915b53ce79288bceb", 706580987,
					"german_language_pack.dcp", "0ed4ef1647445c73b5915b60f85ed8e4", 19251966), Common::DE_DEU, ADGF_UNSTABLE | GF_3D, WME_1_9_3),

	// Alpha Polaris (Polish) (1.1.0) (1280x800 Steam)
	WME_WINENTRY("alphapolaris", "1.1.0 (1280x800 Steam)",
		WME_ENTRY2s("data.dcp", "2d1bd35749b2538915b53ce79288bceb", 706580987,
					"polish_language_pack.dcp", "91f80c5f8d522541d666d11b60b0ea6c", 15006039), Common::PL_POL, ADGF_UNSTABLE | GF_3D, WME_1_9_3),

	// Alpha Polaris (Russian) (1.1.0) (1280x800 Steam)
	WME_WINENTRY("alphapolaris", "1.1.0 (1280x800 Steam)",
		WME_ENTRY2s("data.dcp", "2d1bd35749b2538915b53ce79288bceb", 706580987,
					"russian_language_pack.dcp", "58575db652d371af537b4b8841e962f8", 49395113), Common::RU_RUS, ADGF_UNSTABLE | GF_3D, WME_1_9_3),

	// Alpha Polaris (English) (1.1.0) (1280x768 Steam)
	WME_WINENTRY("alphapolaris", "1.1.0 (1280x768 Steam)",
		WME_ENTRY2s("data.dcp", "481b1bff44178ef2f3d879df05f65a96", 706580987,
					"data.dcp", "481b1bff44178ef2f3d879df05f65a96", 706580987), Common::EN_ANY, ADGF_UNSTABLE | GF_3D, WME_1_9_3),

	// Alpha Polaris (German) (1.1.0) (1280x768 Steam)
	WME_WINENTRY("alphapolaris", "1.1.0 (1280x768 Steam)",
		WME_ENTRY2s("data.dcp", "481b1bff44178ef2f3d879df05f65a96", 706580987,
					"german_language_pack.dcp", "2ffd9b2bdf2c2b3646067644572390c0", 19251966), Common::DE_DEU, ADGF_UNSTABLE | GF_3D, WME_1_9_3),

	// Alpha Polaris (Polish) (1.1.0) (1280x768 Steam)
	WME_WINENTRY("alphapolaris", "1.1.0 (1280x768 Steam)",
		WME_ENTRY2s("data.dcp", "481b1bff44178ef2f3d879df05f65a96", 706580987,
					"polish_language_pack.dcp", "4a24986189321f39b9f48cbc4889d89a", 15006039), Common::PL_POL, ADGF_UNSTABLE | GF_3D, WME_1_9_3),

	// Alpha Polaris (Russian) (1.1.0) (1280x768 Steam)
	WME_WINENTRY("alphapolaris", "1.1.0 (1280x768 Steam)",
		WME_ENTRY2s("data.dcp", "481b1bff44178ef2f3d879df05f65a96", 706580987,
					"russian_language_pack.dcp", "f39ad478a711fa4b34d419ed4aac97bf", 49395113), Common::RU_RUS, ADGF_UNSTABLE | GF_3D, WME_1_9_3),

	// Alpha Polaris (English Demo)
	WME_WINENTRY("alphapolaris", "Demo",
		WME_ENTRY2s("data.dcp", "68f93edfb69de8f8c06c81566f279e07", 409562809,
					"data.dcp", "68f93edfb69de8f8c06c81566f279e07", 409562809), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO | GF_3D, WME_1_9_3),

	// Alpha Polaris (German Demo)
	WME_WINENTRY("alphapolaris", "Demo",
		WME_ENTRY2s("data.dcp", "130545e152d0b53d84c2c99ce118d5e5", 409564920,
					"german language pack.dcp", "71e19682f93399018bef90ceda0a1bfd", 16174995), Common::DE_DEU, ADGF_UNSTABLE | ADGF_DEMO | GF_3D, WME_1_9_3),

	// Alpha Polaris (Polish Demo)
	WME_WINENTRY("alphapolaris", "Demo",
		WME_ENTRY1s("data.dcp", "c040af58c38c40997fce4b4832f43a5d", 416066977), Common::PL_POL, ADGF_UNSTABLE | ADGF_DEMO | GF_3D, WME_1_9_1),

	// Apeiron
	WME_WINENTRY("apeiron", "",
		WME_ENTRY1s("data.dcp", "c0d6190de52b3da9995cbd4b78976bc8", 89912566), Common::SE_SWE, ADGF_UNSTABLE, WME_1_9_1),

	// Art of Murder: FBI Confidential (English)
	WME_WINENTRY("artofmurder1", "",
		WME_ENTRY2s("data.dcp", "7e4c1dc8b1fb08541f7784d6288acfc8", 633692059,
					"us.dcp", "4a02b65edc45444ac69ff8a514e952b8", 176592548), Common::EN_ANY, ADGF_UNSTABLE | GF_3D, WME_1_8_1),

	// Art of Murder: FBI Confidential (French)
	WME_WINENTRY("artofmurder1", "",
		WME_ENTRY2s("data.dcp", "bba4e56a66fd6406a26515cfc86ac125", 23782002,
					"fr.dcp", "5665d84d70cb3e20472e1c3eb9d884c3", 125728694), Common::FR_FRA, ADGF_UNSTABLE | GF_3D, WME_1_8_1),

	// Art of Murder: FBI Confidential (German)
	WME_WINENTRY("artofmurder1", "",
		WME_ENTRY2s("data.dcp", "09e57d77b68dafa73a0924f11f61b059", 630742753,
					"i18n.dcp", "583940f6d3fb4097e7cb1e2cc9a43a7b", 156078991), Common::DE_DEU, ADGF_UNSTABLE | GF_3D, WME_1_8_1),

	// Art of Murder: FBI Confidential (Italian)
	WME_WINENTRY("artofmurder1", "",
		WME_ENTRY2s("data.dcp", "7e4c1dc8b1fb08541f7784d6288acfc8", 633692059,
					"it.dcp", "c3c10399644a1e9f7e977df73fb017e0", 166794517), Common::IT_ITA, ADGF_UNSTABLE | GF_3D, WME_1_8_1),

	// Art of Murder: FBI Confidential (Spanish)
	WME_WINENTRY("artofmurder1", "",
		WME_ENTRY2s("data.dcp", "340f04f9f66a0ab978b78d317544bbed", 23757600,
					"es.dcp", "47b3a609993b3c18ce5bfb1af734ea3e", 148997124), Common::ES_ESP, ADGF_UNSTABLE | GF_3D, WME_1_8_1),

	// Art of Murder: FBI Confidential (Polish)
	WME_WINENTRY("artofmurder1", "",
		WME_ENTRY2s("data.dcp", "592862a5dd1ae90d53561815a535ab2e", 630734278,
					"i18n.dcp", "b43bd7b64991ad9d5d285753767fe3f4", 176591837), Common::PL_POL, ADGF_UNSTABLE | GF_3D, WME_1_8_1),

	// Art of Murder: FBI Confidential (Russian)
	WME_WINENTRY("artofmurder1", "",
		WME_ENTRY2s("data.dcp", "4f383bd02fb0eea54e9b5825c3056b52", 23761530,
					"ru.dcp", "57af214554547437c823a01f6cf51b24", 201702190), Common::RU_RUS, ADGF_UNSTABLE | GF_3D, WME_1_8_3),

	// Art of Murder: FBI Confidential (English Demo)
	WME_WINENTRY("artofmurder1", "Demo",
		WME_ENTRY2s("data.dcp", "d0073ddf2b25527c83785ae7a0978867", 47676670,
					"data.dcp", "d0073ddf2b25527c83785ae7a0978867", 47676670), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO | GF_3D, WME_1_8_1),

	// Art of Murder: FBI Confidential (Polish Demo)
	WME_WINENTRY("artofmurder1", "Demo",
		WME_ENTRY2s("data.dcp", "d0073ddf2b25527c83785ae7a0978867", 47676670,
					"pl.dcp", "4dbc02aa9f67d22226d22dc0d837d20e", 49208405), Common::PL_POL, ADGF_UNSTABLE | ADGF_DEMO | GF_3D, WME_1_8_1),

	// Art of Murder: FBI Confidential (German Demo 1)
	WME_WINENTRY("artofmurder1", "Demo",
		WME_ENTRY2s("data.dcp", "d7f3dd0e87e4904292d19778b8af2ed1", 47662172,
					"de.dcp", "c0bbfee40b79af1f837a9f3f8fcef78c", 42741523), Common::DE_DEU, ADGF_UNSTABLE | ADGF_DEMO | GF_3D, WME_1_8_1),

	// Art of Murder: FBI Confidential (German Demo 2)
	WME_WINENTRY("artofmurder1", "Demo",
		WME_ENTRY2s("data.dcp", "99d63dfee4ea97d31530649c929bee45", 81127581,
					"de.dcp", "e97e2e18b26e5eff916c73f720d5f4f2", 17737492), Common::DE_DEU, ADGF_UNSTABLE | ADGF_DEMO | GF_3D, WME_1_8_1),

	// Awakening: Burning Ashes (Hayoola) (Persian)
	WME_WINENTRY("awakening", "",
		WME_ENTRY1s("data.dcp", "a7b8633fdc5cc8ff659dda5a11d3d971", 1900777616), Common::FA_IRN, ADGF_UNSTABLE | GF_3D, WME_1_9_3),

	// El Engaño de Barbapoca Chapter 1: Esta Pizza Esta de Muerte (Spanish)
	WME_WINENTRY("barbapoca1", "",
		WME_ENTRY1s("data.dcp", "efc555f57ec0db5d5106bc8aa0e7f627", 467198987), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_1),

	// El Engaño de Barbapoca Chapter 2: Competencia Fantasmal (Spanish)
	WME_WINENTRY("barbapoca2", "",
		WME_ENTRY1s("data.dcp", "746b3264178607121169fa32d48f5368", 1256448570), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_1),

	// Barrow Hill - The Dark Path (Steam, September 2018) (English)
	WME_WINENTRY("barrowhilldp", "",
		WME_ENTRY1s("data.dcp", "6b7c8f8185a80c50a71e068aad82862e", 1066752), Common::EN_ANY, ADGF_UNSTABLE | GF_3D, WME_1_9_3),

	// Barrow Hill - The Dark Path (Steam, September 2018) (German)
	WME_WINENTRY("barrowhilldp", "",
		WME_ENTRY1s("data.dcp", "958b89bd394bbaae17a3abe51eb00b5b", 1066776), Common::DE_DEU, ADGF_UNSTABLE | GF_3D, WME_1_9_3),

	// Barrow Hill - The Dark Path (GOG) (English)
	WME_WINENTRY("barrowhilldp", "",
		WME_ENTRY1s("data.dcp", "2b11a98ebc1afd07b4d03f650c32b12b", 1066752), Common::EN_ANY, ADGF_UNSTABLE | GF_3D, WME_1_9_3),

	// Basis Octavus
	WME_WINENTRY("basisoctavus", "",
		WME_ENTRY1s("data.dcp", "021ef97f8f49ec33f83beae0d6e38f08", 49336909), Common::CS_CZE, ADGF_UNSTABLE | GF_3D, WME_1_9_1),

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

	// Bickadoodle (https://web.archive.org/web/20150516210632/http://aethericgames.com/games/bickadoodle/download-bickadoodle/)
	WME_WINENTRY("bickadoodle", "",
		WME_ENTRY1s("data.dcp", "1584d83577c32add0fce27fae91141a2", 35337728), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// BookMania
	WME_WINENTRY("bookmania", "",
		WME_ENTRY1s("bookmania.exe", "2c7c950a7e7c5b3a30e83a6cd603789b", 5084057), Common::CS_CZE, ADGF_UNSTABLE, WME_1_9_1),

	// Book of Gron Part One
	WME_WINENTRY("bookofgron", "",
		WME_ENTRY1s("data.dcp", "e61b2ebee044a82fa0f8ca0fce2c8946", 83129531), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_1),

	// Carol Reed 4 - East Side Story (English)
	WME_WINENTRY("carolreed4", "",
		WME_ENTRY1s("data.dcp", "b26377797f060afc2d440d820100c1ce", 529320536), Common::EN_ANY, ADGF_UNSTABLE, WME_1_7_2),

	// Carol Reed 4 - East Side Story (German)
	WME_WINENTRY("carolreed4", "",
		WME_ENTRY1s("data.dcp", "b3c30d1c6af3e4699a55ceffd4543f35", 602546719), Common::DE_DEU, ADGF_UNSTABLE, WME_1_8_3),

	// Carol Reed 4 - East Side Story (Spanish)
	WME_WINENTRY("carolreed4", "",
		WME_ENTRY1s("data.dcp", "ef8a5aaec2f754a04017a97fed771b43", 601616328), Common::ES_ESP, ADGF_UNSTABLE, WME_1_8_3),

	// Carol Reed 4 - East Side Story (Russian) (CD Navigator)
	WME_WINENTRY("carolreed4", "",
		WME_ENTRY1s("data.dcp", "549e6fb7afebcc4a8092e3898a5c5cc3", 618787625), Common::RU_RUS, ADGF_UNSTABLE, WME_1_8_3),

	// Carol Reed 4 - East Side Story (Russian)
	WME_WINENTRY("carolreed4", "",
		WME_ENTRY1s("data.dcp", "a583bedf9345b55866ad84c84a1166f8", 501910422), Common::RU_RUS, ADGF_UNSTABLE, WME_1_8_3),

	// Carol Reed 4 - East Side Story (Demo) (English)
	WME_WINENTRY("carolreed4", "Demo",
		WME_ENTRY1s("data.dcp", "b3f8b09bb4b05ee3e9d14697525257f9", 59296246), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_7_2),

	// Carol Reed 5 - The Colour of Murder (English)
	WME_WINENTRY("carolreed5", "",
		WME_ENTRY1s("data.dcp", "3fcfca44209545d0e26774156427b494", 603660415), Common::EN_ANY, ADGF_UNSTABLE, WME_1_8_6),

	// Carol Reed 5 - The Colour of Murder (Spanish)
	WME_WINENTRY("carolreed5", "",
		WME_ENTRY1s("data.dcp", "a2885bbc31a7f0b0f616ec7f4d9a6fae", 617467065), Common::ES_ESP, ADGF_UNSTABLE, WME_1_8_6),

	// Carol Reed 5 - The Colour of Murder (Russian)
	WME_WINENTRY("carolreed5", "",
		WME_ENTRY1s("data.dcp", "512b13a67fc5e1a965f19ab158c4a984", 616068459), Common::RU_RUS, ADGF_UNSTABLE, WME_1_8_6),

	// Carol Reed 5 - The Colour of Murder (1.0 Demo)
	WME_WINENTRY("carolreed5", "Demo",
		WME_ENTRY1s("data.dcp", "0b4cb2b9eb5322acb7d8701db95dc721", 92019500), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_8_6),

	// Carol Reed 5 - The Colour of Murder (1.0 Demo)
	WME_WINENTRY("carolreed5", "Demo",
		WME_ENTRY1s("data.dcp", "27b3efc018ade5ee8f4adf08b4e3c0dd", 92019500), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_8_6),

	// Carol Reed 6 - Black Circle
	WME_WINENTRY("carolreed6", "",
		WME_ENTRY1s("data.dcp", "0e4c532beecf23d85012168753f41189", 456258147), Common::EN_ANY, ADGF_UNSTABLE, WME_1_8_11),

	// Carol Reed 6 - Black Circle (BigFish)
	WME_WINENTRY("carolreed6", "",
		WME_ENTRY1s("data.dcp", "6df00158f120bfe1f6671df94064daa9", 456465173), Common::EN_ANY, ADGF_UNSTABLE, WME_1_8_11),

	// Carol Reed 6 - Black Circle (Merscom Games)
	WME_WINENTRY("carolreed6", "",
		WME_ENTRY1s("data.dcp", "050048bc81f700c8e22ed9d1ace6d853", 456267557), Common::EN_ANY, ADGF_UNSTABLE, WME_1_8_11),

	// Carol Reed 6 - Black Circle (Russian)
	WME_WINENTRY("carolreed6", "",
		WME_ENTRY1s("data.dcp", "bd28a7501b0b0cbf83add4b3f938feef", 451468707), Common::RU_RUS, ADGF_UNSTABLE, WME_1_8_11),

	// Carol Reed 6 - Black Circle (Demo)
	WME_WINENTRY("carolreed6", "Demo",
		WME_ENTRY1s("data.dcp", "e18bc4adc843e6dcf08cc20be00d1608", 94399373), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_8_11),

	// Carol Reed 7 - Blue Madonna
	WME_WINENTRY("carolreed7", "",
		WME_ENTRY1s("data.dcp", "24e3db3e2fabfc956713796d87a3efb0", 495471147), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// Carol Reed 7 - Blue Madonna (iWin.com / Digi Ronin Games)
	WME_WINENTRY("carolreed7", "",
		WME_ENTRY1s("data.dcp", "79610b483fc62a60687c6432ee54ef97", 495185438), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// Carol Reed 7 - Blue Madonna (Russian)
	WME_WINENTRY("carolreed7", "",
		WME_ENTRY1s("data.dcp", "5ce25bf0e2eec7774be99ebb06b25c81", 490985628), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_1),

	// Carol Reed 7 - Blue Madonna (Demo)
	WME_WINENTRY("carolreed7", "Demo",
		WME_ENTRY1s("data.dcp", "0372ad0c775266f6355e9e8ae397a2f1", 103719442), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_1),

	// Carol Reed 8 - Amber's Blood
	WME_WINENTRY("carolreed8", "",
		WME_ENTRY1s("data.dcp", "859d16b0d5b9b255e470cbded2c6cedc", 502714557), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// Carol Reed 8 - Amber's Blood (Russian)
	WME_WINENTRY("carolreed8", "",
		WME_ENTRY1s("data.dcp", "ce14d5c9fc31cfed934c688dc621f562", 501843631), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_1),

	// Carol Reed 8 - Amber's Blood (Demo)
	WME_WINENTRY("carolreed8", "Demo",
		WME_ENTRY1s("data.dcp", "e8c6b6e5d2e79bc87bf591f47202c233", 110106328), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_1),

	// Carol Reed 9 - Cold Case Summer
	WME_WINENTRY("carolreed9", "",
		WME_ENTRY1s("data.dcp", "2b343b48a7aee508d728a546b414a255", 620005266), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 9 - Cold Case Summer (Russian)
	WME_WINENTRY("carolreed9", "",
		WME_ENTRY1s("data.dcp", "21b97762d1621cf91a340245cdb6f58d", 643392604), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 9 - Cold Case Summer (Demo)
	WME_WINENTRY("carolreed9", "Demo",
		WME_ENTRY1s("data.dcp", "b3312b0b9f209a9a01b2ba9135829640", 86362217), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_3),

	// Carol Reed 10 - Bosch's Damnation (MDNA Games)
	WME_WINENTRY("carolreed10", "",
		WME_ENTRY1s("data.dcp", "5c698e28fe10854ea52964e3768e50e4", 588439599), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 10 - Bosch's Damnation (Steam)
	WME_WINENTRY("carolreed10", "Steam",
		WME_ENTRY1s("data.dcp", "dd1e858f2eac016eadccdeeb93589015", 588494380), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 10 - Bosch's Damnation (Russian)
	WME_WINENTRY("carolreed10", "",
		WME_ENTRY1s("data.dcp", "ab88da3f4df7f3c4d831f9bf57aa73fc", 586886303), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 10 - Bosch's Damnation (Demo)
	WME_WINENTRY("carolreed10", "Demo",
		WME_ENTRY1s("data.dcp", "72b29cb3f0bc83efa6e3bae41b4e899a", 79971080), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_3),

	// Carol Reed 11 - Shades Of Black (HD) (MDNA Games) (v1.0)
	WME_WINENTRY("carolreed11", "HD/1.0",
		WME_ENTRY1s("data.dcp", "e4080bac9e34d59ed25ae689776f0d15", 853204297), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 11 - Shades Of Black (SD) (MDNA Games) (v1.0)
	WME_WINENTRY("carolreed11", "SD/1.0",
		WME_ENTRY1s("data.dcp", "a1e9f54b48493e9c6d737e3a94779d8c", 525450069), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 11 - Shades Of Black (HD) (MDNA Games) (v1.0+patch1)
	WME_WINENTRY("carolreed11", "HD/1.0+patch1",
		WME_ENTRY2s("data.dcp", "e4080bac9e34d59ed25ae689776f0d15", 853204297,
					"patch1.dcp", "12e6487284bdd12c9c3d0c57d84ceac1", 3206), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 11 - Shades Of Black (SD) (MDNA Games) (v1.0+patch1)
	WME_WINENTRY("carolreed11", "SD/1.0+patch1",
		WME_ENTRY2s("data.dcp", "a1e9f54b48493e9c6d737e3a94779d8c", 525450069,
					"patch1.dcp", "12e6487284bdd12c9c3d0c57d84ceac1", 3206), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 11 - Shades Of Black (HD) (MDNA Games) (v1.1)
	WME_WINENTRY("carolreed11", "HD/1.1",
		WME_ENTRY1s("data.dcp", "7ca1e654e293cb84312a56c6be4ada24", 853204303), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 11 - Shades Of Black (SD) (MDNA Games) (v1.1)
	WME_WINENTRY("carolreed11", "SD/1.1",
		WME_ENTRY1s("data.dcp", "47ab078a83a1ccc76a6446b2e8055857", 525450074), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 11 - Shades Of Black (HD) (Steam)
	WME_WINENTRY("carolreed11", "HD/Steam",
		WME_ENTRY1s("data.dcp", "459f4a851ea053cc4d204e45e2f88afc", 853256580), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 11 - Shades Of Black (SD) (Steam)
	WME_WINENTRY("carolreed11", "SD/Steam",
		WME_ENTRY1s("data.dcp", "c47314872afbbdc587dae0bc976a3375", 525502344), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 11 - Shades Of Black (HD) (Russian)
	WME_WINENTRY("carolreed11", "HD",
		WME_ENTRY1s("data.dcp", "ee927bacf2aa5955946b9ddfa32f84ba", 871145562), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 11 - Shades Of Black (SD) (Russian)
	WME_WINENTRY("carolreed11", "SD",
		WME_ENTRY1s("data.dcp", "58fd92c52ac5e2bd4d7cebedc8cae870", 431784607), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 11 - Shades Of Black (SD) (Demo)
	WME_WINENTRY("carolreed11", "SD Demo",
		WME_ENTRY1s("data.dcp", "8c27236d29747b6bb6a0a563fb769f20", 52692061), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_3),

	// Carol Reed 11 - Shades Of Black (HD) (Demo)
	WME_WINENTRY("carolreed11", "HD Demo",
		WME_ENTRY1s("data.dcp", "ba124af0a0ee3113f3f9dc0c1159ec26", 77909344), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_3),

	// Carol Reed 12 - Profound Red (HD) (MDNA Games)
	WME_WINENTRY("carolreed12", "HD",
		WME_ENTRY1s("data.dcp", "edc1e69dc52ec2a65fa431a51c783c2e", 944403699), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 12 - Profound Red (SD) (MDNA Games)
	WME_WINENTRY("carolreed12", "SD",
		WME_ENTRY1s("data.dcp", "84d296ab65b89e87f6d56accb9360f19", 627095895), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 12 - Profound Red (HD) (Russian)
	WME_WINENTRY("carolreed12", "HD",
		WME_ENTRY1s("data.dcp", "470b7eb5f79d0cfa3e5a58e40bca9420", 915944751), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 12 - Profound Red (SD) (Russian)
	WME_WINENTRY("carolreed12", "SD",
		WME_ENTRY1s("data.dcp", "c58e9a8aa8f87ebfae94a2fa3e07817e", 420274897), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 13 - The Birdwatcher (HD) (MDNA Games) (v1.0)
	WME_WINENTRY("carolreed13", "HD/1.0",
		WME_ENTRY1s("data.dcp", "2fe001cc3463eeca734630180743e3b0", 1268655366), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 13 - The Birdwatcher (SD) (MDNA Games) (v1.0)
	WME_WINENTRY("carolreed13", "SD/1.0",
		WME_ENTRY1s("data.dcp", "3c096a9c4e7adfae31e0916421505939", 774246800), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 13 - The Birdwatcher (HD) (MDNA Games) (v1.1)
	WME_WINENTRY("carolreed13", "HD/1.1",
		WME_ENTRY1s("data.dcp", "954392d19f0c928b700f17badd35d488", 1267391173), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 13 - The Birdwatcher (SD) (MDNA Games) (v1.1)
	WME_WINENTRY("carolreed13", "SD/1.1",
		WME_ENTRY1s("data.dcp", "4c5e1094a2e05b76b2ee5a8d9af5de4a", 774352150), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 13 - The Birdwatcher (HD) (MDNA Games) (v1.2)
	WME_WINENTRY("carolreed13", "HD/1.2",
		WME_ENTRY1s("data.dcp", "85cfc970e2b6aaa3f44d83febb0e8509", 1267524698), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 13 - The Birdwatcher (SD) (MDNA Games) (v1.2)
	WME_WINENTRY("carolreed13", "SD/1.2",
		WME_ENTRY1s("data.dcp", "d7b6aaa1643fec62645b176fe890fe1f", 774409229), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 13 - The Birdwatcher (HD) (Russian)
	WME_WINENTRY("carolreed13", "HD",
		WME_ENTRY1s("data.dcp", "495c87fcfcfa365f3239172acd362a46", 1361946376), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 13 - The Birdwatcher (SD) (Russian)
	WME_WINENTRY("carolreed13", "SD",
		WME_ENTRY1s("data.dcp", "d86676ba56bfac82d19ee63f39098eb7", 607269218), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 14 - The Fall Of April (HD) (MDNA Games)
	WME_WINENTRY("carolreed14", "HD",
		WME_ENTRY1s("data.dcp", "38c9088dca07c9623562e8efdea4c856", 1256960150), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 14 - The Fall Of April (SD) (MDNA Games)
	WME_WINENTRY("carolreed14", "SD",
		WME_ENTRY1s("data.dcp", "718195ae82d0989f91da93518a699c1c", 752801480), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 14 - The Fall Of April (HD) (MDNA Games)
	WME_WINENTRY("carolreed14", "HD",
		WME_ENTRY1s("data.dcp", "4d949fbca9b7de9f3b5eb94ae618eff6", 1256960467), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 14 - The Fall Of April (SD) (MDNA Games)
	WME_WINENTRY("carolreed14", "SD",
		WME_ENTRY1s("data.dcp", "6fca41944257b7532f4195187584507c", 752801794), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 14 - The Fall Of April (HD) (Russian)
	WME_WINENTRY("carolreed14", "HD",
		WME_ENTRY1s("data.dcp", "59784da3fe0d8b016394ad0554eecef4", 1251855682), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 14 - The Fall Of April (SD) (Russian)
	WME_WINENTRY("carolreed14", "SD",
		WME_ENTRY1s("data.dcp", "1a9a5ba095057b4e9d79c7522dd0389f", 717751105), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 15 - Geospots (HD) (MDNA Games) (v1.0)
	WME_WINENTRY("carolreed15", "HD/1.0",
		WME_ENTRY1s("data.dcp", "f3e396b249d706d34a71f5f824ab2d51", 1255754470), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 15 - Geospots (SD) (MDNA Games) (v1.0)
	WME_WINENTRY("carolreed15", "SD/1.0",
		WME_ENTRY1s("data.dcp", "cfeaf13640de63c5d2bb050357342d79", 913385369), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 15 - Geospots (HD) (MDNA Games) (v1.1)
	WME_WINENTRY("carolreed15", "HD/1.1",
		WME_ENTRY1s("data.dcp", "75645132e95924e83c9c7ff578e860a0", 1255837902), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 15 - Geospots (SD) (MDNA Games) (v1.1)
	WME_WINENTRY("carolreed15", "SD/1.1",
		WME_ENTRY1s("data.dcp", "7a82716edee6d10be847ae3d9d49b1de", 913384243), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 15 - Geospots (HD) (Russian)
	WME_WINENTRY("carolreed15", "HD",
		WME_ENTRY1s("data.dcp", "d2ab22116e6a438ea13690a00fd3e99a", 1265758319), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 15 - Geospots (SD) (Russian)
	WME_WINENTRY("carolreed15", "SD",
		WME_ENTRY1s("data.dcp", "984255ea48b26dc4c6703cd6f965a4b4", 849201287), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 16 - Quarantine Diary (HD) (MDNA Games) (v1.0)
	WME_WINENTRY("carolreed16", "HD",
		WME_ENTRY1s("data.dcp", "07158d48b5a1739d9c87dc1d7f780353", 1344041919), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 16 - Quarantine Diary (SD) (MDNA Games) (v1.0)
	WME_WINENTRY("carolreed16", "SD",
		WME_ENTRY1s("data.dcp", "9d98bf2a9a4b11506392f4cd79f94c50", 850376712), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 16 - Quarantine Diary (HD) (Russian)
	WME_WINENTRY("carolreed16", "HD",
		WME_ENTRY1s("data.dcp", "2d18acd49d624f60c64dc3400bff5566", 1350220173), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_3),

	// Carol Reed 16 - Quarantine Diary (SD) (Russian)
	WME_WINENTRY("carolreed16", "SD",
		WME_ENTRY1s("data.dcp", "9a9749e4d9cf6a151e9c95dece48b2c1", 774823176), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_3),

	// Chaos (Tech Demo)
	WME_WINENTRY("chaos", "Tech Demo",
		WME_ENTRY1s("data_main.dcp", "7fc1d1629876ebf85e098df05d8c795f", 107375), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_8_10),

	// Chivalry is Not Dead
	WME_WINENTRY("chivalry", "",
		WME_ENTRY1s("data.dcp", "ebd0915d9a12df5224be22f53bb23eb6", 7278306), Common::EN_ANY, ADGF_NO_FLAGS, WME_1_8_0),

	// Chivalry is Not Dead (Version from deirdrakai.com)
	WME_WINENTRY("chivalry", "",
		WME_ENTRY1s("data.dcp", "ae6d91b9517f4d2851a8ad94c96951c8", 7278302), Common::EN_ANY, ADGF_NO_FLAGS, WME_1_8_0),

	// Colapso: Episode 1 (multi-language)
	WME_WINENTRY("colapso1", "",
		WME_ENTRY1s("data.dcp", "9ad2d643e882ec98a4c37f52555d69f1", 38865249), Common::UNK_LANG, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_3),

	// Colors on Canvas (Demo)
	WME_WINENTRY("colorsoncanvas", "Demo",
		WME_ENTRY1s("data.dcp", "95d98104c0a9470a29da4d833ea19b98", 20824332), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_1),

	// Conspiracao Dumont (Demo)
	WME_WINENTRY("conspiracao", "Demo",
		WME_ENTRY1s("ConspiracaoDumont.exe", "106f3f2c8f18bb5ffffeed634ace256c", 32908032), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_7_0),

	// Corrosion: Cold Winter Waiting
	WME_WINENTRY("corrosion", "",
		WME_ENTRY1s("data.dcp", "ae885b1a8faa0b27f43c0e8f0df02fc9", 525931618), Common::EN_ANY, ADGF_NO_FLAGS, WME_1_9_1),

	// Corrosion: Cold Winter Waiting (Enhanced Edition)
	WME_WINENTRY("corrosion", "Enhanced Edition",
		WME_ENTRY1s("data.dcp", "eeeb63910f6fc6ad1fe9c06db0b0ea2c", 643363346), Common::EN_ANY, ADGF_NO_FLAGS, WME_1_9_1),

	// Corrosion: Cold Winter Waiting (Enhanced Edition) (Steam, v1.2.0)
	WME_WINENTRY("corrosion", "Enhanced Edition v1.2.0/Steam",
		WME_ENTRY2s("data.dcp", "d14d3c858026b1422a2a420e9a49fd16", 643372377,
					"steam_001.dcp", "e5ee8fcadec5dcc24484300c77017bd0", 41986), Common::EN_ANY, ADGF_NO_FLAGS, WME_1_9_1),

	// Corrosion: Cold Winter Waiting (Enhanced Edition) (Steam, v1.2.1)
	WME_WINENTRY("corrosion", "Enhanced Edition v1.2.1/Steam",
		WME_ENTRY2s("data.dcp", "1d3e4d97f135d61c6532b3bc08272156", 643363379,
					"steam_001.dcp", "c69ca653d0b630302afece19c5e9b52a", 42039), Common::EN_ANY, ADGF_NO_FLAGS, WME_1_9_1),

	// Corrosion: Cold Winter Waiting (Enhanced Edition) (Steam, v1.2.2)
	WME_WINENTRY("corrosion", "Enhanced Edition v1.2.2/Steam",
		WME_ENTRY2s("data.dcp", "1d3e4d97f135d61c6532b3bc08272156", 643363379,
					"steam_001.dcp", "e10b58c16e9487cfaf395c3b88ca1fd1", 42049), Common::EN_ANY, ADGF_NO_FLAGS, WME_1_9_1),

	// Corrosion: Cold Winter Waiting (Enhanced Edition) (Steam, v1.2.3)
	WME_WINENTRY("corrosion", "Enhanced Edition v1.2.3/Steam",
		WME_ENTRY2s("data.dcp", "1feab5a4bbf6bbc2ddee61f885abc096", 643363368,
					"steam_001.dcp", "bad99743e3d64368004fb2c172b96346", 42049), Common::EN_ANY, ADGF_NO_FLAGS, WME_1_9_1),

	// Corrosion: Cold Winter Waiting (Enhanced Edition) (IndieGala, v1.2.3)
	WME_WINENTRY("corrosion", "Enhanced Edition v1.2.3",
		WME_ENTRY1s("data.dcp", "07de81442b011d4b820fb1ec5e735ec8", 642860751), Common::EN_ANY, ADGF_NO_FLAGS, WME_1_9_1),

	// Dark Fall: Lost Souls
	WME_WINENTRY("darkfallls", "",
		WME_ENTRY2s("actors.dcp", "170c7ade10edd7ff6d24b16f2eecd818", 453328582,
					"stageplay.dcp", "ed67b4d6e35c19597241eb1d863aa315", 226453373), Common::EN_ANY, ADGF_UNSTABLE | GF_3D, WME_1_8_10),

	// Twilight: Dark Summer (Ario) (Persian)
	WME_WINENTRY("darksummer", "",
		WME_ENTRY1s("data.dcp", "da1142983240a5bb62c32f79a863eac5", 1030873185), Common::FA_IRN, ADGF_UNSTABLE | GF_3D, WME_1_9_3),

	// Dead City (Czech) (25.06.2004)
	WME_WINENTRY("deadcity", "v1.0.1",
		WME_ENTRY1s("data.dcp", "6860a4aa55576cbee1a0f2a04f2c8810", 9132626), Common::CS_CZE, ADGF_UNSTABLE, WME_1_2_43),

	// Dead City (Czech)
	WME_WINENTRY("deadcity", "",
		WME_ENTRY1s("data.dcp", "7ebfd50d1a22370ed7b079bcaa631d62", 9070205), Common::CS_CZE, ADGF_UNSTABLE, WME_1_4_1),

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

	// Dead City (Polish) (18.11.2012)
	WME_WINENTRY("deadcity", "",
		WME_ENTRY2s("string.tab", "9c997f0d58fd7aa6ba9baf3fbfbaba90", 42087,
					"data.dcp", "7ebfd50d1a22370ed7b079bcaa631d62", 9070205), Common::PL_POL, ADGF_UNSTABLE, WME_1_4_1),

	// Dead City (Polish) (08.06.2013)
	WME_WINENTRY("deadcity", "",
		WME_ENTRY2s("string.tab", "9c997f0d58fd7aa6ba9baf3fbfbaba90", 42085,
					"data.dcp", "7ebfd50d1a22370ed7b079bcaa631d62", 9070205), Common::PL_POL, ADGF_UNSTABLE, WME_1_4_1),

	// Devil In The Capital (Hayoola) (Persian)
	WME_WINENTRY("devilincapital", "",
		WME_ENTRY1s("data.dcp", "c54f7bc0b0d3d79096f9b999c23c9865", 1317026760), Common::FA_IRN, ADGF_UNSTABLE | GF_3D, WME_1_9_3),

	// Devil In The Capital (Steam, July 2017) (English)
	WME_WINENTRY("devilincapital", "",
		WME_ENTRY1s("data.dcp", "c3a3cd00c0493cddb84ce243272f60be", 1317709066), Common::EN_ANY, ADGF_UNSTABLE | GF_3D, WME_1_9_3),

	// DFAF Adventure
	WME_WINENTRY("dfafadventure", "",
		WME_ENTRY1s("data.dcp", "5704ebef961176f647742aa66bd09352", 10083417), Common::EN_ANY, ADGF_UNSTABLE | GF_LOWSPEC_ASSETS, WME_1_9_1),

	// Dirty Split (Czech)
	WME_WINENTRY("dirtysplit", "",
		WME_ENTRY2s("czech.dcp", "08a71446467cf8f9444cfea446b46ad6", 127697934,
					"data.dcp", "8b4b81b718bf65f30a67fc0b1e329eb5", 88577623), Common::CS_CZE, ADGF_UNSTABLE, WME_1_8_5),

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

	// Dirty Split (PC Action Coverdisc) (German)
	WME_WINENTRY("dirtysplit", "PC Action",
		WME_ENTRY1s("data.dcp", "6279b5cc077519f472bb18a0d358bd4e", 92673062), Common::DE_DEU, ADGF_UNSTABLE, WME_1_8_5),

	// Dirty Split (Italian)
	WME_WINENTRY("dirtysplit", "",
		WME_ENTRY2s("italian.dcp", "8108807fbd8af70be1ec452d0fd1131b", 125513726,
					"data.dcp", "35a150e22af274185883fdbb142c6fb1", 88577623), Common::IT_ITA, ADGF_UNSTABLE, WME_1_8_5),

	// Dirty Split (Spanish)
	WME_WINENTRY("dirtysplit", "",
		WME_ENTRY2s("spanish.dcp", "b3982c0a5e85b42e1e38240fef004aa4", 164428596,
					"data.dcp", "63766d6c68b9f00b632ea1736fc8a95c", 88577621), Common::ES_ESP, ADGF_UNSTABLE, WME_1_8_5),

	// Des Reves Elastiques Avec Mille Insectes Nommes Georges / Elastic Dreams with a Thousand Insects Named George
	WME_WINENTRY("dreaming", "",
		WME_ENTRY1s("data.dcp", "4af26d97ea063fc1277ce30ae431de90", 8804073), Common::EN_ANY, ADGF_UNSTABLE, WME_1_8_6),

	// The Driller Incident (English)
	WME_WINENTRY("driller", "",
		WME_ENTRY1s("data.dcp", "9cead7a85244263e0a5ff8f69dd7a1fc", 13671792), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// The Driller Incident (Czech)
	WME_WINENTRY("driller", "",
		WME_ENTRY1s("data.dcp", "1adcb6aac675539292a2e8fbe6cf7c76", 13671805), Common::CS_CZE, ADGF_UNSTABLE, WME_1_9_1),

	// The Driller Incident (Russian)
	WME_WINENTRY("driller", "",
		WME_ENTRY1s("data.dcp", "5bec2442339dd1ecf221873fff704617", 13671830), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_1),

	// Dr. Bohus
	WME_WINENTRY("drbohus", "",
		WME_ENTRY1s("data.dcp", "a79e640ea15f7ca36addc08ab7b1db49", 59344013), Common::CS_CZE, ADGF_UNSTABLE, WME_1_2_43),

	// Dr. Doyle - Mystery Of The Cloche Hat
	WME_WINENTRY("drdoylemotch", "Steam",
		WME_ENTRY1s("data.dcp", "9276850f89f71c5e9ab173ce6788daa8", 45779909), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Dreamcat
	WME_WINENTRY("dreamcat", "",
		WME_ENTRY1s("data.dcp", "189bd4eef29034f4ff4ed30120eaac4e", 7758040), Common::EN_ANY, ADGF_UNSTABLE | GF_LOWSPEC_ASSETS, WME_1_9_1),

	// Dreams (Demo)
	WME_WINENTRY("dreams", "Demo",
		WME_ENTRY1s("data.dcp", "1c620f51bef1faffbeebb6253542f834", 23622097), Common::EN_ANY, ADGF_UNSUPPORTED | ADGF_DEMO, WME_1_0_26),

	// Dreamscape (Beta)
	// NOTE: Package is called "Dreamland.rar", however game title is "Dreamscape"
	WME_WINENTRY("dreamscape", "Beta",
		WME_ENTRY1s("data.dcp", "7a5752ed4446c862be9f02d7932acf54", 17034377), Common::EN_ANY, ADGF_UNSTABLE, WME_1_8_11),

	// The Death of Erin Myers (Steam)
	WME_WINENTRY("erinmyers", "Steam",
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

	// Face Noir (Bundle version)
	WME_WINENTRY("facenoir", "",
		WME_ENTRY1s("data.dcp", "50a7a01b97cd3658fc84f2bec7c1212b", 555533359), Common::EN_ANY, ADGF_UNSTABLE | GF_3D, WME_1_9_1),

	// Face Noir
	WME_WINENTRY("facenoir", "",
		WME_ENTRY1s("data.dcp", "dcf71678be9432dcbfb1d9bda640a1a9", 555533343), Common::EN_ANY, ADGF_UNSTABLE | GF_3D, WME_1_9_1),

	// Face Noir (German)
	WME_WINENTRY("facenoir", "",
		WME_ENTRY1s("data.dcp", "a4febf6707f60d37835870cee4e21b14", 551512773), Common::DE_DEU, ADGF_UNSTABLE | GF_3D, WME_1_9_1),

	// Face Noir (Italian)
	WME_WINENTRY("facenoir", "",
		WME_ENTRY1s("data.dcp", "e6e9bd5072f9be8a0d8fda94f73f7fba", 559646789), Common::IT_ITA, ADGF_UNSTABLE | GF_3D, WME_1_9_1),

	// Face Noir (Polish)
	WME_WINENTRY("facenoir", "",
		WME_ENTRY1s("data.dcp", "97259a073e36b1f2d06d3045e8cdeb77", 551511755), Common::PL_POL, ADGF_UNSTABLE | GF_3D, WME_1_9_1),

	// Face Noir (Steam Jul 2014)
	WME_WINENTRY("facenoir", "",
		WME_ENTRY1s("data.dcp", "e162db79d9091faa1d670fc5cdcb4ba9", 555549627), Common::EN_ANY, ADGF_UNSTABLE | GF_3D, WME_1_9_1),

	// Face Noir (English Demo)
	WME_WINENTRY("facenoir", "Demo",
		WME_ENTRY2s("data.dcp", "545a87636e2f8762d2765d99fddc3806", 289889447,
					"data2.dcp", "bb3c7bd14526006b7abd9db4982c8cfd", 197898807), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO | GF_3D, WME_1_9_1),

	// Face Noir (German Demo)
	WME_WINENTRY("facenoir", "Demo",
		WME_ENTRY2s("data.dcp", "dbf0f4545cc31f8bea82387229abe266", 259084466,
					"data2.dcp", "b8bb3b1b5ae3bce3c132cb34c11056fe", 55447397), Common::DE_DEU, ADGF_UNSTABLE | ADGF_DEMO | GF_3D, WME_1_9_1),

	// Face Noir (Polish Demo)
	WME_WINENTRY("facenoir", "Demo",
		WME_ENTRY2s("data.dcp", "8add477c70ffa712a81db4af5fcaa0e1", 293911404,
					"data2.dcp", "1874aa7d51121cc43e78f71dbe9c41b7", 197953641), Common::PL_POL, ADGF_UNSTABLE | ADGF_DEMO | GF_3D, WME_1_9_1),

	// Finding Hope (English)
	WME_WINENTRY("findinghope", "",
		WME_ENTRY1s("data.dcp", "d7c3857b622e038781bf42abd3c64962", 110247769), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// Finding Hope (Russian)
	// NOTE: this game distribution contained unpacked game files instead of usual game.dcp package
	WME_WINENTRY("findinghope", "",
		WME_ENTRY1s("default.game", "57db79e9de62b9d1c7a335822f6242ff", 742), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_1),

	// Forgotten Sound 1 - Revelation (Retail/Hayoola) (Persian)
	WME_WINENTRY("forgottensound1", "",
		WME_ENTRY1s("data.dcp", "1c5871ad711430663b1848f0f2362b44", 2030321701), Common::FA_IRN, ADGF_UNSTABLE | GF_3D, WME_1_9_1),

	// Forgotten Sound 1 - Revelation (Steam, January 2018) (English)
	WME_WINENTRY("forgottensound1", "",
		WME_ENTRY1s("data.dcp", "01b8b2b4c55c2d83071f5730269cb313", 937507449), Common::EN_ANY, ADGF_UNSTABLE | GF_3D, WME_1_9_3),

	// Forgotten Sound 2 - Destiny (Hayoola) (Persian)
	WME_WINENTRY("forgottensound2", "",
		WME_ENTRY1s("data.dcp", "9e1b4f3dd41e8dee79698ea17bc9ab7b", 1630760876), Common::FA_IRN, ADGF_UNSTABLE | GF_3D, WME_1_9_3),

	// Forgotten Sound 2 - Destiny (Steam, January 2018) (English)
	WME_WINENTRY("forgottensound2", "",
		WME_ENTRY1s("data.dcp", "d743f4ffee2a7cc939e314f29a1a0cd6", 1244439717), Common::EN_ANY, ADGF_UNSTABLE | GF_3D, WME_1_9_3),

	// Four
	WME_WINENTRY("four", "",
		WME_ENTRY1s("data.dcp", "ec05cd5e37c9a524053b8859635a4234", 62599855), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// FoxTail 1.2.230.1291 (English)
	WME_WINENTRY("foxtail", "1.2.230.1291",
		WME_ENTRY1s("data.dcp", "651ae5b062073021edaca7e1de131eec", 59357572), Common::EN_ANY, ADGF_UNSTABLE, FOXTAIL_1_2_230),

	// FoxTail 1.2.230.1291 (German)
	WME_WINENTRY("foxtail", "1.2.230.1291",
		WME_ENTRY1s("data.dcp", "651ae5b062073021edaca7e1de131eec", 59357572), Common::DE_DEU, ADGF_UNSTABLE, FOXTAIL_1_2_230),

	// FoxTail 1.2.230.1291 (Russian)
	WME_WINENTRY("foxtail", "1.2.230.1291",
		WME_ENTRY1s("data.dcp", "651ae5b062073021edaca7e1de131eec", 59357572), Common::RU_RUS, ADGF_UNSTABLE, FOXTAIL_1_2_230),

	// FoxTail 1.2.230.1291 (Ukranian)
	WME_WINENTRY("foxtail", "1.2.230.1291",
		WME_ENTRY1s("data.dcp", "651ae5b062073021edaca7e1de131eec", 59357572), Common::UA_UKR, ADGF_UNSTABLE, FOXTAIL_1_2_230),

	// FoxTail 1.2.230.1303 (English)
	WME_WINENTRY("foxtail", "1.2.230.1303",
		WME_ENTRY1s("data.dcp", "03ed77b1ac8b94bbd0247324a41621ad", 59357623), Common::EN_ANY, ADGF_UNSTABLE, FOXTAIL_1_2_230),

	// FoxTail 1.2.230.1303 (German)
	WME_WINENTRY("foxtail", "1.2.230.1303",
		WME_ENTRY1s("data.dcp", "03ed77b1ac8b94bbd0247324a41621ad", 59357623), Common::DE_DEU, ADGF_UNSTABLE, FOXTAIL_1_2_230),

	// FoxTail 1.2.230.1303 (Russian)
	WME_WINENTRY("foxtail", "1.2.230.1303",
		WME_ENTRY1s("data.dcp", "03ed77b1ac8b94bbd0247324a41621ad", 59357623), Common::RU_RUS, ADGF_UNSTABLE, FOXTAIL_1_2_230),

	// FoxTail 1.2.230.1303 (Ukranian)
	WME_WINENTRY("foxtail", "1.2.230.1303",
		WME_ENTRY1s("data.dcp", "03ed77b1ac8b94bbd0247324a41621ad", 59357623), Common::UA_UKR, ADGF_UNSTABLE, FOXTAIL_1_2_230),

	// FoxTail 1.2.230.1313 (English)
	WME_WINENTRY("foxtail", "1.2.230.1313",
		WME_ENTRY1s("data.dcp", "d7287c49210c7c9f9376327c6e224c7b", 59383312), Common::EN_ANY, ADGF_UNSTABLE, FOXTAIL_1_2_230),

	// FoxTail 1.2.230.1313 (German)
	WME_WINENTRY("foxtail", "1.2.230.1313",
		WME_ENTRY1s("data.dcp", "d7287c49210c7c9f9376327c6e224c7b", 59383312), Common::DE_DEU, ADGF_UNSTABLE, FOXTAIL_1_2_230),

	// FoxTail 1.2.230.1313 (Russian)
	WME_WINENTRY("foxtail", "1.2.230.1313",
		WME_ENTRY1s("data.dcp", "d7287c49210c7c9f9376327c6e224c7b", 59383312), Common::RU_RUS, ADGF_UNSTABLE, FOXTAIL_1_2_230),

	// FoxTail 1.2.230.1313 (Ukranian)
	WME_WINENTRY("foxtail", "1.2.230.1313",
		WME_ENTRY1s("data.dcp", "d7287c49210c7c9f9376327c6e224c7b", 59383312), Common::UA_UKR, ADGF_UNSTABLE, FOXTAIL_1_2_230),

	// FoxTail 1.2.230.1315 (English)
	WME_WINENTRY("foxtail", "1.2.230.1315",
		WME_ENTRY1s("data.dcp", "434c4f598582a569972acd4d700a44e5", 59383416), Common::EN_ANY, ADGF_UNSTABLE, FOXTAIL_1_2_230),

	// FoxTail 1.2.230.1315 (German)
	WME_WINENTRY("foxtail", "1.2.230.1315",
		WME_ENTRY1s("data.dcp", "434c4f598582a569972acd4d700a44e5", 59383416), Common::DE_DEU, ADGF_UNSTABLE, FOXTAIL_1_2_230),

	// FoxTail 1.2.230.1315 (Russian)
	WME_WINENTRY("foxtail", "1.2.230.1315",
		WME_ENTRY1s("data.dcp", "434c4f598582a569972acd4d700a44e5", 59383416), Common::RU_RUS, ADGF_UNSTABLE, FOXTAIL_1_2_230),

	// FoxTail 1.2.230.1315 (Ukranian)
	WME_WINENTRY("foxtail", "1.2.230.1315",
		WME_ENTRY1s("data.dcp", "434c4f598582a569972acd4d700a44e5", 59383416), Common::UA_UKR, ADGF_UNSTABLE, FOXTAIL_1_2_230),

	// FoxTail 1.2.230.1316 (English)
	WME_WINENTRY("foxtail", "1.2.230.1316",
		WME_ENTRY1s("data.dcp", "5aa16c180998f1816a734c58a01ab8b1", 59383306), Common::EN_ANY, ADGF_UNSTABLE, FOXTAIL_1_2_230),

	// FoxTail 1.2.230.1316 (German)
	WME_WINENTRY("foxtail", "1.2.230.1316",
		WME_ENTRY1s("data.dcp", "5aa16c180998f1816a734c58a01ab8b1", 59383306), Common::DE_DEU, ADGF_UNSTABLE, FOXTAIL_1_2_230),

	// FoxTail 1.2.230.1316 (Russian)
	WME_WINENTRY("foxtail", "1.2.230.1316",
		WME_ENTRY1s("data.dcp", "5aa16c180998f1816a734c58a01ab8b1", 59383306), Common::RU_RUS, ADGF_UNSTABLE, FOXTAIL_1_2_230),

	// FoxTail 1.2.230.1316 (Ukranian)
	WME_WINENTRY("foxtail", "1.2.230.1316",
		WME_ENTRY1s("data.dcp", "5aa16c180998f1816a734c58a01ab8b1", 59383306), Common::UA_UKR, ADGF_UNSTABLE, FOXTAIL_1_2_230),

	// FoxTail 1.2.230.1318 (English)
	WME_WINENTRY("foxtail", "1.2.230.1318",
		WME_ENTRY1s("data.dcp", "363856606d19fb7e0e3a0a67737697fa", 59382887), Common::EN_ANY, ADGF_UNSTABLE, FOXTAIL_1_2_230),

	// FoxTail 1.2.230.1318 (German)
	WME_WINENTRY("foxtail", "1.2.230.1318",
		WME_ENTRY1s("data.dcp", "363856606d19fb7e0e3a0a67737697fa", 59382887), Common::DE_DEU, ADGF_UNSTABLE, FOXTAIL_1_2_230),

	// FoxTail 1.2.230.1318 (Russian)
	WME_WINENTRY("foxtail", "1.2.230.1318",
		WME_ENTRY1s("data.dcp", "363856606d19fb7e0e3a0a67737697fa", 59382887), Common::RU_RUS, ADGF_UNSTABLE, FOXTAIL_1_2_230),

	// FoxTail 1.2.230.1318 (Ukranian)
	WME_WINENTRY("foxtail", "1.2.230.1318",
		WME_ENTRY1s("data.dcp", "363856606d19fb7e0e3a0a67737697fa", 59382887), Common::UA_UKR, ADGF_UNSTABLE, FOXTAIL_1_2_230),

	// FoxTail 1.2.230.1321 (English)
	WME_WINENTRY("foxtail", "1.2.230.1321",
		WME_ENTRY1s("data.dcp", "bbab16777c4bc979c5f773e12b804a63", 59151985), Common::EN_ANY, ADGF_UNSTABLE, FOXTAIL_1_2_230),

	// FoxTail 1.2.230.1321 (German)
	WME_WINENTRY("foxtail", "1.2.230.1321",
		WME_ENTRY1s("data.dcp", "bbab16777c4bc979c5f773e12b804a63", 59151985), Common::DE_DEU, ADGF_UNSTABLE, FOXTAIL_1_2_230),

	// FoxTail 1.2.230.1321 (Russian)
	WME_WINENTRY("foxtail", "1.2.230.1321",
		WME_ENTRY1s("data.dcp", "bbab16777c4bc979c5f773e12b804a63", 59151985), Common::RU_RUS, ADGF_UNSTABLE, FOXTAIL_1_2_230),

	// FoxTail 1.2.230.1321 (Ukranian)
	WME_WINENTRY("foxtail", "1.2.230.1321",
		WME_ENTRY1s("data.dcp", "bbab16777c4bc979c5f773e12b804a63", 59151985), Common::UA_UKR, ADGF_UNSTABLE, FOXTAIL_1_2_230),

	// FoxTail 1.2.230.1322 (English)
	// not a mistake, data.dcp for 1.2.230.1321 and 1.2.230.1322 have same byte size
	WME_WINENTRY("foxtail", "1.2.230.1322",
		WME_ENTRY1s("data.dcp", "22e5f634742956b6f4087459a9c8acf4", 59151985), Common::EN_ANY, ADGF_UNSTABLE, FOXTAIL_1_2_230),

	// FoxTail 1.2.230.1322 (German)
	WME_WINENTRY("foxtail", "1.2.230.1322",
		WME_ENTRY1s("data.dcp", "22e5f634742956b6f4087459a9c8acf4", 59151985), Common::DE_DEU, ADGF_UNSTABLE, FOXTAIL_1_2_230),

	// FoxTail 1.2.230.1322 (Russian)
	WME_WINENTRY("foxtail", "1.2.230.1322",
		WME_ENTRY1s("data.dcp", "22e5f634742956b6f4087459a9c8acf4", 59151985), Common::RU_RUS, ADGF_UNSTABLE, FOXTAIL_1_2_230),

	// FoxTail 1.2.230.1322 (Ukranian)
	WME_WINENTRY("foxtail", "1.2.230.1322",
		WME_ENTRY1s("data.dcp", "22e5f634742956b6f4087459a9c8acf4", 59151985), Common::UA_UKR, ADGF_UNSTABLE, FOXTAIL_1_2_230),

	// FoxTail 1.2.304.1571 (English)
	WME_WINENTRY("foxtail", "1.2.304.1571",
		WME_ENTRY1s("data.dcp", "32fd78f0b1509863f2e91bc7afc633ff", 59630008), Common::EN_ANY, ADGF_UNSTABLE, FOXTAIL_1_2_304),

	// FoxTail 1.2.304.1571 (German)
	WME_WINENTRY("foxtail", "1.2.304.1571",
		WME_ENTRY1s("data.dcp", "32fd78f0b1509863f2e91bc7afc633ff", 59630008), Common::DE_DEU, ADGF_UNSTABLE, FOXTAIL_1_2_304),

	// FoxTail 1.2.304.1571 (Russian)
	WME_WINENTRY("foxtail", "1.2.304.1571",
		WME_ENTRY1s("data.dcp", "32fd78f0b1509863f2e91bc7afc633ff", 59630008), Common::RU_RUS, ADGF_UNSTABLE, FOXTAIL_1_2_304),

	// FoxTail 1.2.304.1571 (Ukranian)
	WME_WINENTRY("foxtail", "1.2.304.1571",
		WME_ENTRY1s("data.dcp", "32fd78f0b1509863f2e91bc7afc633ff", 59630008), Common::UA_UKR, ADGF_UNSTABLE, FOXTAIL_1_2_304),

	// FoxTail 1.2.362.2039 (English)
	WME_WINENTRY("foxtail", "1.2.362.2039",
		WME_ENTRY1s("data.dcp", "ca1b0379c8f0dffd3bf8b95e91379b2c", 70132635), Common::EN_ANY, ADGF_UNSTABLE, FOXTAIL_1_2_362),

	// FoxTail 1.2.362.2039 (German)
	WME_WINENTRY("foxtail", "1.2.362.2039",
		WME_ENTRY1s("data.dcp", "ca1b0379c8f0dffd3bf8b95e91379b2c", 70132635), Common::DE_DEU, ADGF_UNSTABLE, FOXTAIL_1_2_362),

	// FoxTail 1.2.362.2039 (Russian)
	WME_WINENTRY("foxtail", "1.2.362.2039",
		WME_ENTRY1s("data.dcp", "ca1b0379c8f0dffd3bf8b95e91379b2c", 70132635), Common::RU_RUS, ADGF_UNSTABLE, FOXTAIL_1_2_362),

	// FoxTail 1.2.362.2039 (Ukranian)
	WME_WINENTRY("foxtail", "1.2.362.2039",
		WME_ENTRY1s("data.dcp", "ca1b0379c8f0dffd3bf8b95e91379b2c", 70132635), Common::UA_UKR, ADGF_UNSTABLE, FOXTAIL_1_2_362),

	// FoxTail 1.2.362.2047 (English)
	WME_WINENTRY("foxtail", "1.2.362.2047",
		WME_ENTRY1s("data.dcp", "2c4c744ff103f4fc6e770515e2da8b16", 70124937), Common::EN_ANY, ADGF_UNSTABLE, FOXTAIL_1_2_362),

	// FoxTail 1.2.362.2047 (German)
	WME_WINENTRY("foxtail", "1.2.362.2047",
		WME_ENTRY1s("data.dcp", "2c4c744ff103f4fc6e770515e2da8b16", 70124937), Common::DE_DEU, ADGF_UNSTABLE, FOXTAIL_1_2_362),

	// FoxTail 1.2.362.2047 (Russian)
	WME_WINENTRY("foxtail", "1.2.362.2047",
		WME_ENTRY1s("data.dcp", "2c4c744ff103f4fc6e770515e2da8b16", 70124937), Common::RU_RUS, ADGF_UNSTABLE, FOXTAIL_1_2_362),

	// FoxTail 1.2.362.2047 (Ukranian)
	WME_WINENTRY("foxtail", "1.2.362.2047",
		WME_ENTRY1s("data.dcp", "2c4c744ff103f4fc6e770515e2da8b16", 70124937), Common::UA_UKR, ADGF_UNSTABLE, FOXTAIL_1_2_362),

	// FoxTail 1.2.527.3377 (English)
	WME_WINENTRY("foxtail", "1.2.527.3377",
		WME_ENTRY1s("data.dcp", "e0177c5752d067a3e473b86ad40d57c3", 109502449), Common::EN_ANY, ADGF_UNSTABLE, FOXTAIL_1_2_527),

	// FoxTail 1.2.527.3377 (German)
	WME_WINENTRY("foxtail", "1.2.527.3377",
		WME_ENTRY1s("data.dcp", "e0177c5752d067a3e473b86ad40d57c3", 109502449), Common::DE_DEU, ADGF_UNSTABLE, FOXTAIL_1_2_527),

	// FoxTail 1.2.527.3377 (Polish)
	WME_WINENTRY("foxtail", "1.2.527.3377",
		WME_ENTRY1s("data.dcp", "e0177c5752d067a3e473b86ad40d57c3", 109502449), Common::PL_POL, ADGF_UNSTABLE, FOXTAIL_1_2_527),

	// FoxTail 1.2.527.3377 (Russian)
	WME_WINENTRY("foxtail", "1.2.527.3377",
		WME_ENTRY1s("data.dcp", "e0177c5752d067a3e473b86ad40d57c3", 109502449), Common::RU_RUS, ADGF_UNSTABLE, FOXTAIL_1_2_527),

	// FoxTail 1.2.527.3377 (Ukranian)
	WME_WINENTRY("foxtail", "1.2.527.3377",
		WME_ENTRY1s("data.dcp", "e0177c5752d067a3e473b86ad40d57c3", 109502449), Common::UA_UKR, ADGF_UNSTABLE, FOXTAIL_1_2_527),

	// FoxTail 1.2.527.3389 (English)
	WME_WINENTRY("foxtail", "1.2.527.3389",
		WME_ENTRY1s("data.dcp", "a940ffa1b4347588d13e4a9756bb0bbd", 109503345), Common::EN_ANY, ADGF_UNSTABLE, FOXTAIL_1_2_527),

	// FoxTail 1.2.527.3389 (German)
	WME_WINENTRY("foxtail", "1.2.527.3389",
		WME_ENTRY1s("data.dcp", "a940ffa1b4347588d13e4a9756bb0bbd", 109503345), Common::DE_DEU, ADGF_UNSTABLE, FOXTAIL_1_2_527),

	// FoxTail 1.2.527.3389 (Polish)
	WME_WINENTRY("foxtail", "1.2.527.3389",
		WME_ENTRY1s("data.dcp", "a940ffa1b4347588d13e4a9756bb0bbd", 109503345), Common::PL_POL, ADGF_UNSTABLE, FOXTAIL_1_2_527),

	// FoxTail 1.2.527.3389 (Russian)
	WME_WINENTRY("foxtail", "1.2.527.3389",
		WME_ENTRY1s("data.dcp", "a940ffa1b4347588d13e4a9756bb0bbd", 109503345), Common::RU_RUS, ADGF_UNSTABLE, FOXTAIL_1_2_527),

	// FoxTail 1.2.527.3389 (Ukranian)
	WME_WINENTRY("foxtail", "1.2.527.3389",
		WME_ENTRY1s("data.dcp", "a940ffa1b4347588d13e4a9756bb0bbd", 109503345), Common::UA_UKR, ADGF_UNSTABLE, FOXTAIL_1_2_527),

	// FoxTail 1.2.527.3391 (English)
	WME_WINENTRY("foxtail", "1.2.527.3391",
		WME_ENTRY1s("data.dcp", "e5d06fa058cd9d6f20d6206356e5854d", 109503303), Common::EN_ANY, ADGF_UNSTABLE, FOXTAIL_1_2_527),

	// FoxTail 1.2.527.3391 (German)
	WME_WINENTRY("foxtail", "1.2.527.3391",
		WME_ENTRY1s("data.dcp", "e5d06fa058cd9d6f20d6206356e5854d", 109503303), Common::DE_DEU, ADGF_UNSTABLE, FOXTAIL_1_2_527),

	// FoxTail 1.2.527.3391 (Polish)
	WME_WINENTRY("foxtail", "1.2.527.3391",
		WME_ENTRY1s("data.dcp", "e5d06fa058cd9d6f20d6206356e5854d", 109503303), Common::PL_POL, ADGF_UNSTABLE, FOXTAIL_1_2_527),

	// FoxTail 1.2.527.3391 (Russian)
	WME_WINENTRY("foxtail", "1.2.527.3391",
		WME_ENTRY1s("data.dcp", "e5d06fa058cd9d6f20d6206356e5854d", 109503303), Common::RU_RUS, ADGF_UNSTABLE, FOXTAIL_1_2_527),

	// FoxTail 1.2.527.3391 (Ukranian)
	WME_WINENTRY("foxtail", "1.2.527.3391",
		WME_ENTRY1s("data.dcp", "e5d06fa058cd9d6f20d6206356e5854d", 109503303), Common::UA_UKR, ADGF_UNSTABLE, FOXTAIL_1_2_527),

	// FoxTail 1.2.896.4370 (English)
	WME_WINENTRY("foxtail", "1.2.896.4370",
		WME_ENTRY1s("data.dcp", "cee21687240aa160b8ebf1e0cccaef59", 154006218), Common::EN_ANY, ADGF_UNSTABLE, FOXTAIL_1_2_896),

	// FoxTail 1.2.896.4370 (German)
	WME_WINENTRY("foxtail", "1.2.896.4370",
		WME_ENTRY1s("data.dcp", "cee21687240aa160b8ebf1e0cccaef59", 154006218), Common::DE_DEU, ADGF_UNSTABLE, FOXTAIL_1_2_896),

	// FoxTail 1.2.896.4370 (Polish)
	WME_WINENTRY("foxtail", "1.2.896.4370",
		WME_ENTRY1s("data.dcp", "cee21687240aa160b8ebf1e0cccaef59", 154006218), Common::PL_POL, ADGF_UNSTABLE, FOXTAIL_1_2_896),

	// FoxTail 1.2.896.4370 (Russian)
	WME_WINENTRY("foxtail", "1.2.896.4370",
		WME_ENTRY1s("data.dcp", "cee21687240aa160b8ebf1e0cccaef59", 154006218), Common::RU_RUS, ADGF_UNSTABLE, FOXTAIL_1_2_896),

	// FoxTail 1.2.896.4370 (Ukranian)
	WME_WINENTRY("foxtail", "1.2.896.4370",
		WME_ENTRY1s("data.dcp", "cee21687240aa160b8ebf1e0cccaef59", 154006218), Common::UA_UKR, ADGF_UNSTABLE, FOXTAIL_1_2_896),

	// FoxTail 1.2.896.4371 (English)
	WME_WINENTRY("foxtail", "1.2.896.4371",
		WME_ENTRY1s("data.dcp", "ca9842a6461cc7b00e63b5bc11813971", 154006242), Common::EN_ANY, ADGF_UNSTABLE, FOXTAIL_1_2_896),

	// FoxTail 1.2.896.4371 (German)
	WME_WINENTRY("foxtail", "1.2.896.4371",
		WME_ENTRY1s("data.dcp", "ca9842a6461cc7b00e63b5bc11813971", 154006242), Common::DE_DEU, ADGF_UNSTABLE, FOXTAIL_1_2_896),

	// FoxTail 1.2.896.4371 (Polish)
	WME_WINENTRY("foxtail", "1.2.896.4371",
		WME_ENTRY1s("data.dcp", "ca9842a6461cc7b00e63b5bc11813971", 154006242), Common::PL_POL, ADGF_UNSTABLE, FOXTAIL_1_2_896),

	// FoxTail 1.2.896.4371 (Russian)
	WME_WINENTRY("foxtail", "1.2.896.4371",
		WME_ENTRY1s("data.dcp", "ca9842a6461cc7b00e63b5bc11813971", 154006242), Common::RU_RUS, ADGF_UNSTABLE, FOXTAIL_1_2_896),

	// FoxTail 1.2.896.4371 (Ukranian)
	WME_WINENTRY("foxtail", "1.2.896.4371",
		WME_ENTRY1s("data.dcp", "ca9842a6461cc7b00e63b5bc11813971", 154006242), Common::UA_UKR, ADGF_UNSTABLE, FOXTAIL_1_2_896),

	// FoxTail 1.2.902.4379 (English)
	WME_WINENTRY("foxtail", "1.2.902.4379",
		WME_ENTRY1s("data.dcp", "a4a5458afa42ac1d90f4050b033421a4", 153987445), Common::EN_ANY, ADGF_UNSTABLE, FOXTAIL_1_2_902),

	// FoxTail 1.2.902.4379 (German)
	WME_WINENTRY("foxtail", "1.2.902.4379",
		WME_ENTRY1s("data.dcp", "a4a5458afa42ac1d90f4050b033421a4", 153987445), Common::DE_DEU, ADGF_UNSTABLE, FOXTAIL_1_2_902),

	// FoxTail 1.2.902.4379 (Polish)
	WME_WINENTRY("foxtail", "1.2.902.4379",
		WME_ENTRY1s("data.dcp", "a4a5458afa42ac1d90f4050b033421a4", 153987445), Common::PL_POL, ADGF_UNSTABLE, FOXTAIL_1_2_902),

	// FoxTail 1.2.902.4379 (Russian)
	WME_WINENTRY("foxtail", "1.2.902.4379",
		WME_ENTRY1s("data.dcp", "a4a5458afa42ac1d90f4050b033421a4", 153987445), Common::RU_RUS, ADGF_UNSTABLE, FOXTAIL_1_2_902),

	// FoxTail 1.2.902.4379 (Ukranian)
	WME_WINENTRY("foxtail", "1.2.902.4379",
		WME_ENTRY1s("data.dcp", "a4a5458afa42ac1d90f4050b033421a4", 153987445), Common::UA_UKR, ADGF_UNSTABLE, FOXTAIL_1_2_902),

	// Framed (Beta)
	WME_WINENTRY("framed", "Beta",
		WME_ENTRY1s("data.dcp", "e7259fb36f2c6f9f28242291e0c3de98", 34690568), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_8_11),

	// Fred (English) (PreAlpha)
	WME_WINENTRY("fred", "PreAlpha",
		WME_ENTRY2s("string.tab", "76b48c26e611071810d8182fc27f7d0e", 12590,
		            "data.dcp", "e18b1f9815b9e9cf1d1357bc48269b0b", 7632855), Common::EN_ANY, ADGF_UNSUPPORTED | ADGF_DEMO, WME_1_0_31),

	// Fred (French) (PreAlpha)
	WME_WINENTRY("fred", "PreAlpha",
		WME_ENTRY1s("data.dcp", "e18b1f9815b9e9cf1d1357bc48269b0b", 7632855), Common::FR_FRA, ADGF_UNSUPPORTED | ADGF_DEMO, WME_1_0_31),

	// Ghost in the Sheet (English, v1.00)
	WME_WINENTRY("ghostsheet", "",
		WME_ENTRY2s("english.dcp", "f53acdc5510aa184f6f968e38d8c9ad7", 12008908,
					"data.dcp", "5e7f7d98680825bb81fca11e5493c1ad", 168658), Common::EN_ANY, ADGF_UNSTABLE, WME_1_8_0),

	// Ghost in the Sheet (BigFish) (English)
	WME_WINENTRY("ghostsheet", "",
		WME_ENTRY2s("english.dcp", "e6d0aad2c89996bcabe416105a3d6d3a", 12221017,
					"data.dcp", "b2f8b05328e4881e15e98e845b63f451", 168003), Common::EN_ANY, ADGF_UNSTABLE, WME_1_8_0),

	// Ghost in the Sheet (Italian)
	WME_WINENTRY("ghostsheet", "",
		WME_ENTRY2s("italiano.dcp", "b258151caddee5ecdaf0fdd3afe62151", 11220994,
					"data.dcp", "99ae5d9977d9a542a35c09d27df0ab22", 167980), Common::IT_ITA, ADGF_UNSTABLE, WME_1_8_0),

	// Ghost in the Sheet (Russian)
	WME_WINENTRY("ghostsheet", "",
		WME_ENTRY2s("russian.dcp", "e32a235e77886e4b95ca362252584373", 19283033,
					"data.dcp", "45471184e40fb0e95b2c05c5e2ec8502", 167945), Common::RU_RUS, ADGF_UNSTABLE, WME_1_8_0),

	// Ghost in the Sheet (Demo)
	WME_WINENTRY("ghostsheet", "Demo",
		WME_ENTRY1s("data.dcp", "dc1f6595f412ac25a52eaf47dad4ab81", 169083), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_8_0),

	// Ghost in the Sheet (Italian Demo)
	WME_WINENTRY("ghostsheet", "Demo",
		WME_ENTRY1s("data.dcp", "4995d69dd43e8aa00bcb026c37f00009", 171757), Common::IT_ITA, ADGF_UNSTABLE | ADGF_DEMO, WME_1_8_0),

	// The Golden Calf (Russian)
	WME_WINENTRY("goldencalf", "",
		WME_ENTRY1s("data.dcp", "c70536681d839f9b4357938891402af3", 488532641), Common::RU_RUS, ADGF_UNSTABLE, WME_1_8_11),

	// The Golden Calf (German)
	WME_WINENTRY("goldencalf", "",
		WME_ENTRY1s("data.dcp", "c611503c2c10137d1195878080cee2d1", 1174552), Common::DE_DEU, ADGF_UNSTABLE, WME_1_8_11),

	// The Guttenburg Project (Demo)
	WME_WINENTRY("guttenburg", "Demo",
		WME_ENTRY1s("data.dcp", "0c90b369ad0928a450d092114376cf7f", 24121170), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_1),

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
		WME_ENTRY1s("data.dcp", "f55674e17df2816145d3473401081f05", 88245004), Common::PT_PRT, ADGF_UNSTABLE, WME_1_9_1),

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
					"data.dcp", "25cb955a60b58326f2eeda1ce288fb37", 183251259), Common::CS_CZE, ADGF_UNSTABLE, WME_1_9_3),

	// Helga Deep In Trouble (English)
	WME_WINENTRY("helga", "",
		WME_ENTRY2s("english.dcp", "bfa136b21bdbc7d8691c0770a6d40bc3", 135931,
					"data.dcp", "25cb955a60b58326f2eeda1ce288fb37", 183251259), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Helga Deep In Trouble (Demo) (Czech)
	WME_WINENTRY("helga", "Demo",
		WME_ENTRY2s("data.dcp", "45134ed93bc391edf148b79cdcbf2a09", 154266028,
					"data.dcp", "45134ed93bc391edf148b79cdcbf2a09", 154266028), Common::CS_CZE, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_3),

	// Helga Deep In Trouble (Demo) (English)
	WME_WINENTRY("helga", "Demo",
		WME_ENTRY2s("english.dcp", "b3a93e678f0ef97200f691cd1724643f", 135864,
					"data.dcp", "45134ed93bc391edf148b79cdcbf2a09", 154266028), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_3),

	// Hor v1.0
	WME_WINENTRY("hor", "1.0",
		WME_ENTRY1s("data.dcp", "ae94007f25a21143c028c1b7807dd907", 15077486), Common::UNK_LANG, ADGF_UNSTABLE, WME_1_9_3),

	// Hor v1.3
	WME_WINENTRY("hor", "1.3",
		WME_ENTRY1s("data.dcp", "37b0abeb8651b82b9e6327bd10a18185", 15077486), Common::UNK_LANG, ADGF_UNSTABLE, WME_1_9_3),

	// The Idiot's Tale (Steam, April 2018)
	WME_WINENTRY("idiotstale", "1.0.2",
		WME_ENTRY1s("data.dcp", "3ecf43cb535482b82fd583097921edb9", 417507705), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// I Must Kill...: Fresh Meat (v0.02)
	WME_WINENTRY("imustkill", "v0.02",
		WME_ENTRY1s("data.dcp", "482660d7c196c949b31f4d9599e20777", 56444999), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_3),

	// I Must Kill...: Fresh Meat (v0.03)
	WME_WINENTRY("imustkill", "v0.03",
		WME_ENTRY1s("data.dcp", "ac1e07e1806661e71875e1456302f5d9", 100703459), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_3),

	// I Must Kill...: Fresh Meat (Greenlight demo)
	WME_WINENTRY("imustkill", "Greenlight demo",
		WME_ENTRY1s("data.dcp", "984abb9a75490ff31be00c41270d6913", 133161500), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_3),

	// I Must Kill...: Fresh Meat (Steam Early Access, 22 June 2016)
	WME_WINENTRY("imustkill", "Early Access",
		WME_ENTRY1s("data.dcp", "9474fdfa678d8bf059d311044b781e34", 197732041), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_3),

	// James Peris: No License Nor Control (English)
	WME_WINENTRY("jamesperis", "Version 1.5",
		WME_ENTRY1s("data.dcp", "f5635080b65aaf75c3676ce0cd46460b", 225294032), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// James Peris: No License Nor Control (Spanish)
	WME_WINENTRY("jamesperis", "Version 1.5",
		WME_ENTRY1s("data.dcp", "f5635080b65aaf75c3676ce0cd46460b", 225294032), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_1),

	// James Peris: No License Nor Control (English)
	WME_WINENTRY("jamesperis", "Version 1.7",
		WME_ENTRY1s("data.dcp", "a420961e170cb7d168a0d2bae2fe5218", 225294032), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// James Peris: No License Nor Control (Spanish)
	WME_WINENTRY("jamesperis", "Version 1.7",
		WME_ENTRY1s("data.dcp", "a420961e170cb7d168a0d2bae2fe5218", 225294032), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_1),

	// James Peris: No License Nor Control (English)
	WME_WINENTRY("jamesperis", "Version 1.8",
		WME_ENTRY1s("data.dcp", "d6049dfb5dbe812bae1e96924a012500", 225299340), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// James Peris: No License Nor Control (Spanish)
	WME_WINENTRY("jamesperis", "Version 1.8",
		WME_ENTRY1s("data.dcp", "d6049dfb5dbe812bae1e96924a012500", 225299340), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_1),

	// James Peris: No License Nor Control - Definitive Edition (English) (Steam 2020-03-12)
	WME_WINENTRY("jamesperis", "Version 1.9",
		WME_ENTRY2s("data.dcp", "60568c88c9dc5653bfc94a8cbf228748", 225299678,
					"parche9.dcp", "05363aa13cb8aa5a3fafc68adb4dda5a", 207640138), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// James Peris: No License Nor Control - Definitive Edition (Spanish) (Steam 2020-03-12)
	WME_WINENTRY("jamesperis", "Version 1.9",
		WME_ENTRY2s("data.dcp", "60568c88c9dc5653bfc94a8cbf228748", 225299678,
					"parche9.dcp", "05363aa13cb8aa5a3fafc68adb4dda5a", 207640138), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_3),

	// James Peris: No License Nor Control - Definitive Edition (English) (Steam 2020-04-27)
	WME_WINENTRY("jamesperis", "Version 1.9.1e",
		WME_ENTRY3s("data.dcp", "60568c88c9dc5653bfc94a8cbf228748", 225299678,
					"parche9.dcp", "05363aa13cb8aa5a3fafc68adb4dda5a", 207640138,
					"parche91e.dcp", "ee21819363fb32a38b9f5fcf8545e374", 72496832), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// James Peris: No License Nor Control - Definitive Edition (Spanish) (Steam 2020-04-27)
	WME_WINENTRY("jamesperis", "Version 1.9.1e",
		WME_ENTRY3s("data.dcp", "60568c88c9dc5653bfc94a8cbf228748", 225299678,
					"parche9.dcp", "05363aa13cb8aa5a3fafc68adb4dda5a", 207640138,
					"parche91e.dcp", "ee21819363fb32a38b9f5fcf8545e374", 72496832), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_3),


	// James Peris: No License Nor Control (Older Demo) (English)
	WME_WINENTRY("jamesperis", "Older Demo",
		WME_ENTRY1s("data.dcp", "05d6e37f1078fac57a088f96da4a2ea1", 87340874), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_8_6),

	// James Peris: No License Nor Control (Older Demo) (Spanish)
	WME_WINENTRY("jamesperis", "Older Demo",
		WME_ENTRY1s("data.dcp", "c8c0e63bcc4dbfb76d1048bdc3a38248", 100274569), Common::ES_ESP, ADGF_UNSTABLE | ADGF_DEMO, WME_1_8_6),

	// James Peris: No License Nor Control (Demo) (English)
	WME_WINENTRY("jamesperis", "Demo",
		WME_ENTRY1s("data.dcp", "edb9f9c7a08993c1e28f4e477b5f9830", 116113507), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_1),

	// James Peris: No License Nor Control (Demo) (Spanish)
	WME_WINENTRY("jamesperis", "Demo",
		WME_ENTRY1s("data.dcp", "edb9f9c7a08993c1e28f4e477b5f9830", 116113507), Common::ES_ESP, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_1),

	// Lov Mamuta
	WME_WINENTRY("lovmamuta", "",
		WME_ENTRY1s("data.dcp", "ce2be4691fa0104cfdd63656cedaf810", 11198933), Common::CS_CZE, ADGF_UNSTABLE | GF_3D, WME_1_9_1),

	// J.U.L.I.A. (English)
	WME_WINENTRY("julia", "",
		WME_ENTRY1s("data.dcp", "c2264b4f8fcd132d2913ff5b6076a24f", 10109741), Common::EN_ANY, ADGF_UNSTABLE | GF_3D, WME_1_9_3),

	// J.U.L.I.A. (English, reported by Duffadash in https://bugs.scummvm.org/ticket/11350)
	WME_WINENTRY("julia", "",
		WME_ENTRY1s("data.dcp", "891058639eb4d9815540c95a495b5a88", 10105692), Common::EN_ANY, ADGF_UNSTABLE | GF_3D, WME_1_9_3),

	// J.U.L.I.A. (English, Bundle in a box-version)
	WME_WINENTRY("julia", "Version 1.2",
		WME_ENTRY1s("data.dcp", "fe90023ccc22f35185b40b910e0d03a2", 10101373), Common::EN_ANY, ADGF_UNSTABLE | GF_3D, WME_1_9_3),

	// J.U.L.I.A. (English) (Demo)
	WME_WINENTRY("julia", "Demo",
		WME_ENTRY1s("data.dcp", "f0bbc3394555a9811f6050dae428cab6", 7655237), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO | GF_3D, WME_1_9_3),

	// J.U.L.I.A. (English) (Greenlight Demo)
	WME_WINENTRY("julia", "Greenlight Demo",
		WME_ENTRY1s("data.dcp", "4befd448d36b0dae9c3ab1aa7cb8b78d", 7271886), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO | GF_3D, WME_1_9_3),

	// J.U.L.I.A.: Among the Stars (HD Ready Version) (Steam, January 2017) (English)
	WME_WINENTRY("juliastars", "HD Ready Version/Steam",
		WME_ENTRY2s("data_sd.dcp", "9949302dfaea943113e2f0ee0dd468be", 4249680,
					"data_sd.dcp", "9949302dfaea943113e2f0ee0dd468be", 4249680), Common::EN_ANY, ADGF_UNSTABLE | GF_IGNORE_HD_FILES, WME_LITE),

	// J.U.L.I.A.: Among the Stars (Full HD Version) (Steam, January 2017) (English)
	WME_WINENTRY("juliastars", "Full HD Version/Steam",
		WME_ENTRY2s("data_hd.dcp", "fd579fa333f117882190993ea4f3bba5", 5164463,
					"data_hd.dcp", "fd579fa333f117882190993ea4f3bba5", 5164463), Common::EN_ANY, ADGF_UNSTABLE | GF_IGNORE_SD_FILES, WME_LITE),

	// J.U.L.I.A.: Among the Stars (HD Ready Version) (Steam, November 2016) (German)
	WME_WINENTRY("juliastars", "HD Ready Version/Steam",
		WME_ENTRY2s("data_sd.dcp", "dfaf7e730a66412f68d11cddb0c8737d", 4505667,
					"german_sd.dcp", "23ceb8625cebfe32aaa5950e89ac68ba", 123326075), Common::DE_DEU, ADGF_UNSTABLE | GF_IGNORE_HD_FILES, WME_LITE),

	// J.U.L.I.A.: Among the Stars (Full HD Version) (Steam, November 2016) (German)
	WME_WINENTRY("juliastars", "Full HD Version/Steam",
		WME_ENTRY2s("data_hd.dcp", "f40b3d0778e37c61cf309d214446d233", 5264780,
					"german_hd.dcp", "8d85f83a3fc8f1bec4e5ba2158b05b1e", 152499998), Common::DE_DEU, ADGF_UNSTABLE | GF_IGNORE_SD_FILES, WME_LITE),

	// J.U.L.I.A.: Among the Stars (HD Ready Version) (Steam) (Spanish fanmade translation)
	WME_WINENTRY("juliastars", "HD Ready Version/Steam",
		WME_ENTRY2s("data_sd.dcp", "da3508bd60025bac35211fb6fc959d88", 5655554,
					"data_sd.dcp", "da3508bd60025bac35211fb6fc959d88", 5655554), Common::ES_ESP, ADGF_UNSTABLE | GF_IGNORE_HD_FILES, WME_LITE),

	// J.U.L.I.A.: Among the Stars (Full HD Version) (Steam) (Spanish fanmade translation)
	WME_WINENTRY("juliastars", "Full HD Version/Steam",
		WME_ENTRY2s("data_hd.dcp", "da3508bd60025bac35211fb6fc959d88", 5655554,
					"data_hd.dcp", "da3508bd60025bac35211fb6fc959d88", 5655554), Common::ES_ESP, ADGF_UNSTABLE | GF_IGNORE_SD_FILES, WME_LITE),

	// J.U.L.I.A.: Among the Stars (HD Ready Version) (GOG) (English)
	WME_WINENTRY("juliastars", "HD Ready Version/GOG",
		WME_ENTRY2s("data_sd.dcp", "da1f147a5f2ee6eb0750678a8b955c93", 4526792,
					"data_sd.dcp", "da1f147a5f2ee6eb0750678a8b955c93", 4526792), Common::EN_ANY, ADGF_UNSTABLE | GF_IGNORE_HD_FILES, WME_LITE),

	// J.U.L.I.A.: Among the Stars (Full HD Version) (GOG) (English)
	WME_WINENTRY("juliastars", "Full HD Version/GOG",
		WME_ENTRY2s("data_hd.dcp", "91dcb65523da943f22fca0c025a2ce8e", 5281911,
					"data_hd.dcp", "91dcb65523da943f22fca0c025a2ce8e", 5281911), Common::EN_ANY, ADGF_UNSTABLE | GF_IGNORE_SD_FILES, WME_LITE),

	// J.U.L.I.A.: Among the Stars (HD Ready Version) (GOG) (German)
	WME_WINENTRY("juliastars", "HD Ready Version/GOG",
		WME_ENTRY2s("data_sd.dcp", "070d13b70e35cd95855ddc1687446631", 4526795,
					"german_sd.dcp", "85eb39225083465225c30261a6bcd63e", 123326134), Common::DE_DEU, ADGF_UNSTABLE | GF_IGNORE_HD_FILES, WME_LITE),

	// J.U.L.I.A.: Among the Stars (Full HD Version) (GOG) (German)
	WME_WINENTRY("juliastars", "Full HD Version/GOG",
		WME_ENTRY2s("data_hd.dcp", "7973ca635255d3791123fd750cb848f2", 5281925,
					"german_hd.dcp", "19a771b1a933b71b889026d53734b0c0", 152500044), Common::DE_DEU, ADGF_UNSTABLE | GF_IGNORE_SD_FILES, WME_LITE),

	// J.U.L.I.A.: Among the Stars (HD Ready Version) (GOG) (Spanish fanmade translation)
	WME_WINENTRY("juliastars", "HD Ready Version/GOG",
		WME_ENTRY2s("data_sd.dcp", "29f4856cc1514bdb86d3b19a39d86d76", 5877935,
					"data_sd.dcp", "29f4856cc1514bdb86d3b19a39d86d76", 5877935), Common::ES_ESP, ADGF_UNSTABLE | GF_IGNORE_HD_FILES, WME_LITE),

	// J.U.L.I.A.: Among the Stars (Full HD Version) (GOG) (Spanish fanmade translation)
	WME_WINENTRY("juliastars", "Full HD Version/GOG",
		WME_ENTRY2s("data_hd.dcp", "29f4856cc1514bdb86d3b19a39d86d76", 5877935,
					"data_hd.dcp", "29f4856cc1514bdb86d3b19a39d86d76", 5877935), Common::ES_ESP, ADGF_UNSTABLE | GF_IGNORE_SD_FILES, WME_LITE),

	// J.U.L.I.A.: Untold (Steam, January 2016)
	WME_WINENTRY("juliauntold", "Steam",
		WME_ENTRY1s("data.dcp", "fe995e26253f6e0a925dd7850fce17a9", 26459827), Common::EN_ANY, ADGF_UNSTABLE, WME_LITE),

	// J.U.L.I.A.: Untold (GOG)
	WME_WINENTRY("juliauntold", "GOG",
		WME_ENTRY1s("data.dcp", "b0aefd82647a26425fe3ee21aabb6283", 26462676), Common::EN_ANY, ADGF_UNSTABLE, WME_LITE),

	// Klaymen Episodes: Pilot
	WME_WINENTRY("klaymen1", "",
		WME_ENTRY1s("data.dcp", "8a066d13d32567ca88a6918bfa3c01fa", 17086470), Common::EN_ANY, ADGF_UNSTABLE, WME_1_8_6),

	// K'NOSSOS
	WME_WINENTRY("knossos", "",
		WME_ENTRY1s("K'NOSSOS.exe", "694e488cc99bb78d2fe906f82d3ee9e1", 1654383903), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// K'NOSSOS (Alpha Demo)
	WME_WINENTRY("knossos", "Alpha Demo",
		WME_ENTRY1s("K'NOSSOS.exe", "694e488cc99bb78d2fe906f82d3ee9e1", 867516208), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_3),

	// Kulivocko (Czech)
	WME_WINENTRY("kulivocko", "",
		WME_ENTRY1s("data.dcp", "44306dc470e9b27474043932eccee02f", 155106392), Common::CS_CZE, ADGF_UNSTABLE, WME_1_9_1),

	// Kulivocko (Czech) (Demo 1)
	WME_WINENTRY("kulivocko", "Demo 1",
		WME_ENTRY1s("data.dcp", "63b164bdfadecbb0deb5da691afb8154", 48362234), Common::CS_CZE, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_1),

	// Kulivocko (Czech) (Demo 2)
	WME_WINENTRY("kulivocko", "Demo 2",
		WME_ENTRY1s("data.dcp", "501c59ddb787646d87dead183199c8ed", 73601289), Common::CS_CZE, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_1),

	// Life In 3 Minutes
	WME_WINENTRY("lifein3minutes", "",
		WME_ENTRY1s("data.dcp", "c6368950e37a95bf098b02b4eaa5b929", 141787214), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

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

	// Limbo of the Lost (English)
	WME_WINENTRY("lotl", "",
		WME_ENTRY1s("data.dcp", "637f2195a08f59e809ca48194a2ee73d", 354879400), Common::EN_ANY, ADGF_UNSTABLE | GF_3D, WME_1_7_2),

	// Limbo of the Lost (Russian)
	WME_WINENTRY("lotl", "",
		WME_ENTRY1s("data.dcp", "2042ea14f2d7c52a139d768e962040c1", 354370575), Common::RU_RUS, ADGF_UNSTABLE | GF_3D, WME_1_7_2),

	// Machu Mayu (Demo)
	WME_WINENTRY("machumayu", "Demo",
		WME_ENTRY1s("data.dcp", "79f17943b93c3ff37b5544a5da0d17d0", 12328601), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_1),

	// Mental Repairs Inc (English)
	WME_WINENTRY("mentalrepairs", "",
		WME_ENTRY2s("data.dcp", "414d423bbff697f22fb38932f030e897", 59518068,
					"english.dcp", "7573eb584e662adbc5fa3b1448e56106", 3160232), Common::EN_ANY, ADGF_UNSTABLE | GF_3D, WME_1_8_6),

	// Mental Repairs Inc (German)
	WME_WINENTRY("mentalrepairs", "",
		WME_ENTRY2s("data.dcp", "414d423bbff697f22fb38932f030e897", 59518068,
					"german.dcp", "af59a05ef29768e7fced3794a7a380a3", 3249142), Common::DE_DEU, ADGF_UNSTABLE | GF_3D, WME_1_8_6),

	// Mental Repairs Inc (Polish)
	WME_WINENTRY("mentalrepairs", "",
		WME_ENTRY2s("data.dcp", "414d423bbff697f22fb38932f030e897", 59518068,
					"string.tab", "d3393ba2445fc7e6a1cdf8fe95c59659", 72143), Common::PL_POL, ADGF_UNSTABLE | GF_3D, WME_1_8_6),

	// Mirage
	WME_WINENTRY("mirage", "",
		WME_ENTRY1s("data.dcp", "d230b0b99c0aa77b9ecd094d8ee5573b", 17844056), Common::EN_ANY, ADGF_UNSTABLE, WME_1_8_11),

	// Miskatonic. Part One
	WME_WINENTRY("miskatonic1", "",
		WME_ENTRY1s("Miskatonic.exe", "21d1ed0375f64c13688e2d082ed1aaa1", 340306712), Common::EN_ANY, ADGF_UNSTABLE | GF_3D, WME_1_9_1),

	// Miskatonic. Part One (alt)
	WME_WINENTRY("miskatonic1", "",
		WME_ENTRY1s("Miskatonic.exe", "4c52d1cdc6dc708f47049b5c88a5b866", 339002997), Common::EN_ANY, ADGF_UNSTABLE | GF_3D, WME_1_9_1),

	// Monday Starts on Saturday
	WME_WINENTRY("msos", "",
		WME_ENTRY1s("data.dcp", "2aa5ab924b05c9539a5a118bc263c2f8", 1049803074), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_1),

	// Mukhtar and his Team
	WME_WINENTRY("mukhtar", "",
		WME_ENTRY1s("data.dcp", "ed1deb8e6b2ce0a50ba26e035bdb7006", 1086177292), Common::RU_RUS, ADGF_UNSTABLE | GF_3D, WME_1_9_1),

	// Mystic Triddle (Demo)
	WME_WINENTRY("mystictriddle", "Demo",
		WME_ENTRY1s("data.dcp", "77b54c83d49f2cd9d78d319bafa8a4bd", 27716095), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_5_2),

	// Myth: A Guff's Tale
	WME_WINENTRY("mythguff", "",
		WME_ENTRY1s("data.dcp", "ef53dddd2264a7d5b13f3d71da5261e4", 675663078), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// Night in the Fog (Demo, 0.4.0212a)
	WME_WINENTRY("nightinthefog", "Demo 0.4.0212a",
		WME_ENTRY1s("game.exe", "73c88cb17e380ff72c2f1d156b4545e9", 128294595), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_1),

	// Night Train Demo
	WME_WINENTRY("nighttrain", "Demo",
		WME_ENTRY1s("data.dcp", "5a027ef84b083a730c9a4c85ec1d3a32", 131760816), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_1),

	// Night Train Demo 2
	WME_WINENTRY("nighttrain", "Demo",
		WME_ENTRY1s("data.dcp", "c4c29461bb070735e564ef6612ea371a", 124495883), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_1),

	// Nose Bound: Episode 1 (Demo) (multi-language)
	WME_WINENTRY("nosebound1", "Demo",
		WME_ENTRY1s("data.dcp", "280f2e063385e2ed4c17b43991666e93", 379558560), Common::UNK_LANG, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_3),

	// Octave (Demo, Beta)
	WME_WINENTRY("octave", "Demo",
		WME_ENTRY1s("data.dcp", "e5705856e81ec2adcaa6324e9f331a6e", 78113318), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_1),

	// Oknytt
	WME_WINENTRY("oknytt", "Version 1.0",
		WME_ENTRY1s("data.dcp", "6456cf8f429905c83f07509f9da536dd", 109502959), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Oknytt (Version 1.12 English) (These are detected along with d_sounds.dcp to avoid mass-detecting in the languages-subfolder.)
	WME_WINENTRY("oknytt", "Version 1.12/Steam",
		WME_ENTRY2s("english.dcp", "cea08a6b5c88f40cb9937f77a72dce2e", 293273567,
					"d_sounds.dcp", "8df683174ef01e6f717e2df926fa0b4a", 154943401), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Oknytt (Version 1.12 German)
	WME_WINENTRY("oknytt", "Version 1.12/Steam",
		WME_ENTRY2s("german.dcp", "a4f8e76dd6bec15656e83a871e36920c", 304292572,
					"d_sounds.dcp", "8df683174ef01e6f717e2df926fa0b4a", 154943401), Common::DE_DEU, ADGF_UNSTABLE, WME_1_9_3),

	// Oknytt (Version 1.12 Russian)
	WME_WINENTRY("oknytt", "Version 1.12/Steam",
		WME_ENTRY2s("russian.dcp", "82e0ae002dd6a4106cbd1e4f8db9cfe0", 362681666,
					"d_sounds.dcp", "8df683174ef01e6f717e2df926fa0b4a", 154943401), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_3),

	// Oknytt (Version 1.13 English) (These are detected along with d_sounds.dcp to avoid mass-detecting in the languages-subfolder.)
	WME_WINENTRY("oknytt", "Version 1.13/Steam",
		WME_ENTRY2s("english.dcp", "d2afd722c78cfe66b7d4250d11f6ae16", 293274135,
					"d_sounds.dcp", "7d04dff8ca11174486bd4b7a80fdcabb", 154943401), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Oknytt (Version 1.13 German)
	WME_WINENTRY("oknytt", "Version 1.13/Steam",
		WME_ENTRY2s("german.dcp", "0fc6401d8d76b04f6da49206ecafa0dc", 304292574,
					"d_sounds.dcp", "7d04dff8ca11174486bd4b7a80fdcabb", 154943401), Common::DE_DEU, ADGF_UNSTABLE, WME_1_9_3),

	// Oknytt (Version 1.13 Russian)
	WME_WINENTRY("oknytt", "Version 1.13/Steam",
		WME_ENTRY2s("russian.dcp", "dd24a1c0b36a82e2b02fb6c1050d4aad", 362681669,
					"d_sounds.dcp", "7d04dff8ca11174486bd4b7a80fdcabb", 154943401), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_3),

	// Oknytt (Version 1.13 Spanish)
	WME_WINENTRY("oknytt", "Version 1.13/Steam",
		WME_ENTRY2s("spanish.dcp", "10c46152cb29581671f3b6b7c229c957", 319406572,
					"d_sounds.dcp", "7d04dff8ca11174486bd4b7a80fdcabb", 154943401), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_3),

	// Oknytt (Version 1.13 French)
	WME_WINENTRY("oknytt", "Version 1.13/Steam",
		WME_ENTRY1s("data.dcp", "da0871ff3dc9fe005e2293a121fa1635", 287940237), Common::FR_FRA, ADGF_UNSTABLE, WME_1_9_3),

	// One (Demo)
	WME_WINENTRY("one", "Demo",
		WME_ENTRY1s("data.dcp", "dc2e1682227716099b6acaf5cbce104d", 1676301), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_2_44),

	// One Helluva Day (Demo) (multi-language)
	WME_WINENTRY("onehelluvaday", "Demo",
		WME_ENTRY1s("data.dcp", "144e23fca7c1c54103dad9c1342de2b6", 229963509), Common::UNK_LANG, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_3),

	// One Helluva Day (Steam Demo) (multi-language)
	WME_WINENTRY("onehelluvaday", "Demo",
		WME_ENTRY1s("data.dcp", "0d79c74140c24f10255617f71b7e45da", 230689994), Common::UNK_LANG, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_3),

	// Open Quest
	WME_WINENTRY("openquest", "",
		WME_ENTRY1s("data.dcp", "16893e3fc15a211a49654ae66f684f28", 82281736), Common::EN_ANY, ADGF_UNSTABLE | GF_LOWSPEC_ASSETS, WME_1_9_1),

	// Paintaria
	WME_WINENTRY("paintaria", "",
		WME_ENTRY1s("data.dcp", "354c08440c98150ff0d4008dd2865880", 48326040), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// Palladion (Alpha6h)
	WME_WINENTRY("palladion", "Alpha6h",
		WME_ENTRY1s("Palladion_Alpha6h.exe", "55aec582159410ec6221d4c3fd53db24", 95240860), Common::DE_DEU, ADGF_UNSTABLE | ADGF_DEMO | GF_3D, WME_1_8_6),

	// Papa's Daughters
	WME_WINENTRY("papasdaughters1", "",
		WME_ENTRY1s("data.dcp", "5d801bc95f28e64c33918380f961f454", 80569934), Common::RU_RUS, ADGF_UNSTABLE, WME_HEROCRAFT),

	// Papa's Daughters 2
	WME_WINENTRY("papasdaughters2", "",
		WME_ENTRY1s("data.dcp", "8f7dfc4b46c01318ba2bd8e1d79a0edb", 107690073), Common::RU_RUS, ADGF_UNSTABLE, WME_HEROCRAFT),

	// Red Comrades 0.2: Operation F. (Fan game) (Demo 2015)
	WME_WINENTRY("petka02", "Demo 2015",
		WME_ENTRY1s("data.dcp", "aeba8e75c26625d744f866813450c1d4", 108623057), Common::RU_RUS, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_1),

	// Red Comrades 0.2: Operation F. (Fan game) (Demo 2017)
	WME_WINENTRY("petka02", "Demo 2017",
		WME_ENTRY1s("data.dcp", "20d138270fd7552be9c1959cbfb8bcd7", 29081611), Common::RU_RUS, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_3),

	// Pigeons in the Park
	WME_WINENTRY("pigeons", "",
		WME_ENTRY1s("data.dcp", "9143a5b6ff8206aefe3c4c643add3ec7", 2611100), Common::EN_ANY, ADGF_UNSTABLE, WME_1_8_1),

	// Pizza Morgana (Demo)
	WME_WINENTRY("pizzamorgana", "Demo",
		WME_ENTRY2s("english.dcp", "7fa6149bb44574109668ce585d6c41c9", 9282608,
					"data.dcp", "a69994c463ff5fcc6fe1800662f5b7d0", 34581370), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO | GF_3D, WME_1_9_1),

	// Pole Chudes
	WME_WINENTRY("polechudes", "",
		WME_ENTRY1s("data.dcp", "3329e9af9cb41e052de8d929ea49b594", 76297454), Common::RU_RUS, ADGF_UNSTABLE, WME_HEROCRAFT),

	// Precious Pills, Blasting Bacon and Mowing Minds
	WME_WINENTRY("preciouspills", "",
		WME_ENTRY1s("data.dcp", "9237dac68dd09e07c5d17d5b904774d0", 22451857), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// Project: Doom (Beta)
	WME_WINENTRY("projectdoom", "Beta",
		WME_ENTRY1s("data.dcp", "d5894b65a40706845434b99870bcab92", 99223761), Common::EN_ANY, ADGF_UNSTABLE, WME_1_8_11),

	// Project Joe (Demo)
	WME_WINENTRY("projectjoe", "Demo",
		WME_ENTRY1s("data.dcp", "ada3c08542901295076b5349e655e73f", 160780037), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_2_44),

	// Project Lonely Robot (Beta)
	WME_WINENTRY("lonelyrobot", "Beta",
		WME_ENTRY1s("data.dcp", "a0cf7ad5bab957416dcda454e9f28ef0", 3420120), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_0),

	// Qajary Cat (Retail) (Persian)
	WME_WINENTRY("qajarycat", "",
		WME_ENTRY1s("data.dcp", "f2b6bf3daf6324e58042f214cdc075ca", 1782529732), Common::FA_IRN, ADGF_UNSTABLE, WME_1_9_1),

	// Qajary Cat (Hayoola) (Persian)
	WME_WINENTRY("qajarycat", "",
		WME_ENTRY1s("data.dcp", "5d91d8fdcade34ad1130b890396da39a", 450901389), Common::FA_IRN, ADGF_UNSTABLE, WME_1_9_1),

	// Qajary Cat (Steam, November 2017) (English)
	WME_WINENTRY("qajarycat", "",
		WME_ENTRY1s("data.dcp", "3e47e3cd94133bdd5facf30ee84389ea", 447760816), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Rebecca Carlson Mystery 01 - Silent Footsteps
	WME_WINENTRY("rebeccacarlson1", "",
		WME_ENTRY1s("data.dcp", "5fac4824b7ffdaee982dc18a45428182", 773144334), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Rebecca Carlson Mystery 01 - Silent Footsteps (Demo)
	WME_WINENTRY("rebeccacarlson1", "Demo",
		WME_ENTRY1s("data.dcp", "a46c41edb1ae5e19a3e0fc5cac3ce83b", 212698165), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_3),

	// On the Tracks of Dinosaurs (Beta 2.5) (English)
	WME_WINENTRY("reptilesquest", "Beta 2.5",
		WME_ENTRY2s("data.dcp", "b624d3b19e37c094801a28d817bc3d76", 27345755,
					"english.dcp", "42188d46ee079b555d578ea2b406fa19", 3897), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO | GF_3D, WME_1_9_1),

	// On the Tracks of Dinosaurs (Beta 2.5) (Italian)
	WME_WINENTRY("reptilesquest", "Beta 2.5",
		WME_ENTRY2s("data.dcp", "b624d3b19e37c094801a28d817bc3d76", 27345755,
					"italian.dcp", "361f41b1151119f66de851f07b78b7cc", 4132), Common::IT_ITA, ADGF_UNSTABLE | ADGF_DEMO | GF_3D, WME_1_9_1),

	// On the Tracks of Dinosaurs (Beta 2.5) (Russian)
	WME_WINENTRY("reptilesquest", "Beta 2.5",
		WME_ENTRY2s("data.dcp", "b624d3b19e37c094801a28d817bc3d76", 27345755,
					"russian.dcp", "90d2bf541ba381bbb3375aa7d31613fb", 42175), Common::RU_RUS, ADGF_UNSTABLE | ADGF_DEMO | GF_3D, WME_1_9_1),

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
					"data.dcp", "cb9865dc7e1db2990a8cf4bc13cf4999", 257643032), Common::ZH_ANY, ADGF_UNSTABLE, WME_1_9_3),

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
					"data.dcp", "9ebb12f6fd7c038d079f81beb3bd96d5", 254185907), Common::ZH_ANY, ADGF_UNSTABLE, WME_1_9_3),

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
					"data.dcp", "9ebb12f6fd7c038d079f81beb3bd96d5", 254185907), Common::LV_LVA, ADGF_UNSTABLE, WME_1_9_3),

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
					"data.dcp", "aecb5deeea7b0baa871fbd0cef35a648", 254219204), Common::ZH_ANY, ADGF_UNSTABLE, WME_1_9_3),

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
					"data.dcp", "aecb5deeea7b0baa871fbd0cef35a648", 254219204), Common::LV_LVA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape Version 1.3.2369 (Polish)
	WME_WINENTRY("reversion1", "Version 1.3.2369",
		WME_ENTRY2s("xlanguage_pl.dcp", "c4ad33f57e1e998169552d521c1d6638", 11532215,
					"data.dcp", "aecb5deeea7b0baa871fbd0cef35a648", 254219204), Common::PL_POL, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape Version 1.3.2369 (Portuguese)
	WME_WINENTRY("reversion1", "Version 1.3.2369",
		WME_ENTRY2s("xlanguage_pt.dcp", "886886b6b14aadac844078de856799a6", 10620797,
					"data.dcp", "aecb5deeea7b0baa871fbd0cef35a648", 254219204), Common::PT_BRA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, July 2014) (Spanish)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "438e04e70a107539a56fcd332d31f04d", 254298579,
					"data.dcp", "438e04e70a107539a56fcd332d31f04d", 254298579), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, July 2014) (German)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "438e04e70a107539a56fcd332d31f04d", 254298579,
					"xlanguage_de.dcp", "c11ede8af2fafd0d32f32cfc6240ffc2", 14040310), Common::DE_DEU, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, July 2014) (English)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "438e04e70a107539a56fcd332d31f04d", 254298579,
					"xlanguage_en.dcp", "c94c947f71d986131ac9554eca99c020", 11339619), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, July 2014) (French)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "438e04e70a107539a56fcd332d31f04d", 254298579,
					"xlanguage_fr.dcp", "10895b608051647a0976738c57dba0be", 11963210), Common::FR_FRA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, July 2014) (Italian)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "438e04e70a107539a56fcd332d31f04d", 254298579,
					"xlanguage_it.dcp", "2468ef3b3c8c4eba9146ae5f31ac5f0c", 11913752), Common::IT_ITA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, July 2014) (Latvian)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "438e04e70a107539a56fcd332d31f04d", 254298579,
					"xlanguage_lv.dcp", "846f1b99257c9f034d4cd5c89986a8c5", 11414925), Common::LV_LVA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, July 2014) (Chinese)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "438e04e70a107539a56fcd332d31f04d", 254298579,
					"xlanguage_nz.dcp", "8dadcc96212743162df313629cd3adfd", 13722261), Common::ZH_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, July 2014) (Polish)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "438e04e70a107539a56fcd332d31f04d", 254298579,
					"xlanguage_pl.dcp", "3a353627d7f19261d8092ef48ed9c1bd", 11532215), Common::PL_POL, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, July 2014) (Portuguese)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "438e04e70a107539a56fcd332d31f04d", 254298579,
					"xlanguage_pt.dcp", "47d7336a1c94ca406f7e4a45b0ddccc4", 10620797), Common::PT_BRA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, August 2014) (Spanish)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
					"data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::ES_ESP, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2014) (German)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
					"xlanguage_de.dcp", "7fa066e8102d8bf84b5451af2ee8859f", 14040310,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::DE_DEU, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2014) (English)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
					"xlanguage_en.dcp", "421a94e151d424ee666709673ddbea5e", 11339619,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::EN_ANY, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2014) (French)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
					"xlanguage_fr.dcp", "4902f9ba3841650becff2d687e7a9963", 11963210,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::FR_FRA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2014) (Italian)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
					"xlanguage_it.dcp", "a248da64e7701a7549d25dbb02e7dca8", 11913752,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::IT_ITA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2014) (Latvian)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
					"xlanguage_lv.dcp", "613a9ada4dffe012b7a25f69818c9353", 11414925,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::LV_LVA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2014) (Chinese)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
					"xlanguage_nz.dcp", "b61971e451832f32262a2e5361b40126", 15631919,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::ZH_ANY, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2014) (Polish)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
					"xlanguage_pl.dcp", "1d95192d1bab8ab25395fc3c1b16f160", 11963326,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::PL_POL, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2014) (Portuguese)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
					"xlanguage_pt.dcp", "5a20a886350ab866e5056696100598e7", 10620797,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::PT_BRA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2014) (Spanish)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
					"data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::ES_ESP, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2014) (German)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
					"xlanguage_de.dcp", "7fa066e8102d8bf84b5451af2ee8859f", 14040310,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::DE_DEU, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2014) (English)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
					"xlanguage_en.dcp", "421a94e151d424ee666709673ddbea5e", 11339619,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::EN_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2014) (French)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
					"xlanguage_fr.dcp", "4902f9ba3841650becff2d687e7a9963", 11963210,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::FR_FRA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2014) (Italian)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
					"xlanguage_it.dcp", "a248da64e7701a7549d25dbb02e7dca8", 11913752,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::IT_ITA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2014) (Latvian)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
					"xlanguage_lv.dcp", "613a9ada4dffe012b7a25f69818c9353", 11414925,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::LV_LVA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2014) (Chinese)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
					"xlanguage_nz.dcp", "b61971e451832f32262a2e5361b40126", 15631919,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::ZH_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2014) (Polish)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
					"xlanguage_pl.dcp", "1d95192d1bab8ab25395fc3c1b16f160", 11963326,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::PL_POL, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2014) (Portuguese)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
					"xlanguage_pt.dcp", "5a20a886350ab866e5056696100598e7", 10620797,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::PT_BRA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, September 2014) (Polish)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
					"xlanguage_pl.dcp", "d38188219fd5b47a0f9c6777ef1ec93e", 11550402,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::PL_POL, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, September 2014) (Polish)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "e9d47580933860b2254e14ec04a514fe", 254310313,
					"xlanguage_pl.dcp", "d38188219fd5b47a0f9c6777ef1ec93e", 11550402,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::PL_POL, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (Spanish)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "2ab1e724ea83e61049cc46a3604a9a53", 254281202,
					"data.dcp", "2ab1e724ea83e61049cc46a3604a9a53", 254281202), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, January 2016) (German)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "2ab1e724ea83e61049cc46a3604a9a53", 254281202,
					"xlanguage_de.dcp", "25aa33603fc2fcec328158221c46a536", 14023446), Common::DE_DEU, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, January 2016) (English)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "2ab1e724ea83e61049cc46a3604a9a53", 254281202,
					"xlanguage_en.dcp", "eb6a32dfe6c964a4c244ee9022bb67c8", 11324849), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, January 2016) (French)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "2ab1e724ea83e61049cc46a3604a9a53", 254281202,
					"xlanguage_fr.dcp", "110153166e64550b0b41d5c27ef2fb46", 11949050), Common::FR_FRA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, January 2016) (Italian)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "2ab1e724ea83e61049cc46a3604a9a53", 254281202,
					"xlanguage_it.dcp", "961c4b62b3657c854c98773d4c54bc45", 11898661), Common::IT_ITA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, January 2016) (Latvian)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "2ab1e724ea83e61049cc46a3604a9a53", 254281202,
					"xlanguage_lv.dcp", "44e5b05c60cebdb1c4d969827d5870e3", 11427642), Common::LV_LVA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, January 2016) (Chinese)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "2ab1e724ea83e61049cc46a3604a9a53", 254281202,
					"xlanguage_nz.dcp", "e2391e04c9f6c3c7a66c9754e1c90594", 13736623), Common::ZH_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, January 2016) (Polish)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "2ab1e724ea83e61049cc46a3604a9a53", 254281202,
					"xlanguage_pl.dcp", "b359baf8e1558fab6656f0d3bf85d1d1", 11543266), Common::PL_POL, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, January 2016) (Portuguese)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "2ab1e724ea83e61049cc46a3604a9a53", 254281202,
					"xlanguage_pt.dcp", "86778ac5438c22ebc0d59be9334fae3a", 10600764), Common::PT_BRA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, January 2016) (Russian)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "2ab1e724ea83e61049cc46a3604a9a53", 254281202,
					"xlanguage_ru.dcp", "43460a03a7069d268250008a79776dff", 10831308), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, January 2016) (Spanish)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
					"data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::ES_ESP, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (German)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
					"xlanguage_de.dcp", "1a234a256680db57d23a475957164cf0", 14023446,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::DE_DEU, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (English)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
					"xlanguage_en.dcp", "86ff4e6b3142eaf6b9135acc9ba3f7b9", 11324849,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::EN_ANY, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (French)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
					"xlanguage_fr.dcp", "423929742fd86381b66f8edb518391a9", 11949050,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::FR_FRA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (Italian)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
					"xlanguage_it.dcp", "b4324c09be872001ff0838e3d1502aec", 11898661,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::IT_ITA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (Latvian)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
					"xlanguage_lv.dcp", "4230c16f5b8625b30b961c0bc4312935", 11427642,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::LV_LVA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (Chinese)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
					"xlanguage_nz.dcp", "71eec69ab20f009154575afe52c1eeed", 13736623,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::ZH_ANY, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (Polish)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
					"xlanguage_pl.dcp", "63d8ba6c1ae1472d8ec9447cb388282a", 11543298,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::PL_POL, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (Portuguese)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
					"xlanguage_pt.dcp", "5fcdc5402993dbb45eb17c641c15afe3", 10600764,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::PT_BRA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (Russian)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
					"xlanguage_ru.dcp", "c8e2bfb6e8adca0d93855d583a6a46c0", 10854300,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::RU_RUS, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (Spanish)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
					"data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::ES_ESP, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (German)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
					"xlanguage_de.dcp", "1a234a256680db57d23a475957164cf0", 14023446,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::DE_DEU, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (English)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
					"xlanguage_en.dcp", "86ff4e6b3142eaf6b9135acc9ba3f7b9", 11324849,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::EN_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (French)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
					"xlanguage_fr.dcp", "423929742fd86381b66f8edb518391a9", 11949050,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::FR_FRA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (Italian)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
					"xlanguage_it.dcp", "b4324c09be872001ff0838e3d1502aec", 11898661,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::IT_ITA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (Latvian)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
					"xlanguage_lv.dcp", "4230c16f5b8625b30b961c0bc4312935", 11427642,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::LV_LVA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (Chinese)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
					"xlanguage_nz.dcp", "71eec69ab20f009154575afe52c1eeed", 13736623,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::ZH_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (Polish)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
					"xlanguage_pl.dcp", "63d8ba6c1ae1472d8ec9447cb388282a", 11543298,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::PL_POL, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (Portuguese)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
					"xlanguage_pt.dcp", "5fcdc5402993dbb45eb17c641c15afe3", 10600764,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::PT_BRA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, January 2016) (Russian)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "c34389202fb2c20cb89687d83f3af4fb", 254298801,
					"xlanguage_ru.dcp", "c8e2bfb6e8adca0d93855d583a6a46c0", 10854300,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::RU_RUS, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (Spanish)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "e63ddf8801404555010c32bf849b3428", 254307881,
					"data.dcp", "e63ddf8801404555010c32bf849b3428", 254307881,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::ES_ESP, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (German)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "e63ddf8801404555010c32bf849b3428", 254307881,
					"xlanguage_de.dcp", "c24e6dc396fa879e6c24e6f5b4afc9c0", 14032355,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::DE_DEU, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (English)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "e63ddf8801404555010c32bf849b3428", 254307881,
					"xlanguage_en.dcp", "d47e3b4c94ce34976481bcd9ba97d1d4", 11333684,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::EN_ANY, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (French)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "e63ddf8801404555010c32bf849b3428", 254307881,
					"xlanguage_fr.dcp", "fe5a3476e54456d2140f1164d8030fc5", 11957951,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::FR_FRA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (Italian)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "e63ddf8801404555010c32bf849b3428", 254307881,
					"xlanguage_it.dcp", "f21108641cf7c48444ec61902133033f", 11905940,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::IT_ITA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (Latvian)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "e63ddf8801404555010c32bf849b3428", 254307881,
					"xlanguage_lv.dcp", "3019b4f0854aa5f25eed35d4e04f589b", 11436318,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::LV_LVA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (Chinese)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "e63ddf8801404555010c32bf849b3428", 254307881,
					"xlanguage_nz.dcp", "1dd070d3f754555782a08bd4d3657ed5", 15652628,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::ZH_ANY, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (Polish)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "e63ddf8801404555010c32bf849b3428", 254307881,
					"xlanguage_pl.dcp", "8ba30c378de8a7a7102ef14f17ad7687", 11982966,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::PL_POL, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (Portuguese)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "e63ddf8801404555010c32bf849b3428", 254307881,
					"xlanguage_pt.dcp", "2de479d003cbb3d83df38e26eeb9e342", 10613872,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::PT_BRA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (Russian)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "e63ddf8801404555010c32bf849b3428", 254307881,
					"xlanguage_ru.dcp", "7f887ba9508c7c844861ec8f427a39ff", 10863181,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::RU_RUS, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (Spanish)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "fe268238ebdf069db1954278f174614f", 254307881,
					"data.dcp", "fe268238ebdf069db1954278f174614f", 254307881,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::ES_ESP, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (German)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "fe268238ebdf069db1954278f174614f", 254307881,
					"xlanguage_de.dcp", "af050ff3795c482502ec76a0d95b1e67", 14032355,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::DE_DEU, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (English)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "fe268238ebdf069db1954278f174614f", 254307881,
					"xlanguage_en.dcp", "7f827f7b37d5dccc569941b32bb12b4b", 11333684,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::EN_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (French)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "fe268238ebdf069db1954278f174614f", 254307881,
					"xlanguage_fr.dcp", "b31a1cc4ce0b2280bff48aac8bfb7c3f", 11957951,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::FR_FRA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (Italian)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "fe268238ebdf069db1954278f174614f", 254307881,
					"xlanguage_it.dcp", "75f20f0ac0c77c0e8c33838df9e20abc", 11905940,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::IT_ITA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (Latvian)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "fe268238ebdf069db1954278f174614f", 254307881,
					"xlanguage_lv.dcp", "32b6f5d51a7a4e4e52a89146b02d69a2", 11436318,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::LV_LVA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (Chinese)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "fe268238ebdf069db1954278f174614f", 254307881,
					"xlanguage_nz.dcp", "6647dfd8dc5a1858aaba6571ade3fccb", 13742970,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::ZH_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (Polish)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "fe268238ebdf069db1954278f174614f", 254307881,
					"xlanguage_pl.dcp", "8762cbda33ddb6def83890761c608652", 11551887,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::PL_POL, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (Portuguese)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "fe268238ebdf069db1954278f174614f", 254307881,
					"xlanguage_pt.dcp", "4be1f4670d6d1719bdf545a273af83b7", 10613872,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::PT_BRA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (Russian)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "fe268238ebdf069db1954278f174614f", 254307881,
					"xlanguage_ru.dcp", "4d5cbc9424e4a9f04ae161ede75f9d67", 10863181,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::RU_RUS, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, August 2016) (Spanish)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "2ac5dac7b06ab6ca54beaee85b60c0da", 254290282,
					"data.dcp", "2ac5dac7b06ab6ca54beaee85b60c0da", 254290282), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, August 2016) (German)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "2ac5dac7b06ab6ca54beaee85b60c0da", 254290282,
					"xlanguage_de.dcp", "03018955611279d3035ed4c44dc3d739", 14032355), Common::DE_DEU, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, August 2016) (English)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "2ac5dac7b06ab6ca54beaee85b60c0da", 254290282,
					"xlanguage_en.dcp", "f06b7408164cdc08e9aa94dc29001ed0", 11333684), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, August 2016) (French)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "2ac5dac7b06ab6ca54beaee85b60c0da", 254290282,
					"xlanguage_fr.dcp", "f489d5893560843c1eacbf5f76eb7638", 11957951), Common::FR_FRA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, August 2016) (Italian)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "2ac5dac7b06ab6ca54beaee85b60c0da", 254290282,
					"xlanguage_it.dcp", "82eb7fc23fee6db2b350126804b02139", 11905940), Common::IT_ITA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, August 2016) (Latvian)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "2ac5dac7b06ab6ca54beaee85b60c0da", 254290282,
					"xlanguage_lv.dcp", "49f76e07d2066ec6c4a5507ae9c1aad8", 11436318), Common::LV_LVA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, August 2016) (Chinese)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "2ac5dac7b06ab6ca54beaee85b60c0da", 254290282,
					"xlanguage_nz.dcp", "21e2f1f04d7d683e926cf1934e488630", 13742970), Common::ZH_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, August 2016) (Polish)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "2ac5dac7b06ab6ca54beaee85b60c0da", 254290282,
					"xlanguage_pl.dcp", "bf880fbe62c7743063b37447e228d3a8", 11551855), Common::PL_POL, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, August 2016) (Portuguese)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "2ac5dac7b06ab6ca54beaee85b60c0da", 254290282,
					"xlanguage_pt.dcp", "d81ab6c6316114b70e64e32b761f1084", 10613872), Common::PT_BRA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, August 2016) (Russian)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "2ac5dac7b06ab6ca54beaee85b60c0da", 254290282,
					"xlanguage_ru.dcp", "7f5379114f17593d591e739dc3f873fc", 10840189), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, September 2016) (Portuguese)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "2ac5dac7b06ab6ca54beaee85b60c0da", 254290282,
					"xlanguage_pt.dcp", "bb163dacb99fe93c4a9845fcca72c383", 10639302), Common::PT_BRA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, September 2016) (Portuguese)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "e63ddf8801404555010c32bf849b3428", 254307881,
					"xlanguage_pt.dcp", "8b6d596a84ea8730e0fd0e60c79083de", 10639302,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::PT_BRA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, September 2016) (Portuguese)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "fe268238ebdf069db1954278f174614f", 254307881,
					"xlanguage_pt.dcp", "8b6d596a84ea8730e0fd0e60c79083de", 10639302,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::PT_BRA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (Spanish)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "5e4d40075f69fa7702530e38c349d2fd", 254293949,
					"data.dcp", "5e4d40075f69fa7702530e38c349d2fd", 254293949), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, November 2016) (German)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "5e4d40075f69fa7702530e38c349d2fd", 254293949,
					"xlanguage_de.dcp", "d2471dceea100734afeb4ea77a0153ec", 14036212), Common::DE_DEU, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, November 2016) (English)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "5e4d40075f69fa7702530e38c349d2fd", 254293949,
					"xlanguage_en.dcp", "bf22fff27640014127929fe5347e1460", 11336876), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, November 2016) (French)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "5e4d40075f69fa7702530e38c349d2fd", 254293949,
					"xlanguage_fr.dcp", "5beabe999a110f31bddbc868f743c640", 11961697), Common::FR_FRA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, November 2016) (Italian)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "5e4d40075f69fa7702530e38c349d2fd", 254293949,
					"xlanguage_it.dcp", "a98fa205490f7205d8cc301ab05ed115", 11564269), Common::IT_ITA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, November 2016) (Latvian)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "5e4d40075f69fa7702530e38c349d2fd", 254293949,
					"xlanguage_lv.dcp", "6983daaf6bba6a496a73e272dbe274c4", 11436318), Common::LV_LVA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, November 2016) (Chinese)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "5e4d40075f69fa7702530e38c349d2fd", 254293949,
					"xlanguage_nz.dcp", "51b9d4cb6c4a571503e1d75fc4b4024c", 13739377), Common::ZH_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, November 2016) (Polish)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "5e4d40075f69fa7702530e38c349d2fd", 254293949,
					"xlanguage_pl.dcp", "bf880fbe62c7743063b37447e228d3a8", 11551855), Common::PL_POL, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, November 2016) (Portuguese)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "5e4d40075f69fa7702530e38c349d2fd", 254293949,
					"xlanguage_pt.dcp", "44a1ec3499e968373140dfef041099af", 10642716), Common::PT_BRA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, November 2016) (Russian)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "5e4d40075f69fa7702530e38c349d2fd", 254293949,
					"xlanguage_ru.dcp", "c5860426055e72a0d1ce3467e1835572", 10843778), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, November 2016) (Spanish)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
					"data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::ES_ESP, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (German)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
					"xlanguage_de.dcp", "41e6f33c9c9fd9c8942153cc75975c06", 14036212,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::DE_DEU, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (English)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
					"xlanguage_en.dcp", "1740dfcf1008d5bf5cfff091d17df145", 11336876,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::EN_ANY, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (French)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
					"xlanguage_fr.dcp", "3638aba23c17a06ee03253352345cf74", 11961697,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::FR_FRA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (Italian)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
					"xlanguage_it.dcp", "80abe88fb49b8e30aa9eb7ce8bfa1600", 11564269,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::IT_ITA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (Latvian)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
					"xlanguage_lv.dcp", "aa6be87b3227a7786206f05786e9982a", 11436318,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::LV_LVA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (Chinese)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
					"xlanguage_nz.dcp", "858fcd838458a584b4c0c71e1b7524c8", 15649035,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::ZH_ANY, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (Polish)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
					"xlanguage_pl.dcp", "8ba30c378de8a7a7102ef14f17ad7687", 11982966,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::PL_POL, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (Portuguese)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
					"xlanguage_pt.dcp", "f3c2c3fa8fa63a41552cae6789c7138d", 10642716,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::PT_BRA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (Russian)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
					"xlanguage_ru.dcp", "4e01e8d4759fddbcb6eb07d19c0c3335", 10866770,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::RU_RUS, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (Spanish)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
					"data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::ES_ESP, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (German)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
					"xlanguage_de.dcp", "41e6f33c9c9fd9c8942153cc75975c06", 14036212,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::DE_DEU, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (English)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
					"xlanguage_en.dcp", "1740dfcf1008d5bf5cfff091d17df145", 11336876,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::EN_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (French)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
					"xlanguage_fr.dcp", "3638aba23c17a06ee03253352345cf74", 11961697,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::FR_FRA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (Italian)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
					"xlanguage_it.dcp", "80abe88fb49b8e30aa9eb7ce8bfa1600", 11564269,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::IT_ITA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (Latvian)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
					"xlanguage_lv.dcp", "aa6be87b3227a7786206f05786e9982a", 11436318,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::LV_LVA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (Chinese)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
					"xlanguage_nz.dcp", "858fcd838458a584b4c0c71e1b7524c8", 15649035,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::ZH_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (Polish)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
					"xlanguage_pl.dcp", "8762cbda33ddb6def83890761c608652", 11551887,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::PL_POL, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (Portuguese)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
					"xlanguage_pt.dcp", "f3c2c3fa8fa63a41552cae6789c7138d", 10642716,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::PT_BRA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, November 2016) (Russian)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "0cdbcf31b0a4aa66c1307a66962cb94e", 254311548,
					"xlanguage_ru.dcp", "4e01e8d4759fddbcb6eb07d19c0c3335", 10866770,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::RU_RUS, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, July 2019) (Spanish)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "371f8cde148050eda33e60cf059bf894", 283546874,
					"data.dcp", "371f8cde148050eda33e60cf059bf894", 283546874), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, July 2019) (German)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "371f8cde148050eda33e60cf059bf894", 283546874,
					"xlanguage_de.dcp", "d2471dceea100734afeb4ea77a0153ec", 14036212), Common::DE_DEU, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, July 2019) (English)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "371f8cde148050eda33e60cf059bf894", 283546874,
					"xlanguage_en.dcp", "bf22fff27640014127929fe5347e1460", 11336876), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, July 2019) (French)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "371f8cde148050eda33e60cf059bf894", 283546874,
					"xlanguage_fr.dcp", "5beabe999a110f31bddbc868f743c640", 11961697), Common::FR_FRA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, July 2019) (Italian)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "371f8cde148050eda33e60cf059bf894", 283546874,
					"xlanguage_it.dcp", "a98fa205490f7205d8cc301ab05ed115", 11564269), Common::IT_ITA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, July 2019) (Latvian)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "371f8cde148050eda33e60cf059bf894", 283546874,
					"xlanguage_lv.dcp", "6983daaf6bba6a496a73e272dbe274c4", 11436318), Common::LV_LVA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, July 2019) (Chinese)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "371f8cde148050eda33e60cf059bf894", 283546874,
					"xlanguage_nz.dcp", "51b9d4cb6c4a571503e1d75fc4b4024c", 13739377), Common::ZH_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, July 2019) (Polish)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "371f8cde148050eda33e60cf059bf894", 283546874,
					"xlanguage_pl.dcp", "bf880fbe62c7743063b37447e228d3a8", 11551855), Common::PL_POL, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, July 2019) (Portuguese)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "371f8cde148050eda33e60cf059bf894", 283546874,
					"xlanguage_pt.dcp", "44a1ec3499e968373140dfef041099af", 10642716), Common::PT_BRA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, July 2019) (Russian)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "371f8cde148050eda33e60cf059bf894", 283546874,
					"xlanguage_ru.dcp", "c5860426055e72a0d1ce3467e1835572", 10843778), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, July 2019) (Simplified Chinese)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "371f8cde148050eda33e60cf059bf894", 283546874,
					"xlanguage_zh_s.dcp", "9df07bd28ee2378f684892f079e4166b", 12625743), Common::ZH_CHN, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, July 2019) (Traditional Chinese)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "371f8cde148050eda33e60cf059bf894", 283546874,
					"xlanguage_zh_t.dcp", "3ddeeddbc625077fe8ea0e53108fcdba", 12551685), Common::ZH_TWN, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, July 2019) (Spanish)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "8e8e5f889b93be60ec9b4fb49fa57f35", 283564473,
					"data.dcp", "8e8e5f889b93be60ec9b4fb49fa57f35", 283564473,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::ES_ESP, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, July 2019) (German)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "8e8e5f889b93be60ec9b4fb49fa57f35", 283564473,
					"xlanguage_de.dcp", "41e6f33c9c9fd9c8942153cc75975c06", 14036212,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::DE_DEU, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, July 2019) (English)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "8e8e5f889b93be60ec9b4fb49fa57f35", 283564473,
					"xlanguage_en.dcp", "1740dfcf1008d5bf5cfff091d17df145", 11336876,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::EN_ANY, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, July 2019) (French)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "8e8e5f889b93be60ec9b4fb49fa57f35", 283564473,
					"xlanguage_fr.dcp", "3638aba23c17a06ee03253352345cf74", 11961697,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::FR_FRA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, July 2019) (Italian)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "8e8e5f889b93be60ec9b4fb49fa57f35", 283564473,
					"xlanguage_it.dcp", "80abe88fb49b8e30aa9eb7ce8bfa1600", 11564269,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::IT_ITA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, July 2019) (Latvian)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "8e8e5f889b93be60ec9b4fb49fa57f35", 283564473,
					"xlanguage_lv.dcp", "aa6be87b3227a7786206f05786e9982a", 11436318,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::LV_LVA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, July 2019) (Chinese)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "8e8e5f889b93be60ec9b4fb49fa57f35", 283564473,
					"xlanguage_nz.dcp", "858fcd838458a584b4c0c71e1b7524c8", 15649035,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::ZH_ANY, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, July 2019) (Polish)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "8e8e5f889b93be60ec9b4fb49fa57f35", 283564473,
					"xlanguage_pl.dcp", "8ba30c378de8a7a7102ef14f17ad7687", 11982966,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::PL_POL, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, July 2019) (Portuguese)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "8e8e5f889b93be60ec9b4fb49fa57f35", 283564473,
					"xlanguage_pt.dcp", "f3c2c3fa8fa63a41552cae6789c7138d", 10642716,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::PT_BRA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, July 2019) (Russian)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "8e8e5f889b93be60ec9b4fb49fa57f35", 283564473,
					"xlanguage_ru.dcp", "4e01e8d4759fddbcb6eb07d19c0c3335", 10866770,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::RU_RUS, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, July 2019) (Spanish)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "5a68f570fa4ff84b1479794447d69f48", 283564473,
					"data.dcp", "5a68f570fa4ff84b1479794447d69f48", 283564473,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::ES_ESP, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, July 2019) (German)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "5a68f570fa4ff84b1479794447d69f48", 283564473,
					"xlanguage_de.dcp", "41e6f33c9c9fd9c8942153cc75975c06", 14036212,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::DE_DEU, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, July 2019) (English)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "5a68f570fa4ff84b1479794447d69f48", 283564473,
					"xlanguage_en.dcp", "1740dfcf1008d5bf5cfff091d17df145", 11336876,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::EN_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, July 2019) (French)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "5a68f570fa4ff84b1479794447d69f48", 283564473,
					"xlanguage_fr.dcp", "3638aba23c17a06ee03253352345cf74", 11961697,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::FR_FRA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, July 2019) (Italian)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "5a68f570fa4ff84b1479794447d69f48", 283564473,
					"xlanguage_it.dcp", "80abe88fb49b8e30aa9eb7ce8bfa1600", 11564269,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::IT_ITA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, July 2019) (Latvian)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "5a68f570fa4ff84b1479794447d69f48", 283564473,
					"xlanguage_lv.dcp", "aa6be87b3227a7786206f05786e9982a", 11436318,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::LV_LVA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, July 2019) (Chinese)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "5a68f570fa4ff84b1479794447d69f48", 283564473,
					"xlanguage_nz.dcp", "858fcd838458a584b4c0c71e1b7524c8", 15649035,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::ZH_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, July 2019) (Polish)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "5a68f570fa4ff84b1479794447d69f48", 283564473,
					"xlanguage_pl.dcp", "8762cbda33ddb6def83890761c608652", 11551887,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::PL_POL, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, July 2019) (Portuguese)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "5a68f570fa4ff84b1479794447d69f48", 283564473,
					"xlanguage_pt.dcp", "f3c2c3fa8fa63a41552cae6789c7138d", 10642716,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::PT_BRA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, July 2019) (Russian)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "5a68f570fa4ff84b1479794447d69f48", 283564473,
					"xlanguage_ru.dcp", "4e01e8d4759fddbcb6eb07d19c0c3335", 10866770,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::RU_RUS, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, February 2020) (Spanish)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "89378191c6ef7a1e8943d51eab577ca9", 283546932,
					"data.dcp", "89378191c6ef7a1e8943d51eab577ca9", 283546932), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, February 2020) (German)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "89378191c6ef7a1e8943d51eab577ca9", 283546932,
					"xlanguage_de.dcp", "d2471dceea100734afeb4ea77a0153ec", 14036212), Common::DE_DEU, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, February 2020) (English)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "89378191c6ef7a1e8943d51eab577ca9", 283546932,
					"xlanguage_en.dcp", "bf22fff27640014127929fe5347e1460", 11336876), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, February 2020) (French)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "89378191c6ef7a1e8943d51eab577ca9", 283546932,
					"xlanguage_fr.dcp", "5beabe999a110f31bddbc868f743c640", 11961697), Common::FR_FRA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, February 2020) (Italian)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "89378191c6ef7a1e8943d51eab577ca9", 283546932,
					"xlanguage_it.dcp", "a98fa205490f7205d8cc301ab05ed115", 11564269), Common::IT_ITA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, February 2020) (Latvian)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "89378191c6ef7a1e8943d51eab577ca9", 283546932,
					"xlanguage_lv.dcp", "6983daaf6bba6a496a73e272dbe274c4", 11436318), Common::LV_LVA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, February 2020) (Chinese)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "89378191c6ef7a1e8943d51eab577ca9", 283546932,
					"xlanguage_nz.dcp", "51b9d4cb6c4a571503e1d75fc4b4024c", 13739377), Common::ZH_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, February 2020) (Polish)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "89378191c6ef7a1e8943d51eab577ca9", 283546932,
					"xlanguage_pl.dcp", "bf880fbe62c7743063b37447e228d3a8", 11551855), Common::PL_POL, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, February 2020) (Portuguese)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "89378191c6ef7a1e8943d51eab577ca9", 283546932,
					"xlanguage_pt.dcp", "44a1ec3499e968373140dfef041099af", 10642716), Common::PT_BRA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, February 2020) (Russian)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "89378191c6ef7a1e8943d51eab577ca9", 283546932,
					"xlanguage_ru.dcp", "ecbcf6b59c5df992d8a971cfd01dbbc3", 15026085), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, February 2020) (Simplified Chinese)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "89378191c6ef7a1e8943d51eab577ca9", 283546932,
					"xlanguage_zh_s.dcp", "9df07bd28ee2378f684892f079e4166b", 12625743), Common::ZH_CHN, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, February 2020) (Traditional Chinese)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "89378191c6ef7a1e8943d51eab577ca9", 283546932,
					"xlanguage_zh_t.dcp", "3ddeeddbc625077fe8ea0e53108fcdba", 12551685), Common::ZH_TWN, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, February 2020) (Spanish)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "db3b7d57a9b9decf188328c14bb8118e", 283564531,
					"data.dcp", "db3b7d57a9b9decf188328c14bb8118e", 283564531,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::ES_ESP, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, February 2020) (German)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "db3b7d57a9b9decf188328c14bb8118e", 283564531,
					"xlanguage_de.dcp", "41e6f33c9c9fd9c8942153cc75975c06", 14036212,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::DE_DEU, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, February 2020) (English)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "db3b7d57a9b9decf188328c14bb8118e", 283564531,
					"xlanguage_en.dcp", "1740dfcf1008d5bf5cfff091d17df145", 11336876,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::EN_ANY, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, February 2020) (French)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "db3b7d57a9b9decf188328c14bb8118e", 283564531,
					"xlanguage_fr.dcp", "3638aba23c17a06ee03253352345cf74", 11961697,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::FR_FRA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, February 2020) (Italian)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "db3b7d57a9b9decf188328c14bb8118e", 283564531,
					"xlanguage_it.dcp", "80abe88fb49b8e30aa9eb7ce8bfa1600", 11564269,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::IT_ITA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, February 2020) (Latvian)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "db3b7d57a9b9decf188328c14bb8118e", 283564531,
					"xlanguage_lv.dcp", "aa6be87b3227a7786206f05786e9982a", 11436318,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::LV_LVA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, February 2020) (Chinese)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "db3b7d57a9b9decf188328c14bb8118e", 283564531,
					"xlanguage_nz.dcp", "858fcd838458a584b4c0c71e1b7524c8", 15649035,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::ZH_ANY, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, February 2020) (Polish)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "db3b7d57a9b9decf188328c14bb8118e", 283564531,
					"xlanguage_pl.dcp", "8ba30c378de8a7a7102ef14f17ad7687", 11982966,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::PL_POL, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, February 2020) (Portuguese)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "db3b7d57a9b9decf188328c14bb8118e", 283564531,
					"xlanguage_pt.dcp", "f3c2c3fa8fa63a41552cae6789c7138d", 10642716,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::PT_BRA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, February 2020) (Russian)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "db3b7d57a9b9decf188328c14bb8118e", 283564531,
					"xlanguage_ru.dcp", "000c669e1712c4fe95050d49d55ad6ef", 15049077,
					"Linux.dcp", "2e27778dce77db368b8353895a32930f", 3788181), Common::RU_RUS, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, February 2020) (Spanish)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "0edb5877848e820b79d66002fe927a03", 283564531,
					"data.dcp", "0edb5877848e820b79d66002fe927a03", 283564531,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::ES_ESP, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, February 2020) (German)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "0edb5877848e820b79d66002fe927a03", 283564531,
					"xlanguage_de.dcp", "41e6f33c9c9fd9c8942153cc75975c06", 14036212,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::DE_DEU, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, February 2020) (English)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "0edb5877848e820b79d66002fe927a03", 283564531,
					"xlanguage_en.dcp", "1740dfcf1008d5bf5cfff091d17df145", 11336876,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::EN_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, February 2020) (French)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "0edb5877848e820b79d66002fe927a03", 283564531,
					"xlanguage_fr.dcp", "3638aba23c17a06ee03253352345cf74", 11961697,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::FR_FRA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, February 2020) (Italian)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "0edb5877848e820b79d66002fe927a03", 283564531,
					"xlanguage_it.dcp", "80abe88fb49b8e30aa9eb7ce8bfa1600", 11564269,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::IT_ITA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, February 2020) (Latvian)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "0edb5877848e820b79d66002fe927a03", 283564531,
					"xlanguage_lv.dcp", "aa6be87b3227a7786206f05786e9982a", 11436318,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::LV_LVA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, February 2020) (Chinese)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "0edb5877848e820b79d66002fe927a03", 283564531,
					"xlanguage_nz.dcp", "858fcd838458a584b4c0c71e1b7524c8", 15649035,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::ZH_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, February 2020) (Polish)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "0edb5877848e820b79d66002fe927a03", 283564531,
					"xlanguage_pl.dcp", "8762cbda33ddb6def83890761c608652", 11551887,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::PL_POL, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, February 2020) (Portuguese)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "0edb5877848e820b79d66002fe927a03", 283564531,
					"xlanguage_pt.dcp", "f3c2c3fa8fa63a41552cae6789c7138d", 10642716,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::PT_BRA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, February 2020) (Russian)
	WME_PLATENTRY("reversion1", "Steam",
		WME_ENTRY3s("data.dcp", "0edb5877848e820b79d66002fe927a03", 283564531,
					"xlanguage_ru.dcp", "756531c818db19d91f7747d0567e37c3", 15049077,
					"Mac.dcp", "50911598561dc0ac7d050aeadbb58815", 4866263), Common::RU_RUS, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Escape (Steam, April 2020) (Spanish)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "9074481da1b8a3f89f6951cba5031fbd", 283546933,
					"data.dcp", "9074481da1b8a3f89f6951cba5031fbd", 283546933), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, April 2020) (German)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "9074481da1b8a3f89f6951cba5031fbd", 283546933,
					"xlanguage_de.dcp", "d2471dceea100734afeb4ea77a0153ec", 14036212), Common::DE_DEU, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, April 2020) (English)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "9074481da1b8a3f89f6951cba5031fbd", 283546933,
					"xlanguage_en.dcp", "bf22fff27640014127929fe5347e1460", 11336876), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, April 2020) (French)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "9074481da1b8a3f89f6951cba5031fbd", 283546933,
					"xlanguage_fr.dcp", "5beabe999a110f31bddbc868f743c640", 11961697), Common::FR_FRA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, April 2020) (Italian)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "9074481da1b8a3f89f6951cba5031fbd", 283546933,
					"xlanguage_it.dcp", "a98fa205490f7205d8cc301ab05ed115", 11564269), Common::IT_ITA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, April 2020) (Latvian)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "9074481da1b8a3f89f6951cba5031fbd", 283546933,
					"xlanguage_lv.dcp", "6983daaf6bba6a496a73e272dbe274c4", 11436318), Common::LV_LVA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, April 2020) (Chinese)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "9074481da1b8a3f89f6951cba5031fbd", 283546933,
					"xlanguage_nz.dcp", "51b9d4cb6c4a571503e1d75fc4b4024c", 13739377), Common::ZH_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, April 2020) (Polish)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "9074481da1b8a3f89f6951cba5031fbd", 283546933,
					"xlanguage_pl.dcp", "bf880fbe62c7743063b37447e228d3a8", 11551855), Common::PL_POL, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, April 2020) (Portuguese)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "9074481da1b8a3f89f6951cba5031fbd", 283546933,
					"xlanguage_pt.dcp", "44a1ec3499e968373140dfef041099af", 10642716), Common::PT_BRA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, April 2020) (Russian)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "9074481da1b8a3f89f6951cba5031fbd", 283546933,
					"xlanguage_ru.dcp", "ecbcf6b59c5df992d8a971cfd01dbbc3", 15026085), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, April 2020) (Serbian)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "9074481da1b8a3f89f6951cba5031fbd", 283546933,
					"xlanguage_sr.dcp", "2455cc3770766787b6d7ca7301079230", 10702894), Common::SR_SRB, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, April 2020) (Simplified Chinese)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "9074481da1b8a3f89f6951cba5031fbd", 283546933,
					"xlanguage_zh_s.dcp", "9df07bd28ee2378f684892f079e4166b", 12625743), Common::ZH_CHN, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Escape (Steam, April 2020) (Traditional Chinese)
	WME_WINENTRY("reversion1", "Steam",
		WME_ENTRY2s("data.dcp", "9074481da1b8a3f89f6951cba5031fbd", 283546933,
					"xlanguage_zh_t.dcp", "3ddeeddbc625077fe8ea0e53108fcdba", 12551685), Common::ZH_TWN, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Chinese)
	WME_WINENTRY("reversion2", "",
		WME_ENTRY2s("xlanguage_nz.dcp", "8c3709474a87a7876109025dff41ff3f", 8746015,
					"data.dcp", "f7938cbfdc48f07934550245a3286921", 255672016), Common::ZH_ANY, ADGF_UNSTABLE, WME_1_9_3),

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
					"Linux.dcp", "21858bd77dc86b03f701fd47900e2f51", 984535), Common::ZH_ANY, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting Version 2.0.2412 (English)
	WME_PLATENTRY("reversion2", "Version 2.0.2412",
		WME_ENTRY3s("data.dcp", "f4ffc4df24b7bebad56a24930f33a2bc", 255766600,
					"xlanguage_en.dcp", "0598bf752ce93b42bcaf1094df537c7b", 8533057,
					"Linux.dcp", "21858bd77dc86b03f701fd47900e2f51", 984535), Common::EN_ANY, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Desura, before April 2014) (Spanish)
	WME_WINENTRY("reversion2", "Desura",
		WME_ENTRY2s("data.dcp", "c4aa6295bc6cef6995430232298fdd5f", 255654527,
					"data.dcp", "c4aa6295bc6cef6995430232298fdd5f", 255654527), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Desura, before April 2014) (English)
	WME_WINENTRY("reversion2", "Desura",
		WME_ENTRY2s("data.dcp", "c4aa6295bc6cef6995430232298fdd5f", 255654527,
					"xlanguage_en.dcp", "74f3c75dc4f62e784682016b37990122", 8440487), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Desura, before April 2014) (Chinese)
	WME_WINENTRY("reversion2", "Desura",
		WME_ENTRY2s("data.dcp", "c4aa6295bc6cef6995430232298fdd5f", 255654527,
					"xlanguage_nz.dcp", "42f775cb68f7718d0751758a66816b98", 8734800), Common::ZH_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, July 2014) (Spanish)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "706adf37f47db7306b27ad75c35120e5", 256020490,
					"data.dcp", "706adf37f47db7306b27ad75c35120e5", 256020490), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, July 2014) (English)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "706adf37f47db7306b27ad75c35120e5", 256020490,
					"xlanguage_en.dcp", "be5b15b4c591481d72a1bd869d7f1e99", 8533063), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, July 2014) (Chinese)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "706adf37f47db7306b27ad75c35120e5", 256020490,
					"xlanguage_nz.dcp", "6aa4147492146055ec60d5a288c5ac60", 8827469), Common::ZH_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, July 2014) (Spanish)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "c53b2f750ca06b0559c5776bea4be854", 256038089,
					"data.dcp", "c53b2f750ca06b0559c5776bea4be854", 256038089,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::ES_ESP, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, July 2014) (English)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "c53b2f750ca06b0559c5776bea4be854", 256038089,
					"xlanguage_en.dcp", "476780a51ecae86d1d1d6cacd25793ae", 8533063,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::EN_ANY, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, July 2014) (Chinese)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "c53b2f750ca06b0559c5776bea4be854", 256038089,
					"xlanguage_nz.dcp", "e2bfc09860a5448c65edc75812b34129", 10737127,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::ZH_ANY, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, August 2014) (Spanish)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "2e5d0c90715f8a19ef2f590e9535a907", 256020512,
					"data.dcp", "2e5d0c90715f8a19ef2f590e9535a907", 256020512), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, August 2014) (English)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "2e5d0c90715f8a19ef2f590e9535a907", 256020512,
					"xlanguage_en.dcp", "be5b15b4c591481d72a1bd869d7f1e99", 8533063), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, August 2014) (Chinese)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "2e5d0c90715f8a19ef2f590e9535a907", 256020512,
					"xlanguage_nz.dcp", "6aa4147492146055ec60d5a288c5ac60", 8827469), Common::ZH_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, August 2014) (Spanish)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "970f577fea6e5e78abdf84f2ed26f545", 256038111,
					"data.dcp", "970f577fea6e5e78abdf84f2ed26f545", 256038111,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::ES_ESP, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, August 2014) (English)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "970f577fea6e5e78abdf84f2ed26f545", 256038111,
					"xlanguage_en.dcp", "476780a51ecae86d1d1d6cacd25793ae", 8533063,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::EN_ANY, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, August 2014) (Chinese)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "970f577fea6e5e78abdf84f2ed26f545", 256038111,
					"xlanguage_nz.dcp", "e2bfc09860a5448c65edc75812b34129", 10737127,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::ZH_ANY, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, August 2014) (Spanish)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "970f577fea6e5e78abdf84f2ed26f545", 256038111,
					"data.dcp", "970f577fea6e5e78abdf84f2ed26f545", 256038111,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::ES_ESP, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, August 2014) (English)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "970f577fea6e5e78abdf84f2ed26f545", 256038111,
					"xlanguage_en.dcp", "476780a51ecae86d1d1d6cacd25793ae", 8533063,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::EN_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, August 2014) (Chinese)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "970f577fea6e5e78abdf84f2ed26f545", 256038111,
					"xlanguage_nz.dcp", "e2bfc09860a5448c65edc75812b34129", 10737127,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::ZH_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, February 2016) (Spanish)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "6e507ba64db28dcd766a5fe41c95e152", 256017952,
					"data.dcp", "6e507ba64db28dcd766a5fe41c95e152", 256017952), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, February 2016) (English)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "6e507ba64db28dcd766a5fe41c95e152", 256017952,
					"xlanguage_en.dcp", "0542c42dcaacfdfd29f6e7009a061b1f", 8523503), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, February 2016) (Chinese)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "6e507ba64db28dcd766a5fe41c95e152", 256017952,
					"xlanguage_nz.dcp", "299aee7eaf5e861da1f993114f11bf3e", 8845182), Common::ZH_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, February 2016) (Russian)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "6e507ba64db28dcd766a5fe41c95e152", 256017952,
					"xlanguage_ru.dcp", "15cd8cba2d58413707f03ff3b821e964", 10418988), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, February 2016) (Spanish)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "cfc3be215c525d1171ccdab3236df45b", 256035551,
					"data.dcp", "cfc3be215c525d1171ccdab3236df45b", 256035551,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::ES_ESP, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, February 2016) (English)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "cfc3be215c525d1171ccdab3236df45b", 256035551,
					"xlanguage_en.dcp", "ebb6433c36d0dd38a3ac8fe1c2971614", 8523503,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::EN_ANY, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, February 2016) (Chinese)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "cfc3be215c525d1171ccdab3236df45b", 256035551,
					"xlanguage_nz.dcp", "885d3f7345606cbd159b16ddb46a7146", 10754840,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::ZH_ANY, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, February 2016) (Russian)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "cfc3be215c525d1171ccdab3236df45b", 256035551,
					"xlanguage_ru.dcp", "b91e24a9d8966c420c261050cb59047e", 10441980,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::RU_RUS, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, February 2016) (Spanish)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "3dd1cf8187c0f46d545b386dc4f6091c", 256035551,
					"data.dcp", "3dd1cf8187c0f46d545b386dc4f6091c", 256035551,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::ES_ESP, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, February 2016) (English)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "3dd1cf8187c0f46d545b386dc4f6091c", 256035551,
					"xlanguage_en.dcp", "33ec450364bf50e3c228ac9edcc38892", 8523503,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::EN_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, February 2016) (Chinese)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "3dd1cf8187c0f46d545b386dc4f6091c", 256035551,
					"xlanguage_nz.dcp", "51bdea5ca4483edbcde520e87df643f6", 8845182,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::ZH_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, February 2016) (Russian)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "3dd1cf8187c0f46d545b386dc4f6091c", 256035551,
					"xlanguage_ru.dcp", "aff303c2f2bfbd1d824398d1b742cb9e", 10441980,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::RU_RUS, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, September 2016) (Italian)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "6e507ba64db28dcd766a5fe41c95e152", 256017952,
					"xlanguage_it.dcp", "633e4350a0e45dbb40b26404f0a546e9", 11484621), Common::IT_ITA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, September 2016) (Italian)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "970f577fea6e5e78abdf84f2ed26f545", 256038111,
					"xlanguage_it.dcp", "d9b30c7af8406bae02b9344e73a87b25", 11484621,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::IT_ITA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, September 2016) (Russian)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "970f577fea6e5e78abdf84f2ed26f545", 256038111,
					"xlanguage_ru.dcp", "177b5a206cffba5e353fd3c691ebb0ce", 10441980,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::RU_RUS, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, September 2016) (Italian)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "970f577fea6e5e78abdf84f2ed26f545", 256038111,
					"xlanguage_it.dcp", "d9b30c7af8406bae02b9344e73a87b25", 11484621,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::IT_ITA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, September 2016) (Russian)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "970f577fea6e5e78abdf84f2ed26f545", 256038111,
					"xlanguage_ru.dcp", "177b5a206cffba5e353fd3c691ebb0ce", 10441980,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::RU_RUS, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, November 2016) (Spanish)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "ff3d33de24036afc5697f901fc7d872d", 256030699,
					"data.dcp", "ff3d33de24036afc5697f901fc7d872d", 256030699), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, November 2016) (German)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "ff3d33de24036afc5697f901fc7d872d", 256030699,
					"xlanguage_de.dcp", "48b1f6185480fb3b64ba5f8996df4bda", 8727399), Common::DE_DEU, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, November 2016) (English)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "ff3d33de24036afc5697f901fc7d872d", 256030699,
					"xlanguage_en.dcp", "3323ede6bb321aa0e3af6727b105e73a", 8535530), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, November 2016) (Italian)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "ff3d33de24036afc5697f901fc7d872d", 256030699,
					"xlanguage_it.dcp", "0e315976351c9508355597a3506d521d", 11498415), Common::IT_ITA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, November 2016) (Chinese)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "ff3d33de24036afc5697f901fc7d872d", 256030699,
					"xlanguage_nz.dcp", "45f52816d5ec5f8e0c1bd70a7aa17f7c", 8847936), Common::ZH_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, November 2016) (Russian)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "ff3d33de24036afc5697f901fc7d872d", 256030699,
					"xlanguage_ru.dcp", "a5925d2b4bda5b1d38cf83f851d42404", 10431458), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, November 2016) (Spanish)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "9cb5465054014620ece739039fe3602b", 256048298,
					"data.dcp", "9cb5465054014620ece739039fe3602b", 256048298,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::ES_ESP, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, November 2016) (English)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "9cb5465054014620ece739039fe3602b", 256048298,
					"xlanguage_en.dcp", "28b1230564debb10fbeed1477b998194", 8535530,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::EN_ANY, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, November 2016) (Italian)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "9cb5465054014620ece739039fe3602b", 256048298,
					"xlanguage_it.dcp", "b2cc0807a94d94befb4350db13659ef6", 11498415,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::IT_ITA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, November 2016) (Chinese)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "9cb5465054014620ece739039fe3602b", 256048298,
					"xlanguage_nz.dcp", "fcceb1300b9819abaee6832b7aef7f90", 10757594,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::ZH_ANY, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, November 2016) (Russian)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "9cb5465054014620ece739039fe3602b", 256048298,
					"xlanguage_ru.dcp", "227b88befc98e5c8c0e25825ac80b6ca", 10454450,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::RU_RUS, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, November 2016) (Spanish)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "9cb5465054014620ece739039fe3602b", 256048298,
					"data.dcp", "9cb5465054014620ece739039fe3602b", 256048298,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::ES_ESP, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, November 2016) (English)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "9cb5465054014620ece739039fe3602b", 256048298,
					"xlanguage_en.dcp", "28b1230564debb10fbeed1477b998194", 8535530,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::EN_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, November 2016) (Italian)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "9cb5465054014620ece739039fe3602b", 256048298,
					"xlanguage_it.dcp", "b2cc0807a94d94befb4350db13659ef6", 11498415,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::IT_ITA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, November 2016) (Chinese)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "9cb5465054014620ece739039fe3602b", 256048298,
					"xlanguage_nz.dcp", "fcceb1300b9819abaee6832b7aef7f90", 10757594,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::ZH_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, November 2016) (Russian)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "9cb5465054014620ece739039fe3602b", 256048298,
					"xlanguage_ru.dcp", "227b88befc98e5c8c0e25825ac80b6ca", 10454450,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::RU_RUS, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, August 2019) (Spanish)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "28256eacb651a82cb07ffd99d0102988", 272211168,
					"data.dcp", "28256eacb651a82cb07ffd99d0102988", 272211168), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, August 2019) (German)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "28256eacb651a82cb07ffd99d0102988", 272211168,
					"xlanguage_de.dcp", "ab90c39ef04fc772c1f78af554fb1944", 8725840), Common::DE_DEU, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, August 2019) (English)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "28256eacb651a82cb07ffd99d0102988", 272211168,
					"xlanguage_en.dcp", "726839c23f4dd0edc4d1cfd6be35f5f3", 8534328), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, August 2019) (French)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "28256eacb651a82cb07ffd99d0102988", 272211168,
					"xlanguage_fr.dcp", "afc4e886814c44fc96a25bba214fd888", 8844755), Common::FR_FRA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, August 2019) (Italian)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "28256eacb651a82cb07ffd99d0102988", 272211168,
					"xlanguage_it.dcp", "afc13347351b3d35f0d73d2340d6f2d9", 11495758), Common::IT_ITA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, August 2019) (Chinese)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "28256eacb651a82cb07ffd99d0102988", 272211168,
					"xlanguage_nz.dcp", "45f52816d5ec5f8e0c1bd70a7aa17f7c", 8847936), Common::ZH_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, August 2019) (Russian)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "28256eacb651a82cb07ffd99d0102988", 272211168,
					"xlanguage_ru.dcp", "a221b4a924d88c75606963fb66bf043a", 10430422), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, August 2019) (Simplified Chinese)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "28256eacb651a82cb07ffd99d0102988", 272211168,
					"xlanguage_zh_s.dcp", "f668735372b0dde4b619c4d9d4678e3c", 8498109), Common::ZH_CHN, ADGF_UNSTABLE, WME_1_9_3),
	
	// Reversion: The Meeting (Steam, August 2019) (Traditional Chinese)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "28256eacb651a82cb07ffd99d0102988", 272211168,
					"xlanguage_zh_t.dcp", "8e6b201c76a48348778a32bf9189c664", 8464532), Common::ZH_TWN, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, August 2019) (Spanish)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "3c34d583c658b75650f0c978d47fd59d", 272228767,
					"data.dcp", "3c34d583c658b75650f0c978d47fd59d", 272228767,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::ES_ESP, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, August 2019) (English)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "3c34d583c658b75650f0c978d47fd59d", 272228767,
					"xlanguage_en.dcp", "fa871d22a10f62b06fc22fc1469baf4e", 8534328,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::EN_ANY, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, August 2019) (French)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "3c34d583c658b75650f0c978d47fd59d", 272228767,
					"xlanguage_fr.dcp", "0e7d28a0fec59b2613ab67673fff9e54", 8844755,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::FR_FRA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, August 2019) (Italian)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "3c34d583c658b75650f0c978d47fd59d", 272228767,
					"xlanguage_it.dcp", "b2cc0807a94d94befb4350db13659ef6", 11498415,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::IT_ITA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, August 2019) (Chinese)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "3c34d583c658b75650f0c978d47fd59d", 272228767,
					"xlanguage_nz.dcp", "fcceb1300b9819abaee6832b7aef7f90", 10757594,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::ZH_ANY, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, August 2019) (Russian)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "3c34d583c658b75650f0c978d47fd59d", 272228767,
					"xlanguage_ru.dcp", "227b88befc98e5c8c0e25825ac80b6ca", 10454450,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::RU_RUS, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, August 2019) (Spanish)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "99ab5a155c60377a7a1e64d9dd8f71aa", 272228767,
					"data.dcp", "99ab5a155c60377a7a1e64d9dd8f71aa", 272228767,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::ES_ESP, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, August 2019) (English)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "99ab5a155c60377a7a1e64d9dd8f71aa", 272228767,
					"xlanguage_en.dcp", "e20fecc233319cb73bd81fc8cf2acb66", 8534328,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::EN_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, August 2019) (French)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "99ab5a155c60377a7a1e64d9dd8f71aa", 272228767,
					"xlanguage_fr.dcp", "b7f68c7a9dc6f75eb995cdda77a89692", 8844755,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::FR_FRA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, August 2019) (Italian)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "99ab5a155c60377a7a1e64d9dd8f71aa", 272228767,
					"xlanguage_it.dcp", "b2cc0807a94d94befb4350db13659ef6", 11498415,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::IT_ITA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, August 2019) (Chinese)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "99ab5a155c60377a7a1e64d9dd8f71aa", 272228767,
					"xlanguage_nz.dcp", "fcceb1300b9819abaee6832b7aef7f90", 10757594,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::ZH_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, August 2019) (Russian)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "99ab5a155c60377a7a1e64d9dd8f71aa", 272228767,
					"xlanguage_ru.dcp", "227b88befc98e5c8c0e25825ac80b6ca", 10454450,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::RU_RUS, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, November 2019) (Spanish)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "5b1f360d29a650ea202a89903412e539", 272211168,
					"data.dcp", "5b1f360d29a650ea202a89903412e539", 272211168), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, November 2019) (German)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "5b1f360d29a650ea202a89903412e539", 272211168,
					"xlanguage_de.dcp", "38281c6be6b0114aff9251fddcf35c39", 8716897), Common::DE_DEU, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, November 2019) (English)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "5b1f360d29a650ea202a89903412e539", 272211168,
					"xlanguage_en.dcp", "dc35e8240090c5909fb77ea5369294ab", 8534328), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, November 2019) (French)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "5b1f360d29a650ea202a89903412e539", 272211168,
					"xlanguage_fr.dcp", "51b3579dad9ec26529aa977a6d92a65a", 8844755), Common::FR_FRA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, November 2019) (Italian)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "5b1f360d29a650ea202a89903412e539", 272211168,
					"xlanguage_it.dcp", "2c2e8d80719bcb5d350d88efa738ea8c", 11495758), Common::IT_ITA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, November 2019) (Chinese)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "5b1f360d29a650ea202a89903412e539", 272211168,
					"xlanguage_nz.dcp", "45f52816d5ec5f8e0c1bd70a7aa17f7c", 8847936), Common::ZH_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, November 2019) (Portuguese)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "5b1f360d29a650ea202a89903412e539", 272211168,
					"xlanguage_pt.dcp", "ee19b2ec802e2028a08b7b2c21a85f09", 8449509), Common::PT_BRA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, November 2019) (Russian)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "5b1f360d29a650ea202a89903412e539", 272211168,
					"xlanguage_ru.dcp", "bbff71b306ae5081ed556a44b3f18a2f", 10430422), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, November 2019) (Simplified Chinese)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "5b1f360d29a650ea202a89903412e539", 272211168,
					"xlanguage_zh_s.dcp", "fa6a6f5742184676cd7d967b518db52c", 8498109), Common::ZH_CHN, ADGF_UNSTABLE, WME_1_9_3),
	
	// Reversion: The Meeting (Steam, November 2019) (Traditional Chinese)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "5b1f360d29a650ea202a89903412e539", 272211168,
					"xlanguage_zh_t.dcp", "32103394d993d9c6121655ee24a743b9", 8464532), Common::ZH_TWN, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, November 2019) (German)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "3c34d583c658b75650f0c978d47fd59d", 272228767,
					"xlanguage_de.dcp", "eb52d971ce0ba4b64663aee4506123ca", 8716897,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::DE_DEU, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, November 2019) (English)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "3c34d583c658b75650f0c978d47fd59d", 272228767,
					"xlanguage_en.dcp", "f41a6e220823ac08643e3731151a666b", 8534328,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::EN_ANY, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, November 2019) (French)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "3c34d583c658b75650f0c978d47fd59d", 272228767,
					"xlanguage_fr.dcp", "beded9d13ef3f805c23091fc87aa4a5d", 8844755,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::FR_FRA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, November 2019) (Italian)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "3c34d583c658b75650f0c978d47fd59d", 272228767,
					"xlanguage_it.dcp", "f3743347c7f6a99a0e0c476146bc778b", 11495758,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::IT_ITA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, November 2019) (Portuguese)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "3c34d583c658b75650f0c978d47fd59d", 272228767,
					"xlanguage_pt.dcp", "ead871dc3d80ecafb7e273411460c92a", 8449509,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::PT_BRA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, November 2019) (Russian)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "3c34d583c658b75650f0c978d47fd59d", 272228767,
					"xlanguage_ru.dcp", "689c596d63af3374d60a5f1b520d8d6c", 10453414,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::RU_RUS, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, November 2019) (Simplified Chinese)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "3c34d583c658b75650f0c978d47fd59d", 272228767,
					"xlanguage_zh_s.dcp", "63f3e7f876252fc36b30995d3c9afdf6", 10407767,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::ZH_CHN, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, November 2019) (Traditional Chinese)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "3c34d583c658b75650f0c978d47fd59d", 272228767,
					"xlanguage_zh_t.dcp", "f61a540bf516b1725ef2ed2b7fbf303a", 10374190,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::ZH_TWN, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, November 2019) (German)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "99ab5a155c60377a7a1e64d9dd8f71aa", 272228767,
					"xlanguage_de.dcp", "c6c8afe3d3f3225727ec84f06ecebe5c", 8716897,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::DE_DEU, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, November 2019) (English)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "99ab5a155c60377a7a1e64d9dd8f71aa", 272228767,
					"xlanguage_en.dcp", "dc0b4f477b64b1d1446550e2aa5c52c5", 8534328,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::EN_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, November 2019) (French)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "99ab5a155c60377a7a1e64d9dd8f71aa", 272228767,
					"xlanguage_fr.dcp", "8299790f346f4a598d5eb283632185b8", 8844755,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::FR_FRA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, November 2019) (Italian)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "99ab5a155c60377a7a1e64d9dd8f71aa", 272228767,
					"xlanguage_it.dcp", "8f84605a6d58cf118a441e64a8fd0992", 11495758,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::IT_ITA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, November 2019) (Portuguese)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "99ab5a155c60377a7a1e64d9dd8f71aa", 272228767,
					"xlanguage_pt.dcp", "b7f07bbf2da06d0ec3d1bc68ff9f5476", 8449509,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::PT_BRA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, November 2019) (Russian)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "99ab5a155c60377a7a1e64d9dd8f71aa", 272228767,
					"xlanguage_ru.dcp", "0cf7e6f52b2f1e368a0d6af6f421bbde", 10453414,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::RU_RUS, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, November 2019) (Simplified Chinese)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "99ab5a155c60377a7a1e64d9dd8f71aa", 272228767,
					"xlanguage_zh_s.dcp", "39aedb26886f354f214ea4f91c919731", 8498109,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::ZH_CHN, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, November 2019) (Traditional Chinese)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "99ab5a155c60377a7a1e64d9dd8f71aa", 272228767,
					"xlanguage_zh_t.dcp", "c3cf601669aee770a40f7a995fe2b7fa", 8464532,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::ZH_TWN, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, March 2020) (English)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "5b1f360d29a650ea202a89903412e539", 272211168,
					"xlanguage_en.dcp", "ea35ab390497427ff308cc696553a8c8", 8534328), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Meeting (Steam, March 2020) (Russian)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "5b1f360d29a650ea202a89903412e539", 272211168,
					"xlanguage_ru.dcp", "5fe33043ce644890ef0cccf5d41681fb", 17511868), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Meeting (Steam, March 2020) (Spanish)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "90d95f3415e1c33ea76de75c329f14ca", 272228827,
					"data.dcp", "90d95f3415e1c33ea76de75c329f14ca", 272228827,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::ES_ESP, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, March 2020) (German)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "90d95f3415e1c33ea76de75c329f14ca", 272228827,
					"xlanguage_de.dcp", "eb52d971ce0ba4b64663aee4506123ca", 8716897,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::DE_DEU, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, March 2020) (English)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "90d95f3415e1c33ea76de75c329f14ca", 272228827,
					"xlanguage_en.dcp", "f41a6e220823ac08643e3731151a666b", 8534328,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::EN_ANY, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, March 2020) (French)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "90d95f3415e1c33ea76de75c329f14ca", 272228827,
					"xlanguage_fr.dcp", "beded9d13ef3f805c23091fc87aa4a5d", 8844755,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::FR_FRA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, March 2020) (Italian)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "90d95f3415e1c33ea76de75c329f14ca", 272228827,
					"xlanguage_it.dcp", "f3743347c7f6a99a0e0c476146bc778b", 11495758,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::IT_ITA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, March 2020) (Portuguese)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "90d95f3415e1c33ea76de75c329f14ca", 272228827,
					"xlanguage_pt.dcp", "a3eae825285e0887bfa014325c11df88", 8449389,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::PT_BRA, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, March 2020) (Russian)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "90d95f3415e1c33ea76de75c329f14ca", 272228827,
					"xlanguage_ru.dcp", "d516386f8dc79106402fd06834ea5520", 17534860,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::RU_RUS, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, March 2020) (Chinese)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "90d95f3415e1c33ea76de75c329f14ca", 272228827,
					"xlanguage_nz.dcp", "fcceb1300b9819abaee6832b7aef7f90", 10757594,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::ZH_ANY, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, March 2020) (Simplified Chinese)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "90d95f3415e1c33ea76de75c329f14ca", 272228827,
					"xlanguage_zh_s.dcp", "63f3e7f876252fc36b30995d3c9afdf6", 10407767,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::ZH_CHN, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, March 2020) (Traditional Chinese)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "90d95f3415e1c33ea76de75c329f14ca", 272228827,
					"xlanguage_zh_t.dcp", "f61a540bf516b1725ef2ed2b7fbf303a", 10374190,
					"Linux.dcp", "16c3a8627216aab5b31c43186e0dfa70", 984535), Common::ZH_TWN, Common::kPlatformLinux, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, March 2020) (Spanish)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "e8140afacd9ae3e2e0b2c2a42a8d4cd3", 272228827,
					"data.dcp", "e8140afacd9ae3e2e0b2c2a42a8d4cd3", 272228827,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::ES_ESP, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, March 2020) (German)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "e8140afacd9ae3e2e0b2c2a42a8d4cd3", 272228827,
					"xlanguage_de.dcp", "c6c8afe3d3f3225727ec84f06ecebe5c", 8716897,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::DE_DEU, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, March 2020) (English)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "e8140afacd9ae3e2e0b2c2a42a8d4cd3", 272228827,
					"xlanguage_en.dcp", "dc0b4f477b64b1d1446550e2aa5c52c5", 8534328,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::EN_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, March 2020) (French)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "e8140afacd9ae3e2e0b2c2a42a8d4cd3", 272228827,
					"xlanguage_fr.dcp", "8299790f346f4a598d5eb283632185b8", 8844755,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::FR_FRA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, March 2020) (Italian)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "e8140afacd9ae3e2e0b2c2a42a8d4cd3", 272228827,
					"xlanguage_it.dcp", "8f84605a6d58cf118a441e64a8fd0992", 11495758,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::IT_ITA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, March 2020) (Portuguese)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "e8140afacd9ae3e2e0b2c2a42a8d4cd3", 272228827,
					"xlanguage_pt.dcp", "364ef02c5a4cbc4eeecdcf84c3a672e0", 8449389,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::PT_BRA, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, March 2020) (Russian)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "e8140afacd9ae3e2e0b2c2a42a8d4cd3", 272228827,
					"xlanguage_ru.dcp", "13d22dbb39b1964fa978e225e04b5f46", 17534860,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::RU_RUS, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, March 2020) (Chinese)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "e8140afacd9ae3e2e0b2c2a42a8d4cd3", 272228827,
					"xlanguage_nz.dcp", "fcceb1300b9819abaee6832b7aef7f90", 10757594,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::ZH_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, March 2020) (Simplified Chinese)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "e8140afacd9ae3e2e0b2c2a42a8d4cd3", 272228827,
					"xlanguage_zh_s.dcp", "39aedb26886f354f214ea4f91c919731", 8498109,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::ZH_CHN, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, March 2020) (Traditional Chinese)
	WME_PLATENTRY("reversion2", "Steam",
		WME_ENTRY3s("data.dcp", "e8140afacd9ae3e2e0b2c2a42a8d4cd3", 272228827,
					"xlanguage_zh_t.dcp", "c3cf601669aee770a40f7a995fe2b7fa", 8464532,
					"Mac.dcp", "0b8d95dcb1f7e8c7c2c49e58db2764b6", 1728476), Common::ZH_TWN, Common::kPlatformMacintosh, ADGF_UNSTABLE, WME_LITE),

	// Reversion: The Meeting (Steam, May 2020) (Spanish)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "a59f2f4fe04478a3a078f8b84651ab27", 272211206,
					"data.dcp", "a59f2f4fe04478a3a078f8b84651ab27", 272211206), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, May 2020) (German)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "a59f2f4fe04478a3a078f8b84651ab27", 272211206,
					"xlanguage_de.dcp", "7a5628acf0fc95596b93120d0adb16d2", 8716897), Common::DE_DEU, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, May 2020) (English)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "a59f2f4fe04478a3a078f8b84651ab27", 272211206,
					"xlanguage_en.dcp", "a9a84556bd629fe28244b8dd3dc79d84", 8534328), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, May 2020) (French)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "a59f2f4fe04478a3a078f8b84651ab27", 272211206,
					"xlanguage_fr.dcp", "05ff17668f416fa4b27caf5157cd0ffe", 8844755), Common::FR_FRA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, May 2020) (Italian)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "a59f2f4fe04478a3a078f8b84651ab27", 272211206,
					"xlanguage_it.dcp", "052f8874eddde7d0a107216a36016e79", 11495758), Common::IT_ITA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, May 2020) (Chinese)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "a59f2f4fe04478a3a078f8b84651ab27", 272211206,
					"xlanguage_nz.dcp", "45f52816d5ec5f8e0c1bd70a7aa17f7c", 8847936), Common::ZH_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, May 2020) (Portuguese)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "a59f2f4fe04478a3a078f8b84651ab27", 272211206,
					"xlanguage_pt.dcp", "7718ef7709044bf85941eec4f2703664", 8449389), Common::PT_BRA, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, May 2020) (Russian)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "a59f2f4fe04478a3a078f8b84651ab27", 272211206,
					"xlanguage_ru.dcp", "212e5c6d93e4ecc57694a25e0c5c10bc", 17512467), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Meeting (Steam, May 2020) (Serbian)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "a59f2f4fe04478a3a078f8b84651ab27", 272211206,
					"xlanguage_sr.dcp", "71f1fc086026bb76137cc9b91c642eff", 8541059), Common::SR_SRB, ADGF_UNSTABLE, WME_1_9_3),
	
	// Reversion: The Meeting (Steam, May 2020) (Simplified Chinese)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "a59f2f4fe04478a3a078f8b84651ab27", 272211206,
					"xlanguage_zh_s.dcp", "7a46d2c1bb6a6ed07583e347d4e13d9c", 8498688), Common::ZH_CHN, ADGF_UNSTABLE, WME_1_9_3),
	
	// Reversion: The Meeting (Steam, May 2020) (Traditional Chinese)
	WME_WINENTRY("reversion2", "Steam",
		WME_ENTRY2s("data.dcp", "a59f2f4fe04478a3a078f8b84651ab27", 272211206,
					"xlanguage_zh_t.dcp", "33f7ed1b38cbb94cfcfff06ce94be1f4", 8465111), Common::ZH_TWN, ADGF_UNSTABLE, WME_1_9_3),

	// Reversion: The Return (Steam, February 2020) (Spanish)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "326b44d1edfe2cba6e4135bb2dec801f", 1806938525,
					"data.dcp", "326b44d1edfe2cba6e4135bb2dec801f", 1806938525), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Return (Steam, February 2020) (German)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "326b44d1edfe2cba6e4135bb2dec801f", 1806938525,
					"xlanguage_de.dcp", "c5d72f4b8ee111fddfe1caee8d97dcea", 10235058), Common::DE_DEU, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Return (Steam, February 2020) (English)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "326b44d1edfe2cba6e4135bb2dec801f", 1806938525,
					"xlanguage_en.dcp", "f1af088d8eda8ea96e09eb2a3183a782", 10021001), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Return (Steam, February 2020) (Portuguese)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "326b44d1edfe2cba6e4135bb2dec801f", 1806938525,
					"xlanguage_pt.dcp", "2b55f9688039912fae450e3f6872c8ce", 10204094), Common::PT_BRA, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Return (Steam, February 2020) (Russian)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "326b44d1edfe2cba6e4135bb2dec801f", 1806938525,
					"xlanguage_ru.dcp", "660627fa165d1d60135b3fc2e09e333b", 19607612), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Return (Steam, February 2020) (Simplified Chinese)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "326b44d1edfe2cba6e4135bb2dec801f", 1806938525,
					"xlanguage_zh_s.dcp", "3c0beb597313226a78e15a38194772c6", 10146103), Common::ZH_CHN, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Return (Steam, April 16th 2020) (Spanish)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "b8c6bb6a0bc56bc814f336468bf4f660", 1806937965,
					"data.dcp", "b8c6bb6a0bc56bc814f336468bf4f660", 1806937965), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Return (Steam, April 16th 2020) (German)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "b8c6bb6a0bc56bc814f336468bf4f660", 1806937965,
					"xlanguage_de.dcp", "fd1465e8b305fd49854eb15d0caaa3bb", 10318916), Common::DE_DEU, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Return (Steam, April 16th 2020) (English)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "b8c6bb6a0bc56bc814f336468bf4f660", 1806937965,
					"xlanguage_en.dcp", "690dbc088ccb0e198df17b9b9def3f67", 10104038), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Return (Steam, April 16th 2020) (Italian)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "b8c6bb6a0bc56bc814f336468bf4f660", 1806937965,
					"xlanguage_it.dcp", "8b48db31e4a5d5ef54d17c4d672d5ad7", 10309029), Common::IT_ITA, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Return (Steam, April 16th 2020) (Portuguese)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "b8c6bb6a0bc56bc814f336468bf4f660", 1806937965,
					"xlanguage_pt.dcp", "566e29be1c74e34263562f609f34e49b", 10204094), Common::PT_BRA, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Return (Steam, April 16th 2020) (Russian)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "b8c6bb6a0bc56bc814f336468bf4f660", 1806937965,
					"xlanguage_ru.dcp", "921555991ce2feee9e83f1d483a001c1", 19607612), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Return (Steam, April 16th 2020) (Simplified Chinese)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "b8c6bb6a0bc56bc814f336468bf4f660", 1806937965,
					"xlanguage_zh_s.dcp", "f104564a8d8b19f6ad83ea32c0e7ac01", 10146103), Common::ZH_CHN, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Return (Steam, April 20th 2020) (Spanish)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "d868bcc82a3c4d7b17f24b8f7cabcc78", 1806937960,
					"data.dcp", "d868bcc82a3c4d7b17f24b8f7cabcc78", 1806937960), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Return (Steam, April 20th 2020) (German)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "d868bcc82a3c4d7b17f24b8f7cabcc78", 1806937960,
					"xlanguage_de.dcp", "f676548444517968d4239c9ccfcb3869", 10318916), Common::DE_DEU, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Return (Steam, April 20th 2020) (English)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "d868bcc82a3c4d7b17f24b8f7cabcc78", 1806937960,
					"xlanguage_en.dcp", "7221156703592abc9f98bbaafbb429fc", 10104038), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Return (Steam, April 20th 2020) (Italian)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "d868bcc82a3c4d7b17f24b8f7cabcc78", 1806937960,
					"xlanguage_it.dcp", "9aec04ab7fdcc4f3d8c616b31f6ab1bf", 10307721), Common::IT_ITA, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Return (Steam, April 20th 2020) (Portuguese)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "d868bcc82a3c4d7b17f24b8f7cabcc78", 1806937960,
					"xlanguage_pt.dcp", "45656ce4b79641d018cc178c3ca8468b", 10204094), Common::PT_BRA, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Return (Steam, April 20th 2020) (Russian)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "d868bcc82a3c4d7b17f24b8f7cabcc78", 1806937960,
					"xlanguage_ru.dcp", "f24c2021a9ad80aa7e03ef95dd930dd9", 19607612), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Return (Steam, April 20th 2020) (Simplified Chinese)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "d868bcc82a3c4d7b17f24b8f7cabcc78", 1806937960,
					"xlanguage_zh_s.dcp", "d059c8c11e39e063a60c602a0127d244", 10146103), Common::ZH_CHN, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Return (Steam, May 2020) (Spanish)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "5df2540652f0b77d8bb48e7454840c5e", 1806937960,
					"data.dcp", "5df2540652f0b77d8bb48e7454840c5e", 1806937960), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Return (Steam, May 2020) (German)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "5df2540652f0b77d8bb48e7454840c5e", 1806937960,
					"xlanguage_de.dcp", "cb3c666a014148529321cf30dc81d6c6", 10318916), Common::DE_DEU, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Return (Steam, May 2020) (English)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "5df2540652f0b77d8bb48e7454840c5e", 1806937960,
					"xlanguage_en.dcp", "de87f56e65ac48010bab0dac8decb7e6", 10107964), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Return (Steam, May 2020) (Italian)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "5df2540652f0b77d8bb48e7454840c5e", 1806937960,
					"xlanguage_it.dcp", "dec219bf9289af361a40efeb0a59ab37", 10307721), Common::IT_ITA, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Return (Steam, May 2020) (Portuguese)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "5df2540652f0b77d8bb48e7454840c5e", 1806937960,
					"xlanguage_pt.dcp", "71df5ba5b0b37b5df60bc114d16f37da", 10204094), Common::PT_BRA, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Return (Steam, May 2020) (Russian)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "5df2540652f0b77d8bb48e7454840c5e", 1806937960,
					"xlanguage_ru.dcp", "3112bec9708620107c1a459e890d1320", 19607612), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Return (Steam, May 2020) (Simplified Chinese)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "5df2540652f0b77d8bb48e7454840c5e", 1806937960,
					"xlanguage_zh_s.dcp", "d82a5708af24a879cbeaf7338ea89ab4", 10233899), Common::ZH_CHN, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Return (Steam, May 2020) (Traditional Chinese)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "5df2540652f0b77d8bb48e7454840c5e", 1806937960,
					"xlanguage_zh_t.dcp", "6ca6a83edad54ec1c384fbcea2989ee4", 10170999), Common::ZH_TWN, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Return (Steam, November 2020) (Serbian)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "5df2540652f0b77d8bb48e7454840c5e", 1806937960,
					"xlanguage_sr.dcp", "dc43cc4e382dfa8b4a336e9fcc723bf3", 10164273), Common::SR_SRB, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Return (Steam, December 2020) (Spanish)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "ca788ebe1b607c24c3e4db4771c16559", 1806937971,
					"data.dcp", "ca788ebe1b607c24c3e4db4771c16559", 1806937971), Common::ES_ESP, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Return (Steam, December 2020) (German)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "ca788ebe1b607c24c3e4db4771c16559", 1806937971,
					"xlanguage_de.dcp", "cb3c666a014148529321cf30dc81d6c6", 10318916), Common::DE_DEU, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Return (Steam, December 2020) (English)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "ca788ebe1b607c24c3e4db4771c16559", 1806937971,
					"xlanguage_en.dcp", "de87f56e65ac48010bab0dac8decb7e6", 10107964), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Return (Steam, December 2020) (Italian)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "ca788ebe1b607c24c3e4db4771c16559", 1806937971,
					"xlanguage_it.dcp", "dec219bf9289af361a40efeb0a59ab37", 10307721), Common::IT_ITA, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Return (Steam, December 2020) (Portuguese)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "ca788ebe1b607c24c3e4db4771c16559", 1806937971,
					"xlanguage_pt.dcp", "71df5ba5b0b37b5df60bc114d16f37da", 10204094), Common::PT_BRA, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Return (Steam, December 2020) (Russian)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "ca788ebe1b607c24c3e4db4771c16559", 1806937971,
					"xlanguage_ru.dcp", "3112bec9708620107c1a459e890d1320", 19607612), Common::RU_RUS, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Return (Steam, December 2020) (Serbian)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "ca788ebe1b607c24c3e4db4771c16559", 1806937971,
					"xlanguage_sr.dcp", "e16438a25527e99fb974dd2cdb5ed945", 10164285), Common::SR_SRB, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Return (Steam, December 2020) (Simplified Chinese)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "ca788ebe1b607c24c3e4db4771c16559", 1806937971,
					"xlanguage_zh_s.dcp", "d82a5708af24a879cbeaf7338ea89ab4", 10233899), Common::ZH_CHN, ADGF_UNSTABLE, WME_1_9_2),

	// Reversion: The Return (Steam, December 2020) (Traditional Chinese)
	WME_WINENTRY("reversion3", "Steam",
		WME_ENTRY2s("data.dcp", "ca788ebe1b607c24c3e4db4771c16559", 1806937971,
					"xlanguage_zh_t.dcp", "6ca6a83edad54ec1c384fbcea2989ee4", 10170999), Common::ZH_TWN, ADGF_UNSTABLE, WME_1_9_2),

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

	// 1 1/2 Ritter: Auf der Suche nach der hinreissenden Herzelinde (German demo)
	WME_WINENTRY("ritter", "Demo",
		WME_ENTRY1s("data.dcp", "7fadb30dbe987528489f50bd85cc0c12", 99975952), Common::DE_DEU, ADGF_UNSTABLE | ADGF_DEMO, WME_1_8_6),

	// Satan and Sons (Demo)
	WME_WINENTRY("satanandsons", "Demo",
		WME_ENTRY1s("data.dcp", "16a6ba8174b697bbba9299619d1e20c4", 67539054), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_3_2),

	// Rosemary
	WME_WINENTRY("rosemary", "",
		WME_ENTRY1s("data.dcp", "4f2631138bd4d27587d9043f8aeff3df", 29483643), Common::EN_ANY, ADGF_UNSTABLE, WME_1_8_6),

	// Royal Mahjong: King's Journey (French)
	WME_WINENTRY("royalmahjong", "",
		WME_ENTRY2s("data.dcp", "f8f8b0308693d07261373766e001bb61", 28856313,
				"fr.dcp", "84fc731590c7ee8c8ae7b8b89d6a8d39", 2560185), Common::FR_FRA, ADGF_UNSTABLE, WME_MAHJONG),

	// Royal Mahjong: King's Journey (German)
	WME_WINENTRY("royalmahjong", "",
		WME_ENTRY2s("data.dcp", "f8f8b0308693d07261373766e001bb61", 28856313,
				"de.dcp", "70f7e6d70787773df11da69e9f5df5e0", 2750615), Common::DE_DEU, ADGF_UNSTABLE, WME_MAHJONG),

	// Royal Mahjong: King's Journey (Russian)
	WME_WINENTRY("royalmahjong", "",
		WME_ENTRY2s("data.dcp", "f8f8b0308693d07261373766e001bb61", 28856313,
				"ru.dcp", "3e6ed74b4571ccada26700e7a0c62696", 978996), Common::RU_RUS, ADGF_UNSTABLE, WME_MAHJONG),

	// Royal Mahjong: King's Journey (English)
	WME_WINENTRY("royalmahjong", "",
		WME_ENTRY1s("data.dcp", "f8f8b0308693d07261373766e001bb61", 28856313), Common::EN_ANY, ADGF_UNSTABLE, WME_MAHJONG),

	// Securanote
	WME_PLATENTRY("securanote", "",
		WME_ENTRY1s("data.dcp", "5213d3e59b9e95b7fbd5c56f7de5341a", 2625554), Common::EN_ANY, Common::kPlatformIOS, ADGF_UNSTABLE, WME_LITE),

	// Shaban (Hayoola) (Persian)
	WME_WINENTRY("shaban", "",
		WME_ENTRY2s("data.dcp", "7d945a8219af9bf306980f526f2a885f", 589685802,
					"persian.dcp", "395b6b13665f1afabaa51a4136c069e4", 12555497), Common::FA_IRN, ADGF_UNSTABLE, WME_1_9_2),

	// Shaban (Hayoola) (English)
	WME_WINENTRY("shaban", "",
		WME_ENTRY2s("data.dcp", "7d945a8219af9bf306980f526f2a885f", 589685802,
					"data.dcp", "7d945a8219af9bf306980f526f2a885f", 589685802), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_2),

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

	// Stroke of Fate: Operation Valkyrie (English)
	WME_WINENTRY("sof1", "",
		WME_ENTRY2s("data.dcp", "6f96e541de77de1999feabb2ec6ac877", 952625188,
					"local.dcp", "d130c371faaf3f6767a7dd230650130d", 201155672), Common::EN_ANY, ADGF_UNSTABLE | GF_3D, WME_1_8_10),

	// Stroke of Fate: Operation Valkyrie (Russian)
	WME_WINENTRY("sof1", "",
		WME_ENTRY2s("data.dcp", "ae4ba7a60da6c706b2f914f8b173295e", 930410566,
					"local.dcp", "3f846bb0673dc92512dea526ce01a4fb", 431994000), Common::RU_RUS, ADGF_UNSTABLE | GF_3D, WME_1_8_10),

	// Stroke of Fate: Operation Valkyrie (Spanish)
	WME_WINENTRY("sof1", "",
		WME_ENTRY2s("data.dcp", "e5032a792e63bb8210f3eab96402614d", 951892593,
					"local.dcp", "2d8af7da80c91ef8bfa594b5f50693ab", 201101595), Common::ES_ESP, ADGF_UNSTABLE | GF_3D, WME_1_8_10),

	// Stroke of Fate: Operation Bunker (English)
	WME_WINENTRY("sof2", "",
		WME_ENTRY2s("data.dcp", "9f4de2a53ef396cc70786a1dc0ab191d", 828351641,
					"local.dcp", "a78f5353c6539b4ff9c39223202ba8d0", 433003083), Common::EN_ANY, ADGF_UNSTABLE | GF_3D, WME_1_8_9),

	// Shadow of Nebula (Steam: Early Access Game, Mar 2016) (English)
	WME_WINENTRY("shadowofnebula", "",
		WME_ENTRY1s("data.dcp", "fd3631791e66885427ccdb6063752a6d", 1402609157), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_3),

	// Shadow of Nebula (Demo) (English)
	WME_WINENTRY("shadowofnebula", "",
		WME_ENTRY1s("data.dcp", "f16f8a354bd90cb233dafc6b1a318f26", 913267667), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Legacy Version) (Steam, Jul 2014) (German)
	WME_WINENTRY("sotv1", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "e19b63c6aa19d491a4f533ac9c1609ef", 97061505,
					"data_sd.dcp", "aee08a5a713c1b45e67471134772f72f", 97833631,
					"i18n_de_strings.dcp", "9cc009980d018476b177e2a4075f56b4", 43104), Common::DE_DEU, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Legacy Version) (Steam, Jul 2014) (English)
	WME_WINENTRY("sotv1", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "e19b63c6aa19d491a4f533ac9c1609ef", 97061505,
					"data_sd.dcp", "aee08a5a713c1b45e67471134772f72f", 97833631,
					"i18n_en_strings.dcp", "40f8ea49a3e8d54a5202aa88c12fba80", 75108), Common::EN_ANY, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Legacy Version) (Steam, Jul 2014) (Spanish)
	WME_WINENTRY("sotv1", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "e19b63c6aa19d491a4f533ac9c1609ef", 97061505,
					"data_sd.dcp", "aee08a5a713c1b45e67471134772f72f", 97833631,
					"i18n_es_strings.dcp", "6a8d5b434dfe758abb2ace1a75a4dab1", 40122), Common::ES_ESP, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Legacy Version) (Steam, Jul 2014) (French)
	WME_WINENTRY("sotv1", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "e19b63c6aa19d491a4f533ac9c1609ef", 97061505,
					"data_sd.dcp", "aee08a5a713c1b45e67471134772f72f", 97833631,
					"i18n_fr_strings.dcp", "b3fe8c720bb6a5378f6da2f593339c70", 40760), Common::FR_FRA, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Legacy Version) (Steam, Jul 2014) (Italian)
	WME_WINENTRY("sotv1", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "e19b63c6aa19d491a4f533ac9c1609ef", 97061505,
					"data_sd.dcp", "aee08a5a713c1b45e67471134772f72f", 97833631,
					"i18n_it_strings.dcp", "5abaab4d57d7585b3c4b23f34f2d0dd8", 40468), Common::IT_ITA, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Legacy Version) (Steam, Feb 12th 2015) (German)
	WME_WINENTRY("sotv1", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "e382f34f0bd319ad310aba2ecd239dec", 95879376,
					"data_sd.dcp", "6842ddf537f2cd186df050aa8c05d363", 97835480,
					"i18n_de_strings.dcp", "780b0d8d4f2b2b32e729090c1018df43", 43136), Common::DE_DEU, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Legacy Version) (Steam, Feb 12th 2015) (English)
	WME_WINENTRY("sotv1", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "e382f34f0bd319ad310aba2ecd239dec", 95879376,
					"data_sd.dcp", "6842ddf537f2cd186df050aa8c05d363", 97835480,
					"i18n_en_strings.dcp", "636e32cf89f02fca30a6f4caa38dede1", 75126), Common::EN_ANY, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Legacy Version) (Steam, Feb 12th 2015) (Spanish)
	WME_WINENTRY("sotv1", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "e382f34f0bd319ad310aba2ecd239dec", 95879376,
					"data_sd.dcp", "6842ddf537f2cd186df050aa8c05d363", 97835480,
					"i18n_es_strings.dcp", "c62f94e9cd543ecbdc0f02acc744cd29", 40153), Common::ES_ESP, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Legacy Version) (Steam, Feb 12th 2015) (French)
	WME_WINENTRY("sotv1", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "e382f34f0bd319ad310aba2ecd239dec", 95879376,
					"data_sd.dcp", "6842ddf537f2cd186df050aa8c05d363", 97835480,
					"i18n_fr_strings.dcp", "548e88d67be123bb54f5b265226f051a", 40807), Common::FR_FRA, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Legacy Version) (Steam, Feb 12th 2015) (Italian)
	WME_WINENTRY("sotv1", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "e382f34f0bd319ad310aba2ecd239dec", 95879376,
					"data_sd.dcp", "6842ddf537f2cd186df050aa8c05d363", 97835480,
					"i18n_it_strings.dcp", "c3d180dd634705cb16ccd650066e1da8", 40502), Common::IT_ITA, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Legacy Version) (Steam, Feb 12th 2015) (Polish)
	WME_WINENTRY("sotv1", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "e382f34f0bd319ad310aba2ecd239dec", 95879376,
					"data_sd.dcp", "6842ddf537f2cd186df050aa8c05d363", 97835480,
					"i18n_pl_strings.dcp", "8dac09efb73ae4a7a2b897de6f37e906", 40062), Common::PL_POL, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Legacy Version) (Steam, Feb 12th 2015) (Russian)
	WME_WINENTRY("sotv1", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "e382f34f0bd319ad310aba2ecd239dec", 95879376,
					"data_sd.dcp", "6842ddf537f2cd186df050aa8c05d363", 97835480,
					"i18n_ru_strings.dcp", "88daa5c022c18535e2da86fa558db792", 45838), Common::RU_RUS, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Legacy Version) (Steam, Feb 22th 2015) (German)
	WME_WINENTRY("sotv1", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "bb6136d76ebadf06c90877283aa1d55a", 95890682,
					"data_sd.dcp", "6135b62bb28434c1af42de84ef8b96fe", 97808411,
					"i18n_de_strings.dcp", "d245d247a282cda33b83ed6918b2e6d5", 43136), Common::DE_DEU, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Legacy Version) (Steam, Feb 22th 2015) (English)
	WME_WINENTRY("sotv1", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "bb6136d76ebadf06c90877283aa1d55a", 95890682,
					"data_sd.dcp", "6135b62bb28434c1af42de84ef8b96fe", 97808411,
					"i18n_en_strings.dcp", "5e9cf5a8403b98f7d92de55efccc0d34", 75119), Common::EN_ANY, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Legacy Version) (Steam, Feb 22th 2015) (Spanish)
	WME_WINENTRY("sotv1", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "bb6136d76ebadf06c90877283aa1d55a", 95890682,
					"data_sd.dcp", "6135b62bb28434c1af42de84ef8b96fe", 97808411,
					"i18n_es_strings.dcp", "edb19a0758243da8929844bac035d384", 40153), Common::ES_ESP, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Legacy Version) (Steam, Feb 22th 2015) (French)
	WME_WINENTRY("sotv1", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "bb6136d76ebadf06c90877283aa1d55a", 95890682,
					"data_sd.dcp", "6135b62bb28434c1af42de84ef8b96fe", 97808411,
					"i18n_fr_strings.dcp", "40dda8156fa93717cf962e75d76929b1", 40807), Common::FR_FRA, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Legacy Version) (Steam, Feb 22th 2015) (Italian)
	WME_WINENTRY("sotv1", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "bb6136d76ebadf06c90877283aa1d55a", 95890682,
					"data_sd.dcp", "6135b62bb28434c1af42de84ef8b96fe", 97808411,
					"i18n_it_strings.dcp", "0d1dac14379e55356794fb7cca8865b2", 40502), Common::IT_ITA, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Legacy Version) (Steam, Feb 22th 2015) (Polish)
	WME_WINENTRY("sotv1", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "bb6136d76ebadf06c90877283aa1d55a", 95890682,
					"data_sd.dcp", "6135b62bb28434c1af42de84ef8b96fe", 97808411,
					"i18n_pl_strings.dcp", "cdd0b203c591dfb411cb6fc89ac009c1", 40062), Common::PL_POL, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Legacy Version) (Steam, Feb 22th 2015) (Russian)
	WME_WINENTRY("sotv1", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "bb6136d76ebadf06c90877283aa1d55a", 95890682,
					"data_sd.dcp", "6135b62bb28434c1af42de84ef8b96fe", 97808411,
					"i18n_ru_strings.dcp", "77fc889bb25438bafe897d1566bd7e50", 45546), Common::RU_RUS, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Legacy Version) (Steam, Sep 2016) (German)
	WME_WINENTRY("sotv1", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "46bb5822abf6d422d08a68070e05bd86", 95890675,
					"data_sd.dcp", "8864e2e552bb7816916d8c6630e8f1a5", 97821007,
					"i18n_de_strings.dcp", "94230807d77dacb420f446c34dd60072", 43136), Common::DE_DEU, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Legacy Version) (Steam, Sep 2016) (English)
	WME_WINENTRY("sotv1", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "46bb5822abf6d422d08a68070e05bd86", 95890675,
					"data_sd.dcp", "8864e2e552bb7816916d8c6630e8f1a5", 97821007,
					"i18n_en_strings.dcp", "253e7f5e2bb4a33c7df52a04624d18c6", 75119), Common::EN_ANY, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Legacy Version) (Steam, Sep 2016) (Spanish)
	WME_WINENTRY("sotv1", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "46bb5822abf6d422d08a68070e05bd86", 95890675,
					"data_sd.dcp", "8864e2e552bb7816916d8c6630e8f1a5", 97821007,
					"i18n_es_strings.dcp", "2227cee67309f564178e48d3eb30fc98", 40153), Common::ES_ESP, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Legacy Version) (Steam, Sep 2016) (French)
	WME_WINENTRY("sotv1", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "46bb5822abf6d422d08a68070e05bd86", 95890675,
					"data_sd.dcp", "8864e2e552bb7816916d8c6630e8f1a5", 97821007,
					"i18n_fr_strings.dcp", "f50d251df43e5433b9664d5c2463fb08", 40807), Common::FR_FRA, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Legacy Version) (Steam, Sep 2016) (Italian)
	WME_WINENTRY("sotv1", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "46bb5822abf6d422d08a68070e05bd86", 95890675,
					"data_sd.dcp", "8864e2e552bb7816916d8c6630e8f1a5", 97821007,
					"i18n_it_strings.dcp", "6ad0d7c0a0c450d6af334cd1b4dbe72e", 40502), Common::IT_ITA, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Legacy Version) (Steam, Sep 2016) (Polish)
	WME_WINENTRY("sotv1", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "46bb5822abf6d422d08a68070e05bd86", 95890675,
					"data_sd.dcp", "8864e2e552bb7816916d8c6630e8f1a5", 97821007,
					"i18n_pl_strings.dcp", "7c07fb021517dd21ba2e2a5739d0168e", 40062), Common::PL_POL, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Legacy Version) (Steam, Sep 2016) (Russian)
	WME_WINENTRY("sotv1", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "46bb5822abf6d422d08a68070e05bd86", 95890675,
					"data_sd.dcp", "8864e2e552bb7816916d8c6630e8f1a5", 97821007,
					"i18n_ru_strings.dcp", "5cecc876ec1b364f22780d18a0821349", 45546), Common::RU_RUS, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Legacy Version) (Steam, Jun 2018) (German)
	WME_WINENTRY("sotv1", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "e0d5b1276cf80c858b7404f1f3381e2b", 95890675,
					"data_sd.dcp", "551feca25b9e0ac9d467c105efb373e8", 97979104,
					"i18n_de_strings.dcp", "59d15bb3c32354b5104475df0ff6c50b", 43136), Common::DE_DEU, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Legacy Version) (Steam, Jun 2018) (English)
	WME_WINENTRY("sotv1", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "e0d5b1276cf80c858b7404f1f3381e2b", 95890675,
					"data_sd.dcp", "551feca25b9e0ac9d467c105efb373e8", 97979104,
					"i18n_en_strings.dcp", "733a53213ab5be27941c34dcef531e0e", 75119), Common::EN_ANY, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Legacy Version) (Steam, Jun 2018) (Spanish)
	WME_WINENTRY("sotv1", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "e0d5b1276cf80c858b7404f1f3381e2b", 95890675,
					"data_sd.dcp", "551feca25b9e0ac9d467c105efb373e8", 97979104,
					"i18n_es_strings.dcp", "18876a252741b8bda888805ca860f600", 40153), Common::ES_ESP, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Legacy Version) (Steam, Jun 2018) (French)
	WME_WINENTRY("sotv1", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "e0d5b1276cf80c858b7404f1f3381e2b", 95890675,
					"data_sd.dcp", "551feca25b9e0ac9d467c105efb373e8", 97979104,
					"i18n_fr_strings.dcp", "850cce9217d83785e4b56028441b0d71", 40807), Common::FR_FRA, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Legacy Version) (Steam, Jun 2018) (Italian)
	WME_WINENTRY("sotv1", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "e0d5b1276cf80c858b7404f1f3381e2b", 95890675,
					"data_sd.dcp", "551feca25b9e0ac9d467c105efb373e8", 97979104,
					"i18n_it_strings.dcp", "de567598fba43589841c354306a80377", 40502), Common::IT_ITA, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Legacy Version) (Steam, Jun 2018) (Polish)
	WME_WINENTRY("sotv1", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "e0d5b1276cf80c858b7404f1f3381e2b", 95890675,
					"data_sd.dcp", "551feca25b9e0ac9d467c105efb373e8", 97979104,
					"i18n_pl_strings.dcp", "daa4b251caec6c6e2c3b96aaf87ae33a", 40062), Common::PL_POL, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Legacy Version) (Steam, Jun 2018) (Russian)
	WME_WINENTRY("sotv1", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "e0d5b1276cf80c858b7404f1f3381e2b", 95890675,
					"data_sd.dcp", "551feca25b9e0ac9d467c105efb373e8", 97979104,
					"i18n_ru_strings.dcp", "ebf28ca1475b3be9754a323966807683", 45546), Common::RU_RUS, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Full HD Version) (Steam, Jul 2014) (German)
	WME_WINENTRY("sotv1", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "e19b63c6aa19d491a4f533ac9c1609ef", 97061505,
					"data_hd.dcp", "d8d903cbda2ff7001cc7ce949775897e", 197016744,
					"i18n_de_strings.dcp", "9cc009980d018476b177e2a4075f56b4", 43104), Common::DE_DEU, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Full HD Version) (Steam, Jul 2014) (English)
	WME_WINENTRY("sotv1", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "e19b63c6aa19d491a4f533ac9c1609ef", 97061505,
					"data_hd.dcp", "d8d903cbda2ff7001cc7ce949775897e", 197016744,
					"i18n_en_strings.dcp", "40f8ea49a3e8d54a5202aa88c12fba80", 75108), Common::EN_ANY, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Full HD Version) (Steam, Jul 2014) (Spanish)
	WME_WINENTRY("sotv1", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "e19b63c6aa19d491a4f533ac9c1609ef", 97061505,
					"data_hd.dcp", "d8d903cbda2ff7001cc7ce949775897e", 197016744,
					"i18n_es_strings.dcp", "6a8d5b434dfe758abb2ace1a75a4dab1", 40122), Common::ES_ESP, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Full HD Version) (Steam, Jul 2014) (French)
	WME_WINENTRY("sotv1", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "e19b63c6aa19d491a4f533ac9c1609ef", 97061505,
					"data_hd.dcp", "d8d903cbda2ff7001cc7ce949775897e", 197016744,
					"i18n_fr_strings.dcp", "b3fe8c720bb6a5378f6da2f593339c70", 40760), Common::FR_FRA, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Full HD Version) (Steam, Jul 2014) (Italian)
	WME_WINENTRY("sotv1", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "e19b63c6aa19d491a4f533ac9c1609ef", 97061505,
					"data_hd.dcp", "d8d903cbda2ff7001cc7ce949775897e", 197016744,
					"i18n_it_strings.dcp", "5abaab4d57d7585b3c4b23f34f2d0dd8", 40468), Common::IT_ITA, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Full HD Version) (Steam, Feb 12th 2015) (German)
	WME_WINENTRY("sotv1", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "e382f34f0bd319ad310aba2ecd239dec", 95879376,
					"data_hd.dcp", "1a077e884c659f34da61dd205591f83d", 197018612,
					"i18n_de_strings.dcp", "780b0d8d4f2b2b32e729090c1018df43", 43136), Common::DE_DEU, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Full HD Version) (Steam, Feb 12th 2015) (English)
	WME_WINENTRY("sotv1", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "e382f34f0bd319ad310aba2ecd239dec", 95879376,
					"data_hd.dcp", "1a077e884c659f34da61dd205591f83d", 197018612,
					"i18n_en_strings.dcp", "636e32cf89f02fca30a6f4caa38dede1", 75126), Common::EN_ANY, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Full HD Version) (Steam, Feb 12th 2015) (Spanish)
	WME_WINENTRY("sotv1", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "e382f34f0bd319ad310aba2ecd239dec", 95879376,
					"data_hd.dcp", "1a077e884c659f34da61dd205591f83d", 197018612,
					"i18n_es_strings.dcp", "c62f94e9cd543ecbdc0f02acc744cd29", 40153), Common::ES_ESP, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Full HD Version) (Steam, Feb 12th 2015) (French)
	WME_WINENTRY("sotv1", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "e382f34f0bd319ad310aba2ecd239dec", 95879376,
					"data_hd.dcp", "1a077e884c659f34da61dd205591f83d", 197018612,
					"i18n_fr_strings.dcp", "548e88d67be123bb54f5b265226f051a", 40807), Common::FR_FRA, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Full HD Version) (Steam, Feb 12th 2015) (Italian)
	WME_WINENTRY("sotv1", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "e382f34f0bd319ad310aba2ecd239dec", 95879376,
					"data_hd.dcp", "1a077e884c659f34da61dd205591f83d", 197018612,
					"i18n_it_strings.dcp", "c3d180dd634705cb16ccd650066e1da8", 40502), Common::IT_ITA, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Full HD Version) (Steam, Feb 12th 2015) (Polish)
	WME_WINENTRY("sotv1", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "e382f34f0bd319ad310aba2ecd239dec", 95879376,
					"data_hd.dcp", "1a077e884c659f34da61dd205591f83d", 197018612,
					"i18n_pl_strings.dcp", "8dac09efb73ae4a7a2b897de6f37e906", 40062), Common::PL_POL, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Full HD Version) (Steam, Feb 12th 2015) (Russian)
	WME_WINENTRY("sotv1", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "e382f34f0bd319ad310aba2ecd239dec", 95879376,
					"data_hd.dcp", "1a077e884c659f34da61dd205591f83d", 197018612,
					"i18n_ru_strings.dcp", "88daa5c022c18535e2da86fa558db792", 45838), Common::RU_RUS, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Full HD Version) (Steam, Feb 22th 2015) (German)
	WME_WINENTRY("sotv1", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "bb6136d76ebadf06c90877283aa1d55a", 95890682,
					"data_hd.dcp", "bfd3bc963c073af866e5405d6d5f1347", 197125364,
					"i18n_de_strings.dcp", "d245d247a282cda33b83ed6918b2e6d5", 43136), Common::DE_DEU, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Full HD Version) (Steam, Feb 22th 2015) (English)
	WME_WINENTRY("sotv1", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "bb6136d76ebadf06c90877283aa1d55a", 95890682,
					"data_hd.dcp", "bfd3bc963c073af866e5405d6d5f1347", 197125364,
					"i18n_en_strings.dcp", "5e9cf5a8403b98f7d92de55efccc0d34", 75119), Common::EN_ANY, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Full HD Version) (Steam, Feb 22th 2015) (Spanish)
	WME_WINENTRY("sotv1", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "bb6136d76ebadf06c90877283aa1d55a", 95890682,
					"data_hd.dcp", "bfd3bc963c073af866e5405d6d5f1347", 197125364,
					"i18n_es_strings.dcp", "edb19a0758243da8929844bac035d384", 40153), Common::ES_ESP, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Full HD Version) (Steam, Feb 22th 2015) (French)
	WME_WINENTRY("sotv1", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "bb6136d76ebadf06c90877283aa1d55a", 95890682,
					"data_hd.dcp", "bfd3bc963c073af866e5405d6d5f1347", 197125364,
					"i18n_fr_strings.dcp", "40dda8156fa93717cf962e75d76929b1", 40807), Common::FR_FRA, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Full HD Version) (Steam, Feb 22th 2015) (Italian)
	WME_WINENTRY("sotv1", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "bb6136d76ebadf06c90877283aa1d55a", 95890682,
					"data_hd.dcp", "bfd3bc963c073af866e5405d6d5f1347", 197125364,
					"i18n_it_strings.dcp", "0d1dac14379e55356794fb7cca8865b2", 40502), Common::IT_ITA, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Full HD Version) (Steam, Feb 22th 2015) (Polish)
	WME_WINENTRY("sotv1", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "bb6136d76ebadf06c90877283aa1d55a", 95890682,
					"data_hd.dcp", "bfd3bc963c073af866e5405d6d5f1347", 197125364,
					"i18n_pl_strings.dcp", "cdd0b203c591dfb411cb6fc89ac009c1", 40062), Common::PL_POL, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Full HD Version) (Steam, Feb 22th 2015) (Russian)
	WME_WINENTRY("sotv1", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "bb6136d76ebadf06c90877283aa1d55a", 95890682,
					"data_hd.dcp", "bfd3bc963c073af866e5405d6d5f1347", 197125364,
					"i18n_ru_strings.dcp", "77fc889bb25438bafe897d1566bd7e50", 45546), Common::RU_RUS, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Full HD Version) (Steam, Sep 2016) (German)
	WME_WINENTRY("sotv1", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "46bb5822abf6d422d08a68070e05bd86", 95890675,
					"data_hd.dcp", "79a5c4ae560817a149506dce894274d0", 197155130,
					"i18n_de_strings.dcp", "94230807d77dacb420f446c34dd60072", 43136), Common::DE_DEU, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Full HD Version) (Steam, Sep 2016) (English)
	WME_WINENTRY("sotv1", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "46bb5822abf6d422d08a68070e05bd86", 95890675,
					"data_hd.dcp", "79a5c4ae560817a149506dce894274d0", 197155130,
					"i18n_en_strings.dcp", "253e7f5e2bb4a33c7df52a04624d18c6", 75119), Common::EN_ANY, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Full HD Version) (Steam, Sep 2016) (Spanish)
	WME_WINENTRY("sotv1", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "46bb5822abf6d422d08a68070e05bd86", 95890675,
					"data_hd.dcp", "79a5c4ae560817a149506dce894274d0", 197155130,
					"i18n_es_strings.dcp", "2227cee67309f564178e48d3eb30fc98", 40153), Common::ES_ESP, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Full HD Version) (Steam, Sep 2016) (French)
	WME_WINENTRY("sotv1", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "46bb5822abf6d422d08a68070e05bd86", 95890675,
					"data_hd.dcp", "79a5c4ae560817a149506dce894274d0", 197155130,
					"i18n_fr_strings.dcp", "f50d251df43e5433b9664d5c2463fb08", 40807), Common::FR_FRA, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Full HD Version) (Steam, Sep 2016) (Italian)
	WME_WINENTRY("sotv1", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "46bb5822abf6d422d08a68070e05bd86", 95890675,
					"data_hd.dcp", "79a5c4ae560817a149506dce894274d0", 197155130,
					"i18n_it_strings.dcp", "6ad0d7c0a0c450d6af334cd1b4dbe72e", 40502), Common::IT_ITA, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Full HD Version) (Steam, Sep 2016) (Polish)
	WME_WINENTRY("sotv1", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "46bb5822abf6d422d08a68070e05bd86", 95890675,
					"data_hd.dcp", "79a5c4ae560817a149506dce894274d0", 197155130,
					"i18n_pl_strings.dcp", "7c07fb021517dd21ba2e2a5739d0168e", 40062), Common::PL_POL, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Full HD Version) (Steam, Sep 2016) (Russian)
	WME_WINENTRY("sotv1", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "46bb5822abf6d422d08a68070e05bd86", 95890675,
					"data_hd.dcp", "79a5c4ae560817a149506dce894274d0", 197155130,
					"i18n_ru_strings.dcp", "5cecc876ec1b364f22780d18a0821349", 45546), Common::RU_RUS, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Full HD Version) (Steam, Jun 2018) (German)
	WME_WINENTRY("sotv1", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "e0d5b1276cf80c858b7404f1f3381e2b", 95890675,
					"data_hd.dcp", "05ce36b4c7b947c503496a1d895671e5", 197431006,
					"i18n_de_strings.dcp", "59d15bb3c32354b5104475df0ff6c50b", 43136), Common::DE_DEU, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Full HD Version) (Steam, Jun 2018) (English)
	WME_WINENTRY("sotv1", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "e0d5b1276cf80c858b7404f1f3381e2b", 95890675,
					"data_hd.dcp", "05ce36b4c7b947c503496a1d895671e5", 197431006,
					"i18n_en_strings.dcp", "733a53213ab5be27941c34dcef531e0e", 75119), Common::EN_ANY, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Full HD Version) (Steam, Jun 2018) (Spanish)
	WME_WINENTRY("sotv1", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "e0d5b1276cf80c858b7404f1f3381e2b", 95890675,
					"data_hd.dcp", "05ce36b4c7b947c503496a1d895671e5", 197431006,
					"i18n_es_strings.dcp", "18876a252741b8bda888805ca860f600", 40153), Common::ES_ESP, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Full HD Version) (Steam, Jun 2018) (French)
	WME_WINENTRY("sotv1", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "e0d5b1276cf80c858b7404f1f3381e2b", 95890675,
					"data_hd.dcp", "05ce36b4c7b947c503496a1d895671e5", 197431006,
					"i18n_fr_strings.dcp", "850cce9217d83785e4b56028441b0d71", 40807), Common::FR_FRA, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Full HD Version) (Steam, Jun 2018) (Italian)
	WME_WINENTRY("sotv1", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "e0d5b1276cf80c858b7404f1f3381e2b", 95890675,
					"data_hd.dcp", "05ce36b4c7b947c503496a1d895671e5", 197431006,
					"i18n_it_strings.dcp", "de567598fba43589841c354306a80377", 40502), Common::IT_ITA, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Full HD Version) (Steam, Jun 2018) (Polish)
	WME_WINENTRY("sotv1", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "e0d5b1276cf80c858b7404f1f3381e2b", 95890675,
					"data_hd.dcp", "05ce36b4c7b947c503496a1d895671e5", 197431006,
					"i18n_pl_strings.dcp", "daa4b251caec6c6e2c3b96aaf87ae33a", 40062), Common::PL_POL, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Full HD Version) (Steam, Jun 2018) (Russian)
	WME_WINENTRY("sotv1", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "e0d5b1276cf80c858b7404f1f3381e2b", 95890675,
					"data_hd.dcp", "05ce36b4c7b947c503496a1d895671e5", 197431006,
					"i18n_ru_strings.dcp", "ebf28ca1475b3be9754a323966807683", 45546), Common::RU_RUS, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act I: Greed (Legacy Version) (NewTech) (Persian)
	// NOTE: this game distribution contained unpacked game files instead of usual game.dcp package
	WME_WINENTRY("sotv1", "Legacy Version",
		WME_ENTRY1s("default.game", "2631e8ba12cdd7ef08c5ee72391ad703", 968), Common::FA_IRN, ADGF_UNSTABLE | GF_3D, WME_1_9_1),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Oct 26th 2015) (English)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "cfea0d6c7e4a96627d16887c3480266a", 273132663,
					"data_sd.dcp", "bd51d2a1e662bd9ed3af7aa1f2180900", 91701364,
					"i18n_en_strings.dcp", "33db4beabfe9813f16133c97198b7520", 114808), Common::EN_ANY, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Oct 26th 2015) (Italian)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "cfea0d6c7e4a96627d16887c3480266a", 273132663,
					"data_sd.dcp", "bd51d2a1e662bd9ed3af7aa1f2180900", 91701364,
					"i18n_it_strings.dcp", "1fefff6f0fb87abe9acf88da9af1a8e7", 127306), Common::IT_ITA, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Oct 26th 2015) (German)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "cfea0d6c7e4a96627d16887c3480266a", 273132663,
					"data_sd.dcp", "bd51d2a1e662bd9ed3af7aa1f2180900", 91701364,
					"i18n_de_strings.dcp", "30a2d7f796b3924af7e77e28f52a18b0", 60239), Common::DE_DEU, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Oct 29th 2015) (German)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "cfea0d6c7e4a96627d16887c3480266a", 273132663,
					"data_sd.dcp", "bd51d2a1e662bd9ed3af7aa1f2180900", 91701364,
					"i18n_de_strings.dcp", "1159db7384da56aae6953d1b943e4c57", 60737), Common::DE_DEU, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Oct 26th 2015) (French)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "cfea0d6c7e4a96627d16887c3480266a", 273132663,
					"data_sd.dcp", "bd51d2a1e662bd9ed3af7aa1f2180900", 91701364,
					"i18n_fr_strings.dcp", "0ce3927e47f9ed8ca6668d0728508abb", 59087), Common::FR_FRA, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Oct 26th 2015) (Russian)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "cfea0d6c7e4a96627d16887c3480266a", 273132663,
					"data_sd.dcp", "bd51d2a1e662bd9ed3af7aa1f2180900", 91701364,
					"i18n_ru_strings.dcp", "20aebb86e857f213fd46d24fba3f6b9c", 73828), Common::RU_RUS, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Oct 26th 2015) (Polish)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "cfea0d6c7e4a96627d16887c3480266a", 273132663,
					"data_sd.dcp", "bd51d2a1e662bd9ed3af7aa1f2180900", 91701364,
					"i18n_pl_strings.dcp", "cc5e50a22672c17211008b6f710e2009", 119066), Common::PL_POL, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Oct 31th 2015) (English)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "e499fac283cf68c6a85638415c4ec083", 273132663,
					"data_sd.dcp", "f9d1bb722eee17696c1c9266d6905924", 91701364,
					"i18n_en_strings.dcp", "fe5b2bb6dd91bbac101f61f388ae8e09", 114808), Common::EN_ANY, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Oct 31th 2015) (Italian)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "e499fac283cf68c6a85638415c4ec083", 273132663,
					"data_sd.dcp", "f9d1bb722eee17696c1c9266d6905924", 91701364,
					"i18n_it_strings.dcp", "cf17089a0c047e7521c4da4a534b0c75", 127245), Common::IT_ITA, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Oct 31th 2015) (German)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "e499fac283cf68c6a85638415c4ec083", 273132663,
					"data_sd.dcp", "f9d1bb722eee17696c1c9266d6905924", 91701364,
					"i18n_de_strings.dcp", "e0285a53d947e6e6925094604d011d3c", 60728), Common::DE_DEU, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Oct 31th 2015) (French)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "e499fac283cf68c6a85638415c4ec083", 273132663,
					"data_sd.dcp", "f9d1bb722eee17696c1c9266d6905924", 91701364,
					"i18n_fr_strings.dcp", "9155180fb4b3a727a5ae58555e77fe0f", 59087), Common::FR_FRA, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Oct 31th 2015) (Russian)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "e499fac283cf68c6a85638415c4ec083", 273132663,
					"data_sd.dcp", "f9d1bb722eee17696c1c9266d6905924", 91701364,
					"i18n_ru_strings.dcp", "decc382f1e1c382e43f7a65f76177cc6", 73757), Common::RU_RUS, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Oct 31th 2015) (Polish)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "e499fac283cf68c6a85638415c4ec083", 273132663,
					"data_sd.dcp", "f9d1bb722eee17696c1c9266d6905924", 91701364,
					"i18n_pl_strings.dcp", "70e7aac260fc6e114b9a52a163276889", 119066), Common::PL_POL, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Nov 2015) (English)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "8c9aa3c3d705f61c753e2f54f3104b8e", 273131920,
					"data_sd.dcp", "570aec5b2ceeea6e3d327b54b183d46b", 91702107,
					"i18n_en_strings.dcp", "b53006a80b7c7c6c40d69ee4ac6eab0e", 114808), Common::EN_ANY, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Nov 2015) (Italian)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "8c9aa3c3d705f61c753e2f54f3104b8e", 273131920,
					"data_sd.dcp", "570aec5b2ceeea6e3d327b54b183d46b", 91702107,
					"i18n_it_strings.dcp", "4c84c9662e758c14a127130e20895f65", 127245), Common::IT_ITA, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Nov 2015) (German)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "8c9aa3c3d705f61c753e2f54f3104b8e", 273131920,
					"data_sd.dcp", "570aec5b2ceeea6e3d327b54b183d46b", 91702107,
					"i18n_de_strings.dcp", "3a454226e403b32ac3b16a15711afe3f", 60728), Common::DE_DEU, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Nov 2015) (French)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "8c9aa3c3d705f61c753e2f54f3104b8e", 273131920,
					"data_sd.dcp", "570aec5b2ceeea6e3d327b54b183d46b", 91702107,
					"i18n_fr_strings.dcp", "b2c65cef26a889663e7311a15796469b", 59087), Common::FR_FRA, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Nov 2015) (Russian)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "8c9aa3c3d705f61c753e2f54f3104b8e", 273131920,
					"data_sd.dcp", "570aec5b2ceeea6e3d327b54b183d46b", 91702107,
					"i18n_ru_strings.dcp", "d9efa7d1d872f81d4d8602d5b4eb4f70", 73757), Common::RU_RUS, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Nov 2015) (Polish)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "8c9aa3c3d705f61c753e2f54f3104b8e", 273131920,
					"data_sd.dcp", "570aec5b2ceeea6e3d327b54b183d46b", 91702107,
					"i18n_pl_strings.dcp", "31da3aa3c184282290f5e418e6412eb3", 119066), Common::PL_POL, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Jan 2016) (English)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "988f6f424110bf0d88b9c8066809df84", 273131920,
					"data_sd.dcp", "30e5c1bd6e98485886c0e8c665510897", 91702107,
					"i18n_en_strings.dcp", "699872d03e5c379299d1cd75894c6ef5", 114808), Common::EN_ANY, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Jan 2016) (Italian)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "988f6f424110bf0d88b9c8066809df84", 273131920,
					"data_sd.dcp", "30e5c1bd6e98485886c0e8c665510897", 91702107,
					"i18n_it_strings.dcp", "fd8a38801ff03401447e9507965841b2", 127245), Common::IT_ITA, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Jan 2016) (German)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "988f6f424110bf0d88b9c8066809df84", 273131920,
					"data_sd.dcp", "30e5c1bd6e98485886c0e8c665510897", 91702107,
					"i18n_de_strings.dcp", "56057bb46e86965e2b5d5ca7823baad5", 60235), Common::DE_DEU, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Jan 2016) (French)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "988f6f424110bf0d88b9c8066809df84", 273131920,
					"data_sd.dcp", "30e5c1bd6e98485886c0e8c665510897", 91702107,
					"i18n_fr_strings.dcp", "29a3a13a8bf787c0811eba67f7c59b41", 59087), Common::FR_FRA, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Jan 2016) (Russian)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "988f6f424110bf0d88b9c8066809df84", 273131920,
					"data_sd.dcp", "30e5c1bd6e98485886c0e8c665510897", 91702107,
					"i18n_ru_strings.dcp", "92ddba944cc23ad12122bf571ac6b856", 73757), Common::RU_RUS, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Jan 2016) (Polish)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "988f6f424110bf0d88b9c8066809df84", 273131920,
					"data_sd.dcp", "30e5c1bd6e98485886c0e8c665510897", 91702107,
					"i18n_pl_strings.dcp", "a3729952b1e24d2d4367dd07a735235b", 119066), Common::PL_POL, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, May 2016) (English)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "208df61df9b95c9d9d0107877eb2f4d7", 273131926,
					"data_sd.dcp", "ff35bbc26334a58b3b4da6d828d69ac2", 94358335,
					"i18n_en_strings.dcp", "6a389c7509da41c4e15b63a7c0530243", 114808), Common::EN_ANY, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, May 2016) (Italian)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "208df61df9b95c9d9d0107877eb2f4d7", 273131926,
					"data_sd.dcp", "ff35bbc26334a58b3b4da6d828d69ac2", 94358335,
					"i18n_it_strings.dcp", "6222689f42ea2948e3d38e537710383f", 127239), Common::IT_ITA, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, May 2016) (German)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "208df61df9b95c9d9d0107877eb2f4d7", 273131926,
					"data_sd.dcp", "ff35bbc26334a58b3b4da6d828d69ac2", 94358335,
					"i18n_de_strings.dcp", "975928849951229f497de36e1a707b61", 60230), Common::DE_DEU, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, May 2016) (Spanish)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "208df61df9b95c9d9d0107877eb2f4d7", 273131926,
					"data_sd.dcp", "ff35bbc26334a58b3b4da6d828d69ac2", 94358335,
					"i18n_es_strings.dcp", "83d88a4b2e5da2a40ac981fcfa9a95bc", 55013), Common::ES_ESP, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, May 2016) (French)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "208df61df9b95c9d9d0107877eb2f4d7", 273131926,
					"data_sd.dcp", "ff35bbc26334a58b3b4da6d828d69ac2", 94358335,
					"i18n_fr_strings.dcp", "6bfc2f1753141e28d22756d9768f4d44", 59086), Common::FR_FRA, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, May 2016) (Russian)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "208df61df9b95c9d9d0107877eb2f4d7", 273131926,
					"data_sd.dcp", "ff35bbc26334a58b3b4da6d828d69ac2", 94358335,
					"i18n_ru_strings.dcp", "e85982376d9fb1c21e1acc8272b49412", 73757), Common::RU_RUS, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, May 2016) (Polish)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "208df61df9b95c9d9d0107877eb2f4d7", 273131926,
					"data_sd.dcp", "ff35bbc26334a58b3b4da6d828d69ac2", 94358335,
					"i18n_pl_strings.dcp", "5ba1e92f1fc762ecec74104f4588ba04", 119066), Common::PL_POL, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Sep 2016) (English)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "73b14ef68eecd41afbe39c2c4f671986", 273131946,
					"data_sd.dcp", "f06a138b758e888f030ce659a42a6e31", 97327255,
					"i18n_en_strings.dcp", "250626c49627e5f3f18883d6eb71e869", 114808), Common::EN_ANY, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Sep 2016) (Italian)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "73b14ef68eecd41afbe39c2c4f671986", 273131946,
					"data_sd.dcp", "f06a138b758e888f030ce659a42a6e31", 97327255,
					"i18n_it_strings.dcp", "338be9aa9c611e349ab7c0a4065a6c78", 127241), Common::IT_ITA, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Sep 2016) (German)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "73b14ef68eecd41afbe39c2c4f671986", 273131946,
					"data_sd.dcp", "f06a138b758e888f030ce659a42a6e31", 97327255,
					"i18n_de_strings.dcp", "46811e68b29fd07e115343e17c53f676", 60429), Common::DE_DEU, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Sep 2016) (Spanish)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "73b14ef68eecd41afbe39c2c4f671986", 273131946,
					"data_sd.dcp", "f06a138b758e888f030ce659a42a6e31", 97327255,
					"i18n_es_strings.dcp", "61e518d05b62a48827ce6fa0a95e8021", 54964), Common::ES_ESP, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Sep 2016) (French)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "73b14ef68eecd41afbe39c2c4f671986", 273131946,
					"data_sd.dcp", "f06a138b758e888f030ce659a42a6e31", 97327255,
					"i18n_fr_strings.dcp", "4605094d334272dd2bc3ba3203b36c48", 59086), Common::FR_FRA, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Sep 2016) (Russian)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "73b14ef68eecd41afbe39c2c4f671986", 273131946,
					"data_sd.dcp", "f06a138b758e888f030ce659a42a6e31", 97327255,
					"i18n_ru_strings.dcp", "8a948c77f185feac630600e3d8f55195", 73757), Common::RU_RUS, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Sep 2016) (Polish)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "73b14ef68eecd41afbe39c2c4f671986", 273131946,
					"data_sd.dcp", "f06a138b758e888f030ce659a42a6e31", 97327255,
					"i18n_pl_strings.dcp", "4a07c25e2d082320880536cc2a835868", 119077), Common::PL_POL, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Oct 2016) (English)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "f04bc0d4fb2034adea3e7f9652b617ec", 273131946,
					"data_sd.dcp", "eedb6e8addffd6cd882f18b231e439a4", 97327255,
					"i18n_en_strings.dcp", "db0db417d6dc8c70de625ee7520d5e40", 114808), Common::EN_ANY, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Oct 2016) (Italian)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "f04bc0d4fb2034adea3e7f9652b617ec", 273131946,
					"data_sd.dcp", "eedb6e8addffd6cd882f18b231e439a4", 97327255,
					"i18n_it_strings.dcp", "809c993d42983fe8664a628804cba98e", 127241), Common::IT_ITA, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Oct 2016) (German)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "f04bc0d4fb2034adea3e7f9652b617ec", 273131946,
					"data_sd.dcp", "eedb6e8addffd6cd882f18b231e439a4", 97327255,
					"i18n_de_strings.dcp", "e6570cb5365d6122e0ac549f83945a9f", 60429), Common::DE_DEU, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Oct 2016) (Spanish)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "f04bc0d4fb2034adea3e7f9652b617ec", 273131946,
					"data_sd.dcp", "eedb6e8addffd6cd882f18b231e439a4", 97327255,
					"i18n_es_strings.dcp", "c06845d464212b022c7eee21cc6502d1", 54964), Common::ES_ESP, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Oct 2016) (French)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "f04bc0d4fb2034adea3e7f9652b617ec", 273131946,
					"data_sd.dcp", "eedb6e8addffd6cd882f18b231e439a4", 97327255,
					"i18n_fr_strings.dcp", "47ff2736401cde872ef454f41a5216ec", 59086), Common::FR_FRA, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Oct 2016) (Russian)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "f04bc0d4fb2034adea3e7f9652b617ec", 273131946,
					"data_sd.dcp", "eedb6e8addffd6cd882f18b231e439a4", 97327255,
					"i18n_ru_strings.dcp", "5ffe06a72c914d287878f2f018109f04", 73757), Common::RU_RUS, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Oct 2016) (Polish)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "f04bc0d4fb2034adea3e7f9652b617ec", 273131946,
					"data_sd.dcp", "eedb6e8addffd6cd882f18b231e439a4", 97327255,
					"i18n_pl_strings.dcp", "43df37eaa12d9a60561297a199ba0e70", 119077), Common::PL_POL, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Jun 2018) (English)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "0410ed71d9d6f133c703009edab38da4", 273131997,
					"data_sd.dcp", "e235c10ece56ac3056ebfa851726dca6", 97169915,
					"i18n_en_strings.dcp", "4b6a631516bd9bd1aa20028b0c7266bd", 114808), Common::EN_ANY, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Jun 2018) (Italian)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "0410ed71d9d6f133c703009edab38da4", 273131997,
					"data_sd.dcp", "e235c10ece56ac3056ebfa851726dca6", 97169915,
					"i18n_it_strings.dcp", "ba71ea60d10a440b6604add1452994f6", 127751), Common::IT_ITA, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Jun 2018) (German)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "0410ed71d9d6f133c703009edab38da4", 273131997,
					"data_sd.dcp", "e235c10ece56ac3056ebfa851726dca6", 97169915,
					"i18n_de_strings.dcp", "999727c3c9bae93f06d19c71337b5d66", 60429), Common::DE_DEU, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Jun 2018) (Spanish)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "0410ed71d9d6f133c703009edab38da4", 273131997,
					"data_sd.dcp", "e235c10ece56ac3056ebfa851726dca6", 97169915,
					"i18n_es_strings.dcp", "fe5b26d90019d9601de6b3fb13daee87", 54964), Common::ES_ESP, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Jun 2018) (French)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "0410ed71d9d6f133c703009edab38da4", 273131997,
					"data_sd.dcp", "e235c10ece56ac3056ebfa851726dca6", 97169915,
					"i18n_fr_strings.dcp", "4f336e2ba1941c006dab3c9ec543db69", 59086), Common::FR_FRA, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Jun 2018) (Russian)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "0410ed71d9d6f133c703009edab38da4", 273131997,
					"data_sd.dcp", "e235c10ece56ac3056ebfa851726dca6", 97169915,
					"i18n_ru_strings.dcp", "f7243c0c9b1a9393fdd97b0c2496ba05", 73757), Common::RU_RUS, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Legacy Version) (Steam, Jun 2018) (Polish)
	WME_WINENTRY("sotv2", "Legacy Version/Steam",
		WME_ENTRY3s("data.dcp", "0410ed71d9d6f133c703009edab38da4", 273131997,
					"data_sd.dcp", "e235c10ece56ac3056ebfa851726dca6", 97169915,
					"i18n_pl_strings.dcp", "67c194c45375d2e26f8bf5ae17800944", 119354), Common::PL_POL, ADGF_UNSTABLE | GF_IGNORE_HD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Oct 26th 2015) (English)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "cfea0d6c7e4a96627d16887c3480266a", 273132663,
					"data_hd.dcp", "a6a3c9dd40902bf6177349f70cc5d215", 259399315,
					"i18n_en_strings.dcp", "33db4beabfe9813f16133c97198b7520", 114808), Common::EN_ANY, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Oct 26th 2015) (Italian)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "cfea0d6c7e4a96627d16887c3480266a", 273132663,
					"data_hd.dcp", "a6a3c9dd40902bf6177349f70cc5d215", 259399315,
					"i18n_it_strings.dcp", "1fefff6f0fb87abe9acf88da9af1a8e7", 127306), Common::IT_ITA, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Oct 26th 2015) (German)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "cfea0d6c7e4a96627d16887c3480266a", 273132663,
					"data_hd.dcp", "a6a3c9dd40902bf6177349f70cc5d215", 259399315,
					"i18n_de_strings.dcp", "30a2d7f796b3924af7e77e28f52a18b0", 60239), Common::DE_DEU, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Oct 29th 2015) (German)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "cfea0d6c7e4a96627d16887c3480266a", 273132663,
					"data_hd.dcp", "a6a3c9dd40902bf6177349f70cc5d215", 259399315,
					"i18n_de_strings.dcp", "1159db7384da56aae6953d1b943e4c57", 60737), Common::DE_DEU, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Oct 26th 2015) (French)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "cfea0d6c7e4a96627d16887c3480266a", 273132663,
					"data_hd.dcp", "a6a3c9dd40902bf6177349f70cc5d215", 259399315,
					"i18n_fr_strings.dcp", "0ce3927e47f9ed8ca6668d0728508abb", 59087), Common::FR_FRA, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Oct 26th 2015) (Russian)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "cfea0d6c7e4a96627d16887c3480266a", 273132663,
					"data_hd.dcp", "a6a3c9dd40902bf6177349f70cc5d215", 259399315,
					"i18n_ru_strings.dcp", "20aebb86e857f213fd46d24fba3f6b9c", 73828), Common::RU_RUS, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Oct 26th 2015) (Polish)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "cfea0d6c7e4a96627d16887c3480266a", 273132663,
					"data_hd.dcp", "a6a3c9dd40902bf6177349f70cc5d215", 259399315,
					"i18n_pl_strings.dcp", "cc5e50a22672c17211008b6f710e2009", 119066), Common::PL_POL, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Oct 31th 2015) (English)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "e499fac283cf68c6a85638415c4ec083", 273132663,
					"data_hd.dcp", "13b45668b605dea1829ca5f276de1339", 259399315,
					"i18n_en_strings.dcp", "fe5b2bb6dd91bbac101f61f388ae8e09", 114808), Common::EN_ANY, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Oct 31th 2015) (Italian)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "e499fac283cf68c6a85638415c4ec083", 273132663,
					"data_hd.dcp", "13b45668b605dea1829ca5f276de1339", 259399315,
					"i18n_it_strings.dcp", "cf17089a0c047e7521c4da4a534b0c75", 127245), Common::IT_ITA, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Oct 31th 2015) (German)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "e499fac283cf68c6a85638415c4ec083", 273132663,
					"data_hd.dcp", "13b45668b605dea1829ca5f276de1339", 259399315,
					"i18n_de_strings.dcp", "e0285a53d947e6e6925094604d011d3c", 60728), Common::DE_DEU, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Oct 31th 2015) (French)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "e499fac283cf68c6a85638415c4ec083", 273132663,
					"data_hd.dcp", "13b45668b605dea1829ca5f276de1339", 259399315,
					"i18n_fr_strings.dcp", "9155180fb4b3a727a5ae58555e77fe0f", 59087), Common::FR_FRA, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Oct 31th 2015) (Russian)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "e499fac283cf68c6a85638415c4ec083", 273132663,
					"data_hd.dcp", "13b45668b605dea1829ca5f276de1339", 259399315,
					"i18n_ru_strings.dcp", "decc382f1e1c382e43f7a65f76177cc6", 73757), Common::RU_RUS, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Oct 31th 2015) (Polish)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "e499fac283cf68c6a85638415c4ec083", 273132663,
					"data_hd.dcp", "13b45668b605dea1829ca5f276de1339", 259399315,
					"i18n_pl_strings.dcp", "70e7aac260fc6e114b9a52a163276889", 119066), Common::PL_POL, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Nov 2015) (English)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "8c9aa3c3d705f61c753e2f54f3104b8e", 273131920,
					"data_hd.dcp", "b4d2ce0a1f5e2e342af7cbf74630300f", 259399911,
					"i18n_en_strings.dcp", "b53006a80b7c7c6c40d69ee4ac6eab0e", 114808), Common::EN_ANY, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Nov 2015) (Italian)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "8c9aa3c3d705f61c753e2f54f3104b8e", 273131920,
					"data_hd.dcp", "b4d2ce0a1f5e2e342af7cbf74630300f", 259399911,
					"i18n_it_strings.dcp", "4c84c9662e758c14a127130e20895f65", 127245), Common::IT_ITA, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Nov 2015) (German)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "8c9aa3c3d705f61c753e2f54f3104b8e", 273131920,
					"data_hd.dcp", "b4d2ce0a1f5e2e342af7cbf74630300f", 259399911,
					"i18n_de_strings.dcp", "3a454226e403b32ac3b16a15711afe3f", 60728), Common::DE_DEU, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Nov 2015) (French)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "8c9aa3c3d705f61c753e2f54f3104b8e", 273131920,
					"data_hd.dcp", "b4d2ce0a1f5e2e342af7cbf74630300f", 259399911,
					"i18n_fr_strings.dcp", "b2c65cef26a889663e7311a15796469b", 59087), Common::FR_FRA, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Nov 2015) (Russian)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "8c9aa3c3d705f61c753e2f54f3104b8e", 273131920,
					"data_hd.dcp", "b4d2ce0a1f5e2e342af7cbf74630300f", 259399911,
					"i18n_ru_strings.dcp", "d9efa7d1d872f81d4d8602d5b4eb4f70", 73757), Common::RU_RUS, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Nov 2015) (Polish)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "8c9aa3c3d705f61c753e2f54f3104b8e", 273131920,
					"data_hd.dcp", "b4d2ce0a1f5e2e342af7cbf74630300f", 259399911,
					"i18n_pl_strings.dcp", "31da3aa3c184282290f5e418e6412eb3", 119066), Common::PL_POL, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Jan 2016) (English)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "988f6f424110bf0d88b9c8066809df84", 273131920,
					"data_hd.dcp", "a5670ea7c401ce187a54223bdc101408", 259399911,
					"i18n_en_strings.dcp", "699872d03e5c379299d1cd75894c6ef5", 114808), Common::EN_ANY, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Jan 2016) (Italian)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "988f6f424110bf0d88b9c8066809df84", 273131920,
					"data_hd.dcp", "a5670ea7c401ce187a54223bdc101408", 259399911,
					"i18n_it_strings.dcp", "fd8a38801ff03401447e9507965841b2", 127245), Common::IT_ITA, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Jan 2016) (German)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "988f6f424110bf0d88b9c8066809df84", 273131920,
					"data_hd.dcp", "a5670ea7c401ce187a54223bdc101408", 259399911,
					"i18n_de_strings.dcp", "56057bb46e86965e2b5d5ca7823baad5", 60235), Common::DE_DEU, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Jan 2016) (French)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "988f6f424110bf0d88b9c8066809df84", 273131920,
					"data_hd.dcp", "a5670ea7c401ce187a54223bdc101408", 259399911,
					"i18n_fr_strings.dcp", "29a3a13a8bf787c0811eba67f7c59b41", 59087), Common::FR_FRA, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Jan 2016) (Russian)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "988f6f424110bf0d88b9c8066809df84", 273131920,
					"data_hd.dcp", "a5670ea7c401ce187a54223bdc101408", 259399911,
					"i18n_ru_strings.dcp", "92ddba944cc23ad12122bf571ac6b856", 73757), Common::RU_RUS, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Jan 2016) (Polish)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "988f6f424110bf0d88b9c8066809df84", 273131920,
					"data_hd.dcp", "a5670ea7c401ce187a54223bdc101408", 259399911,
					"i18n_pl_strings.dcp", "a3729952b1e24d2d4367dd07a735235b", 119066), Common::PL_POL, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, May 2016) (English)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "208df61df9b95c9d9d0107877eb2f4d7", 273131926,
					"data_hd.dcp", "79a959116026a085435d1cc95bfa9570", 265023033,
					"i18n_en_strings.dcp", "6a389c7509da41c4e15b63a7c0530243", 114808), Common::EN_ANY, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, May 2016) (Italian)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "208df61df9b95c9d9d0107877eb2f4d7", 273131926,
					"data_hd.dcp", "79a959116026a085435d1cc95bfa9570", 265023033,
					"i18n_it_strings.dcp", "6222689f42ea2948e3d38e537710383f", 127239), Common::IT_ITA, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, May 2016) (German)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "208df61df9b95c9d9d0107877eb2f4d7", 273131926,
					"data_hd.dcp", "79a959116026a085435d1cc95bfa9570", 265023033,
					"i18n_de_strings.dcp", "975928849951229f497de36e1a707b61", 60230), Common::DE_DEU, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, May 2016) (Spanish)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "208df61df9b95c9d9d0107877eb2f4d7", 273131926,
					"data_hd.dcp", "79a959116026a085435d1cc95bfa9570", 265023033,
					"i18n_es_strings.dcp", "83d88a4b2e5da2a40ac981fcfa9a95bc", 55013), Common::ES_ESP, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, May 2016) (French)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "208df61df9b95c9d9d0107877eb2f4d7", 273131926,
					"data_hd.dcp", "79a959116026a085435d1cc95bfa9570", 265023033,
					"i18n_fr_strings.dcp", "6bfc2f1753141e28d22756d9768f4d44", 59086), Common::FR_FRA, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, May 2016) (Russian)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "208df61df9b95c9d9d0107877eb2f4d7", 273131926,
					"data_hd.dcp", "79a959116026a085435d1cc95bfa9570", 265023033,
					"i18n_ru_strings.dcp", "e85982376d9fb1c21e1acc8272b49412", 73757), Common::RU_RUS, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, May 2016) (Polish)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "208df61df9b95c9d9d0107877eb2f4d7", 273131926,
					"data_hd.dcp", "79a959116026a085435d1cc95bfa9570", 265023033,
					"i18n_pl_strings.dcp", "5ba1e92f1fc762ecec74104f4588ba04", 119066), Common::PL_POL, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Sep 2016) (English)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "73b14ef68eecd41afbe39c2c4f671986", 273131946,
					"data_hd.dcp", "d7d7fbdbf0ec41eddf92078d38be8de5", 271652871,
					"i18n_en_strings.dcp", "250626c49627e5f3f18883d6eb71e869", 114808), Common::EN_ANY, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Sep 2016) (Italian)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "73b14ef68eecd41afbe39c2c4f671986", 273131946,
					"data_hd.dcp", "d7d7fbdbf0ec41eddf92078d38be8de5", 271652871,
					"i18n_it_strings.dcp", "338be9aa9c611e349ab7c0a4065a6c78", 127241), Common::IT_ITA, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Sep 2016) (German)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "73b14ef68eecd41afbe39c2c4f671986", 273131946,
					"data_hd.dcp", "d7d7fbdbf0ec41eddf92078d38be8de5", 271652871,
					"i18n_de_strings.dcp", "46811e68b29fd07e115343e17c53f676", 60429), Common::DE_DEU, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Sep 2016) (Spanish)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "73b14ef68eecd41afbe39c2c4f671986", 273131946,
					"data_hd.dcp", "d7d7fbdbf0ec41eddf92078d38be8de5", 271652871,
					"i18n_es_strings.dcp", "61e518d05b62a48827ce6fa0a95e8021", 54964), Common::ES_ESP, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Sep 2016) (French)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "73b14ef68eecd41afbe39c2c4f671986", 273131946,
					"data_hd.dcp", "d7d7fbdbf0ec41eddf92078d38be8de5", 271652871,
					"i18n_fr_strings.dcp", "4605094d334272dd2bc3ba3203b36c48", 59086), Common::FR_FRA, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Sep 2016) (Russian)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "73b14ef68eecd41afbe39c2c4f671986", 273131946,
					"data_hd.dcp", "d7d7fbdbf0ec41eddf92078d38be8de5", 271652871,
					"i18n_ru_strings.dcp", "8a948c77f185feac630600e3d8f55195", 73757), Common::RU_RUS, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Sep 2016) (Polish)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "73b14ef68eecd41afbe39c2c4f671986", 273131946,
					"data_hd.dcp", "d7d7fbdbf0ec41eddf92078d38be8de5", 271652871,
					"i18n_pl_strings.dcp", "4a07c25e2d082320880536cc2a835868", 119077), Common::PL_POL, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Oct 2016) (English)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "f04bc0d4fb2034adea3e7f9652b617ec", 273131946,
					"data_hd.dcp", "5e4f57e77da13286bfa22ab157fcf147", 271652871,
					"i18n_en_strings.dcp", "db0db417d6dc8c70de625ee7520d5e40", 114808), Common::EN_ANY, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Oct 2016) (Italian)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "f04bc0d4fb2034adea3e7f9652b617ec", 273131946,
					"data_hd.dcp", "5e4f57e77da13286bfa22ab157fcf147", 271652871,
					"i18n_it_strings.dcp", "809c993d42983fe8664a628804cba98e", 127241), Common::IT_ITA, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Oct 2016) (German)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "f04bc0d4fb2034adea3e7f9652b617ec", 273131946,
					"data_hd.dcp", "5e4f57e77da13286bfa22ab157fcf147", 271652871,
					"i18n_de_strings.dcp", "e6570cb5365d6122e0ac549f83945a9f", 60429), Common::DE_DEU, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Oct 2016) (Spanish)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "f04bc0d4fb2034adea3e7f9652b617ec", 273131946,
					"data_hd.dcp", "5e4f57e77da13286bfa22ab157fcf147", 271652871,
					"i18n_es_strings.dcp", "c06845d464212b022c7eee21cc6502d1", 54964), Common::ES_ESP, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Oct 2016) (French)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "f04bc0d4fb2034adea3e7f9652b617ec", 273131946,
					"data_hd.dcp", "5e4f57e77da13286bfa22ab157fcf147", 271652871,
					"i18n_fr_strings.dcp", "47ff2736401cde872ef454f41a5216ec", 59086), Common::FR_FRA, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Oct 2016) (Russian)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "f04bc0d4fb2034adea3e7f9652b617ec", 273131946,
					"data_hd.dcp", "5e4f57e77da13286bfa22ab157fcf147", 271652871,
					"i18n_ru_strings.dcp", "5ffe06a72c914d287878f2f018109f04", 73757), Common::RU_RUS, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Oct 2016) (Polish)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "f04bc0d4fb2034adea3e7f9652b617ec", 273131946,
					"data_hd.dcp", "5e4f57e77da13286bfa22ab157fcf147", 271652871,
					"i18n_pl_strings.dcp", "43df37eaa12d9a60561297a199ba0e70", 119077), Common::PL_POL, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Jun 2018) (English)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "0410ed71d9d6f133c703009edab38da4", 273131997,
					"data_hd.dcp", "61b79bd9f732e48bb097227ee615463b", 272405838,
					"i18n_en_strings.dcp", "4b6a631516bd9bd1aa20028b0c7266bd", 114808), Common::EN_ANY, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Jun 2018) (Italian)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "0410ed71d9d6f133c703009edab38da4", 273131997,
					"data_hd.dcp", "61b79bd9f732e48bb097227ee615463b", 272405838,
					"i18n_it_strings.dcp", "ba71ea60d10a440b6604add1452994f6", 127751), Common::IT_ITA, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Jun 2018) (German)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "0410ed71d9d6f133c703009edab38da4", 273131997,
					"data_hd.dcp", "61b79bd9f732e48bb097227ee615463b", 272405838,
					"i18n_de_strings.dcp", "999727c3c9bae93f06d19c71337b5d66", 60429), Common::DE_DEU, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Jun 2018) (Spanish)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "0410ed71d9d6f133c703009edab38da4", 273131997,
					"data_hd.dcp", "61b79bd9f732e48bb097227ee615463b", 272405838,
					"i18n_es_strings.dcp", "fe5b26d90019d9601de6b3fb13daee87", 54964), Common::ES_ESP, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Jun 2018) (French)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "0410ed71d9d6f133c703009edab38da4", 273131997,
					"data_hd.dcp", "61b79bd9f732e48bb097227ee615463b", 272405838,
					"i18n_fr_strings.dcp", "4f336e2ba1941c006dab3c9ec543db69", 59086), Common::FR_FRA, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Jun 2018) (Russian)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "0410ed71d9d6f133c703009edab38da4", 273131997,
					"data_hd.dcp", "61b79bd9f732e48bb097227ee615463b", 272405838,
					"i18n_ru_strings.dcp", "f7243c0c9b1a9393fdd97b0c2496ba05", 73757), Common::RU_RUS, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shadows on the Vatican - Act II: Wrath (Full HD Version) (Steam, Jun 2018) (Polish)
	WME_WINENTRY("sotv2", "Full HD Version/Steam",
		WME_ENTRY3s("data.dcp", "0410ed71d9d6f133c703009edab38da4", 273131997,
					"data_hd.dcp", "61b79bd9f732e48bb097227ee615463b", 272405838,
					"i18n_pl_strings.dcp", "67c194c45375d2e26f8bf5ae17800944", 119354), Common::PL_POL, ADGF_UNSTABLE | GF_IGNORE_SD_FILES | GF_3D, WME_1_9_3),

	// Shelter (Demo)
	WME_WINENTRY("shelter", "Demo",
		WME_ENTRY1s("data.dcp", "789f15a26fc06a814cfca723e33fc5f7", 5417715), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_6_2),

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

	// Strange Change
	WME_WINENTRY("strangechange", "",
		WME_ENTRY1s("data.dcp", "818e53c1584dac28f336195d7dcfb97d", 3762512), Common::EN_ANY, ADGF_UNSTABLE | GF_3D, WME_1_8_6),

	// Sunny (Demo)
	WME_WINENTRY("sunny", "Demo",
		WME_ENTRY1s("data.dcp", "685f4db26ed8134aa82bf5b6bae3963e", 4745194), Common::EN_ANY, ADGF_UNSUPPORTED | ADGF_DEMO, WME_1_0_31),

	// Sunrise: The game (German)
	WME_WINENTRY("sunrise", "",
		WME_ENTRY1s("data.dcp", "d06744fc60982a30e82d96075698c8bc", 134409552), Common::DE_DEU, ADGF_UNSTABLE | GF_3D, WME_1_7_93),

	// Sunrise: The game (Demo) (German)
	WME_WINENTRY("sunrise", "Demo",
		WME_ENTRY1s("data.dcp", "3bac4da87472d2e7676e9548970d521a", 52095549), Common::DE_DEU, ADGF_UNSTABLE | ADGF_DEMO | GF_3D, WME_1_7_93),

	// Susan Rose: Mysterious Child
	WME_WINENTRY("susanrose1", "",
		WME_ENTRY1s("data.dcp", "bc607fe93d6751de2d0d0b7f4b2c90f8", 798965082), Common::RU_RUS, ADGF_UNSTABLE | GF_3D, WME_1_8_9),

	// Susan Rose: Delicate Murder
	WME_WINENTRY("susanrose2", "",
		WME_ENTRY1s("data.dcp", "789660631a79ed76d95446d1f1e6e694", 922032035), Common::RU_RUS, ADGF_UNSTABLE | GF_3D, WME_1_8_11),

	// Tanya Grotter and the Magical Double Bass
	WME_WINENTRY("tanya1", "",
		WME_ENTRY1s("data.dcp", "035bbdaff078cc4053ecf4b518c0d0fd", 1007507786), Common::RU_RUS, ADGF_UNSTABLE, WME_1_8_0),

	// Tanya Grotter and the Disappearing Floor
	WME_WINENTRY("tanya2", "",
		WME_ENTRY1s("data.dcp", "9c15f14990f630177e063da885d03e6d", 936959767), Common::RU_RUS, ADGF_UNSTABLE, WME_1_8_3),

	// Murder In Tehran's Alleys 1933 (Retail) (Persian)
	WME_WINENTRY("tehran1933", "",
		WME_ENTRY1s("data.dcp", "0ab314e014b5e9d45411fee436d98dfd", 1921736252), Common::FA_IRN, ADGF_UNSTABLE | GF_3D, WME_1_9_1),

	// Murder In Tehran's Alleys 1933 (Hayoola) (Persian)
	WME_WINENTRY("tehran1933", "",
		WME_ENTRY1s("data.dcp", "acc00770c734c8c5bb25141f6d3bd82d", 872649217), Common::FA_IRN, ADGF_UNSTABLE | GF_3D, WME_1_9_1),

	// Murder In Tehran's Alleys 1933 (Steam, June 2017) (English)
	WME_WINENTRY("tehran1933", "",
		WME_ENTRY1s("data.dcp", "7d74999f8a926bce367b2f851a06bc1c", 890506879), Common::EN_ANY, ADGF_UNSTABLE | GF_3D, WME_1_9_3),

	// Murder In Tehran's Alleys 2016 (Hayoola) (Persian)
	WME_WINENTRY("tehran2016", "",
		WME_ENTRY1s("data.dcp", "24c54a6f4dc9ff0498972ae4572df2ee", 1156014223), Common::FA_IRN, ADGF_UNSTABLE | GF_3D, WME_1_9_1),

	// Murder In Tehran's Alleys 2016 (Steam, June 2017) (English)
	WME_WINENTRY("tehran2016", "",
		WME_ENTRY1s("data.dcp", "952346c4d7a08986688b3cc583215d33", 1163612836), Common::EN_ANY, ADGF_UNSTABLE | GF_3D, WME_1_9_3),

	// The Ancient Mark - Episode 1
	WME_WINENTRY("theancientmark1", "",
		WME_ENTRY1s("data.dcp", "ca04c26f03b2bd307368b306b297ddd7", 364664692), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// TeTRIks
	WME_WINENTRY("tetriks", "",
		WME_ENTRY1s("data.dcp", "94040eb5ae132ac2ef1d40888934f9fa", 2289693), Common::CS_CZE, ADGF_UNSTABLE, WME_1_1_38),

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

	// The Last Crown - Midnight Horror (Steam, Dec 2015) (English)
	// NOTE: Same packages were reuploaded to Steam at October 2017
	WME_WINENTRY("thelastcrownmh", "",
		WME_ENTRY1s("stageplay.dcp", "eaf6c27f45cbb786306bf9dce0db7f94", 920651607), Common::EN_ANY, ADGF_UNSTABLE | GF_3D, WME_1_9_3),

	// The Last Crown - Midnight Horror (Steam, Oct 2017) (German)
	WME_WINENTRY("thelastcrownmh", "",
		WME_ENTRY1s("stageplay.dcp", "563dd3383cb91b0f988fd9650815830f", 871649803), Common::DE_DEU, ADGF_UNSTABLE | GF_3D, WME_1_9_3),

	// The Lost Crown - A Ghost Hunting Adventure (2CD version) (English)
	WME_WINENTRY("thelostcrowngha", "",
		WME_ENTRY1s("theatre.dcp", "d0ee83038af66a6a4bb7c513e9550cbb", 77989556), Common::EN_ANY, ADGF_UNSTABLE | GF_3D, WME_1_8_2),

	// The Lost Crown - A Ghost Hunting Adventure (DVD version) (English)
	WME_WINENTRY("thelostcrowngha", "",
		WME_ENTRY1s("theatre.dcp", "741867f67bfb639ac0c96f6990822893", 78078952), Common::EN_ANY, ADGF_UNSTABLE | GF_3D, WME_1_8_2),

	// The Lost Crown - A Ghost Hunting Adventure (DVD version) (German)
	WME_WINENTRY("thelostcrowngha", "",
		WME_ENTRY1s("theatre.dcp", "3deed61c6f6f02e7422b639c52b9169a", 78455706), Common::DE_DEU, ADGF_UNSTABLE | GF_3D, WME_1_8_2),

	// The Lost Crown - A Ghost Hunting Adventure (DVD version) (Russian/Akella)
	WME_WINENTRY("thelostcrowngha", "",
		WME_ENTRY1s("theatre.dcp", "01ab6ced306f11e0d0c7d1dfbc7a2658", 78352318), Common::RU_RUS, ADGF_UNSTABLE | GF_3D, WME_1_8_2),

	// The Lost Crown - A Ghost Hunting Adventure (Italian)
	WME_WINENTRY("thelostcrowngha", "",
		WME_ENTRY1s("theatre.dcp", "4ecf7175f1d7dd6524ff3c0e2cba0a28", 78444724), Common::IT_ITA, ADGF_UNSTABLE | GF_3D, WME_1_8_2),

	// The Lost Crown - A Ghost Hunting Adventure (Steam, Jul 2014) (English)
	WME_WINENTRY("thelostcrowngha", "Steam",
		WME_ENTRY1s("theatre.dcp", "25e005501162a96743ae3d3d33e7bbc3", 88472760), Common::EN_ANY, ADGF_UNSTABLE | GF_3D, WME_1_9_3),

	// The Lost Crown - A Ghost Hunting Adventure (Steam, Jun 2014) (German)
	WME_WINENTRY("thelostcrowngha", "Steam",
		WME_ENTRY1s("theatre.dcp", "46cf3cfa4c4a1007b94c00d779bb24bd", 89163239), Common::DE_DEU, ADGF_UNSTABLE | GF_3D, WME_1_9_3),

	// The Lost Crown - A Ghost Hunting Adventure (Steam, Jul 2014) (German)
	WME_WINENTRY("thelostcrowngha", "Steam",
		WME_ENTRY1s("theatre.dcp", "21e943958e69c3f80803c649b6290140", 89163394), Common::DE_DEU, ADGF_UNSTABLE | GF_3D, WME_1_9_3),

	// Fairy Tales About Toshechka and Boshechka
	WME_WINENTRY("tib", "",
		WME_ENTRY1s("data.dcp", "87d296ef3f46570ed18f000d3885db77", 340264526), Common::RU_RUS, ADGF_UNSTABLE, WME_1_7_2),

	// The Trader of Stories (Demo)
	WME_WINENTRY("tradestory", "Demo",
		WME_ENTRY1s("data.dcp", "0a0b51191636cc8ead89b905281c3218", 40401902), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_0),

	// The White Chamber (Version 1.7 - Definitive Edition) (built 2008.06.26) (multi-language)
	WME_WINENTRY("twc", "Definitive Edition",
		WME_ENTRY1s("data.dcp", "325abfaeb5fbfcc30d91296f1390a454", 186451273), Common::UNK_LANG, ADGF_UNSTABLE, WME_1_8_6),

	// The White Chamber (Version 1.7 - Definitive Edition) (built 2008.06.29) (multi-language)
	WME_WINENTRY("twc", "Definitive Edition",
		WME_ENTRY1s("data.dcp", "0011d01142547c61e51ba24dc42b579e", 186451273), Common::UNK_LANG, ADGF_UNSTABLE, WME_1_8_6),

	// Today, Mama!
	WME_WINENTRY("todaymama", "",
		WME_ENTRY1s("data.dcp", "b89e4a2000109f21d1aba10ef0920180", 1266324389), Common::RU_RUS, ADGF_UNSTABLE | GF_3D, WME_1_7_3),

	// Vsevolod Prologue (Demo)
	WME_WINENTRY("vsevolod", "Prologue",
		WME_ENTRY1s("data.dcp", "f2dcffd2692dbfcc9371fa1a87970fe7", 388669493), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, WME_1_9_1),

	// War
	WME_WINENTRY("war", "",
		WME_ENTRY1s("data.dcp", "003e317cda6d0137bbd5e5d7f089ee4d", 32591890), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_1),

	// The Way Of Love: Sub Zero (Retail) (Persian)
	WME_WINENTRY("wayoflove", "",
		WME_ENTRY2s("data.dcp", "e86fcca7d5044998945a6e1e7a874430", 1650426981,
					"language.dcp", "91b100d10719bcfc355739a41f4eca4b", 662313), Common::FA_IRN, ADGF_UNSTABLE | GF_3D, WME_1_9_3),

	// The Way Of Love: Sub Zero (English)
	WME_WINENTRY("wayoflove", "",
		WME_ENTRY2s("data.dcp", "201d74cce6b89ce6d5b61a24b5e88809", 1224687223,
					"language.dcp", "97ae6c028be982ca419416fab77efc76", 663912), Common::EN_ANY, ADGF_UNSTABLE | GF_3D, WME_1_9_3),

	// Wintermute Engine Technology Demo (1.2)
	WME_WINENTRY("wmedemo", "1.2",
		WME_ENTRY1s("data.dcp", "511f447de05211a4dc979fafce74c046", 6031195), Common::EN_ANY, ADGF_UNSTABLE, WME_1_7_0),

	// Wilma Tetris
	WME_WINENTRY("wtetris", "",
		WME_ENTRY1s("data.dcp", "0898412c1d7c5b759089b2408a9591fe", 2779976), Common::EN_ANY, ADGF_UNSTABLE, WME_1_1_39),

	// Wilma Tetris
	WME_WINENTRY("wtetris", "",
		WME_ENTRY1s("data.dcp", "946e3a0496e6c12fb344c9ed861ff015", 2780093), Common::EN_ANY, ADGF_UNSTABLE, WME_1_1_39),

	// WME 3D characters technology demo
	WME_WINENTRY("wmedemo3d", "Demo",
		WME_ENTRY1s("data.dcp", "9ca18ca52f873ca8fbc78bf408e0a68d", 3224894), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO | GF_3D, WME_1_8_0),

	// Wilma Tetris
	WME_WINENTRY("wtetris", "",
		WME_ENTRY1s("wtetris.exe", "fb89ec8c4fa2279120a8939449c0bda9", 3918669), Common::EN_ANY, ADGF_UNSTABLE, WME_1_1_39),

	// Zilm: A Game of Reflex
	WME_WINENTRY("zilm", "",
		WME_ENTRY1s("data.dcp", "81cece0c8105b4725fc35064a32b4b52", 351726), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Zilm: A Game of Reflex 1.0
	WME_WINENTRY("zilm", "1.0",
		WME_ENTRY1s("data.dcp", "098dffaf03d8adbb4cb5633e4733e63c", 351726), Common::EN_ANY, ADGF_UNSTABLE, WME_1_9_3),

	// Zbang! The Game (Demo)
	WME_WINENTRY("zbang", "0.89",
		WME_ENTRY1s("data.dcp", "db9101f08d12ab95c81042d154bb0ea8", 7210044), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO | GF_3D, WME_1_7_1),
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
