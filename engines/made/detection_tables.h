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

#ifndef MADE_DETECTION_TABLES_H
#define MADE_DETECTION_TABLES_H

#include "engines/advancedDetector.h"

namespace Made {

struct MadeGameDescription {
	ADGameDescription desc;

	int gameID;
	int gameType;
	uint32 features;
	uint16 version;
};

static const MadeGameDescription gameDescriptions[] = {
	{
		// NOTE: Return to Zork entries with *.dat are used to detect the game via rtzcd.dat,
		// which is packed inside rtzcd.red. Entries with *.red refer to the packed file
		// directly, which is the "official" way.

		// Return to Zork - English CD version 1.0 9/15/93 (installed)
		// Patch #1953654 submitted by spookypeanut
		{
			"rtz",
			"V1.0, 9/15/93, installed, CD",
			AD_ENTRY1("rtzcd.dat", "e95c38ded389e39cfbf87a8cb250b12e"),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GID_RTZ,
		0,
		GF_CD,
		3,
	},

	{
		// Return to Zork - English CD version 1.0 9/15/93
		// Patch #1953654 submitted by spookypeanut
		{
			"rtz",
			"V1.0, 9/15/93, CD",
			AD_ENTRY1("rtzcd.red", "cd8b62ece4677c438688c1de3f5379b9"),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO0()
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
			GUIO0()
		},
		GID_RTZ,
		0,
		GF_CD,
		3,
	},

	{
		// Return to Zork - English CD version 1.1 12/7/93
		{
			"rtz",
			"V1.1, 12/7/93, CD",
			AD_ENTRY1s("rtzcd.red", "c4e2430e6b6c6ff1562a80fb4a9df24c", 276177),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO0()
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
			GUIO0()
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
			AD_ENTRY1s("rtzcd.red", "946997d8b0aa6cb4e848bad02a1fc3d2", 276584),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO0()
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
			GUIO0()
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
			GUIO0()
		},
		GID_RTZ,
		0,
		GF_CD_COMPRESSED,
		3,
	},

	{
		// Return to Zork - Italian CD version 1.2 3/31/95 (installed)
		// Patch #2685032 submitted by goodoldgeorg
		{
			"rtz",
			"V1.2, 3/31/95, installed, CD",
			AD_ENTRY1s("rtzcd.dat", "5b86035aed0277f96e3d173542b5364a", 523776),
			Common::IT_ITA,
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
		// Return to Zork - Italian CD version 1.2 3/31/95
		// Patch #2685032 submitted by goodoldgeorg
		{
			"rtz",
			"V1.2, 3/31/95, CD",
			AD_ENTRY1s("rtzcd.red", "946997d8b0aa6cb4e848bad02a1fc3d2", 354971),
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO0()
		},
		GID_RTZ,
		0,
		GF_CD_COMPRESSED,
		3,
	},

	{
		// Return to Zork - French CD version 1.2 5/13/95 (installed)
		// Patch #2685032 submitted by goodoldgeorg
		{
			"rtz",
			"V1.2, 5/13/95, installed, CD",
			AD_ENTRY1s("rtzcd.dat", "bde8251a8e34e87c54e3f93147d56c9e", 523776),
			Common::FR_FRA,
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
		// Return to Zork - French CD version 1.2 5/13/95
		// Patch #2685032 submitted by goodoldgeorg
		{
			"rtz",
			"V1.2, 3/31/95, CD",
			AD_ENTRY1s("rtzcd.red", "946997d8b0aa6cb4e848bad02a1fc3d2", 354614),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_CD,
			GUIO0()
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

// The Manhole: Masterpiece Edition is not a MADE engine and should not be
// added to the detection list. It is a HyperCard-like engine
//	{
//		// The Manhole: Masterpiece Edition (GOG/CD)
//		{
//			"manhole",
//			"",
//			AD_ENTRY1("manhole.dat", "e8cec9bf21e4c50a7ebc193a4e0b48f5"),
//			Common::EN_ANY,
//			Common::kPlatformDOS,
//			ADGF_UNSTABLE,
//			GUIO1(GUIO_NOSPEECH)
//		},
//		GID_MANHOLE,
//		0,
//		GF_CD,
//		2,
//	},

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
		// Supplied by windlepoons (bug tracker #2675695)
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
		// Supplied by goodoldgeorg (bug tracker #2675759)
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
		// Supplied by goodoldgeorg (bug tracker #2675759)
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
