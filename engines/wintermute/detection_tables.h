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
	{"corrosion",		"Corrosion: Cold Winter Waiting"},
	{"deadcity",        "Dead City"},
	{"dreaming",        "Des Reves Elastiques Avec Mille Insectes Nommes Georges"},
	{"dirtysplit",      "Dirty Split"},
	{"dreamscape",      "Dreamscape"},
	{"escapemansion",   "Escape from the Mansion"},
	{"framed",          "Framed"},
	{"ghostsheet",      "Ghost in the Sheet"},
	{"hamlet",          "Hamlet or the last game without MMORPS features, shaders and product placement"},
	{"helga",           "Helga Deep In Trouble"},
	{"jamesperis",      "James Peris: No License Nor Control"},
	{"kulivocko",       "Kulivocko"},
	{"lonelyrobot",     "Project Lonely Robot"},
	{"looky",           "Looky"},
	{"julia",           "J.U.L.I.A."},
	{"mirage",          "Mirage"},
	{"oknytt",          "Oknytt"},
	{"paintaria",       "Paintaria"},
	{"pigeons",         "Pigeons in the Park"},
	{"projectdoom",     "Project: Doom"},
	{"reversion1",      "Reversion: The Escape"},
	{"reversion2",      "Reversion: The Meeting"},
	{"rhiannon",		"Rhiannon: Curse of the four Branches"},
	{"ritter",			"1 1/2 Ritter: Auf der Suche nach der hinreissenden Herzelinde"},
	{"rosemary",        "Rosemary"},
	{"securanote",      "Securanote"},
	{"shaban",          "Shaban"},
	{"shinestar",       "The Shine of a Star"},
	{"spaceinvaders",   "Space Invaders"},
	{"spacemadness",    "Space Madness"},
	{"thebox",          "The Box"},
	{"thekite",			"The Kite"},
	{"tib",             "Fairy Tales About Toshechka and Boshechka"},
	{"tradestory",      "The Trader of Stories"},
	{"twc",             "the white chamber"},
	{"vsevolod",        "Vsevolod"},
	{"wintermute",      "Wintermute engine game"},
	{"wtetris",         "Wilma Tetris"},
	{"zilm",            "Zilm: A Game of Reflex"},
	{0, 0}
};

