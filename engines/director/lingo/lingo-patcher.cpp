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

	// Typo
	{"rodneyfs", nullptr, kPlatformMacintosh, "Shared Cast", kMovieScript, 496, DEFAULT_CAST_LIB,
			7, "if the soundLevel <> 7 then set the the soundLevel to 7", "if the soundLevel <> 7 then set the soundLevel to 7"},

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


	// Hack to fix the undefined sprite collision behaviour relied on by the boar hunt
	{"wrath", "", kPlatformWindows, "57AM1", kMovieScript, 1, DEFAULT_CAST_LIB,
			385, "(StartV57a-6) <=  YesV57a", "    if sprite 5 intersects 3 and StartV57a <=  YesV57a + 16 then"},
	{"wrath", "", kPlatformMacintosh, "Wrath:57AM1", kMovieScript, 1, DEFAULT_CAST_LIB,
			382, "(StartV57a-6) <=  YesV57a", "    if sprite 5 intersects 3 and StartV57a <=  YesV57a + 16 then"},


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


	{"cts", "Metric", kPlatformMacintosh, "CTS", kMovieScript, 0, DEFAULT_CAST_LIB,
			307, "    alert(\"Sorry. No keyword was entered for this recipe.)", "    alert(\"Sorry. No keyword was entered for this recipe.\")"},
	{"cts", "Imperial", kPlatformMacintosh, "CTS", kMovieScript, 0, DEFAULT_CAST_LIB,
			307, "    alert(\"Sorry. No keyword was entered for this recipe.)", "    alert(\"Sorry. No keyword was entered for this recipe.\")"},


	// garbage script
	{"refixion2", "", kPlatformMacintosh, "data:Movie:ROgo", kScoreScript, 3, DEFAULT_CAST_LIB,
			1, "Are you sure to cut off  KANJI Talk", ""},


	{nullptr, nullptr, kPlatformUnknown, nullptr, kNoneScript, 0, 0, 0, nullptr, nullptr}
};

/*
 * Cosmology of Kyoto has a text entry system, however for the English version
 * at least you are very unlikely to guess the correct sequence of letters that
 * constitute a valid answer. This is an attempt to make things fairer by removing
 * the need for precise whitespace and punctuation. As a fallback, "yes" should
 * always mean a yes response, and "no" should always mean a no response.
 */

const char *kyotoTextEntryFix = " \
on scrubInput inputString \r\
  set result = \"\" \r\
  repeat with x = 1 to the number of chars in inputString \r\
    if chars(inputString, x, x) = \" \" then continue \r\
	else if chars(inputString, x, x) = \".\" then continue \r\
	else if chars(inputString, x, x) = \"!\" then continue \r\
	else if chars(inputString, x, x) = \"?\" then continue \r\
	else if chars(inputString, x, x) = \"。\" then continue \r\
	else \r\
      set result = result & char x of inputString \r\
	end if \r\
  end repeat \r\
  return result \r\
end \r\
\r\
on checkkaiwa kaiwatrue, kaiwafalse \r\
  global myparadata \r\
  if (keyCode() <> 36) and (keyCode() <> 76) then \r\
    exit \r\
  end if \r\
  put \"Original YES options: \" & kaiwatrue \r\
  put \"Original NO options: \" & kaiwafalse \r\
  -- pre-scrub all input and choices to remove effect of whitespace/punctuation \r\
  set kaiwaans = scrubInput(field \"KaiwaWindow\") \r\
  set kaiwatrue = scrubInput(kaiwatrue) \r\
  set kaiwafalse = scrubInput(kaiwafalse) \r\
  -- yes and no should always give consistent results \r\
  if kaiwaans = \"yes\" then \r\
    return \"YES\" \r\
  else if kaiwaans = \"no\" then \r\
    return \"NO\" \r\
  end if \r\
  repeat with y = 1 to the number of items in kaiwatrue \r\
    if item y of kaiwatrue starts kaiwaans then \r\
      when keyDown then CheckQuit \r\
      put EMPTY into field \"KaiwaWindow\" \r\
      return \"YES\" \r\
    end if \r\
  end repeat \r\
  repeat with n = 1 to the number of items in kaiwafalse \r\
    if item n of kaiwafalse starts kaiwaans then \r\
      when keyDown then CheckQuit \r\
      put EMPTY into field \"KaiwaWindow\" \r\
      return \"NO\" \r\
    end if \r\
  end repeat \r\
    set kaiwafool to scrubInput(\"あほんだら,ばか,うんこ,しっこ,しね,死ね,うるさい,うるせえ,\" & \"fool,simpleton,stupid person,kill,Shut up!,Get out of my hair!\") \r\
  repeat with f = 1 to the number of items in kaiwafool \r\
    if item f of kaiwafool starts kaiwaans then \r\
      myparadata(maddparadata, 2, 1) \r\
      when keyDown then CheckQuit \r\
      put EMPTY into field \"KaiwaWindow\" \r\
      return \"error\" \r\
    end if \r\
  end repeat \r\
  when keyDown then CheckQuit \r\
  put EMPTY into field \"KaiwaWindow\" \r\
  return \"error\" \r\
end \r\
";

/*
 * Virtual Nightclub will try and list all the files from all 26 drive letters
 * to determine which has the CD. This works, but takes forever.
 */

const char *vncSkipDetection = " \
global cdDriveLetter, gMultiDisk \r\
on findVNCVolume \r\
  set cdDriveLetter to \"D\" \r\
  set gMultiDisk to 1 \r\
  return 1 \r\
end \r\
";

/*
 * Virtual Nightclub has a number of cheat codes for debugging.
 * These are normally enabled by pressing Option + 0, however the
 * released game has this code stubbed out with a return.
 */

