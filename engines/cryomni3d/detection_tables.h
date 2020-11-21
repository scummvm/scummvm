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

namespace CryOmni3D {

#define GUI_OPTIONS_VERSAILLES                   GUIO3(GUIO_NOMIDI, GUIO_NOSFX, GUIO_NOASPECT)

// To correctly detect root we need files from various places: CD1, CD2, HDD, on-CD install files
// We use files common to all installations except the documentation links and the binary
// We only check the file presence to simplify and use program to discriminate the version
#define VERSAILLES_ENTRY(f, x, s, lien_doc_ext) { \
    { "11D_LEB1.HNM", 0, nullptr, -1}, \
    { "COFBOUM.HNM", 0, nullptr, -1}, \
    { "lien_doc." lien_doc_ext, 0, nullptr, -1}, \
    { f, 0, x, s}, \
    AD_LISTEND}

#define VERSAILLES_ENTRY_DEF(f, x, s) VERSAILLES_ENTRY(f, x, s, "txt")


// To add new entries, you should check which fonts are loaded by the binary by looking at strings in it
// and you should check if audio files have underscores to pad to 8.3 format
// The simplest is to request a "tree /f" or "find ." and check which files are present
// From experience, numeric should be used when available
static const CryOmni3DGameDescription gameDescriptions[] = {
	// Versailles 1685
	// French Windows 95 from hybrid Win95/DOS CD
	// From lePhilousophe
	{
		{
			"versailles",
			"",
			VERSAILLES_ENTRY_DEF("VERSAILL.EXE", "3775004b96f056716ce615b458b1f394", 372736),
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_FONTS_NUMERIC | GF_VERSAILLES_AUDIOPADDING_YES | GF_VERSAILLES_LINK_STANDARD,
	},

	// Versailles 1685
	// French Windows 95 compressed from hybrid Win95/DOS CD
	// From lePhilousophe
	{
		{
			"versailles",
			"",
			VERSAILLES_ENTRY_DEF("PROGRAM.Z", "a07b5d86af5f3a8883ba97db2bade87d", 293223),
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_FONTS_NUMERIC | GF_VERSAILLES_AUDIOPADDING_YES | GF_VERSAILLES_LINK_STANDARD,
	},

	// Versailles 1685
	// French DOS from hybrid Win95/DOS CD
	// From lePhilousophe
	{
		{
			"versailles",
			"",
			VERSAILLES_ENTRY_DEF("VERSAILL.PGM", "1c992f034f43418a5da2e8ebd0b92620", 630431),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_FONTS_NUMERIC | GF_VERSAILLES_AUDIOPADDING_YES | GF_VERSAILLES_LINK_STANDARD,
	},

	// Versailles 1685
	// French DOS from hybrid Win95/DOS CD
	// From legloutondunet, ticket #11035
	{
		{
			"versailles",
			"",
			VERSAILLES_ENTRY_DEF("VERSAILL.PGM", "1c992f034f43418a5da2e8ebd0b92620", 598767),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_FONTS_SET_B | GF_VERSAILLES_AUDIOPADDING_YES | GF_VERSAILLES_LINK_STANDARD,
	},

	// Versailles 1685
	// French Windows from DVD
	// From Uka in forum
	{
		{
			"versailles",
			"",
			VERSAILLES_ENTRY_DEF("Versaill.exe", "09b4734ce473e4cb78738677ce39f536", 346624),
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_FONTS_NUMERIC | GF_VERSAILLES_AUDIOPADDING_YES | GF_VERSAILLES_LINK_STANDARD,
	},

	// Versailles 1685
	// French Windows compressed from DVD
	// From Uka in forum
	{
		{
			"versailles",
			"",
			VERSAILLES_ENTRY_DEF("PROGRAM.Z", "167ac4d6e60856ee84d7369107d858d4", 230056),
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_FONTS_NUMERIC | GF_VERSAILLES_AUDIOPADDING_YES | GF_VERSAILLES_LINK_STANDARD,
	},

	// Versailles 1685
	// French Macintosh
	// From criezy
	{
		{
			"versailles",
			"",
			VERSAILLES_ENTRY_DEF("Versailles", "f81935517b1bbb58acf70f25efa5c7f3", 375868),
			Common::FR_FRA,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_FONTS_SET_A | GF_VERSAILLES_AUDIOPADDING_YES | GF_VERSAILLES_LINK_STANDARD,
	},

	// Versailles 1685
	// English Windows from Mac/Win hybrid CD
	// From lePhilousophe
	{
		{
			"versailles",
			"",
			VERSAILLES_ENTRY_DEF("Versaill.exe", "5c3c10ec821b8d96016041ab649af8c7", 377856),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_FONTS_NUMERIC | GF_VERSAILLES_AUDIOPADDING_NO | GF_VERSAILLES_LINK_STANDARD,
	},

	// Versailles 1685
	// English Macintosh from Mac/Win hybrid CD
	// From lePhilousophe
	{
		{
			"versailles",
			"",
			VERSAILLES_ENTRY_DEF("Versailles", "7fa3cb6a3c18f6b4ba6be85dcd433cff", 366199),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_FONTS_NUMERIC | GF_VERSAILLES_AUDIOPADDING_NO | GF_VERSAILLES_LINK_STANDARD,
	},

	// Versailles 1685
	// English DOS
	// From lePhilousophe
	{
		{
			"versailles",
			"",
			VERSAILLES_ENTRY_DEF("VERSAILL.PGM", "1c992f034f43418a5da2e8ebd0b92620", 598639),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_FONTS_SET_B | GF_VERSAILLES_AUDIOPADDING_YES | GF_VERSAILLES_LINK_STANDARD,
	},

	// Versailles 1685
	// English Windows 95 compressed from hybrid Win95/DOS CD
	// From scoria in forum
	{
		{
			"versailles",
			"",
			VERSAILLES_ENTRY_DEF("PROGRAM.Z", "f5327cb860a67a24a52b6125ddc5e00b", 256146),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_FONTS_NUMERIC | GF_VERSAILLES_AUDIOPADDING_NO | GF_VERSAILLES_LINK_STANDARD,
	},

	// Versailles 1685
	// English DOS from hybrid Win95/DOS CD
	// From scoria in forum
	{
		{
			"versailles",
			"",
			VERSAILLES_ENTRY_DEF("VERSAILL.PGM", "1c992f034f43418a5da2e8ebd0b92620", 710467),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_FONTS_NUMERIC | GF_VERSAILLES_AUDIOPADDING_NO | GF_VERSAILLES_LINK_STANDARD,
	},

	// Versailles 1685
	// English Windows compressed from DVD
	// From Uka in forum
	{
		{
			"versailles",
			"",
			VERSAILLES_ENTRY_DEF("PROGRAM.Z", "daeeb8bce80fe74fe28ecc22b6a97f83", 237679),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_FONTS_NUMERIC | GF_VERSAILLES_AUDIOPADDING_YES | GF_VERSAILLES_LINK_STANDARD,
	},

	// Versailles 1685
	// Italian DOS
	// From lePhilousophe
	{
		{
			"versailles",
			"",
			VERSAILLES_ENTRY_DEF("VERSAILL.PGM", "1c992f034f43418a5da2e8ebd0b92620", 603023),
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_FONTS_SET_C | GF_VERSAILLES_AUDIOPADDING_YES | GF_VERSAILLES_LINK_STANDARD,
	},

	// Versailles 1685
	// Italian Windows compressed from DVD
	// From Uka in forum
	{
		{
			"versailles",
			"",
			VERSAILLES_ENTRY_DEF("PROGRAM.Z", "8b0dcf71a7eb21b8378add8b16857bae", 237878),
			Common::IT_ITA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_FONTS_SET_C | GF_VERSAILLES_AUDIOPADDING_YES | GF_VERSAILLES_LINK_STANDARD,
	},

	// Versailles 1685
	// German Macintosh
	// From lePhilousophe
	{
		{
			"versailles",
			"",
			VERSAILLES_ENTRY("Versailles", "f1aa0603e7f71404f936e4189b4c5b2b", 348614, "ALM"),
			Common::DE_DEU,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_FONTS_SET_B | GF_VERSAILLES_AUDIOPADDING_YES | GF_VERSAILLES_LINK_LOCALIZED,
	},

	// Versailles 1685
	// German Windows compressed from DVD
	// From laenion, ticket #11963
	{
		{
			"versailles",
			"",
			VERSAILLES_ENTRY("PROGRAM.Z", "cdd35a623d1ed05d1dc3248735cea868", 236363, "ALM"),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_FONTS_NUMERIC | GF_VERSAILLES_AUDIOPADDING_YES | GF_VERSAILLES_LINK_LOCALIZED,
	},

	// Versailles 1685
	// German Windows compressed from DVD
	// From Uka in forum
	{
		{
			"versailles",
			"",
			VERSAILLES_ENTRY_DEF("PROGRAM.Z", "2e1a40237f8b28cb6ef29cff137fa561", 238041),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_FONTS_NUMERIC | GF_VERSAILLES_AUDIOPADDING_YES | GF_VERSAILLES_LINK_STANDARD,
	},

	// Versailles 1685
	// Spanish Windows 95 from hybrid Win95/DOS CD
	// From lePhilousophe
	{
		{
			"versailles",
			"",
			VERSAILLES_ENTRY_DEF("VERSAILL.EXE", "78d90d656ec3b76f158721d38bc68083", 346112),
			Common::ES_ESP,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_FONTS_NUMERIC | GF_VERSAILLES_AUDIOPADDING_YES | GF_VERSAILLES_LINK_STANDARD,
	},

	// Versailles 1685
	// Spanish Windows 95 compressed from hybrid Win95/DOS CD
	// From lePhilousophe
	{
		{
			"versailles",
			"",
			VERSAILLES_ENTRY_DEF("PROGRAM.Z", "9e7c0c3125124010d45dde9dc62744ef", 237800),
			Common::ES_ESP,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_FONTS_NUMERIC | GF_VERSAILLES_AUDIOPADDING_YES | GF_VERSAILLES_LINK_STANDARD,
	},

	// Versailles 1685
	// Spanish DOS from hybrid Win95/DOS CD
	// From lePhilousophe
	{
		{
			"versailles",
			"",
			VERSAILLES_ENTRY_DEF("VERSAILL.PGM", "1c992f034f43418a5da2e8ebd0b92620", 715887),
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_FONTS_NUMERIC | GF_VERSAILLES_AUDIOPADDING_YES | GF_VERSAILLES_LINK_STANDARD,
	},

	// Versailles 1685
	// Brazilian Windows from DVD
	// From Uka in forum
	{
		{
			"versailles",
			"",
			VERSAILLES_ENTRY_DEF("Versaill.exe", "49aa4581b8c652aa88c633b6c5fe84ea", 346112),
			Common::PT_BRA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_FONTS_NUMERIC | GF_VERSAILLES_AUDIOPADDING_YES | GF_VERSAILLES_LINK_STANDARD,
	},

	// Versailles 1685
	// Brazilian Windows compressed from DVD
	// From Uka in forum
	{
		{
			"versailles",
			"",
			VERSAILLES_ENTRY_DEF("PROGRAM.Z", "013eabf30fdec7bb7302a5312d094c64", 237952),
			Common::PT_BRA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_FONTS_NUMERIC | GF_VERSAILLES_AUDIOPADDING_YES | GF_VERSAILLES_LINK_STANDARD,
	},

	// Versailles 1685
	// Chinese Windows compressed from DVD
	// From lePhilousophe
	{
		{
			"versailles",
			"",
			VERSAILLES_ENTRY_DEF("PROGRAM.Z", "37f3d691e90e17b78050d6a91f7e0377", 242583),
			Common::ZH_TWN,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		/* GF_VERSAILLES_FONTS_ | */ GF_VERSAILLES_AUDIOPADDING_YES,
	},

	// Versailles 1685
	// Chinese Windows from DVD
	// From lePhilousophe
	{
		{
			"versailles",
			"",
			VERSAILLES_ENTRY_DEF("VERSAILL.EXE", "5209e7c9b20612467af7e9745758ee72", 352256),
			Common::ZH_TWN,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		/* GF_VERSAILLES_FONTS_ | */ GF_VERSAILLES_AUDIOPADDING_YES,
	},

	// Versailles 1685
	// Korean Windows compressed from DVD
	// From lePhilousophe
	{
		{
			"versailles",
			"",
			VERSAILLES_ENTRY_DEF("PROGRAM.Z", "ffb7599d042fb71f22ab4d76a2e0147f", 244155),
			Common::KO_KOR,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		/* GF_VERSAILLES_FONTS_ | */ GF_VERSAILLES_AUDIOPADDING_YES,
	},

	// Versailles 1685
	// Korean Windows from DVD
	// From lePhilousophe
	{
		{
			"versailles",
			"",
			VERSAILLES_ENTRY_DEF("VERSAILL.EXE", "549c588e05df8d42b531ffc9b2796303", 355840),
			Common::KO_KOR,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		/* GF_VERSAILLES_FONTS_ | */ GF_VERSAILLES_AUDIOPADDING_YES,
	},

	// Versailles 1685
	// Japanese Windows compressed from DVD
	// From lePhilousophe
	{
		{
			"versailles",
			"",
			VERSAILLES_ENTRY_DEF("PROGRAM.Z", "1944d7c30dbb25ab10f684422e196c16", 248700),
			Common::JA_JPN,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		/* GF_VERSAILLES_FONTS_ | */ GF_VERSAILLES_AUDIOPADDING_YES,
	},

	// Versailles 1685
	// Japanese Windows from DVD
	// From lePhilousophe
	{
		{
			"versailles",
			"",
			VERSAILLES_ENTRY_DEF("VERSAILL.EXE", "b7dadaf14cc5783e235125f9d6f6adea", 358912),
			Common::JA_JPN,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		/* GF_VERSAILLES_FONTS_ | */ GF_VERSAILLES_AUDIOPADDING_YES,
	},

	{ AD_TABLE_END_MARKER, 0, 0 }
};

static const char *directoryGlobs[] = {
	/** Versailles 1685 **/
	/* DATAS_V/ANIMACTI/LEVEL1/11D_LEB1.HNM
	 * DATAS_V/ANIMACTI/LEVEL7/COFBOUM.HNM */
	"DATAS_V",
	/* When user doesn't want to overwrite */
	"DATAS_V1",
	"DATAS_V2",
	"ANIMACTI",
	"LEVEL1",
	"LEVEL7",
	/* PC Setup */
	/* INSTALL/DATA/TEXTES/LIEN_DOC.TXT
	 * INSTALL/WIN/PROGRAM.Z
	 * INSTALL/DOS/VERSAILL.PGM */
	"INSTALL",
	"DATA",
	"WIN",
	"DOS",
	/* Mac Setup */
	/* Versailles Folder/DATAV_HD/TEXTES/LIEN_DOC.TXT
	 * Versailles Folder/Versailles */
	"*Versailles*",
	"DATAV_HD",
	/* lien_doc.* */
	"TEXTES",

	/** End of list **/
	nullptr
};

//////////////////////////////
//Fallback detection
//////////////////////////////

static const CryOmni3DGameDescription fallbackDescs[] = {
	{
		{
			"versailles",
			"",
			AD_ENTRY1(0, 0),
			Common::UNK_LANG,
			Common::kPlatformUnknown,
			ADGF_UNSTABLE,
			GUIO0()
		},
		0,
		0,
	},
};

static const ADFileBasedFallback fileBased[] = {
	{ &fallbackDescs[0].desc,  { "11D_LEB1.HNM", "COFBOUM.HNM", "VERSAILL.PGM", 0 } },
	{ &fallbackDescs[0].desc,  { "11D_LEB1.HNM", "COFBOUM.HNM", "PROGRAM.Z", 0 } },
	{ &fallbackDescs[0].desc,  { "11D_LEB1.HNM", "COFBOUM.HNM", "VERSAILL.EXE", 0 } },
	{ &fallbackDescs[0].desc,  { "11D_LEB1.HNM", "COFBOUM.HNM", "Versailles", 0 } },
	{ 0, { 0 } }
};

} // End of namespace CryOmni3D
