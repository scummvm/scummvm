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

#ifndef DIRECTOR_DETECTION_TABLES_H
#define DIRECTOR_DETECTION_TABLES_H

namespace Director {

static const DirectorGameDescription gameDescriptions[] = {
	{
		{
			"directortest",
			"",
			AD_ENTRY1("lingotests.lingo", 0),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_TEST,
		3
	},

	{
		{
			"theapartment",
			"",
			AD_ENTRY1s("Main Menu", "9e838fe1a6af7992d656ca325e38dee5", 47911),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		3
	},

	{
		{
			"gundam0079",
			"",
			AD_ENTRY1("Gundam0079.exe", "1a7acbba10a7246ba58c1d53fc7203f5"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		5
	},

	{
		{
			"gundam0079",
			"",
			AD_ENTRY1("Gundam0079", "4c38a51a21a1ad231f218c4786ff771d"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		5
	},

	{
		{
			"jewels",
			"",
			AD_ENTRY1("JEWELS.EXE", "bb6d81471d166088260090472c6c3a87"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		4
	},

	{
		{
			"jewels",
			"",
			AD_ENTRY1("Jewels.exe", "c1a2e8b7e41fa204009324a9c7db1030"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		7
	},

	{
		{
			"jewels",
			"Two-Minute Demo",
			AD_ENTRY1("DEMO.EXE", "ebee52d3c4280674c600177df5b09da0"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		4
	},

	// Note: There are four versions of the binary included on the disc.
	// 5.6, 6, and 9 Meg variants all exist too.
	{
		{
			"jewels",
			"",
			AD_ENTRY1("Jewels 11 Meg", "339c89a148c4ff2c5c815c62ac006325"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		4
	},

	{
		{
			"jewels",
			"Two-Minute Demo",
			AD_ENTRY1("Two-Minute Demo", "01be45e7241194dad07938e7059b88e3"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK | ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		4
	},

	{
		{
			"jewels",
			"",
			AD_ENTRY1("Jewels of the Oracle", "fa52f0136cde568a46249ce74f01a324"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		7
	},

	{
		{
			"jewels",
			"Demo",
			AD_ENTRY1("JEWELS.EXE", "abcc448c035e88d4edb4a29034fd1e34"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS | ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		4
	},

	{
		{
			"jman",
			"",
			AD_ENTRY1s("JMAN.EXE", "7c8230a804abf9353b05627a675b5ffb", 375282),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		3
	},

	{
		{
			"jman",
			"",
			AD_ENTRY1s("JMDEMO.EXE", "7c8230a804abf9353b05627a675b5ffb", 375305),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		3
	},

	{
		{
			"jman",
			"",
			AD_ENTRY1("JOURNEY.EXE", "65d06b5fef155a2473434571aff5bc29"),
			Common::JA_JPN,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		3
	},

	{
		{
			"jman",
			"Turbo!",
			AD_ENTRY1("JMP Turbo\xE2\x84\xA2", "cc3321069072b90f091f220bba16e4d4"), // Trademark symbol (UTF-8)
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		4
	},

	{
		{
			"majestic",
			"",
			AD_ENTRY1("MAJESTIC.EXE", "624267f70253e5327981003a6fc0aeba"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		4
	},

	{
		{
			// Masters of the Elements - English (from rootfather)
			// Developed by IJsfontein, published by Tivola
			// File version of MVM.EXE is 6.0.2.32
			// The game disc is a hybrid CD-ROM containing both the Windows and the Macintosh release.

			"melements", "",
			{
				{"CHECK.DXR", 0, "c31ee30eebd24a8cf31691fc9926daa4", 901820},
				{"MVM.EXE", 0, 0, 2565921},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		5
	},

	{
		{
			// Masters of the Elements - English (from rootfather)

			"melements", "",
			{
				{"check.dxr", 0, "36f42340e819d1532c850880afe16581", 898206},
				{"Masters of the Elements", 0, 0, 1034962},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		5
	},

	{
		{
			// Masters of the Elements - German (from rootfather)
			// Released in Germany as "Meister Zufall und die Herrscher der Elemente"
			// Developed by IJsfontein, published by Tivola
			// File version of MVM.EXE is 6.0.2.32
			// The game disc is a hybrid CD-ROM containing both the Windows and the Macintosh release.

			"melements", "",
			{
				{"CHECK.DXR", 0, "d1cd0ed95b0e30597e0089bf3e5caf0f", 575414},
				{"MVM.EXE", 0, 0, 1512503},
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		5
	},

	{
		{
			// Masters of the Elements - German (from rootfather)

			"melements", "",
			{
				{"check.dxr", 0, "9c81934b7616ab077f44825b8afaa83e", 575426},
				{"Meister Zufall", 0, 0, 1034962},
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		5
	},

	{
		{
			"spyclub",
			"",
			AD_ENTRY1("SPYCLUB.EXE", "65d06b5fef155a2473434571aff5bc29"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		3
	},

	{
		{
			"amber",
			"",
			AD_ENTRY1("amber_jb.exe", "1a7acbba10a7246ba58c1d53fc7203f5"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		5
	},

	{
		{
			"vvvampire",
			"",
			AD_ENTRY1("VAMPIRE.EXE", "88f4f7406f34ec36e751a64f7c76f2c4"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		3
	},

	{
		{
			"vvvampire",
			"",
			AD_ENTRY1("The Vampire's Coffin", "d41d8cd98f00b204e9800998ecf8427e"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		3
	},

	{
		{
			"vvdinosaur",
			"",
			AD_ENTRY1("DINOSAUR.EXE", "4e6303630f4dd588e730d09241cf7e76"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		3
	},

	{
		{
			"vvdinosaur",
			"",
			AD_ENTRY1("Start Game", "d41d8cd98f00b204e9800998ecf8427e"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		3
	},

	{
		{
			"warlock",
			"",
			AD_ENTRY1s("Spaceship Warlock.bin", "cfa68a1bc49251497ebde18e5fc9c217", 271107),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		2
	},

	{
		{
			"warlock",
			"",
			AD_ENTRY1s("SSWARLCK.EXE", "65d06b5fef155a2473434571aff5bc29", 370867),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		2
	},

	{
		{
			"warlock",
			"",
			AD_ENTRY1s("SSWDEMO.EXE", "65d06b5fef155a2473434571aff5bc29", 370934),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		2
	},

	{ AD_TABLE_END_MARKER, GID_GENERIC, 0 }
};

} // End of Namespace Director

#endif
