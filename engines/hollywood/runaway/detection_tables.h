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
	// English retail release. Hospital, museum and chapter-three shack.
	{
		"runaway",
		nullptr,
		AD_ENTRY3s("RESOURCE.000", "f23900dd41f74bfaf71578b66821e228", 110312868,
			"RESOURCE.003", "8aa07584175fc9d28c1aff5fa5f93a5f", 7340291,
			"RESOURCE.B02", "088b730c4be3dfb4ac7c38e1fc215ef7", 8161628),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO2(GUIO_NOMIDI, GAMEOPTION_RESTORED_CONTENT)
	},
	// English GOG release, with Bink movies.
	{
		"runaway",
		"GOG",
		AD_ENTRY3s("RESOURCE.000", "22f373594fc346cf2d739b454ca5c789", 110716101,
			"RESOURCE.003", "8aa07584175fc9d28c1aff5fa5f93a5f", 7340291,
			"RESOURCE.B02", "486eace90c76d8772d751a879466cc39", 8352653),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO2(GUIO_NOMIDI, GAMEOPTION_RESTORED_CONTENT)
	},
	// Spanish DVD release, with Bink movies.
	{
		"runaway",
		"DVD",
		AD_ENTRY3s("RESOURCE.000", "7484159230d1a921bda9927be4bd54ca", 110312901,
			"RESOURCE.003", "8aa07584175fc9d28c1aff5fa5f93a5f", 7340291,
			"RESOURCE.B02", "486eace90c76d8772d751a879466cc39", 8352653),
		Common::ES_ESP,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO2(GUIO_NOMIDI, GAMEOPTION_RESTORED_CONTENT)
	},
	// English museum demo (2000). Gameplay POC; movies are not supported.
	{
		"runaway",
		"Museum demo",
		AD_ENTRY3s("RESOURCE.D06", "f1048f161dca69a3be6ffad173c3db43", 10608050,
			"RESOURCE.003", "a25c71fc759558e81aae49290b0cfdfc", 965642,
			"RESOURCE.000", "f1690b1de499e0555498173ad7916415", 3662162),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DEMO | ADGF_UNSTABLE | GF_RUNAWAY_MUSEUM_DEMO,
		GUIO2(GUIO_NOMIDI, GUIO_NOSPEECH)
	},
	// Spanish chapter-three demo (July 2001 build, Micromania 80).
	{
		"runaway",
		"Demo",
		AD_ENTRY3s("RESOURCE.E03", "df350293b7dab8f39aead666dcedce72", 41872288,
			"RESOURCE.003", "b9fb386d4af6ef7cc535ed4089e32aff", 7330982,
			"RESOURCE.000", "231c7a354e1452246c89d816f8df4833", 34789119),
		Common::ES_ESP,
		Common::kPlatformWindows,
		ADGF_DEMO | ADGF_UNSTABLE,
		GUIO1(GUIO_NOMIDI)
	},
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
		GUIO1(GUIO_NOMIDI)
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
		GUIO1(GUIO_NOMIDI)
	},

#endif // HOLLYWOOD_RUNAWAY_DETECTION_TABLES_H
