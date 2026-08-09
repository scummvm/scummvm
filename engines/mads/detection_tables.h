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

namespace MADS {

static const MADSGameDescription gameDescriptions[] = {
	{
		// Rex Nebular and the Cosmic Gender Bender DOS English USA (compressed, 3.5" floppy installer)
		{
			"nebular",
			"3.5 inch floppy installer",
			AD_ENTRY2s("mpslabs.001", "4df5c557b52abb5b661cf4befe5ae301", 1315354, "mpslabs.idx", "598ede8e361a60a4e577bbe40f83295b", 5598),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE | GF_INSTALLER,
#ifdef USE_TTS
			GUIO8(GUIO_NOSPEECH, GAMEOPTION_EASY_MOUSE, GAMEOPTION_ANIMATED_INVENTORY, GAMEOPTION_ANIMATED_INTERFACE, GAMEOPTION_NAUGHTY_MODE, GAMEOPTION_TTS_NARRATOR, GAMEOPTION_COPY_PROTECTION, GAMEOPTION_ORIGINAL_SAVELOAD)
#else
			GUIO7(GUIO_NOSPEECH, GAMEOPTION_EASY_MOUSE, GAMEOPTION_ANIMATED_INVENTORY, GAMEOPTION_ANIMATED_INTERFACE, GAMEOPTION_NAUGHTY_MODE, GAMEOPTION_COPY_PROTECTION, GAMEOPTION_ORIGINAL_SAVELOAD)
#endif
		},
		GType_RexNebular,
		0
	},

	{
		// Rex Nebular and the Cosmic Gender Bender DOS English UK 8.44 (compressed, 3.5" floppy installer)
		{
			"nebular",
			"3.5 inch floppy installer",
			AD_ENTRY2s("mpslabs.001", "4df5c557b52abb5b661cf4befe5ae301", 1315354, "mpslabs.idx", "d74bed6513d0a5d514269750f574a23b", 5598),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE | GF_INSTALLER,
#ifdef USE_TTS
			GUIO8(GUIO_NOSPEECH, GAMEOPTION_EASY_MOUSE, GAMEOPTION_ANIMATED_INVENTORY, GAMEOPTION_ANIMATED_INTERFACE, GAMEOPTION_NAUGHTY_MODE, GAMEOPTION_TTS_NARRATOR, GAMEOPTION_COPY_PROTECTION, GAMEOPTION_ORIGINAL_SAVELOAD)
#else
			GUIO7(GUIO_NOSPEECH, GAMEOPTION_EASY_MOUSE, GAMEOPTION_ANIMATED_INVENTORY, GAMEOPTION_ANIMATED_INTERFACE, GAMEOPTION_NAUGHTY_MODE, GAMEOPTION_COPY_PROTECTION, GAMEOPTION_ORIGINAL_SAVELOAD)
#endif
		},
		GType_RexNebular,
		0
	},

	{
		// Rex Nebular and the Cosmic Gender Bender DOS English (compressed, 5.25" floppy installer)
		{
			"nebular",
			"5.25 inch floppy installer",
			AD_ENTRY2s("mpslabs.001", "4df5c557b52abb5b661cf4befe5ae301", 1071642, "mpslabs.idx", "d8fac57aac1c0c41ba3a1554ccc89d21", 5500),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE | GF_INSTALLER,
#ifdef USE_TTS
			GUIO8(GUIO_NOSPEECH, GAMEOPTION_EASY_MOUSE, GAMEOPTION_ANIMATED_INVENTORY, GAMEOPTION_ANIMATED_INTERFACE, GAMEOPTION_NAUGHTY_MODE, GAMEOPTION_TTS_NARRATOR, GAMEOPTION_COPY_PROTECTION, GAMEOPTION_ORIGINAL_SAVELOAD)
#else
			GUIO7(GUIO_NOSPEECH, GAMEOPTION_EASY_MOUSE, GAMEOPTION_ANIMATED_INVENTORY, GAMEOPTION_ANIMATED_INTERFACE, GAMEOPTION_NAUGHTY_MODE, GAMEOPTION_COPY_PROTECTION, GAMEOPTION_ORIGINAL_SAVELOAD)
#endif
		},
		GType_RexNebular,
		0
	},

	{
		// Rex Nebular and the Cosmic Gender Bender DOS English, same as USA/UK entries but uncompressed
		{
			"nebular",
			0,
			AD_ENTRY1s("section1.hag", "6f725eb38660de8af31ec7cdd628d615", 927222),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
#ifdef USE_TTS
			GUIO8(GUIO_NOSPEECH, GAMEOPTION_EASY_MOUSE, GAMEOPTION_ANIMATED_INVENTORY, GAMEOPTION_ANIMATED_INTERFACE, GAMEOPTION_NAUGHTY_MODE, GAMEOPTION_TTS_NARRATOR, GAMEOPTION_COPY_PROTECTION, GAMEOPTION_ORIGINAL_SAVELOAD)
#else
			GUIO7(GUIO_NOSPEECH, GAMEOPTION_EASY_MOUSE, GAMEOPTION_ANIMATED_INVENTORY, GAMEOPTION_ANIMATED_INTERFACE, GAMEOPTION_NAUGHTY_MODE, GAMEOPTION_COPY_PROTECTION, GAMEOPTION_ORIGINAL_SAVELOAD)
#endif
		},
		GType_RexNebular,
		0
	},

	{
		// Rex Nebular and the Cosmic Gender Bender DOS English 8.49 Alternate
		{
			"nebular",
			0,
			AD_ENTRY1s("section1.hag", "d583576923e3437937fb7f46f4b6274f", 927222),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
#ifdef USE_TTS
			GUIO8(GUIO_NOSPEECH, GAMEOPTION_EASY_MOUSE, GAMEOPTION_ANIMATED_INVENTORY, GAMEOPTION_ANIMATED_INTERFACE, GAMEOPTION_NAUGHTY_MODE, GAMEOPTION_TTS_NARRATOR, GAMEOPTION_COPY_PROTECTION, GAMEOPTION_ORIGINAL_SAVELOAD)
#else
			GUIO7(GUIO_NOSPEECH, GAMEOPTION_EASY_MOUSE, GAMEOPTION_ANIMATED_INVENTORY, GAMEOPTION_ANIMATED_INTERFACE, GAMEOPTION_NAUGHTY_MODE, GAMEOPTION_COPY_PROTECTION, GAMEOPTION_ORIGINAL_SAVELOAD)
#endif
		},
		GType_RexNebular,
		0
	},

	{
		// Rex Nebular and the Cosmic Gender Bender DOS Russian fanmade
		{
			"nebular",
			"Fanmade",
			AD_ENTRY1s("section1.hag", "1c09f2cb2c801741769be55c83f89c52", 943426),
			Common::RU_RUS,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO7(GUIO_NOSPEECH, GAMEOPTION_EASY_MOUSE, GAMEOPTION_ANIMATED_INVENTORY, GAMEOPTION_ANIMATED_INTERFACE, GAMEOPTION_NAUGHTY_MODE, GAMEOPTION_COPY_PROTECTION, GAMEOPTION_ORIGINAL_SAVELOAD)
		},
		GType_RexNebular,
		0
	},

	{
		// Rex Nebular and the Cosmic Gender Bender DOS English Demo
		{
			"nebular",
			"Demo",
			AD_ENTRY1s("section1.hag", "ead7ea63459173c32569feecac608c4f", 192065),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE | ADGF_DEMO,
			GUIO6(GUIO_NOSPEECH, GAMEOPTION_EASY_MOUSE, GAMEOPTION_ANIMATED_INVENTORY, GAMEOPTION_ANIMATED_INTERFACE, GAMEOPTION_NAUGHTY_MODE, GAMEOPTION_ORIGINAL_SAVELOAD)
		},
		GType_RexNebular,
		0
	},

	{
		// Rex Nebular and the Cosmic Gender Bender Macintosh English
		{
			"nebular",
			0,
			AD_ENTRY4s("Rex Nebular", "r:e70957f9448af272e27990524b8a3f1a", 509575,
					   "Rex Global Data", "r:ff13f7f2e6d6bb85a990015f44c19f2e", 1297502,
					   "Rex Section I Data", "r:63cad9da79cc235a4021128bfb9f2063", 924486,
					   "Rex Sound Data", "r:36dcb243224b489c38f1289fa61ca874", 2064349),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_UNSTABLE,
#ifdef USE_TTS
			GUIO10(GUIO_NOMIDI, GUIO_NOSPEECH, GUIO_NOASPECT, GAMEOPTION_EASY_MOUSE, GAMEOPTION_ANIMATED_INVENTORY, GAMEOPTION_ANIMATED_INTERFACE, GAMEOPTION_NAUGHTY_MODE, GAMEOPTION_TTS_NARRATOR, GAMEOPTION_COPY_PROTECTION, GAMEOPTION_ORIGINAL_MAC_MENUS)
#else
			GUIO9(GUIO_NOMIDI, GUIO_NOSPEECH, GUIO_NOASPECT, GAMEOPTION_EASY_MOUSE, GAMEOPTION_ANIMATED_INVENTORY, GAMEOPTION_ANIMATED_INTERFACE, GAMEOPTION_NAUGHTY_MODE, GAMEOPTION_COPY_PROTECTION, GAMEOPTION_ORIGINAL_MAC_MENUS)
#endif
		},
		GType_RexNebular,
		0
	},

	{
		// Rex Nebular and the Cosmic Gender Bender Bonus Disk
		{
			"nebularbonus",
			0,
			AD_ENTRY1s("section0.hag", "6aee8cef63774def1f9b33fef6262a47", 50710),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO2(GUIO_NOSPEECH, GUIO_NOLAUNCHLOAD)
		},
		GType_RexNebular,
		GF_BONUS_DISK
	},

	{
		// Return of the Phantom DOS English
		{
			"phantom",
			0,
			AD_ENTRY1s("global.hag", "bdce9ca93a015f0883d1bc0fabd0cdfa", 812150),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO3(GUIO_NOMIDI, GAMEOPTION_EASY_MOUSE, GAMEOPTION_ORIGINAL_SAVELOAD)
		},
		GType_Phantom,
		0
	},

	{
		// Return of the Phantom GOG DOS English
		{
			"phantom",
			"CD",
			AD_ENTRY1s("global.hag", "8a51c984eb4c64e8b30a7e6670f6bddb", 101154000),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE | ADGF_CD,
			GUIO3(GUIO_NOMIDI, GAMEOPTION_EASY_MOUSE, GAMEOPTION_ORIGINAL_SAVELOAD)
		},
		GType_Phantom,
		0
	},

	{
		// Return of the Phantom CD Installer
		{
			"phantom",
			"CD Installer",
			AD_ENTRY2s("mpslabs.001", "6f891664a0b09a00e45eaa3ee9b24669", 633998, "mpslabs.idx", "156a856072f4eb66b1f9b337358225e9", 4048),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE | ADGF_CD | GF_INSTALLER,
			GUIO3(GUIO_NOMIDI, GAMEOPTION_EASY_MOUSE, GAMEOPTION_ORIGINAL_SAVELOAD)
		},
		GType_Phantom,
		0
	},

	{
		// Return of the Phantom DOS English Demo
		{
			"phantom",
			"Demo",
			AD_ENTRY1s("section1.hag", "ad738c8edb139790ebaf78f8ceceb182", 123627),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE | ADGF_DEMO,
			GUIO3(GUIO_NOMIDI, GAMEOPTION_EASY_MOUSE, GAMEOPTION_ORIGINAL_SAVELOAD)
		},
		GType_Phantom,
		0
	},

	{
		// Dragonsphere DOS English CD
		{
			"dragonsphere",
			"CD",
			AD_ENTRY2s("section1.hag", "2770e441d296be5e806194693eebd95a", 2061199, "speech.idx", "80d36ba3ca88e9d078cc19a7678aeb87", 1250),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE | ADGF_CD,
			GUIO3(GUIO_NOMIDI, GAMEOPTION_EASY_MOUSE, GAMEOPTION_ORIGINAL_SAVELOAD)
		},
		GType_Dragonsphere,
		0
	},

	{
		// Dragonsphere DOS English
		{
			"dragonsphere",
			0,
			AD_ENTRY1s("section1.hag", "2770e441d296be5e806194693eebd95a", 2061199),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO3(GUIO_NOMIDI, GAMEOPTION_EASY_MOUSE, GAMEOPTION_ORIGINAL_SAVELOAD)
		},
		GType_Dragonsphere,
		0
	},

	{
		// Dragonsphere Microprose DOS English
		{
			"dragonsphere",
			"CD Installer",
			AD_ENTRY2s("mpslabs.001", "0d2143364be8e12b9807e111ffbe4fb5", 771220,
				"mpslabs.idx", "db604f6e665516c59bebe60b404abe44", 4636),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE | ADGF_CD | GF_INSTALLER,
			GUIO3(GUIO_NOMIDI, GAMEOPTION_EASY_MOUSE, GAMEOPTION_ORIGINAL_SAVELOAD)
		},
		GType_Dragonsphere,
		0
	},

	{
		// Dragonsphere DOS English demo
		{
			"dragonsphere",
			"Demo",
			AD_ENTRY1s("section1.hag", "9587b06eb4f9ee7ed5b19af4643743b0", 328083),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE | ADGF_DEMO,
			GUIO3(GUIO_NOMIDI, GAMEOPTION_EASY_MOUSE, GAMEOPTION_ORIGINAL_SAVELOAD)
		},
		GType_Dragonsphere,
		0
	},

	{
		// Once Upon a Forest DOS English
		{
			"forest",
			"",
			AD_ENTRY1s("section1.hag", "042518994daa7f96f9602e3b7e14a672", 816076),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO2(GUIO_NOSFX, GAMEOPTION_ORIGINAL_SAVELOAD)
		},
		GType_Forest,
		0
	},

	{ AD_TABLE_END_MARKER, 0, 0 }
};

} // namespace MADS
