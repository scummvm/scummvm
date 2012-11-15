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

	{ AD_TABLE_END_MARKER, GID_GENERIC, 0 }
};

} // End of Namespace Director

#endif