const char *vncEnableCheats = " \
on togCh\r\
  if getFlag(#cheats) then\r\
    setFlag(#cheats, 0)\r\
	setMode(0) -- disable debug logging\r\
    set the foreColor of field \"viewName_cast\" to 255\r\
    alert(\"VNC Cheats off\")\r\
  else\r\
    if platform() < 256 then\r\
      set the textFont of field \"viewName_cast\" to \"Monaco\"\r\
    end if\r\
    set the foreColor of field \"viewName_cast\" to 172\r\
    set the textSize of field \"viewName_cast\" to 9\r\
    setFlag(#cheats)\r\
	setMode(10) -- enable debug logging\r\
    alert(\"VNC Cheats on\")\r\
  end if\r\
end\r\
";

/* AMBER: Journeys Beyond has a check to ensure that the CD and hard disk data are on
 * different drive letters. ScummVM will pretend that every drive letter contains the
 * game contents, so we need to hotpatch the CD detection routine to return D:.
 */
const char *amberDriveDetectionFix = " \
on GetCDLetter tagFile, discNumber\r\
  return \"D:\"\r\
end \r\
";

/* Frankenstein: Through The Eyes Of The Monster uses a projector FRANKIE.EXE, which calls an
 * identically-named submovie FRANKIE.DIR. For now we can work around this mess by referring to
 * the full "path" of the embedded submovie so path detection doesn't collide with FRANKIE.EXE.
 */
const char *frankensteinSwapFix = " \
on exitFrame \r\
  go(1, \"FRANKIE\\FRANKIE.DIR\")\r\
end \r\
";

struct ScriptHandlerPatch {
	const char *gameId;
	const char *extra;
	Common::Platform platform; // Specify kPlatformUnknown for skipping platform check
	const char *movie;
	ScriptType type;
	uint16 id;
	uint16 castLib;
	const char **handlerBody;
} const scriptHandlerPatches[] = {
	{"kyoto", nullptr, kPlatformWindows, "ck_data\\dd_dairi\\shared.dxr", kMovieScript, 906, DEFAULT_CAST_LIB, &kyotoTextEntryFix},
	{"kyoto", nullptr, kPlatformWindows, "ck_data\\findfldr\\shared.dxr", kMovieScript, 802, DEFAULT_CAST_LIB, &kyotoTextEntryFix},
	{"kyoto", nullptr, kPlatformWindows, "ck_data\\ichi\\shared.dxr", kMovieScript, 906, DEFAULT_CAST_LIB, &kyotoTextEntryFix},
	{"kyoto", nullptr, kPlatformWindows, "ck_data\\jigoku\\shared.dxr", kMovieScript, 840, DEFAULT_CAST_LIB, &kyotoTextEntryFix},
	{"kyoto", nullptr, kPlatformWindows, "ck_data\\kusamura\\shared.dxr", kMovieScript, 906, DEFAULT_CAST_LIB, &kyotoTextEntryFix},
	{"kyoto", nullptr, kPlatformWindows, "ck_data\\map01\\shared.dxr", kMovieScript, 906, DEFAULT_CAST_LIB, &kyotoTextEntryFix},
	{"kyoto", nullptr, kPlatformWindows, "ck_data\\map02\\shared.dxr", kMovieScript, 906, DEFAULT_CAST_LIB, &kyotoTextEntryFix},
	{"kyoto", nullptr, kPlatformWindows, "ck_data\\map03\\shared.dxr", kMovieScript, 906, DEFAULT_CAST_LIB, &kyotoTextEntryFix},
	{"kyoto", nullptr, kPlatformWindows, "ck_data\\map04\\shared.dxr", kMovieScript, 906, DEFAULT_CAST_LIB, &kyotoTextEntryFix},
	{"kyoto", nullptr, kPlatformWindows, "ck_data\\opening\\shared.dxr", kMovieScript, 802, DEFAULT_CAST_LIB, &kyotoTextEntryFix},
	{"kyoto", nullptr, kPlatformWindows, "ck_data\\rajoumon\\shared.dxr", kMovieScript, 840, DEFAULT_CAST_LIB, &kyotoTextEntryFix},
	{"kyoto", nullptr, kPlatformWindows, "ck_data\\rokudou\\shared.dxr", kMovieScript, 846, DEFAULT_CAST_LIB, &kyotoTextEntryFix},
	{"vnc", nullptr, kPlatformWindows, "VNC\\VNC.EXE", kMovieScript, 57, DEFAULT_CAST_LIB, &vncSkipDetection},
	{"vnc", nullptr, kPlatformWindows, "VNC2\\SHARED.DXR", kMovieScript, 1248, DEFAULT_CAST_LIB, &vncEnableCheats},
	{"amber", nullptr, kPlatformWindows, "AMBER_F\\AMBER_JB.EXE", kMovieScript, 7, DEFAULT_CAST_LIB, &amberDriveDetectionFix},
	{"frankenstein", nullptr, kPlatformWindows, "FRANKIE.EXE", kScoreScript, 21, DEFAULT_CAST_LIB, &frankensteinSwapFix},
	{nullptr, nullptr, kPlatformUnknown, nullptr, kNoneScript, 0, 0, nullptr},

};

void LingoArchive::patchScriptHandler(ScriptType type, CastMemberID id) {
	const ScriptHandlerPatch *patch = scriptHandlerPatches;
	Common::String movie = g_director->getCurrentPath() + cast->getMacName();

	// So far, we have not many patches, so do linear lookup
	while (patch->gameId) {
		// First, we do cheap comparisons
		if (patch->type != type || patch->id != id.member || patch->castLib != id.castLib ||
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
		patchCode(Common::U32String(*patch->handlerBody), patch->type, patch->id);
		patch++;
	}
}


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
