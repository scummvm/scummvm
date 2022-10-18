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

#ifndef MADE_DETECTION_TABLES_H
#define MADE_DETECTION_TABLES_H

#include "engines/advancedDetector.h"
#include "common/translation.h"

#define GAMEOPTION_INTRO_MUSIC_DIGITAL GUIO_GAMEOPTIONS1

namespace Made {

static const MadeGameDescription gameDescriptions[] = {

	{
		// NOTE: Return to Zork entries with *.dat are used to detect the game via rtzcd.dat,
		// which is packed inside rtzcd.red. Entries with *.red refer to the packed file
		// directly, which is the "official" way.

		// Return to Zork - English CD version 1.0 9/15/93 (installed)
		// Ticket #8858 submitted by spookypeanut
		{
			"rtz",
			"V1.0, 9/15/93, installed, CD",
			AD_ENTRY1("rtzcd.dat", "e95c38ded389e39cfbf87a8cb250b12e"),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GAMEOPTION_INTRO_MUSIC_DIGITAL)
		},
		GID_RTZ,
		0,
		GF_CD,
		3,
	},

	{
		// Return to Zork - English CD version 1.0 9/15/93
		// Ticket #8858 submitted by spookypeanut
		{
			"rtz",
			"V1.0, 9/15/93, CD",
			AD_ENTRY1("rtzcd.red", "cd8b62ece4677c438688c1de3f5379b9"),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO1(GAMEOPTION_INTRO_MUSIC_DIGITAL)
		},
		GID_RTZ,
		0,
		GF_CD_COMPRESSED,
		3,
	},

	{
		// Return to Zork - English CD version 1.1 12/7/93 (installed)
		{
			"rtz",
			"V1.1, 12/7/93, installed, CD",
			AD_ENTRY1s("rtzcd.dat", "a1db8c97a78dae10f91d356f16ad07b8", 536064),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO1(GAMEOPTION_INTRO_MUSIC_DIGITAL)
		},
		GID_RTZ,
		0,
		GF_CD,
		3,
	},

	{
		// Return to Zork - English OEM CD version 1.1 12/7/93
		{
			"rtz",
			"V1.1, 12/7/93, CD",
			AD_ENTRY1s("rtzcd.red", "c4e2430e6b6c6ff1562a80fb4a9df24c", 276177),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO1(GAMEOPTION_INTRO_MUSIC_DIGITAL)
		},
		GID_RTZ,
		0,
		GF_CD_COMPRESSED,
		3,
	},

	{
		// Return to Zork - English Retail CD version 1.1 12/7/93
		{
			"rtz",
			"V1.1, 12/7/93, CD",
			AD_ENTRY1s("rtzcd.red", "c4e2430e6b6c6ff1562a80fb4a9df24c", 276466),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO1(GAMEOPTION_INTRO_MUSIC_DIGITAL)
		},
		GID_RTZ,
		0,
		GF_CD_COMPRESSED,
		3,
	},

	{
		// Return to Zork - English CD version 1.2 9/29/94 (installed)
		// Supplied by Dark-Star in the ScummVM forums
		{
			"rtz",
			"V1.2, 9/29/94, installed, CD",
			AD_ENTRY1("rtzcd.dat", "9d740378da2d16e83d0d0efff01bf83a"),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO1(GAMEOPTION_INTRO_MUSIC_DIGITAL)
		},
		GID_RTZ,
		0,
		GF_CD,
		3,
	},

