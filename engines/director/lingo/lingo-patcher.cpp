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

#include "director/director.h"
#include "director/cast.h"
#include "director/movie.h"
#include "director/lingo/lingo-codegen.h"


namespace Director {

using namespace Common;

struct ScriptPatch {
	const char *gameId;
	const char *extra;
	Common::Platform platform; // Specify kPlatformUnknown for skipping platform check
	const char *movie;
	ScriptType type;
	uint16 id;
	uint16 castLib;
	int linenum;
	const char *orig;
	const char *replace;
} const scriptPatches[] = {
	// Garbage at end of script
	{"warlock", nullptr, kPlatformMacintosh, "WARLOCKSHIP/UpForeECall", kScoreScript, 12, 0,
			2, "SS Warlock:DATA:WARLOCKSHIP:Up.GCGunner", ""},
	{"warlock", nullptr, kPlatformMacintosh, "WARLOCKSHIP/UpForeECall", kScoreScript, 12, 0,
			3, "Channels 17 to 18", ""},
	{"warlock", nullptr, kPlatformMacintosh, "WARLOCKSHIP/UpForeECall", kScoreScript, 12, 0,
			4, "Frames 150 to 160", ""},

	// Garbage at end of script
	{"warlock", nullptr, kPlatformMacintosh, "DATA/WARLOCKSHIP/HE.Aft", kScoreScript, 8, 0,
			2, "SS Warlock:DATA:WARLOCKSHIP:HangStairsFore", ""},
	{"warlock", nullptr, kPlatformMacintosh, "DATA/WARLOCKSHIP/HE.Aft", kScoreScript, 8, 0,
			3, "Channels 4 to 5", ""},
	{"warlock", nullptr, kPlatformMacintosh, "DATA/WARLOCKSHIP/HE.Aft", kScoreScript, 8, 0,
			4, "Frames 20 to 20", ""},

	// Garbage at end of script
	{"warlock", nullptr, kPlatformMacintosh, "DATA/WARLOCKSHIP/ENG/D10", kScoreScript, 8, 0,
			2, "SS Warlock:ENG.Fold:C9", ""},
	{"warlock", nullptr, kPlatformMacintosh, "DATA/WARLOCKSHIP/ENG/D10", kScoreScript, 8, 0,
			3, "Channels 19 to 20", ""},
	{"warlock", nullptr, kPlatformMacintosh, "DATA/WARLOCKSHIP/ENG/D10", kScoreScript, 8, 0,
			4, "Frames 165 to 180", ""},

	// Garbage at end of script
	{"warlock", nullptr, kPlatformMacintosh, "DATA/WARLOCKSHIP/Up.c2", kScoreScript, 10, 0,
			2, "Frames 150 to 160", ""},

	// Garbage at end of script
	{"warlock", nullptr, kPlatformMacintosh, "DATA/WARLOCKSHIP/Up.ForeECall", kScoreScript, 12, 0,
			2, "SS Warlock:DATA:WARLOCKSHIP:Up.GCGunner", ""},
	{"warlock", nullptr, kPlatformMacintosh, "DATA/WARLOCKSHIP/Up.ForeECall", kScoreScript, 12, 0,
			3, "Channels 17 to 18", ""},
	{"warlock", nullptr, kPlatformMacintosh, "DATA/WARLOCKSHIP/Up.ForeECall", kScoreScript, 12, 0,
			4, "Frames 150 to 160", ""},

	// Garbage at end of script
	{"warlock", nullptr, kPlatformMacintosh, "DATA/WARLOCKSHIP/Up.B2", kScoreScript, 9, 0,
			2, "SS Warlock:DATA:WARLOCKSHIP:Up.GCGunner", ""},
	{"warlock", nullptr, kPlatformMacintosh, "DATA/WARLOCKSHIP/Up.B2", kScoreScript, 9, 0,
			3, "Channels 17 to 18", ""},
	{"warlock", nullptr, kPlatformMacintosh, "DATA/WARLOCKSHIP/Up.B2", kScoreScript, 9, 0,
			4, "Frames 150 to 160", ""},

	// Garbage at end of script
	{"warlock", nullptr, kPlatformMacintosh, "DATA/BELSHAZZAR/STELLA/ORIGIN", kScoreScript, 12, 0,
			2, "Frames 1 to 1", ""},

	// Garbage at end of script
	{"warlock", nullptr, kPlatformMacintosh, "DATA/WARLOCKSHIP/HangHallAft", kScoreScript, 7, 0,
			2, "SS Warlock:DATA:WARLOCKSHIP:HangStairsFore", ""},
	{"warlock", nullptr, kPlatformMacintosh, "DATA/WARLOCKSHIP/HangHallAft", kScoreScript, 7, 0,
			3, "Channels 4 to 5", ""},
	{"warlock", nullptr, kPlatformMacintosh, "DATA/WARLOCKSHIP/HangHallAft", kScoreScript, 7, 0,
			4, "Frames 20 to 20", ""},

	// Stray 'then' (obvious copy/paste error)
	{"warlock", nullptr, kPlatformMacintosh, "DATA/K/KT/OutMarauderKT", kMovieScript, 14, 0,
			23, "set Spacesuit = 0 then", "set Spacesuit = 0"},

	// Missing '&'
	{"warlock", nullptr, kPlatformMacintosh, "DATA/NAV/Shared Cast", kMovieScript, 510, 0,
			19, "alert \"Failed Save.\" & return & \"Error message number: \" string ( filer )",
				"alert \"Failed Save.\" & return & \"Error message number: \" & string ( filer )"},

	// Garbage at end of script
	{"warlock", "", kPlatformWindows, "WRLCKSHP/UpForeECall", kScoreScript, 12, 0,
			2, "SS Warlock:DATA:WARLOCKSHIP:Up.GCGunner", ""},
	{"warlock", "", kPlatformWindows, "WRLCKSHP/UpForeECall", kScoreScript, 12, 0,
			3, "Channels 17 to 18", ""},
	{"warlock", "", kPlatformWindows, "WRLCKSHP/UpForeECall", kScoreScript, 12, 0,
			4, "Frames 150 to 160", ""},

	// Missing '&'
	{"warlock", nullptr, kPlatformUnknown, "NAV/Shared Cast", kMovieScript, 510, 0,
			23, "alert \"Failed Save.\" & return & \"Error message number: \" string ( filer )",
				"alert \"Failed Save.\" & return & \"Error message number: \" & string ( filer )"},


	// Patching dead loop which was fixed in v2
	{"lzone", "", kPlatformMacintosh, "DATA/R-A/Ami-00", kScoreScript, 3, 0,
			2, "continue", "go \"OUT\""},

	// Garbage at end of statements
	{"lzone", "", kPlatformMacintosh, "DATA/R-E/ZD2-LAS", kScoreScript, 7, 0,
			4, "go to the frame 0", "go to the frame"},
	{"lzone", "", kPlatformMacintosh, "DATA/R-E/zd1-con1", kScoreScript, 27, 0,
			1, "go to the frame 0", "go to the frame"},
	{"lzone", "", kPlatformMacintosh, "DATA/R-E/zd1-con1", kScoreScript, 30, 0,
			4, "go the frame 0", "go to the frame"},
	{"lzone", "", kPlatformMacintosh, "DATA/R-G/st-c", kScoreScript, 14, 0,
			1, "go to the frame 0", "go to the frame"},
	{"lzone", "", kPlatformMacintosh, "DATA/R-G/st-d.mo", kScoreScript, 4, 0,
			1, "go to the frame 0", "go to the frame"},
	{"lzone", "", kPlatformMacintosh, "DATA/R-F/ARCH-U.D-1", kScoreScript, 8, 0,
			1, "GO \"SPACE\" OF MOVIE \"L-ZONE:DATA:R-G:ST-A2\",\"242,197\"",
			   "GO \"SPACE\" OF MOVIE \"L-ZONE:DATA:R-G:ST-A2\""},


	{"jman", "", kPlatformWindows, "mmm/Mars Space Game 05", kMovieScript, 10, 0,
			68, "set DamageParameter = (gProcessorSpeed/2) + 7)", "set DamageParameter = (gProcessorSpeed/2) + 7"},

	{nullptr, nullptr, kPlatformUnknown, nullptr, kNoneScript, 0, 0, 0, nullptr, nullptr}
};

Common::U32String LingoCompiler::patchLingoCode(const Common::U32String &line, LingoArchive *archive, ScriptType type, CastMemberID id, int linenum) {
	if (!archive)
		return line;

	const ScriptPatch *patch = scriptPatches;
	Common::String movie = g_director->getCurrentPath() + archive->cast->getMacName();

	// So far, we have not many patches, so do linear lookup
	while (patch->gameId) {
		// First, we do cheap comparisons
		if (patch->type != type || patch->id != id.member || patch->castLib != id.castLib || patch->linenum != linenum ||
				(patch->platform != kPlatformUnknown && patch->platform != g_director->getPlatform())) {
			patch++;
			continue;
		}

		// Now expensive ones
		if (movie.compareToIgnoreCase(patch->movie) || strcmp(patch->gameId, g_director->getGameId())
				|| (patch->extra && strcmp(patch->extra, g_director->getExtra()))) {
			patch++;
			continue;
		}

		// Now do a safeguard
		if (!line.contains(Common::U32String(patch->orig))) {
			warning("Lingo::patchLingoCode(): Unmatched patch for '%s', '%s' %s:%s @ %d. Expecting '%s' but got '%s'",
					patch->gameId, patch->movie, scriptType2str(type), id.asString().c_str(), linenum,
					patch->orig, line.encode().c_str());
			return line;
		}

		// Now everything matched
		warning("Lingo::patchLingoCode(): Applied a patch for '%s', '%s' %s:%s @ %d. \"%s\" -> \"%s\"",
				patch->gameId, patch->movie, scriptType2str(type), id.asString().c_str(), linenum,
				patch->orig, patch->replace);
		return Common::U32String(patch->replace);
	}

	return line;
}

} // End of namespace Director
