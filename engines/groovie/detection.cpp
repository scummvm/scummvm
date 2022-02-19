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

#include "common/system.h"
#include "common/translation.h"

#include "engines/advancedDetector.h"
#include "groovie/detection.h"
#include "groovie/groovie.h"

using namespace Common;

namespace Groovie {

#define GAMEOPTION_T7G_FAST_MOVIE_SPEED GUIO_GAMEOPTIONS1
#define GAMEOPTION_ORIGINAL_SAVELOAD GUIO_GAMEOPTIONS2
#define GAMEOPTION_EASIER_AI GUIO_GAMEOPTIONS3
#define GAMEOPTION_FINAL_HOUR GUIO_GAMEOPTIONS4

static const DebugChannelDef debugFlagList[] = {
	{Groovie::kDebugVideo, "Video", "Debug video and audio playback"},
	{Groovie::kDebugResource, "Resource", "Debug resource management"},
	{Groovie::kDebugScript, "Script", "Debug the scripts"},
	{Groovie::kDebugUnknown, "Unknown", "Report values of unknown data in files"},
	{Groovie::kDebugHotspots, "Hotspots", "Show the hotspots"},
	{Groovie::kDebugCursor, "Cursor", "Debug cursor decompression / switching"},
	{Groovie::kDebugMIDI, "MIDI", "Debug MIDI / XMIDI files"},
	{Groovie::kDebugScriptvars, "Scriptvars", "Print out any change to script variables"},
	{Groovie::kDebugLogic, "Logic", "Debug the AI puzzles in the logic folder and TLC questionnaires"},
	{Groovie::kDebugFast, "Fast", "Play videos quickly, with no sound (unstable)"},
	DEBUG_CHANNEL_END};

static const PlainGameDescriptor groovieGames[] = {
	// Games
	{"t7g", "The 7th Guest"},
	{"11h", "The 11th Hour: The Sequel to The 7th Guest"},
	{"making11h", "The Making of The 11th Hour"},
	{"clandestiny", "Clandestiny"},
	{"unclehenry", "Uncle Henry's Playhouse"},
	{"tlc", "Tender Loving Care"},

	{nullptr, nullptr}};

const int BASE_FLAGS = ADGF_NO_FLAGS;

#define GROOVIEGAME(id, extra, f1, x1, s1, f2, x2, s2, language, platform, flags, guiOptions, version) \
	{																							\
		{																				\
			id, extra,																	\
			AD_ENTRY2s(f1, x1, s1, f2, x2, s2),											\
			language, platform, (flags),												\
			(guiOptions)																\
		},																				\
		version																			\
	}


#define T7GENTRY(extra, f1, x1, s1, f2, x2, s2, language, platform, flags) \
	GROOVIEGAME("t7g", extra, f1, x1, s1, f2, x2, s2, language, platform, flags, GUIO8(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT, GUIO_NOSFX, GAMEOPTION_T7G_FAST_MOVIE_SPEED, GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_EASIER_AI), kGroovieT7G)

#define T7GNOMIDIENTRY(extra, f1, x1, s1, f2, x2, s2, language, platform, flags, guiOptions) \
	GROOVIEGAME("t7g", extra, f1, x1, s1, f2, x2, s2, language, platform, flags, (GUIO_NOMIDI GUIO_NOASPECT GUIO_NOSFX GAMEOPTION_T7G_FAST_MOVIE_SPEED guiOptions), kGroovieT7G)

#define T11HENTRY(extra, f1, x1, s1, f2, x2, s2, language, platform, flags) \
	GROOVIEGAME("11h", extra, f1, x1, s1, f2, x2, s2, language, platform, flags | ADGF_TESTING, GUIO6(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM GUIO_NOASPECT, GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_EASIER_AI, GAMEOPTION_FINAL_HOUR), kGroovieT11H)

#define T11HDEMOENTRY(extra, f1, x1, s1, f2, x2, s2, language, platform, flags) \
	GROOVIEGAME("11h", extra, f1, x1, s1, f2, x2, s2, language, platform, flags | ADGF_DEMO | ADGF_TESTING, GUIO5(GUIO_NOLAUNCHLOAD, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT), kGroovieT11H)

#define T11HMAKINGOFENTRY(f1, x1, s1, f2, x2, s2, language, platform) \
	GROOVIEGAME("making11h", "", f1, x1, s1, f2, x2, s2, language, platform, ADGF_TESTING, GUIO4(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT), kGroovieT11H)

#define CLANENTRY(extra, f1, x1, s1, f2, x2, s2, language, platform, flags) \
	GROOVIEGAME("clandestiny", extra, f1, x1, s1, f2, x2, s2, language, platform, flags | ADGF_TESTING, GUIO4(GUIO_NOMIDI, GUIO_NOASPECT, GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_EASIER_AI), kGroovieCDY)

#define CLANDEMOENTRY(extra, f1, x1, s1, f2, x2, s2, language, platform, flags) \
	GROOVIEGAME("clandestiny", extra, f1, x1, s1, f2, x2, s2, language, platform, flags | ADGF_DEMO | ADGF_TESTING, GUIO3(GUIO_NOMIDI, GUIO_NOLAUNCHLOAD, GUIO_NOASPECT), kGroovieCDY)

#define UHPENTRY(extra, f1, x1, s1, f2, x2, s2, language, platform, flags) \
	GROOVIEGAME("unclehenry", extra, f1, x1, s1, f2, x2, s2, language, platform, flags | ADGF_TESTING, GUIO3(GUIO_NOMIDI, GUIO_NOASPECT, GAMEOPTION_EASIER_AI), kGroovieUHP)

#define TLCENTRY(extra, f1, x1, s1, f2, x2, s2, language, platform, flags) \
	GROOVIEGAME("tlc", extra, f1, x1, s1, f2, x2, s2, language, platform, flags | ADGF_TESTING | ADGF_CD, GUIO2(GUIO_NOMIDI, GUIO_NOASPECT), kGroovieTLC)

#define TLCDVDENTRY(f1, x1, s1, f2, x2, s2, language, platform) \
	GROOVIEGAME("tlc", _s("Missing game code"), f1, x1, s1, f2, x2, s2, language, platform, ADGF_UNSUPPORTED | ADGF_DVD, GUIO2(GUIO_NOMIDI, GUIO_NOASPECT), kGroovieTLC)

// clang-format off
static const GroovieGameDescription gameDescriptions[] = {
	// groovie.cpp requires the first file to be the main .grv file for v2 games, might as well stick to that convention for v1 games from now on too

/*==== The 7th Guest ====*/
	// The 7th Guest 25th Anniversary
	T7GENTRY("25th Anniversary Edition", "script.grv", "d1b8033b40aa67c076039881eccce90d", 16659,
				"AT.gjd", "bbaa95ce6e600a8ba5b2902326cd11f8", 28827446, EN_ANY, kPlatformWindows, ADGF_UNSTABLE | ADGF_REMASTERED),

	// The 7th Guest DOS English
	T7GENTRY("", "script.grv", "d1b8033b40aa67c076039881eccce90d", 16659,
					"AT.gjd", "bbaa95ce6e600a8ba5b2902326cd11f8", 45171574, EN_ANY, kPlatformDOS, BASE_FLAGS),

	// The 7th Guest Mac English
	T7GENTRY("", "script.grv", nullptr, -1,// FIXMEMD5
					"T7GMac", "acdc4a58dd3f007f65e99b99d78e0bce", 1814029, EN_ANY, kPlatformMacintosh, ADGF_MACRESFORK),

	// The 7th Guest DOS Russian (Akella)
	T7GENTRY("", "script.grv", "d1b8033b40aa67c076039881eccce90d", 16659,
					"intro.gjd", nullptr, 31711554, RU_RUS, kPlatformDOS, BASE_FLAGS),
	
	// The 7th Guest iOS English
	T7GNOMIDIENTRY("", "script.grv", "d1b8033b40aa67c076039881eccce90d", 16659,
						"SeventhGuest", nullptr, -1, EN_ANY, kPlatformIOS, BASE_FLAGS, GAMEOPTION_ORIGINAL_SAVELOAD GAMEOPTION_EASIER_AI),

	// "Guest" early DOS demo
	T7GNOMIDIENTRY("The early \"Guest\" demo is not supported", "playtlc.exe", "9cff0e9649ddf49e9fe5168730aa7201", 254768,
						"FACE.VDX", "614f820265274dc99d8869de67df1718", 1382814,
						EN_ANY, kPlatformDOS, ADGF_DEMO | ADGF_UNSUPPORTED, GUIO_NONE),

/*==== The 11th Hour ====*/
	// The 11th Hour DOS/Windows English (Available on Steam)
	T11HENTRY("", "script.grv", "bdb9a783d4debe477ac3856adc454c17", 62447, "introd1.gjd", nullptr, -1, EN_ANY, kPlatformWindows, BASE_FLAGS),

	// The 11th Hour DOS/Windows German
	T11HENTRY("", "script.grv", "560e90b47054639668e44a8b365fbe26", 62447, "introd1.gjd", nullptr, -1, DE_DEU, kPlatformWindows, BASE_FLAGS),

	// The 11th Hour DOS/Windows French
	T11HENTRY("", "script.grv", "752c0a8ea62a1207c8583f3dbc16e6ef", 62447, "introd1.gjd", nullptr, -1, FR_FRA, kPlatformWindows, BASE_FLAGS),

	// The 11th Hour Mac English
	T11HENTRY("", "script.grv", "bdb9a783d4debe477ac3856adc454c17", 62447,
					"The 11th Hour Installer", "bcdb4040b27f15b18f39fb9e496d384a", 1002987, EN_ANY, kPlatformMacintosh, BASE_FLAGS),

	// The 11th Hour Mac English (Installed)
	T11HENTRY("Installed", "script.grv", "bdb9a783d4debe477ac3856adc454c17", 62447,
					"el01.mov", "70f42dfc25b1488a08011dc45bb5145d", 6039, EN_ANY, kPlatformMacintosh, BASE_FLAGS),

/*==== The 11th Hour Demos ====*/
	// The 11th Hour DOS Interactive Demo English https://archive.org/details/11th_Hour_demo
	T11HDEMOENTRY("Interactive Demo", "demo.grv", "5faec559b9abf18cf143751b420208dc", 15991,
						"dvmod1a.gjd", "e304fe68f95c54fc82d785768e372892", 8068568, EN_ANY, kPlatformDOS, BASE_FLAGS),

	// The 11th Hour DOS Interactive Demo English (packaged with the Non-Interactive Demo) https://archive.org/details/11HDEMO
	T11HDEMOENTRY("Interactive Demo", "demo.grv", "824b1a051f841a50ab7a6b4c10180bbc", 15898,
						"dvmod1a.gjd", "e304fe68f95c54fc82d785768e372892", 8068568, EN_ANY, kPlatformDOS, BASE_FLAGS),

	// The 11th Hour DOS Non-Interactive Demo English https://archive.org/details/11HDEMO
	T11HDEMOENTRY("Non-Interactive Demo", "niloop.grv", "b4c35a2a6ebaf72fbd830b590d48f8ea", 456,
						"dvmod1b.gjd", "43eb268ef6d64a75b9846df5be453d30", 11264100, EN_ANY, kPlatformDOS, BASE_FLAGS),

/*==== The Making of The 11th Hour ====*/
	// all are in english even if they came packaged with alternate language versions of the game
	// I removed the hash check for now so they all match with a single entry since the language field is useless here
	
	// The Making of The 11th Hour DOS/Windows
	T11HMAKINGOFENTRY("makingof.grv", nullptr, 994, "zmakd2a.gjd", nullptr, -1, EN_ANY, kPlatformWindows),

	/*// The Making of The 11th Hour DOS/Windows English
	T11HMAKINGOFENTRY(GROOVIEFILES("makingof.grv", "12e1e5eef2c7a9536cd12ac800b31408", 994, "zmakd2a.gjd"), EN_ANY, kPlatformWindows),

	// The Making of The 11th Hour DOS/Windows German
	T11HMAKINGOFENTRY(GROOVIEFILES("makingof.grv", "03492c6ad3088b3f9f51a3eaba6b8c8e", 994, "zmakd2a.gjd"), EN_ANY, kPlatformWindows),

	// The Making of The 11th Hour DOS/Windows French
	T11HMAKINGOFENTRY(GROOVIEFILES("makingof.grv", "77c4ae4deb0e323ccd7dcca0f99de2b9", 994, "zmakd2a.gjd"), EN_ANY, kPlatformWindows),

	// The Making of The 11th Hour Macintosh English
	T11HMAKINGOFENTRY(AD_ENTRY2s("makingof.grv", "12e1e5eef2c7a9536cd12ac800b31408", 994,
					   "The 11th Hour Installer", "bcdb4040b27f15b18f39fb9e496d384a", 1002987), EN_ANY, kPlatformMacintosh),*/

	// The Making of The 11th Hour Macintosh
	T11HMAKINGOFENTRY("makingof.grv", nullptr, 994,
					   "The 11th Hour Installer", nullptr, 1002987, EN_ANY, kPlatformMacintosh),

/*==== Clandestiny Demos ====*/
	// Clandestiny Trailer Macintosh English
	CLANDEMOENTRY("Trailer", "trailer.grv", "a7c8bdc4e8bff621f4f50928a95eaaba", 6,
						"The 11th Hour Installer", "bcdb4040b27f15b18f39fb9e496d384a", 1002987, EN_ANY, kPlatformMacintosh, BASE_FLAGS),

	// Clandestiny PC Demo English https://archive.org/details/Clandestiny_demo
	CLANDEMOENTRY("Demo", "clandemo.grv", "faa863738da1c93673ed58a4b9597a63", 6744, "cddemo.gjd", nullptr, -1, EN_ANY, kPlatformWindows, BASE_FLAGS),

	// Clandestiny PC Trailer English https://downloads.scummvm.org/frs/demos/groovie/clandestiny-dos-ni-demo-en.zip
	CLANDEMOENTRY("Trailer", "trailer.grv", "a7c8bdc4e8bff621f4f50928a95eaaba", 6,
						"zclan.gjd", "4a7258166916fcc0d217c8f21fa3cc79", 20454932, EN_ANY, kPlatformWindows, BASE_FLAGS),

/*==== Clandestiny ====*/
	// Clandestiny PC English
	CLANENTRY("", "clanmain.grv", "dd424120fa1daa9d6b576d0ba22a4936", 54253, "ACT01MUS.MPG", nullptr, -1, EN_ANY, kPlatformWindows, BASE_FLAGS),

	// Clandestiny Mac/iOS App Store
	CLANENTRY("Mac/iOS", "CLANMAIN.GRV", "dd424120fa1daa9d6b576d0ba22a4936", 54253, "ACT01MUS.m4a", nullptr, -1, EN_ANY, kPlatformUnknown, BASE_FLAGS),

/*==== Uncle Henry's Playhouse ====*/
	// Uncle Henry's Playhouse PC English (1996-09-13)
	UHPENTRY("", "tpot.grv", "849dc7e5309e1b9acf72d8abc9e145df", 11693, "trt7g.gjd", nullptr, -1, EN_ANY, kPlatformWindows, BASE_FLAGS),

	// Uncle Henry's Playhouse PC German
	// Funsoft (1997-02-14)
	UHPENTRY("", "tpot.grv", "30d06af7669004f1ea7a99a5ebdb6935", 10469, "trt7g.gjd", nullptr, -1, DE_DEU, kPlatformWindows, BASE_FLAGS),

/*==== Tender Loving Care ====*/
	// Tender Loving Care PC English (CD-ROM 1998-05-01)
	TLCENTRY("CD", "tlcmain.grv", "47c235155de5103e72675fe7294720b8", 17479, "tlcnav.gjd", nullptr, -1, EN_ANY, kPlatformWindows, ADGF_CD),

	// Tender Loving Care PC English (DVD-ROM 1998-06-12)
	TLCDVDENTRY("tlcmain.grv", "8a591c47d24dde38615e6ea2e79b51a5", 17375, "tlcnav.gjd", nullptr, -1, EN_ANY, kPlatformWindows),

	// Tender Loving Care PC English (DVD-ROM 1998-08-26)
	TLCDVDENTRY("tlcmain.grv", "151af191015beb6f662919153e6c28d8", 17379, "tlcnav.gjd", nullptr, -1, EN_ANY, kPlatformWindows),

	// Tender Loving Care PC German (CD-ROM 1998-04-08)
	// "Die Versuchung", Funsoft
	TLCENTRY("CD", "tlcmain.grv", "3459a25a5f31b430d320cba2e47d3c98", 17353, "tlcnav.gjd", nullptr, -1, DE_DEU, kPlatformWindows, BASE_FLAGS),

	// Tender Loving Care PC German (DVD-ROM 1998-08-23)
	// "Die Versuchung", Conspiracy Entertainment Europe
	TLCDVDENTRY("tlcmain.grv", "50e62d41ad2cddd0f31ea0a542338387", 17344, "tlcnav.gjd", nullptr, -1, DE_DEU, kPlatformWindows),

	// Tender Loving Care PC Demo German (CD-ROM 1998-03-23)
	// https://archive.org/details/Tender_Loving_Care_demo
	{
		{
			"tlc", "Demo",
			AD_ENTRY2s("tlcmain.grv", "6ec818f595eedca6570280af0c681642", 17361, "tlcnav.gjd", nullptr, -1),
			DE_DEU, kPlatformWindows, ADGF_DEMO | ADGF_TESTING,
			GUIO3(GUIO_NOMIDI, GUIO_NOASPECT, GUIO_NOLAUNCHLOAD)
		},
		kGroovieTLC
	},

	// Tender Loving Care PC Trailer (CD-ROM 1998-03-23)
	// On the same disc with the above German demo
	{
		{
			"tlc", "Trailer",
			AD_ENTRY2s("preview.grv", "d95401509a0ef251e8c340737edf728c", 19,
							"drama1.gjd", "2a4ca274d832675248e51baf7e537bb3", 390727225),
			UNK_LANG, kPlatformWindows, ADGF_DEMO | ADGF_TESTING,
			GUIO3(GUIO_NOMIDI, GUIO_NOASPECT, GUIO_NOLAUNCHLOAD)
		},
		kGroovieTLC
	},

	{AD_TABLE_END_MARKER, kGroovieT7G}
};

static const char *directoryGlobs[] = {
	"MIDI",
	"GROOVIE",
	"MEDIA",
	nullptr
};

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_T7G_FAST_MOVIE_SPEED,
		{
			_s("Fast movie speed"),
			_s("Play movies at an increased speed"),
			"fast_movie_speed",
			false
		}
	},

	{
		GAMEOPTION_ORIGINAL_SAVELOAD,
		{
			_s("Use original save/load screens"),
			_s("Use the original save/load screens instead of the ScummVM ones"),
			"originalsaveload",
			false
		}
	},

	{
		GAMEOPTION_EASIER_AI,
		{
			_s("Easier AI"),
			_s("Decrease the difficulty of AI puzzles"),
			"easier_ai",
			false
		}
	},

	{
		GAMEOPTION_FINAL_HOUR,
		{
			_s("Updated Credits Music"),
			_s("Play the song The Final Hour during the credits instead of reusing MIDI songs"),
			"credits_music",
			false
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};
// clang-format on

class GroovieMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	GroovieMetaEngineDetection() : AdvancedMetaEngineDetection(gameDescriptions, sizeof(GroovieGameDescription), groovieGames, optionsList) {
		// Use kADFlagUseExtraAsHint in order to distinguish the 11th hour from
		// its "Making of" as well as the Clandestiny Trailer; they all share
		// the same MD5.
		// TODO: Is this the only reason, or are there others (like the three
		// potentially sharing a single directory) ? In the former case, then
		// perhaps a better solution would be to add additional files
		// to the detection entries. In the latter case, this TODO should be
		// replaced with an according explanation.
		_flags = kADFlagUseExtraAsHint;
		_guiOptions = GUIO2(GUIO_NOSUBTITLES, GUIO_NOASPECT);

		// Need MIDI directory to detect 11H Mac Installed
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
	}

	const char *getEngineId() const override {
		return "groovie";
	}

	const char *getName() const override {
		return "Groovie";
	}

	const char *getOriginalCopyright() const override {
		return "Groovie Engine (C) 1990-1996 Trilobyte";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

} // End of namespace Groovie

REGISTER_PLUGIN_STATIC(GROOVIE_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, Groovie::GroovieMetaEngineDetection);
