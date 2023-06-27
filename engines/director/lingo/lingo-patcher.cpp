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

#include "common/punycode.h"
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
	{"warlock", nullptr, kPlatformMacintosh, "WARLOCKSHIP:UpForeECall", kScoreScript, 12, DEFAULT_CAST_LIB,
			2, "SS Warlock:DATA:WARLOCKSHIP:Up.GCGunner", ""},
	{"warlock", nullptr, kPlatformMacintosh, "WARLOCKSHIP:UpForeECall", kScoreScript, 12, DEFAULT_CAST_LIB,
			3, "Channels 17 to 18", ""},
	{"warlock", nullptr, kPlatformMacintosh, "WARLOCKSHIP:UpForeECall", kScoreScript, 12, DEFAULT_CAST_LIB,
			4, "Frames 150 to 160", ""},

	// Garbage at end of script
	{"warlock", nullptr, kPlatformMacintosh, "DATA:WARLOCKSHIP:HE.Aft", kScoreScript, 8, DEFAULT_CAST_LIB,
			2, "SS Warlock:DATA:WARLOCKSHIP:HangStairsFore", ""},
	{"warlock", nullptr, kPlatformMacintosh, "DATA:WARLOCKSHIP:HE.Aft", kScoreScript, 8, DEFAULT_CAST_LIB,
			3, "Channels 4 to 5", ""},
	{"warlock", nullptr, kPlatformMacintosh, "DATA:WARLOCKSHIP:HE.Aft", kScoreScript, 8, DEFAULT_CAST_LIB,
			4, "Frames 20 to 20", ""},

	// Garbage at end of script
	{"warlock", nullptr, kPlatformMacintosh, "DATA:WARLOCKSHIP:ENG:D10", kScoreScript, 8, DEFAULT_CAST_LIB,
			2, "SS Warlock:ENG.Fold:C9", ""},
	{"warlock", nullptr, kPlatformMacintosh, "DATA:WARLOCKSHIP:ENG:D10", kScoreScript, 8, DEFAULT_CAST_LIB,
			3, "Channels 19 to 20", ""},
	{"warlock", nullptr, kPlatformMacintosh, "DATA:WARLOCKSHIP:ENG:D10", kScoreScript, 8, DEFAULT_CAST_LIB,
			4, "Frames 165 to 180", ""},

	// Garbage at end of script
	{"warlock", nullptr, kPlatformMacintosh, "DATA:WARLOCKSHIP:Up.c2", kScoreScript, 10, DEFAULT_CAST_LIB,
			2, "Frames 150 to 160", ""},

	// Garbage at end of script
	{"warlock", nullptr, kPlatformMacintosh, "DATA:WARLOCKSHIP:Up.ForeECall", kScoreScript, 12, DEFAULT_CAST_LIB,
			2, "SS Warlock:DATA:WARLOCKSHIP:Up.GCGunner", ""},
	{"warlock", nullptr, kPlatformMacintosh, "DATA:WARLOCKSHIP:Up.ForeECall", kScoreScript, 12, DEFAULT_CAST_LIB,
			3, "Channels 17 to 18", ""},
	{"warlock", nullptr, kPlatformMacintosh, "DATA:WARLOCKSHIP:Up.ForeECall", kScoreScript, 12, DEFAULT_CAST_LIB,
			4, "Frames 150 to 160", ""},

	// Garbage at end of script
	{"warlock", nullptr, kPlatformMacintosh, "DATA:WARLOCKSHIP:Up.B2", kScoreScript, 9, DEFAULT_CAST_LIB,
			2, "SS Warlock:DATA:WARLOCKSHIP:Up.GCGunner", ""},
	{"warlock", nullptr, kPlatformMacintosh, "DATA:WARLOCKSHIP:Up.B2", kScoreScript, 9, DEFAULT_CAST_LIB,
			3, "Channels 17 to 18", ""},
	{"warlock", nullptr, kPlatformMacintosh, "DATA:WARLOCKSHIP:Up.B2", kScoreScript, 9, DEFAULT_CAST_LIB,
			4, "Frames 150 to 160", ""},

	// Garbage at end of script
	{"warlock", nullptr, kPlatformMacintosh, "DATA:BELSHAZZAR:STELLA:ORIGIN", kScoreScript, 12, DEFAULT_CAST_LIB,
			2, "Frames 1 to 1", ""},
	{"warlock", nullptr, kPlatformMacintosh, "DATA:BELSHAZZAR:STELLA:ORIGIN", kScoreScript, 13, DEFAULT_CAST_LIB,
			2, "Frames 1 to 1", ""},

	// Garbage at end of script
	{"warlock", nullptr, kPlatformMacintosh, "DATA:WARLOCKSHIP:HangHallAft", kScoreScript, 7, DEFAULT_CAST_LIB,
			2, "SS Warlock:DATA:WARLOCKSHIP:HangStairsFore", ""},
	{"warlock", nullptr, kPlatformMacintosh, "DATA:WARLOCKSHIP:HangHallAft", kScoreScript, 7, DEFAULT_CAST_LIB,
			3, "Channels 4 to 5", ""},
	{"warlock", nullptr, kPlatformMacintosh, "DATA:WARLOCKSHIP:HangHallAft", kScoreScript, 7, DEFAULT_CAST_LIB,
			4, "Frames 20 to 20", ""},

	// Stray 'then' (obvious copy/paste error)
	{"warlock", nullptr, kPlatformMacintosh, "DATA:K:KT:OutMarauderKT", kMovieScript, 14, DEFAULT_CAST_LIB,
			23, "set Spacesuit = 0 then", "set Spacesuit = 0"},

	// Missing '&'
	{"warlock", nullptr, kPlatformMacintosh, "DATA:NAV:Shared Cast", kMovieScript, 510, DEFAULT_CAST_LIB,
			19, "alert \"Failed Save.\" & return & \"Error message number: \" string ( filer )",
				"alert \"Failed Save.\" & return & \"Error message number: \" & string ( filer )"},

	// Garbage at end of script
	{"warlock", "v1.1.3 MPC", kPlatformWindows, "WRLCKSHP:UpForeECall", kScoreScript, 12, DEFAULT_CAST_LIB,
			2, "SS Warlock:DATA:WARLOCKSHIP:Up.GCGunner", ""},
	{"warlock", "v1.1.3 MPC", kPlatformWindows, "WRLCKSHP:UpForeECall", kScoreScript, 12, DEFAULT_CAST_LIB,
			3, "Channels 17 to 18", ""},
	{"warlock", "v1.1.3 MPC", kPlatformWindows, "WRLCKSHP:UpForeECall", kScoreScript, 12, DEFAULT_CAST_LIB,
			4, "Frames 150 to 160", ""},

	// Missing '&'
	{"warlock", nullptr, kPlatformUnknown, "NAV:Shared Cast", kMovieScript, 510, DEFAULT_CAST_LIB,
			23, "alert \"Failed Save.\" & return & \"Error message number: \" string ( filer )",
				"alert \"Failed Save.\" & return & \"Error message number: \" & string ( filer )"},

	// Non-existent menu cast reference
	{"warlock", nullptr, kPlatformWindows, "STARBIRD:ABOUT", kScoreScript, 4, DEFAULT_CAST_LIB,
			1, "installmenu A13", ""},


	// Patching dead loop which was fixed in v2
	{"lzone", "", kPlatformMacintosh, "DATA:R-A:Ami-00", kScoreScript, 3, DEFAULT_CAST_LIB,
			2, "continue", "go \"OUT\""},

	// Garbage at end of statements
	{"lzone", "", kPlatformMacintosh, "DATA:R-E:ZD2-LAS", kScoreScript, 7, DEFAULT_CAST_LIB,
			4, "go to the frame 0", "go to the frame"},
	{"lzone", "", kPlatformMacintosh, "DATA:R-E:zd1-con1", kScoreScript, 27, DEFAULT_CAST_LIB,
			1, "go to the frame 0", "go to the frame"},
	{"lzone", "", kPlatformMacintosh, "DATA:R-E:zd1-con1", kScoreScript, 30, DEFAULT_CAST_LIB,
			4, "go the frame 0", "go to the frame"},
	{"lzone", "", kPlatformMacintosh, "DATA:R-G:st-c", kScoreScript, 14, DEFAULT_CAST_LIB,
			1, "go to the frame 0", "go to the frame"},
	{"lzone", "", kPlatformMacintosh, "DATA:R-G:st-d.mo", kScoreScript, 4, DEFAULT_CAST_LIB,
			1, "go to the frame 0", "go to the frame"},
	{"lzone", "", kPlatformMacintosh, "DATA:R-F:ARCH-U.D-1", kScoreScript, 8, DEFAULT_CAST_LIB,
			1, "GO \"SPACE\" OF MOVIE \"L-ZONE:DATA:R-G:ST-A2\",\"242,197\"",
			   "GO \"SPACE\" OF MOVIE \"L-ZONE:DATA:R-G:ST-A2\""},


	{"lingoexpo", "", kPlatformMacintosh, "Lingo Expo:Navigator", kMovieScript, 9, DEFAULT_CAST_LIB,
			97, "  append(codeExampleList,\"6,301,302,303,304,305,306\")  - KIOSK SCRIPTS",
				"  append(codeExampleList,\"6,301,302,303,304,305,306\")"},


	{"jman", "", kPlatformWindows, "mmm:Mars Space Game 05", kMovieScript, 10, DEFAULT_CAST_LIB,
			68, "set DamageParameter = (gProcessorSpeed/2) + 7)",
				"set DamageParameter = (gProcessorSpeed/2) + 7"},

	{"jman", "", kPlatformWindows, "MMM:Shared Cast B&W", kMovieScript, 323, DEFAULT_CAST_LIB,
			187, "set the trails of sprite 19 to 0", "set the locH of sprite 19 to 408"},
	{"jman", "", kPlatformWindows, "MMM:Shared Cast B&W", kMovieScript, 323, DEFAULT_CAST_LIB,
			188, "set the locH of sprite 19 to 408", "set the locV of sprite 19 to 168"},
	{"jman", "", kPlatformWindows, "MMM:Shared Cast B&W", kMovieScript, 323, DEFAULT_CAST_LIB,
			189, "set the locV of sprite 19 to 168", "set the text of field \"Description\" = description"},
	{"jman", "", kPlatformWindows, "MMM:Shared Cast B&W", kMovieScript, 323, DEFAULT_CAST_LIB,
			190, "set the text of field \"Description\" = description", "set the castnum of sprite 19 to the number of cast \"Description\""},
	{"jman", "", kPlatformWindows, "MMM:Shared Cast B&W", kMovieScript, 323, DEFAULT_CAST_LIB,
			191, "set the castnum of sprite 19 to the number of cast \"Description\"", "updateStage"},
	{"jman", "", kPlatformWindows, "MMM:Shared Cast B&W", kMovieScript, 323, DEFAULT_CAST_LIB,
			192, "updateStage", "set the trails of sprite 19 to 0"},


	{"snh", "Hybrid release", kPlatformWindows, "SNHstart", kMovieScript, 0, DEFAULT_CAST_LIB,
			3, "changedrive", ""}, // HACK: This macro inserts \x01 after the first character in myCD/myHD
	{"snh", "Hybrid release", kPlatformWindows, "SNHstart", kMovieScript, 0, DEFAULT_CAST_LIB,
			6, "set mytest2 = FileIO(mnew, \"read\" mymovie)", "set mytest2 = FileIO(mnew, \"read\", mymovie)"},
	{"snh", "Hybrid release", kPlatformWindows, "SNHstart", kMovieScript, 0, DEFAULT_CAST_LIB,
			14, "set mytest3 = FileIO(mnew, \"read\" mymovie)", "set mytest3 = FileIO(mnew, \"read\", mymovie)"},


	// Ambiguous syntax that's parsed differently between D3 and later versions
	{"henachoco03", "", kPlatformMacintosh, "xn--oj7cxalkre7cjz1d2agc0e8b1cm", kMovieScript, 0, DEFAULT_CAST_LIB,
			183, "locaobject(mLHizikaraHand (rhenka + 1),dotti)", "locaobject(mLHizikaraHand,(rhenka + 1),dotti)"},
	{"henachoco03", "", kPlatformMacintosh, "xn--oj7cxalkre7cjz1d2agc0e8b1cm", kMovieScript, 0, DEFAULT_CAST_LIB,
			196, "locaobject(mRHizikaraHand (rhenka + 1),dotti)", "locaobject(mRHizikaraHand,(rhenka + 1),dotti)"},

	// Same patch applied to the demos, with different line numbers
	{"henachoco03", "Trial Version", kPlatformMacintosh, "ITA Choco", kMovieScript, 0, DEFAULT_CAST_LIB,
			123, "locaobject(mLHizikaraHand (rhenka + 1),dotti)", "locaobject(mLHizikaraHand,(rhenka + 1),dotti)"},
	{"henachoco03", "Trial Version", kPlatformMacintosh, "ITA Choco", kMovieScript, 0, DEFAULT_CAST_LIB,
			136, "locaobject(mRHizikaraHand (rhenka + 1),dotti)", "locaobject(mRHizikaraHand,(rhenka + 1),dotti)"},
	{"henachoco03", "Demo", kPlatformMacintosh, "Muzukashiihon", kMovieScript, 0, DEFAULT_CAST_LIB,
			123, "locaobject(mLHizikaraHand (rhenka + 1),dotti)", "locaobject(mLHizikaraHand,(rhenka + 1),dotti)"},
	{"henachoco03", "Demo", kPlatformMacintosh, "Muzukashiihon", kMovieScript, 0, DEFAULT_CAST_LIB,
			136, "locaobject(mRHizikaraHand (rhenka + 1),dotti)", "locaobject(mRHizikaraHand,(rhenka + 1),dotti)"},

	// The same ambiguous syntax as above, in a different disc
	{"journey2source", "", kPlatformMacintosh, "StartJourney", kScoreScript, 2, DEFAULT_CAST_LIB,
			2, "set DiskChk = FileIO(mnew,\"read\"¬\"The Source:Put Contents on Hard Drive:Journey to the Source:YZ.DATA\")", "set DiskChk = FileIO(mnew,\"read\",¬\"The Source:Put Contents on Hard Drive:Journey to the Source:YZ.DATA\")"},

	// C.H.A.O.S
	{"chaos", "", kPlatformWindows, "Intro", kCastScript, 10, DEFAULT_CAST_LIB,
			9, "rHyperPACo \"blank\", 498, 350 gGenPathWay", "rHyperPACo \"blank\", 498, 350, gGenPathWay"},


	{"smile", "v1.1", kPlatformMacintosh, "SMILE! The Splattering", kScoreScript, 24, DEFAULT_CAST_LIB,
			1, "go to frame \"Info b\"If you have not paid   ", "go to frame \"Info b\""},


	{"amandastories", "", kPlatformWindows, "Shared Cast", kMovieScript, 512, DEFAULT_CAST_LIB,
			55, "    set mytest1 = FileIO(mnew, \"read\" mymovie)", "    set mytest1 = FileIO(mnew, \"read\", mymovie)"},
	{"amandastories", "", kPlatformWindows, "Shared Cast", kMovieScript, 512, DEFAULT_CAST_LIB,
			63, "      set mytest2 = FileIO(mnew, \"read\" mymovie)", "      set mytest2 = FileIO(mnew, \"read\", mymovie)"},
	{"amandastories", "", kPlatformWindows, "Shared Cast", kMovieScript, 512, DEFAULT_CAST_LIB,
			70, "          set mytest3 = FileIO(mnew, \"read\" mymovie)", "          set mytest3 = FileIO(mnew, \"read\", mymovie)"},
	{"amandastories", "", kPlatformWindows, "ASstart", kMovieScript, 0, DEFAULT_CAST_LIB,
			5, "  set mytest = FileIO(mnew, \"read\" mymovie)", "  set mytest = FileIO(mnew, \"read\", mymovie)"},
	{"amandastories", "", kPlatformWindows, "ASstart", kMovieScript, 0, DEFAULT_CAST_LIB,
			11, "    set mytest2 = FileIO(mnew, \"read\" mymovie)", "    set mytest2 = FileIO(mnew, \"read\", mymovie)"},
	{"amandastories", "", kPlatformWindows, "ASstart", kMovieScript, 0, DEFAULT_CAST_LIB,
			19, "      set mytest3 = FileIO(mnew, \"read\" mymovie)", "      set mytest3 = FileIO(mnew, \"read\", mymovie)"},


	{"erikotamuraoz", "Demo", kPlatformMacintosh, "Shared Cast", kMovieScript, 391, DEFAULT_CAST_LIB,
			21, "", "end repeat"},


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
		U32String moviename = punycode_decode(patch->movie);
		if (movie.compareToIgnoreCase(moviename) || strcmp(patch->gameId, g_director->getGameId())
				|| (patch->extra && strcmp(patch->extra, g_director->getExtra()))) {
			patch++;
			continue;
		}

		// Now do a safeguard
		if (!line.contains(Common::U32String(patch->orig)) && line.encode().c_str() != Common::U32String()) {
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