	{
		// Return to Zork - English CD version 1.2 9/29/94
		{
			"rtz",
			"V1.2, 9/29/94, CD",
			{
				{ "rtzcd.red", 0, "946997d8b0aa6cb4e848bad02a1fc3d2", 276584 },
				{ "rtzcd.prj", 0, "974d74410c3c29d50e857863e8bf40e2", 43016792 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO1(GAMEOPTION_INTRO_MUSIC_DIGITAL)
		},
		GID_RTZ,
		0,
		GF_CD_COMPRESSED,
		3,
	},

	{
		// Return to Zork - German CD version 1.2 9/29/94 (installed)
		// Supplied by Dark-Star in the ScummVM forums
		{
			"rtz",
			"V1.2, 9/29/94, installed, CD",
			AD_ENTRY1s("rtzcd.dat", "9d740378da2d16e83d0d0efff01bf83a", 525824),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO1(GAMEOPTION_INTRO_MUSIC_DIGITAL)
		},
		GID_RTZ,
		0,
		GF_CD,
		3,
	},

	{
		// Return to Zork - German CD version 1.2 4/18/95
		// Supplied by Dark-Star in the ScummVM forums
		{
			"rtz",
			"V1.2, 4/18/95, CD",
			AD_ENTRY1s("rtzcd.red", "946997d8b0aa6cb4e848bad02a1fc3d2", 355442),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO1(GAMEOPTION_INTRO_MUSIC_DIGITAL)
		},
		GID_RTZ,
		0,
		GF_CD_COMPRESSED,
		3,
	},

	{
		// Return to Zork - Italian CD version 1.2 3/31/95 (installed)
		// Patch #4225 submitted by goodoldgeorg
		{
			"rtz",
			"V1.2, 3/31/95, installed, CD",
			AD_ENTRY1s("rtzcd.dat", "5b86035aed0277f96e3d173542b5364a", 523776),
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO1(GAMEOPTION_INTRO_MUSIC_DIGITAL)
		},
		GID_RTZ,
		0,
		GF_CD,
		3,
	},

	{
		// Return to Zork - Italian CD version 1.2 3/31/95
		// Patch #4225 submitted by goodoldgeorg
		{
			"rtz",
			"V1.2, 3/31/95, CD",
			AD_ENTRY1s("rtzcd.red", "946997d8b0aa6cb4e848bad02a1fc3d2", 354971),
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO1(GAMEOPTION_INTRO_MUSIC_DIGITAL)
		},
		GID_RTZ,
		0,
		GF_CD_COMPRESSED,
		3,
	},

	{
		// Return to Zork - French CD version 1.2 5/13/95 (installed)
		// Patch #4225 submitted by goodoldgeorg
		{
			"rtz",
			"V1.2, 5/13/95, installed, CD",
			AD_ENTRY1s("rtzcd.dat", "bde8251a8e34e87c54e3f93147d56c9e", 523776),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO1(GAMEOPTION_INTRO_MUSIC_DIGITAL)
		},
		GID_RTZ,
		0,
		GF_CD,
		3,
	},

	{
		// Return to Zork - French CD version 1.2 5/13/95
		// Patch #4225 submitted by goodoldgeorg
		{
			"rtz",
			"V1.2, 3/31/95, CD",
			AD_ENTRY1s("rtzcd.red", "946997d8b0aa6cb4e848bad02a1fc3d2", 354614),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO1(GAMEOPTION_INTRO_MUSIC_DIGITAL)
		},
		GID_RTZ,
		0,
		GF_CD_COMPRESSED,
		3,
	},

	{
		// Return to Zork - Korean CD version 1.2 9/29/94
		// Dub only. No text was translated, even in menus, so there are no font issues.
		// submitted by trembyle
		{
			"rtz",
			"V1.2, 9/29/94, CD",
			{
				{ "rtzcd.red", 0, "946997d8b0aa6cb4e848bad02a1fc3d2", 276584 },
				{ "rtzcd.prj", 0, "3c8644f7ce77b74968637c035c3532d8", 48083511 },
				AD_LISTEND
			},
			Common::KO_KOR,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO1(GAMEOPTION_INTRO_MUSIC_DIGITAL)
		},
		GID_RTZ,
		0,
		GF_CD_COMPRESSED,
		3,
	},

	{
		// Return to Zork - English floppy version
		{
			"rtz",
			"Floppy",
			AD_ENTRY1("rtz.prj", "764d02f52ce1c219f2c0066677fba4ce"),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_RTZ,
		0,
		GF_FLOPPY,
		3,
	},

	{
		// Return to Zork - Demo
		{
			"rtz",
			"Demo",
			AD_ENTRY1("demo.dat", "2a6a1354bd5346fad4aee08e5b56caaa"),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DEMO,
			GUIO0()
		},
		GID_RTZ,
		0,
		GF_DEMO,
		3,
	},

	{
		// Return to Zork - Standalone CD Demo v1.1
		{
			"rtz",
			"V1.1, 12/6/93, Demo CD",
			AD_ENTRY1s("rtzcd.red", "827cfb323eae37b385985a2359fae3e9", 133784),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DEMO | ADGF_CD,
			GUIO1(GAMEOPTION_INTRO_MUSIC_DIGITAL)
		},
		GID_RTZ,
		0,
		GF_CD_COMPRESSED,
		3,
	},

	{
		// Return to Zork - Demo from Zork Anthology CD
		// Bugreport #11202
		{
			"rtz",
			"V1.2, 9/8/94, Demo CD",
			AD_ENTRY1s("rtzcd.red", "946997d8b0aa6cb4e848bad02a1fc3d2", 130683),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_DEMO | ADGF_CD,
			GUIO1(GAMEOPTION_INTRO_MUSIC_DIGITAL)
		},
		GID_RTZ,
		0,
		GF_CD_COMPRESSED,
		3,
	},

	{
		// Return to Zork - Mac Demo from Zork Anthology CD
		// Same disc as DOS version (on ISO-9660)
		// The only resource fork is in the executable
		{
			"rtz",
			"V1.2, 5/4/94, Demo CD",
			AD_ENTRY1s("Return To Zork", "0c1377afd4b6fc4ee900e1882ac13895", 1714064),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_DEMO | ADGF_CD | ADGF_MACRESFORK,
			GUIO1(GAMEOPTION_INTRO_MUSIC_DIGITAL)
		},
		GID_RTZ,
		0,
		GF_CD_COMPRESSED,
		3,
	},

	{
		// Return to Zork - Japanese DOS
		// This is the RTZCD.DAT in the base directory of the FM-Towns CD
		{
			"rtz",
			"",
			AD_ENTRY1("rtzcd.dat", "c4fccf67ad247f09b94c3c808b138576"),
			Common::JA_JPN,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO0()
		},
		GID_RTZ,
		0,
		GF_CD,
		3,
	},

	{
		// Return to Zork - Japanese FM-Towns
		// This is in the RTZFM folder of the FM-Towns CD
		{
			"rtz",
			"",
			AD_ENTRY1("rtzcd.dat", "e949a6a42d82daabfa7d4dc0a87a9843"),
			Common::JA_JPN,
			Common::kPlatformFMTowns,
			ADGF_CD,
			GUIO1(GUIO_NOASPECT)
		},
		GID_RTZ,
		0,
		GF_CD,
		3,
	},

	{
		// Return to Zork - Japanese PC-98
		// This is in the RTZ9821 folder of the FM-Towns CD
		{
			"rtz",
			"",
			AD_ENTRY1("rtzcd.dat", "0c0117e98530c736a141c2aad6834dc5"),
			Common::JA_JPN,
			Common::kPlatformPC98,
			ADGF_CD,
			GUIO0()
		},
		GID_RTZ,
		0,
		GF_CD,
		3,
	},

	// The Manhole: Masterpiece Edition is not a MADE engine and cannot be
	// supported by MADE. It is a HyperCard-like engine
	{
		// The Manhole: Masterpiece Edition (GOG/CD)
		{
			"manhole",
			_s("The game is using unsupported engine"),
			AD_ENTRY1("manhole.dat", "e8cec9bf21e4c50a7ebc193a4e0b48f5"),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSUPPORTED,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_MANHOLE,
		0,
		GF_CD,
		2,
	},

	// Bugreport #5855
	{
		{
			"manhole",
			_s("The game is using unsupported engine"),
			AD_ENTRY1s("manhole.dat", "df77ad5232757d7149342fb6471de4ed", 99317),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSUPPORTED,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_MANHOLE,
		0,
		GF_CD,
		2,
	},

	{
		// The Manhole: New and Enhanced
		{
			"manhole",
			"",
			AD_ENTRY1("manhole.dat", "cb21e31ed35c963208343bc995225b73"),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_MANHOLE,
		0,
		GF_CD,
		2,
	},

	{
		// The Manhole (EGA, 5.25")
		{
			"manhole",
			"EGA",
			AD_ENTRY1("manhole.dat", "2b1658292599a861c4cd3cf6cdb3c581"),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_MANHOLE,
		0,
		GF_FLOPPY,
		1,
	},

	{
		// Leather Goddesses of Phobos 2 (English)
		{
			"lgop2",
			"",
			AD_ENTRY1("lgop2.dat", "8137996db200ff67e8f172ff106f2e48"),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_LGOP2,
		0,
		GF_FLOPPY,
		2,
	},

	{
		// Leather Goddesses of Phobos 2 (German)
		// Supplied by windlepoons (bug tracker #4218)
		{
			"lgop2",
			"",
			AD_ENTRY1s("lgop2.dat", "a0ffea6a3b7e39bd861edd00c397641c", 299466),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_LGOP2,
		0,
		GF_FLOPPY,
		2,
	},

	{
		// Leather Goddesses of Phobos 2 (French)
		// Supplied by goodoldgeorg (bug tracker #4219)
		{
			"lgop2",
			"",
			AD_ENTRY1s("lgop2.dat", "f9e974087af7cf4b7ec2d8dc45d01e0c", 295366),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_LGOP2,
		0,
		GF_FLOPPY,
		2,
	},

	{
		// Leather Goddesses of Phobos 2 (Spanish)
		// Supplied by goodoldgeorg (bug tracker #4219)
		{
			"lgop2",
			"",
			AD_ENTRY1s("lgop2.dat", "96eb95b4d75b9a3da0b0d67e3b4a787d", 288984),
			Common::ES_ESP,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_LGOP2,
		0,
		GF_FLOPPY,
		2,
	},

	{
		// Rodney's Funscreen
		{
			"rodney",
			"",
			AD_ENTRY1("rodneys.dat", "a79887dbaa47689facd7c6f09258ba5a"),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOSPEECH)
		},
		GID_RODNEY,
		0,
		GF_FLOPPY,
		2,
	},

	{ AD_TABLE_END_MARKER, 0, 0, 0, 0 }
};

/**
 * The fallback game descriptor used by the Made engine's fallbackDetector.
 * Contents of this struct are to be overwritten by the fallbackDetector.
 */
static MadeGameDescription g_fallbackDesc = {
	{
		"",
		"",
		AD_ENTRY1(0, 0), // This should always be AD_ENTRY1(0, 0) in the fallback descriptor
		Common::UNK_LANG,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	0,
	0,
	0,
	0,
};

} // End of namespace Made

#endif
