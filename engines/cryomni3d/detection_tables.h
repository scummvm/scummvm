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

static const CryOmni3DGameDescription gameDescriptions[] = {
	// Versailles 1685
	// French Windows 95 from hybrid Win95/DOS CD
	// From lePhilousophe
	{
		{
			"versailles",
			"",
			AD_ENTRY1s("VERSAILL.EXE", "3775004b96f056716ce615b458b1f394", 372736),
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_TESTING,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_NUMERICFONTS | GF_VERSAILLES_AUDIOPADDING,
	},

	// Versailles 1685
	// French Windows 95 compressed from hybrid Win95/DOS CD
	// From lePhilousophe
	{
		{
			"versailles",
			"",
			AD_ENTRY1s("PROGRAM.Z", "a07b5d86af5f3a8883ba97db2bade87d", 293223),
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_TESTING,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_NUMERICFONTS | GF_VERSAILLES_AUDIOPADDING,
	},

	// Versailles 1685
	// French DOS from hybrid Win95/DOS CD
	// From lePhilousophe
	{
		{
			"versailles",
			"",
			AD_ENTRY1s("VERSAILL.PGM", "1c992f034f43418a5da2e8ebd0b92620", 630431),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_TESTING,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_NUMERICFONTS | GF_VERSAILLES_AUDIOPADDING,
	},

	// Versailles 1685
	// French DOS from hybrid Win95/DOS CD
	// From legloutondunet, ticket #11035
	{
		{
			"versailles",
			"",
			AD_ENTRY1s("VERSAILL.PGM", "1c992f034f43418a5da2e8ebd0b92620", 598767),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_TESTING,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_AUDIOPADDING,
	},

	// Versailles 1685
	// French Macintosh
	// From criezy
	{
		{
			"versailles",
			"",
			AD_ENTRY1s("Versailles", "f81935517b1bbb58acf70f25efa5c7f3", 375868),
			Common::FR_FRA,
			Common::kPlatformMacintosh,
			ADGF_TESTING,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_AUDIOPADDING,
	},

	// Versailles 1685
	// English Windows from Mac/Win hybrid CD
	// From lePhilousophe
	{
		{
			"versailles",
			"",
			AD_ENTRY1s("Versaill.exe", "5c3c10ec821b8d96016041ab649af8c7", 377856),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_TESTING,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_NUMERICFONTS,
	},

	// Versailles 1685
	// English Macintosh from Mac/Win hybrid CD
	// From lePhilousophe
	{
		{
			"versailles",
			"",
			AD_ENTRY1s("Versailles", "7fa3cb6a3c18f6b4ba6be85dcd433cff", 366199),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_TESTING,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_NUMERICFONTS,
	},

	// Versailles 1685
	// English DOS
	// From lePhilousophe
	{
		{
			"versailles",
			"",
			AD_ENTRY1s("VERSAILL.PGM", "1c992f034f43418a5da2e8ebd0b92620", 598639),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_TESTING,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_AUDIOPADDING,
	},

	// Versailles 1685
	// English Windows 95 compressed from hybrid Win95/DOS CD
	// From scoria in forum
	{
		{
			"versailles",
			"",
			AD_ENTRY1s("PROGRAM.Z", "f5327cb860a67a24a52b6125ddc5e00b", 256146),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_TESTING,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_NUMERICFONTS,
	},

	// Versailles 1685
	// English DOS from hybrid Win95/DOS CD
	// From scoria in forum
	{
		{
			"versailles",
			"",
			AD_ENTRY1s("VERSAILL.PGM", "1c992f034f43418a5da2e8ebd0b92620", 710467),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_TESTING,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_NUMERICFONTS,
	},

	// Versailles 1685
	// Italian DOS
	// From lePhilousophe
	{
		{
			"versailles",
			"",
			AD_ENTRY1s("VERSAILL.PGM", "1c992f034f43418a5da2e8ebd0b92620", 603023),
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_TESTING,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_AUDIOPADDING,
	},

	// Versailles 1685
	// German Macintosh
	// From lePhilousophe
	{
		{
			"versailles",
			"",
			AD_ENTRY1s("Versailles", "f1aa0603e7f71404f936e4189b4c5b2b", 348614),
			Common::DE_DEU,
			Common::kPlatformMacintosh,
			ADGF_TESTING,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_AUDIOPADDING,
	},

	// Versailles 1685
	// Spanish Windows 95 from hybrid Win95/DOS CD
	// From lePhilousophe
	{
		{
			"versailles",
			"",
			AD_ENTRY1s("VERSAILL.EXE", "78d90d656ec3b76f158721d38bc68083", 346112),
			Common::ES_ESP,
			Common::kPlatformWindows,
			ADGF_TESTING,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_NUMERICFONTS | GF_VERSAILLES_AUDIOPADDING,
	},

	// Versailles 1685
	// Spanish Windows 95 compressed from hybrid Win95/DOS CD
	// From lePhilousophe
	{
		{
			"versailles",
			"",
			AD_ENTRY1s("PROGRAM.Z", "9e7c0c3125124010d45dde9dc62744ef", 237800),
			Common::ES_ESP,
			Common::kPlatformWindows,
			ADGF_TESTING,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_NUMERICFONTS | GF_VERSAILLES_AUDIOPADDING,
	},

	// Versailles 1685
	// Spanish DOS from hybrid Win95/DOS CD
	// From lePhilousophe
	{
		{
			"versailles",
			"",
			AD_ENTRY1s("VERSAILL.PGM", "1c992f034f43418a5da2e8ebd0b92620", 715887),
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_TESTING,
			GUI_OPTIONS_VERSAILLES
		},
		GType_VERSAILLES,
		GF_VERSAILLES_NUMERICFONTS | GF_VERSAILLES_AUDIOPADDING,
	},

	{ AD_TABLE_END_MARKER, 0, 0 }
};

//////////////////////////////
//Fallback detection
//////////////////////////////

static const CryOmni3DGameDescription fallbackDescs[] = {
	{
		{
			"",
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
	{ 0, { 0 } }
};

} // End of Namespace CryOmni3D
