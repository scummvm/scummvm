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

namespace Groovie {

#define GAMEOPTION_T7G_FAST_MOVIE_SPEED  GUIO_GAMEOPTIONS1
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
	DEBUG_CHANNEL_END
};

static const PlainGameDescriptor groovieGames[] = {
	// Games
	{"t7g", "The 7th Guest"},
	{"11h", "The 11th Hour: The Sequel to The 7th Guest"},
	{"clandestiny", "Clandestiny"},
	{"unclehenry", "Uncle Henry's Playhouse"},
	{"tlc", "Tender Loving Care"},

	{nullptr, nullptr}
};

// clang-format off
static const GroovieGameDescription gameDescriptions[] = {
	// groovie.cpp requires the first file to be the main .grv file for v2 games, might as well stick to that convention for v1 games from now on too

	// The 7th Guest 25th Anniversary
	{
		{
			"t7g", "25th Anniversary Edition",
			AD_ENTRY2s("script.grv", "d1b8033b40aa67c076039881eccce90d", 16659,
						"AT.gjd", "bbaa95ce6e600a8ba5b2902326cd11f8", 28827446),
			Common::EN_ANY, Common::kPlatformWindows, ADGF_UNSTABLE,
			GUIO8(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT, GUIO_NOSFX, GAMEOPTION_T7G_FAST_MOVIE_SPEED, GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_EASIER_AI)
		},
		kGroovieT7G
	},

	// The 7th Guest DOS English
	{
		{
			"t7g", "",
			AD_ENTRY2s("script.grv", "d1b8033b40aa67c076039881eccce90d", 16659,
						"AT.gjd", "bbaa95ce6e600a8ba5b2902326cd11f8", 45171574),
			Common::EN_ANY, Common::kPlatformDOS, ADGF_NO_FLAGS,
			GUIO8(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT, GUIO_NOSFX, GAMEOPTION_T7G_FAST_MOVIE_SPEED, GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_EASIER_AI)
		},
		kGroovieT7G
	},

	// The 7th Guest Mac English
	{
		{
			"t7g", "",
			AD_ENTRY2s("script.grv", nullptr, -1,				// FIXMEMD5
						"T7GMac", "acdc4a58dd3f007f65e99b99d78e0bce", 1814029),
			Common::EN_ANY, Common::kPlatformMacintosh, ADGF_MACRESFORK,
			GUIO8(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT, GUIO_NOSFX, GAMEOPTION_T7G_FAST_MOVIE_SPEED, GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_EASIER_AI)
		},
		kGroovieT7G
	},

	// The 7th Guest DOS Russian (Akella)
	{
		{
			"t7g", "",
			AD_ENTRY2s("script.grv", "d1b8033b40aa67c076039881eccce90d", 16659,
						"intro.gjd", nullptr, 31711554),
			Common::RU_RUS, Common::kPlatformDOS, ADGF_NO_FLAGS,
			GUIO8(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT, GUIO_NOSFX, GAMEOPTION_T7G_FAST_MOVIE_SPEED, GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_EASIER_AI)
		},
		kGroovieT7G
	},

	// The 7th Guest iOS English
	{
		{
			"t7g", "",
			AD_ENTRY2s("script.grv", "d1b8033b40aa67c076039881eccce90d", 16659,
						"SeventhGuest", nullptr, -1),
			Common::EN_ANY, Common::kPlatformIOS, ADGF_NO_FLAGS,
			GUIO6(GUIO_NOMIDI, GUIO_NOASPECT, GUIO_NOSFX, GAMEOPTION_T7G_FAST_MOVIE_SPEED, GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_EASIER_AI)
		},
		kGroovieT7G
	},

	// "Guest" early DOS demo
	{
		{
			"t7g", "The early \"Guest\" demo is not supported",
			AD_ENTRY1s("playtlc.exe", "9cff0e9649ddf49e9fe5168730aa7201", 254768),
			Common::EN_ANY, Common::kPlatformDOS, ADGF_DEMO | ADGF_UNSUPPORTED,
			GUIO3(GUIO_NOMIDI, GUIO_NOASPECT, GAMEOPTION_T7G_FAST_MOVIE_SPEED)
		},
		kGroovieT7G
	},

	// The 11th Hour DOS/Windows English (Available on Steam)
	{
		{
			"11h", "",
			AD_ENTRY2s("script.grv", "bdb9a783d4debe477ac3856adc454c17", 62447,
						"introd1.gjd", nullptr, -1),
			Common::EN_ANY, Common::kPlatformWindows, ADGF_UNSTABLE,
			GUIO7(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT, GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_EASIER_AI, GAMEOPTION_FINAL_HOUR)
		},
		kGroovieT11H
	},

	// The 11th Hour Mac English
	{
		{
			"11h", "",
			AD_ENTRY2s("script.grv", "bdb9a783d4debe477ac3856adc454c17", 62447,
					"The 11th Hour Installer", "bcdb4040b27f15b18f39fb9e496d384a", 1002987),
			Common::EN_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE,
			GUIO7(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT, GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_EASIER_AI, GAMEOPTION_FINAL_HOUR)
		},
		kGroovieT11H
	},

	// The 11th Hour Mac English (Installed)
	{
		{
			"11h", "Installed",
			AD_ENTRY2s("script.grv", "bdb9a783d4debe477ac3856adc454c17", 62447,
					"el01.mov", "70f42dfc25b1488a08011dc45bb5145d", 6039),
			Common::EN_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE,
			GUIO7(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT, GAMEOPTION_ORIGINAL_SAVELOAD, GAMEOPTION_EASIER_AI, GAMEOPTION_FINAL_HOUR)
		},
		kGroovieT11H
	},

	// The 11th Hour DOS Interactive Demo English https://archive.org/details/11th_Hour_demo
	{
		{
			"11h", "Interactive Demo",
			AD_ENTRY2s("demo.grv", "5faec559b9abf18cf143751b420208dc", 15991,
						"dvmod1a.gjd", "e304fe68f95c54fc82d785768e372892", 8068568),
			Common::EN_ANY, Common::kPlatformDOS, ADGF_DEMO | ADGF_UNSTABLE,
			GUIO5(GUIO_NOLAUNCHLOAD, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT)
		},
		kGroovieT11H
	},

	// The 11th Hour DOS Interactive Demo English (packaged with the Non-Interactive Demo) https://archive.org/details/11HDEMO
	{
		{
			"11h", "Interactive Demo",
			AD_ENTRY2s("demo.grv", "824b1a051f841a50ab7a6b4c10180bbc", 15898,
						"dvmod1a.gjd", "e304fe68f95c54fc82d785768e372892", 8068568),
			Common::EN_ANY, Common::kPlatformDOS, ADGF_DEMO | ADGF_UNSTABLE,
			GUIO5(GUIO_NOLAUNCHLOAD, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT)
		},
		kGroovieT11H
	},

	// The 11th Hour DOS Non-Interactive Demo English https://archive.org/details/11HDEMO
	{
		{
			"11h", "Non-Interactive Demo",
			AD_ENTRY2s("niloop.grv", "b4c35a2a6ebaf72fbd830b590d48f8ea", 456,
						"dvmod1b.gjd", "43eb268ef6d64a75b9846df5be453d30", 11264100),
			Common::EN_ANY, Common::kPlatformDOS, ADGF_DEMO | ADGF_UNSTABLE,
			GUIO5(GUIO_NOLAUNCHLOAD, GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT)
		},
		kGroovieT11H
	},

	// The Making of The 11th Hour DOS/Windows English
	{
		{
			"11h", "Making Of",
			AD_ENTRY2s("makingof.grv", "12e1e5eef2c7a9536cd12ac800b31408", 994,
						"zmakd2a.gjd", nullptr, -1),
			Common::EN_ANY, Common::kPlatformWindows, ADGF_UNSTABLE,
			GUIO3(GUIO_NOMIDI, GUIO_NOLAUNCHLOAD, GUIO_NOASPECT)
		},
		kGroovieT11H
	},

	// The Making of The 11th Hour Macintosh English
	{
		{
			"11h", "Making Of",
			AD_ENTRY2s("makingof.grv", "12e1e5eef2c7a9536cd12ac800b31408", 994,
					   "The 11th Hour Installer", "bcdb4040b27f15b18f39fb9e496d384a", 1002987),
			Common::EN_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE,
			GUIO4(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT)
		},
		kGroovieT11H
	},

	// The Making of The 11th Hour Macintosh English (Installed)
	{
		{
			"11h", "Making Of (Installed)",
			AD_ENTRY2s("makingof.grv", "12e1e5eef2c7a9536cd12ac800b31408", 994,
					   "el01.mov",	"70f42dfc25b1488a08011dc45bb5145d", 6039),
			Common::EN_ANY, Common::kPlatformMacintosh, ADGF_UNSTABLE,
			GUIO4(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT)
		},
		kGroovieT11H
	},

	// Clandestiny Trailer DOS/Windows English
	{
		{
			"clandestiny", "Trailer",
			AD_ENTRY2s("trailer.grv", "a7c8bdc4e8bff621f4f50928a95eaaba", 6,
						"atpuz.gjd", nullptr, -1),
			Common::EN_ANY, Common::kPlatformWindows, ADGF_DEMO | ADGF_UNSTABLE,
			GUIO3(GUIO_NOMIDI, GUIO_NOLAUNCHLOAD, GUIO_NOASPECT)
		},
		kGroovieCDY
	},

	// Clandestiny Trailer Macintosh English
	{
		{
			"clandestiny", "Trailer",
			AD_ENTRY2s("trailer.grv", "a7c8bdc4e8bff621f4f50928a95eaaba", 6,
						"The 11th Hour Installer", "bcdb4040b27f15b18f39fb9e496d384a", 1002987),
			Common::EN_ANY, Common::kPlatformMacintosh, ADGF_DEMO | ADGF_UNSTABLE,
			GUIO4(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT)
		},
		kGroovieCDY
	},

	// Clandestiny Trailer Macintosh English (Installed)
	{
		{
			"clandestiny", "Trailer (Installed)",
			AD_ENTRY2s("trailer.grv", "a7c8bdc4e8bff621f4f50928a95eaaba", 6,
						"el01.mov", "70f42dfc25b1488a08011dc45bb5145d", 6039),
			Common::EN_ANY, Common::kPlatformMacintosh, ADGF_DEMO | ADGF_UNSTABLE,
			GUIO4(GUIO_MIDIADLIB, GUIO_MIDIMT32, GUIO_MIDIGM, GUIO_NOASPECT)
		},
		kGroovieCDY
	},

	// Clandestiny PC Demo English https://archive.org/details/Clandestiny_demo
	{
		{
			"clandestiny", "Demo",
			AD_ENTRY2s("clandemo.grv", "faa863738da1c93673ed58a4b9597a63", 6744,
						"cddemo.gjd", nullptr, -1),
			Common::EN_ANY, Common::kPlatformWindows, ADGF_DEMO | ADGF_UNSTABLE,
			GUIO3(GUIO_NOMIDI, GUIO_NOLAUNCHLOAD, GUIO_NOASPECT)
		},
		kGroovieCDY
	},

	// Clandestiny PC English
	{
		{
			"clandestiny", "",
			AD_ENTRY2s("clanmain.grv", "dd424120fa1daa9d6b576d0ba22a4936", 54253,
						"ACT01MUS.MPG", nullptr, -1),
			Common::EN_ANY, Common::kPlatformWindows, ADGF_UNSTABLE,
			GUIO3(GUIO_NOMIDI, GUIO_NOASPECT, GAMEOPTION_EASIER_AI)
		},
		kGroovieCDY
	},

	// Clandestiny Mac/iOS App Store
    {
        {
            "clandestiny", "Mac/iOS",
            AD_ENTRY2s("CLANMAIN.GRV", "dd424120fa1daa9d6b576d0ba22a4936", 54253,
                       "ACT01MUS.m4a", nullptr, -1),
            Common::EN_ANY, Common::kPlatformUnknown, ADGF_UNSTABLE,
            GUIO3(GUIO_NOMIDI, GUIO_NOASPECT, GAMEOPTION_EASIER_AI)
        },
        kGroovieCDY
    },

	// Uncle Henry's Playhouse PC English
	{
		{
			"unclehenry", "",
			AD_ENTRY2s("tpot.grv", "849dc7e5309e1b9acf72d8abc9e145df", 11693,
						"trt7g.gjd", nullptr, -1),
			Common::EN_ANY, Common::kPlatformWindows, ADGF_UNSTABLE,
			GUIO3(GUIO_NOMIDI, GUIO_NOASPECT, GAMEOPTION_EASIER_AI)
		},
		kGroovieUHP
	},

	// Tender Loving Care PC English (CD-ROM)
	{
		{
			"tlc", "CD",
			AD_ENTRY2s("tlcmain.grv", "47c235155de5103e72675fe7294720b8", 17479,
						"tlcnav.gjd", nullptr, -1),
			Common::EN_ANY, Common::kPlatformWindows, ADGF_UNSTABLE,
			GUIO2(GUIO_NOMIDI, GUIO_NOASPECT)
		},
		kGroovieTLC
	},

	// Tender Loving Care PC English (DVD-ROM)
	{
		{
			"tlc", "DVD",
			AD_ENTRY2s("tlcmain.grv", "151af191015beb6f662919153e6c28d8", 17379,
						"tlcnav.gjd", nullptr, -1),
			Common::EN_ANY, Common::kPlatformWindows, ADGF_UNSTABLE,
			GUIO2(GUIO_NOMIDI, GUIO_NOASPECT)
		},
		kGroovieTLC
	},

	// Tender Loving Care PC Demo German https://archive.org/details/Tender_Loving_Care_demo
	{
		{
			"tlc", "Demo",
			AD_ENTRY2s("tlcmain.grv", "6ec818f595eedca6570280af0c681642", 17361,
						"tlcnav.gjd", nullptr, -1),
			Common::DE_DEU, Common::kPlatformWindows, ADGF_DEMO | ADGF_UNSTABLE,
			GUIO2(GUIO_NOMIDI, GUIO_NOASPECT)
		},
		kGroovieTLC
	},

	// Tender Loving Care PC Trailer https://archive.org/details/Tender_Loving_Care_demo
	{
		{
			"tlc", "Trailer",
			AD_ENTRY2s("preview.grv", "d95401509a0ef251e8c340737edf728c", 19,
						"drama1.gjd", nullptr, -1),
			Common::UNK_LANG, Common::kPlatformWindows, ADGF_DEMO | ADGF_UNSTABLE,
			GUIO2(GUIO_NOMIDI, GUIO_NOASPECT)
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
