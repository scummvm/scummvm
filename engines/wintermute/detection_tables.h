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
	{"agustin",			"Boredom of Agustin Cordes"},
	{"bickadoodle",     "Bickadoodle"},
	{"bookofgron",      "Book of Gron Part One"},
	{"bthreshold",      "Beyond the Threshold"},
	{"carolreed4",      "Carol Reed 4 - East Side Story"},
	{"carolreed5",      "Carol Reed 5 - The Colour of Murder"},
	{"carolreed6",      "Carol Reed 6 - Black Circle"},
	{"carolreed7",      "Carol Reed 7 - Blue Madonna"},
	{"carolreed8",      "Carol Reed 8 - Amber's Blood"},
	{"carolreed9",      "Carol Reed 9 - Cold Case Summer"},
	{"chivalry",        "Chivalry is Not Dead"},
	{"conspiracao",     "Conspiracao Dumont"},
	{"corrosion",       "Corrosion: Cold Winter Waiting"},
	{"deadcity",        "Dead City"},
	{"dfafadventure",        "DFAF Adventure"},
	{"dreamcat",        "Dreamcat"},
	{"dreaming",        "Des Reves Elastiques Avec Mille Insectes Nommes Georges"},
	{"dirtysplit",      "Dirty Split"},
	{"dreamscape",      "Dreamscape"},
	{"escapemansion",   "Escape from the Mansion"},
	{"four",            "Four"},
	{"framed",          "Framed"},
	{"ghostsheet",      "Ghost in the Sheet"},
	{"hamlet",          "Hamlet or the last game without MMORPS features, shaders and product placement"},
	{"helga",           "Helga Deep In Trouble"},
	{"jamesperis",      "James Peris: No License Nor Control"},
	{"kulivocko",       "Kulivocko"},
	{"lifein3minutes",  "Life In 3 Minutes"},
	{"lonelyrobot",     "Project Lonely Robot"},
	{"looky",           "Looky"},
	{"julia",           "J.U.L.I.A."},
	{"mirage",          "Mirage"},
	{"nighttrain",      "Night Train"},
	{"oknytt",          "Oknytt"},
	{"openquest",        "Open Quest"},
	{"paintaria",       "Paintaria"},
	{"pigeons",         "Pigeons in the Park"},
	{"projectdoom",     "Project: Doom"},
	{"projectjoe",      "Project Joe"},
	{"reversion1",      "Reversion: The Escape"},
	{"reversion2",      "Reversion: The Meeting"},
	{"rhiannon",        "Rhiannon: Curse of the four Branches"},
	{"ritter",          "1 1/2 Ritter: Auf der Suche nach der hinreissenden Herzelinde"},
	{"rosemary",        "Rosemary"},
	{"satanandson",     "Satan and Son"},
	{"securanote",      "Securanote"},
	{"shaban",          "Shaban"},
	{"shinestar",       "The Shine of a Star"},
	{"spaceinvaders",   "Space Invaders"},
	{"spacemadness",    "Space Madness"},
	{"sofiasdebt",      "Sofia's Debt"},
	{"theancientmark1", "The Ancient Mark - Episode 1"},
	{"thebox",          "The Box"},
	{"thekite",         "The Kite"},
	{"tib",             "Fairy Tales About Toshechka and Boshechka"},
	{"tradestory",      "The Trader of Stories"},
	{"twc",             "the white chamber"},
	{"war",             "War"},
	{"vsevolod",        "Vsevolod"},
	{"wintermute",      "Wintermute engine game"},
	{"wtetris",         "Wilma Tetris"},
	{"zilm",            "Zilm: A Game of Reflex"},
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
	// Five Lethal Demons
	WME_WINENTRY("5ld", "",
		WME_ENTRY1s("data.dcp", "1037a77cbd001e0644898addc022322c", 15407750), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Five Magical Amulets (Czech)
	WME_WINENTRY("5ma", "",
		WME_ENTRY2s("czech.dcp", "7b2515a8ceb955c72bc14f0f1fca869e", 184,
					"data.dcp", "0134e92bcd5fd2837df3971087e96067", 163316498), Common::CZ_CZE, ADGF_UNSTABLE, WME_1_7_0),
	// Five Magical Amulets (English)
	WME_WINENTRY("5ma", "",
		WME_ENTRY2s("english.dcp", "2f97bca09260ba23b645da9f0855ce7f", 893681,
					"data.dcp", "0134e92bcd5fd2837df3971087e96067", 163316498), Common::EN_ANY, ADGF_UNSTABLE, WME_1_7_0),
	// Five Magical Amulets (German)
	WME_WINENTRY("5ma", "",
		WME_ENTRY2s("german.dcp", "bfa74aae81672803d0d0748ac0a532b7", 885150,
					"data.dcp", "0134e92bcd5fd2837df3971087e96067", 163316498), Common::DE_DEU, ADGF_UNSTABLE, WME_1_7_0),
	// Five Magical Amulets (Polish)
	WME_WINENTRY("5ma", "",
		WME_ENTRY2s("polish.dcp", "bb877d48795471a17f25b0b5109100d1", 1132197,
					"data.dcp", "0134e92bcd5fd2837df3971087e96067", 163316498), Common::PL_POL, ADGF_UNSTABLE, WME_1_7_0),
	// Actual Destination
	WME_WINENTRY("actualdest", "",
		WME_ENTRY1s("data.dcp", "6926f44b26f21ceb1d840eaab9aeb510", 9081740), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Boredom of Agustin Cordes
	WME_WINENTRY("agustin", "",
		WME_ENTRY1s("data.dcp", "abb79c16c9b92e9b06525a4c7c3f5861", 2461949), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Beyond the Threshold
	WME_WINENTRY("bthreshold", "",
		WME_ENTRY1s("data.dcp", "d49bf9ccb2e74507447c82d6ad3e2bc4", 12773712), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Bickadoodle
	WME_WINENTRY("bickadoodle", "",
		WME_ENTRY1s("data.dcp", "84db4d1594cac95e25614985775d10a8", 35303844), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Bickadoodle (Ver 1.1)
	WME_WINENTRY("bickadoodle", "Version 1.1",
		WME_ENTRY1s("data.dcp", "8bb52ac9a9ee129c5059e8e808b669d7", 35337760), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Bickadoodle (Ver 1.2)
	WME_WINENTRY("bickadoodle", "Version 1.2",
		WME_ENTRY1s("data.dcp", "1796a48f3ed72dd785ce93334ab883cc", 35337760), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Bickadoodle (download from http://aethericgames.com/games/bickadoodle/download-bickadoodle/)
	WME_WINENTRY("bickadoodle", "",
		WME_ENTRY1s("data.dcp", "1584d83577c32add0fce27fae91141a2", 35337728), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Book of Gron Part One
	WME_WINENTRY("bookofgron", "",
		WME_ENTRY1s("data.dcp", "e61b2ebee044a82fa0f8ca0fce2c8946", 83129531), Common::RU_RUS, ADGF_UNSTABLE, LATEST_VERSION),
	// Carol Reed 4 - East Side Story (Demo)
	WME_WINENTRY("carolreed4", "Demo",
		WME_ENTRY1s("data.dcp", "b3f8b09bb4b05ee3e9d14697525257f9", 59296246), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, LATEST_VERSION),
	// Carol Reed 4 - East Side Story
	WME_WINENTRY("carolreed4", "",
		WME_ENTRY1s("data.dcp", "b26377797f060afc2d440d820100c1ce", 529320536), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Carol Reed 5 - The Colour of Murder
	WME_WINENTRY("carolreed5", "",
		WME_ENTRY1s("data.dcp", "3fcfca44209545d0e26774156427b494", 603660415), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Carol Reed 5 - The Colour of Murder (1.0 Demo)
	WME_WINENTRY("carolreed5", "Demo",
		WME_ENTRY1s("data.dcp", "27b3efc018ade5ee8f4adf08b4e3c0dd", 92019500), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, LATEST_VERSION),
	// Carol Reed 6 - Black Circle
	WME_WINENTRY("carolreed6", "",
		WME_ENTRY1s("data.dcp", "0e4c532beecf23d85012168753f41189", 456258147), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Carol Reed 7 - Blue Madonna (Demo)
	WME_WINENTRY("carolreed7", "Demo",
		WME_ENTRY1s("data.dcp", "0372ad0c775266f6355e9e8ae397a2f1", 103719442), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, LATEST_VERSION),
	// Carol Reed 7 - Blue Madonna
	WME_WINENTRY("carolreed7", "",
		WME_ENTRY1s("data.dcp", "24e3db3e2fabfc956713796d87a3efb0", 495471147), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Carol Reed 8 - Amber's Blood
	WME_WINENTRY("carolreed8", "",
		WME_ENTRY1s("data.dcp", "859d16b0d5b9b255e470cbded2c6cedc", 502714557), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Carol Reed 9 - Cold Case Summer
	WME_WINENTRY("carolreed9", "",
		WME_ENTRY1s("data.dcp", "2b343b48a7aee508d728a546b414a255", 620005266), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Chivalry is Not Dead
	WME_WINENTRY("chivalry", "",
		WME_ENTRY1s("data.dcp", "ebd0915d9a12df5224be22f53bb23eb6", 7278306), Common::EN_ANY, ADGF_NO_FLAGS, LATEST_VERSION),
	// Chivalry is Not Dead (Version from deirdrakai.com)
	WME_WINENTRY("chivalry", "",
		WME_ENTRY1s("data.dcp", "ae6d91b9517f4d2851a8ad94c96951c8", 7278302), Common::EN_ANY, ADGF_NO_FLAGS, LATEST_VERSION),
	// Conspiracao Dumont
	WME_WINENTRY("conspiracao", "",
		WME_ENTRY1s("ConspiracaoDumont.exe", "106f3f2c8f18bb5ffffeed634ace256c", 32908032), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, LATEST_VERSION),
	// Corrosion: Cold Winter Waiting
	WME_WINENTRY("corrosion", "",
		WME_ENTRY1s("data.dcp", "ae885b1a8faa0b27f43c0e8f0df02fc9", 525931618), Common::EN_ANY, ADGF_NO_FLAGS, LATEST_VERSION),
	// Dead City (Czech)
	// The Czech data are in data.dcp, so in this case we'll have to
	// just detect the english version twice, to give the user a choice.
	WME_WINENTRY("deadcity", "",
		WME_ENTRY2s("english.dcp", "c591046d6de7e381d76f70e0787b2b1f", 415935,
					"data.dcp", "7ebfd50d1a22370ed7b079bcaa631d62", 9070205), Common::CZ_CZE, ADGF_UNSTABLE, LATEST_VERSION),
	// Dead City (English)
	WME_WINENTRY("deadcity", "",
		WME_ENTRY2s("english.dcp", "c591046d6de7e381d76f70e0787b2b1f", 415935,
					"data.dcp", "7ebfd50d1a22370ed7b079bcaa631d62", 9070205), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Dead City (Italian)
	WME_WINENTRY("deadcity", "",
		WME_ENTRY2s("italian.dcp", "92d8efb94436bec7bd1b7fe0b548192e", 454037,
					"data.dcp", "7ebfd50d1a22370ed7b079bcaa631d62", 9070205), Common::IT_ITA, ADGF_UNSTABLE, LATEST_VERSION),
	// Dead City (Russian)
	WME_WINENTRY("deadcity", "",
		WME_ENTRY2s("russian.dcp", "a0ae71e9e1185596fffb07ad2c951eb9", 653317,
					"data.dcp", "7ebfd50d1a22370ed7b079bcaa631d62", 9070205), Common::RU_RUS, ADGF_UNSTABLE, LATEST_VERSION),
	// DFAF Adventure
	WME_WINENTRY("dfafadventure", "",
		WME_ENTRY1s("data.dcp","5704ebef961176f647742aa66bd09352", 10083417), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Dirty Split (Czech)
	WME_WINENTRY("dirtysplit", "",
		WME_ENTRY2s("czech.dcp", "08a71446467cf8f9444cfea446b46ad6", 127697934,
					"data.dcp", "8b4b81b718bf65f30a67fc0b1e329eb5", 88577623), Common::CZ_CZE, ADGF_UNSTABLE, LATEST_VERSION),
	// Dirty Split (English)
	WME_WINENTRY("dirtysplit", "",
		WME_ENTRY1s("data.dcp", "8f3dae199361ece0f59fb20cfff6eed3", 88577621), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Dirty Split (French)
	WME_WINENTRY("dirtysplit", "",
		WME_ENTRY2s("french.dcp", "a0508dedebd0fe478d0158fa4c2a1136", 125534323,
					"data.dcp", "e6d70c7f5d181b761cfcf974adf9186a", 88577623), Common::FR_FRA, ADGF_UNSTABLE, LATEST_VERSION),
	// Dirty Split (German)
	WME_WINENTRY("dirtysplit", "",
		WME_ENTRY1s("data.dcp", "139d8a25579e969f8b37d20e6e3de5f9", 92668291), Common::DE_DEU, ADGF_UNSTABLE, LATEST_VERSION),
	// Dirty Split (Italian)
	WME_WINENTRY("dirtysplit", "",
		WME_ENTRY2s("italian.dcp", "8108807fbd8af70be1ec452d0fd1131b", 125513726,
					"data.dcp", "35a150e22af274185883fdbb142c6fb1", 88577623), Common::IT_ITA, ADGF_UNSTABLE, LATEST_VERSION),
	// Dirty Split (Spanish)
	WME_WINENTRY("dirtysplit", "",
		WME_ENTRY2s("spanish.dcp", "b3982c0a5e85b42e1e38240fef004aa4", 164428596,
					"data.dcp", "63766d6c68b9f00b632ea1736fc8a95c", 88577621), Common::ES_ESP, ADGF_UNSTABLE, LATEST_VERSION),
	// Des Reves Elastiques Avec Mille Insectes Nommes Georges
	WME_WINENTRY("dreaming", "",
		WME_ENTRY1s("data.dcp", "4af26d97ea063fc1277ce30ae431de90", 8804073), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Dreamcat
	WME_WINENTRY("dreamcat", "",
		WME_ENTRY1s("data.dcp","189bd4eef29034f4ff4ed30120eaac4e", 7758040), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Dreamscape
	WME_WINENTRY("dreamscape", "",
		WME_ENTRY1s("data.dcp",  "7a5752ed4446c862be9f02d7932acf54", 17034377), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Escape from the Mansion
	WME_WINENTRY("escapemansion", "Beta 1",
		WME_ENTRY1s("data.dcp",  "d8e348b2312cc36a929cad75f12e0b3a", 21452380), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Escape from the Mansion
	WME_WINENTRY("escapemansion", "Beta 2",
		WME_ENTRY1s("data.dcp",  "ded5fa6c5f2afdaf2cafb53e52cd3dd8", 21455763), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Escape from the Mansion
	WME_WINENTRY("escapemansion", "1.3",
		WME_ENTRY1s("data.dcp",  "1e5d231b56c8a228cd15cb690f50253e", 29261972), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Four
	WME_WINENTRY("four", "",
		WME_ENTRY1s("data.dcp", "ec05cd5e37c9a524053b8859635a4234", 62599855), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Framed
	WME_WINENTRY("framed", "",
		WME_ENTRY1s("data.dcp",  "e7259fb36f2c6f9f28242291e0c3de98", 34690568), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Ghost in the Sheet
	WME_WINENTRY("ghostsheet", "",
		WME_ENTRY2s("english.dcp", "e6d0aad2c89996bcabe416105a3d6d3a", 12221017,
					"data.dcp", "b2f8b05328e4881e15e98e845b63f451", 168003), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Ghost in the Sheet (Demo)
	WME_WINENTRY("ghostsheet", "Demo",
		WME_ENTRY1s("data.dcp", "dc1f6595f412ac25a52eaf47dad4ab81", 169083), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, LATEST_VERSION),
	// Hamlet or the last game without MMORPS features, shaders and product placement
	WME_WINENTRY("hamlet", "",

		WME_ENTRY1s("data.dcp", "f624add957a77c9930529fb28cc2450f", 88183022), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Helga Deep In Trouble (English)
	WME_WINENTRY("helga", "",
		WME_ENTRY2s("english.dcp", "bfa136b21bdbc7d8691c0770a6d40bc3", 135931,
					"data.dcp", "25cb955a60b58326f2eeda1ce288fb37", 183251259), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Helga Deep In Trouble (Demo) (English)
	WME_WINENTRY("helga", "Demo",
		WME_ENTRY2s("english.dcp", "b3a93e678f0ef97200f691cd1724643f", 135864,
					"data.dcp", "45134ed93bc391edf148b79cdcbf2a09", 154266028), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, LATEST_VERSION),
	// James Peris: No License Nor Control (English)
	WME_WINENTRY("jamesperis", "",
		WME_ENTRY1s("data.dcp", "a420961e170cb7d168a0d2bae2fe5218", 225294032), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// James Peris: No License Nor Control (Spanish)
	WME_WINENTRY("jamesperis", "",
		WME_ENTRY1s("data.dcp", "a420961e170cb7d168a0d2bae2fe5218", 225294032), Common::ES_ESP, ADGF_UNSTABLE, LATEST_VERSION),
	// James Peris: No License Nor Control (Demo) (English)
	WME_WINENTRY("jamesperis", "Demo",
		WME_ENTRY1s("data.dcp", "edb9f9c7a08993c1e28f4e477b5f9830", 116113507), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, LATEST_VERSION),
	// James Peris: No License Nor Control (Demo) (Spanish)
	WME_WINENTRY("jamesperis", "Demo",
		WME_ENTRY1s("data.dcp", "edb9f9c7a08993c1e28f4e477b5f9830", 116113507), Common::ES_ESP, ADGF_UNSTABLE | ADGF_DEMO, LATEST_VERSION),
	// J.U.L.I.A. (English)
	WME_WINENTRY("julia", "",
		WME_ENTRY1s("data.dcp", "c2264b4f8fcd132d2913ff5b6076a24f", 10109741), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// J.U.L.I.A. (English, Bundle in a box-version)
	WME_WINENTRY("julia", "Version 1.2",
		WME_ENTRY1s("data.dcp", "fe90023ccc22f35185b40b910e0d03a2", 10101373), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// J.U.L.I.A. (English) (Demo)
	WME_WINENTRY("julia", "Demo",
		WME_ENTRY1s("data.dcp", "f0bbc3394555a9811f6050dae428cab6", 7655237), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, LATEST_VERSION),
	// J.U.L.I.A. (English) (Greenlight Demo)
	WME_WINENTRY("julia", "Greenlight Demo",
		WME_ENTRY1s("data.dcp", "4befd448d36b0dae9c3ab1aa7cb8b78d", 7271886), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, LATEST_VERSION),
	// Kulivocko (Czech)
	WME_WINENTRY("kulivocko", "",
		WME_ENTRY1s("data.dcp", "44306dc470e9b27474043932eccee02f", 155106392), Common::CZ_CZE, ADGF_UNSTABLE, LATEST_VERSION),
	// Kulivocko (Czech) (Demo)
	WME_WINENTRY("kulivocko", "Demo",
		WME_ENTRY1s("data.dcp", "63b164bdfadecbb0deb5da691afb8154", 48362234), Common::CZ_CZE, ADGF_UNSTABLE | ADGF_DEMO, LATEST_VERSION),
	// Life In 3 Minutes
	WME_WINENTRY("lifein3minutes", "",
		WME_ENTRY1s("data.dcp", "c6368950e37a95bf098b02b4eaa5b929", 141787214), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Looky Demo (English)
	WME_WINENTRY("looky", "Demo",
		WME_ENTRY2s("english.dcp", "1388e1dd320f4d553dea3b0316812f9d", 1358442,
					"data.dcp", "7074bcd7bc7ad7eb04c271aafb964c32", 13815660), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, LATEST_VERSION),
	// Looky Demo (German)
	WME_WINENTRY("looky", "Demo",
		WME_ENTRY2s("german.dcp", "606c048426dfbe94442b59fd34a5c76e", 14339496,
					"data.dcp", "7074bcd7bc7ad7eb04c271aafb964c32", 13815660), Common::DE_DEU, ADGF_UNSTABLE | ADGF_DEMO, LATEST_VERSION),
	// Looky (German)
	WME_WINENTRY("looky", "",
		WME_ENTRY2s("german.dcp", "bf4c2b8c26342342441a6d64934ab832", 107027865,
					"data.dcp", "50de0beaa5ad621aa9f020df901d1e74", 1342214), Common::DE_DEU, ADGF_UNSTABLE, LATEST_VERSION),
	// Mirage
	WME_WINENTRY("mirage", "",
		WME_ENTRY1s("data.dcp", "d230b0b99c0aa77b9ecd094d8ee5573b", 17844056), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Oknytt
	WME_WINENTRY("oknytt", "Version 1.0",
		WME_ENTRY1s("data.dcp", "6456cf8f429905c83f07509f9da536dd", 109502959), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Oknytt (Version 1.13 English) (These are detected along with d_sounds.dcp to avoid mass-detecting in the languages-subfolder.)
	WME_WINENTRY("oknytt", "Version 1.13",
		WME_ENTRY2s("english.dcp", "d2afd722c78cfe66b7d4250d11f6ae16", 293274135,
					"d_sounds.dcp", "7d04dff8ca11174486bd4b7a80fdcabb", 154943401), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Oknytt (Version 1.13 German)
	WME_WINENTRY("oknytt", "Version 1.13",
		WME_ENTRY2s("german.dcp", "0fc6401d8d76b04f6da49206ecafa0dc", 304292574,
					"d_sounds.dcp", "7d04dff8ca11174486bd4b7a80fdcabb", 154943401), Common::DE_DEU, ADGF_UNSTABLE, LATEST_VERSION),
	// Oknytt (Version 1.13 Russian)
	WME_WINENTRY("oknytt", "Version 1.13",
		WME_ENTRY2s("russian.dcp", "dd24a1c0b36a82e2b02fb6c1050d4aad", 362681669,
					"d_sounds.dcp", "7d04dff8ca11174486bd4b7a80fdcabb", 154943401), Common::RU_RUS, ADGF_UNSTABLE, LATEST_VERSION),
	// Oknytt (Version 1.13 Spanish)
	WME_WINENTRY("oknytt", "Version 1.13",
		WME_ENTRY2s("spanish.dcp", "10c46152cb29581671f3b6b7c229c957", 319406572,
					"d_sounds.dcp", "7d04dff8ca11174486bd4b7a80fdcabb", 154943401), Common::ES_ESP, ADGF_UNSTABLE, LATEST_VERSION),
	// Open Quest
	WME_WINENTRY("openquest", "",
		WME_ENTRY1s("data.dcp", "16893e3fc15a211a49654ae66f684f28", 82281736), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, LATEST_VERSION),	
	// Night Train Demo
	WME_WINENTRY("nighttrain", "",
		WME_ENTRY1s("data.dcp", "5a027ef84b083a730c9a4c85ec1d3a32", 131760816), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, LATEST_VERSION),
	// Paintaria
	WME_WINENTRY("paintaria", "",
		WME_ENTRY1s("data.dcp", "354c08440c98150ff0d4008dd2865880", 48326040), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Pigeons in the Park
	WME_WINENTRY("pigeons", "",
		WME_ENTRY1s("data.dcp", "9143a5b6ff8206aefe3c4c643add3ec7", 2611100), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Project: Doom
	WME_WINENTRY("projectdoom", "",
		WME_ENTRY1s("data.dcp", "d5894b65a40706845434b99870bcab92", 99223761), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Project Joe
	WME_WINENTRY("projectjoe", "",
		WME_ENTRY1s("data.dcp", "ada3c08542901295076b5349e655e73f", 160780037), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, LATEST_VERSION),
	// Project Lonely Robot
	WME_WINENTRY("lonelyrobot", "beta",
		WME_ENTRY1s("data.dcp", "a0cf7ad5bab957416dcda454e9f28ef0", 3420120), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, LATEST_VERSION),
	// Reversion: The Escape Version 1.0
	WME_WINENTRY("reversion1", "Version 1.0",
		WME_ENTRY1s("data.dcp", "cd616f98ebfd047e0c540b50b4b70761", 254384531), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Reversion: The Escape Version 1.1 (Chinese)
	WME_WINENTRY("reversion1", "Version 1.1",
		WME_ENTRY2s("chinese.dcp", "cf97150739499a4c15f51dc534ff85a1", 6330561,
					"data.dcp", "cb9865dc7e1db2990a8cf4bc13cf4999", 257643032), Common::ZH_CNA, ADGF_UNSTABLE, LATEST_VERSION),
	// Reversion: The Escape Version 1.1 (English)
	WME_WINENTRY("reversion1", "Version 1.1",
		WME_ENTRY2s("english.dcp", "7b2f061d7c91365c5d04605f1de032b3", 5702699,
					"data.dcp", "cb9865dc7e1db2990a8cf4bc13cf4999", 257643032), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Reversion: The Escape Version 1.1 (French)
	WME_WINENTRY("reversion1", "Version 1.1",
		WME_ENTRY2s("french.dcp", "214204b6022c5ed67fada44557690faf", 6327400,
					"data.dcp", "cb9865dc7e1db2990a8cf4bc13cf4999", 257643032), Common::FR_FRA, ADGF_UNSTABLE, LATEST_VERSION),
	// Reversion: The Escape Version 1.1 (German)
	WME_WINENTRY("reversion1", "Version 1.1",
		WME_ENTRY2s("german.dcp", "96677823b36d580a4a29e3659071071c", 6340699,
					"data.dcp", "cb9865dc7e1db2990a8cf4bc13cf4999", 257643032), Common::DE_DEU, ADGF_UNSTABLE, LATEST_VERSION),
	// Reversion: The Escape Version 1.1 (Italian)
	WME_WINENTRY("reversion1", "Version 1.1",
		WME_ENTRY2s("italian.dcp", "9ce80c1835108f10170a02969f71efe1", 6301836,
					"data.dcp", "cb9865dc7e1db2990a8cf4bc13cf4999", 257643032), Common::IT_ITA, ADGF_UNSTABLE, LATEST_VERSION),
	// Reversion: The Escape Version 1.1 (Portuguese)
	WME_WINENTRY("reversion1", "Version 1.1",
		WME_ENTRY2s("portugues.dcp", "8772501afa2c630a7c697eb99e9c7bda", 5053303,
					"data.dcp", "cb9865dc7e1db2990a8cf4bc13cf4999", 257643032), Common::PT_BRA, ADGF_UNSTABLE, LATEST_VERSION),
	// Reversion: The Escape Version 1.3 (Chinese)
	WME_WINENTRY("reversion1", "Version 1.3",
		WME_ENTRY2s("xlanguage_nz.dcp", "92c4065156e464211685bf799b3279fd", 5130600,
					"data.dcp", "9ebb12f6fd7c038d079f81beb3bd96d5", 254185907), Common::ZH_CNA, ADGF_UNSTABLE, LATEST_VERSION),
	// Reversion: The Escape Version 1.3 (English)
	WME_WINENTRY("reversion1", "Version 1.3",
		WME_ENTRY2s("xlanguage_en.dcp", "05845e1283920a6e4044f2a54f7a9519", 4818543,
					"data.dcp", "9ebb12f6fd7c038d079f81beb3bd96d5", 254185907), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Reversion: The Escape Version 1.3 (French)
	WME_WINENTRY("reversion1", "Version 1.3",
		WME_ENTRY2s("xlanguage_fr.dcp", "441795490e9307eb2ed07830779881ac", 5425959,
					"data.dcp", "9ebb12f6fd7c038d079f81beb3bd96d5", 254185907), Common::FR_FRA, ADGF_UNSTABLE, LATEST_VERSION),
	// Reversion: The Escape Version 1.3 (German)
	WME_WINENTRY("reversion1", "Version 1.3",
		WME_ENTRY2s("xlanguage_de.dcp", "b588041015b93e54b4c246ca77d01e76", 5423798,
					"data.dcp", "9ebb12f6fd7c038d079f81beb3bd96d5", 254185907), Common::DE_DEU, ADGF_UNSTABLE, LATEST_VERSION),
	// Reversion: The Escape Version 1.3 (Italian)
	WME_WINENTRY("reversion1", "Version 1.3",
		WME_ENTRY2s("xlanguage_it.dcp", "a1f4199079b75ee10cded41f05b45d5f", 5386424,
					"data.dcp", "9ebb12f6fd7c038d079f81beb3bd96d5", 254185907), Common::IT_ITA, ADGF_UNSTABLE, LATEST_VERSION),
	// Reversion: The Escape Version 1.3 (Portuguese)
	WME_WINENTRY("reversion1", "Version 1.3",
		WME_ENTRY2s("xlanguage_pt.dcp", "3d653debd37e56756a79401e1004c4d2", 4149165,
					"data.dcp", "9ebb12f6fd7c038d079f81beb3bd96d5", 254185907), Common::PT_BRA, ADGF_UNSTABLE, LATEST_VERSION),
	// Reversion: The Escape Version 1.3.2369 (Chinese)
	WME_WINENTRY("reversion1", "Version 1.3.2369",
		WME_ENTRY2s("xlanguage_nz.dcp", "7146dfa43ffdf0886e034fffe2c8a0c0", 13722261,
					"data.dcp", "aecb5deeea7b0baa871fbd0cef35a648", 254219204), Common::ZH_CNA, ADGF_UNSTABLE, LATEST_VERSION),
	// Reversion: The Escape Version 1.3.2369 (English)
	WME_WINENTRY("reversion1", "Version 1.3.2369",
		WME_ENTRY2s("xlanguage_en.dcp", "64b6fa7eedc09c231f6ce046e77fee05", 11339619,
					"data.dcp", "aecb5deeea7b0baa871fbd0cef35a648", 254219204), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Reversion: The Escape Version 1.3.2369 (French)
	WME_WINENTRY("reversion1", "Version 1.3.2369",
		WME_ENTRY2s("xlanguage_fr.dcp", "d561d562224afea809153a1fd9fdb0c0", 11963210,
					"data.dcp", "aecb5deeea7b0baa871fbd0cef35a648", 254219204), Common::FR_FRA, ADGF_UNSTABLE, LATEST_VERSION),
	// Reversion: The Escape Version 1.3.2369 (German)
	WME_WINENTRY("reversion1", "Version 1.3.2369",
		WME_ENTRY2s("xlanguage_de.dcp", "4e3f614c36bd6bae74b8cc83e663a8f0", 14040310,
					"data.dcp", "aecb5deeea7b0baa871fbd0cef35a648", 254219204), Common::DE_DEU, ADGF_UNSTABLE, LATEST_VERSION),
	// Reversion: The Escape Version 1.3.2369 (Italian)
	WME_WINENTRY("reversion1", "Version 1.3.2369",
		WME_ENTRY2s("xlanguage_it.dcp", "10d09b7fe61946f09dd91d5e8d090f94", 11913752,
					"data.dcp", "aecb5deeea7b0baa871fbd0cef35a648", 254219204), Common::IT_ITA, ADGF_UNSTABLE, LATEST_VERSION),
	// Reversion: The Escape Version 1.3.2369 (Latvian)
	WME_WINENTRY("reversion1", "Version 1.3.2369",
		WME_ENTRY2s("xlanguage_lv.dcp", "704359ab5040b0dab6545064d7aa6eb9", 11414925,
					"data.dcp", "aecb5deeea7b0baa871fbd0cef35a648", 254219204), Common::LV_LAT, ADGF_UNSTABLE, LATEST_VERSION),
	// Reversion: The Escape Version 1.3.2369 (Polish)
	WME_WINENTRY("reversion1", "Version 1.3.2369",
		WME_ENTRY2s("xlanguage_pl.dcp", "c4ad33f57e1e998169552d521c1d6638", 11532215,
					"data.dcp", "aecb5deeea7b0baa871fbd0cef35a648", 254219204), Common::PL_POL, ADGF_UNSTABLE, LATEST_VERSION),
	// Reversion: The Escape Version 1.3.2369 (Portuguese)
	WME_WINENTRY("reversion1", "Version 1.3.2369",
		WME_ENTRY2s("xlanguage_pt.dcp", "886886b6b14aadac844078de856799a6", 10620797,
					"data.dcp", "aecb5deeea7b0baa871fbd0cef35a648", 254219204), Common::PT_BRA, ADGF_UNSTABLE, LATEST_VERSION),
	// Reversion: The Meeting (Chinese)
	WME_WINENTRY("reversion2", "",
		WME_ENTRY2s("xlanguage_nz.dcp", "8c3709474a87a7876109025dff41ff3f", 8746015,
					"data.dcp", "cb9865dc7e1db2990a8cf4bc13cf4999", 257643032), Common::ZH_CNA, ADGF_UNSTABLE, LATEST_VERSION),
	// Reversion: The Meeting (English)
	WME_WINENTRY("reversion2", "",
		WME_ENTRY2s("xlanguage_en.dcp", "ca357d86618d1ab76a21c913f4403cbd", 8414976,
					"data.dcp", "f7938cbfdc48f07934550245a3286921", 255672016), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Reversion: The Meeting (Spanish)
	WME_WINENTRY("reversion2", "",
		WME_ENTRY1s("data.dcp", "f7938cbfdc48f07934550245a3286921", 255672016), Common::ES_ESP, ADGF_UNSTABLE, LATEST_VERSION),
	// Reversion: The Meeting Version 2.0.2412 (Chinese)
	WME_PLATENTRY("reversion2", "Version 2.0.2412",
		WME_ENTRY3s("data.dcp", "f4ffc4df24b7bebad56a24930f33a2bc", 255766600,
					"xlanguage_nz.dcp", "17c79af4928e24484bee77a7e807cc2a", 10737127,
					"Linux.dcp", "21858bd77dc86b03f701fd47900e2f51", 984535), Common::ZH_CNA, Common::kPlatformLinux, ADGF_UNSTABLE, LATEST_VERSION),
	// Reversion: The Meeting Version 2.0.2412 (English)
	WME_PLATENTRY("reversion2", "Version 2.0.2412",
		WME_ENTRY3s("data.dcp", "f4ffc4df24b7bebad56a24930f33a2bc", 255766600,
					"xlanguage_en.dcp", "0598bf752ce93b42bcaf1094df537c7b", 8533057,
					"Linux.dcp", "21858bd77dc86b03f701fd47900e2f51", 984535), Common::EN_ANY, Common::kPlatformLinux, ADGF_UNSTABLE, LATEST_VERSION),
	// Rhiannon: Curse of the four Branches
	WME_WINENTRY("rhiannon", "",
		WME_ENTRY1s("data.dcp", "870f348900b735f1cc79c0608ce32b0e", 1046169851), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Rhiannon: Curse of the four Branches (English PC DVD)
	WME_WINENTRY("rhiannon", "DVD",
		WME_ENTRY1s("data.dcp", "6736bbc921bb6ce5161b3ad095a97bd4", 1053441028), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// 1 1/2 Ritter: Auf der Suche nach der hinreissenden Herzelinde
	WME_WINENTRY("ritter", "",
		WME_ENTRY1s("data.dcp", "5ac416cee605d3a30f4d59687b1cdab2", 364260278), Common::DE_DEU, ADGF_UNSTABLE, LATEST_VERSION),
	// Satan and Son
	WME_WINENTRY("satanandson", "",
		WME_ENTRY1s("data.dcp",  "16a6ba8174b697bbba9299619d1e20c4", 67539054), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, LATEST_VERSION),
	// Rosemary
	WME_WINENTRY("rosemary", "",
		WME_ENTRY1s("data.dcp", "4f2631138bd4d27587d9043f8aeff3df", 29483643), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Securanote
	WME_PLATENTRY("securanote", "",
		WME_ENTRY1s("data.dcp",  "5213d3e59b9e95b7fbd5c56f7de5341a", 2625554), Common::EN_ANY, Common::kPlatformIOS, ADGF_UNSTABLE, LATEST_VERSION),
	// Shaban
	WME_WINENTRY("shaban", "",
		WME_ENTRY1s("data.dcp",  "35f702ca9baabc5c620e0be230195c8a", 755388466), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// The Shine of a Star
	WME_WINENTRY("shinestar", "",
		WME_ENTRY1s("data.dcp", "f05abe9e2427a5e4f73648fa09c4ba8e", 94113060), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Sofia's Debt
	WME_WINENTRY("sofiasdebt", "",
		WME_ENTRY1s("SD.exe", "e9515f9ba1a2925bb6733476a826a650", 9915047), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Space Invaders (Demo)
	WME_WINENTRY("spaceinvaders", "Demo",
		WME_ENTRY1s("data.dcp", "3f27adefdf72f2c1601cf555c80a509f", 1308361), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, LATEST_VERSION),
	// Space Madness
	WME_WINENTRY("spacemadness", "1.0.2",
		WME_ENTRY1s("data.dcp",  "b9b83135dc7a9e1b4b5f50195dbeb630", 39546622), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// The Ancient Mark - Episode 1
	WME_WINENTRY("theancientmark1", "",
		WME_ENTRY1s("data.dcp", "ca04c26f03b2bd307368b306b297ddd7", 364664692), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// The Box
	WME_WINENTRY("thebox", "",
		WME_ENTRY1s("data.dcp", "ec5f0c7e8174e307701447b53afe7e2f", 108372483), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// The Kite (Version 1.1)
	WME_WINENTRY("thekite", "Version 1.1",
		WME_ENTRY1s("data.dcp", "92d29428f464469bda2d81b03d4d5c3e", 47332296), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// The Kite (Version 1.2.e)
	WME_WINENTRY("thekite", "Version 1.2.e",
		WME_ENTRY1s("data.dcp", "92451578b1bdd2b32a1db592a4f6d5fc", 47360539), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// The Kite (Version 1.2.i) (Italian)
	WME_WINENTRY("thekite", "Version 1.2.i",
		WME_ENTRY1s("data.dcp", "d3435b106a1b3b4c1df8ad596d271586", 47509274), Common::IT_ITA, ADGF_UNSTABLE, LATEST_VERSION),
	// The Kite (Version 1.2.r) (Russian)
	WME_WINENTRY("thekite", "Version 1.2.r",
		WME_ENTRY1s("data.dcp", "d531e097dd884737469da014ed882cde", 47554582 ), Common::RU_RUS, ADGF_UNSTABLE, LATEST_VERSION),
	// The Kite (Version 1.3.e)
	WME_WINENTRY("thekite", "Version 1.3.e",
		WME_ENTRY1s("data.dcp", "9761827b51370263b7623721545d7627", 47382987), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Fairy Tales About Toshechka and Boshechka
	WME_WINENTRY("tib", "",
		WME_ENTRY1s("data.dcp", "87d296ef3f46570ed18f000d3885db77", 340264526), Common::RU_RUS, ADGF_UNSTABLE, LATEST_VERSION),
	// The Trader of Stories
	WME_WINENTRY("tradestory", "Demo",
		WME_ENTRY1s("data.dcp", "0a0b51191636cc8ead89b905281c3218", 40401902), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, LATEST_VERSION),
	// the white chamber (multi-language)
	WME_WINENTRY("twc", "",
		WME_ENTRY1s("data.dcp", "0011d01142547c61e51ba24dc42b579e", 186451273), Common::UNK_LANG, ADGF_UNSTABLE, LATEST_VERSION),
	// Vsevolod Prologue (Demo)
	WME_WINENTRY("vsevolod", "Prologue",
		WME_ENTRY1s("data.dcp", "f2dcffd2692dbfcc9371fa1a87970fe7", 388669493), Common::EN_ANY, ADGF_UNSTABLE | ADGF_DEMO, LATEST_VERSION),
	// War
	WME_WINENTRY("war", "",
		WME_ENTRY1s("data.dcp", "003e317cda6d0137bbd5e5d7f089ee4d", 32591890), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Wilma Tetris
	WME_WINENTRY("wtetris", "",
		WME_ENTRY1s("data.dcp", "946e3a0496e6c12fb344c9ed861ff015", 2780093), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
	// Zilm: A Game of Reflex 1.0
	WME_WINENTRY("Zilm", "1.0",
		WME_ENTRY1s("data.dcp",  "098dffaf03d8adbb4cb5633e4733e63c", 351726), Common::EN_ANY, ADGF_UNSTABLE, LATEST_VERSION),
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

