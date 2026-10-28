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

#ifndef HOLLYWOOD_MONSTERS_DETECTION_TABLES_H
#define HOLLYWOOD_MONSTERS_DETECTION_TABLES_H

// Included in the shared ADGameDescription array in hollywood/detection.cpp.
	// Spanish DOS demo.
	{
		"hollywood",
		"Demo",
		AD_ENTRY4s("MONSTERS.EXE", "54a1e8749448e08086a1929510ec4b6a", 1027587,
			"RESOURCE.000", "0f86d33d0519b0dcef62de5f46754e85", 4348283,
			"RESOURCE.M04", "396d3a7cbe11f14747cdc52f1ac038ba", 2286658,
			"RESOURCE.S0D", "96bba558e5508affca4ffab8d546de83", 1408634),
		Common::ES_ESP,
		Common::kPlatformDOS,
		ADGF_DEMO | ADGF_UNSTABLE,
		GUIO2(GAMEOPTION_RESTORED_CONTENT, GUIO_NOMIDI)
	},

	// Italian Windows demo distributed with the Italian release.
	{
		"hollywood",
		"Demo",
		AD_ENTRY4s("MONSTERS.EXE", "fd174231edb65bafcaea04c911c2d4bb", 671232,
			"RESOURCE.000", "b16a8300ca1af75a0322e443a8971159", 3662162,
			"RESOURCE.003", "cd0ecdca8faaf56c905e0b52d025c8e4", 2185543,
			"RESOURCE.S0D", "96bba558e5508affca4ffab8d546de83", 1408634),
		Common::IT_ITA,
		Common::kPlatformWindows,
		ADGF_DEMO | ADGF_UNSTABLE,
		GUIO3(GAMEOPTION_RESTORED_CONTENT, GUIO_NOMIDI, GUIO_NOSPEECH)
	},

	// Spanish Windows first edition full game, installed MONSTERS directory.
	{
		"hollywood",
		"1st edition",
		AD_ENTRY4s("MONSTERS.EXE", "1129cea40124683fc4aabaeaf6478f5a", 1167360,
			"RESOURCE.000", "b16a8300ca1af75a0322e443a8971159", 3662162,
			"RESOURCE.003", "352126965c51832ce970faece65cc2f8", 9444153,
			"RESOURCE.004", "a79d937879665b899c255957c7310d42", 265002432),
		Common::ES_ESP,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO2(GAMEOPTION_RESTORED_CONTENT, GUIO_NOMIDI)
	},

	// Italian Windows full game, installed Monsters directory.
	{
		"hollywood",
		nullptr,
		AD_ENTRY4s("MONSTERS.EXE", "d047170b3a143eaef506dec2f18fe993", 1559552,
			"RESOURCE.000", "b16a8300ca1af75a0322e443a8971159", 3662162,
			"RESOURCE.003", "72475bb0eff3f4920452342537584569", 9444153,
			"RESOURCE.004", "fb9f09029e73eb5ee708b995db9823d0", 256474138),
		Common::IT_ITA,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO2(GAMEOPTION_RESTORED_CONTENT, GUIO_NOMIDI)
	},

	// Spanish Windows full game, installed MONSTERS directory.
	{
		"hollywood",
		nullptr,
		AD_ENTRY3s("MONSTERS.EXE", "5b097e04e6ca7225d9dea6cd99909f6d", 1431552,
			"RESOURCE.000", "bf580b6b3eedf45e427bb83ab29ccd21", 4320424,
			"RESOURCE.003", "352126965c51832ce970faece65cc2f8", 9444153),
		Common::ES_ESP,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO2(GAMEOPTION_RESTORED_CONTENT, GUIO_NOMIDI)
	},

	// English fan translation v1.0.0 by ShinjiGR and A.P.$lasH, based on the Spanish Windows release.
	// RESOURCE.003 retains the original header, so hash its translated text at the end.
	{
		"hollywood",
		"Fanmade",
		AD_ENTRY4s("MONSTERS.EXE", "1129cea40124683fc4aabaeaf6478f5a", 1167360,
			"RESOURCE.000", "bf580b6b3eedf45e427bb83ab29ccd21", 4320424,
			"RESOURCE.003", "t:69d8f3b0333a9809f597765f4afb30a2", 9444153,
			"RESOURCE.004", "a79d937879665b899c255957c7310d42", 265002432),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO2(GAMEOPTION_RESTORED_CONTENT, GUIO_NOMIDI)
	},

	// Spanish DOS CD full game, MONSTERS directory from the CD.
	{
		"hollywood",
		nullptr,
		AD_ENTRY3s("MONSTERS.EXE", "54a1e8749448e08086a1929510ec4b6a", 2513867,
			"RESOURCE.000", "bf580b6b3eedf45e427bb83ab29ccd21", 4320424,
			"RESOURCE.003", "352126965c51832ce970faece65cc2f8", 9444153),
		Common::ES_ESP,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO2(GAMEOPTION_RESTORED_CONTENT, GUIO_NOMIDI)
	},

#endif // HOLLYWOOD_MONSTERS_DETECTION_TABLES_H
