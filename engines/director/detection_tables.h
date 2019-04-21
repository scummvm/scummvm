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

#ifndef DIRECTOR_DETECTION_TABLES_H
#define DIRECTOR_DETECTION_TABLES_H

namespace Director {

#define GENGAME_(t,e,f,m,s,l,p,fl,v) 	{ { t, e, AD_ENTRY1s(f, m, s), l, p, fl, GUIO1(GUIO_NOASPECT) }, GID_GENERIC, v }

#define MACGAME(t,e,f,m,s,v) 	GENGAME_(t,e,f,m,s,Common::EN_ANY,Common::kPlatformMacintosh,ADGF_MACRESFORK,v)
#define WINGAME(t,e,f,m,s,v) 	GENGAME_(t,e,f,m,s,Common::EN_ANY,Common::kPlatformWindows,ADGF_NO_FLAGS,v)
#define MACDEMO(t,e,f,m,s,v) 	GENGAME_(t,e,f,m,s,Common::EN_ANY,Common::kPlatformMacintosh,(ADGF_MACRESFORK|ADGF_DEMO),v)
#define WINDEMO(t,e,f,m,s,v) 	GENGAME_(t,e,f,m,s,Common::EN_ANY,Common::kPlatformWindows,ADGF_DEMO,v)

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

	// Generic D3 Mac entry
	MACGAME("director", "", "D3-mac", 0, -1, 3),
	// Generic D4 Mac entry
	MACGAME("director", "", "D4-mac", 0, -1, 4),

	MACGAME("theapartment", "D2", "Main Menu", "fc56c179cb8c6d4938e61ee61fd0032c", 48325, 2), // Original name is "•Main Menu"
	MACGAME("theapartment", "D3", "Main Menu", "9e838fe1a6af7992d656ca325e38dee5", 47911, 3), // Original name is "•Main Menu"
	MACGAME("theapartment", "D4", "Main Menu", "ff86181f03fe6eb060f65a985ca0580d", 160612, 4), // Original name is "•Main Menu"

	WINGAME("gundam0079", "", "Gundam0079.exe", "1a7acbba10a7246ba58c1d53fc7203f5", -1, 5),
	MACGAME("gundam0079", "", "Gundam0079", "4c38a51a21a1ad231f218c4786ff771d", -1, 5),

	WINGAME("jewels", "", "JEWELS.EXE", "bb6d81471d166088260090472c6c3a87", -1, 4),
	WINGAME("jewels", "", "Jewels.exe", "c1a2e8b7e41fa204009324a9c7db1030", -1, 7),
	WINDEMO("jewels", "Two-Minute Demo", "DEMO.EXE", "ebee52d3c4280674c600177df5b09da0", -1, 4),

	// Note: There are four versions of the binary included on the disc.
	// 5.6, 6, and 9 Meg variants all exist too.
	MACGAME("jewels", "", "Jewels 11 Meg", "339c89a148c4ff2c5c815c62ac006325", -1, 4),
	MACDEMO("jewels", "Two-Minute Demo", "Two-Minute Demo", "01be45e7241194dad07938e7059b88e3", -1, 4),

	MACGAME("jewels", "", "Jewels of the Oracle", "fa52f0136cde568a46249ce74f01a324", -1, 7),
	WINDEMO("jewels", "Demo", "JEWELS.EXE", "abcc448c035e88d4edb4a29034fd1e34", -1, 4),

	WINGAME("jman", "", "JMAN.EXE", "7c8230a804abf9353b05627a675b5ffb", 375282, 3),
	WINDEMO("jman", "", "JMDEMO.EXE", "7c8230a804abf9353b05627a675b5ffb", 375305, 3),

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

	MACGAME("jman", "Turbo!", "JMP Turbo\xE2\x84\xA2", "cc3321069072b90f091f220bba16e4d4", -1, 4), // Trademark symbol (UTF-8)

	WINGAME("majestic", "", "MAJESTIC.EXE", "624267f70253e5327981003a6fc0aeba", -1, 4),

	// Meet Mediaband
	WINGAME("mediaband", "", "MEDIABND.EXE", "0cfb9b4762e33ab56d656a0eb146a048", 717921, 4),

	{
		{
			// Masters of the Elements - English (from lotharsm)
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
			// Masters of the Elements - English (from lotharsm)

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
			// Masters of the Elements - German (from lotharsm)
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
			// Masters of the Elements - German (from lotharsm)

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

	WINGAME("spyclub", "", "SPYCLUB.EXE", "65d06b5fef155a2473434571aff5bc29", -1, 3),

	WINGAME("amber", "", "amber_jb.exe", "1a7acbba10a7246ba58c1d53fc7203f5", -1, 5),

	WINGAME("vvvampire", "", "VAMPIRE.EXE", "88f4f7406f34ec36e751a64f7c76f2c4", -1, 3),
	MACGAME("vvvampire", "", "The Vampire's Coffin", "d41d8cd98f00b204e9800998ecf8427e", -1, 3),

	WINGAME("vvdinosaur", "", "DINOSAUR.EXE", "4e6303630f4dd588e730d09241cf7e76", -1, 3),
	MACGAME("vvdinosaur", "", "Start Game", "d41d8cd98f00b204e9800998ecf8427e", -1, 3),

	MACGAME("warlock", "", "Spaceship Warlock", "cfa68a1bc49251497ebde18e5fc9c217", 271107, 2),
	WINGAME("warlock", "", "SSWARLCK.EXE", "65d06b5fef155a2473434571aff5bc29", 370867, 2),

	WINDEMO("warlock", "", "SSWDEMO.EXE", "65d06b5fef155a2473434571aff5bc29", 370934, 2),

	{
		{
			"ernie",
			"Demo",
			AD_ENTRY1s("ERNIE.EXE", "1a7acbba10a7246ba58c1d53fc7203f5", 1417371),
			Common::SE_SWE,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		5
	},

	{
		{
			"ernie",
			"",
			AD_ENTRY1s("Ernie.exe", "1a7acbba10a7246ba58c1d53fc7203f5", 1417481),
			Common::SE_SWE,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		5
	},

	{
		{
			"gadget",
			"Gadget: Past as Future",
			AD_ENTRY1s("GADGET.EXE", "d62438566e44826960fc16c5c23dbe43", 2212541),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS, //ADGF_HICOLOR,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		5
	},

	{ AD_TABLE_END_MARKER, GID_GENERIC, 0 }
};

} // End of Namespace Director

#endif
