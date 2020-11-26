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

#ifndef PETKA_DETECTION_TABLES_H
#define PETKA_DETECTION_TABLES_H

namespace Petka {

static const ADGameDescription gameDescriptions[] = {

	// Red Comrades Demo
	{
		"petka1",
		0,
		AD_ENTRY2s("DEMO.EXE",   "5ef1ceaba05413d04fd733a81e6adbae", 888832,
				   "SCRIPT.DAT", "8712157c39daf8dd1f441a11d19b9e48", 1851),
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_TESTING | ADGF_DROPPLATFORM | ADGF_DROPLANGUAGE | ADGF_DEMO,
		GUIO1(GUIO_NOMIDI)

	},

	// Red Comrades 1: Save the Galaxy
	{
		"petka1",
		0,
		AD_ENTRY1s("MAIN.STR", "2523bf402ac8b7b2bf54e6e29a79831d", 27414919),
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_TESTING | ADGF_DROPPLATFORM | ADGF_DROPLANGUAGE,
		GUIO1(GUIO_NOMIDI)

	},

	// Red Comrades 2: For the Great Justice
	{
		"petka2",
		0,
		AD_ENTRY1s("main.str", "4e515669c343609518277cab6e7d8c8f", 18992879),
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_TESTING | ADGF_DROPPLATFORM | ADGF_DROPLANGUAGE,
		GUIO1(GUIO_NOMIDI)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Petka

#endif