static const ADGameDescription gameDescriptions[] = {
	// Five Lethal Demons
	{
		"5ld",
		"",
		AD_ENTRY1s("data.dcp", "1037a77cbd001e0644898addc022322c", 15407750),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Five Magical Amulets
	{
		"5ma",
		"",
		AD_ENTRY1s("data.dcp", "0134e92bcd5fd2837df3971087e96067", 163316498),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Actual Destination
	{
		"actualdest",
		"",
		AD_ENTRY1s("data.dcp", "6926f44b26f21ceb1d840eaab9aeb510", 9081740),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Boredom of Agustin Cordes
	{
		"agustin",
		"",
		AD_ENTRY1s("data.dcp", "abb79c16c9b92e9b06525a4c7c3f5861", 2461949),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Beyond the Threshold
	{
		"bthreshold",
		"",
		AD_ENTRY1s("data.dcp", "d49bf9ccb2e74507447c82d6ad3e2bc4", 12773712),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Bickadoodle
	{
		"bickadoodle",
		"",
		AD_ENTRY1s("data.dcp", "84db4d1594cac95e25614985775d10a8", 35303844),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Bickadoodle (Ver 1.1)
	{
		"bickadoodle",
		"Version 1.1",
		AD_ENTRY1s("data.dcp", "8bb52ac9a9ee129c5059e8e808b669d7", 35337760),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Bickadoodle (download from http://aethericgames.com/games/bickadoodle/download-bickadoodle/)
	{
		"bickadoodle",
		"",
		AD_ENTRY1s("data.dcp", "1584d83577c32add0fce27fae91141a2", 35337728),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Book of Gron Part One
	{
		"bookofgron",
		"",
		AD_ENTRY1s("data.dcp", "e61b2ebee044a82fa0f8ca0fce2c8946", 83129531),
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Carol Reed 4 - East Side Story (Demo)
	{
		"carolreed4",
		"Demo",
		AD_ENTRY1s("data.dcp", "b3f8b09bb4b05ee3e9d14697525257f9", 59296246),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE |
		ADGF_DEMO,
		GUIO0()
	},
	// Carol Reed 4 - East Side Story
	{
		"carolreed4",
		"",
		AD_ENTRY1s("data.dcp", "b26377797f060afc2d440d820100c1ce", 529320536),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE |
		ADGF_DEMO,
		GUIO0()
	},
	// Carol Reed 5 - The Colour of Murder
	{
		"carolreed5",
		"",
		AD_ENTRY1s("data.dcp", "3fcfca44209545d0e26774156427b494", 603660415),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Carol Reed 6 - Black Circle
	{
		"carolreed6",
		"",
		AD_ENTRY1s("data.dcp", "0e4c532beecf23d85012168753f41189", 456258147),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Carol Reed 7 - Blue Madonna (Demo)
	{
		"carolreed7",
		"Demo",
		AD_ENTRY1s("data.dcp", "0372ad0c775266f6355e9e8ae397a2f1", 103719442),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE |
		ADGF_DEMO,
		GUIO0()
	},
	// Carol Reed 7 - Blue Madonna
	{
		"carolreed7",
		"",
		AD_ENTRY1s("data.dcp", "24e3db3e2fabfc956713796d87a3efb0", 495471147),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Carol Reed 8 - Amber's Blood
	{
		"carolreed8",
		"",
		AD_ENTRY1s("data.dcp", "859d16b0d5b9b255e470cbded2c6cedc", 502714557),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Carol Reed 9 - Cold Case Summer
	{
		"carolreed9",
		"",
		AD_ENTRY1s("data.dcp", "2b343b48a7aee508d728a546b414a255", 620005266),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Chivalry is Not Dead
	{
		"chivalry",
		"",
		AD_ENTRY1s("data.dcp", "ebd0915d9a12df5224be22f53bb23eb6", 7278306),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_TESTING,
		GUIO0()
	},
	// Chivalry is Not Dead (Version from deirdrakai.com)
	{
		"chivalry",
		"",
		AD_ENTRY1s("data.dcp", "ae6d91b9517f4d2851a8ad94c96951c8", 7278302),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_TESTING,
		GUIO0()
	},
	// Corrosion: Cold Winter Waiting
	{
		"corrosion",
		"",
		AD_ENTRY1s("data.dcp", "ae885b1a8faa0b27f43c0e8f0df02fc9", 525931618),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_TESTING,
		GUIO0()
	},
	// Dead City (Czech)
	{
		"deadcity",
		"",
		{
			// The Czech data are in data.dcp, so in this case we'll have to
			// just detect the english version twice, to give the user a choice.
			{"english.dcp", 0, "c591046d6de7e381d76f70e0787b2b1f", 415935},
			{"data.dcp", 0, "7ebfd50d1a22370ed7b079bcaa631d62", 9070205},
			AD_LISTEND
		},
		Common::CZ_CZE,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Dead City (English)
	{
		"deadcity",
		"",
		{
			{"english.dcp", 0, "c591046d6de7e381d76f70e0787b2b1f", 415935},
			{"data.dcp", 0, "7ebfd50d1a22370ed7b079bcaa631d62", 9070205},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Dead City (Italian)
	{
		"deadcity",
		"",
		{
			{"italian.dcp", 0, "92d8efb94436bec7bd1b7fe0b548192e", 454037},
			{"data.dcp", 0, "7ebfd50d1a22370ed7b079bcaa631d62", 9070205},
			AD_LISTEND
		},
		Common::IT_ITA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Dead City (Russian)
	{
		"deadcity",
		"",
		{
			{"russian.dcp", 0, "a0ae71e9e1185596fffb07ad2c951eb9", 653317},
			{"data.dcp", 0, "7ebfd50d1a22370ed7b079bcaa631d62", 9070205},
			AD_LISTEND
		},
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Dirty Split (Czech)
	{
		"dirtysplit",
		"",
		{
			{"czech.dcp", 0, "08a71446467cf8f9444cfea446b46ad6", 127697934},
			{"data.dcp", 0, "8b4b81b718bf65f30a67fc0b1e329eb5", 88577623},
		},
		Common::CZ_CZE,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Dirty Split (English)
	{
		"dirtysplit",
		"",
		AD_ENTRY1s("data.dcp", "8f3dae199361ece0f59fb20cfff6eed3", 88577621),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Dirty Split (French)
	{
		"dirtysplit",
		"",
		{
			{"french.dcp", 0, "a0508dedebd0fe478d0158fa4c2a1136", 125534323},
			{"data.dcp", 0, "e6d70c7f5d181b761cfcf974adf9186a", 88577623},
			AD_LISTEND
		},
		Common::FR_FRA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Dirty Split (German)
	{
		"dirtysplit",
		"",
		AD_ENTRY1s("data.dcp", "139d8a25579e969f8b37d20e6e3de5f9", 92668291),
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Dirty Split (Italian)
	{
		"dirtysplit",
		"",
		{
			{"italian.dcp", 0, "8108807fbd8af70be1ec452d0fd1131b", 125513726},
			{"data.dcp", 0, "35a150e22af274185883fdbb142c6fb1", 88577623},
		},
		Common::IT_ITA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Dirty Split (Spanish)
	{
		"dirtysplit",
		"",
		{
			{"spanish.dcp", 0, "b3982c0a5e85b42e1e38240fef004aa4", 164428596},
			{"data.dcp", 0, "63766d6c68b9f00b632ea1736fc8a95c", 88577621},
		},
		Common::ES_ESP,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Des Reves Elastiques Avec Mille Insectes Nommes Georges
	{
		"dreaming",
		"",
		AD_ENTRY1s("data.dcp", "4af26d97ea063fc1277ce30ae431de90", 8804073),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Dreamscape
	{
		"dreamscape",
		"",
		AD_ENTRY1s("data.dcp",  "7a5752ed4446c862be9f02d7932acf54", 17034377),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Escape from the Mansion
	{
		"escapemansion",
		"Beta 1",
		AD_ENTRY1s("data.dcp",  "d8e348b2312cc36a929cad75f12e0b3a", 21452380),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Escape from the Mansion
	{
		"escapemansion",
		"Beta 2",
		AD_ENTRY1s("data.dcp",  "ded5fa6c5f2afdaf2cafb53e52cd3dd8", 21455763),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Escape from the Mansion
	{
		"escapemansion",
		"1.3",
		AD_ENTRY1s("data.dcp",  "1e5d231b56c8a228cd15cb690f50253e", 29261972),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Framed
	{
		"framed",
		"",
		AD_ENTRY1s("data.dcp",  "e7259fb36f2c6f9f28242291e0c3de98", 34690568),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Ghost in the Sheet
	{
		"ghostsheet",
		"",
		{
			{"english.dcp", 0, "e6d0aad2c89996bcabe416105a3d6d3a", 12221017},
			{"data.dcp", 0, "b2f8b05328e4881e15e98e845b63f451", 168003},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Ghost in the Sheet (Demo)
	{
		"ghostsheet",
		"Demo",
		AD_ENTRY1s("data.dcp", "dc1f6595f412ac25a52eaf47dad4ab81", 169083),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE |
		ADGF_DEMO,
		GUIO0()
	},
	// Hamlet or the last game without MMORPS features, shaders and product placement
	{
		"hamlet",
		"",
		AD_ENTRY1s("data.dcp", "f624add957a77c9930529fb28cc2450f", 88183022),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Helga Deep In Trouble (English)
	{
		"helga",
		"",
		{
			{"english.dcp", 0, "bfa136b21bdbc7d8691c0770a6d40bc3", 135931},
			{"data.dcp", 0, "25cb955a60b58326f2eeda1ce288fb37", 183251259},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Helga Deep In Trouble (Demo) (English)
	{
		"helga",
		"Demo",
		{
			{"english.dcp", 0, "b3a93e678f0ef97200f691cd1724643f", 135864},
			{"data.dcp", 0, "45134ed93bc391edf148b79cdcbf2a09", 154266028},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE |
		ADGF_DEMO,
		GUIO0()
	},
	// James Peris: No License Nor Control (English)
	{
		"jamesperis",
		"",
		AD_ENTRY1s("data.dcp", "a420961e170cb7d168a0d2bae2fe5218", 225294032),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// James Peris: No License Nor Control (Spanish)
	{
		"jamesperis",
		"",
		AD_ENTRY1s("data.dcp", "a420961e170cb7d168a0d2bae2fe5218", 225294032),
		Common::ES_ESP,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// James Peris: No License Nor Control (Demo) (English)
	{
		"jamesperis",
		"Demo",
		AD_ENTRY1s("data.dcp", "edb9f9c7a08993c1e28f4e477b5f9830", 116113507),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE |
		ADGF_DEMO,
		GUIO0()
	},
	// James Peris: No License Nor Control (Demo) (Spanish)
	{
		"jamesperis",
		"Demo",
		AD_ENTRY1s("data.dcp", "edb9f9c7a08993c1e28f4e477b5f9830", 116113507),
		Common::ES_ESP,
		Common::kPlatformWindows,
		ADGF_UNSTABLE |
		ADGF_DEMO,
		GUIO0()
	},
	// J.U.L.I.A. (English)
	{
		"julia",
		"",
		AD_ENTRY1s("data.dcp", "c2264b4f8fcd132d2913ff5b6076a24f", 10109741),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// J.U.L.I.A. (English, Bundle in a box-version)
	{
		"julia",
		"Version 1.2",
		AD_ENTRY1s("data.dcp", "fe90023ccc22f35185b40b910e0d03a2", 10101373),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// J.U.L.I.A. (English) (Demo)
	{
		"julia",
		"Demo",
		AD_ENTRY1s("data.dcp", "f0bbc3394555a9811f6050dae428cab6", 7655237),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE |
		ADGF_DEMO,
		GUIO0()
	},
	// J.U.L.I.A. (English) (Greenlight Demo)
	{
		"julia",
		"Greenlight Demo",
		AD_ENTRY1s("data.dcp", "4befd448d36b0dae9c3ab1aa7cb8b78d", 7271886),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE |
		ADGF_DEMO,
		GUIO0()
	},
	// Kulivocko (Czech)
	{
		"kulivocko",
		"",
		AD_ENTRY1s("data.dcp", "44306dc470e9b27474043932eccee02f", 155106392),
		Common::CZ_CZE,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Kulivocko (Czech) (Demo)
	{
		"kulivocko",
		"Demo",
		AD_ENTRY1s("data.dcp", "63b164bdfadecbb0deb5da691afb8154", 48362234),
		Common::CZ_CZE,
		Common::kPlatformWindows,
		ADGF_UNSTABLE |
		ADGF_DEMO,
		GUIO0()
	},
	// Looky Demo (English)
	{
		"looky",
		"Demo",
		{
			{"english.dcp", 0, "1388e1dd320f4d553dea3b0316812f9d", 1358442},
			{"data.dcp", 0, "7074bcd7bc7ad7eb04c271aafb964c32", 13815660},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE |
		ADGF_DEMO,
		GUIO0()
	},
	// Looky Demo (German)
	{
		"looky",
		"Demo",
		{
			{"german.dcp", 0, "606c048426dfbe94442b59fd34a5c76e", 14339496},
			{"data.dcp", 0, "7074bcd7bc7ad7eb04c271aafb964c32", 13815660},
			AD_LISTEND
		},
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_UNSTABLE |
		ADGF_DEMO,
		GUIO0()
	},
	// Looky (German)
	{
		"looky",
		"",
		{
			{"german.dcp", 0, "bf4c2b8c26342342441a6d64934ab832", 107027865},
			{"data.dcp", 0, "50de0beaa5ad621aa9f020df901d1e74", 1342214},
			AD_LISTEND
		},
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Mirage
	{
		"mirage",
		"",
		AD_ENTRY1s("data.dcp", "d230b0b99c0aa77b9ecd094d8ee5573b", 17844056),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Oknytt
	{
		"oknytt",
		"Version 1.0",
		AD_ENTRY1s("data.dcp", "6456cf8f429905c83f07509f9da536dd", 109502959),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Paintaria
	{
		"paintaria",
		"",
		AD_ENTRY1s("data.dcp", "354c08440c98150ff0d4008dd2865880", 48326040),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Pigeons in the Park
	{
		"pigeons",
		"",
		AD_ENTRY1s("data.dcp", "9143a5b6ff8206aefe3c4c643add3ec7", 2611100),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Project: Doom
	{
		"projectdoom",
		"",
		AD_ENTRY1s("data.dcp", "d5894b65a40706845434b99870bcab92", 99223761),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Project Lonely Robot
	{
		"lonelyrobot",
		"beta",
		AD_ENTRY1s("data.dcp", "a0cf7ad5bab957416dcda454e9f28ef0", 3420120),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE |
		ADGF_DEMO,
		GUIO0()
	},
	// Reversion: The Escape Version 1.0
	{
		"reversion1",
		"Version 1.0",
		AD_ENTRY1s("data.dcp", "cd616f98ebfd047e0c540b50b4b70761", 254384531),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Escape Version 1.1 (Chinese)
	{
		"reversion1",
		"Version 1.1",
		{
			{"chinese.dcp", 0, "cf97150739499a4c15f51dc534ff85a1", 6330561},
			{"data.dcp", 0, "cb9865dc7e1db2990a8cf4bc13cf4999", 257643032},
			AD_LISTEND
		},
		Common::ZH_CNA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Escape Version 1.1 (English)
	{
		"reversion1",
		"Version 1.1",
		{
			{"english.dcp", 0, "7b2f061d7c91365c5d04605f1de032b3", 5702699},
			{"data.dcp", 0, "cb9865dc7e1db2990a8cf4bc13cf4999", 257643032},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Escape Version 1.1 (French)
	{
		"reversion1",
		"Version 1.1",
		{
			{"french.dcp", 0, "214204b6022c5ed67fada44557690faf", 6327400},
			{"data.dcp", 0, "cb9865dc7e1db2990a8cf4bc13cf4999", 257643032},
			AD_LISTEND
		},
		Common::FR_FRA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Escape Version 1.1 (German)
	{
		"reversion1",
		"Version 1.1",
		{
			{"german.dcp", 0, "96677823b36d580a4a29e3659071071c", 6340699},
			{"data.dcp", 0, "cb9865dc7e1db2990a8cf4bc13cf4999", 257643032},
			AD_LISTEND
		},
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Escape Version 1.1 (Italian)
	{
		"reversion1",
		"Version 1.1",
		{
			{"italian.dcp", 0, "9ce80c1835108f10170a02969f71efe1", 6301836},
			{"data.dcp", 0, "cb9865dc7e1db2990a8cf4bc13cf4999", 257643032},
			AD_LISTEND
		},
		Common::IT_ITA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Escape Version 1.1 (Portuguese)
	{
		"reversion1",
		"Version 1.1",
		{
			{"portugues.dcp", 0, "8772501afa2c630a7c697eb99e9c7bda", 5053303},
			{"data.dcp", 0, "cb9865dc7e1db2990a8cf4bc13cf4999", 257643032},
			AD_LISTEND
		},
		Common::PT_BRA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Escape Version 1.3 (Chinese)
	{
		"reversion1",
		"Version 1.3",
		{
			{"xlanguage_nz.dcp", 0, "92c4065156e464211685bf799b3279fd", 5130600},
			{"data.dcp", 0, "9ebb12f6fd7c038d079f81beb3bd96d5", 254185907},
			AD_LISTEND
		},
		Common::ZH_CNA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Escape Version 1.3 (English)
	{
		"reversion1",
		"Version 1.3",
		{
			{"xlanguage_en.dcp", 0, "05845e1283920a6e4044f2a54f7a9519", 4818543},
			{"data.dcp", 0, "9ebb12f6fd7c038d079f81beb3bd96d5", 254185907},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Escape Version 1.3 (French)
	{
		"reversion1",
		"Version 1.3",
		{
			{"xlanguage_fr.dcp", 0, "441795490e9307eb2ed07830779881ac", 5425959},
			{"data.dcp", 0, "9ebb12f6fd7c038d079f81beb3bd96d5", 254185907},
			AD_LISTEND
		},
		Common::FR_FRA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Escape Version 1.3 (German)
	{
		"reversion1",
		"Version 1.3",
		{
			{"xlanguage_de.dcp", 0, "b588041015b93e54b4c246ca77d01e76", 5423798},
			{"data.dcp", 0, "9ebb12f6fd7c038d079f81beb3bd96d5", 254185907},
			AD_LISTEND
		},
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Escape Version 1.3 (Italian)
	{
		"reversion1",
		"Version 1.3",
		{
			{"xlanguage_it.dcp", 0, "a1f4199079b75ee10cded41f05b45d5f", 5386424},
			{"data.dcp", 0, "9ebb12f6fd7c038d079f81beb3bd96d5", 254185907},
			AD_LISTEND
		},
		Common::IT_ITA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Escape Version 1.3 (Portuguese)
	{
		"reversion1",
		"Version 1.3",
		{
			{"xlanguage_pt.dcp", 0, "3d653debd37e56756a79401e1004c4d2", 4149165},
			{"data.dcp", 0, "9ebb12f6fd7c038d079f81beb3bd96d5", 254185907},
			AD_LISTEND
		},
		Common::PT_BRA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Escape Version 1.3.2369 (Chinese)
	{
		"reversion1",
		"Version 1.3.2369",
		{
			{"xlanguage_nz.dcp", 0, "7146dfa43ffdf0886e034fffe2c8a0c0", 13722261},
			{"data.dcp", 0, "aecb5deeea7b0baa871fbd0cef35a648", 254219204},
			AD_LISTEND
		},
		Common::ZH_CNA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Escape Version 1.3.2369 (English)
	{
		"reversion1",
		"Version 1.3.2369",
		{
			{"xlanguage_en.dcp", 0, "64b6fa7eedc09c231f6ce046e77fee05", 11339619},
			{"data.dcp", 0, "aecb5deeea7b0baa871fbd0cef35a648", 254219204},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Escape Version 1.3.2369 (French)
	{
		"reversion1",
		"Version 1.3.2369",
		{
			{"xlanguage_fr.dcp", 0, "d561d562224afea809153a1fd9fdb0c0", 11963210},
			{"data.dcp", 0, "aecb5deeea7b0baa871fbd0cef35a648", 254219204},
			AD_LISTEND
		},
		Common::FR_FRA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Escape Version 1.3.2369 (German)
	{
		"reversion1",
		"Version 1.3.2369",
		{
			{"xlanguage_de.dcp", 0, "4e3f614c36bd6bae74b8cc83e663a8f0", 14040310},
			{"data.dcp", 0, "aecb5deeea7b0baa871fbd0cef35a648", 254219204},
			AD_LISTEND
		},
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Escape Version 1.3.2369 (Italian)
	{
		"reversion1",
		"Version 1.3.2369",
		{
			{"xlanguage_it.dcp", 0, "10d09b7fe61946f09dd91d5e8d090f94", 11913752},
			{"data.dcp", 0, "aecb5deeea7b0baa871fbd0cef35a648", 254219204},
			AD_LISTEND
		},
		Common::IT_ITA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Escape Version 1.3.2369 (Latvian)
	{
		"reversion1",
		"Version 1.3.2369",
		{
			{"xlanguage_lv.dcp", 0, "704359ab5040b0dab6545064d7aa6eb9", 11414925},
			{"data.dcp", 0, "aecb5deeea7b0baa871fbd0cef35a648", 254219204},
			AD_LISTEND
		},
		Common::LV_LAT,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Escape Version 1.3.2369 (Polish)
	{
		"reversion1",
		"Version 1.3.2369",
		{
			{"xlanguage_pl.dcp", 0, "c4ad33f57e1e998169552d521c1d6638", 11532215},
			{"data.dcp", 0, "aecb5deeea7b0baa871fbd0cef35a648", 254219204},
			AD_LISTEND
		},
		Common::PL_POL,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Escape Version 1.3.2369 (Portuguese)
	{
		"reversion1",
		"Version 1.3.2369",
		{
			{"xlanguage_pt.dcp", 0, "886886b6b14aadac844078de856799a6", 10620797},
			{"data.dcp", 0, "aecb5deeea7b0baa871fbd0cef35a648", 254219204},
			AD_LISTEND
		},
		Common::PT_BRA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Meeting (Chinese)
	{
		"reversion2",
		"",
		{
			{"xlanguage_nz.dcp", 0, "8c3709474a87a7876109025dff41ff3f", 8746015},
			{"data.dcp", 0, "cb9865dc7e1db2990a8cf4bc13cf4999", 257643032},
			AD_LISTEND
		},
		Common::ZH_CNA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Meeting (English)
	{
		"reversion2",
		"",
		{
			{"xlanguage_en.dcp", 0, "ca357d86618d1ab76a21c913f4403cbd", 8414976},
			{"data.dcp", 0, "f7938cbfdc48f07934550245a3286921", 255672016},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Meeting (Spanish)
	{
		"reversion2",
		"",
		AD_ENTRY1s("data.dcp", "f7938cbfdc48f07934550245a3286921", 255672016),
		Common::ES_ESP,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Meeting Version 2.0.2412 (Chinese)
	{
		"reversion2",
		"Version 2.0.2412",
		{
			{"data.dcp", 0, "f4ffc4df24b7bebad56a24930f33a2bc", 255766600},
			{"xlanguage_nz.dcp", 0, "17c79af4928e24484bee77a7e807cc2a", 10737127},
			{"Linux.dcp", 0, "21858bd77dc86b03f701fd47900e2f51", 984535},
			AD_LISTEND
		},
		Common::ZH_CNA,
		Common::kPlatformLinux,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Reversion: The Meeting Version 2.0.2412 (English)
	{
		"reversion2",
		"Version 2.0.2412",
		{
			{"data.dcp", 0, "f4ffc4df24b7bebad56a24930f33a2bc", 255766600},
			{"xlanguage_en.dcp", 0, "0598bf752ce93b42bcaf1094df537c7b", 8533057},
			{"Linux.dcp", 0, "21858bd77dc86b03f701fd47900e2f51", 984535},
			AD_LISTEND
		},
		Common::EN_ANY,
		Common::kPlatformLinux,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Rhiannon: Curse of the four Branches
	{
		"rhiannon",
		"",
		AD_ENTRY1s("data.dcp", "870f348900b735f1cc79c0608ce32b0e", 1046169851),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Rhiannon: Curse of the four Branches (English PC DVD)
	{
		"rhiannon",
		"DVD",
		AD_ENTRY1s("data.dcp", "6736bbc921bb6ce5161b3ad095a97bd4", 1053441028),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// 1 1/2 Ritter: Auf der Suche nach der hinreissenden Herzelinde
	{
		"ritter",
		"",
		AD_ENTRY1s("data.dcp", "5ac416cee605d3a30f4d59687b1cdab2", 364260278),
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Rosemary
	{
		"rosemary",
		"",
		AD_ENTRY1s("data.dcp", "4f2631138bd4d27587d9043f8aeff3df", 29483643),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Securanote
	{
		"securanote",
		"",
		AD_ENTRY1s("data.dcp",  "5213d3e59b9e95b7fbd5c56f7de5341a", 2625554),
		Common::EN_ANY,
		Common::kPlatformIOS,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Shaban
	{
		"shaban",
		"",
		AD_ENTRY1s("data.dcp",  "35f702ca9baabc5c620e0be230195c8a", 755388466),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// The Shine of a Star
	{
		"shinestar",
		"",
		AD_ENTRY1s("data.dcp", "f05abe9e2427a5e4f73648fa09c4ba8e", 94113060),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Space Invaders (Demo)
	{
		"spaceinvaders",
		"Demo",
		AD_ENTRY1s("data.dcp", "3f27adefdf72f2c1601cf555c80a509f", 1308361),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE |
		ADGF_DEMO,
		GUIO0()
	},
	// Space Madness
	{
		"spacemadness",
		"1.0.2",
		AD_ENTRY1s("data.dcp",  "b9b83135dc7a9e1b4b5f50195dbeb630", 39546622),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// The Box
	{
		"thebox",
		"",
		AD_ENTRY1s("data.dcp", "ec5f0c7e8174e307701447b53afe7e2f", 108372483),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// The Kite (Version 1.1)
	{
		"thekite",
		"Version 1.1",
		AD_ENTRY1s("data.dcp", "92d29428f464469bda2d81b03d4d5c3e", 47332296),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// The Kite (Version 1.2.e)
	{
		"thekite",
		"Version 1.2.e",
		AD_ENTRY1s("data.dcp", "92451578b1bdd2b32a1db592a4f6d5fc", 47360539),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// The Kite (Version 1.2.i) (Italian)
	{
		"thekite",
		"Version 1.2.i",
		AD_ENTRY1s("data.dcp", "d3435b106a1b3b4c1df8ad596d271586", 47509274),
		Common::IT_ITA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// The Kite (Version 1.2.r) (Russian)
	{
		"thekite",
		"Version 1.2.r",
		AD_ENTRY1s("data.dcp", "d531e097dd884737469da014ed882cde", 47554582 ),
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// The Kite (Version 1.3.e)
	{
		"thekite",
		"Version 1.3.e",
		AD_ENTRY1s("data.dcp", "9761827b51370263b7623721545d7627", 47382987),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Fairy Tales About Toshechka and Boshechka
	{
		"tib",
		"",
		AD_ENTRY1s("data.dcp", "87d296ef3f46570ed18f000d3885db77", 340264526),
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// The Trader of Stories
	{
		"tradestory",
		"Demo",
		AD_ENTRY1s("data.dcp", "0a0b51191636cc8ead89b905281c3218", 40401902),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE |
		ADGF_DEMO,
		GUIO0()
	},
	// the white chamber (multi-language)
	{
		"twc",
		"",
		AD_ENTRY1s("data.dcp", "0011d01142547c61e51ba24dc42b579e", 186451273),
		Common::UNK_LANG,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Vsevolod Prologue (Demo)
	{
		"vsevolod",
		"Prologue",
		AD_ENTRY1s("data.dcp", "f2dcffd2692dbfcc9371fa1a87970fe7", 388669493),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE |
		ADGF_DEMO,
		GUIO0()
	},
	// Wilma Tetris
	{
		"wtetris",
		"",
		AD_ENTRY1s("data.dcp", "946e3a0496e6c12fb344c9ed861ff015", 2780093),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	// Zilm: A Game of Reflex 1.0
	{
		"Zilm",
		"1.0",
		AD_ENTRY1s("data.dcp",  "098dffaf03d8adbb4cb5633e4733e63c", 351726),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO0()
	},
	AD_TABLE_END_MARKER
};

} // End of namespace Wintermute

