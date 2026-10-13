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

#ifndef HOLLYWOOD_RUNAWAY_DETECTION_TABLES_H
#define HOLLYWOOD_RUNAWAY_DETECTION_TABLES_H

// Included in the shared ADGameDescription array in hollywood/detection.cpp.
	// German Runaway demo, starting in the shack (scene 5030).
	{
		"runaway",
		"Demo",
		AD_ENTRY3s("RESOURCE.E03", "df350293b7dab8f39aead666dcedce72", 41872288,
			"RESOURCE.003", "6e7e639375a14c5b694879c88a8aeaa6", 444292,
			"RESOURCE.000", "3ccd0619e1475efd4f1212b92db25843", 38045367),
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_DEMO | ADGF_UNSTABLE,
		GUIO2(GUIO_NOMIDI, GUIO_NOLAUNCHLOAD)
	},
	// French Runaway demo (February 2003 build), also starting in scene 5030.
	{
		"runaway",
		"Demo",
		AD_ENTRY3s("RESOURCE.E03", "df350293b7dab8f39aead666dcedce72", 41872288,
			"RESOURCE.003", "c6f4c7e73599d5f43978a377e1881144", 662916,
			"RESOURCE.000", "16d32881d3f34fdf0da3c03c31f8f1ba", 35512149),
		Common::FR_FRA,
		Common::kPlatformWindows,
		ADGF_DEMO | ADGF_UNSTABLE,
		GUIO2(GUIO_NOMIDI, GUIO_NOLAUNCHLOAD)
	},

#endif // HOLLYWOOD_RUNAWAY_DETECTION_TABLES_H
