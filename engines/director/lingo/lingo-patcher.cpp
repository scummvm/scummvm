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
#include "director/lingo/lingo.h"


namespace Director {

using namespace Common;

struct ScriptPatch {
	const char *gameId;
	Common::Platform platform; // Specify kPlatformUnknown for skipping platform check
	const char *movie;
	ScriptType type;
	uint16 id;
	int linenum;
	const char *orig;
	const char *replace;
} const scriptPatches[] = {
	// Garbage at end of script
	{"warlock", kPlatformMacintosh, "WARLOCKSHIP/UpForeECall", kScoreScript, 12,
			2, "SS Warlock:DATA:WARLOCKSHIP:Up.GCGunner", ""},
	{"warlock", kPlatformMacintosh, "WARLOCKSHIP/UpForeECall", kScoreScript, 12,
			3, "Channels 17 to 18", ""},
	{"warlock", kPlatformMacintosh, "WARLOCKSHIP/UpForeECall", kScoreScript, 12,
			4, "Frames 150 to 160", ""},

	// Garbage at end of script
	{"warlock", kPlatformMacintosh, "DATA/WARLOCKSHIP/HE.Aft", kScoreScript, 8,
			2, "SS Warlock:DATA:WARLOCKSHIP:HangStairsFore", ""},
	{"warlock", kPlatformMacintosh, "DATA/WARLOCKSHIP/HE.Aft", kScoreScript, 8,
			3, "Channels 4 to 5", ""},
	{"warlock", kPlatformMacintosh, "DATA/WARLOCKSHIP/HE.Aft", kScoreScript, 8,
			4, "Frames 20 to 20", ""},

	// Garbage at end of script
	{"warlock", kPlatformMacintosh, "DATA/WARLOCKSHIP/ENG/D10", kScoreScript, 8,
			2, "SS Warlock:ENG.Fold:C9", ""},
	{"warlock", kPlatformMacintosh, "DATA/WARLOCKSHIP/ENG/D10", kScoreScript, 8,
			3, "Channels 19 to 20", ""},
	{"warlock", kPlatformMacintosh, "DATA/WARLOCKSHIP/ENG/D10", kScoreScript, 8,
			4, "Frames 165 to 180", ""},

	// Garbage at end of script
	{"warlock", kPlatformMacintosh, "DATA/WARLOCKSHIP/Up.c2", kScoreScript, 10,
			2, "Frames 150 to 160", ""},

	// Garbage at end of script
	{"warlock", kPlatformMacintosh, "DATA/WARLOCKSHIP/Up.ForeECall", kScoreScript, 12,
			2, "SS Warlock:DATA:WARLOCKSHIP:Up.GCGunner", ""},
	{"warlock", kPlatformMacintosh, "DATA/WARLOCKSHIP/Up.ForeECall", kScoreScript, 12,
			3, "Channels 17 to 18", ""},
	{"warlock", kPlatformMacintosh, "DATA/WARLOCKSHIP/Up.ForeECall", kScoreScript, 12,
			4, "Frames 150 to 160", ""},

	// Garbage at end of script
	{"warlock", kPlatformMacintosh, "DATA/WARLOCKSHIP/Up.B2", kScoreScript, 9,
			2, "SS Warlock:DATA:WARLOCKSHIP:Up.GCGunner", ""},
	{"warlock", kPlatformMacintosh, "DATA/WARLOCKSHIP/Up.B2", kScoreScript, 9,
			3, "Channels 17 to 18", ""},
	{"warlock", kPlatformMacintosh, "DATA/WARLOCKSHIP/Up.B2", kScoreScript, 9,
			4, "Frames 150 to 160", ""},

	// Garbage at end of script
	{"warlock", kPlatformMacintosh, "DATA/BELSHAZZAR/STELLA/ORIGIN", kScoreScript, 12,
			2, "Frames 1 to 1", ""},

	// Garbage at end of script
	{"warlock", kPlatformMacintosh, "DATA/WARLOCKSHIP/HangHallAft", kScoreScript, 7,
			2, "SS Warlock:DATA:WARLOCKSHIP:HangStairsFore", ""},
	{"warlock", kPlatformMacintosh, "DATA/WARLOCKSHIP/HangHallAft", kScoreScript, 7,
			3, "Channels 4 to 5", ""},
	{"warlock", kPlatformMacintosh, "DATA/WARLOCKSHIP/HangHallAft", kScoreScript, 7,
			4, "Frames 20 to 20", ""},

	// Stray 'then' (obvious copy/paste error)
	{"warlock", kPlatformMacintosh, "DATA/K/KT/OutMarauderKT", kMovieScript, 2,
			23, "set Spacesuit = 0 then", "set Spacesuit = 0"},

	// Unbalanced 'end if' at the end of the script
	{"warlock", kPlatformMacintosh, "DATA/STAMBUL/DRUNK", kMovieScript, 4,
			5, "end if", ""},

	// Unbalanced 'end if' at the end of the script
	{"warlock", kPlatformMacintosh, "STAMBUL/DRUNK", kMovieScript, 5,
			5, "end if", ""},

	// Missing '&'
	{"warlock", kPlatformMacintosh, "DATA/NAV/Shared Cast", kMovieScript, 1,
			19, "alert \"Failed Save.\" & return & \"Error message number: \" string ( filer )",
				"alert \"Failed Save.\" & return & \"Error message number: \" & string ( filer )"},

	// Garbage at end of script
	{"warlock", kPlatformWindows, "WRLCKSHP/UpForeECall", kScoreScript, 12,
			2, "SS Warlock:DATA:WARLOCKSHIP:Up.GCGunner", ""},
	{"warlock", kPlatformWindows, "WRLCKSHP/UpForeECall", kScoreScript, 12,
			3, "Channels 17 to 18", ""},
	{"warlock", kPlatformWindows, "WRLCKSHP/UpForeECall", kScoreScript, 12,
			4, "Frames 150 to 160", ""},

	// Unbalanced 'end if' at the end of the script
	{"warlock", kPlatformWindows, "STAMBUL/DRUNK", kMovieScript, 5,
			5, "end if", ""},

	// Missing '&'
	{"warlock", kPlatformUnknown, "NAV/Shared Cast", kMovieScript, 0,
			23, "alert \"Failed Save.\" & return & \"Error message number: \" string ( filer )",
				"alert \"Failed Save.\" & return & \"Error message number: \" & string ( filer )"},

	{"warlock", kPlatformUnknown, "NAV/Shared Cast", kMovieScript, 1,	// For running by the buildbot
			23, "alert \"Failed Save.\" & return & \"Error message number: \" string ( filer )",
				"alert \"Failed Save.\" & return & \"Error message number: \" & string ( filer )"},


	// Unbalanced 'end if' at the end of the script
	{"jman", kPlatformWindows, "mmm/TSA RR 06", kScoreScript, 26,
			17, "end if", ""},
	{"jman", kPlatformWindows, "mmm/Mars Space Game 05", kMovieScript, 3,
			68, "set DamageParameter = (gProcessorSpeed/2) + 7)", "set DamageParameter = (gProcessorSpeed/2) + 7"},

	{nullptr, kPlatformUnknown, nullptr, kNoneScript, 0, 0, nullptr, nullptr}
};

Common::String Lingo::patchLingoCode(Common::String &line, LingoArchive *archive, ScriptType type, uint16 id, int linenum) {
	if (!archive)
		return line;

	const ScriptPatch *patch = scriptPatches;
	Common::String movie = _vm->getCurrentPath() + archive->cast->getMacName();

	// So far, we have not many patches, so do linear lookup
	while (patch->gameId) {
		// First, we do cheap comparisons
		if (patch->type != type || patch->id != id || patch->linenum != linenum ||
				(patch->platform != kPlatformUnknown && patch->platform != _vm->getPlatform())) {
			patch++;
			continue;
		}

		// Now expensive ones
		if (movie.compareToIgnoreCase(patch->movie) || strcmp(patch->gameId, _vm->getGameId())) {
			patch++;
			continue;
		}

		// Now do a safeguard
		if (!line.contains(patch->orig)) {
			warning("Lingo::patchLingoCode(): Unmatched patch for '%s', '%s' %s:%d @ %d. Expecting '%s' but got '%s'",
					patch->gameId, patch->movie, scriptType2str(type), id, linenum,
					patch->orig, line.c_str());
			return line;
		}

		// Now everything matched
		debugC(1, kDebugParse | kDebugPreprocess, "Lingo::patchLingoCode(): Applied a patch for '%s', '%s' %s:%d @ %d. \"%s\" -> \"%s\"",
				patch->gameId, patch->movie, scriptType2str(type), id, linenum,
				patch->orig, patch->replace);
		return patch->replace;
	}

	return line;
}

} // End of namespace Director
