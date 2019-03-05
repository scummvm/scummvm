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

//#define GAMEOPTION_PLAY_MYST_FLYBY         GUIO_GAMEOPTIONS1

//#define GUI_OPTIONS_MYST                   GUIO3(GUIO_NOASPECT, GUIO_NOSUBTITLES, GUIO_NOMIDI)

static const CryOmni3DGameDescription gameDescriptions[] = {
	// Versailles 1685
	// French Windows 95
	// From lePhilousophe
	{
		{
			"versailles",
			"",
			AD_ENTRY1s("VERSAILL.EXE", "3775004b96f056716ce615b458b1f394", 372736),
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GType_VERSAILLES,
		0,
		0,
	},

	// Versailles 1685
	// French Windows 95 compressed
	// From lePhilousophe
	{
		{
			"versailles",
			"",
			AD_ENTRY1s("PROGRAM.Z", "a07b5d86af5f3a8883ba97db2bade87d", 293223),
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GType_VERSAILLES,
		0,
		0,
	},

	// Versailles 1685
	// French DOS
	// From lePhilousophe
	{
		{
			"versailles",
			"",
			AD_ENTRY1s("VERSAILL.PGM", "1c992f034f43418a5da2e8ebd0b92620", 630431),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GType_VERSAILLES,
		0,
		0,
	},

	{ AD_TABLE_END_MARKER, 0, 0, 0 }
};

//////////////////////////////
//Fallback detection
//////////////////////////////

static const CryOmni3DGameDescription fallbackDescs[] = {
	{
		{
			"versailles",
			"unknown",
			AD_ENTRY1(0, 0),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GType_VERSAILLES,
		0,
		0
	},
};

static const ADFileBasedFallback fileBased[] = {
	{ &fallbackDescs[0].desc,  { "VERSAILL.EXE", 0 } },
	{ &fallbackDescs[0].desc,  { "VERSAILL.PGM", 0 } },
	{ 0, { 0 } }
};

} // End of Namespace CryOmni3D
