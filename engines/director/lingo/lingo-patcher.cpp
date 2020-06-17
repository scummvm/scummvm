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
#include "director/score.h"
#include "director/lingo/lingo.h"


namespace Director {

using namespace Common;

struct ScriptPatch {
	const char *gameId;
	Common::Platform platform;
	const char *movie;
	ScriptType type;
	uint16 id;
	int linenum;
	const char *orig;
	const char *replace;
} const scriptPatches[] = {
	// Garbage at end of script
	{"warlock", kPlatformMacintosh, "WARLOCKSHIP/WARLOCKSHIP/UpForeECall", kScoreScript, 12,
			2, "SS Warlock:DATA:WARLOCKSHIP:Up.GCGunner", ""},
	{"warlock", kPlatformMacintosh, "WARLOCKSHIP/WARLOCKSHIP/UpForeECall", kScoreScript, 12,
			3, "Channels 17 to 18", ""},
	{"warlock", kPlatformMacintosh, "WARLOCKSHIP/WARLOCKSHIP/UpForeECall", kScoreScript, 12,
			4, "Frames 150 to 160", ""},

	// Unbalanced 'end if' at the end of the script
	{"warlock", kPlatformMacintosh, "STAMBUL/STAMBUL/DRUNK", kMovieScript, 5,
			5, "end if", ""},

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


	// Unbalanced 'end if' at the end of the script
	{"jman", kPlatformWindows, "mmm/TSA RR 06", kScoreScript, 26,
			17, "end if", ""},

	{nullptr, kPlatformUnknown, nullptr, kNoneScript, 0, 0, nullptr, nullptr}
};

Common::String Lingo::patchLingoCode(Common::String &line, ScriptType type, uint16 id, int linenum) {
	if (!_vm->getCurrentScore())
		return line;

	const ScriptPatch *patch = scriptPatches;
	Common::String movie = _vm->getCurrentPath() + _vm->getCurrentScore()->getMacName();

	// So far, we have not many patches, so do linear lookup
	while (patch->gameId) {
		// First, we do cheap comparisons
		if (patch->type != type || patch->id != id || patch->linenum != linenum) {
			patch++;
			continue;
		}

		// Now expensive ones
		if (movie.compareToIgnoreCase(patch->movie) || strcmp(patch->gameId, _vm->getGameId())) {
			patch++;
			continue;
		}

		// Now do a safeguard
		if (line.compareToIgnoreCase(patch->orig)) {
			warning("Lingo::patchLingoCode(): Unmatched patch for '%s', '%s' %s:%d @ %d. Expecting '%s' but got '%s'",
					patch->gameId, patch->movie, scriptType2str(type), id, linenum,
					patch->orig, line.c_str());
			return line;
		}

		// Now everything matched
		debugC(1, kDebugParse, "Lingo::patchLingoCode(): Applied a patch for '%s', '%s' %s:%d @ %d. \"%s\" -> \"%s\"",
				patch->gameId, patch->movie, scriptType2str(type), id, linenum,
				patch->orig, line.c_str());
		return patch->replace;
	}

	return line;
}

} // End of namespace Director
